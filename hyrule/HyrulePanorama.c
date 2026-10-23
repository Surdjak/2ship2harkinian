// OoT's prerendered room panoramas: up to four 256x256 CI8 faces around the camera, drawn through OoT's
// skybox system (z_vr_box.c and z_vr_box_draw.c in OoT) when a prerendered room's camera is not the fixed
// one. MM's skybox code has no such panoramas, so they are kept here. Ported from SoH.

#include "global.h"
#include "HyrulePanorama.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define PANORAMA_FACES_MAX 4

typedef struct {
    /* 0x00 */ u8 ootSkyboxId;
    /* 0x01 */ u8 numFaces; // 2 for shops, 3 for OoT's skybox type 2, 4 otherwise
    /* 0x04 */ f32 rotY;
    /* 0x08 */ const char* textures[PANORAMA_FACES_MAX];
    /* 0x18 */ const char* palettes[PANORAMA_FACES_MAX];
} PanoramaInfo;

#define BG_TEX(file, name) "__OTR__oot/textures/" file "/" name

// The panoramas of the OoT scenes in the scene table (Skybox_Setup in OoT).
static const PanoramaInfo sPanoramas[] = {
    { 0x07, // SKYBOX_HOUSE_LINK
      4,
      0.0f,
      { BG_TEX("vr_LHVR_static", "gLinksHouseBgTex"), BG_TEX("vr_LHVR_static", "gLinksHouse2BgTex"),
        BG_TEX("vr_LHVR_static", "gLinksHouse3BgTex"), BG_TEX("vr_LHVR_static", "gLinksHouse4BgTex") },
      { BG_TEX("vr_LHVR_pal_static", "gLinksHouseBgTLUT"), BG_TEX("vr_LHVR_pal_static", "gLinksHouseBg2TLUT"),
        BG_TEX("vr_LHVR_pal_static", "gLinksHouseBg3TLUT"), BG_TEX("vr_LHVR_pal_static", "gLinksHouseBg4TLUT") } },
    { 0x0C, // SKYBOX_HOUSE_KNOW_IT_ALL_BROTHERS
      4,
      0.0f,
      { BG_TEX("vr_KHVR_static", "gKnowItAllBrosHouseBgTex"), BG_TEX("vr_KHVR_static", "gKnowItAllBrosHouse2BgTex"),
        BG_TEX("vr_KHVR_static", "gKnowItAllBrosHouse3BgTex"), BG_TEX("vr_KHVR_static", "gKnowItAllBrosHouse4BgTex") },
      { BG_TEX("vr_KHVR_pal_static", "gKnowItAllBrosHouseBgTLUT"),
        BG_TEX("vr_KHVR_pal_static", "gKnowItAllBrosHouseBg2TLUT"),
        BG_TEX("vr_KHVR_pal_static", "gKnowItAllBrosHouseBg3TLUT"),
        BG_TEX("vr_KHVR_pal_static", "gKnowItAllBrosHouseBg4TLUT") } },
    { 0x0E, // SKYBOX_HOUSE_OF_TWINS
      3,
      0.0f,
      { BG_TEX("vr_K3VR_static", "gHouseOfTwinsBgTex"), BG_TEX("vr_K3VR_static", "gHouseOfTwins2BgTex"),
        BG_TEX("vr_K3VR_static", "gHouseOfTwins3BgTex") },
      { BG_TEX("vr_K3VR_pal_static", "gHouseOfTwinsBgTLUT"), BG_TEX("vr_K3VR_pal_static", "gHouseOfTwinsBg2TLUT"),
        BG_TEX("vr_K3VR_pal_static", "gHouseOfTwinsBg3TLUT") } },
    { 0x11, // SKYBOX_KOKIRI_SHOP
      2,
      0.8f,
      { BG_TEX("vr_KSVR_static", "gKokiriShopBgTex"), BG_TEX("vr_KSVR_static", "gKokiriShop2BgTex") },
      { BG_TEX("vr_KSVR_pal_static", "gKokiriShopBgTLUT"), BG_TEX("vr_KSVR_pal_static", "gKokiriShopBg2TLUT") } },
    { 0x20, // SKYBOX_HOUSE_MIDO
      3,
      0.0f,
      { BG_TEX("vr_K4VR_static", "gMidosHouseBgTex"), BG_TEX("vr_K4VR_static", "gMidosHouse2BgTex"),
        BG_TEX("vr_K4VR_static", "gMidosHouse3BgTex") },
      { BG_TEX("vr_K4VR_pal_static", "gMidosHouseBgTLUT"), BG_TEX("vr_K4VR_pal_static", "gMidosHouseBg2TLUT"),
        BG_TEX("vr_K4VR_pal_static", "gMidosHouseBg3TLUT") } },
    { 0x21, // SKYBOX_HOUSE_SARIA
      3,
      0.0f,
      { BG_TEX("vr_K5VR_static", "gSariasHouseBgTex"), BG_TEX("vr_K5VR_static", "gSariasHouse2BgTex"),
        BG_TEX("vr_K5VR_static", "gSariasHouse3BgTex") },
      { BG_TEX("vr_K5VR_pal_static", "gSariasHouseBgTLUT"), BG_TEX("vr_K5VR_pal_static", "gSariasHouseBg2TLUT"),
        BG_TEX("vr_K5VR_pal_static", "gSariasHouseBg3TLUT") } },
};

// Face corners and texture steps (D_8012AEBC in OoT).
static const s32 sFaceParams[PANORAMA_FACES_MAX][5] = {
    { -0x7E, 0x7C, -0x7E, 0x3F, -0x1F },
    { 0x7E, 0x7C, -0x7E, 0x3F, -0x1F },
    { 0x7E, 0x7C, 0x7E, -0x3F, -0x1F },
    { -0x7E, 0x7C, 0x7E, -0x3F, -0x1F },
};

// Vertex order and grid (D_8012ACA0, D_8012AD20, D_8012AD2C and D_8012AD40 in OoT).
static const u16 sVtxIndices[2][0x20] = {
    { 0x00, 0x02, 0x0A, 0x0C, 0x02, 0x04, 0x0C, 0x0E, 0x0A, 0x0C, 0x14, 0x16, 0x0C, 0x0E, 0x16, 0x18,
      0x01, 0x03, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0B, 0x0D, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x15, 0x17 },
    { 0x14, 0x16, 0x1E, 0x20, 0x16, 0x18, 0x20, 0x22, 0x1E, 0x20, 0x28, 0x2A, 0x20, 0x22, 0x2A, 0x2C,
      0x15, 0x17, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27, 0x29, 0x2B },
};
static const s16 sTexS[5] = { 0x0000, 0x0FC0, 0x1F80, 0x2F40, 0x3F00 };
static const s16 sTexT[9] = { 0x0000, 0x07C0, 0x0F80, 0x1740, 0x1F00, 0x26C0, 0x2E80, 0x3640, 0x3E00 };
static const s16 sQuadIndices[0x40] = {
    0x00, 0x10, 0x13, 0x12, 0x10, 0x01, 0x14, 0x13, 0x01, 0x11, 0x15, 0x14, 0x11, 0x05, 0x16, 0x15,
    0x12, 0x13, 0x17, 0x02, 0x13, 0x14, 0x03, 0x17, 0x14, 0x15, 0x18, 0x03, 0x15, 0x16, 0x07, 0x18,
    0x02, 0x17, 0x1A, 0x19, 0x17, 0x03, 0x1B, 0x1A, 0x03, 0x18, 0x1C, 0x1B, 0x18, 0x07, 0x1D, 0x1C,
    0x19, 0x1A, 0x1E, 0x0A, 0x1A, 0x1B, 0x0B, 0x1E, 0x1B, 0x1C, 0x1F, 0x0B, 0x1C, 0x1D, 0x0F, 0x1F,
};

static const PanoramaInfo* sPanorama = NULL;
static Vtx sVtx[PANORAMA_FACES_MAX * 2 * 0x20];
static Gfx sDLists[PANORAMA_FACES_MAX * 2][150];

// Builds one face's two display lists, each half of the face (func_800ADBB0 in OoT).
static s32 Panorama_BuildFace(s32 vtxIndex, s32 x, s32 y, s32 z, s32 stepA, s32 stepB, s32 face) {
    s32 posX[9 * 5];
    s32 posY[9 * 5];
    s32 posZ[9 * 5];
    s32 texS[9 * 5];
    s32 texT[9 * 5];
    s32 i;
    s32 j;
    s32 k;
    s32 a;
    s32 b;
    s32 half;
    s32 quad;
    s32 tileS;
    s32 tileT = 0;
    s32 row;
    s32 col;
    Gfx* gfx;

    for (i = 0, k = 0, b = y; k < 9 * 5; i++, b += stepB) {
        for (j = 0, a = ((face == 0) || (face == 2)) ? x : z; j < 5; j++, k++, a += stepA) {
            if ((face == 0) || (face == 2)) {
                posX[k] = a;
                posZ[k] = z;
            } else {
                posX[k] = x;
                posZ[k] = a;
            }
            posY[k] = b;
            texS[k] = sTexS[j];
            texT[k] = sTexT[i];
        }
    }

    for (half = 0; half < 2; half++) {
        gfx = sDLists[face * 2 + half];

        for (i = 0; i < 0x20; i++) {
            u16 index = sVtxIndices[half][i];
            Vtx* vtx = &sVtx[vtxIndex + i];

            vtx->v.ob[0] = posX[index];
            vtx->v.ob[1] = posY[index];
            vtx->v.ob[2] = posZ[index];
            vtx->v.flag = 0;
            vtx->v.tc[0] = texS[index];
            vtx->v.tc[1] = texT[index];
            vtx->v.cn[0] = 255;
            vtx->v.cn[1] = 0;
            vtx->v.cn[2] = 0;
        }
        gSPVertex(gfx++, &sVtx[vtxIndex], 32, 0);
        vtxIndex += 0x20;
        gSPCullDisplayList(gfx++, 0, 15);

        for (quad = 0, row = 0; row < 4; row++, tileT += 0x1F) {
            for (tileS = 0, col = 0; col < 4; col++, tileS += 0x3F, quad += 4) {
                gDPLoadTextureTile(gfx++, sPanorama->textures[face], G_IM_FMT_CI, G_IM_SIZ_8b, 256, 0, tileS, tileT,
                                   tileS + 0x3F, tileT + 0x1F, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
                gSP1Quadrangle(gfx++, sQuadIndices[quad + 1], sQuadIndices[quad + 2], sQuadIndices[quad + 3],
                               sQuadIndices[quad + 0], 3);
            }
        }
        gSPEndDisplayList(gfx++);
    }
    return vtxIndex;
}

void Hyrule_InitPanorama(u8 ootSkyboxId) {
    s32 face;
    s32 vtxIndex = 0;

    sPanorama = NULL;
    for (s32 i = 0; i < ARRAY_COUNT(sPanoramas); i++) {
        if (sPanoramas[i].ootSkyboxId == ootSkyboxId) {
            sPanorama = &sPanoramas[i];
        }
    }
    if (sPanorama == NULL) {
        return;
    }
    for (face = 0; face < sPanorama->numFaces; face++) {
        vtxIndex = Panorama_BuildFace(vtxIndex, sFaceParams[face][0], sFaceParams[face][1], sFaceParams[face][2],
                                      sFaceParams[face][3], sFaceParams[face][4], face);
    }
}

void Hyrule_DrawPanorama(PlayState* play, f32 x, f32 y, f32 z) {
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    Mtx* mtx;
    s32 face;

    if ((sPanorama == NULL) || (GET_ACTIVE_CAM(play)->setting == CAM_SET_PREREND_FIXED)) {
        return;
    }

    OPEN_DISPS(gfxCtx);
    FrameInterpolation_RecordOpenChild(NULL, FrameInterpolation_GetCameraEpoch());

    Gfx_SetupDL40_Opa(gfxCtx);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, 0);
    gSPTexture(POLY_OPA_DISP++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);

    mtx = GRAPH_ALLOC(gfxCtx, sizeof(Mtx));
    Matrix_Translate(x, y, z, MTXMODE_NEW);
    Matrix_RotateYF(sPanorama->rotY, MTXMODE_APPLY);
    Matrix_ToMtx(mtx);
    gSPMatrix(POLY_OPA_DISP++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gDPSetColorDither(POLY_OPA_DISP++, G_CD_MAGICSQ);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_RGBA16);
    gDPSetTextureConvert(POLY_OPA_DISP++, G_TC_FILT);

    // Each face has its own palette.
    for (face = 0; face < sPanorama->numFaces; face++) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPLoadTLUT_pal256(POLY_OPA_DISP++, sPanorama->palettes[face]);
        gSPDisplayList(POLY_OPA_DISP++, sDLists[face * 2]);
        gSPDisplayList(POLY_OPA_DISP++, sDLists[face * 2 + 1]);
    }
    gDPPipeSync(POLY_OPA_DISP++);

    FrameInterpolation_RecordCloseChild();
    CLOSE_DISPS(gfxCtx);
}
