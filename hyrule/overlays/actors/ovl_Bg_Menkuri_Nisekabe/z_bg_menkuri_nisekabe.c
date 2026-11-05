/*
 * File: z_bg_menkuri_nisekabe.c
 * Overlay: ovl_Bg_Menkuri_Nisekabe
 * Description: False Stone Walls (Gerudo Training Ground)
 */

#include "z_bg_menkuri_nisekabe.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_menkuri_objects/object_menkuri_objects.h"

#define FLAGS 0

static void BgMenkuriNisekabe_Init(Actor* thisx, PlayState* play);
static void BgMenkuriNisekabe_Destroy(Actor* thisx, PlayState* play);
static void BgMenkuriNisekabe_Update(Actor* thisx, PlayState* play);
static void BgMenkuriNisekabe_Draw(Actor* thisx, PlayState* play);

ActorProfile Bg_Menkuri_Nisekabe_Oot_Profile = {
    /**/ ACTOR_BG_MENKURI_NISEKABE_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_MENKURI_OBJECTS_Oot,
    /**/ sizeof(BgMenkuriNisekabe),
    /**/ BgMenkuriNisekabe_Init,
    /**/ BgMenkuriNisekabe_Destroy,
    /**/ BgMenkuriNisekabe_Update,
    /**/ BgMenkuriNisekabe_Draw,
};

static Gfx* sDLists[] = { gGTGFakeWallDL, gGTGFakeCeilingDL };

static void BgMenkuriNisekabe_Init(Actor* thisx, PlayState* play) {
    BgMenkuriNisekabe* this = (BgMenkuriNisekabe*)thisx;

    Actor_SetScale(&this->actor, 0.1f);
}

static void BgMenkuriNisekabe_Destroy(Actor* thisx, PlayState* play) {
}

static void BgMenkuriNisekabe_Update(Actor* thisx, PlayState* play) {
    BgMenkuriNisekabe* this = (BgMenkuriNisekabe*)thisx;

    if (play->actorCtx.lensActive) {
        this->actor.flags |= ACTOR_FLAG_REACT_TO_LENS;
    } else {
        this->actor.flags &= ~ACTOR_FLAG_REACT_TO_LENS;
    }
}

static void BgMenkuriNisekabe_Draw(Actor* thisx, PlayState* play) {
    BgMenkuriNisekabe* this = (BgMenkuriNisekabe*)thisx;
    u32 index = this->actor.params & 0xFF;

    if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_REACT_TO_LENS)) {
        Gfx_DrawDListXlu(play, sDLists[index]);
    } else {
        Gfx_DrawDListOpa(play, sDLists[index]);
    }
}
