#include "z_en_shopnuts.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_shopnuts/object_shopnuts.h"
#include "../ovl_En_Dns/z_en_dns.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE)

static void EnShopnuts_Init(Actor* thisx, PlayState* play);
static void EnShopnuts_Destroy(Actor* thisx, PlayState* play);
static void EnShopnuts_Update(Actor* thisx, PlayState* play);
static void EnShopnuts_Draw(Actor* thisx, PlayState* play);

static void EnShopnuts_SetupWait(EnShopnuts* this);
static void EnShopnuts_Wait(EnShopnuts* this, PlayState* play);
static void EnShopnuts_LookAround(EnShopnuts* this, PlayState* play);
static void EnShopnuts_Stand(EnShopnuts* this, PlayState* play);
static void EnShopnuts_ThrowNut(EnShopnuts* this, PlayState* play);
static void EnShopnuts_Burrow(EnShopnuts* this, PlayState* play);
static void EnShopnuts_SpawnSalesman(EnShopnuts* this, PlayState* play);

ActorProfile En_Shopnuts_Oot_Profile = {
    /**/ ACTOR_EN_SHOPNUTS_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_SHOPNUTS_Oot,
    /**/ sizeof(EnShopnuts),
    /**/ EnShopnuts_Init,
    /**/ EnShopnuts_Destroy,
    /**/ EnShopnuts_Update,
    /**/ EnShopnuts_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_HIT6,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 20, 40, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sColChkInfoInit = { 1, 20, 40, 0xFE };

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x4E
    ICHAIN_F32(gravity, -1, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 2600, ICHAIN_STOP),
};

static void EnShopnuts_Init(Actor* thisx, PlayState* play) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 35.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gBusinessScrubSkel, &gBusinessScrubPeekAnim, this->jointTable,
                       this->morphTable, 18);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
    Collider_UpdateCylinder(&this->actor, &this->collider);

    if ((((this->actor.params == DNS_TYPE_HEART_PIECE) &&
             (GET_ITEMGETINF_OOT(ITEMGETINF_DEKU_SCRUB_HEART_PIECE_Oot))) ||
                ((this->actor.params == DNS_TYPE_DEKU_STICK_UPGRADE) &&
                 (GET_INFTABLE_OOT(INFTABLE_BOUGHT_STICK_UPGRADE_Oot))) ||
                ((this->actor.params == DNS_TYPE_DEKU_NUT_UPGRADE) && (GET_INFTABLE_OOT(INFTABLE_BOUGHT_NUT_UPGRADE_Oot))))) {
        Actor_Kill(&this->actor);
    } else {
        EnShopnuts_SetupWait(this);
    }
}

static void EnShopnuts_Destroy(Actor* thisx, PlayState* play) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnShopnuts_SetupWait(EnShopnuts* this) {
    Animation_PlayOnceSetSpeed(&this->skelAnime, &gBusinessScrubInitialAnim, 0.0f);
    this->animFlagAndTimer = Rand_S16Offset(100, 50);
    this->collider.dim.height = 5;
    this->collider.base.acFlags &= ~AC_ON;
    this->actionFunc = EnShopnuts_Wait;
}

static void EnShopnuts_SetupLookAround(EnShopnuts* this) {
    Animation_PlayLoop(&this->skelAnime, &gBusinessScrubLookAroundAnim);
    this->animFlagAndTimer = 2;
    this->actionFunc = EnShopnuts_LookAround;
}

static void EnShopnuts_SetupThrowNut(EnShopnuts* this) {
    Animation_PlayOnce(&this->skelAnime, &gBusinessScrubThrowNutAnim);
    this->actionFunc = EnShopnuts_ThrowNut;
}

static void EnShopnuts_SetupStand(EnShopnuts* this) {
    Animation_MorphToLoop(&this->skelAnime, &gBusinessScrubPeekAnim, -3.0f);
    if (this->actionFunc == EnShopnuts_ThrowNut) {
        this->animFlagAndTimer = 2 | 0x1000; // sets timer and flag
    } else {
        this->animFlagAndTimer = 1;
    }
    this->actionFunc = EnShopnuts_Stand;
}

static void EnShopnuts_SetupBurrow(EnShopnuts* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gBusinessScrubPeekBurrowAnim, -5.0f);
    Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_DOWN);
    this->actionFunc = EnShopnuts_Burrow;
}

static void EnShopnuts_SetupSpawnSalesman(EnShopnuts* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gBusinessScrubRotateAnim, -3.0f);
    Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_DAMAGE);
    this->collider.base.acFlags &= ~AC_ON;
    this->actionFunc = EnShopnuts_SpawnSalesman;
}

static void EnShopnuts_Wait(EnShopnuts* this, PlayState* play) {
    s32 hasSlowPlaybackSpeed = false;

    if (this->skelAnime.playSpeed < 0.5f) {
        hasSlowPlaybackSpeed = true;
    }
    if (hasSlowPlaybackSpeed && (this->animFlagAndTimer != 0)) {
        this->animFlagAndTimer--;
    }
    if (Animation_OnFrame(&this->skelAnime, 9.0f)) {
        this->collider.base.acFlags |= AC_ON;
    } else if (Animation_OnFrame(&this->skelAnime, 8.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_UP);
    }

    this->collider.dim.height = ((CLAMP(this->skelAnime.curFrame, 9.0f, 13.0f) - 9.0f) * 9.0f) + 5.0f;
    if (!hasSlowPlaybackSpeed && (this->actor.xzDistToPlayer < 120.0f)) {
        EnShopnuts_SetupBurrow(this);
    } else if (SkelAnime_Update(&this->skelAnime)) {
        if (this->actor.xzDistToPlayer < 120.0f) {
            EnShopnuts_SetupBurrow(this);
        } else if ((this->animFlagAndTimer == 0) && (this->actor.xzDistToPlayer > 320.0f)) {
            EnShopnuts_SetupLookAround(this);
        } else {
            EnShopnuts_SetupStand(this);
        }
    }
    if (hasSlowPlaybackSpeed &&
        ((this->actor.xzDistToPlayer > 160.0f) && (fabsf(this->actor.playerHeightRel) < 120.0f)) &&
        ((this->animFlagAndTimer == 0) || (this->actor.xzDistToPlayer < 480.0f))) {
        this->skelAnime.playSpeed = 1.0f;
    }
}

static void EnShopnuts_LookAround(EnShopnuts* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f) && (this->animFlagAndTimer != 0)) {
        this->animFlagAndTimer--;
    }
    if ((this->actor.xzDistToPlayer < 120.0f) || (this->animFlagAndTimer == 0)) {
        EnShopnuts_SetupBurrow(this);
    }
}

static void EnShopnuts_Stand(EnShopnuts* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f) && (this->animFlagAndTimer != 0)) {
        this->animFlagAndTimer--;
    }
    if (!(this->animFlagAndTimer & 0x1000)) {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 2, 0xE38);
    }
    if ((this->actor.xzDistToPlayer < 120.0f) || (this->animFlagAndTimer == 0x1000)) {
        EnShopnuts_SetupBurrow(this);
    } else if (this->animFlagAndTimer == 0) {
        EnShopnuts_SetupThrowNut(this);
    }
}

static void EnShopnuts_ThrowNut(EnShopnuts* this, PlayState* play) {
    Vec3f spawnPos;

    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 2, 0xE38);
    if (this->actor.xzDistToPlayer < 120.0f) {
        EnShopnuts_SetupBurrow(this);
    } else if (SkelAnime_Update(&this->skelAnime)) {
        EnShopnuts_SetupStand(this);
    } else if (Animation_OnFrame(&this->skelAnime, 6.0f)) {
        spawnPos.x = this->actor.world.pos.x + (Math_SinS(this->actor.shape.rot.y) * 23.0f);
        spawnPos.y = this->actor.world.pos.y + 12.0f;
        spawnPos.z = this->actor.world.pos.z + (Math_CosS(this->actor.shape.rot.y) * 23.0f);
        if (Actor_Spawn(&play->actorCtx, play, ACTOR_EN_NUTSBALL_Oot, spawnPos.x, spawnPos.y, spawnPos.z,
                        this->actor.shape.rot.x, this->actor.shape.rot.y, this->actor.shape.rot.z, 2) != NULL) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_THROW);
        }
    }
}

static void EnShopnuts_Burrow(EnShopnuts* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        EnShopnuts_SetupWait(this);
    } else {
        this->collider.dim.height = ((4.0f - CLAMP_MAX(this->skelAnime.curFrame, 4.0f)) * 10.0f) + 5.0f;
    }
    if (Animation_OnFrame(&this->skelAnime, 4.0f)) {
        this->collider.base.acFlags &= ~AC_ON;
    }
}

static void EnShopnuts_SpawnSalesman(EnShopnuts* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_DNS_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                    this->actor.world.pos.z, this->actor.shape.rot.x, this->actor.shape.rot.y, this->actor.shape.rot.z,
                    this->actor.params);
        Actor_Kill(&this->actor);
    } else {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 2, 0xE38);
    }
}

static void EnShopnuts_ColliderCheck(EnShopnuts* this, PlayState* play) {
    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_SetDropFlag_Oot(&this->actor, &this->collider.elem, 1);
        EnShopnuts_SetupSpawnSalesman(this);
    } else if (play->actorCtx.unk2 != 0) {
        EnShopnuts_SetupSpawnSalesman(this);
    }
}

static void EnShopnuts_Update(Actor* thisx, PlayState* play) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    EnShopnuts_ColliderCheck(this, play);
    this->actionFunc(this, play);
    Actor_UpdateBgCheckInfo(play, &this->actor, 20.0f, this->collider.dim.radius, this->collider.dim.height, 0x404);
    if (this->collider.base.acFlags & AC_ON) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    if (this->actionFunc == EnShopnuts_Wait) {
        Actor_SetFocus(&this->actor, this->skelAnime.curFrame);
    } else if (this->actionFunc == EnShopnuts_Burrow) {
        Actor_SetFocus(&this->actor, 20.0f - ((this->skelAnime.curFrame * 20.0f) /
                                              Animation_GetLastFrame(&gBusinessScrubPeekBurrowAnim)));
    } else {
        Actor_SetFocus(&this->actor, 20.0f);
    }
}

static s32 EnShopnuts_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    if ((limbIndex == 9) && (this->actionFunc == EnShopnuts_ThrowNut)) {
        *dList = NULL;
    }
    return 0;
}

static void EnShopnuts_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    f32 curFrame;
    f32 x;
    f32 y;
    f32 z;

    if ((limbIndex == 9) && (this->actionFunc == EnShopnuts_ThrowNut)) {
        OPEN_DISPS(play->state.gfxCtx);
        curFrame = this->skelAnime.curFrame;
        if (curFrame <= 6.0f) {
            y = 1.0f - (curFrame * 0.0833f);
            x = z = (curFrame * 0.1167f) + 1.0f;
        } else if (curFrame <= 7.0f) {
            curFrame -= 6.0f;
            y = 0.5f + curFrame;
            x = z = 1.7f - (curFrame * 0.7f);
        } else if (curFrame <= 10.0f) {
            y = 1.5f - ((curFrame - 7.0f) * 0.1667f);
            x = z = 1.0f;
        } else {
            x = y = z = 1.0f;
        }

        Matrix_Scale(x, y, z, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gBusinessScrubNoseDL);
        CLOSE_DISPS(play->state.gfxCtx);
    }
}

static void EnShopnuts_Draw(Actor* thisx, PlayState* play) {
    EnShopnuts* this = (EnShopnuts*)thisx;

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnShopnuts_OverrideLimbDraw, EnShopnuts_PostLimbDraw, this);
}
