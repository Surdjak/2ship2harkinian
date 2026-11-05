/*
 * File: z_bg_gnd_nisekabe.c
 * Overlay: ovl_Bg_Gnd_Nisekabe
 * Description: Ganon's Castle Fake Wall
 */

#include "z_bg_gnd_nisekabe.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_demo_kekkai/object_demo_kekkai.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgGndNisekabe_Init(Actor* thisx, PlayState* play);
static void BgGndNisekabe_Destroy(Actor* thisx, PlayState* play);
static void BgGndNisekabe_Update(Actor* thisx, PlayState* play);
static void BgGndNisekabe_Draw(Actor* thisx, PlayState* play);

ActorProfile Bg_Gnd_Nisekabe_Oot_Profile = {
    /**/ ACTOR_BG_GND_NISEKABE_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_DEMO_KEKKAI_Oot,
    /**/ sizeof(BgGndNisekabe),
    /**/ BgGndNisekabe_Init,
    /**/ BgGndNisekabe_Destroy,
    /**/ BgGndNisekabe_Update,
    /**/ BgGndNisekabe_Draw,
};

static void BgGndNisekabe_Init(Actor* thisx, PlayState* play) {
    BgGndNisekabe* this = (BgGndNisekabe*)thisx;

    Actor_SetScale(&this->actor, 0.1);
    this->actor.cullingVolumeDistance = 3000.0;
}

static void BgGndNisekabe_Destroy(Actor* thisx, PlayState* play) {
}

static void BgGndNisekabe_Update(Actor* thisx, PlayState* play) {
    BgGndNisekabe* this = (BgGndNisekabe*)thisx;

    if (play->actorCtx.lensActive) {
        this->actor.flags |= ACTOR_FLAG_REACT_TO_LENS;
    } else {
        this->actor.flags &= ~ACTOR_FLAG_REACT_TO_LENS;
    }
}

static void BgGndNisekabe_Draw(Actor* thisx, PlayState* play) {
    static Gfx* dLists[] = {
        gLightTrialFakeWallDL,
        gGanonsCastleUnusedFakeWallDL,
        gGanonsCastleScrubsFakeWallDL,
    };
    BgGndNisekabe* this = (BgGndNisekabe*)thisx;
    u32 index = this->actor.params & 0xFF;

    if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_REACT_TO_LENS)) {
        Gfx_DrawDListXlu(play, dLists[index]);
    } else {
        Gfx_DrawDListOpa(play, dLists[index]);
    }
}
