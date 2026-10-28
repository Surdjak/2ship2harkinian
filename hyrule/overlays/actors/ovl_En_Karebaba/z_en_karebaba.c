/**
 * File: z_en_karebaba.c
 * Overlay: ovl_En_Karebaba
 * Description: Withered Deku Baba
 */

#include "z_en_karebaba.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_dekubaba/object_dekubaba.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE)

static void EnKarebaba_Init(Actor* thisx, PlayState* play);
static void EnKarebaba_Destroy(Actor* thisx, PlayState* play);
static void EnKarebaba_Update(Actor* thisx, PlayState* play);
static void EnKarebaba_Draw(Actor* thisx, PlayState* play);

static void EnKarebaba_SetupGrow(EnKarebaba* this);
static void EnKarebaba_SetupIdle(EnKarebaba* this);
static void EnKarebaba_Grow(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Idle(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Awaken(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Spin(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Dying(EnKarebaba* this, PlayState* play);
static void EnKarebaba_DeadItemDrop(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Retract(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Dead(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Regrow(EnKarebaba* this, PlayState* play);
static void EnKarebaba_Upright(EnKarebaba* this, PlayState* play);

ActorProfile En_Karebaba_Oot_Profile = {
    /**/ ACTOR_EN_KAREBABA_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_DEKUBABA_Oot,
    /**/ sizeof(EnKarebaba),
    /**/ EnKarebaba_Init,
    /**/ EnKarebaba_Destroy,
    /**/ EnKarebaba_Update,
    /**/ EnKarebaba_Draw,
};

static ColliderCylinderInit sBodyColliderInit = {
    {
        COL_MATERIAL_HARD,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_NONE,
    },
    { 7, 25, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sHeadColliderInit = {
    {
        COL_MATERIAL_HARD,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_HARD,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 4, 25, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sColCheckInfoInit = { 1, 15, 80, MASS_HEAVY };

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(lockOnArrowOffset, 2500, ICHAIN_CONTINUE),
    ICHAIN_U8(attentionRangeType, 1, ICHAIN_CONTINUE),
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_STOP), // PORT: OoT's Navi enemy info 0x09
};

static void EnKarebaba_Init(Actor* thisx, PlayState* play) {
    EnKarebaba* this = (EnKarebaba*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 22.0f);
    SkelAnime_Init(play, &this->skelAnime, &gDekuBabaSkel, &gDekuBabaFastChompAnim, this->jointTable, this->morphTable,
                   8);
    Collider_InitCylinder(play, &this->bodyCollider);
    Collider_SetCylinder(play, &this->bodyCollider, &this->actor, &sBodyColliderInit);
    Collider_UpdateCylinder(&this->actor, &this->bodyCollider);
    Collider_InitCylinder(play, &this->headCollider);
    Collider_SetCylinder(play, &this->headCollider, &this->actor, &sHeadColliderInit);
    Collider_UpdateCylinder(&this->actor, &this->headCollider);
    CollisionCheck_SetInfo(&this->actor.colChkInfo, DamageTable_Get_Oot(1), &sColCheckInfoInit);

    this->boundFloor = NULL;

    if (this->actor.params == 0) {
        EnKarebaba_SetupGrow(this);
    } else {
        EnKarebaba_SetupIdle(this);
    }
}

static void EnKarebaba_Destroy(Actor* thisx, PlayState* play) {
    EnKarebaba* this = (EnKarebaba*)thisx;

    Collider_DestroyCylinder(play, &this->bodyCollider);
    Collider_DestroyCylinder(play, &this->headCollider);
}

static void EnKarebaba_ResetCollider(EnKarebaba* this) {
    this->bodyCollider.dim.radius = 7;
    this->bodyCollider.dim.height = 25;
    this->bodyCollider.base.colMaterial = COL_MATERIAL_HARD;
    this->bodyCollider.base.acFlags |= AC_HARD;
    this->bodyCollider.elem.acDmgInfo.dmgFlags = ~DMG_FROM_OOT(0x00300000);
    this->headCollider.dim.height = 25;
}

static void EnKarebaba_SetupGrow(EnKarebaba* this) {
    Actor_SetScale(&this->actor, 0.0f);
    this->actor.shape.rot.x = -0x4000;
    this->actionFunc = EnKarebaba_Grow;
    this->actor.world.pos.y = this->actor.home.pos.y + 14.0f;
}

static void EnKarebaba_SetupIdle(EnKarebaba* this) {
    Actor_SetScale(&this->actor, 0.005f);
    this->actor.shape.rot.x = -0x4000;
    this->actionFunc = EnKarebaba_Idle;
    this->actor.world.pos.y = this->actor.home.pos.y + 14.0f;
}

static void EnKarebaba_SetupAwaken(EnKarebaba* this) {
    Animation_Change(&this->skelAnime, &gDekuBabaFastChompAnim, 4.0f, 0.0f,
                     Animation_GetLastFrame(&gDekuBabaFastChompAnim), ANIMMODE_LOOP, -3.0f);
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEKU_WAKEUP);
    this->actionFunc = EnKarebaba_Awaken;
}

static void EnKarebaba_SetupUpright(EnKarebaba* this) {
    if (this->actionFunc != EnKarebaba_Spin) {
        Actor_SetScale(&this->actor, 0.01f);
        this->bodyCollider.base.colMaterial = COL_MATERIAL_HIT6;
        this->bodyCollider.base.acFlags &= ~AC_HARD;
        this->bodyCollider.elem.acDmgInfo.dmgFlags =
            !LINK_IS_ADULT_OOT ? DMG_FROM_OOT(0x07C00710) : DMG_FROM_OOT(0x0FC00710);
        this->bodyCollider.dim.radius = 15;
        this->bodyCollider.dim.height = 80;
        this->headCollider.dim.height = 80;
    }

    this->actor.params = 40;
    this->actionFunc = EnKarebaba_Upright;
}

static void EnKarebaba_SetupSpin(EnKarebaba* this) {
    this->actor.params = 40;
    this->actionFunc = EnKarebaba_Spin;
}

static void EnKarebaba_SetupDying(EnKarebaba* this) {
    this->actor.params = 0;
    this->actor.gravity = -0.8f;
    this->actor.velocity.y = 4.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y + 0x8000;
    this->actor.speed = 3.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEKU_JR_DEAD);
    this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
    this->actionFunc = EnKarebaba_Dying;
}

static void EnKarebaba_SetupDeadItemDrop(EnKarebaba* this, PlayState* play) {
    Actor_SetScale(&this->actor, 0.03f);
    this->actor.shape.rot.x -= 0x4000;
    this->actor.shape.yOffset = 1000.0f;
    this->actor.gravity = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actor.shape.shadowScale = 3.0f;
    Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_MISC);
    this->actor.params = 200;
    this->actor.flags &= ~ACTOR_FLAG_DRAW_CULLING_DISABLED;
    this->actionFunc = EnKarebaba_DeadItemDrop;
}

static void EnKarebaba_SetupRetract(EnKarebaba* this) {
    Animation_Change(&this->skelAnime, &gDekuBabaFastChompAnim, -3.0f, Animation_GetLastFrame(&gDekuBabaFastChompAnim),
                     0.0f, ANIMMODE_ONCE, -3.0f);
    EnKarebaba_ResetCollider(this);
    this->actionFunc = EnKarebaba_Retract;
}

static void EnKarebaba_SetupDead(EnKarebaba* this) {
    Animation_Change(&this->skelAnime, &gDekuBabaFastChompAnim, 0.0f, 0.0f, 0.0f, ANIMMODE_ONCE, 0.0f);
    EnKarebaba_ResetCollider(this);
    this->actor.shape.rot.x = -0x4000;
    this->actor.params = 200;
    this->actor.parent = NULL;
    this->actor.shape.shadowScale = 0.0f;
    Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.home.pos);
    this->actionFunc = EnKarebaba_Dead;
}

static void EnKarebaba_SetupRegrow(EnKarebaba* this) {
    this->actor.shape.yOffset = 0.0f;
    this->actor.shape.shadowScale = 22.0f;
    this->headCollider.dim.radius = sHeadColliderInit.dim.radius;
    Actor_SetScale(&this->actor, 0.0f);
    this->actionFunc = EnKarebaba_Regrow;
}

static void EnKarebaba_Grow(EnKarebaba* this, PlayState* play) {
    f32 scale;

    this->actor.params++;
    scale = this->actor.params * 0.05f;
    Actor_SetScale(&this->actor, 0.005f * scale);
    this->actor.world.pos.y = this->actor.home.pos.y + (14.0f * scale);
    if (this->actor.params == 20) {
        EnKarebaba_SetupIdle(this);
    }
}

static void EnKarebaba_Idle(EnKarebaba* this, PlayState* play) {
    if (this->actor.xzDistToPlayer < 200.0f && fabsf(this->actor.playerHeightRel) < 30.0f) {
        EnKarebaba_SetupAwaken(this);
    }
}

static void EnKarebaba_Awaken(EnKarebaba* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_StepToF(&this->actor.scale.x, 0.01f, 0.0005f);
    this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;
    if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 60.0f, 5.0f)) {
        EnKarebaba_SetupUpright(this);
    }
    this->actor.shape.rot.y += 0x1999;
    EffectSsHahen_SpawnBurst(play, &this->actor.home.pos, 3.0f, 0, 12, 5, 1, HAHEN_OBJECT_DEFAULT, 10, NULL);
}

static void EnKarebaba_Upright(EnKarebaba* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);

    if (this->actor.params != 0) {
        this->actor.params--;
    }

    if (Animation_OnFrame(&this->skelAnime, 0.0f) || Animation_OnFrame(&this->skelAnime, 12.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIZUBABA1_MOUTH);
    }

    if (this->bodyCollider.base.acFlags & AC_HIT) {
        EnKarebaba_SetupDying(this);
        Enemy_StartFinishingBlow(play, &this->actor);
    } else if (Math_Vec3f_DistXZ(&this->actor.home.pos, &player->actor.world.pos) > 240.0f) {
        EnKarebaba_SetupRetract(this);
    } else if (this->actor.params == 0) {
        EnKarebaba_SetupSpin(this);
    }
}

static void EnKarebaba_Spin(EnKarebaba* this, PlayState* play) {
    s32 value;
    f32 cos60;

    if (this->actor.params != 0) {
        this->actor.params--;
    }

    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 0.0f) || Animation_OnFrame(&this->skelAnime, 12.0f)) {

        Actor_PlaySfx(&this->actor, NA_SE_EN_MIZUBABA1_MOUTH);
    }

    value = 20 - this->actor.params;
    value = 20 - ABS(value);

    if (value > 10) {
        value = 10;
    }

    this->headCollider.dim.radius = sHeadColliderInit.dim.radius + (value * 2);
    this->actor.shape.rot.x = 0xC000 - (value * 0x100);
    this->actor.shape.rot.y += value * 0x2C0;
    this->actor.world.pos.y = (Math_SinS(this->actor.shape.rot.x) * -60.0f) + this->actor.home.pos.y;

    cos60 = Math_CosS(this->actor.shape.rot.x) * 60.0f;

    this->actor.world.pos.x = (Math_SinS(this->actor.shape.rot.y) * cos60) + this->actor.home.pos.x;
    this->actor.world.pos.z = (Math_CosS(this->actor.shape.rot.y) * cos60) + this->actor.home.pos.z;

    if (this->bodyCollider.base.acFlags & AC_HIT) {
        EnKarebaba_SetupDying(this);
        Enemy_StartFinishingBlow(play, &this->actor);
    } else if (this->actor.params == 0) {
        EnKarebaba_SetupUpright(this);
    }
}

static void EnKarebaba_Dying(EnKarebaba* this, PlayState* play) {
    static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    s32 i;
    Vec3f position;
    Vec3f rotation;

    Math_StepToF(&this->actor.speed, 0.0f, 0.1f);

    if (this->actor.params == 0) {
        Math_ScaledStepToS(&this->actor.shape.rot.x, 0x4800, 0x71C);
        EffectSsHahen_SpawnBurst(play, &this->actor.world.pos, 3.0f, 0, 12, 5, 1, HAHEN_OBJECT_DEFAULT, 10, NULL);

        if (this->actor.scale.x > 0.005f &&
            ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) || (this->actor.bgCheckFlags & BGCHECKFLAG_WALL))) {
            this->actor.scale.x = this->actor.scale.y = this->actor.scale.z = 0.0f;
            this->actor.speed = 0.0f;
            this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE);
            EffectSsHahen_SpawnBurst(play, &this->actor.world.pos, 3.0f, 0, 12, 5, 15, HAHEN_OBJECT_DEFAULT, 10, NULL);
        }

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
            this->actor.params = 1;
        }
    } else if (this->actor.params == 1) {
        Math_Vec3f_Copy(&position, &this->actor.world.pos);
        rotation.z = Math_SinS(this->actor.shape.rot.x) * 20.0f;
        rotation.x = -20.0f * Math_CosS(this->actor.shape.rot.x) * Math_SinS(this->actor.shape.rot.y);
        rotation.y = -20.0f * Math_CosS(this->actor.shape.rot.x) * Math_CosS(this->actor.shape.rot.y);

        for (i = 0; i < 4; i++) {
            func_800286CC(play, &position, &zeroVec, &zeroVec, 500, 50);
            position.x += rotation.x;
            position.y += rotation.z;
            position.z += rotation.y;
        }

        func_800286CC(play, &this->actor.home.pos, &zeroVec, &zeroVec, 500, 100);
        EnKarebaba_SetupDeadItemDrop(this, play);
    }
}

static void EnKarebaba_DeadItemDrop(EnKarebaba* this, PlayState* play) {
    if (this->actor.params != 0) {
        this->actor.params--;
    }

    if (Actor_HasParent(&this->actor, play) || this->actor.params == 0) {
        EnKarebaba_SetupDead(this);
    } else {
        Actor_OfferGetItemNearby(&this->actor, play, GI_STICKS_1_Oot);
    }
}

static void EnKarebaba_Retract(EnKarebaba* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_StepToF(&this->actor.scale.x, 0.005f, 0.0005f);
    this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;

    if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 14.0f, 5.0f)) {
        EnKarebaba_SetupIdle(this);
    }

    this->actor.shape.rot.y += 0x1999;
    EffectSsHahen_SpawnBurst(play, &this->actor.home.pos, 3.0f, 0, 12, 5, 1, HAHEN_OBJECT_DEFAULT, 10, NULL);
}

static void EnKarebaba_Dead(EnKarebaba* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.params != 0) {
        this->actor.params--;
    }
    if (this->actor.params == 0) {
        EnKarebaba_SetupRegrow(this);
    }
}

static void EnKarebaba_Regrow(EnKarebaba* this, PlayState* play) {
    f32 scaleFactor;

    this->actor.params++;
    scaleFactor = this->actor.params * 0.05f;
    Actor_SetScale(&this->actor, 0.005f * scaleFactor);
    this->actor.world.pos.y = this->actor.home.pos.y + (14.0f * scaleFactor);

    if (this->actor.params == 20) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE;
        Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_ENEMY);
        EnKarebaba_SetupIdle(this);
    }
}

static void EnKarebaba_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnKarebaba* this = (EnKarebaba*)thisx;
    f32 height;

    this->actionFunc(this, play);

    if (this->actionFunc != EnKarebaba_Dead) {
        if (this->actionFunc == EnKarebaba_Dying) {
            Actor_MoveWithGravity(&this->actor);
            Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 15.0f, 10.0f, 0x405);
        } else {
            Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
            if (this->boundFloor == NULL) {
                this->boundFloor = this->actor.floorPoly;
            }
        }
        if (this->actionFunc != EnKarebaba_Dying && this->actionFunc != EnKarebaba_DeadItemDrop) {
            if (this->actionFunc != EnKarebaba_Regrow && this->actionFunc != EnKarebaba_Grow) {
                CollisionCheck_SetAT(play, &play->colChkCtx, &this->headCollider.base);
                CollisionCheck_SetAC(play, &play->colChkCtx, &this->bodyCollider.base);
            }
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->headCollider.base);
            Actor_SetFocus(&this->actor, (this->actor.scale.x * 10.0f) / 0.01f);
            height = this->actor.home.pos.y + 40.0f;
            this->actor.focus.pos.x = this->actor.home.pos.x;
            this->actor.focus.pos.y = CLAMP_MAX(this->actor.focus.pos.y, height);
            this->actor.focus.pos.z = this->actor.home.pos.z;
        }
    }
}

static void EnKarebaba_DrawBaseShadow(EnKarebaba* this, PlayState* play) {
    MtxF mf;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL44_Xlu(play->state.gfxCtx);

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0, 0, 0, 255);
    func_800C0094(this->boundFloor, this->actor.home.pos.x, this->actor.home.pos.y, this->actor.home.pos.z, &mf);
    Matrix_Mult(&mf, MTXMODE_NEW);
    Matrix_Scale(0.15f, 1.0f, 0.15f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gCircleShadowDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnKarebaba_Draw(Actor* thisx, PlayState* play) {
    static Color_RGBA8 black = { 0, 0, 0, 0 };
    static Gfx* stemDLists[] = { gDekuBabaStemTopDL, gDekuBabaStemMiddleDL, gDekuBabaStemBaseDL };
    static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    EnKarebaba* this = (EnKarebaba*)thisx;
    s32 i;
    s32 stemSections;
    f32 scale;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    if (this->actionFunc == EnKarebaba_DeadItemDrop) {
        if (this->actor.params > 40 || (this->actor.params & 1)) {
            Matrix_Translate(0.0f, 0.0f, 200.0f, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, gDekuBabaStickDropDL);
        }
    } else if (this->actionFunc != EnKarebaba_Dead) {
        func_800AE2A0(play, &black, 1, 2);
        SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, NULL);
        Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);

        if ((this->actionFunc == EnKarebaba_Regrow) || (this->actionFunc == EnKarebaba_Grow)) {
            scale = this->actor.params * 0.0005f;
        } else {
            scale = 0.01f;
        }

        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
        Matrix_RotateZYX(this->actor.shape.rot.x, this->actor.shape.rot.y, 0, MTXMODE_APPLY);

        if (this->actionFunc == EnKarebaba_Dying) {
            stemSections = 2;
        } else {
            stemSections = 3;
        }

        for (i = 0; i < stemSections; i++) {
            Matrix_Translate(0.0f, 0.0f, -2000.0f, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, stemDLists[i]);

            if (i == 0 && this->actionFunc == EnKarebaba_Dying) {
                Matrix_MultVec3f(&zeroVec, &this->actor.focus.pos);
            }
        }

        func_800AE5A0(play);
    }

    func_800AE2A0(play, &black, 1, 2);
    Matrix_Translate(this->actor.home.pos.x, this->actor.home.pos.y, this->actor.home.pos.z, MTXMODE_NEW);

    if (this->actionFunc != EnKarebaba_Grow) {
        scale = 0.01f;
    }

    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    Matrix_RotateYF(this->actor.home.rot.y * (M_PI / 0x8000), MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gDekuBabaBaseLeavesDL);

    if (this->actionFunc == EnKarebaba_Dying) {
        Matrix_RotateZYX(-0x4000, (s16)(this->actor.shape.rot.y - this->actor.home.rot.y), 0, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gDekuBabaStemBaseDL);
    }

    func_800AE5A0(play);

    CLOSE_DISPS(play->state.gfxCtx);

    if (this->boundFloor != NULL) {
        EnKarebaba_DrawBaseShadow(this, play);
    }
}
