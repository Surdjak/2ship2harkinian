/*
 * File: z_en_boom.c
 * Overlay: ovl_En_Boom
 * Description: Thrown Boomerang. Actor spawns when thrown and is killed when caught.
 *
 * 3Ship [Hyrule] OoT's Boomerang for MM's Player (docs/OOT_ITEMS.md). Player's Zora boomerang flags and
 * pointer stand in for OoT's; collectibles are MM's En_Item00.
 */

#include "z_en_boom.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EnBoom_Init(Actor* thisx, PlayState* play);
static void EnBoom_Destroy(Actor* thisx, PlayState* play);
static void EnBoom_Update(Actor* thisx, PlayState* play);
static void EnBoom_Draw(Actor* thisx, PlayState* play);

static void EnBoom_Fly(EnBoomOot* this, PlayState* play);

ActorProfile En_Boom_Oot_Profile = {
    /**/ ACTOR_EN_BOOM_Oot,
    /**/ ACTORCAT_MISC,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBoomOot),
    /**/ EnBoom_Init,
    /**/ EnBoom_Destroy,
    /**/ EnBoom_Update,
    /**/ EnBoom_Draw,
};

static ColliderQuadInit sQuadInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_PLAYER,
        COLSHAPE_QUAD,
    },
    {
        ELEM_MATERIAL_UNK2,
        { DMG_FROM_OOT(0x00000010), 0x00, 0x01 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_S8(attentionRangeType, 5, ICHAIN_CONTINUE),
    ICHAIN_VEC3S(shape.rot, 0, ICHAIN_STOP),
};

static void EnBoom_SetupAction(EnBoomOot* this, EnBoomOotActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void EnBoom_Init(Actor* thisx, PlayState* play) {
    EnBoomOot* this = (EnBoomOot*)thisx;
    EffectBlureInit1 blure;

    this->actor.room = -1;

    Actor_ProcessInitChain(&this->actor, sInitChain);

    blure.p1StartColor[0] = 255;
    blure.p1StartColor[1] = 255;
    blure.p1StartColor[2] = 100;
    blure.p1StartColor[3] = 255;

    blure.p2StartColor[0] = 255;
    blure.p2StartColor[1] = 255;
    blure.p2StartColor[2] = 100;
    blure.p2StartColor[3] = 64;

    blure.p1EndColor[0] = 255;
    blure.p1EndColor[1] = 255;
    blure.p1EndColor[2] = 100;
    blure.p1EndColor[3] = 0;

    blure.p2EndColor[0] = 255;
    blure.p2EndColor[1] = 255;
    blure.p2EndColor[2] = 100;
    blure.p2EndColor[3] = 0;

    blure.elemDuration = 8;
    blure.unkFlag = 0;
    blure.calcMode = 0;

    Effect_Add(play, &this->effectIndex, EFFECT_BLURE1, 0, 0, &blure);

    Collider_InitQuad(play, &this->collider);
    Collider_SetQuad(play, &this->collider, &this->actor, &sQuadInit);

    EnBoom_SetupAction(this, EnBoom_Fly);
}

static void EnBoom_Destroy(Actor* thisx, PlayState* play) {
    EnBoomOot* this = (EnBoomOot*)thisx;

    Effect_Destroy(play, this->effectIndex);
    Collider_DestroyQuad(play, &this->collider);
}

static void EnBoom_Fly(EnBoomOot* this, PlayState* play) {
    Actor* target;
    Player* player;
    s32 collided;
    s16 yawTarget;
    s16 yawDiff;
    s16 pitchTarget;
    s16 pitchDiff;
    s32 pad1;
    f32 distXYZScale;
    f32 distFromLink;
    DynaPolyActor* hitActor;
    s32 hitDynaID;
    Vec3f hitPoint;
    s32 pad2;

    player = GET_PLAYER(play);
    target = this->moveTo;

    // If the boomerang is moving toward a targeted actor, handle setting the proper x and y angle to fly toward it.
    if (target != NULL) {
        yawTarget = Actor_WorldYawTowardPoint(&this->actor, &target->focus.pos);
        yawDiff = this->actor.world.rot.y - yawTarget;

        pitchTarget = Actor_WorldPitchTowardPoint(&this->actor, &target->focus.pos);
        pitchDiff = this->actor.world.rot.x - pitchTarget;

        distXYZScale = (200.0f - Math_Vec3f_DistXYZ(&this->actor.world.pos, &target->focus.pos)) * 0.005f;
        if (distXYZScale < 0.12f) {
            distXYZScale = 0.12f;
        }

        if ((target != &player->actor) && ((target->update == NULL) || (ABS(yawDiff) > 0x4000))) {
            //! @bug  This condition is why the boomerang will randomly fly off in a the down left direction sometimes.
            //      If the actor targetted is not Link and the difference between the 2 y angles is greater than 0x4000,
            //      the moveTo pointer is nulled and it flies off in a seemingly random direction.
            this->moveTo = NULL;
        } else {
            Math_ScaledStepToS(&this->actor.world.rot.y, yawTarget, (s16)(ABS(yawDiff) * distXYZScale));
            Math_ScaledStepToS(&this->actor.world.rot.x, pitchTarget, (s16)(ABS(pitchDiff) * distXYZScale));
        }
    }

    // Set xyz speed, move forward, and play the boomerang sound
    Actor_SetSpeeds(&this->actor, 12.0f);
    Actor_MoveWithGravity(&this->actor);
    Actor_PlaySfx_Flagged(&this->actor, NA_SE_IT_BOOMERANG_FLY - SFX_FLAG);

    // If the boomerang collides with EnItem00 or a Skulltula token, set grabbed pointer to pick it up
    collided = this->collider.base.atFlags & AT_HIT;
    collided = !!(collided);
    if (collided) {
        if (((this->collider.base.at->id == ACTOR_EN_ITEM00) || (this->collider.base.at->id == ACTOR_EN_SI_Oot))) {
            this->grabbed = this->collider.base.at;
            if (this->collider.base.at->id == ACTOR_EN_SI_Oot) {
                this->collider.base.at->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED;
            }
        }
    }

    // Decrement the return timer and check if it's 0. If it is, check if Link can catch it and handle accordingly.
    // Otherwise handle grabbing and colliding.
    if (DECR(this->returnTimer) == 0) {
        distFromLink = Math_Vec3f_DistXYZ(&this->actor.world.pos, &player->actor.focus.pos);
        this->moveTo = &player->actor;

        // If the boomerang is less than 40 units away from Link, he can catch it.
        if (distFromLink < 40.0f) {
            target = this->grabbed;
            if (target != NULL) {
                Math_Vec3f_Copy(&target->world.pos, &player->actor.world.pos);

                // If the grabbed actor is EnItem00 (HP/Key etc) set gravity and flags so it falls in front of Link.
                // Otherwise if it's a Skulltula Token, just set flags so he collides with it to collect it.
                if (target->id == ACTOR_EN_ITEM00) {
                    target->gravity = -0.9f;
                    target->bgCheckFlags &= ~0x03;
                } else {
                    target->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
                }
            }
            // Set player flags and kill the boomerang beacause Link caught it.
            // 3Ship [Hyrule] MM's Player catches on PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT.
            player->stateFlags1 &= ~PLAYER_STATE1_ZORA_BOOMERANG_THROWN;
            player->stateFlags3 |= PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT;
            Actor_Kill(&this->actor);
        }
    } else {
        collided = (this->collider.base.atFlags & AT_HIT);
        collided = (!!(collided));
        if (collided) {
            // Copy the position from the prevous frame to the boomerang to start the bounce back.
            Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.prevPos);
        } else {
            collided = BgCheck_EntityLineTest1(&play->colCtx, &this->actor.prevPos, &this->actor.world.pos, &hitPoint,
                                               &this->actor.wallPoly, true, true, true, true, &hitDynaID);

            if (collided) {
                // If the boomerang collides with something and it's is a Jabu Object actor with params equal to 0, then
                // set collided to 0 so that the boomerang will go through the wall.
                // Otherwise play a clank sound and keep collided set to bounce back.
                // 3Ship [Hyrule] OoT's func_8002F9EC (Jabu-Jabu's flesh walls) comes with Jabu-Jabu.
                if ((hitDynaID != BGCHECK_SCENE) &&
                    ((hitActor = DynaPoly_GetActor(&play->colCtx, hitDynaID)) != NULL) &&
                    (hitActor->actor.id == ACTOR_BG_BDAN_OBJECTS_Oot) && (hitActor->actor.params == 0)) {
                    collided = false;
                } else {
                    CollisionCheck_SpawnShieldParticlesMetal(play, &hitPoint);
                }
            }
        }

        // If the boomerang needs to bounce back, set x and y angle accordingly.
        // Set timer to 0 and set return actor to player so it goes back to Link.
        if (collided) {
            this->actor.world.rot.x = -this->actor.world.rot.x;
            this->actor.world.rot.y += 0x8000;
            this->moveTo = &player->actor;
            this->returnTimer = 0;
        }
    }

    // If the actor the boomerang is holding has a null update function, set grabbed to null.
    // Otherwise, copy the position from the boomerang to the actor to move it.
    target = this->grabbed;
    if (target != NULL) {
        if (target->update == NULL) {
            this->grabbed = NULL;
        } else {
            Math_Vec3f_Copy(&target->world.pos, &this->actor.world.pos);
        }
    }
}

static void EnBoom_Update(Actor* thisx, PlayState* play) {
    EnBoomOot* this = (EnBoomOot*)thisx;
    Player* player = GET_PLAYER(play);

    if (!(player->stateFlags1 & PLAYER_STATE1_20000000)) {
        this->actionFunc(this, play);
        Actor_SetFocus(&this->actor, 0.0f);
        this->activeTimer = this->activeTimer + 1;
    }
}

static void EnBoom_Draw(Actor* thisx, PlayState* play) {
    static Vec3f sMultVec1 = { -960.0f, 0.0f, 0.0f };
    static Vec3f sMultVec2 = { 960.0f, 0.0f, 0.0f };
    EnBoomOot* this = (EnBoomOot*)thisx;
    Vec3f vec1;
    Vec3f vec2;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_RotateYF(this->actor.world.rot.y * (M_PI / 0x8000), MTXMODE_APPLY);
    Matrix_RotateZF(0x1F40 * (M_PI / 0x8000), MTXMODE_APPLY);
    Matrix_RotateXFApply(this->actor.world.rot.x * (M_PI / 0x8000));
    Matrix_MultVec3f(&sMultVec1, &vec1);
    Matrix_MultVec3f(&sMultVec2, &vec2);

    if (func_80126440(play, &this->collider, &this->boomerangInfo, &vec1, &vec2) != 0) {
        EffectBlure_AddVertex(Effect_GetByIndex(this->effectIndex), &vec1, &vec2);
    }

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Matrix_RotateYF((this->activeTimer * 12000) * (M_PI / 0x8000), MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gBoomerangRefDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
