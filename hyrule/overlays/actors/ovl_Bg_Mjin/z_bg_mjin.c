/*
 * File: z_bg_mjin.c
 * Overlay: ovl_Bg_Mjin
 * Description: Warp Pad
 */

#include "z_bg_mjin.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_mjin/object_mjin.h"
#include "assets/oot/objects/object_mjin_wind/object_mjin_wind.h"
#include "assets/oot/objects/object_mjin_soul/object_mjin_soul.h"
#include "assets/oot/objects/object_mjin_dark/object_mjin_dark.h"
#include "assets/oot/objects/object_mjin_ice/object_mjin_ice.h"
#include "assets/oot/objects/object_mjin_flame/object_mjin_flame.h"
#include "assets/oot/objects/object_mjin_flash/object_mjin_flash.h"
#include "assets/oot/objects/object_mjin_oka/object_mjin_oka.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgMjin_Init(Actor* thisx, PlayState* play);
static void BgMjin_Destroy(Actor* thisx, PlayState* play);
static void BgMjin_Update(Actor* thisx, PlayState* play);
static void BgMjin_Draw(Actor* thisx, PlayState* play);

static void func_808A0850(BgMjin* this, PlayState* play);
static void BgMjin_DoNothing(BgMjin* this, PlayState* play);

ActorProfile Bg_Mjin_Oot_Profile = {
    /**/ ACTOR_BG_MJIN_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(BgMjin),
    /**/ BgMjin_Init,
    /**/ BgMjin_Destroy,
    /**/ BgMjin_Update,
    /**/ NULL,
};

extern UNK_TYPE D_06000000;

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 4000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 400, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 400, ICHAIN_STOP),
};

static s16 sObjectIDs[] = { OBJECT_MJIN_FLASH_Oot, OBJECT_MJIN_DARK_Oot, OBJECT_MJIN_FLAME_Oot,
                            OBJECT_MJIN_ICE_Oot,   OBJECT_MJIN_SOUL_Oot, OBJECT_MJIN_WIND_Oot };

void* gPedestalEmblems[] = { gLightMedallionPlatformTex, gShadowMedallionPlatformTex, gFireMedallionPlatformTex,
                             gWaterMedallionPlatformTex, gSpiritMedallionPlatformTex, gForestMedallionPlatformTex };

static void BgMjin_SetupAction(BgMjin* this, BgMjinActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgMjin_Init(Actor* thisx, PlayState* play) {
    BgMjin* this = (BgMjin*)thisx;
    s8 objectSlot;

    Actor_ProcessInitChain(thisx, sInitChain);
    objectSlot = Object_GetSlot(&play->objectCtx, (thisx->params != 0 ? OBJECT_MJIN_Oot : OBJECT_MJIN_OKA_Oot));
    this->objectSlot = objectSlot;
    if (objectSlot < 0) {
        Actor_Kill(thisx);
    } else {
        BgMjin_SetupAction(this, func_808A0850);
    }
}

static void BgMjin_Destroy(Actor* thisx, PlayState* play) {
    BgMjin* this = (BgMjin*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_808A0850(BgMjin* this, PlayState* play) {
    CollisionHeader* colHeader;
    CollisionHeader* collision;

    if (Object_IsLoaded(&play->objectCtx, this->objectSlot)) {
        colHeader = NULL;
        this->dyna.actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->dyna.actor.objectSlot = this->objectSlot;
        Actor_SetObjectDependency(play, &this->dyna.actor);
        DynaPolyActor_Init(&this->dyna, 0);
        collision = this->dyna.actor.params != 0 ? &gWarpPadCol : &gOcarinaWarpPadCol;
        CollisionHeader_GetVirtual(collision, &colHeader);
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
        BgMjin_SetupAction(this, BgMjin_DoNothing);
        this->dyna.actor.draw = BgMjin_Draw;
    }
}

static void BgMjin_DoNothing(BgMjin* this, PlayState* play) {
}

static void BgMjin_Update(Actor* thisx, PlayState* play) {
    BgMjin* this = (BgMjin*)thisx;

    this->actionFunc(this, play);
}

static void BgMjin_Draw(Actor* thisx, PlayState* play) {
    BgMjin* this = (BgMjin*)thisx;
    Gfx* dlist;

    OPEN_DISPS(play->state.gfxCtx);

    if (thisx->params != 0) {
        s32 objectSlot = Object_GetSlot(&play->objectCtx, sObjectIDs[thisx->params - 1]);

        if (objectSlot >= 0) {
            gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[objectSlot].segment);
        }

        gSPSegment(POLY_OPA_DISP++, 0x08, gPedestalEmblems[thisx->params - 1]);
        dlist = gWarpPadBaseDL;
    } else {
        dlist = gOcarinaWarpPadDL;
    }

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, dlist);

    CLOSE_DISPS(play->state.gfxCtx);
}
