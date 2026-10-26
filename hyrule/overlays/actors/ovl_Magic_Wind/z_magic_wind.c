/*
 * File: z_magic_wind.c
 * Overlay: ovl_Magic_Wind
 * Description: Farore's Wind
 */

#include "z_magic_wind.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "2s2h/BenPort.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

static void MagicWind_Init(Actor* thisx, PlayState* play);
static void MagicWind_Destroy(Actor* thisx, PlayState* play);
static void MagicWind_Update(Actor* thisx, PlayState* play);
static void MagicWind_Draw(Actor* thisx, PlayState* play);

static void MagicWind_Shrink(MagicWind* this, PlayState* play);
static void MagicWind_WaitForTimer(MagicWind* this, PlayState* play);
static void MagicWind_FadeOut(MagicWind* this, PlayState* play);
static void MagicWind_WaitAtFullSize(MagicWind* this, PlayState* play);
static void MagicWind_Grow(MagicWind* this, PlayState* play);

ActorProfile Magic_Wind_Oot_Profile = {
    /**/ ACTOR_MAGIC_WIND_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicWind),
    /**/ MagicWind_Init,
    /**/ MagicWind_Destroy,
    /**/ MagicWind_Update,
    /**/ MagicWind_Draw,
};

#include "assets/oot/overlays/ovl_Magic_Wind/ovl_Magic_Wind.h"

static u8 sAlphaUpdVals[] = {
    0x00, 0x03, 0x04, 0x07, 0x09, 0x0A, 0x0D, 0x0F, 0x11, 0x12, 0x15, 0x16, 0x19, 0x1B, 0x1C, 0x1F, 0x21, 0x23,
};

static void MagicWind_SetupAction(MagicWind* this, MagicWindFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void MagicWind_Init(Actor* thisx, PlayState* play) {
    MagicWind* this = (MagicWind*)thisx;
    Player* player = GET_PLAYER(play);

    if (SkelCurve_Init(play, &this->skelCurve, &sSkel, &sAnim) == 0) {
    }
    this->actor.room = -1;
    switch (this->actor.params) {
        case 0:
            SkelCurve_SetAnim(&this->skelCurve, &sAnim, 0.0f, 60.0f, 0.0f, 1.0f);
            this->timer = 29;
            MagicWind_SetupAction(this, MagicWind_WaitForTimer);
            break;
        case 1:
            SkelCurve_SetAnim(&this->skelCurve, &sAnim, 60.0f, 0.0f, 60.0f, -1.0f);
            MagicWind_SetupAction(this, MagicWind_Shrink);
            Player_PlaySfx(player, NA_SE_PL_MAGIC_WIND_WARP);
            break;
    }
}

static void MagicWind_Destroy(Actor* thisx, PlayState* play) {
    MagicWind* this = (MagicWind*)thisx;
    SkelCurve_Destroy(play, &this->skelCurve);
    Magic_Reset(play);
}

static void MagicWind_UpdateAlpha(f32 alpha) {
    s32 i;

    Vtx* vtx = ResourceMgr_LoadVtxByName(sCylinderVtx);

    for (i = 0; i < ARRAY_COUNT(sAlphaUpdVals); i++) {
        vtx[sAlphaUpdVals[i]].n.a = alpha * 255.0f;
    }
}

static void MagicWind_WaitForTimer(MagicWind* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->timer > 0) {
        this->timer--;
        return;
    }

    Player_PlaySfx(player, NA_SE_PL_MAGIC_WIND_NORMAL);
    MagicWind_UpdateAlpha(1.0f);
    MagicWind_SetupAction(this, MagicWind_Grow);
    SkelCurve_Update(play, &this->skelCurve);
}

static void MagicWind_Grow(MagicWind* this, PlayState* play) {
    if (SkelCurve_Update(play, &this->skelCurve)) {
        MagicWind_SetupAction(this, MagicWind_WaitAtFullSize);
        this->timer = 50;
    }
}

static void MagicWind_WaitAtFullSize(MagicWind* this, PlayState* play) {
    if (this->timer > 0) {
        this->timer--;
    } else {
        MagicWind_SetupAction(this, MagicWind_FadeOut);
        this->timer = 30;
    }
}

static void MagicWind_FadeOut(MagicWind* this, PlayState* play) {
    if (this->timer > 0) {
        MagicWind_UpdateAlpha((f32)this->timer * (1.0f / 30.0f));
        this->timer--;
    } else {
        Actor_Kill(&this->actor);
    }
}

static void MagicWind_Shrink(MagicWind* this, PlayState* play) {
    if (SkelCurve_Update(play, &this->skelCurve)) {
        Actor_Kill(&this->actor);
    }
}

static void MagicWind_Update(Actor* thisx, PlayState* play) {
    MagicWind* this = (MagicWind*)thisx;
    if (play->msgCtx.msgMode == MSGMODE_DISPLAY_SONG_PLAYED || play->msgCtx.msgMode == MSGMODE_SONG_PLAYED) {
        Actor_Kill(thisx);
        return;
    }

    this->actionFunc(this, play);
}

static s32 MagicWind_OverrideLimbDraw(PlayState* play, SkelCurve* skelCurve, s32 limbIndex, void* thisx) {
    MagicWind* this = (MagicWind*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == 1) {
        gSPSegment(POLY_XLU_DISP++, 8,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (play->state.frames * 9) & 0xFF,
                                      0xFF - ((play->state.frames * 0xF) & 0xFF), 0x40, 0x40, 1,
                                      (play->state.frames * 0xF) & 0xFF, 0xFF - ((play->state.frames * 0x1E) & 0xFF),
                                      0x40, 0x40, 9, -0xF, 0xF, -0x1E));

    } else if (limbIndex == 2) {
        gSPSegment(POLY_XLU_DISP++, 9,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (play->state.frames * 3) & 0xFF,
                                      0xFF - ((play->state.frames * 5) & 0xFF), 0x40, 0x40, 1,
                                      (play->state.frames * 6) & 0xFF, 0xFF - ((play->state.frames * 0xA) & 0xFF), 0x40,
                                      0x40, 3, -5, 6, -0xA));
    }

    CLOSE_DISPS(play->state.gfxCtx);

    return true;
}

static void MagicWind_Draw(Actor* thisx, PlayState* play) {
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    MagicWind* this = (MagicWind*)thisx;

    OPEN_DISPS(gfxCtx);

    if (this->actionFunc != MagicWind_WaitForTimer) {
        POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, 25);
        SkelCurve_Draw(thisx, play, &this->skelCurve, MagicWind_OverrideLimbDraw, NULL, 1, NULL);
    }

    CLOSE_DISPS(gfxCtx);
}
