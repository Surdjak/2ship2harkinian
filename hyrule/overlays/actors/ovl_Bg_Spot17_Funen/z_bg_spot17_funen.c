/*
 * File: z_bg_spot17_funen
 * Overlay: ovl_Bg_Spot17_Funen
 * Description: Crater Smoke Cone
 */

#include "z_bg_spot17_funen.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_spot17_obj/object_spot17_obj.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgSpot17Funen_Init(Actor* thisx, PlayState* play);
static void BgSpot17Funen_Destroy(Actor* thisx, PlayState* play);
static void BgSpot17Funen_Update(Actor* thisx, PlayState* play);
static void func_808B746C(Actor* thisx, PlayState* play);
static void func_808B7478(Actor* thisx, PlayState* play);

ActorProfile Bg_Spot17_Funen_Oot_Profile = {
    /**/ ACTOR_BG_SPOT17_FUNEN_Oot,
    /**/ ACTORCAT_SWITCH,
    /**/ FLAGS,
    /**/ OBJECT_SPOT17_OBJ_Oot,
    /**/ sizeof(BgSpot17Funen),
    /**/ BgSpot17Funen_Init,
    /**/ BgSpot17Funen_Destroy,
    /**/ BgSpot17Funen_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgSpot17Funen_Init(Actor* thisx, PlayState* play) {
    BgSpot17Funen* this = (BgSpot17Funen*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
}

static void BgSpot17Funen_Destroy(Actor* thisx, PlayState* play) {
}

static void BgSpot17Funen_Update(Actor* thisx, PlayState* play) {
    BgSpot17Funen* this = (BgSpot17Funen*)thisx;

    this->actor.draw = func_808B7478;
    this->actor.update = func_808B746C;
}

static void func_808B746C(Actor* thisx, PlayState* play) {
}

static void func_808B7478(Actor* thisx, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    Matrix_RotateYF((s16)(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) - thisx->shape.rot.y + 0x8000) * 9.58738019108e-05f, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (0 - play->gameplayFrames) & 0x7F, 0x20, 0x20, 1, 0,
                                  (0 - play->gameplayFrames) & 0x7F, 0x20, 0x20, 0, -1, 0, -1));
    gSPDisplayList(POLY_XLU_DISP++, gCraterSmokeConeDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
