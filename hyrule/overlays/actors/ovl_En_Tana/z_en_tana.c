/*
 * File: z_en_tana.c
 * Overlay: ovl_En_Tana
 * Description: Shop Shelves
 */

#include "z_en_tana.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_shop_dungen/object_shop_dungen.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnTana_Init(Actor* thisx, PlayState* play);
static void EnTana_Destroy(Actor* thisx, PlayState* play);
static void EnTana_Update(Actor* thisx, PlayState* play);
static void EnTana_DrawWoodenShelves(Actor* thisx, PlayState* play);
static void EnTana_DrawStoneShelves(Actor* thisx, PlayState* play);

ActorProfile En_Tana_Oot_Profile = {
    /**/ ACTOR_EN_TANA_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_SHOP_DUNGEN_Oot,
    /**/ sizeof(EnTana),
    /**/ EnTana_Init,
    /**/ EnTana_Destroy,
    /**/ EnTana_Update,
    /**/ NULL,
};

//! @bug A third entry is missing here. When printing the string indexed by `params` for type 2, the
//! next data entry will be dereferenced and print garbage, stopping any future printing.
//! In a non-matching context, this can cause a crash if the next item isn't a valid pointer.
static const char* sShelfTypes[] = {
    "木の棚", // "Wooden Shelves"
    "石の棚", // "Stone Shelves"
    "石の棚", // "Stone Shelves"
};

static const ActorFunc sDrawFuncs[] = {
    EnTana_DrawWoodenShelves,
    EnTana_DrawStoneShelves,
    EnTana_DrawStoneShelves,
};

static Gfx* sShelfDLists[] = {
    gShopDungenWoodenShelvesDL,
    gShopDungenStoneShelvesDL,
    gShopDungenStoneShelvesDL,
};

static void* sStoneTextures[] = {
    NULL,
    gShopDungenStone1Tex,
    gShopDungenStone2Tex,
};

static void EnTana_Init(Actor* thisx, PlayState* play) {
    EnTana* this = (EnTana*)thisx;

    Actor_SetScale(thisx, 1.0f);
    thisx->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    thisx->draw = sDrawFuncs[thisx->params];
}

static void EnTana_Destroy(Actor* thisx, PlayState* play) {
}

static void EnTana_Update(Actor* thisx, PlayState* play) {
}

static void EnTana_DrawWoodenShelves(Actor* thisx, PlayState* play) {
    EnTana* this = (EnTana*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sShelfDLists[thisx->params]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnTana_DrawStoneShelves(Actor* thisx, PlayState* play) {
    EnTana* this = (EnTana*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sStoneTextures[thisx->params]));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sShelfDLists[thisx->params]);

    CLOSE_DISPS(play->state.gfxCtx);
}
