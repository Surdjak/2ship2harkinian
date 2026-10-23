/*
 * File: z_en_ishi.c
 * Overlay: ovl_En_Ishi
 * Description: Small and large gray rocks
 *
 * 3Ship [Hyrule] Ported from SoH. See hyrule/README.md for the porting conventions.
 */

#include "z_en_ishi.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "overlays/effects/ovl_Effect_Ss_Kakera/z_eff_ss_kakera.h"
#include "assets/oot/objects/gameplay_field_keep/gameplay_field_keep.h"

#define FLAGS ACTOR_FLAG_THROW_ONLY

static void EnIshi_Init(Actor* thisx, PlayState* play);
static void EnIshi_Destroy(Actor* thisx, PlayState* play);
static void EnIshi_Update(Actor* thisx, PlayState* play);
static void EnIshi_Draw(Actor* thisx, PlayState* play);

static void EnIshi_SetupWait(EnIshi* this);
static void EnIshi_Wait(EnIshi* this, PlayState* play);
static void EnIshi_SetupLiftedUp(EnIshi* this);
static void EnIshi_LiftedUp(EnIshi* this, PlayState* play);
static void EnIshi_SetupFly(EnIshi* this);
static void EnIshi_Fly(EnIshi* this, PlayState* play);
static void EnIshi_SpawnFragmentsSmall(EnIshi* this, PlayState* play);
static void EnIshi_SpawnFragmentsLarge(EnIshi* this, PlayState* play);
static void EnIshi_SpawnDustSmall(EnIshi* this, PlayState* play);
static void EnIshi_SpawnDustLarge(EnIshi* this, PlayState* play);

static s16 sRotSpeedX = 0;
static s16 sRotSpeedY = 0;

static void EnIshi_Reset(void);

ActorProfile En_Ishi_Oot_Profile = {
    /**/ ACTOR_EN_ISHI_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_GAMEPLAY_FIELD_KEEP_Oot,
    /**/ sizeof(EnIshi),
    /**/ EnIshi_Init,
    /**/ EnIshi_Destroy,
    /**/ EnIshi_Update,
    /**/ EnIshi_Draw,
    /**/ EnIshi_Reset,
};

static f32 sRockScales[] = { 0.1f, 0.4f };
static f32 D_80A7FA20[] = { 58.0f, 80.0f };
static f32 D_80A7FA28[] = { 0.0f, 0.005f };

// the sizes of these arrays are very large and take up way more space than it needs to.
// coincidentally the sizes are the same as the ID for NA_SE_EV_ROCK_BROKEN, which may explain a mistake that could
// have been made here
static u16 sBreakSounds[] = { NA_SE_EV_ROCK_BROKEN, NA_SE_EV_WALL_BROKEN };

static u8 sBreakSoundDurations[] = { 20, 40 };

static EnIshiEffectSpawnFunc sFragmentSpawnFuncs[] = { EnIshi_SpawnFragmentsSmall, EnIshi_SpawnFragmentsLarge };

static EnIshiEffectSpawnFunc sDustSpawnFuncs[] = { EnIshi_SpawnDustSmall, EnIshi_SpawnDustLarge };

static ColliderCylinderInit sCylinderInits[] = {
    {
        {
            COL_MATERIAL_HARD,
            AT_NONE,
            AC_ON | AC_HARD | AC_TYPE_PLAYER,
            OC1_ON | OC1_TYPE_ALL,
            OC2_TYPE_2,
            COLSHAPE_CYLINDER,
        },
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x4FC1FFFE), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON,
            OCELEM_ON,
        },
        { 10, 18, -2, { 0, 0, 0 } },
    },
    {
        {
            COL_MATERIAL_HARD,
            AT_NONE,
            AC_ON | AC_HARD | AC_TYPE_PLAYER,
            OC1_ON | OC1_TYPE_ALL,
            OC2_TYPE_2,
            COLSHAPE_CYLINDER,
        },
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x4FC1FFF6), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON,
            OCELEM_ON,
        },
        { 55, 70, 0, { 0, 0, 0 } },
    },
};

static CollisionCheckInfoInit sColChkInfoInit = { 0, 12, 60, MASS_IMMOVABLE };

static void EnIshi_InitCollider(Actor* thisx, PlayState* play) {
    EnIshi* this = (EnIshi*)thisx;

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInits[this->actor.params & 1]);
    Collider_UpdateCylinder(&this->actor, &this->collider);
}

static s32 EnIshi_SnapToFloor(EnIshi* this, PlayState* play, f32 arg2) {
    CollisionPoly* poly;
    Vec3f pos;
    s32 bgId;
    f32 floorY;

    pos.x = this->actor.world.pos.x;
    pos.y = this->actor.world.pos.y + 30.0f;
    pos.z = this->actor.world.pos.z;
    floorY = BgCheck_EntityRaycastFloor5(&play->colCtx, &poly, &bgId, &this->actor, &pos);
    if (floorY > BGCHECK_Y_MIN) {
        this->actor.world.pos.y = floorY + arg2;
        Math_Vec3f_Copy(&this->actor.home.pos, &this->actor.world.pos);
        return true;
    } else {
        return false;
    }
}

static void EnIshi_SpawnFragmentsSmall(EnIshi* this, PlayState* play) {
    static s16 scales[] = { 16, 13, 11, 9, 7, 5 };
    s32 pad;
    Vec3f velocity;
    Vec3f pos;
    s16 phi_v0;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(scales); i++) {
        pos.x = this->actor.world.pos.x + (Rand_ZeroOne() - 0.5f) * 8.0f;
        pos.y = this->actor.world.pos.y + (Rand_ZeroOne() * 5.0f) + 5.0f;
        pos.z = this->actor.world.pos.z + (Rand_ZeroOne() - 0.5f) * 8.0f;
        Math_Vec3f_Copy(&velocity, &this->actor.velocity);
        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            velocity.x *= 0.8f;
            velocity.y *= -0.8f;
            velocity.z *= 0.8f;
        } else if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
            velocity.x *= -0.8f;
            velocity.y *= 0.8f;
            velocity.z *= -0.8f;
        }
        velocity.x += (Rand_ZeroOne() - 0.5f) * 11.0f;
        velocity.y += Rand_ZeroOne() * 6.0f;
        velocity.z += (Rand_ZeroOne() - 0.5f) * 11.0f;
        if (Rand_ZeroOne() < 0.5f) {
            phi_v0 = 65;
        } else {
            phi_v0 = 33;
        }
        EffectSsKakera_Spawn(play, &pos, &velocity, &pos, -420, phi_v0, 30, 5, 0, scales[i], 3, 10, 40,
                             KAKERA_COLOR_NONE, OBJECT_GAMEPLAY_FIELD_KEEP_Oot, gFieldKakeraDL);
    }
}

static void EnIshi_SpawnFragmentsLarge(EnIshi* this, PlayState* play) {
    static s16 scales[] = { 145, 135, 120, 100, 70, 50, 45, 40, 35 };
    Actor* thisx = &this->actor;
    Vec3f velocity;
    Vec3f pos;
    s16 angle = 0x1000;
    s32 i;
    f32 rand;
    s16 phi_v0;
    s16 phi_v1;

    for (i = 0; i < ARRAY_COUNT(scales); i++) {
        angle += 0x4E20;
        rand = Rand_ZeroOne() * 10.0f;
        pos.x = this->actor.world.pos.x + (Math_SinS(angle) * rand);
        pos.y = this->actor.world.pos.y + (Rand_ZeroOne() * 40.0f) + 5.0f;
        pos.z = this->actor.world.pos.z + (Math_CosS(angle) * rand);
        Math_Vec3f_Copy(&velocity, &thisx->velocity);
        if (thisx->bgCheckFlags & 1) {
            velocity.x *= 0.9f;
            velocity.y *= -0.8f;
            velocity.z *= 0.9f;
        } else if (thisx->bgCheckFlags & 8) {
            velocity.x *= -0.9f;
            velocity.y *= 0.8f;
            velocity.z *= -0.9f;
        }
        rand = Rand_ZeroOne() * 10.0f;
        velocity.x += rand * Math_SinS(angle);
        velocity.y += (Rand_ZeroOne() * 4.0f) + ((Rand_ZeroOne() * i) * 0.7f);
        velocity.z += rand * Math_CosS(angle);
        if (i == 0) {
            phi_v0 = 41;
            phi_v1 = -450;
        } else if (i < 4) {
            phi_v0 = 37;
            phi_v1 = -380;
        } else {
            phi_v0 = 69;
            phi_v1 = -320;
        }
        EffectSsKakera_Spawn(play, &pos, &velocity, &this->actor.world.pos, phi_v1, phi_v0, 30, 5, 0, scales[i], 5, 2,
                             70, KAKERA_COLOR_WHITE, OBJECT_GAMEPLAY_FIELD_KEEP_Oot, gSilverRockFragmentsDL);
    }
}

static void EnIshi_SpawnDustSmall(EnIshi* this, PlayState* play) {
    Vec3f pos;

    Math_Vec3f_Copy(&pos, &this->actor.world.pos);
    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        pos.x += 2.0f * this->actor.velocity.x;
        pos.y -= 2.0f * this->actor.velocity.y;
        pos.z += 2.0f * this->actor.velocity.z;
    } else if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        pos.x -= 2.0f * this->actor.velocity.x;
        pos.y += 2.0f * this->actor.velocity.y;
        pos.z -= 2.0f * this->actor.velocity.z;
    }
    func_800BBFB0(play, &pos, 60.0f, 3, 0x50, 0x3C, 1);
}

static void EnIshi_SpawnDustLarge(EnIshi* this, PlayState* play) {
    Vec3f pos;

    Math_Vec3f_Copy(&pos, &this->actor.world.pos);
    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        pos.x += 2.0f * this->actor.velocity.x;
        pos.y -= 2.0f * this->actor.velocity.y;
        pos.z += 2.0f * this->actor.velocity.z;
    } else if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        pos.x -= 2.0f * this->actor.velocity.x;
        pos.y += 2.0f * this->actor.velocity.y;
        pos.z -= 2.0f * this->actor.velocity.z;
    }
    func_800BBFB0(play, &pos, 140.0f, 0xA, 0xB4, 0x5A, 1);
}

static void EnIshi_DropCollectible(EnIshi* this, PlayState* play) {
    s16 dropParams;

    if ((this->actor.params & 1) == ROCK_SMALL) {
        dropParams = (this->actor.params >> 8) & 0xF;

        if (dropParams >= 0xD) {
            dropParams = 0;
        }

        // PORT: OoT's drop tables are not ported, so this draws from MM's table with the same index.
        Item_DropCollectibleRandom(play, NULL, &this->actor.world.pos, dropParams << 4);
    }
}

static void EnIshi_Fall(EnIshi* this) {
    this->actor.velocity.y += this->actor.gravity;

    if (this->actor.velocity.y < this->actor.terminalVelocity) {
        this->actor.velocity.y = this->actor.terminalVelocity;
    }
}

static void func_80A7ED94(Vec3f* arg0, f32 arg1) {
    arg1 += ((Rand_ZeroOne() * 0.2f) - 0.1f) * arg1;
    arg0->x -= arg0->x * arg1;
    arg0->y -= arg0->y * arg1;
    arg0->z -= arg0->z * arg1;
}

static void EnIshi_SpawnBugs(EnIshi* this, PlayState* play) {
    s32 i;

    for (i = 0; i < 3; i++) {
        Actor* bug = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_INSECT_Oot, this->actor.world.pos.x,
                                 this->actor.world.pos.y, this->actor.world.pos.z, 0, Rand_ZeroOne() * 0xFFFF, 0, 1);

        if (bug == NULL) {
            break;
        }
    }
}

static InitChainEntry sInitChains[][5] = {
    {
        ICHAIN_F32_DIV1000(gravity, -1200, ICHAIN_CONTINUE),
        ICHAIN_F32_DIV1000(terminalVelocity, -20000, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeDistance, 1200, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeScale, 150, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeDownward, 400, ICHAIN_STOP),
    },
    {
        ICHAIN_F32_DIV1000(gravity, -2500, ICHAIN_CONTINUE),
        ICHAIN_F32_DIV1000(terminalVelocity, -20000, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeDistance, 2000, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeScale, 250, ICHAIN_CONTINUE),
        ICHAIN_F32(cullingVolumeDownward, 500, ICHAIN_STOP),
    },
};

static void EnIshi_Init(Actor* thisx, PlayState* play) {
    EnIshi* this = (EnIshi*)thisx;
    s16 type = this->actor.params & 1;

    Actor_ProcessInitChain(&this->actor, sInitChains[type]);
    if (play->csCtx.state != CS_STATE_IDLE) {
        this->actor.cullingVolumeDistance += 1000.0f;
    }
    if (this->actor.shape.rot.y == 0) {
        this->actor.shape.rot.y = this->actor.world.rot.y = Rand_ZeroFloat(0x10000);
    }
    Actor_SetScale(&this->actor, sRockScales[type]);
    EnIshi_InitCollider(&this->actor, play);
    if ((type == ROCK_LARGE) &&
        Flags_GetSwitch(play, ((this->actor.params >> 0xA) & 0x3C) | ((this->actor.params >> 6) & 3))) {
        Actor_Kill(&this->actor);
        return;
    }
    CollisionCheck_SetInfo(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
    this->actor.shape.yOffset = D_80A7FA20[type];
    if (!((this->actor.params >> 5) & 1) && !EnIshi_SnapToFloor(this, play, 0.0f)) {
        Actor_Kill(&this->actor);
        return;
    }
    EnIshi_SetupWait(this);
}

static void EnIshi_Destroy(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnIshi* this = (EnIshi*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnIshi_SetupWait(EnIshi* this) {
    this->actionFunc = EnIshi_Wait;
}

static void EnIshi_Wait(EnIshi* this, PlayState* play) {
    static u16 liftSounds[] = { NA_SE_PL_PULL_UP_ROCK, NA_SE_PL_PULL_UP_BIGROCK };
    s32 pad;
    s16 type = this->actor.params & 1;

    if (Actor_HasParent(&this->actor, play)) {
        EnIshi_SetupLiftedUp(this);
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 20, liftSounds[type]);
        if ((this->actor.params >> 4) & 1) {
            EnIshi_SpawnBugs(this, play);
        }
    } else if ((this->collider.base.acFlags & AC_HIT) && (type == ROCK_SMALL) &&
               (this->collider.elem.acHitElem->atDmgInfo.dmgFlags & DMG_FROM_OOT(0x40000048))) {
        EnIshi_DropCollectible(this, play);
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, sBreakSoundDurations[type],
                                           sBreakSounds[type]);
        sFragmentSpawnFuncs[type](this, play);
        sDustSpawnFuncs[type](this, play);
        Actor_Kill(&this->actor);
    } else if (this->actor.xzDistToPlayer < 600.0f) {
        Collider_UpdateCylinder(&this->actor, &this->collider);
        this->collider.base.acFlags &= ~AC_HIT;
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        if (this->actor.xzDistToPlayer < 400.0f) {
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            if (this->actor.xzDistToPlayer < 90.0f) {
                // GI_NONE in these cases allows the player to lift the actor
                if (type == ROCK_LARGE) {
                    Actor_OfferGetItem(&this->actor, play, GI_NONE, 80.0f, 20.0f);
                } else {
                    Actor_OfferGetItem(&this->actor, play, GI_NONE, 50.0f, 10.0f);
                }
            }
        }
    }
}

static void EnIshi_SetupLiftedUp(EnIshi* this) {
    this->actionFunc = EnIshi_LiftedUp;
    this->actor.room = -1;
    this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
}

static void EnIshi_LiftedUp(EnIshi* this, PlayState* play) {
    if (Actor_HasNoParent(&this->actor, play)) {
        this->actor.room = play->roomCtx.curRoom.num;
        if ((this->actor.params & 1) == ROCK_LARGE) {
            Flags_SetSwitch(play, ((this->actor.params >> 0xA) & 0x3C) | ((this->actor.params >> 6) & 3));
        }
        EnIshi_SetupFly(this);
        EnIshi_Fall(this);
        func_80A7ED94(&this->actor.velocity, D_80A7FA28[this->actor.params & 1]);
        Actor_UpdatePos(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 7.5f, 35.0f, 0.0f, 0xC5);
    }
}

static void EnIshi_SetupFly(EnIshi* this) {
    this->actor.velocity.x = Math_SinS(this->actor.world.rot.y) * this->actor.speed;
    this->actor.velocity.z = Math_CosS(this->actor.world.rot.y) * this->actor.speed;
    if ((this->actor.params & 1) == ROCK_SMALL) {
        sRotSpeedX = (Rand_ZeroOne() - 0.5f) * 16000.0f;
        sRotSpeedY = (Rand_ZeroOne() - 0.5f) * 2400.0f;
    } else {
        sRotSpeedX = (Rand_ZeroOne() - 0.5f) * 8000.0f;
        sRotSpeedY = (Rand_ZeroOne() - 0.5f) * 1600.0f;
    }
    this->actor.colChkInfo.mass = 240;
    this->actionFunc = EnIshi_Fly;
}

static void EnIshi_Fly(EnIshi* this, PlayState* play) {
    s32 pad;
    s16 type = this->actor.params & 1;
    s32 pad2;
    s32 quakeIdx;
    Vec3f contactPos;

    if (this->actor.bgCheckFlags & 9) {
        EnIshi_DropCollectible(this, play);
        sFragmentSpawnFuncs[type](this, play);
        if (!(this->actor.bgCheckFlags & 0x20)) {
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, sBreakSoundDurations[type],
                                               sBreakSounds[type]);
            sDustSpawnFuncs[type](this, play);
        }
        if (type == ROCK_LARGE) {
            quakeIdx = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);
            Quake_SetSpeed(quakeIdx, -0x3CB0);
            Quake_SetPerturbations(quakeIdx, 3, 0, 0, 0);
            Quake_SetDuration(quakeIdx, 7);
            Rumble_Request(this->actor.xyzDistToPlayerSq, 0xFF, 0x14, 0x96);
        }
        Actor_Kill(&this->actor);
        return;
    }
    if (this->actor.bgCheckFlags & 0x40) {
        contactPos.x = this->actor.world.pos.x;
        contactPos.y = this->actor.world.pos.y + this->actor.depthInWater;
        contactPos.z = this->actor.world.pos.z;
        EffectSsGSplash_Spawn(play, &contactPos, 0, 0, 0, 350);
        if (type == ROCK_SMALL) {
            EffectSsGRipple_Spawn(play, &contactPos, 150, 650, 0);
            EffectSsGRipple_Spawn(play, &contactPos, 400, 800, 4);
            EffectSsGRipple_Spawn(play, &contactPos, 500, 1100, 8);
        } else {
            EffectSsGRipple_Spawn(play, &contactPos, 300, 700, 0);
            EffectSsGRipple_Spawn(play, &contactPos, 500, 900, 4);
            EffectSsGRipple_Spawn(play, &contactPos, 500, 1300, 8);
        }
        this->actor.terminalVelocity = -6.0f;
        sRotSpeedX >>= 2;
        sRotSpeedY >>= 2;
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 40, NA_SE_EV_DIVE_INTO_WATER_L);
        this->actor.bgCheckFlags &= ~0x40;
    }
    Math_StepToF(&this->actor.shape.yOffset, 0.0f, 2.0f);
    EnIshi_Fall(this);
    func_80A7ED94(&this->actor.velocity, D_80A7FA28[type]);
    Actor_UpdatePos(&this->actor);
    this->actor.shape.rot.x += sRotSpeedX;
    this->actor.shape.rot.y += sRotSpeedY;
    Actor_UpdateBgCheckInfo(play, &this->actor, 7.5f, 35.0f, 0.0f, 0xC5);
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static void EnIshi_Update(Actor* thisx, PlayState* play) {
    EnIshi* this = (EnIshi*)thisx;

    this->actionFunc(this, play);
}

static void EnIshi_DrawSmall(EnIshi* this, PlayState* play) {
    Gfx_DrawDListOpa(play, gFieldKakeraDL);
}

static void EnIshi_DrawLarge(EnIshi* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
    gSPDisplayList(POLY_OPA_DISP++, gSilverRockDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

static EnIshiDrawFunc sDrawFuncs[] = { EnIshi_DrawSmall, EnIshi_DrawLarge };

static void EnIshi_Draw(Actor* thisx, PlayState* play) {
    EnIshi* this = (EnIshi*)thisx;

    sDrawFuncs[this->actor.params & 1](this, play);
}

static void EnIshi_Reset(void) {
    sRotSpeedX = 0;
    sRotSpeedY = 0;
}