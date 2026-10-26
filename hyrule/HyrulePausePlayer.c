// Link's figure on the equipment page (docs/PAUSE.md, U6), as SoH draws OoT's: Link is drawn into a small
// framebuffer from the work display list, then that framebuffer is drawn on the page. MM has no pause Link,
// so this draws the Player itself, in its current body and equipment, posed in its body's idle animation.

#include "global.h"
#include "HyrulePausePlayer.h"
#include "HyrulePlayer.h"

#define PAUSE_PLAYER_WIDTH 64
#define PAUSE_PLAYER_HEIGHT 112

int gfx_create_framebuffer(uint32_t width, uint32_t height, uint32_t native_width, uint32_t native_height,
                           uint8_t resize, bool forceFixedAspect);

extern PlayerAnimationHeader* D_8085BE84[PLAYER_ANIMGROUP_MAX][PLAYER_ANIMTYPE_MAX];

static s32 sPausePlayerFrameBuffer = -1;

// Where each body stands in front of the camera, as OoT's child Link does (KaleidoScope_DrawPlayerWork).
typedef struct PausePlayerPose {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ f32 scale;
} PausePlayerPose; // size = 0x10

static const PausePlayerPose sPoses[PLAYER_FORM_MAX] = {
    { { 2.0f, -175.0f, -150.0f }, 0.036f }, // PLAYER_FORM_FIERCE_DEITY
    { { 2.0f, -125.0f, -150.0f }, 0.030f }, // PLAYER_FORM_GORON
    { { 2.0f, -150.0f, -150.0f }, 0.040f }, // PLAYER_FORM_ZORA
    { { 2.0f, -118.0f, -150.0f }, 0.050f }, // PLAYER_FORM_DEKU
    { { 2.0f, -130.0f, -150.0f }, 0.046f }, // PLAYER_FORM_HUMAN
};

// Adult Link (docs/ADULT_LINK.md), as OoT stands him when he is not holding up the Master Sword.
static const PausePlayerPose sAdultPose = { { 25.0f, -228.0f, 60.0f }, 0.056f };

void Hyrule_DrawPausePlayer(PlayState* play) {
    static Vp sViewport = { (PAUSE_PLAYER_WIDTH / 2) << 2, (PAUSE_PLAYER_HEIGHT / 2) << 2, G_MAXZ / 2, 0,
                            (PAUSE_PLAYER_WIDTH / 2) << 2, (PAUSE_PLAYER_HEIGHT / 2) << 2, G_MAXZ / 2, 0 };
    static Lights1 sLights = gdSPDefLights1(80, 80, 80, 255, 255, 255, 84, 84, -84);
    // The Player is drawn as in gameplay, whose limb callbacks write back positions (hands, feet, focus...);
    // it is restored after.
    static Player sSavedPlayer;
    static Vec3s sJointTable[PLAYER_LIMB_MAX + 1];
    Player* player = GET_PLAYER(play);
    const PausePlayerPose* pose = Hyrule_IsAdultPlayer(player) ? &sAdultPose : &sPoses[player->transformation];
    Mtx* perspMtx = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx));
    Mtx* lookAtMtx = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx));
    Vec3s rot = { 0, 32300, 0 };
    u16 perspNorm;
    Gfx* opaRef;
    Gfx* xluRef;

    if (sPausePlayerFrameBuffer == -1) {
        sPausePlayerFrameBuffer =
            gfx_create_framebuffer(PAUSE_PLAYER_WIDTH, PAUSE_PLAYER_HEIGHT, PAUSE_PLAYER_WIDTH, PAUSE_PLAYER_HEIGHT,
                                   true, true);
    }

    OPEN_DISPS(play->state.gfxCtx);

    // The work display list runs first: it switches to the framebuffer and calls this, which the opaque and
    // translucent lists branch over.
    gsSPSetFB(WORK_DISP++, sPausePlayerFrameBuffer);
    opaRef = POLY_OPA_DISP++;
    xluRef = POLY_XLU_DISP++;
    gSPDisplayList(WORK_DISP++, POLY_OPA_DISP);
    gSPDisplayList(WORK_DISP++, POLY_XLU_DISP);

    gSPSegment(POLY_OPA_DISP++, 0x00, NULL);
    gDPPipeSync(POLY_OPA_DISP++);
    gSPLoadGeometryMode(POLY_OPA_DISP++, 0);
    gSPTexture(POLY_OPA_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_SHADE, G_CC_SHADE);
    gDPSetOtherMode(POLY_OPA_DISP++,
                    G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_PERSP | G_CYC_FILL | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PIXEL | G_RM_NOOP | G_RM_NOOP2);
    gSPLoadGeometryMode(POLY_OPA_DISP++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH);
    gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, PAUSE_PLAYER_WIDTH, PAUSE_PLAYER_HEIGHT);
    gSPClipRatio(POLY_OPA_DISP++, FRUSTRATIO_1);

    // Clear depth, then colour to black
    gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, PAUSE_PLAYER_WIDTH,
                     play->state.gfxCtx->curFrameBuffer + PAUSE_PLAYER_WIDTH * PAUSE_PLAYER_HEIGHT);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_FILL);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor(POLY_OPA_DISP++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
    gDPFillRectangle(POLY_OPA_DISP++, 0, 0, PAUSE_PLAYER_WIDTH - 1, PAUSE_PLAYER_HEIGHT - 1);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetColorImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, PAUSE_PLAYER_WIDTH,
                     play->state.gfxCtx->curFrameBuffer);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_FILL);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor(POLY_OPA_DISP++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
    gDPFillRectangle(POLY_OPA_DISP++, 0, 0, PAUSE_PLAYER_WIDTH - 1, PAUSE_PLAYER_HEIGHT - 1);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetDepthImage(POLY_OPA_DISP++, play->state.gfxCtx->curFrameBuffer + PAUSE_PLAYER_WIDTH * PAUSE_PLAYER_HEIGHT);

    sViewport.vp.vscale[0] = sViewport.vp.vtrans[0] = PAUSE_PLAYER_WIDTH * ((1 << 2) / 2);
    sViewport.vp.vscale[1] = sViewport.vp.vtrans[1] = PAUSE_PLAYER_HEIGHT * ((1 << 2) / 2);
    gSPViewport(POLY_OPA_DISP++, &sViewport);
    guPerspective(perspMtx, &perspNorm, 60.0f, (f32)PAUSE_PLAYER_WIDTH / (f32)PAUSE_PLAYER_HEIGHT, 10.0f, 4000.0f,
                  1.0f);
    gSPPerspNormalize(POLY_OPA_DISP++, perspNorm);
    gSPMatrix(POLY_OPA_DISP++, perspMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    guLookAt(lookAtMtx, 0.0f, 0.0f, -400.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    gSPMatrix(POLY_OPA_DISP++, lookAtMtx, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

    gSPSetLights1(POLY_OPA_DISP++, sLights);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    POLY_OPA_DISP = Gfx_SetFog2(POLY_OPA_DISP, 0, 0, 0, 0, 997, 1000);
    gSPSegment(POLY_OPA_DISP++, 0x0C, gCullBackDList);
    gSPSegment(POLY_XLU_DISP++, 0x0C, gCullBackDList);

    Matrix_Push();
    Matrix_SetTranslateRotateYXZ(pose->pos.x, pose->pos.y, pose->pos.z, &rot);
    Matrix_Scale(pose->scale, pose->scale, pose->scale, MTXMODE_APPLY);

    memcpy(&sSavedPlayer, player, sizeof(Player));
    AnimTaskQueue_AddLoadPlayerFrame(play, D_8085BE84[PLAYER_ANIMGROUP_wait][player->modelAnimType], 0,
                                     player->skelAnime.limbCount, sJointTable);
    Player_DrawImpl(play, player->skelAnime.skeleton, sJointTable, player->skelAnime.dListCount, 0,
                    player->transformation, 0, player->actor.shape.face, Player_OverrideLimbDrawGameplayDefault,
                    Player_PostLimbDrawGameplay, &player->actor);
    memcpy(player, &sSavedPlayer, sizeof(Player));

    Matrix_Pop();

    gSPEndDisplayList(POLY_OPA_DISP++);
    gSPEndDisplayList(POLY_XLU_DISP++);
    gSPBranchList(opaRef, POLY_OPA_DISP);
    gSPBranchList(xluRef, POLY_XLU_DISP);
    gsSPResetFB(WORK_DISP++);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_DrawPausePlayerImage(PlayState* play, Vtx* quad, u8 alpha) {
    // A loaded tile of the image's size, which the framebuffer then replaces (as SoH does).
    static u16 sTileSource[PAUSE_PLAYER_WIDTH * PAUSE_PLAYER_HEIGHT];

    if (sPausePlayerFrameBuffer == -1) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_POINT);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, alpha);
    gDPSetTileCustom(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, PAUSE_PLAYER_WIDTH, PAUSE_PLAYER_HEIGHT, 0,
                     G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                     G_TX_NOLOD);
    gSPVertex(POLY_OPA_DISP++, quad, 4, 0);
    gDPSetTextureImage(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, PAUSE_PLAYER_WIDTH, sTileSource);
    gDPLoadSync(POLY_OPA_DISP++);
    gDPLoadTile(POLY_OPA_DISP++, G_TX_LOADTILE, 0, 0, (PAUSE_PLAYER_WIDTH - 1) << 2, (PAUSE_PLAYER_HEIGHT - 1) << 2);
    gDPSetTextureImageFB(POLY_OPA_DISP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, PAUSE_PLAYER_WIDTH, sPausePlayerFrameBuffer);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);

    CLOSE_DISPS(play->state.gfxCtx);
}
