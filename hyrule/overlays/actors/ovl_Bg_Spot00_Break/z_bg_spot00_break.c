/*
 * File: z_bg_spot00_break.c
 * Overlay: ovl_Bg_Spot00_Break
 * Description: Broken drawbridge in Hyrule Field.
 */

#include "z_bg_spot00_break.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_spot00_break/object_spot00_break.h"

#define FLAGS 0

static void BgSpot00Break_Init(Actor* thisx, PlayState* play);
static void BgSpot00Break_Destroy(Actor* thisx, PlayState* play);
static void BgSpot00Break_Update(Actor* thisx, PlayState* play);
static void BgSpot00Break_Draw(Actor* thisx, PlayState* play);

ActorProfile Bg_Spot00_Break_Oot_Profile = {
    /**/ ACTOR_BG_SPOT00_BREAK_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_SPOT00_BREAK_Oot,
    /**/ sizeof(BgSpot00Break),
    /**/ BgSpot00Break_Init,
    /**/ BgSpot00Break_Destroy,
    /**/ BgSpot00Break_Update,
    /**/ BgSpot00Break_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeScale, 1200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 2000, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static void BgSpot00Break_Init(Actor* thisx, PlayState* play) {
    BgSpot00Break* this = (BgSpot00Break*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);

    if (this->dyna.actor.params == 1) {
        CollisionHeader_GetVirtual(&gBarbedWireFenceCol, &colHeader);
    } else {
        CollisionHeader_GetVirtual(&gBrokenDrawbridgeCol, &colHeader);
    }

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if (!LINK_IS_ADULT_OOT) {
        Actor_Kill(&this->dyna.actor);
    }
}

static void BgSpot00Break_Destroy(Actor* thisx, PlayState* play) {
    BgSpot00Break* this = (BgSpot00Break*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgSpot00Break_Update(Actor* thisx, PlayState* play) {
}

static void BgSpot00Break_Draw(Actor* thisx, PlayState* play) {
    BgSpot00Break* this = (BgSpot00Break*)thisx;

    if (this->dyna.actor.params == 1) {
        Gfx_DrawDListOpa(play, gBarbedWireFenceDL);
    } else {
        Gfx_DrawDListOpa(play, gBrokenDrawbridgeDL);
    }
}
