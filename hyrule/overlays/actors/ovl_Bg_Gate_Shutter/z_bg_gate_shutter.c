/*
 * File: z_bg_gate_shutter.c
 * Overlay: Bg_Gate_Shutter
 * Description: Death Mountain Trail Gate
 */

#include "z_bg_gate_shutter.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_spot01_matoyab/object_spot01_matoyab.h"

#define FLAGS 0

static void BgGateShutter_Init(Actor* thisx, PlayState* play);
static void BgGateShutter_Destroy(Actor* thisx, PlayState* play);
static void BgGateShutter_Update(Actor* thisx, PlayState* play);
static void BgGateShutter_Draw(Actor* thisx, PlayState* play);

static void func_8087828C(BgGateShutter* this, PlayState* play);
static void func_80878300(BgGateShutter* this, PlayState* play);
static void func_808783AC(BgGateShutter* this, PlayState* play);
static void func_808783D4(BgGateShutter* this, PlayState* play);

ActorProfile Bg_Gate_Shutter_Oot_Profile = {
    /**/ ACTOR_BG_GATE_SHUTTER_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_SPOT01_MATOYAB_Oot,
    /**/ sizeof(BgGateShutter),
    /**/ BgGateShutter_Init,
    /**/ BgGateShutter_Destroy,
    /**/ BgGateShutter_Update,
    /**/ BgGateShutter_Draw,
};

static void BgGateShutter_Init(Actor* thisx, PlayState* play) {
    BgGateShutter* this = (BgGateShutter*)thisx;
    s32 pad[2];
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gKakarikoGuardGateCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    this->somePos.x = thisx->world.pos.x;
    this->somePos.y = thisx->world.pos.y;
    this->somePos.z = thisx->world.pos.z;
    if (((GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) ||
         (GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot))) &&
        (play->sceneId == SCENE_KAKARIKO_VILLAGE_Oot)) {
        thisx->world.pos.x = -89.0f;
        thisx->world.pos.z = -1375.0f;
    }
    thisx->scale.x = 1.0f;
    thisx->scale.y = 1.0f;
    thisx->scale.z = 1.0f;
    this->actionFunc = func_8087828C;
}

static void BgGateShutter_Destroy(Actor* thisx, PlayState* play) {
    BgGateShutter* this = (BgGateShutter*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_8087828C(BgGateShutter* this, PlayState* play) {
    if (this->openingState == 1 && !GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) {
        this->unk_178 = 2;
        this->actionFunc = func_80878300;
    } else if (this->openingState == 2) {
        this->unk_178 = 2;
        this->actionFunc = func_80878300;
    } else if (this->openingState < 0) {
        this->unk_178 = 2;
        this->actionFunc = func_808783D4;
    }
}

static void func_80878300(BgGateShutter* this, PlayState* play) {
    Actor* thisx = &this->dyna.actor;

    if (this->unk_178 == 0) {
        Actor_PlaySfx(thisx, NA_SE_EV_METALGATE_OPEN - SFX_FLAG);
        thisx->world.pos.x -= 2.0f;
        Math_ApproachF(&thisx->world.pos.z, -1375.0f, 0.8f, 0.3f);
        if (thisx->world.pos.x < -89.0f) {
            Actor_PlaySfx(thisx, NA_SE_EV_BRIDGE_OPEN_STOP);
            this->unk_178 = 0x1E;
            this->actionFunc = func_808783AC;
        }
    }
}

static void func_808783AC(BgGateShutter* this, PlayState* play) {
    if (this->unk_178 == 0) {
        this->openingState = 0;
        this->actionFunc = func_8087828C;
    }
}

static void func_808783D4(BgGateShutter* this, PlayState* play) {
    Actor* thisx = &this->dyna.actor;

    if (this->unk_178 == 0) {
        Actor_PlaySfx(thisx, NA_SE_EV_METALGATE_OPEN - SFX_FLAG);
        thisx->world.pos.x += 2.0f;
        Math_ApproachF(&thisx->world.pos.z, -1350.0f, 0.8f, 0.3f);
        if (thisx->world.pos.x > 90.0f) {
            thisx->world.pos.x = 91.0f;
            Actor_PlaySfx(thisx, NA_SE_EV_BRIDGE_OPEN_STOP);
            this->unk_178 = 30;
            this->actionFunc = func_808783AC;
        }
    }
}

static void BgGateShutter_Update(Actor* thisx, PlayState* play) {
    BgGateShutter* this = (BgGateShutter*)thisx;

    if (this->unk_178 != 0) {
        this->unk_178 -= 1;
    }
    this->actionFunc(this, play);
}

static void BgGateShutter_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gKakarikoGuardGateDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
