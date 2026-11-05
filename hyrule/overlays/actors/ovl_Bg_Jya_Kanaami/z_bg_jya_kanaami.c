/*
 * File: z_bg_jya_kanaami.c
 * Overlay: ovl_Bg_Jya_Kanaami
 * Description: Climbable grating/bridge (Spirit Temple)
 */

#include "z_bg_jya_kanaami.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64quake.h"
#include "assets/oot/objects/object_jya_obj/object_jya_obj.h"

#define FLAGS 0

static void BgJyaKanaami_Init(Actor* thisx, PlayState* play);
static void BgJyaKanaami_Destroy(Actor* thisx, PlayState* play);
static void BgJyaKanaami_Update(Actor* thisx, PlayState* play);
static void BgJyaKanaami_Draw(Actor* thisx, PlayState* play);

static void func_80899880(BgJyaKanaami* this);
static void func_80899894(BgJyaKanaami* this, PlayState* play);
static void func_8089993C(BgJyaKanaami* this);
static void func_80899950(BgJyaKanaami* this, PlayState* play);
static void func_80899A08(BgJyaKanaami* this);

ActorProfile Bg_Jya_Kanaami_Oot_Profile = {
    /**/ ACTOR_BG_JYA_KANAAMI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_JYA_OBJ_Oot,
    /**/ sizeof(BgJyaKanaami),
    /**/ BgJyaKanaami_Init,
    /**/ BgJyaKanaami_Destroy,
    /**/ BgJyaKanaami_Update,
    /**/ BgJyaKanaami_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 700, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1000, ICHAIN_STOP),
};

static void BgJyaKanaami_InitDynaPoly(BgJyaKanaami* this, PlayState* play, CollisionHeader* collision, s32 flag) {
    s32 pad;
    CollisionHeader* colHeader = NULL;
    s32 pad2;

    DynaPolyActor_Init(&this->dyna, flag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgJyaKanaami_Init(Actor* thisx, PlayState* play) {
    BgJyaKanaami* this = (BgJyaKanaami*)thisx;

    BgJyaKanaami_InitDynaPoly(this, play, &gKanaamiCol, 0);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    if (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
        func_80899A08(this);
    } else {
        func_80899880(this);
    }
}

static void BgJyaKanaami_Destroy(Actor* thisx, PlayState* play) {
    BgJyaKanaami* this = (BgJyaKanaami*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_80899880(BgJyaKanaami* this) {
    this->actionFunc = func_80899894;
    this->unk_16A = 0;
}

static void func_80899894(BgJyaKanaami* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F) || this->unk_16A > 0) {
        if (this->dyna.actor.world.pos.x > -1000.0f && this->unk_16A == 0) {
            OnePointCutscene_Init(play, 3450, -99, &this->dyna.actor, CAM_ID_MAIN);
        }
        this->unk_16A += 1;
        if (this->unk_16A >= 0xA) {
            func_8089993C(this);
        }
    }
}

static void func_8089993C(BgJyaKanaami* this) {
    this->actionFunc = func_80899950;
    this->unk_168 = 0;
}

static void func_80899950(BgJyaKanaami* this, PlayState* play) {
    s32 pad[2];
    s32 quakeId;

    this->unk_168 += 0x20;
    if (Math_ScaledStepToS(&this->dyna.actor.world.rot.x, 0x4000, this->unk_168)) {
        func_80899A08(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_TRAP_BOUND);
        quakeId = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);
        Quake_SetSpeed(quakeId, 25000);
        Quake_SetPerturbations(quakeId, 2, 0, 0, 0);
        Quake_SetDuration(quakeId, 16);
    }
}

static void func_80899A08(BgJyaKanaami* this) {
    this->actionFunc = 0;
    this->dyna.actor.world.rot.x = 0x4000;
}

static void BgJyaKanaami_Update(Actor* thisx, PlayState* play) {
    BgJyaKanaami* this = (BgJyaKanaami*)thisx;

    if (this->actionFunc != NULL) {
        this->actionFunc(this, play);
    }
    this->dyna.actor.shape.rot.x = this->dyna.actor.world.rot.x;
}

static void BgJyaKanaami_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, gKanaamiDL);
}
