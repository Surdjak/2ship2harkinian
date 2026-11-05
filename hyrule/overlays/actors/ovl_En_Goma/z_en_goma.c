#include "z_en_goma.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_dangeon_keep/gameplay_dangeon_keep.h"
#include "assets/oot/objects/object_gol/object_gol.h"
#include "../ovl_Boss_Goma/z_boss_goma.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

#define FLAGS                                                                                 \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EnGoma_Init(Actor* thisx, PlayState* play);
static void EnGoma_Destroy(Actor* thisx, PlayState* play);
static void EnGoma_Update(Actor* thisx, PlayState* play);
static void EnGoma_Draw(Actor* thisx, PlayState* play);
static void EnGoma_Reset(void);

static void EnGoma_Flee(EnGoma* this, PlayState* play);
static void EnGoma_EggFallToGround(EnGoma* this, PlayState* play);
static void EnGoma_Egg(EnGoma* this, PlayState* play);
static void EnGoma_Hatch(EnGoma* this, PlayState* play);
static void EnGoma_Hurt(EnGoma* this, PlayState* play);
static void EnGoma_Die(EnGoma* this, PlayState* play);
static void EnGoma_Dead(EnGoma* this, PlayState* play);
static void EnGoma_PrepareJump(EnGoma* this, PlayState* play);
static void EnGoma_Land(EnGoma* this, PlayState* play);
static void EnGoma_Jump(EnGoma* this, PlayState* play);
static void EnGoma_Stand(EnGoma* this, PlayState* play);
static void EnGoma_ChasePlayer(EnGoma* this, PlayState* play);
static void EnGoma_Stunned(EnGoma* this, PlayState* play);
static void EnGoma_LookAtPlayer(EnGoma* this, PlayState* play);
static void EnGoma_UpdateHit(EnGoma* this, PlayState* play);
static void EnGoma_Debris(EnGoma* this, PlayState* play);
static void EnGoma_SpawnHatchDebris(EnGoma* this, PlayState* play2);
static void EnGoma_BossLimb(EnGoma* this, PlayState* play);

static void EnGoma_SetupFlee(EnGoma* this);
static void EnGoma_SetupHatch(EnGoma* this, PlayState* play);
static void EnGoma_SetupHurt(EnGoma* this, PlayState* play);
static void EnGoma_SetupDie(EnGoma* this);
static void EnGoma_SetupDead(EnGoma* this);
static void EnGoma_SetupStand(EnGoma* this);
static void EnGoma_SetupChasePlayer(EnGoma* this);
static void EnGoma_SetupPrepareJump(EnGoma* this);
static void EnGoma_SetupLand(EnGoma* this);
static void EnGoma_SetupJump(EnGoma* this);
static void EnGoma_SetupStunned(EnGoma* this, PlayState* play);

ActorProfile En_Goma_Oot_Profile = {
    /**/ ACTOR_EN_GOMA_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_GOL_Oot,
    /**/ sizeof(EnGoma),
    /**/ EnGoma_Init,
    /**/ EnGoma_Destroy,
    /**/ EnGoma_Update,
    /**/ EnGoma_Draw,
    /**/ EnGoma_Reset,
};

static ColliderCylinderInit D_80A4B7A0 = {
    {
        COL_MATERIAL_HIT3,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { DMG_FROM_OOT(0xFFDFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 15, 30, 10, { 0, 0, 0 } },
};

static ColliderCylinderInit D_80A4B7CC = {
    {
        COL_MATERIAL_HIT3,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { DMG_FROM_OOT(0xFFDFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_NONE,
    },
    { 15, 30, 10, { 0, 0, 0 } },
};

static u8 sSpawnNum = 0;
static Vec3f sDeadEffectVel = { 0.0f, 0.0f, 0.0f };

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 3, ICHAIN_CONTINUE),
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x03
    ICHAIN_F32_DIV1000(gravity, 0, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 20, ICHAIN_STOP),
};

static void EnGoma_Init(Actor* thisx, PlayState* play) {
    EnGoma* this = (EnGoma*)thisx;
    s16 params;

    this->eggTimer = Rand_ZeroOne() * 200.0f;
    Actor_ProcessInitChain(&this->actor, sInitChain);
    Actor_SetScale(&this->actor, 0.01f);
    params = this->actor.params;

    if (params >= 100) { // piece of boss goma
        Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_BOSS);
        this->actionFunc = EnGoma_BossLimb;
        this->gomaType = ENGOMA_BOSSLIMB;
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 0.0f);
        this->actionTimer = this->actor.params + 150;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    } else if (params >= 10) { // Debris when hatching
        this->actor.gravity = -1.3f;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        this->actionTimer = 50;
        this->gomaType = ENGOMA_HATCH_DEBRIS;
        this->eggScale = 1.0f;
        this->actor.velocity.y = Rand_ZeroOne() * 5.0f + 5.0f;
        this->actionFunc = EnGoma_Debris;
        this->actor.speed = Rand_ZeroOne() * 2.3f + 1.5f;
        this->actionTimer = 30;
        this->actor.scale.x = Rand_ZeroOne() * 0.005f + 0.01f;
        this->actor.scale.y = Rand_ZeroOne() * 0.005f + 0.01f;
        this->actor.scale.z = Rand_ZeroOne() * 0.005f + 0.01f;
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 0.0f);
    } else { // Egg
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 40.0f);
        SkelAnime_Init(play, &this->skelanime, &gObjectGolSkel, &gObjectGolStandAnim, this->jointTable,
                       this->morphTable, GOMA_LIMB_MAX);
        Animation_PlayLoop(&this->skelanime, &gObjectGolStandAnim);
        this->actor.colChkInfo.health = 2;

        if (this->actor.params < 3) { // Spawned by boss
            this->actionFunc = EnGoma_EggFallToGround;
            this->invincibilityTimer = 10;
            this->actor.speed = 1.5f;
        } else if (this->actor.params == 8 || this->actor.params == 6) {
            this->actionFunc = EnGoma_Egg;
            this->spawnNum = sSpawnNum++;
        } else if (this->actor.params == 9 || this->actor.params == 7) {
            this->actionFunc = EnGoma_Egg;
        }

        if (this->actor.params >= 8) { // on ceiling
            this->eggYOffset = -1500.0f;
        } else {
            this->eggYOffset = 1500.0f;
        }

        this->gomaType = ENGOMA_EGG;
        this->eggScale = 1.0f;
        this->eggSquishAngle = Rand_ZeroOne() * 1000.0f;
        this->actionTimer = 50;
        Collider_InitCylinder(play, &this->colCyl1);
        Collider_SetCylinder(play, &this->colCyl1, &this->actor, &D_80A4B7A0);
        Collider_InitCylinder(play, &this->colCyl2);
        Collider_SetCylinder(play, &this->colCyl2, &this->actor, &D_80A4B7CC);
    }
}

static void EnGoma_Destroy(Actor* thisx, PlayState* play) {
    EnGoma* this = (EnGoma*)thisx;

    if (this->actor.params < 10) {
        Collider_DestroyCylinder(play, &this->colCyl1);
        Collider_DestroyCylinder(play, &this->colCyl2);
    }
}

static void EnGoma_SetupFlee(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolRunningAnim, 2.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolRunningAnim), ANIMMODE_LOOP, -2.0f);
    this->actionFunc = EnGoma_Flee;
    this->actionTimer = 20;

    if (this->actor.params < 6) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_ROAR_OLD);
    } else {
        Actor_PlaySfx(&this->actor, NA_SE_EN_FROG_JUMP_MID);
    }
}

static void EnGoma_Flee(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);
    Math_ApproachF(&this->actor.speed, 20.0f / 3.0f, 0.5f, 2.0f);
    Math_ApproachS(&this->actor.world.rot.y, Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor) + 0x8000,
                   3, 2000);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 2, 3000);

    if (this->actionTimer == 0) {
        EnGoma_SetupStand(this);
    }
}

static void EnGoma_EggFallToGround(EnGoma* this, PlayState* play) {
    this->actor.gravity = -1.3f;
    this->eggSquishAccel += 0.03f;
    this->eggSquishAngle += 1.0f + this->eggSquishAccel;
    Math_ApproachZeroF(&this->eggSquishAmount, 1.0f, 0.005f);
    Math_ApproachF(&this->eggYOffset, 1500.0f, 1.0f, 150.0f);

    switch (this->hatchState) {
        case 0:
            if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) { // floor
                if (this->actor.params < 6) {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_JUMP2);
                } else {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_FROG_PUNCH1);
                }

                if (this->actor.params > 5) {
                    EnGoma_SetupHatch(this, play);
                } else {
                    this->hatchState = 1;
                    this->actionTimer = 3;
                    Math_ApproachF(&this->eggScale, 1.5f, 0.5f, 1.0f);
                }
            }
            break;

        case 1:
            if (this->actionTimer == 0) {
                this->hatchState = 2;
                this->actionTimer = 3;
                Math_ApproachF(&this->eggScale, 0.75f, 0.5f, 1.0f);
                this->actor.velocity.y = 5.0f;
                this->actor.speed = 2.0f;
            } else {
                Math_ApproachF(&this->eggScale, 1.5f, 0.5f, 1.0f);
            }
            break;

        case 2:
            if (this->actionTimer == 0) {
                this->hatchState = 3;
                this->actionTimer = 80;
            } else {
                Math_ApproachF(&this->eggScale, 0.75f, 0.5f, 1.0f);
            }
            break;

        case 3:
            Math_ApproachF(&this->eggScale, 1.0f, 0.1f, 0.1f);
            if (this->actionTimer == 0) {
                EnGoma_SetupHatch(this, play);
            }
            break;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        Math_ApproachZeroF(&this->actor.speed, 0.2f, 0.05f);
    }
    this->eggPitch += (this->actor.speed * 0.1f);
    this->actor.shape.rot.y = this->actor.world.rot.y;
}

static void EnGoma_Egg(EnGoma* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 i;

    this->eggSquishAngle += 1.0f;
    Math_ApproachF(&this->eggSquishAmount, 0.1f, 1.0f, 0.005f);
    if (fabsf(this->actor.world.pos.x - player->actor.world.pos.x) < 100.0f &&
        fabsf(this->actor.world.pos.z - player->actor.world.pos.z) < 100.0f) {
        if (++this->playerDetectionTimer > 9) {
            this->actionFunc = EnGoma_EggFallToGround;
        }
    } else {
        this->playerDetectionTimer = 0;
    }

    if (!(this->eggTimer & 0xF) && Rand_ZeroOne() < 0.5f) {
        for (i = 0; i < 2; i++) {
            Vec3f vel = { 0.0f, 0.0f, 0.0f };
            Vec3f acc = { 0.0f, -0.5f, 0.0f };
            Vec3f pos;

            pos.x = Rand_CenteredFloat(30.0f) + this->actor.world.pos.x;
            pos.y = Rand_ZeroFloat(30.0f) + this->actor.world.pos.y;
            pos.z = Rand_CenteredFloat(30.0f) + this->actor.world.pos.z;
            EffectSsHahen_Spawn(play, &pos, &vel, &acc, 0, (s16)(Rand_ZeroOne() * 5.0f) + 10, HAHEN_OBJECT_DEFAULT, 10,
                                NULL);
        }
    }
}

static void EnGoma_SetupHatch(EnGoma* this, PlayState* play) {
    Animation_Change(&this->skelanime, &gObjectGolJumpHeadbuttAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolJumpHeadbuttAnim), ANIMMODE_ONCE, 0.0f);
    this->actionFunc = EnGoma_Hatch;
    Actor_SetScale(&this->actor, 0.005f);
    this->gomaType = ENGOMA_NORMAL;
    this->actionTimer = 5;
    this->actor.shape.rot.y = Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnGoma_SpawnHatchDebris(this, play);
    this->eggScale = 1.0f;
    this->actor.speed = 0.0f;
}

static void EnGoma_Hatch(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);
    if (this->actionTimer == 0) {
        EnGoma_SetupStand(this);
    }
}

static void EnGoma_SetupHurt(EnGoma* this, PlayState* play) {
    Animation_Change(&this->skelanime, &gObjectGolDamagedAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolDamagedAnim), ANIMMODE_ONCE, -2.0f);
    this->actionFunc = EnGoma_Hurt;

    if ((s8)this->actor.colChkInfo.health <= 0) {
        this->actionTimer = 5;
        Enemy_StartFinishingBlow(play, &this->actor);
    } else {
        this->actionTimer = 10;
    }

    this->actor.speed = 20.0f;
    this->actor.world.rot.y = this->actor.yawTowardsPlayer + 0x8000;
    if (this->actor.params < 6) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_SAND_OLD);
    } else {
        Actor_PlaySfx(&this->actor, NA_SE_EN_FROG_THROW_SLIME);
    }
}

static void EnGoma_Hurt(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        Math_ApproachZeroF(&this->actor.speed, 1.0f, 2.0f);
    }

    if (this->actionTimer == 0) {
        if ((s8)this->actor.colChkInfo.health <= 0) {
            EnGoma_SetupDie(this);
        } else {
            EnGoma_SetupFlee(this);
        }
    }
}

static void EnGoma_SetupDie(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolDeathAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gObjectGolDeathAnim),
                     ANIMMODE_ONCE, -2.0f);
    this->actionFunc = EnGoma_Die;
    this->actionTimer = 30;

    if (this->actor.params < 6) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_DAMAGE_OLD);
    } else {
        Actor_PlaySfx(&this->actor, NA_SE_EN_KONB_BITE_OLD);
    }

    this->invincibilityTimer = 100;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
}

static void EnGoma_Die(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        Math_ApproachZeroF(&this->actor.speed, 1.0f, 2.0f);
    }

    if (this->actionTimer == 17) {
        if (this->actor.params < 6) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_LAST2_WALK2_OLD);
        } else {
            Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_ATTACK_2ND_OLD);
        }
    }

    if (this->actionTimer == 0) {
        EnGoma_SetupDead(this);
    }
}

static void EnGoma_SetupDead(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolDeadTwitchingAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolDeadTwitchingAnim), ANIMMODE_LOOP, -2.0f);
    this->actionFunc = EnGoma_Dead;
    this->actionTimer = 3;
}

static void EnGoma_Dead(EnGoma* this, PlayState* play) {
    Vec3f accel;
    Vec3f pos;

    SkelAnime_Update(&this->skelanime);
    Math_ApproachZeroF(&this->actor.speed, 1.0f, 2.0f);

    if (this->actionTimer == 2) {
        pos.x = this->actor.world.pos.x;
        pos.y = (this->actor.world.pos.y + 5.0f) - 10.0f;
        pos.z = this->actor.world.pos.z;
        accel = sDeadEffectVel;
        accel.y = 0.03f;
        EffectSsKFire_Spawn(play, &pos, &sDeadEffectVel, &accel, 40, 0);
    }

    if (this->actionTimer == 0 && Math_SmoothStepToF(&this->actor.scale.y, 0.0f, 0.5f, 0.00225f, 0.00001f) <= 0.001f) {
        if (this->actor.params < 6) {
            BossGoma* parent = (BossGoma*)this->actor.parent;

            parent->childrenGohmaState[this->actor.params] = -1;
        }
        Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_EN_EXTINCT);
        Actor_Kill(&this->actor);
        Item_DropCollectibleRandom(play, NULL, &this->actor.world.pos, 0x30);
    }
    this->visualState = 2;
}

static void EnGoma_SetupStand(EnGoma* this) {
    f32 lastFrame;

    lastFrame = Animation_GetLastFrame(&gObjectGolStandAnim);
    this->actionTimer = Rand_S16Offset(10, 30);
    Animation_Change(&this->skelanime, &gObjectGolStandAnim, 1.0f, 0.0f, lastFrame, ANIMMODE_LOOP, -5.0f);
    this->actionFunc = EnGoma_Stand;
    this->gomaType = ENGOMA_NORMAL;
}

static void EnGoma_SetupChasePlayer(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolRunningAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolRunningAnim), ANIMMODE_LOOP, -5.0f);
    this->actionFunc = EnGoma_ChasePlayer;
    this->actionTimer = Rand_S16Offset(70, 110);
}

static void EnGoma_SetupPrepareJump(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolPrepareJumpAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolPrepareJumpAnim), ANIMMODE_ONCE, -5.0f);
    this->actionFunc = EnGoma_PrepareJump;
    this->actionTimer = 30;
}

static void EnGoma_PrepareJump(EnGoma* this, PlayState* play) {
    s16 targetAngle;

    SkelAnime_Update(&this->skelanime);
    Math_ApproachZeroF(&this->actor.speed, 0.5f, 2.0f);

    targetAngle = Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor);
    Math_ApproachS(&this->actor.world.rot.y, targetAngle, 2, 4000);
    Math_ApproachS(&this->actor.shape.rot.y, targetAngle, 2, 3000);

    if (this->actionTimer == 0) {
        EnGoma_SetupJump(this);
    }
    this->visualState = 0;
}

static void EnGoma_SetupLand(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolLandFromJumpAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolLandFromJumpAnim), ANIMMODE_ONCE, 0.0f);
    this->actionFunc = EnGoma_Land;
    this->actionTimer = 10;
}

static void EnGoma_Land(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        Math_ApproachZeroF(&this->actor.speed, 1.0f, 2.0f);
    }
    if (this->actionTimer == 0) {
        EnGoma_SetupStand(this);
    }
}

static void EnGoma_SetupJump(EnGoma* this) {
    Animation_Change(&this->skelanime, &gObjectGolJumpHeadbuttAnim, 1.0f, 0.0f,
                     Animation_GetLastFrame(&gObjectGolJumpHeadbuttAnim), ANIMMODE_ONCE, 0.0f);
    this->actionFunc = EnGoma_Jump;
    this->actor.velocity.y = 8.0f;

    if (this->actor.params < 6) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_VOICE3_OLD);
    } else {
        Actor_PlaySfx(&this->actor, NA_SE_EN_FROG_HOLD_SLIME);
    }
}

static void EnGoma_Jump(EnGoma* this, PlayState* play) {
    this->actor.flags |= ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT;
    SkelAnime_Update(&this->skelanime);
    Math_ApproachF(&this->actor.speed, 10.0f, 0.5f, 5.0f);

    if (this->actor.velocity.y <= 0.0f && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        EnGoma_SetupLand(this);
        if (this->actor.params < 6) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_FLOAT_OLD);
        } else {
            Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_ROLLING_OLD);
        }
    }
    this->visualState = 0;
}

static void EnGoma_Stand(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);
    Math_ApproachZeroF(&this->actor.speed, 0.5f, 2.0f);
    Math_ApproachS(&this->actor.shape.rot.y, Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor), 2,
                   3000);

    if (this->actionTimer == 0) {
        EnGoma_SetupChasePlayer(this);
    }
}

static void EnGoma_ChasePlayer(EnGoma* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);

    if (Animation_OnFrame(&this->skelanime, 1.0f) || Animation_OnFrame(&this->skelanime, 5.0f)) {
        if (this->actor.params < 6) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_VOICE2_OLD);
        } else {
            Actor_PlaySfx(&this->actor, NA_SE_EN_FROG_GREET);
        }
    }

    Math_ApproachF(&this->actor.speed, 10.0f / 3.0f, 0.5f, 2.0f);
    Math_ApproachS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 3, 2000);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 2, 3000);

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.velocity.y = 0.0f;
    }
    if (this->actor.xzDistToPlayer <= 150.0f) {
        EnGoma_SetupPrepareJump(this);
    }
}

static void EnGoma_SetupStunned(EnGoma* this, PlayState* play) {
    this->actionFunc = EnGoma_Stunned;
    this->stunTimer = 100;
    Animation_MorphToLoop(&this->skelanime, &gObjectGolStandAnim, -5.0f);
    this->actionTimer = (s16)Rand_ZeroFloat(15.0f) + 3;

    if (this->actor.params < 6) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIZUBABA_DEAD);
    } else {
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
    }
}

static void EnGoma_Stunned(EnGoma* this, PlayState* play) {
    Actor_SetColorFilter(&this->actor, 0, 180, 0, 2);
    this->visualState = 2;

    if (this->actionTimer != 0) {
        SkelAnime_Update(&this->skelanime);
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.velocity.y = 0.0f;
        Math_ApproachZeroF(&this->actor.speed, 0.5f, 2.0f);
    }

    if (this->stunTimer == 0) {
        EnGoma_SetupStand(this);
    } else if (--this->stunTimer < 30) {
        if (this->stunTimer & 1) {
            this->actor.world.pos.x += 1.5f;
            this->actor.world.pos.z += 1.5f;
        } else {
            this->actor.world.pos.x -= 1.5f;
            this->actor.world.pos.z -= 1.5f;
        }
    }
}

static void EnGoma_LookAtPlayer(EnGoma* this, PlayState* play) {
    s16 eyePitch;
    s16 eyeYaw;

    eyeYaw = Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor) - this->actor.shape.rot.y;
    eyePitch = Actor_WorldPitchTowardActor(&this->actor, &GET_PLAYER(play)->actor) - this->actor.shape.rot.x;

    if (eyeYaw > 6000) {
        eyeYaw = 6000;
    }
    if (eyeYaw < -6000) {
        eyeYaw = -6000;
    }
    Math_ApproachS(&this->eyeYaw, eyeYaw, 3, 2000);
    Math_ApproachS(&this->eyePitch, eyePitch, 3, 2000);
}

static void EnGoma_UpdateHit(EnGoma* this, PlayState* play) {
    static Vec3f sShieldKnockbackVel = { 0.0f, 0.0f, 20.0f };
    Player* player = GET_PLAYER(play);

    if (this->hurtTimer != 0) {
        this->hurtTimer--;
    } else {
        ColliderElement* acHitElem;
        u8 swordDamage;

        if ((this->colCyl1.base.atFlags & 2) && this->actionFunc == EnGoma_Jump) {
            EnGoma_SetupLand(this);
            this->actor.speed = 0.0f;
            this->actor.velocity.y = 0.0f;
        }

        if ((this->colCyl2.base.acFlags & AC_HIT) && (s8)this->actor.colChkInfo.health > 0) {
            acHitElem = this->colCyl2.elem.acHitElem;
            this->colCyl2.base.acFlags &= ~AC_HIT;

            if (this->gomaType == ENGOMA_NORMAL) {
                u32 dmgFlags = acHitElem->atDmgInfo.dmgFlags;

                if (dmgFlags & 0x100000) {
                    if (this->actionFunc == EnGoma_Jump) {
                        EnGoma_SetupLand(this);
                        this->actor.velocity.y = 0.0f;
                        this->actor.speed = -5.0f;
                    } else {
                        Matrix_RotateYF(player->actor.shape.rot.y / (f32)0x8000 * M_PI, MTXMODE_NEW);
                        Matrix_MultVec3f(&sShieldKnockbackVel, &this->shieldKnockbackVel);
                        this->invincibilityTimer = 5;
                    }
                } else if (dmgFlags & 1) { // stun
                    if (this->actionFunc != EnGoma_Stunned) {
                        EnGoma_SetupStunned(this, play);
                        this->hurtTimer = 8;
                    }
                } else {
                    swordDamage = CollisionCheck_GetSwordDamageOot(dmgFlags, play);

                    if (swordDamage) {
                        EffectSsSibuki_SpawnBurst(play, &this->actor.focus.pos);
                    } else {
                        swordDamage = 1;
                    }

                    this->actor.colChkInfo.health -= swordDamage;
                    EnGoma_SetupHurt(this, play);
                    Actor_SetColorFilter(&this->actor, 0x4000, 255, 0, 5);
                    this->hurtTimer = 13;
                }
            } else {
                // die if still an egg
                if (this->actor.params <= 5) { //! BossGoma only has 3 children
                    BossGoma* parent = (BossGoma*)this->actor.parent;

                    parent->childrenGohmaState[this->actor.params] = -1;
                }

                EnGoma_SpawnHatchDebris(this, play);
                Actor_Kill(&this->actor);
            }
        }
    }
}

static void EnGoma_UpdateEyeEnvColor(EnGoma* this) {
    static f32 sTargetEyeEnvColors[][3] = {
        { 255.0f, 0.0f, 50.0f },
        { 17.0f, 255.0f, 50.0f },
        { 0.0f, 170.0f, 50.0f },
    };

    Math_ApproachF(&this->eyeEnvColor[0], sTargetEyeEnvColors[0][this->visualState], 0.5f, 20.0f);
    Math_ApproachF(&this->eyeEnvColor[1], sTargetEyeEnvColors[1][this->visualState], 0.5f, 20.0f);
    Math_ApproachF(&this->eyeEnvColor[2], sTargetEyeEnvColors[2][this->visualState], 0.5f, 20.0f);
}

static void EnGoma_SetFloorRot(EnGoma* this) {
    f32 nx;
    f32 ny;
    f32 nz;

    if (this->actor.floorPoly != NULL) {
        nx = COLPOLY_GET_NORMAL(this->actor.floorPoly->normal.x);
        ny = COLPOLY_GET_NORMAL(this->actor.floorPoly->normal.y);
        nz = COLPOLY_GET_NORMAL(this->actor.floorPoly->normal.z);
        Math_ApproachS(&this->slopePitch, -Math_FAtan2F(-nz * ny, 1.0f) * (0x8000 / M_PI), 1, 1000);
        Math_ApproachS(&this->slopeRoll, Math_FAtan2F(-nx * ny, 1.0f) * (0x8000 / M_PI), 1, 1000);
    }
}

static void EnGoma_Update(Actor* thisx, PlayState* play) {
    EnGoma* this = (EnGoma*)thisx;
    s32 pad;
    Player* player = GET_PLAYER(play);

    if (this->actionTimer != 0) {
        this->actionTimer--;
    }
    if (this->invincibilityTimer != 0) {
        this->invincibilityTimer--;
    }

    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->actor);
    this->actor.world.pos.x = this->actor.world.pos.x + this->shieldKnockbackVel.x;
    this->actor.world.pos.z = this->actor.world.pos.z + this->shieldKnockbackVel.z;
    Math_ApproachZeroF(&this->shieldKnockbackVel.x, 1.0f, 3.0f);
    Math_ApproachZeroF(&this->shieldKnockbackVel.z, 1.0f, 3.0f);

    if (this->actor.params < 10) {
        this->eggTimer++;
        Math_SmoothStepToF(&this->actor.scale.x, 0.01f, 0.5f, 0.00075f, 0.000001f);
        Math_SmoothStepToF(&this->actor.scale.y, 0.01f, 0.5f, 0.00075f, 0.000001f);
        Math_SmoothStepToF(&this->actor.scale.z, 0.01f, 0.5f, 0.00075f, 0.000001f);
        EnGoma_UpdateHit(this, play);
        Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 50.0f, 100.0f, 0x405);
        EnGoma_SetFloorRot(this);
        Actor_SetFocus(&this->actor, 20.0f);
        EnGoma_LookAtPlayer(this, play);
        EnGoma_UpdateEyeEnvColor(this);
        this->visualState = 1;
        if (player->meleeWeaponState != 0) {
            this->colCyl2.dim.radius = 35;
            this->colCyl2.dim.height = 35;
            this->colCyl2.dim.yShift = 0;
        } else {
            this->colCyl2.dim.radius = 15;
            this->colCyl2.dim.height = 30;
            this->colCyl2.dim.yShift = 10;
        }
        if (this->invincibilityTimer == 0) {
            Collider_UpdateCylinder(&this->actor, &this->colCyl1);
            Collider_UpdateCylinder(&this->actor, &this->colCyl2);
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->colCyl1.base);
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->colCyl2.base);
            CollisionCheck_SetAT(play, &play->colChkCtx, &this->colCyl1.base);
        }
    }
}

static s32 EnGoma_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnGoma* this = (EnGoma*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, (s16)this->eyeEnvColor[0], (s16)this->eyeEnvColor[1], (s16)this->eyeEnvColor[2],
                   255);

    if (limbIndex == GOMA_LIMB_EYE_IRIS_ROOT1) {
        rot->x += this->eyePitch;
        rot->y += this->eyeYaw;
    } else if (limbIndex == GOMA_LIMB_BODY && this->hurtTimer != 0) {
        gDPSetEnvColor(POLY_OPA_DISP++, (s16)(Rand_ZeroOne() * 255.0f), (s16)(Rand_ZeroOne() * 255.0f),
                       (s16)(Rand_ZeroOne() * 255.0f), 255);
    }

    CLOSE_DISPS(play->state.gfxCtx);
    return 0;
}

static Gfx* EnGoma_NoBackfaceCullingDlist(GraphicsContext* gfxCtx) {
    Gfx* dListHead;
    Gfx* dList;

    dListHead = dList = GRAPH_ALLOC(gfxCtx, sizeof(Gfx) * 4);
    gDPPipeSync(dListHead++);
    gDPSetRenderMode(dListHead++, G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2);
    gSPClearGeometryMode(dListHead++, G_CULL_BACK);
    gSPEndDisplayList(dListHead++);
    return dList;
}

static void EnGoma_Draw(Actor* thisx, PlayState* play) {
    EnGoma* this = (EnGoma*)thisx;
    s32 y;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    switch (this->gomaType) {
        case ENGOMA_NORMAL:
            // PORT: OoT gives Navi enemy info 0x03;
            Matrix_Translate(this->actor.world.pos.x,
                             this->actor.world.pos.y +
                                 ((this->actor.shape.yOffset * this->actor.scale.y) + play->mainCamera.quakeOffset.y),
                             this->actor.world.pos.z, MTXMODE_NEW);
            Matrix_RotateXFApply(this->slopePitch / (f32)0x8000 * M_PI);
            Matrix_RotateZF(this->slopeRoll / (f32)0x8000 * M_PI, MTXMODE_APPLY);
            Matrix_RotateYF(this->actor.shape.rot.y / (f32)0x8000 * M_PI, MTXMODE_APPLY);
            Matrix_RotateXFApply(this->actor.shape.rot.x / (f32)0x8000 * M_PI);
            Matrix_RotateZF(this->actor.shape.rot.z / (f32)0x8000 * M_PI, MTXMODE_APPLY);
            Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);
            SkelAnime_DrawSkeletonOpaOot(play, &this->skelanime, EnGoma_OverrideLimbDraw, NULL, this);
            break;

        case ENGOMA_EGG:
            // PORT: OoT gives Navi enemy info 0x02;
            y = (s16)(sinf((this->eggTimer * 5.0f * 3.1415f) / 180.0f) * 31.9f);
            y = (s16)(y + 31);
            gSPSegment(POLY_OPA_DISP++, 0x08, Gfx_TexScrollOot(play->state.gfxCtx, 0, y));
            Matrix_Push();
            Matrix_Scale(this->eggScale, 1.0f / this->eggScale, this->eggScale, MTXMODE_APPLY);
            Matrix_RotateYF(this->eggSquishAngle * 0.15f, MTXMODE_APPLY);
            Matrix_RotateZF(this->eggSquishAngle * 0.1f, MTXMODE_APPLY);
            Matrix_Scale(0.95f - this->eggSquishAmount, this->eggSquishAmount + 1.05f, 0.95f - this->eggSquishAmount,
                         MTXMODE_APPLY);
            Matrix_RotateZF(-(this->eggSquishAngle * 0.1f), MTXMODE_APPLY);
            Matrix_RotateYF(-(this->eggSquishAngle * 0.15f), MTXMODE_APPLY);
            Matrix_Translate(0.0f, this->eggYOffset, 0.0f, MTXMODE_APPLY);
            Matrix_RotateXFApply(this->eggPitch);
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, gObjectGolEggDL);
            Matrix_Pop();
            break;

        case ENGOMA_HATCH_DEBRIS:
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, gBrownFragmentDL);
            break;

        case ENGOMA_BOSSLIMB:
            if (this->bossLimbDL != NULL) {
                gSPSegment(POLY_OPA_DISP++, 0x08, EnGoma_NoBackfaceCullingDlist(play->state.gfxCtx));
                MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
                gSPDisplayList(POLY_OPA_DISP++, this->bossLimbDL);
            }
            break;
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnGoma_Debris(EnGoma* this, PlayState* play) {
    this->actor.shape.rot.y += 2500;
    this->actor.shape.rot.x += 3500;
    if (this->actionTimer == 0) {
        Actor_Kill(&this->actor);
    }
}

static void EnGoma_SpawnHatchDebris(EnGoma* this, PlayState* play2) {
    PlayState* play = play2;
    s16 i;

    if (this->actor.params < 6) {
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 40, NA_SE_EN_MIBOSS_VOICE1_OLD);
    } else {
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 40, NA_SE_EN_FROG_PUNCH2);
    }

    for (i = 0; i < 15; i++) {
        Actor_SpawnAsChild(
            &play->actorCtx, &this->actor, play, ACTOR_EN_GOMA_Oot, Rand_CenteredFloat(10.0f) + this->actor.world.pos.x,
            Rand_CenteredFloat(10.0f) + this->actor.world.pos.y + 15.0f,
            Rand_CenteredFloat(10.0f) + this->actor.world.pos.z, 0, Rand_CenteredFloat(0x10000 - 0.01f), 0, i + 10);
    }
}

static void EnGoma_BossLimb(EnGoma* this, PlayState* play) {
    Vec3f vel = { 0.0f, 0.0f, 0.0f };
    Vec3f accel = { 0.0f, 1.0f, 0.0f };
    Color_RGBA8 primColor = { 255, 255, 255, 255 };
    Color_RGBA8 envColor = { 0, 100, 255, 255 };
    Vec3f pos;

    this->actor.world.pos.y -= 5.0f;
    Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 50.0f, 100.0f, 0x404);
    this->actor.world.pos.y += 5.0f;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.velocity.y = 0.0f;
    } else if (this->actionTimer < 250) {
        this->actor.shape.rot.y += 2000;
    }

    if (this->actionTimer == 250) {
        this->actor.gravity = -1.0f;
    }

    if (this->actionTimer < 121) {
        if (Math_SmoothStepToF(&this->actor.scale.y, 0.0f, 1.0f, 0.00075f, 0) <= 0.001f) {
            Actor_Kill(&this->actor);
        }
        this->actor.scale.x = this->actor.scale.z = this->actor.scale.y;
    }

    if (this->actionTimer % 8 == 0 && this->actionTimer != 0) {
        pos.x = Rand_CenteredFloat(20.0f) + this->actor.world.pos.x;
        pos.y = Rand_CenteredFloat(10.0f) + this->actor.world.pos.y;
        pos.z = Rand_CenteredFloat(20.0f) + this->actor.world.pos.z;
        func_8002836C(play, &pos, &vel, &accel, &primColor, &envColor, 500, 10, 10);
    }
}

static void EnGoma_Reset(void) {
    sSpawnNum = 0;
}
