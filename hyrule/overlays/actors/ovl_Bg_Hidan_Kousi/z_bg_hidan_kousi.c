/*
 * File: z_bg_hidan_kousi.c
 * Overlay: ovl_Bg_Hidan_Kousi
 * Description:
 */

#include "z_bg_hidan_kousi.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_hidan_objects/object_hidan_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgHidanKousi_Init(Actor* thisx, PlayState* play);
static void BgHidanKousi_Destroy(Actor* thisx, PlayState* play);
static void BgHidanKousi_Update(Actor* thisx, PlayState* play);
static void BgHidanKousi_Draw(Actor* thisx, PlayState* play);

static void func_80889ACC(BgHidanKousi* this);
static void func_80889B5C(BgHidanKousi* this, PlayState* play);
static void func_80889BC0(BgHidanKousi* this, PlayState* play);
static void func_80889C18(BgHidanKousi* this, PlayState* play);
static void func_80889C90(BgHidanKousi* this, PlayState* play);
static void func_80889D28(BgHidanKousi* this, PlayState* play);

static f32 D_80889E40[] = { 120.0f, 150.0f, 150.0f };

ActorProfile Bg_Hidan_Kousi_Oot_Profile = {
    /**/ ACTOR_BG_HIDAN_KOUSI_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_HIDAN_OBJECTS_Oot,
    /**/ sizeof(BgHidanKousi),
    /**/ BgHidanKousi_Init,
    /**/ BgHidanKousi_Destroy,
    /**/ BgHidanKousi_Update,
    /**/ BgHidanKousi_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static CollisionHeader* sMetalFencesCollisions[] = {
    &gFireTempleMetalFenceWithSlantCol,
    &gFireTempleMetalFenceCol,
    &gFireTempleMetalFence2Col,
};

static s16 D_80889E7C[] = {
    0x4000,
    0xC000,
    0xC000,
    0x0000,
};

static Gfx* sMetalFencesDLs[] = {
    gFireTempleMetalFenceWithSlantDL,
    gFireTempleMetalFenceDL,
    gFireTempleMetalFence2DL,
};

static void BgHidanKousi_SetupAction(BgHidanKousi* this, BgHidanKousiActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgHidanKousi_Init(Actor* thisx, PlayState* play) {
    BgHidanKousi* this = (BgHidanKousi*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    Actor_SetFocus(thisx, 50.0f);

    Actor_ProcessInitChain(thisx, sInitChain);
    if (((thisx->params & 0xFF) < 0) || ((thisx->params & 0xFF) >= 3)) {
    }

    CollisionHeader_GetVirtual(sMetalFencesCollisions[thisx->params & 0xFF], &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    thisx->world.rot.y = D_80889E7C[this->dyna.actor.params & 0xFF] + thisx->shape.rot.y;
    if (Flags_GetSwitch(play, (thisx->params >> 8) & 0xFF)) {
        func_80889ACC(this);
        BgHidanKousi_SetupAction(this, func_80889D28);
    } else {
        BgHidanKousi_SetupAction(this, func_80889B5C);
    }
}

static void BgHidanKousi_Destroy(Actor* thisx, PlayState* play) {
    BgHidanKousi* this = (BgHidanKousi*)thisx;
    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_80889ACC(BgHidanKousi* this) {
    s32 pad[2];
    Vec3s* rot = &this->dyna.actor.world.rot;
    f32 temp1 = D_80889E40[this->dyna.actor.params & 0xFF] * Math_SinS(rot->y);
    f32 temp2 = D_80889E40[this->dyna.actor.params & 0xFF] * Math_CosS(rot->y);

    this->dyna.actor.world.pos.x = this->dyna.actor.home.pos.x + temp1;
    this->dyna.actor.world.pos.z = this->dyna.actor.home.pos.z + temp2;
}

static void func_80889B5C(BgHidanKousi* this, PlayState* play) {
    if (Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0xFF)) {
        BgHidanKousi_SetupAction(this, func_80889BC0);
        OnePointCutscene_Attention(play, &this->dyna.actor);
        this->unk_168 = 0xC8;
    }
}

static void func_80889BC0(BgHidanKousi* this, PlayState* play) {
    this->unk_168 -= 1;
    if (this->dyna.actor.category == Hyrule_GetAttentionCategoryOot() || (this->unk_168 <= 0)) {
        BgHidanKousi_SetupAction(this, func_80889C18);
    }
}

static void func_80889C18(BgHidanKousi* this, PlayState* play) {
    this->dyna.actor.speed += 0.2f;
    if (this->dyna.actor.speed > 2.0f) {
        this->dyna.actor.speed = 2.0f;
        BgHidanKousi_SetupAction(this, func_80889C90);
    }
    Actor_MoveWithGravity(&this->dyna.actor);
    Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
}

static void func_80889C90(BgHidanKousi* this, PlayState* play) {
    Actor_UpdatePos(&this->dyna.actor);
    if (D_80889E40[this->dyna.actor.params & 0xFF] <
        Math_Vec3f_DistXYZ(&this->dyna.actor.home.pos, &this->dyna.actor.world.pos)) {
        func_80889ACC(this);
        BgHidanKousi_SetupAction(this, func_80889D28);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_METALDOOR_STOP);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
    }
}

static void func_80889D28(BgHidanKousi* this, PlayState* play) {
}

static void BgHidanKousi_Update(Actor* thisx, PlayState* play) {
    BgHidanKousi* this = (BgHidanKousi*)thisx;

    this->actionFunc(this, play);
}

static void BgHidanKousi_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sMetalFencesDLs[thisx->params & 0xFF]);

    CLOSE_DISPS(play->state.gfxCtx);
}
