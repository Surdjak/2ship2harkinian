/*
 * File: z_end_title.c
 * Overlay: ovl_End_Title
 * Description: "The End" message
 */

#include "z_end_title.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EndTitle_Init(Actor* thisx, PlayState* play);
static void EndTitle_Destroy(Actor* thisx, PlayState* play);
static void EndTitle_Update(Actor* thisx, PlayState* play);
static void EndTitle_DrawFull(Actor* thisx, PlayState* play);
static void EndTitle_DrawNintendoLogo(Actor* thisx, PlayState* play);

ActorProfile End_Title_Oot_Profile = {
    /**/ ACTOR_END_TITLE_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EndTitle),
    /**/ EndTitle_Init,
    /**/ EndTitle_Destroy,
    /**/ EndTitle_Update,
    /**/ EndTitle_DrawFull,
};

#include "assets/oot/overlays/ovl_End_Title/ovl_End_Title.h"

static void EndTitle_Init(Actor* thisx, PlayState* play) {
    EndTitle* this = (EndTitle*)thisx;

    this->endAlpha = 0;
    this->tlozAlpha = 0;
    this->ootAlpha = 0;
    if (this->actor.params == 1) {
        this->actor.draw = EndTitle_DrawNintendoLogo;
    }
}

static void EndTitle_Destroy(Actor* thisx, PlayState* play) {
}

static void EndTitle_Update(Actor* thisx, PlayState* play) {
}

// Used in the castle courtyard
static void EndTitle_DrawFull(Actor* thisx, PlayState* play) {
    MtxF* mf;
    EndTitle* this = (EndTitle*)thisx;
    s32 frameCount = gCsCtxOot.frames;
    Player* player = GET_PLAYER(play);

    mf = &player->leftHandMf;

    OPEN_DISPS(play->state.gfxCtx);

    // Draw the Triforce on Link's left hand
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    Matrix_Mult(mf, MTXMODE_NEW);
    Matrix_Translate(0.0f, 150.0f, 170.0f, MTXMODE_APPLY);
    Matrix_Scale(0.13f, 0.13f, 0.13f, MTXMODE_APPLY);
    Matrix_RotateXFApply(0xBB8 * M_PI / 0x8000);
    Matrix_RotateYF(0.0f, MTXMODE_APPLY);
    Matrix_RotateZF(0.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_Finalize(play->state.gfxCtx), G_MTX_LOAD);
    gSPDisplayList(POLY_XLU_DISP++, sTriforceDL);

    CLOSE_DISPS(play->state.gfxCtx);

    OPEN_DISPS(play->state.gfxCtx);

    // Draw title cards on the screen
    // PORT: OoT's game ends here; 3Ship's goes on, back in Zelda's courtyard (Hyrule_ContinueAfterOotEnding).
    if (this->endAlpha >= 200) {
        Hyrule_SetOotEndCardShown();
    }
    if ((frameCount > 890) && (this->endAlpha < 200)) {
        this->endAlpha += 7;
    }
    if ((frameCount > 810) && (this->tlozAlpha < 200)) {
        this->tlozAlpha += 15;
    }
    if ((frameCount > 850) && (this->ootAlpha < 200)) {
        this->ootAlpha += 15;
    }

    OVERLAY_DISP = Gfx_SetupDL64(OVERLAY_DISP);
    gDPSetTextureLUT(OVERLAY_DISP++, G_TT_NONE);
    gDPSetEnvColor(OVERLAY_DISP++, 255, 120, 30, 0);
    gDPSetRenderMode(OVERLAY_DISP++, G_RM_PASS, G_RM_XLU_SURF2);
    gSPClearGeometryMode(OVERLAY_DISP++,
                         G_TEXTURE_ENABLE | G_CULL_BACK | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                      COMBINED, 0, 0, 0, COMBINED);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, this->endAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sTheEndTex, G_IM_FMT_IA, G_IM_SIZ_8b, 80, 24, 0, 0, 80 - 1, 24 - 1, 0,
                       G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 120 << 2, 90 << 2, 200 << 2, 113 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, this->tlozAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sTheLegendOfZeldaTex, G_IM_FMT_IA, G_IM_SIZ_8b, 120, 24, 0, 0, 120 - 1, 24 - 1,
                       0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 100 << 2, 160 << 2, 220 << 2, 183 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, this->ootAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sOcarinaOfTimeTex, G_IM_FMT_IA, G_IM_SIZ_8b, 112, 16, 0, 0, 112 - 1, 16 - 1, 0,
                       G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 104 << 2, 177 << 2, 216 << 2, 192 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);
    CLOSE_DISPS(play->state.gfxCtx);
}

// Used in the Temple of Time
static void EndTitle_DrawNintendoLogo(Actor* thisx, PlayState* play) {
    EndTitle* this = (EndTitle*)thisx;
    s32 pad;
    s32 frames = gCsCtxOot.frames;

    if ((frames >= 1101) && (this->endAlpha < 255)) {
        this->endAlpha += 3;
    }

    OPEN_DISPS(play->state.gfxCtx);

    OVERLAY_DISP = Gfx_SetupDL64(OVERLAY_DISP);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0x80, 0, 0, 0, this->endAlpha);
    gSPDisplayList(OVERLAY_DISP++, sPresentedByNintendoDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
