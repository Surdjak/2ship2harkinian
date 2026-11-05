/*
 * File: z_en_weiyer.c
 * Overlay: ovl_En_Weiyer
 * Description: Stinger (Water)
 */

#include "z_en_weiyer.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_ei/object_ei.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE)

static void EnWeiyer_Init(Actor* thisx, PlayState* play);
static void EnWeiyer_Destroy(Actor* thisx, PlayState* play);
static void EnWeiyer_Update(Actor* thisx, PlayState* play);
static void EnWeiyer_Draw(Actor* thisx, PlayState* play);

static void EnWeiyer_InitInsideWaterBox(EnWeiyer* this, PlayState* play);
static void EnWeiyer_FreeSwim(EnWeiyer* this, PlayState* play);
static void EnWeiyer_TurnAround(EnWeiyer* this, PlayState* play);
static void EnWeiyer_StuckOnFloor(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Attack(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Inactive(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Hurt(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Stunned(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Die(EnWeiyer* this, PlayState* play);
static void EnWeiyer_Dead(EnWeiyer* this, PlayState* play);
static void EnWeiyer_OutOfWater(EnWeiyer* this, PlayState* play);

ActorProfile En_Weiyer_Oot_Profile = {
    /**/ ACTOR_EN_WEIYER_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_EI_Oot,
    /**/ sizeof(EnWeiyer),
    /**/ EnWeiyer_Init,
    /**/ EnWeiyer_Destroy,
    /**/ EnWeiyer_Update,
    /**/ EnWeiyer_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_HIT0,
        AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_HARD,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 16, 10, -6, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sColChkInfoInit = { 2, 45, 15, 100 };

static DamageTable sDamageTable = DMG_TABLE_FROM_OOT(
    /* Deku nut      */ DMG_ENTRY(0, 0x1),
    /* Deku stick    */ DMG_ENTRY(2, 0x0),
    /* Slingshot     */ DMG_ENTRY(1, 0x0),
    /* Explosive     */ DMG_ENTRY(2, 0x0),
    /* Boomerang     */ DMG_ENTRY(0, 0x1),
    /* Normal arrow  */ DMG_ENTRY(2, 0x0),
    /* Hammer swing  */ DMG_ENTRY(2, 0x0),
    /* Hookshot      */ DMG_ENTRY(2, 0x0),
    /* Kokiri sword  */ DMG_ENTRY(1, 0x0),
    /* Master sword  */ DMG_ENTRY(2, 0x0),
    /* Giant's Knife */ DMG_ENTRY(4, 0x0),
    /* Fire arrow    */ DMG_ENTRY(2, 0x0),
    /* Ice arrow     */ DMG_ENTRY(2, 0x0),
    /* Light arrow   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 1   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 2   */ DMG_ENTRY(0, 0x0),
    /* Unk arrow 3   */ DMG_ENTRY(0, 0x0),
    /* Fire magic    */ DMG_ENTRY(0, 0x0),
    /* Ice magic     */ DMG_ENTRY(0, 0x0),
    /* Light magic   */ DMG_ENTRY(0, 0x0),
    /* Shield        */ DMG_ENTRY(0, 0x0),
    /* Mirror Ray    */ DMG_ENTRY(0, 0x0),
    /* Kokiri spin   */ DMG_ENTRY(1, 0x0),
    /* Giant spin    */ DMG_ENTRY(4, 0x0),
    /* Master spin   */ DMG_ENTRY(2, 0x0),
    /* Kokiri jump   */ DMG_ENTRY(2, 0x0),
    /* Giant jump    */ DMG_ENTRY(8, 0x0),
    /* Master jump   */ DMG_ENTRY(4, 0x0),
    /* Unknown 1     */ DMG_ENTRY(0, 0x0),
    /* Unblockable   */ DMG_ENTRY(0, 0x0),
    /* Hammer jump   */ DMG_ENTRY(4, 0x0),
    /* Unknown 2     */ DMG_ENTRY(0, 0x0)
);
static InitChainEntry sInitChain[] = {
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x19
    ICHAIN_VEC3F_DIV1000(scale, 3, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 2500, ICHAIN_STOP),
};

static void EnWeiyer_Init(Actor* thisx, PlayState* play) {
    EnWeiyer* this = (EnWeiyer*)thisx;

    Actor_ProcessInitChain(thisx, sInitChain);
    ActorShape_Init(&this->actor.shape, 1000.0f, ActorShadow_DrawCircle, 65.0f);
    SkelAnime_Init(play, &this->skelAnime, &gStingerSkel, &gStingerIdleAnim, this->jointTable, this->morphTable, 19);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo(&this->actor.colChkInfo, &sDamageTable, &sColChkInfoInit);
    this->actionFunc = EnWeiyer_InitInsideWaterBox;
}

static void EnWeiyer_Destroy(Actor* thisx, PlayState* play) {
    EnWeiyer* this = (EnWeiyer*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnWeiyer_SetupFreeSwim(EnWeiyer* this) {
    this->targetYaw = this->actor.shape.rot.y;
    this->swimHeight = (cosf(-M_PI / 8) * 3.0f) + this->actor.world.pos.y;
    Animation_MorphToLoop(&this->skelAnime, &gStingerHitAnim, -5.0f);
    this->timer = 30;
    this->actor.speed = CLAMP_MAX(this->actor.speed, 2.5f);
    this->collider.base.atFlags &= ~AT_ON;
    this->targetSwimHeight = this->actor.floorHeight;
    this->actionFunc = EnWeiyer_FreeSwim;
}

static void EnWeiyer_SetupTurnAround(EnWeiyer* this) {
    Animation_MorphToLoop(&this->skelAnime, &gStingerHitAnim, -5.0f);
    this->collider.base.atFlags |= AT_ON;
    this->timer = 0;
    this->actor.speed = 5.0f;
    this->actionFunc = EnWeiyer_TurnAround;
}

static void EnWeiyer_SetupStuckOnFloor(EnWeiyer* this) {
    Animation_Change(&this->skelAnime, &gStingerPopOutAnim, 2.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f);
    this->timer = 40;
    this->collider.base.atFlags |= AT_ON;
    this->actionFunc = EnWeiyer_StuckOnFloor;
}

static void EnWeiyer_SetupAttack(EnWeiyer* this) {
    this->timer = 200;
    this->collider.base.atFlags |= AT_ON;
    this->skelAnime.playSpeed = 3.0f;
    this->actionFunc = EnWeiyer_Attack;
}

static void EnWeiyer_SetupInactive(EnWeiyer* this) {
    this->timer = 200;
    this->targetYaw = this->actor.yawTowardsPlayer + 0x8000;
    this->swimHeight = this->actor.world.pos.y;
    this->actor.speed = CLAMP_MAX(this->actor.speed, 4.0f);
    this->collider.base.atFlags &= ~AT_ON;
    this->skelAnime.playSpeed = 1.0f;
    this->actionFunc = EnWeiyer_Inactive;
}

static void EnWeiyer_SetupHurt(EnWeiyer* this) {
    Animation_Change(&this->skelAnime, &gStingerHitAnim, 2.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -3.0f);
    this->timer = 40;
    this->collider.base.atFlags &= ~AT_ON;
    this->collider.base.acFlags &= ~AC_ON;
    this->actor.gravity = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actor.speed = 3.0f;
    Actor_SetColorFilter(&this->actor, 0x4000, 0xC8, 0, 0x28);
    this->collider.dim.height = sCylinderInit.dim.height;
    this->actionFunc = EnWeiyer_Hurt;
}

static void EnWeiyer_SetupStunned(EnWeiyer* this) {
    Animation_Change(&this->skelAnime, &gStingerPopOutAnim, 2.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f);
    this->timer = 80;
    this->actor.speed = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = -1.0f;
    this->collider.dim.height = sCylinderInit.dim.height + 15;
    Actor_SetColorFilter(&this->actor, 0, 0xC8, 0, 0x50);
    this->collider.base.atFlags &= ~AT_ON;
    Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
    this->actionFunc = EnWeiyer_Stunned;
}

static void EnWeiyer_SetupDie(EnWeiyer* this) {
    Animation_MorphToLoop(&this->skelAnime, &gStingerHitAnim, -5.0f);
    this->timer = 20;
    Actor_SetColorFilter(&this->actor, 0x4000, 0xC8, 0, 0x28);
    this->collider.base.atFlags &= ~AT_ON;
    this->collider.base.acFlags &= ~AC_ON;
    this->actor.speed = 3.0f;
    this->actionFunc = EnWeiyer_Die;
}

static void EnWeiyer_SetupDead(EnWeiyer* this) {
    this->actor.colorFilterParams |= 0x2000;
    this->actor.speed = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actionFunc = EnWeiyer_Dead;
}

static void EnWeiyer_SetupOutOfWater(EnWeiyer* this) {
    this->actor.shape.rot.x = -0x2000;
    this->timer = -1;
    this->actor.speed = 5.0f;
    this->actionFunc = EnWeiyer_OutOfWater;
}

static void EnWeiyer_InitInsideWaterBox(EnWeiyer* this, PlayState* play) {
    WaterBox* waterBox;
    s32 bgId;

    this->actor.world.pos.y += 0.5f;
    this->actor.floorHeight =
        BgCheck_EntityRaycastFloor5(&play->colCtx, &this->actor.floorPoly, &bgId, &this->actor, &this->actor.world.pos);

    if (!BgCheck_GetWaterSurfaceNoBgId(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                 &this->actor.home.pos.y, &waterBox) ||
        ((this->actor.home.pos.y - 5.0f) <= this->actor.floorHeight)) {
        Actor_Kill(&this->actor);
    } else {
        this->actor.home.pos.y -= 5.0f;
        this->actor.world.pos.y = (this->actor.home.pos.y + this->actor.floorHeight) / 2.0f;
        EnWeiyer_SetupFreeSwim(this);
    }
}

static void EnWeiyer_FreeSwim(EnWeiyer* this, PlayState* play) {
    s32 sp34;
    f32 curFrame;

    SkelAnime_Update(&this->skelAnime);
    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x800);
    sp34 = Animation_OnFrame(&this->skelAnime, 0.0f);
    curFrame = this->skelAnime.curFrame;
    Math_StepToF(&this->swimHeight, this->targetSwimHeight, 0.5f);
    this->actor.world.pos.y = this->swimHeight - cosf((curFrame - 5.0f) * (M_PI / 40)) * 3.0f;

    if (curFrame <= 45.0f) {
        Math_StepToF(&this->actor.speed, 1.0f, 0.03f);
    } else {
        Math_StepToF(&this->actor.speed, 1.3f, 0.03f);
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        this->targetYaw = this->actor.wallYaw;
        this->timer = 30;
    }

    if (Math_ScaledStepToS(&this->actor.shape.rot.y, this->targetYaw, 182)) {
        if (this->timer != 0) {
            this->timer--;
        }

        if (this->timer == 0) {
            this->targetYaw =
                Rand_S16Offset(0x2000, 0x2000) * ((Rand_ZeroOne() < 0.5f) ? -1 : 1) + this->actor.shape.rot.y;
            this->timer = 30;

            if (Rand_ZeroOne() < 0.3333f) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_B_PAMET_REVERSE);
            }
        }
    }

    if (this->actor.home.pos.y < this->actor.world.pos.y) {
        if (this->actor.home.pos.y < this->actor.floorHeight) {
            EnWeiyer_SetupTurnAround(this);
        } else {
            this->actor.world.pos.y = this->actor.home.pos.y;
            this->targetSwimHeight =
                Rand_ZeroOne() * ((this->actor.home.pos.y - this->actor.floorHeight) / 2.0f) + this->actor.floorHeight;
        }
    } else {
        Player* player = GET_PLAYER(play);

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            this->targetSwimHeight =
                this->actor.home.pos.y - Rand_ZeroOne() * ((this->actor.home.pos.y - this->actor.floorHeight) / 2.0f);
        } else if (sp34 && (Rand_ZeroOne() < 0.1f)) {
            this->targetSwimHeight =
                Rand_ZeroOne() * (this->actor.home.pos.y - this->actor.floorHeight) + this->actor.floorHeight;
        }

        if ((this->actor.xzDistToPlayer < 400.0f) && (fabsf(this->actor.playerHeightRel) < 250.0f) &&
            (player->actor.world.pos.y < (this->actor.home.pos.y + 20.0f))) {
            EnWeiyer_SetupAttack(this);
        }
    }
}

static void EnWeiyer_TurnAround(EnWeiyer* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (this->timer == 0) {
        if (Math_ScaledStepToS(&this->actor.shape.rot.x, -0x4000, 0x800)) {
            this->actor.shape.rot.z = 0;
            this->actor.shape.rot.y += 0x8000;
            this->timer = 1;
        } else {
            this->actor.shape.rot.z = this->actor.shape.rot.x * 2;
        }
    } else {
        Math_ScaledStepToS(&this->actor.shape.rot.x, 0x1800, 0x800);

        if (this->actor.world.pos.y < this->actor.home.pos.y) {
            if (this->actor.shape.rot.x > 0) {
                EffectSsGSplash_Spawn(play, &this->actor.world.pos, NULL, NULL, 1, 400);
                Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_SINK);
            }

            EnWeiyer_SetupInactive(this);
        } else if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            EnWeiyer_SetupStuckOnFloor(this);
        }
    }
}

static void EnWeiyer_StuckOnFloor(EnWeiyer* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_STEAMS_DEMO_OLD);
    }

    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x800);
    Math_StepToF(&this->actor.speed, 0.0f, 1.0f);

    if (this->timer != 0) {
        this->timer--;
    }

    if (this->timer == 0) {
        EnWeiyer_SetupTurnAround(this);
    } else if (this->actor.world.pos.y < this->actor.home.pos.y) {
        EnWeiyer_SetupFreeSwim(this);
    }
}

static s16 EnWeiyer_PitchTowardPlayer(EnWeiyer* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f vec;

    vec.x = player->actor.world.pos.x;
    vec.y = player->actor.world.pos.y + 20.0f;
    vec.z = player->actor.world.pos.z;

    return Actor_WorldPitchTowardPoint(&this->actor, &vec);
}

static void EnWeiyer_Attack(EnWeiyer* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);

    if (this->timer != 0) {
        this->timer--;
    }

    if ((this->timer == 0) || ((this->actor.home.pos.y + 20.0f) <= player->actor.world.pos.y) ||
        (this->collider.base.atFlags & AT_HIT)) {
        EnWeiyer_SetupInactive(this);
    } else {
        if (Actor_IsFacingPlayer(&this->actor, 0x2800)) {
            Math_StepToF(&this->actor.speed, 4.0f, 0.2f);
        } else {
            Math_StepToF(&this->actor.speed, 1.3f, 0.2f);
        }

        if (this->actor.home.pos.y < this->actor.world.pos.y) {
            if (this->actor.home.pos.y < this->actor.floorHeight) {
                this->actor.shape.rot.x = 0;
                EnWeiyer_SetupTurnAround(this);
                return;
            }

            this->actor.world.pos.y = this->actor.home.pos.y;
            Math_SmoothStepToS(&this->actor.shape.rot.x, 0x1000, 2, 0x100, 0x40);
        } else {
            Math_SmoothStepToS(&this->actor.shape.rot.x, EnWeiyer_PitchTowardPlayer(this, play), 2, 0x100, 0x40);
        }

        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 2, 0x200, 0x80);

        if ((player->actor.depthInWater < 50.0f) && (this->actor.depthInWater < 20.0f) &&
            Actor_IsFacingPlayer(&this->actor, 0x2000)) {
            EnWeiyer_SetupOutOfWater(this);
        }
    }
}

static void EnWeiyer_Inactive(EnWeiyer* this, PlayState* play) {
    f32 curFrame;

    SkelAnime_Update(&this->skelAnime);
    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x800);
    curFrame = this->skelAnime.curFrame;
    Math_StepToF(&this->swimHeight, (this->actor.home.pos.y - this->actor.floorHeight) / 4.0f + this->actor.floorHeight,
                 1.0f);
    this->actor.world.pos.y = this->swimHeight - cosf((curFrame - 5.0f) * (M_PI / 40)) * 3.0f;

    if (curFrame <= 45.0f) {
        Math_StepToF(&this->actor.speed, 1.0f, 0.03f);
    } else {
        Math_StepToF(&this->actor.speed, 1.3f, 0.03f);
    }

    if (this->timer != 0) {
        this->timer--;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        this->targetYaw = this->actor.wallYaw;
    }

    if (Math_SmoothStepToS(&this->actor.shape.rot.y, this->targetYaw, 2, 0x200, 0x80) == 0) {
        this->targetYaw = this->actor.yawTowardsPlayer + 0x8000;
    }

    if (this->actor.home.pos.y < this->actor.world.pos.y) {
        if (this->actor.home.pos.y < this->actor.floorHeight) {
            EnWeiyer_SetupTurnAround(this);
        } else {
            this->actor.world.pos.y = this->actor.home.pos.y;
        }
    }

    if (this->timer == 0) {
        EnWeiyer_SetupFreeSwim(this);
    }
}

static void EnWeiyer_Hurt(EnWeiyer* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (this->timer != 0) {
        this->timer--;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        this->targetYaw = this->actor.wallYaw;
    } else {
        this->targetYaw = this->actor.yawTowardsPlayer + 0x8000;
    }

    Math_ScaledStepToS(&this->actor.world.rot.y, this->targetYaw, 0x38E);
    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x200);
    this->actor.shape.rot.z = sinf(this->timer * (M_PI / 5)) * 5120.0f;

    if (this->timer == 0) {
        this->actor.shape.rot.z = 0;
        this->collider.base.acFlags |= AC_ON;
        EnWeiyer_SetupFreeSwim(this);
    }
}

static void EnWeiyer_Die(EnWeiyer* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_ScaledStepToS(&this->actor.shape.rot.x, -0x4000, 0x400);
    this->actor.shape.rot.z += 0x1000;

    if (this->timer != 0) {
        this->timer--;
    }

    if ((this->timer == 0) || (this->actor.bgCheckFlags & 0x10)) {
        EnWeiyer_SetupDead(this);
    }
}

static void EnWeiyer_Dead(EnWeiyer* this, PlayState* play) {
    this->actor.shape.shadowAlpha = CLAMP_MIN((s16)(this->actor.shape.shadowAlpha - 5), 0);
    this->actor.world.pos.y -= 2.0f;

    if (this->actor.shape.shadowAlpha == 0) {
        Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0xE0);
        Actor_Kill(&this->actor);
    }
}

static void EnWeiyer_Stunned(EnWeiyer* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }

    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x200);
    Math_ScaledStepToS(&this->actor.shape.rot.z, 0, 0x200);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.home.pos.y < this->actor.floorHeight) {
        if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_STEAMS_DEMO_OLD);
        }

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
        }
    }

    if (this->timer == 0) {
        this->actor.gravity = 0.0f;
        this->actor.velocity.y = 0.0f;
        this->collider.dim.height = sCylinderInit.dim.height;
        EnWeiyer_SetupFreeSwim(this);
    }
}

static void EnWeiyer_OutOfWater(EnWeiyer* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 phi_a1;
    s32 phi_a0;

    SkelAnime_Update(&this->skelAnime);

    phi_a0 = ((this->actor.home.pos.y + 20.0f) <= player->actor.world.pos.y);

    if (this->timer == -1) {
        if (phi_a0 || (this->collider.base.atFlags & AT_HIT)) {
            EnWeiyer_SetupInactive(this);
        } else if (this->actor.depthInWater < 0.0f) {
            this->timer = 10;
            EffectSsGSplash_Spawn(play, &this->actor.world.pos, NULL, NULL, 1, 400);
            Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_JUMP);
        }
    } else {
        if (phi_a0 || (this->collider.base.atFlags & AT_HIT)) {
            this->timer = 0;
        } else if (this->timer != 0) {
            this->timer--;
        }

        if (this->timer == 0) {
            phi_a1 = 0x1800;
        } else {
            phi_a1 = EnWeiyer_PitchTowardPlayer(this, play);
            phi_a1 = CLAMP_MIN(phi_a1, 0);
        }

        if (this->actor.shape.rot.x < phi_a1) {
            Math_ScaledStepToS(&this->actor.shape.rot.x, phi_a1, 0x400);
        }

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            EnWeiyer_SetupTurnAround(this);
        } else if ((this->actor.bgCheckFlags & 0x20) && (this->actor.shape.rot.x > 0)) {
            EffectSsGSplash_Spawn(play, &this->actor.world.pos, NULL, NULL, 1, 400);
            Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_SINK);
            EnWeiyer_SetupInactive(this);
        } else {
            Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x100, 0x80);
        }
    }
}

static void EnWeiyer_UpdateDamage(EnWeiyer* this, PlayState* play) {
    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_SetDropFlag_Oot(&this->actor, &this->collider.elem, 1);

        if ((this->actor.colChkInfo.damageEffect != 0) || (this->actor.colChkInfo.damage != 0)) {
            if (this->actor.colChkInfo.damageEffect == 1) {
                if (this->actionFunc != EnWeiyer_Stunned) {
                    EnWeiyer_SetupStunned(this);
                }
            } else if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
                Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_STEAM_LONG_OLD);
                this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
                EnWeiyer_SetupDie(this);
            } else {
                EnWeiyer_SetupHurt(this);
            }
        }
    }
}

static void EnWeiyer_Update(Actor* thisx, PlayState* play) {
    EnWeiyer* this = (EnWeiyer*)thisx;
    s32 pad;

    this->actor.home.pos.y = this->actor.depthInWater + this->actor.world.pos.y - 5.0f;
    EnWeiyer_UpdateDamage(this, play);
    this->actionFunc(this, play);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.world.rot.x = -this->actor.shape.rot.x;

    if ((this->actor.world.rot.x == 0) || (this->actionFunc == EnWeiyer_Stunned)) {
        Actor_MoveWithGravity(&this->actor);
    } else {
        Actor_MoveWithoutGravity(&this->actor);
    }

    Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 30.0f, 45.0f, 0x407);
    Actor_SetFocus(&this->actor, 0.0f);

    if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.atFlags &= ~(AT_ON | AT_HIT);
        Actor_PlaySfx(&this->actor, NA_SE_EN_B_PAMET_VOICE);
    }

    Collider_UpdateCylinder(&this->actor, &this->collider);

    if (this->collider.base.atFlags & AT_ON) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    }

    if (this->collider.base.acFlags & AT_ON) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }

    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static s32 EnWeiyer_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx,
                              Gfx** gfx) {
    if (limbIndex == 1) {
        pos->z += 2000.0f;
    }

    return 0;
}

static void EnWeiyer_Draw(Actor* thisx, PlayState* play) {
    EnWeiyer* this = (EnWeiyer*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->actionFunc != EnWeiyer_Dead) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x08, &D_80116280_Oot[2]);
        gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
        POLY_OPA_DISP = SkelAnime_DrawSkeleton2Oot(play, &this->skelAnime, EnWeiyer_OverrideLimbDraw, NULL, &this->actor,
                                                POLY_OPA_DISP);
    } else {
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPSegment(POLY_XLU_DISP++, 0x08, &D_80116280_Oot[0]);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, this->actor.shape.shadowAlpha);
        POLY_XLU_DISP = SkelAnime_DrawSkeleton2Oot(play, &this->skelAnime, EnWeiyer_OverrideLimbDraw, NULL, &this->actor,
                                                POLY_XLU_DISP);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
