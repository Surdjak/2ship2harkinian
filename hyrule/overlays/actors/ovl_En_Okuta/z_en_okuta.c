#include "z_en_okuta.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyrulePlayer.h"
#include "assets/oot/objects/object_okuta/object_okuta.h"
#include "assets/oot/objects/gameplay_field_keep/gameplay_field_keep.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE)

static void EnOkuta_Init(Actor* thisx, PlayState* play);
static void EnOkuta_Destroy(Actor* thisx, PlayState* play);
static void EnOkuta_Update(Actor* thisx, PlayState* play);
static void EnOkuta_Draw(Actor* thisx, PlayState* play);

static void EnOkuta_SetupWaitToAppear(EnOkuta* this);
static void EnOkuta_WaitToAppear(EnOkuta* this, PlayState* play);
static void EnOkuta_Appear(EnOkuta* this, PlayState* play);
static void EnOkuta_Hide(EnOkuta* this, PlayState* play);
static void EnOkuta_WaitToShoot(EnOkuta* this, PlayState* play);
static void EnOkuta_Shoot(EnOkuta* this, PlayState* play);
static void EnOkuta_WaitToDie(EnOkuta* this, PlayState* play);
static void EnOkuta_Die(EnOkuta* this, PlayState* play);
static void EnOkuta_Freeze(EnOkuta* this, PlayState* play);
static void EnOkuta_ProjectileFly(EnOkuta* this, PlayState* play);

ActorProfile En_Okuta_Oot_Profile = {
    /**/ ACTOR_EN_OKUTA_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_OKUTA_Oot,
    /**/ sizeof(EnOkuta),
    /**/ EnOkuta_Init,
    /**/ EnOkuta_Destroy,
    /**/ EnOkuta_Update,
    /**/ EnOkuta_Draw,
};

static ColliderCylinderInit sProjectileColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
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
    { 13, 20, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sOctorockColliderInit = {
    {
        COL_MATERIAL_HIT0,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 20, 40, -30, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sColChkInfoInit = { 1, 15, 60, 100 };

static DamageTable sDamageTable = DMG_TABLE_FROM_OOT(
    /* Deku nut      */ DMG_ENTRY(0, 0x0),
    /* Deku stick    */ DMG_ENTRY(2, 0x0),
    /* Slingshot     */ DMG_ENTRY(1, 0x0),
    /* Explosive     */ DMG_ENTRY(2, 0x0),
    /* Boomerang     */ DMG_ENTRY(1, 0x0),
    /* Normal arrow  */ DMG_ENTRY(2, 0x0),
    /* Hammer swing  */ DMG_ENTRY(2, 0x0),
    /* Hookshot      */ DMG_ENTRY(2, 0x0),
    /* Kokiri sword  */ DMG_ENTRY(1, 0x0),
    /* Master sword  */ DMG_ENTRY(2, 0x0),
    /* Giant's Knife */ DMG_ENTRY(4, 0x0),
    /* Fire arrow    */ DMG_ENTRY(2, 0x0),
    /* Ice arrow     */ DMG_ENTRY(4, 0x3),
    /* Light arrow   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 1   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 2   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 3   */ DMG_ENTRY(2, 0x0),
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
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x42
    ICHAIN_F32(lockOnArrowOffset, 6500, ICHAIN_STOP),
};

static void EnOkuta_Init(Actor* thisx, PlayState* play) {
    EnOkuta* this = (EnOkuta*)thisx;
    s32 pad;
    WaterBox* outWaterBox;
    f32 ySurface;
    s32 sp30;

    Actor_ProcessInitChain(thisx, sInitChain);
    this->numShots = (thisx->params >> 8) & 0xFF;
    thisx->params &= 0xFF;
    if (thisx->params == 0) {
        SkelAnime_Init(play, &this->skelAnime, &gOctorokSkel, &gOctorokAppearAnim, this->jointTable, this->morphTable,
                       38);
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, thisx, &sOctorockColliderInit);
        CollisionCheck_SetInfo(&thisx->colChkInfo, &sDamageTable, &sColChkInfoInit);
        if ((this->numShots == 0xFF) || (this->numShots == 0)) {
            this->numShots = 1;
        }
        thisx->floorHeight =
            BgCheck_EntityRaycastFloor5(&play->colCtx, &thisx->floorPoly, &sp30, thisx, &thisx->world.pos);
        //! @bug calls WaterBox_GetSurfaceImpl directly
        if (!BgCheck_GetWaterSurfaceNoBgId(play, &play->colCtx, thisx->world.pos.x, thisx->world.pos.z, &ySurface,
                                     &outWaterBox) ||
            (ySurface <= thisx->floorHeight)) {
            Actor_Kill(thisx);
        } else {
            thisx->home.pos.y = ySurface;
        }
        EnOkuta_SetupWaitToAppear(this);
    } else {
        ActorShape_Init(&thisx->shape, 1100.0f, ActorShadow_DrawCircle, 18.0f);
        thisx->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        thisx->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, thisx, &sProjectileColliderInit);
        Actor_ChangeCategory(play, &play->actorCtx, thisx, ACTORCAT_PROP);
        this->timer = 30;
        thisx->shape.rot.y = 0;
        this->actionFunc = EnOkuta_ProjectileFly;
        thisx->speed = 10.0f;
    }
}

static void EnOkuta_Destroy(Actor* thisx, PlayState* play) {
    EnOkuta* this = (EnOkuta*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnOkuta_SpawnBubbles(EnOkuta* this, PlayState* play) {
    s32 i;

    for (i = 0; i < 10; i++) {
        EffectSsBubble_Spawn(play, &this->actor.world.pos, -10.0f, 10.0f, 30.0f, 0.25f);
    }
}

static void EnOkuta_SpawnDust(Vec3f* pos, Vec3f* velocity, s16 scaleStep, PlayState* play) {
    static Vec3f accel = { 0.0f, 0.0f, 0.0f };
    static Color_RGBA8 primColor = { 255, 255, 255, 255 };
    static Color_RGBA8 envColor = { 150, 150, 150, 255 };

    func_8002829C(play, pos, velocity, &accel, &primColor, &envColor, 0x190, scaleStep);
}

static void EnOkuta_SpawnSplash(EnOkuta* this, PlayState* play) {
    EffectSsGSplash_Spawn(play, &this->actor.home.pos, NULL, NULL, 0, 1300);
}

static void EnOkuta_SpawnRipple(EnOkuta* this, PlayState* play) {
    Vec3f pos;

    pos.x = this->actor.world.pos.x;
    pos.y = this->actor.home.pos.y;
    pos.z = this->actor.world.pos.z;
    if ((play->gameplayFrames % 7) == 0 &&
        ((this->actionFunc != EnOkuta_Shoot) || ((this->actor.world.pos.y - this->actor.home.pos.y) < 50.0f))) {
        EffectSsGRipple_Spawn(play, &pos, 250, 650, 0);
    }
}

static void EnOkuta_SetupWaitToAppear(EnOkuta* this) {
    this->actor.draw = NULL;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->actionFunc = EnOkuta_WaitToAppear;
    this->actor.world.pos.y = this->actor.home.pos.y;
}

static void EnOkuta_SetupAppear(EnOkuta* this, PlayState* play) {
    this->actor.draw = EnOkuta_Draw;
    this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    Animation_PlayOnce(&this->skelAnime, &gOctorokAppearAnim);
    EnOkuta_SpawnBubbles(this, play);
    this->actionFunc = EnOkuta_Appear;
}

static void EnOkuta_SetupHide(EnOkuta* this) {
    Animation_PlayOnce(&this->skelAnime, &gOctorokHideAnim);
    this->actionFunc = EnOkuta_Hide;
}

static void EnOkuta_SetupWaitToShoot(EnOkuta* this) {
    Animation_PlayLoop(&this->skelAnime, &gOctorokFloatAnim);
    this->timer = (this->actionFunc == EnOkuta_Shoot) ? 2 : 0;
    this->actionFunc = EnOkuta_WaitToShoot;
}

static void EnOkuta_SetupShoot(EnOkuta* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, &gOctorokShootAnim);
    if (this->actionFunc != EnOkuta_Shoot) {
        this->timer = this->numShots;
    }
    this->jumpHeight = this->actor.playerHeightRel + 20.0f;
    this->jumpHeight = CLAMP_MIN(this->jumpHeight, 10.0f);
    if (this->jumpHeight > 50.0f) {
        EnOkuta_SpawnSplash(this, play);
    }
    if (this->jumpHeight > 50.0f) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_JUMP);
    }
    this->actionFunc = EnOkuta_Shoot;
}

static void EnOkuta_SetupWaitToDie(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gOctorokHitAnim, -5.0f);
    Actor_SetColorFilter(&this->actor, 0x4000, 0xFF, 0, 0xB);
    this->collider.base.acFlags &= ~AC_HIT;
    Actor_SetScale(&this->actor, 0.01f);
    Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_DEAD1);
    this->actionFunc = EnOkuta_WaitToDie;
}

static void EnOkuta_SetupDie(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gOctorokDieAnim, -3.0f);
    this->timer = 0;
    this->actionFunc = EnOkuta_Die;
}

static void EnOkuta_SetupFreeze(EnOkuta* this) {
    this->timer = 80;
    Actor_SetColorFilter(&this->actor, 0, 0xFF, 0, 0x50);
    this->actionFunc = EnOkuta_Freeze;
}

static void EnOkuta_SpawnProjectile(EnOkuta* this, PlayState* play) {
    Vec3f pos;
    Vec3f velocity;
    f32 sin = Math_SinS(this->actor.shape.rot.y);
    f32 cos = Math_CosS(this->actor.shape.rot.y);

    pos.x = this->actor.world.pos.x + (25.0f * sin);
    pos.y = this->actor.world.pos.y - 6.0f;
    pos.z = this->actor.world.pos.z + (25.0f * cos);
    if (Actor_Spawn(&play->actorCtx, play, ACTOR_EN_OKUTA_Oot, pos.x, pos.y, pos.z, this->actor.shape.rot.x,
                    this->actor.shape.rot.y, this->actor.shape.rot.z, 0x10) != NULL) {
        pos.x = this->actor.world.pos.x + (40.0f * sin);
        pos.z = this->actor.world.pos.z + (40.0f * cos);
        pos.y = this->actor.world.pos.y;
        velocity.x = 1.5f * sin;
        velocity.y = 0.0f;
        velocity.z = 1.5f * cos;
        EnOkuta_SpawnDust(&pos, &velocity, 20, play);
    }
    Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_THROW);
}

static void EnOkuta_WaitToAppear(EnOkuta* this, PlayState* play) {
    this->actor.world.pos.y = this->actor.home.pos.y;
    if ((this->actor.xzDistToPlayer < 480.0f) && (this->actor.xzDistToPlayer > 200.0f)) {
        EnOkuta_SetupAppear(this, play);
    }
}

static void EnOkuta_Appear(EnOkuta* this, PlayState* play) {
    s32 pad;

    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->actor.xzDistToPlayer < 160.0f) {
            EnOkuta_SetupHide(this);
        } else {
            EnOkuta_SetupWaitToShoot(this);
        }
    } else if (this->skelAnime.curFrame <= 4.0f) {
        Actor_SetScale(&this->actor, this->skelAnime.curFrame * 0.25f * 0.01f);
    } else if (Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_SetScale(&this->actor, 0.01f);
    }
    if (Animation_OnFrame(&this->skelAnime, 2.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_JUMP);
    }
    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
    }
    if (Animation_OnFrame(&this->skelAnime, 3.0f) || Animation_OnFrame(&this->skelAnime, 15.0f)) {
        EnOkuta_SpawnSplash(this, play);
    }
}

static void EnOkuta_Hide(EnOkuta* this, PlayState* play) {
    s32 pad;

    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 30.0f);
    if (SkelAnime_Update(&this->skelAnime)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_WATER_MID);
        EnOkuta_SpawnBubbles(this, play);
        EnOkuta_SetupWaitToAppear(this);
    } else if (this->skelAnime.curFrame >= 4.0f) {
        Actor_SetScale(&this->actor, (6.0f - this->skelAnime.curFrame) * 0.5f * 0.01f);
    }
    if (Animation_OnFrame(&this->skelAnime, 2.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_SINK);
    }
    if (Animation_OnFrame(&this->skelAnime, 4.0f)) {
        EnOkuta_SpawnSplash(this, play);
    }
}

static void EnOkuta_WaitToShoot(EnOkuta* this, PlayState* play) {
    s16 temp_v0_2;
    s32 phi_v1;

    this->actor.world.pos.y = this->actor.home.pos.y;
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        if (this->timer != 0) {
            this->timer--;
        }
    }
    if (Animation_OnFrame(&this->skelAnime, 0.5f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_WATER_SLW);
    }
    if (this->actor.xzDistToPlayer < 160.0f || this->actor.xzDistToPlayer > 560.0f) {
        EnOkuta_SetupHide(this);
    } else {
        temp_v0_2 = Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0x71C, 0x38E);
        phi_v1 = ABS(temp_v0_2);
        if ((phi_v1 < 0x38E) && (this->timer == 0) && (this->actor.playerHeightRel < 200.0f)) {
            EnOkuta_SetupShoot(this, play);
        }
    }
}

static void EnOkuta_Shoot(EnOkuta* this, PlayState* play) {
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0x71C);
    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->timer != 0) {
            this->timer--;
        }
        if (this->timer == 0) {
            EnOkuta_SetupWaitToShoot(this);
        } else {
            EnOkuta_SetupShoot(this, play);
        }
    } else {
        f32 curFrame = this->skelAnime.curFrame;

        if (curFrame < 13.0f) {
            this->actor.world.pos.y = (sinf((0.08333f * M_PI) * curFrame) * this->jumpHeight) + this->actor.home.pos.y;
        }
        if (Animation_OnFrame(&this->skelAnime, 6.0f)) {
            EnOkuta_SpawnProjectile(this, play);
        }
        if ((this->jumpHeight > 50.0f) && Animation_OnFrame(&this->skelAnime, 13.0f)) {
            EnOkuta_SpawnSplash(this, play);
        }
        if ((this->jumpHeight > 50.0f) && Animation_OnFrame(&this->skelAnime, 13.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
        }
    }
    if (this->actor.xzDistToPlayer < 160.0f) {
        EnOkuta_SetupHide(this);
    }
}

static void EnOkuta_WaitToDie(EnOkuta* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        EnOkuta_SetupDie(this);
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
}

static void EnOkuta_Die(EnOkuta* this, PlayState* play) {
    static Vec3f accel = { 0.0f, -0.5f, 0.0f };
    static Color_RGBA8 primColor = { 255, 255, 255, 255 };
    static Color_RGBA8 envColor = { 150, 150, 150, 0 };
    Vec3f velocity;
    Vec3f pos;
    s32 i;

    if (SkelAnime_Update(&this->skelAnime)) {
        this->timer++;
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
    if (this->timer == 5) {
        pos.x = this->actor.world.pos.x;
        pos.y = this->actor.world.pos.y + 40.0f;
        pos.z = this->actor.world.pos.z;
        velocity.x = 0.0f;
        velocity.y = -0.5f;
        velocity.z = 0.0f;
        EnOkuta_SpawnDust(&pos, &velocity, -0x14, play);
        Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_DEAD2);
    }
    if (Animation_OnFrame(&this->skelAnime, 15.0f)) {
        EnOkuta_SpawnSplash(this, play);
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
    }
    if (this->timer < 3) {
        Actor_SetScale(&this->actor, ((this->timer * 0.25f) + 1.0f) * 0.01f);
    } else if (this->timer < 6) {
        Actor_SetScale(&this->actor, (1.5f - ((this->timer - 2) * 0.2333f)) * 0.01f);
    } else if (this->timer < 11) {
        Actor_SetScale(&this->actor, (((this->timer - 5) * 0.04f) + 0.8f) * 0.01f);
    } else {
        if (Math_StepToF(&this->actor.scale.x, 0.0f, 0.0005f)) {
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 30, NA_SE_EN_COMMON_WATER_MID);
            // PORT: OoT's drop tables are not ported, so this draws from MM's table with the same index.
            Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0x70);
            for (i = 0; i < 20; i++) {
                velocity.x = (Rand_ZeroOne() - 0.5f) * 7.0f;
                velocity.y = Rand_ZeroOne() * 7.0f;
                velocity.z = (Rand_ZeroOne() - 0.5f) * 7.0f;
                EffectSsDtBubble_SpawnCustomColor(play, &this->actor.world.pos, &velocity, &accel, &primColor,
                                                  &envColor, Rand_S16Offset(100, 50), 25, 0);
            }
            Actor_Kill(&this->actor);
        }
        this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;
    }
}

static void EnOkuta_Freeze(EnOkuta* this, PlayState* play) {
    Vec3f pos;
    s16 temp_v1;

    if (this->timer != 0) {
        this->timer--;
    }
    if (this->timer == 0) {
        EnOkuta_SetupDie(this);
    }
    if ((this->timer >= 64) && (this->timer & 1)) {
        temp_v1 = (this->timer - 64) >> 1;
        pos.y = (this->actor.world.pos.y - 32.0f) + (8.0f * (8 - temp_v1));
        pos.x = this->actor.world.pos.x + ((temp_v1 & 2) ? 10.0f : -10.0f);
        pos.z = this->actor.world.pos.z + ((temp_v1 & 1) ? 10.0f : -10.0f);
        EffectSsEnIce_SpawnFlyingVec3f(play, &this->actor, &pos, 150, 150, 150, 250, 235, 245, 255,
                                       (Rand_ZeroOne() * 0.2f) + 1.9f);
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
}

static void EnOkuta_ProjectileFly(EnOkuta* this, PlayState* play) {
    Vec3f pos;
    Player* player = GET_PLAYER(play);
    Vec3s sp40;

    this->timer--;
    if (this->timer == 0) {
        this->actor.gravity = -1.0f;
    }
    this->actor.home.rot.z += 0x1554;
    if (this->actor.bgCheckFlags & 0x20) {
        this->actor.gravity = -1.0f;
        this->actor.speed -= 0.1f;
        this->actor.speed = CLAMP_MIN(this->actor.speed, 1.0f);
    }
    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) || (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        (this->collider.base.atFlags & AT_HIT) || this->collider.base.acFlags & AC_HIT ||
        this->collider.base.ocFlags1 & OC1_HIT || this->actor.floorHeight == BGCHECK_Y_MIN) {
        // MM's Hero's Shield, in the Deku Shield's place, sends rocks back as it does MM's Octoroks'.
        if ((player->currentShield == PLAYER_SHIELD_DEKU_Oot ||
             player->currentShield == PLAYER_SHIELD_HEROS_SHIELD ||
             (player->currentShield == PLAYER_SHIELD_HYLIAN_Oot && LINK_IS_ADULT_OOT)) &&
            this->collider.base.atFlags & AT_HIT && this->collider.base.atFlags & AT_TYPE_ENEMY &&
            this->collider.base.atFlags & AT_BOUNCED) {
            this->collider.base.atFlags &= ~(AT_HIT | AT_BOUNCED | AT_TYPE_ENEMY);
            this->collider.base.atFlags |= AT_TYPE_PLAYER;
            this->collider.elem.atDmgInfo.dmgFlags = DMG_FROM_OOT(0x2);
            Matrix_MtxFToYXZRot(&player->shieldMf, &sp40, 0);
            this->actor.world.rot.y = sp40.y + 0x8000;
            this->timer = 30;
        } else {
            pos.x = this->actor.world.pos.x;
            pos.y = this->actor.world.pos.y + 11.0f;
            pos.z = this->actor.world.pos.z;
            EffectSsHahen_SpawnBurst(play, &pos, 6.0f, 0, 1, 2, 15, 7, 10, gOctorokProjectileDL);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 20, NA_SE_EN_OCTAROCK_ROCK);
            Actor_Kill(&this->actor);
        }
    } else if (this->timer == -300) {
        Actor_Kill(&this->actor);
    }
}

static void EnOkuta_UpdateHeadScale(EnOkuta* this) {
    f32 curFrame = this->skelAnime.curFrame;

    if (this->actionFunc == EnOkuta_Appear) {
        if (curFrame < 8.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = 1.0f;
        } else if (curFrame < 10.0f) {
            this->headScale.x = this->headScale.z = 1.0f;
            this->headScale.y = ((curFrame - 7.0f) * 0.4f) + 1.0f;
        } else if (curFrame < 14.0f) {
            this->headScale.x = this->headScale.z = ((curFrame - 9.0f) * 0.075f) + 1.0f;
            this->headScale.y = 1.8f - ((curFrame - 9.0f) * 0.25f);
        } else {
            this->headScale.x = this->headScale.z = 1.3f - ((curFrame - 13.0f) * 0.05f);
            this->headScale.y = ((curFrame - 13.0f) * 0.0333f) + 0.8f;
        }
    } else if (this->actionFunc == EnOkuta_Hide) {
        if (curFrame < 3.0f) {
            this->headScale.y = 1.0f;
        } else if (curFrame < 4.0f) {
            this->headScale.y = (curFrame - 2.0f) + 1.0f;
        } else {
            this->headScale.y = 2.0f - ((curFrame - 3.0f) * 0.333f);
        }
        this->headScale.x = this->headScale.z = 1.0f;
    } else if (this->actionFunc == EnOkuta_Shoot) {
        if (curFrame < 5.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = (curFrame * 0.125f) + 1.0f;
        } else if (curFrame < 7.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = 1.5f - ((curFrame - 4.0f) * 0.35f);
        } else if (curFrame < 17.0f) {
            this->headScale.x = this->headScale.z = ((curFrame - 6.0f) * 0.05f) + 0.8f;
            this->headScale.y = 0.8f;
        } else {
            this->headScale.x = this->headScale.z = 1.3f - ((curFrame - 16.0f) * 0.1f);
            this->headScale.y = ((curFrame - 16.0f) * 0.0666f) + 0.8f;
        }
    } else if (this->actionFunc == EnOkuta_WaitToShoot) {
        this->headScale.x = this->headScale.z = 1.0f;
        this->headScale.y = (sinf((M_PI / 16) * curFrame) * 0.2f) + 1.0f;
    } else {
        this->headScale.x = this->headScale.y = this->headScale.z = 1.0f;
    }
}

static void EnOkuta_ColliderCheck(EnOkuta* this, PlayState* play) {
    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_SetDropFlag_Oot(&this->actor, &this->collider.elem, 1);
        if ((this->actor.colChkInfo.damageEffect != 0) || (this->actor.colChkInfo.damage != 0)) {
            Enemy_StartFinishingBlow(play, &this->actor);
            this->actor.colChkInfo.health = 0;
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            if (this->actor.colChkInfo.damageEffect == 3) {
                EnOkuta_SetupFreeze(this);
            } else {
                EnOkuta_SetupWaitToDie(this);
            }
        }
    }
}

static void EnOkuta_Update(Actor* thisx, PlayState* play2) {
    EnOkuta* this = (EnOkuta*)thisx;
    PlayState* play = play2;
    Player* player = GET_PLAYER(play);
    WaterBox* outWaterBox;
    f32 ySurface;
    Vec3f sp38;
    s32 sp34;

    if (!(player->stateFlags1 &
          (PLAYER_STATE1_TALKING | PLAYER_STATE1_DEAD | PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000))) {
        if (this->actor.params == 0) {
            EnOkuta_ColliderCheck(this, play);
            if (!BgCheck_GetWaterSurfaceNoBgId(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                         &ySurface, &outWaterBox) ||
                (ySurface < this->actor.floorHeight)) {
                if (this->actor.colChkInfo.health != 0) {
                    Actor_Kill(&this->actor);
                    return;
                }
            } else {
                this->actor.home.pos.y = ySurface;
            }
        }
        this->actionFunc(this, play);
        if (this->actor.params == 0) {
            EnOkuta_UpdateHeadScale(this);
            this->collider.dim.height =
                (((sOctorockColliderInit.dim.height * this->headScale.y) - this->collider.dim.yShift) *
                 this->actor.scale.y * 100.0f);
        } else {
            sp34 = false;
            Actor_MoveWithGravity(&this->actor);
            Math_Vec3f_Copy(&sp38, &this->actor.world.pos);
            Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 15.0f, 30.0f, 0x405);
            if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
                SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.wallPoly, this->actor.wallBgId)) {
                sp34 = true;
                this->actor.bgCheckFlags &= ~8;
            }
            if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
                SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId)) {
                sp34 = true;
                this->actor.bgCheckFlags &= ~1;
            }
            if (sp34 && !(this->actor.bgCheckFlags & 9)) {
                Math_Vec3f_Copy(&this->actor.world.pos, &sp38);
            }
        }
        Collider_UpdateCylinder(&this->actor, &this->collider);
        if ((this->actionFunc == EnOkuta_Appear) || (this->actionFunc == EnOkuta_Hide)) {
            this->collider.dim.pos.y = this->actor.world.pos.y + (this->skelAnime.jointTable->y * this->actor.scale.y);
            this->collider.dim.radius = sOctorockColliderInit.dim.radius * this->actor.scale.x * 100.0f;
        }
        if (this->actor.params == 0x10) {
            this->actor.flags |= ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT;
            CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        }
        if (this->actionFunc != EnOkuta_WaitToAppear) {
            if ((this->actionFunc != EnOkuta_Die) && (this->actionFunc != EnOkuta_WaitToDie) &&
                (this->actionFunc != EnOkuta_Freeze)) {
                CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
            }
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
        }
        Actor_SetFocus(&this->actor, 15.0f);
        if ((this->actor.params == 0) && (this->actor.draw != NULL)) {
            EnOkuta_SpawnRipple(this, play);
        }
    }
}

static s32 EnOkuta_GetSnoutScale(EnOkuta* this, f32 curFrame, Vec3f* scale) {
    if (this->actionFunc == EnOkuta_WaitToShoot) {
        scale->x = scale->z = 1.0f;
        scale->y = (sinf((M_PI / 16) * curFrame) * 0.4f) + 1.0f;
    } else if (this->actionFunc == EnOkuta_Shoot) {
        if (curFrame < 5.0f) {
            scale->x = 1.0f;
            scale->y = scale->z = (curFrame * 0.25f) + 1.0f;
        } else if (curFrame < 7.0f) {
            scale->x = (curFrame - 4.0f) * 0.5f + 1.0f;
            scale->y = scale->z = 2.0f - (curFrame - 4.0f) * 0.5f;
        } else {
            scale->x = 2.0f - ((curFrame - 6.0f) * 0.0769f);
            scale->y = scale->z = 1.0f;
        }
    } else if (this->actionFunc == EnOkuta_Die) {
        if (curFrame >= 35.0f || curFrame < 25.0f) {
            return false;
        }
        if (curFrame < 27.0f) {
            scale->x = 1.0f;
            scale->y = scale->z = ((curFrame - 24.0f) * 0.5f) + 1.0f;
        } else if (curFrame < 30.0f) {
            scale->x = (curFrame - 26.0f) * 0.333f + 1.0f;
            scale->y = scale->z = 2.0f - (curFrame - 26.0f) * 0.333f;
        } else {
            scale->x = 2.0f - ((curFrame - 29.0f) * 0.2f);
            scale->y = scale->z = 1.0f;
        }
    } else {
        return false;
    }

    return true;
}

static s32 EnOkuta_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnOkuta* this = (EnOkuta*)thisx;
    f32 curFrame = this->skelAnime.curFrame;
    Vec3f scale;
    s32 doScale = false;

    if (this->actionFunc == EnOkuta_Die) {
        curFrame += this->timer;
    }
    if (limbIndex == 5) {
        if ((this->headScale.x != 1.0f) || (this->headScale.y != 1.0f) || (this->headScale.z != 1.0f)) {
            scale = this->headScale;
            doScale = true;
        }
    } else if (limbIndex == 8) {
        doScale = EnOkuta_GetSnoutScale(this, curFrame, &scale);
    }
    if (doScale) {
        Matrix_Scale(scale.x, scale.y, scale.z, MTXMODE_APPLY);
    }
    return false;
}

static void EnOkuta_Draw(Actor* thisx, PlayState* play) {
    EnOkuta* this = (EnOkuta*)thisx;
    s32 pad;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    if (this->actor.params == 0) {
        SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, EnOkuta_OverrideLimbDraw, NULL, this);
    } else {
        OPEN_DISPS(play->state.gfxCtx);

        Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
        Matrix_RotateZF(this->actor.home.rot.z * (M_PI / 0x8000), MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gOctorokProjectileDL);
        

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
