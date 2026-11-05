/*
 * File: z_bg_jya_bombiwa.c
 * Overlay: ovl_Bg_Jya_Bombiwa
 * Description: Spirit Temple top room bombable wall
 */

#include "z_bg_jya_bombiwa.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "overlays/effects/ovl_Effect_Ss_Kakera/z_eff_ss_kakera.h"
#include "assets/oot/objects/object_jya_obj/object_jya_obj.h"

#define FLAGS 0

static void BgJyaBombiwa_Init(Actor* thisx, PlayState* play);
static void BgJyaBombiwa_Destroy(Actor* thisx, PlayState* play);
static void BgJyaBombiwa_Update(Actor* thisx, PlayState* play);
static void BgJyaBombiwa_Draw(Actor* thisx, PlayState* play);

ActorProfile Bg_Jya_Bombiwa_Oot_Profile = {
    /**/ ACTOR_BG_JYA_BOMBIWA_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_JYA_OBJ_Oot,
    /**/ sizeof(BgJyaBombiwa),
    /**/ BgJyaBombiwa_Init,
    /**/ BgJyaBombiwa_Destroy,
    /**/ BgJyaBombiwa_Update,
    /**/ BgJyaBombiwa_Draw,
};

static ColliderJntSphElementInit sJntSphElementsInit[] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000008), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON,
            OCELEM_NONE,
        },
        { 0, { { 0, 0, 0 }, 50 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1000, ICHAIN_STOP),
};

static void BgJyaBombiwa_SetupDynaPoly(BgJyaBombiwa* this, PlayState* play, CollisionHeader* collision, s32 flag) {
    s16 pad1;
    CollisionHeader* colHeader = NULL;
    s16 pad2;

    DynaPolyActor_Init(&this->dyna, flag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    if (this->dyna.bgId == BG_ACTOR_MAX) {

    }
}

static void BgJyaBombiwa_InitCollider(BgJyaBombiwa* this, PlayState* play) {
    s32 pad;

    Collider_InitJntSph(play, &this->collider);
    Collider_SetJntSph(play, &this->collider, &this->dyna.actor, &sJntSphInit, this->colliderItems);
}

static void BgJyaBombiwa_Init(Actor* thisx, PlayState* play) {
    BgJyaBombiwa* this = (BgJyaBombiwa*)thisx;

    if ((this->dyna.actor.params & 0x3F) != 0x29) {

    }
    BgJyaBombiwa_SetupDynaPoly(this, play, &gBombiwaCol, 0);
    BgJyaBombiwa_InitCollider(this, play);
    if (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
        Actor_Kill(&this->dyna.actor);
    } else {
        Actor_ProcessInitChain(&this->dyna.actor, sInitChain);

    }
}

static void BgJyaBombiwa_Destroy(Actor* thisx, PlayState* play) {
    BgJyaBombiwa* this = (BgJyaBombiwa*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    Collider_DestroyJntSph(play, &this->collider);
}

static void BgJyaBombiwa_Break(BgJyaBombiwa* this, PlayState* play) {
    Vec3f pos;
    Vec3f velocity;
    s16 arg5;
    s8 arg6;
    s8 arg7;
    s32 i;
    s16 scale;

    for (i = 0; i < 16; i++) {
        pos.x = ((Rand_ZeroOne() * 80.0f) + this->dyna.actor.world.pos.x) - 40.0f;
        pos.y = (Rand_ZeroOne() * 140.0f) + this->dyna.actor.world.pos.y;
        pos.z = ((Rand_ZeroOne() * 80.0f) + this->dyna.actor.world.pos.z) - 40.0f;
        velocity.x = (Rand_ZeroOne() - 0.5f) * 10.0f;
        velocity.y = Rand_ZeroOne() * 12.0f;
        velocity.z = (Rand_ZeroOne() - 0.5f) * 10.0f;
        scale = (s32)(i * 1.8f) + 3;
        if (scale > 15) {
            arg5 = 5;
        } else {
            arg5 = 1;
        }
        if (Rand_ZeroOne() < 0.4f) {
            arg5 |= 0x40;
            arg6 = 0xC;
            arg7 = 8;
        } else {
            arg5 |= 0x20;
            arg6 = 0xC;
            arg7 = 8;
            if (scale < 10) {
                arg6 = 0x50;
                arg7 = 80;
            }
        }
        EffectSsKakera_Spawn(play, &pos, &velocity, &pos, -400, arg5, arg6, arg7, 0, scale, 1, 20, 80,
                             KAKERA_COLOR_NONE, OBJECT_JYA_OBJ_Oot, gBombiwaEffectDL);
    }
    pos.x = this->dyna.actor.world.pos.x;
    pos.y = this->dyna.actor.world.pos.y + 70.0f;
    pos.z = this->dyna.actor.world.pos.z;
    func_800BBFB0(play, &pos, 100.0f, 0xA, 0x64, 0xA0, 1);
}

static void BgJyaBombiwa_Update(Actor* thisx, PlayState* play) {
    BgJyaBombiwa* this = (BgJyaBombiwa*)thisx;

    if (this->collider.base.acFlags & AC_HIT) {
        BgJyaBombiwa_Break(this, play);
        Flags_SetSwitch(play, this->dyna.actor.params & 0x3F);
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->dyna.actor.world.pos, 40, NA_SE_EV_WALL_BROKEN);
        Actor_Kill(&this->dyna.actor);
    } else {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void BgJyaBombiwa_Draw(Actor* thisx, PlayState* play) {
    BgJyaBombiwa* this = (BgJyaBombiwa*)thisx;

    Gfx_DrawDListOpa(play, gBombiwaDL);
    Collider_UpdateSpheres(0, &this->collider);
}
