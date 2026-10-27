/*
 * File: z_en_insect.c
 * Overlay: ovl_En_Insect
 * Description: Bugs
 */

#include "z_en_insect.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS 0

static void EnInsect_Init(Actor* thisx, PlayState* play);
static void EnInsect_Destroy(Actor* thisx, PlayState* play);
static void EnInsect_Update(Actor* thisx, PlayState* play);
static void EnInsect_Draw(Actor* thisx, PlayState* play);
static void EnInsect_Reset(void);

static void EnInsect_SetupSlowDown(EnInsect* this);
static void EnInsect_SlowDown(EnInsect* this, PlayState* play);
static void EnInsect_SetupCrawl(EnInsect* this);
static void EnInsect_Crawl(EnInsect* this, PlayState* play);
static void EnInsect_SetupRunFromPlayer(EnInsect* this);
static void EnInsect_RunFromPlayer(EnInsect* this, PlayState* play);
static void EnInsect_Caught(EnInsect* this, PlayState* play);
static void EnInsect_SetupDig(EnInsect* this);
static void EnInsect_Dig(EnInsect* this, PlayState* play);
static void EnInsect_SetupWalkOnWater(EnInsect* this);
static void EnInsect_WalkOnWater(EnInsect* this, PlayState* play);
static void EnInsect_SetupDrown(EnInsect* this);
static void EnInsect_Drown(EnInsect* this, PlayState* play);
static void EnInsect_SetupDropped(EnInsect* this);
static void EnInsect_Dropped(EnInsect* this, PlayState* play);

static f32 D_80A7DEB0 = 0.0f;
static s16 sCaughtCount = 0;
s16 sDroppedCount = 0; // not static: read by NoBugsDespawn.cpp

ActorProfile En_Insect_Oot_Profile = {
    /**/ ACTOR_EN_INSECT_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnInsect),
    /**/ EnInsect_Init,
    /**/ EnInsect_Destroy,
    /**/ EnInsect_Update,
    /**/ EnInsect_Draw,
    /**/ EnInsect_Reset,
};

static ColliderJntSphElementInit sColliderItemInit[1] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_NONE,
            OCELEM_ON,
        },
        { 0, { { 0, 0, 0 }, 5 }, 100 },
    },
};

static ColliderJntSphInit sColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_PLAYER | OC1_TYPE_1,
        OC2_TYPE_1,
        COLSHAPE_JNTSPH,
    },
    1,
    sColliderItemInit,
};

static u16 D_80A7DF10[] = { 0, 5, 7, 7 };

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 10, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 700, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 20, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 600, ICHAIN_STOP),
};

static void EnInsect_InitFlags(EnInsect* this) {
    this->insectFlags = D_80A7DF10[this->actor.params & 3];
}

static f32 EnInsect_XZDistanceSquared(Vec3f* v1, Vec3f* v2) {
    return SQ(v1->x - v2->x) + SQ(v1->z - v2->z);
}

static s32 EnInsect_InBottleRange(EnInsect* this, PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);
    Vec3f pos;

    if (this->actor.xzDistToPlayer < 32.0f) {
        pos.x = Math_SinS(this->actor.yawTowardsPlayer + 0x8000) * 16.0f + player->actor.world.pos.x;
        pos.y = player->actor.world.pos.y;
        pos.z = Math_CosS(this->actor.yawTowardsPlayer + 0x8000) * 16.0f + player->actor.world.pos.z;

        //! @bug: this check is superfluous: it is automatically satisfied if the coarse check is satisfied. It may have
        //! been intended to check the actor is in front of Player, but yawTowardsPlayer does not depend on Player's
        //! world rotation.
        if (EnInsect_XZDistanceSquared(&pos, &this->actor.world.pos) <= SQ(20.0f)) {
            return true;
        }
    }

    return false;
}

static void EnInsect_SetCrawlAnim(EnInsect* this) {
    Animation_Change(&this->skelAnime, &gBugCrawlAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP_INTERP, 0.0f);
}

/**
 * Find the nearest soft dirt patch within 6400 units in the xz plane and the current room
 *
 * @return 1 if one was found, 0 otherwise
 */
static s32 EnInsect_FoundNearbySoil(EnInsect* this, PlayState* play) {
    Actor* currentActor;
    f32 currentDistance;
    f32 bestDistance;
    s32 ret;

    ret = 0;
    currentActor = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
    bestDistance = 6400.0f;
    this->soilActor = NULL;

    while (currentActor != NULL) {
        if (currentActor->id == ACTOR_OBJ_MAKEKINSUTA_Oot) {
            currentDistance = Math3D_Dist2DSq(this->actor.world.pos.x, this->actor.world.pos.z,
                                              currentActor->world.pos.x, currentActor->world.pos.z);

            if (currentDistance < bestDistance && currentActor->room == this->actor.room) {
                ret = 1;
                bestDistance = currentDistance;
                this->soilActor = (ObjMakekinsuta*)currentActor;
            }
        }
        currentActor = currentActor->next;
    }
    return ret;
}

static void EnInsect_UpdateCrawlSfx(EnInsect* this) {
    if (this->crawlSoundDelay > 0) {
        this->crawlSoundDelay--;
        return;
    }

    Actor_PlaySfx(&this->actor, NA_SE_EN_MUSI_WALK);

    this->crawlSoundDelay = 3.0f / CLAMP_MIN(this->skelAnime.playSpeed, 0.1f);
    if (this->crawlSoundDelay < 2) {
        this->crawlSoundDelay = 2;
    }
}

static void EnInsect_Init(Actor* thisx, PlayState* play2) {
    EnInsect* this = (EnInsect*)thisx;
    PlayState* play = play2;
    f32 rand;
    s16 temp_s2;
    s32 count;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    EnInsect_InitFlags(this);

    temp_s2 = this->actor.params & 3;

    SkelAnime_Init(play, &this->skelAnime, &gBugSkel, &gBugCrawlAnim, this->jointTable, this->morphTable, 24);
    Collider_InitJntSph(play, &this->collider);
    Collider_SetJntSph(play, &this->collider, &this->actor, &sColliderInit, &this->colliderItem);

    this->actor.colChkInfo.mass = 30;

    if (this->insectFlags & 1) {
        this->actor.gravity = -0.2f;
        this->actor.terminalVelocity = -2.0f;
    }

    if (this->insectFlags & 4) {
        this->lifeTimer = Rand_S16Offset(200, 40);
        this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    }

    if (temp_s2 == 2 || temp_s2 == 3) {
        if (EnInsect_FoundNearbySoil(this, play)) {
            this->insectFlags |= 0x10;
            D_80A7DEB0 = 0.0f;
        }

        if (temp_s2 == 2) {
            this->actor.world.rot.z = 0;
            this->actor.shape.rot.z = this->actor.world.rot.z;

            for (count = 0; count < 2; count++) {
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_INSECT_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                            this->actor.world.pos.z, this->actor.shape.rot.x, this->actor.shape.rot.y,
                            this->actor.shape.rot.z, 3);
            }
        }

        EnInsect_SetupDropped(this);

        sDroppedCount++;
    } else {
        rand = Rand_ZeroOne();

        if (rand < 0.3f) {
            EnInsect_SetupSlowDown(this);
        } else if (rand < 0.4f) {
            EnInsect_SetupCrawl(this);
        } else {
            EnInsect_SetupRunFromPlayer(this);
        }
    }
}

static void EnInsect_Destroy(Actor* thisx, PlayState* play) {
    s16 temp_v0;
    EnInsect* this = (EnInsect*)thisx;

    temp_v0 = this->actor.params & 3;
    Collider_DestroyJntSph(play, &this->collider);
    if ((temp_v0 == 2 || temp_v0 == 3) && sDroppedCount > 0) {
        sDroppedCount--;
    }
}

static void EnInsect_SetupSlowDown(EnInsect* this) {
    this->actionTimer = Rand_S16Offset(5, 35);
    EnInsect_SetCrawlAnim(this);
    this->actionFunc = EnInsect_SlowDown;
    this->insectFlags |= 0x100;
}

static void EnInsect_SlowDown(EnInsect* this, PlayState* play) {
    s32 pad[2];
    s16 sp2E;
    f32 playSpeed;

    sp2E = this->actor.params & 3;

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 0.1f, 0.5f, 0.0f);

    playSpeed = (Rand_ZeroOne() * 0.8f) + (this->actor.speed * 1.2f);
    this->skelAnime.playSpeed = CLAMP(playSpeed, 0.0f, 1.9f);

    SkelAnime_Update(&this->skelAnime);
    this->actor.shape.rot.y = this->actor.world.rot.y;
    if (this->actionTimer <= 0) {
        EnInsect_SetupCrawl(this);
    }

    if (((this->insectFlags & 4) && this->lifeTimer <= 0) ||
        ((sp2E == 2 || sp2E == 3) && (this->insectFlags & 1) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
         sDroppedCount >= 4)) {
        EnInsect_SetupDig(this);
    } else if ((this->insectFlags & 1) && (this->actor.bgCheckFlags & 0x40)) {
        EnInsect_SetupWalkOnWater(this);
    } else if (this->actor.xzDistToPlayer < 40.0f) {
        EnInsect_SetupRunFromPlayer(this);
    }
}

static void EnInsect_SetupCrawl(EnInsect* this) {
    this->actionTimer = Rand_S16Offset(10, 45);
    EnInsect_SetCrawlAnim(this);
    this->actionFunc = EnInsect_Crawl;
    this->insectFlags |= 0x100;
}

static void EnInsect_Crawl(EnInsect* this, PlayState* play) {
    s32 pad1;
    s32 pad2;
    s16 yaw;
    s16 sp34 = this->actor.params & 3;

    Math_SmoothStepToF(&this->actor.speed, 1.5f, 0.1f, 0.5f, 0.0f);

    if (EnInsect_XZDistanceSquared(&this->actor.world.pos, &this->actor.home.pos) > 1600.0f ||
        (this->actionTimer < 4)) {
        yaw = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.home.pos);
        Math_ScaledStepToS(&this->actor.world.rot.y, yaw, 2000);
    } else if (this->actor.child != NULL && &this->actor != this->actor.child) {
        yaw = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.child->world.pos);
        Math_ScaledStepToS(&this->actor.world.rot.y, yaw, 2000);
    }

    this->actor.shape.rot.y = this->actor.world.rot.y;
    this->skelAnime.playSpeed = CLAMP(this->actor.speed * 1.4f, 0.7f, 1.9f);

    SkelAnime_Update(&this->skelAnime);

    if (this->actionTimer <= 0) {
        EnInsect_SetupSlowDown(this);
    }

    if (((this->insectFlags & 4) && this->lifeTimer <= 0) ||
        ((sp34 == 2 || sp34 == 3) && (this->insectFlags & 1) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
         sDroppedCount >= 4)) {
        EnInsect_SetupDig(this);
    } else if ((this->insectFlags & 1) && (this->actor.bgCheckFlags & 0x40)) {
        EnInsect_SetupWalkOnWater(this);
    } else if (this->actor.xzDistToPlayer < 40.0f) {
        EnInsect_SetupRunFromPlayer(this);
    }
}

static void EnInsect_SetupRunFromPlayer(EnInsect* this) {
    this->actionTimer = Rand_S16Offset(10, 40);
    EnInsect_SetCrawlAnim(this);
    this->actionFunc = EnInsect_RunFromPlayer;
    this->insectFlags |= 0x100;
}

static void EnInsect_RunFromPlayer(EnInsect* this, PlayState* play) {
    s32 pad1;
    s32 pad2;
    s16 pad3;
    s16 frames;
    s16 yaw;
    s16 sp38 = this->actor.xzDistToPlayer < 40.0f;

    Math_SmoothStepToF(&this->actor.speed, 1.8f, 0.1f, 0.5f, 0.0f);

    if (EnInsect_XZDistanceSquared(&this->actor.world.pos, &this->actor.home.pos) > 25600.0f || this->actionTimer < 4) {
        yaw = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.home.pos);
        Math_ScaledStepToS(&this->actor.world.rot.y, yaw, 2000);
    } else if (sp38 != 0) {
        frames = play->state.frames;
        yaw = this->actor.yawTowardsPlayer + 0x8000;

        if (frames & 0x10) {
            if (frames & 0x20) {
                yaw += 0x2000;
            }
        } else {
            if (frames & 0x20) {
                yaw -= 0x2000;
            }
        }
        if (play) {} // Must be 'play'
        Math_ScaledStepToS(&this->actor.world.rot.y, yaw, 2000);
    }
    this->actor.shape.rot.y = this->actor.world.rot.y;
    this->skelAnime.playSpeed = CLAMP(this->actor.speed * 1.6f, 0.8f, 1.9f);
    SkelAnime_Update(&this->skelAnime);

    if (this->actionTimer <= 0 || !sp38) {
        EnInsect_SetupSlowDown(this);
    } else if ((this->insectFlags & 1) && (this->actor.bgCheckFlags & 0x40)) {
        EnInsect_SetupWalkOnWater(this);
    }
}

static void EnInsect_SetupCaught(EnInsect* this) {
    this->actionTimer = 200;

    Actor_SetScale(&this->actor, 0.001f);

    this->actor.draw = NULL;
    this->actor.speed = 0.0f;

    EnInsect_SetCrawlAnim(this);

    this->skelAnime.playSpeed = 0.3f;
    this->actionFunc = EnInsect_Caught;
    this->insectFlags &= ~0x100;
}

static void EnInsect_Caught(EnInsect* this, PlayState* play) {
    if (this->actionTimer == 20 && !(this->insectFlags & 4)) {
        this->actor.draw = EnInsect_Draw;
    } else if (this->actionTimer == 0) {
        if (this->insectFlags & 4) {
            Actor_Kill(&this->actor);
        } else {
            Actor_SetScale(&this->actor, 0.01f);
            EnInsect_SetupSlowDown(this);
        }
    } else if (this->actionTimer < 20) {
        Actor_SetScale(&this->actor, CLAMP_MAX(this->actor.scale.x + 0.001f, 0.01f));
        SkelAnime_Update(&this->skelAnime);
    }
}

static void EnInsect_SetupDig(EnInsect* this) {
    this->actionTimer = 60;
    EnInsect_SetCrawlAnim(this);
    this->skelAnime.playSpeed = 1.9f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_STALTURA_BOUND);
    Math_Vec3f_Copy(&this->actor.home.pos, &this->actor.world.pos);
    this->actionFunc = EnInsect_Dig;
    this->insectFlags &= ~0x100;
    this->insectFlags |= 0x8;
}

static void EnInsect_Dig(EnInsect* this, PlayState* play) {
    static Vec3f accel = { 0.0f, 0.0f, 0.0f };
    static Vec3f unused = { 0.0f, 0.0f, 0.0f };
    s32 pad[2];
    Vec3f velocity;

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 0.1f, 0.5f, 0.0f);
    Math_StepToS(&this->actor.shape.rot.x, 10922, 352);

    Actor_SetScale(&this->actor, CLAMP_MIN(this->actor.scale.x - 0.0002f, 0.001f));

    this->actor.shape.yOffset -= 0.8f;
    this->actor.world.pos.x = Rand_ZeroOne() + this->actor.home.pos.x - 0.5f;
    this->actor.world.pos.z = Rand_ZeroOne() + this->actor.home.pos.z - 0.5f;

    SkelAnime_Update(&this->skelAnime);

    if (this->actionTimer > 20 && Rand_ZeroOne() < 0.1f) {
        velocity.x = Math_SinS(this->actor.shape.rot.y) * -0.6f;
        velocity.y = Math_SinS(this->actor.shape.rot.x) * 0.6f;
        velocity.z = Math_CosS(this->actor.shape.rot.y) * -0.6f;
        func_800286CC(play, &this->actor.world.pos, &velocity, &accel, Rand_ZeroOne() * 5.0f + 8.0f,
                      Rand_ZeroOne() * 5.0f + 8.0f);
    }

    if (this->actionTimer <= 0) {
        if ((this->insectFlags & 0x10) && this->soilActor != NULL &&
            Math3D_Vec3fDistSq(&this->soilActor->actor.world.pos, &this->actor.world.pos) < 64.0f) {
            this->soilActor->unk_152 = 1;
        }
        Actor_Kill(&this->actor);
    }
}

static void EnInsect_SetupWalkOnWater(EnInsect* this) {
    this->actionTimer = Rand_S16Offset(120, 50);
    EnInsect_SetCrawlAnim(this);
    this->unk_316 = this->unk_318 = 0;
    this->actionFunc = EnInsect_WalkOnWater;
    this->insectFlags &= ~0x100;
}

static void EnInsect_WalkOnWater(EnInsect* this, PlayState* play) {
    f32 temp_f0;
    s16 temp_v1;
    s16 pad;
    s16 sp4E;
    Vec3f sp40;
    s32 phi_v0;
    s32 phi_v0_2;

    sp4E = this->actor.params & 3;

    if (this->actionTimer >= 81) {
        Math_StepToF(&this->actor.speed, 0.6f, 0.08f);
    } else {
        Math_StepToF(&this->actor.speed, 0.0f, 0.02f);
    }
    this->actor.velocity.y = 0.0f;
    this->actor.world.pos.y += this->actor.depthInWater;
    this->skelAnime.playSpeed = CLAMP(this->actionTimer * 0.018f, 0.1f, 1.9f);

    SkelAnime_Update(&this->skelAnime);

    if (this->actionTimer >= 81) {
        this->unk_316 += Rand_S16Offset(-50, 100);
        this->unk_318 += Rand_S16Offset(-300, 600);
    }

    temp_v1 = this->skelAnime.playSpeed * 200.0f;

    if (this->unk_316 < -temp_v1) {
        this->unk_316 = -temp_v1;
    } else {
        if (temp_v1 < this->unk_316) {
            phi_v0 = temp_v1;
        } else {
            phi_v0 = this->unk_316;
        }
        this->unk_316 = phi_v0;
    }
    this->actor.world.rot.y += this->unk_316;

    temp_v1 = this->skelAnime.playSpeed * 1000.0f;
    if (this->unk_318 < -temp_v1) {
        this->unk_318 = -temp_v1;
    } else {
        if (temp_v1 < this->unk_318) {
            phi_v0_2 = temp_v1;
        } else {
            phi_v0_2 = this->unk_318;
        }
        this->unk_318 = phi_v0_2;
    }
    this->actor.shape.rot.y += this->unk_318;

    Math_ScaledStepToS(&this->actor.world.rot.x, 0, 3000);
    this->actor.shape.rot.x = this->actor.world.rot.x;

    if (Rand_ZeroOne() < 0.03f) {
        sp40.x = this->actor.world.pos.x;
        sp40.y = this->actor.world.pos.y + this->actor.depthInWater;
        sp40.z = this->actor.world.pos.z;
        EffectSsGRipple_Spawn(play, &sp40, 20, 100, 4);
        EffectSsGRipple_Spawn(play, &sp40, 40, 200, 8);
    }

    if (this->actionTimer <= 0 || ((this->insectFlags & 4) && this->lifeTimer <= 0) ||
        ((sp4E == 2 || sp4E == 3) && (this->insectFlags & 1) && sDroppedCount >= 4)) {
        EnInsect_SetupDrown(this);
    } else if (!(this->actor.bgCheckFlags & 0x40)) {
        if (this->insectFlags & 0x10) {
            EnInsect_SetupDropped(this);
        } else {
            EnInsect_SetupSlowDown(this);
        }
    }
}

static void EnInsect_SetupDrown(EnInsect* this) {
    this->actionTimer = 100;
    EnInsect_SetCrawlAnim(this);
    this->actor.velocity.y = 0.0f;
    this->actor.speed = 0.0f;
    this->actor.terminalVelocity = -0.8f;
    this->actor.gravity = -0.04f;
    this->insectFlags &= ~0x3;
    this->actionFunc = EnInsect_Drown;
    this->insectFlags &= ~0x100;
    this->insectFlags |= 8;
}

static void EnInsect_Drown(EnInsect* this, PlayState* play) {
    this->actor.shape.rot.x -= 500;
    this->actor.shape.rot.y += 200;
    Actor_SetScale(&this->actor, CLAMP_MIN(this->actor.scale.x - 0.00005f, 0.001f));

    if (this->actor.depthInWater > 5.0f && this->actor.depthInWater < 30.0f && Rand_ZeroOne() < 0.3f) {
        EffectSsBubble_Spawn(play, &this->actor.world.pos, -5.0f, 5.0f, 5.0f, (Rand_ZeroOne() * 0.04f) + 0.02f);
    }

    if (this->actionTimer <= 0) {
        Actor_Kill(&this->actor);
    }
}

static void EnInsect_SetupDropped(EnInsect* this) {
    EnInsect_SetCrawlAnim(this);
    this->actionTimer = 100;
    this->unk_324 = 1.5f;
    this->unk_328 = Rand_ZeroOne() * (0xFFFF + 0.5f);
    this->unk_316 = (Rand_ZeroOne() - 0.5f) * 1500.0f;
    this->actor.world.rot.y = Rand_ZeroOne() * (0xFFFF + 0.5f);
    Actor_SetScale(&this->actor, 0.003f);
    this->actionFunc = EnInsect_Dropped;
    this->insectFlags |= 0x100;
}

static void EnInsect_Dropped(EnInsect* this, PlayState* play) {
    s32 temp_a0;
    s32 sp50;
    f32 phi_f0;
    EnInsect* thisTemp = this;
    s32 temp_a1;
    f32 sp40;
    f32 phi_f2;
    s16 sp3A;
    s16 sp38;
    f32 sp34;

    sp50 = 0;
    sp3A = this->actor.params & 3;

    if (this->soilActor != NULL) {
        sp40 = Math3D_Vec3fDistSq(&this->actor.world.pos, &this->soilActor->actor.world.pos);
    } else {
        if (this->insectFlags & 0x10) {
        }
        sp40 = 40.0f;
    }

    D_80A7DEB0 += 0.99999994f / 300.0f;
    if (D_80A7DEB0 > 1.0f) {
        D_80A7DEB0 = 1.0f;
    }

    if (D_80A7DEB0 > 0.999f) {
        phi_f2 = 0.0f;
    } else {
        if (sp40 > 900.0f) {
            phi_f2 = ((1.1f - D_80A7DEB0) * 100.0f) + 20.0f;
        } else {
            phi_f2 = (1.0f - D_80A7DEB0) * 10.0f;
        }
    }

    if (this->soilActor != NULL && Rand_ZeroOne() < 0.07f) {
        this->actor.home.pos.x = (Rand_ZeroOne() - 0.5f) * phi_f2 + thisTemp->soilActor->actor.world.pos.x;
        this->actor.home.pos.y = thisTemp->soilActor->actor.world.pos.y;
        this->actor.home.pos.z = (Rand_ZeroOne() - 0.5f) * phi_f2 + thisTemp->soilActor->actor.world.pos.z;
    }

    if (D_80A7DEB0 > 0.999f) {
        this->unk_328 = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.home.pos);
        this->unk_324 = Rand_ZeroOne() * 0.6f + 0.6f;
    } else if (Rand_ZeroOne() < 0.07f) {
        if (this->unk_324 > 1.0f) {
            this->unk_324 = 0.1f;
        } else {
            this->unk_324 = Rand_ZeroOne() * 0.8f + 1.0f;
        }

        sp34 = 1.3f - D_80A7DEB0;
        if (sp34 < 0.0f) {
            sp34 = 0.0f;
        } else {
            if (sp34 > 1.0f) {
                phi_f0 = 1.0f;
            } else {
                phi_f0 = sp34;
            }
            sp34 = phi_f0;
        }

        sp38 = (Rand_ZeroOne() - 0.5f) * 65535.0f * sp34;
        this->unk_328 = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.home.pos) + sp38;
    }

    Actor_SetScale(&this->actor, CLAMP_MAX(thisTemp->actor.scale.x + 0.0008f, 0.01f));

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        Math_SmoothStepToF(&this->actor.speed, this->unk_324, 0.1f, 0.5f, 0.0f);
        Math_ScaledStepToS(&this->actor.world.rot.y, this->unk_328, 2000);
        sp50 = Math_ScaledStepToS(&this->actor.world.rot.x, 0, 2000);
        this->actor.shape.rot.y = this->actor.world.rot.y;
        this->actor.shape.rot.x = this->actor.world.rot.x;
    } else {
        Math_SmoothStepToF(&this->actor.speed, 0.0f, 0.1f, 0.5f, 0.0f);
        this->actor.speed += (Rand_ZeroOne() - 0.5f) * 0.14f;
        this->actor.velocity.y += Rand_ZeroOne() * 0.12f;
        this->actor.world.rot.y += this->unk_316;
        this->actor.shape.rot.y = this->actor.world.rot.y;
        this->actor.shape.rot.x -= 2000;
    }

    phi_f2 = Rand_ZeroOne() * 0.5f + this->actor.speed * 1.3f;
    if (phi_f2 < 0.0f) {
        this->skelAnime.playSpeed = 0.0f;
    } else {
        if (phi_f2 > 1.9f) {
            phi_f0 = 1.9f;
        } else {
            phi_f0 = phi_f2;
        }
        this->skelAnime.playSpeed = phi_f0;
    }

    SkelAnime_Update(&this->skelAnime);
    if (!(this->insectFlags & 0x40) && (this->insectFlags & 1) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_GERUDOFT_WALK);
        this->insectFlags |= 0x40;
    }

    if (sp3A == 2 && (this->insectFlags & 0x10) && !(this->insectFlags & 0x80)) {
        if (this->unk_32A >= 15) {
            if (this->soilActor != NULL) {
                if (!(GET_GS_FLAGS_OOT(((this->soilActor->actor.params >> 8) & 0x1F) - 1) &
                      (this->soilActor->actor.params & 0xFF))) {
                    Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
                }
            }
            this->insectFlags |= 0x80;
        } else {
            this->unk_32A++;
        }
    }

    if ((this->insectFlags & 1) && (this->actor.bgCheckFlags & 0x40)) {
        EnInsect_SetupWalkOnWater(this);
    } else if (this->insectFlags & 0x10) {
        if (sp40 < 9.0f) {
            EnInsect_SetupDig(this);
        } else if (this->actionTimer <= 0 || this->lifeTimer <= 0 ||
                   ((this->insectFlags & 1) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && sDroppedCount >= 4 &&
                    (sp3A == 2 || sp3A == 3))) {
            EnInsect_SetupDig(this);
        } else {
            if (sp40 < 900.0f) {
                this->lifeTimer++;
                this->insectFlags |= 0x20;
            } else {
                this->actionTimer = 100;
            }
        }
    } else if (sp50 != 0) {
        EnInsect_SetupSlowDown(this);
    } else if ((sp3A == 2 || sp3A == 3) && (this->insectFlags & 1) && this->lifeTimer <= 0 && this->actionTimer <= 0 &&
               this->actor.floorHeight < BGCHECK_Y_MIN + 10.0f) {
        Actor_Kill(&this->actor);
    }
}

static void EnInsect_Update(Actor* thisx, PlayState* play) {
    EnInsect* this = (EnInsect*)thisx;
    s32 phi_v0;

    if (this->actor.child != NULL) {
        if (this->actor.child->update == NULL) {
            if (&this->actor != this->actor.child) {
                this->actor.child = NULL;
            }
        }
    }

    if (this->actionTimer > 0) {
        this->actionTimer--;
    }

    if (this->lifeTimer > 0) {
        this->lifeTimer--;
    }

    this->actionFunc(this, play);

    if (this->actor.update != NULL) {
        Actor_MoveWithGravity(&this->actor);
        if (this->insectFlags & 0x100) {
            if (this->insectFlags & 1) {
                if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
                    EnInsect_UpdateCrawlSfx(this);
                }
            } else {
                EnInsect_UpdateCrawlSfx(this);
            }
        }

        phi_v0 = 0;

        if (this->insectFlags & 1) {
            phi_v0 = 4;
        }

        if (this->insectFlags & 2) {
            phi_v0 |= 1;
        }

        if (phi_v0 != 0) {
            phi_v0 |= 0x40;
            Actor_UpdateBgCheckInfo(play, &this->actor, 8.0f, 5.0f, 0.0f, phi_v0);
        }

        if (Actor_HasParent(&this->actor, play)) {
            this->actor.parent = NULL;
            phi_v0 = this->actor.params & 3;

            if (phi_v0 == 2 || phi_v0 == 3) {
                Actor_Kill(&this->actor);
            } else {
                EnInsect_SetupCaught(this);
            }
        } else if (this->actor.xzDistToPlayer < 50.0f && this->actionFunc != EnInsect_Caught) {
            if (!(this->insectFlags & 0x20) && this->lifeTimer < 180) {
                CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            }

            if (!(this->insectFlags & 8) && sCaughtCount < 4 && EnInsect_InBottleRange(this, play) &&
                // GI_MAX in this case allows the player to catch the actor in a bottle
                Actor_OfferGetItem(&this->actor, play, GI_MAX, 60.0f, 30.0f)) {
                sCaughtCount++;
            }
        }

        Actor_SetFocus(&this->actor, 0.0f);
    }
}

static void EnInsect_Draw(Actor* thisx, PlayState* play) {
    EnInsect* this = (EnInsect*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, NULL, NULL, NULL);
    Collider_UpdateSpheres(0, &this->collider);
    sCaughtCount = 0;
}

static void EnInsect_Reset(void) {
    D_80A7DEB0 = 0.0f;
    sCaughtCount = 0;
    sDroppedCount = 0;
}
