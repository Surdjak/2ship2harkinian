/*
 * File: z_bg_ingate.c
 * Overlay: ovl_Bg_Ingate
 * Description: Ingo's Gates (Lon Lon Ranch)
 */

#include "z_bg_ingate.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleCutscene.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_ingate/object_ingate.h"

#define FLAGS 0

static void BgInGate_Init(Actor* thisx, PlayState* play);
static void BgInGate_Destroy(Actor* thisx, PlayState* play);
static void BgInGate_Update(Actor* thisx, PlayState* play);
static void BgInGate_Draw(Actor* thisx, PlayState* play);

static void func_80892890(BgInGate* this, PlayState* play);
static void BgInGate_DoNothing(BgInGate* this, PlayState* play);

ActorProfile Bg_Ingate_Oot_Profile = {
    /**/ ACTOR_BG_INGATE_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_INGATE_Oot,
    /**/ sizeof(BgInGate),
    /**/ BgInGate_Init,
    /**/ BgInGate_Destroy,
    /**/ BgInGate_Update,
    /**/ BgInGate_Draw,
};

static void BgInGate_SetupAction(BgInGate* this, BgInGateActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgInGate_Init(Actor* thisx, PlayState* play) {
    BgInGate* this = (BgInGate*)thisx;

    s32 pad;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gIngoGateCol, &colHeader);

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if ((play->sceneId != SCENE_LON_LON_RANCH_Oot || !LINK_IS_ADULT_OOT) ||
        (((GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot))) && (gSaveContext.save.cutsceneIndex != 0xFFF0))) {
        Actor_Kill(&this->dyna.actor);
        return;
    }

    Actor_SetScale(&this->dyna.actor, 0.1f);
    if (((this->dyna.actor.params & 1) != 0) && ((gSaveContext.eventInf[0] & 0xF) == 6)) {
        gCsCtxOot.frames = 0;
        BgInGate_SetupAction(this, func_80892890);
    } else {
        BgInGate_SetupAction(this, BgInGate_DoNothing);
    }
}

static void BgInGate_Destroy(Actor* thisx, PlayState* play) {
    BgInGate* this = (BgInGate*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_80892890(BgInGate* this, PlayState* play) {
    s32 phi0;
    s16 phi1;
    s16 csFrames;

    if (gCsCtxOot.frames >= 50) {
        phi0 = 0x4000;
        if ((this->dyna.actor.params & 2) == 0) {
            phi0 = -0x4000;
        }
        this->dyna.actor.shape.rot.y = this->dyna.actor.world.rot.y + phi0;
        BgInGate_SetupAction(this, BgInGate_DoNothing);
    } else if (gCsCtxOot.frames >= 10) {
        csFrames = gCsCtxOot.frames - 10;
        csFrames *= 400;
        phi1 = csFrames;
        if (csFrames > 0x4000) {
            csFrames = 0x4000;
        }
        csFrames = (Math_SinS(csFrames) * 16384.0f);
        phi1 = csFrames;
        if ((this->dyna.actor.params & 2) == 0) {
            phi1 = -phi1;
        }
        this->dyna.actor.shape.rot.y = this->dyna.actor.world.rot.y + phi1;
    }
}

static void BgInGate_DoNothing(BgInGate* this, PlayState* play) {
}

static void BgInGate_Update(Actor* thisx, PlayState* play) {
    BgInGate* this = (BgInGate*)thisx;

    this->actionFunc(this, play);
}

static void BgInGate_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_OPA_DISP++, gIngoGateDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
