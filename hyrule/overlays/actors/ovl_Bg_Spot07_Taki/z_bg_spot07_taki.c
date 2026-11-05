/*
 * File: z_bg_spot07_taki.c
 * Overlay: ovl_Bg_Spot07_Taki
 * Description: Zora's Domain Waterfall and Ice
 */

#include "z_bg_spot07_taki.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_spot07_object/object_spot07_object.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgSpot07Taki_Init(Actor* thisx, PlayState* play);
static void BgSpot07Taki_Destroy(Actor* thisx, PlayState* play);
static void BgSpot07Taki_Update(Actor* thisx, PlayState* play);
static void BgSpot07Taki_Draw(Actor* thisx, PlayState* play);

static void BgSpot07Taki_DoNothing(BgSpot07Taki* this, PlayState* play);

ActorProfile Bg_Spot07_Taki_Oot_Profile = {
    /**/ ACTOR_BG_SPOT07_TAKI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SPOT07_OBJECT_Oot,
    /**/ sizeof(BgSpot07Taki),
    /**/ BgSpot07Taki_Init,
    /**/ BgSpot07Taki_Destroy,
    /**/ BgSpot07Taki_Update,
    /**/ BgSpot07Taki_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgSpot07Taki_Init(Actor* thisx, PlayState* play) {
    BgSpot07Taki* this = (BgSpot07Taki*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    if (LINK_IS_ADULT_OOT) {
        if (this->dyna.actor.params == 0) {
            CollisionHeader_GetVirtual(&object_spot07_object_Col_002590, &colHeader);
        } else {
            CollisionHeader_GetVirtual(&object_spot07_object_Col_0038FC, &colHeader);
        }
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    }
    this->actionFunc = BgSpot07Taki_DoNothing;
}

static void BgSpot07Taki_Destroy(Actor* thisx, PlayState* play) {
    BgSpot07Taki* this = (BgSpot07Taki*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgSpot07Taki_DoNothing(BgSpot07Taki* this, PlayState* play) {
}

static void BgSpot07Taki_Update(Actor* thisx, PlayState* play) {
    BgSpot07Taki* this = (BgSpot07Taki*)thisx;

    this->actionFunc(this, play);
}

static void BgSpot07Taki_Draw(Actor* thisx, PlayState* play) {
    BgSpot07Taki* this = (BgSpot07Taki*)thisx;
    u32 frames;

    OPEN_DISPS(play->state.gfxCtx);
    frames = play->gameplayFrames;
    if (LINK_IS_ADULT_OOT) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        if (this->dyna.actor.params == 0) {
            gSPDisplayList(POLY_OPA_DISP++, object_spot07_object_DL_001CF0);
        } else {
            gSPDisplayList(POLY_OPA_DISP++, object_spot07_object_DL_003210);
        }
    }
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, ((frames * -1) & 0x7F), ((frames * 1) & 0x7F), 32, 32, 1,
                                  ((frames * 1) & 0x7F), ((frames * 1) & 0x7F), 32, 32, -1, 1, 1, 1));

    if (!LINK_IS_ADULT_OOT) {
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, 128);
        if (this->dyna.actor.params == 0) {
            gSPSegment(POLY_XLU_DISP++, 0x09,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, ((frames * -1) & 0x7F), ((frames * -3) & 0xFF), 64, 64,
                                          1, ((frames * 1) & 0x7F), ((frames * -3) & 0xFF), 64, 64, -1, -3, 1, -3));
            gSPSegment(POLY_XLU_DISP++, 0x0A,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, frames * 0, ((frames * 3) & 0x1FF), 32, 128, 1,
                                          frames * 0, ((frames * 3) & 0x1FF), 32, 128, 0, 3, 0, 3));
            gSPDisplayList(POLY_XLU_DISP++, object_spot07_object_DL_000460);
        } else {
            gSPSegment(POLY_XLU_DISP++, 0x09,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, frames * 0, ((frames * -1) & 0x7F), 32, 32, 1,
                                          frames * 0, ((frames * -1) & 0x7F), 32, 32, 0, -1, 0, -1));
            gSPSegment(POLY_XLU_DISP++, 0x0A,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, frames * 0, ((frames * 3) & 0x1FF), 32, 128, 1,
                                          frames * 0, ((frames * 3) & 0x1FF), 32, 128, 0, 3, 0, 3));
            gSPDisplayList(POLY_XLU_DISP++, object_spot07_object_DL_000BE0);
        }
    } else if (this->dyna.actor.params == 0) {
        gSPDisplayList(POLY_XLU_DISP++, object_spot07_object_DL_001F68);
    } else {
        gSPDisplayList(POLY_XLU_DISP++, object_spot07_object_DL_0032D8);
    }
    CLOSE_DISPS(play->state.gfxCtx);
}
