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
    { 0x02, // SKYBOX_BAZAAR
      2,
      0.8f,
      { BG_TEX("vr_SP1a_static", "gBazaarBgTex"), BG_TEX("vr_SP1a_static", "gBazaar2BgTex") },
      { BG_TEX("vr_SP1a_pal_static", "gBazaarBgTLUT"), BG_TEX("vr_SP1a_pal_static", "gBazaarBg2TLUT") } },
    { 0x0B, // SKYBOX_HAPPY_MASK_SHOP
      2,
      0.8f,
      { BG_TEX("vr_FCVR_static", "gMaskShopBgTex"), BG_TEX("vr_FCVR_static", "gMaskShop2BgTex") },
      { BG_TEX("vr_FCVR_pal_static", "gMaskShopBgTLUT"), BG_TEX("vr_FCVR_pal_static", "gMaskShopBg2TLUT") } },
    { 0x17, // SKYBOX_POTION_SHOP_MARKET
      2,
      0.8f,
      { BG_TEX("vr_ALVR_static", "gMarketPotionShopBgTex"), BG_TEX("vr_ALVR_static", "gMarketPotionShop2BgTex") },
      { BG_TEX("vr_ALVR_pal_static", "gMarketPotionShopBgTLUT"),
        BG_TEX("vr_ALVR_pal_static", "gMarketPotionShopBg2TLUT") } },
    { 0x18, // SKYBOX_BOMBCHU_SHOP
      2,
      0.8f,
      { BG_TEX("vr_NSVR_static", "gBombchuShopBgTex"), BG_TEX("vr_NSVR_static", "gBombchuShop2BgTex") },
      { BG_TEX("vr_NSVR_pal_static", "gBombchuShopBgTLUT"), BG_TEX("vr_NSVR_pal_static", "gBombchuShopBg2TLUT") } },
    { 0x1A, // SKYBOX_HOUSE_RICHARD
      4,
      0.0f,
      { BG_TEX("vr_IPVR_static", "gRichardsHouseBgTex"), BG_TEX("vr_IPVR_static", "gRichardsHouse2BgTex"),
        BG_TEX("vr_IPVR_static", "gRichardsHouse3BgTex"), BG_TEX("vr_IPVR_static", "gRichardsHouse4BgTex") },
      { BG_TEX("vr_IPVR_pal_static", "gRichardsHouseBgTLUT"), BG_TEX("vr_IPVR_pal_static", "gRichardsHouseBg2TLUT"),
        BG_TEX("vr_IPVR_pal_static", "gRichardsHouseBg3TLUT"),
        BG_TEX("vr_IPVR_pal_static", "gRichardsHouseBg4TLUT") } },
    { 0x22, // SKYBOX_HOUSE_ALLEY
      3,
      0.0f,
      { BG_TEX("vr_KR3VR_static", "gBackAlleyHouseBgTex"), BG_TEX("vr_KR3VR_static", "gBackAlleyHouse2BgTex"),
        BG_TEX("vr_KR3VR_static", "gBackAlleyHouse3BgTex") },
      { BG_TEX("vr_KR3VR_pal_static", "gBackAlleyHouseBgTLUT"),
        BG_TEX("vr_KR3VR_pal_static", "gBackAlleyHouseBg2TLUT"),
        BG_TEX("vr_KR3VR_pal_static", "gBackAlleyHouseBg3TLUT") } },
    { 0x10, // SKYBOX_HOUSE_KAKARIKO
      4,
      0.0f,
      { BG_TEX("vr_KKRVR_static", "gCarpentersHouseBgTex"), BG_TEX("vr_KKRVR_static", "gCarpentersHouse2BgTex"), BG_TEX("vr_KKRVR_static", "gCarpentersHouse3BgTex"), BG_TEX("vr_KKRVR_static", "gCarpentersHouse4BgTex") },
      { BG_TEX("vr_KKRVR_pal_static", "gCarpentersHouseBgTLUT"), BG_TEX("vr_KKRVR_pal_static", "gCarpentersHouseBg2TLUT"), BG_TEX("vr_KKRVR_pal_static", "gCarpentersHouseBg3TLUT"), BG_TEX("vr_KKRVR_pal_static", "gCarpentersHouseBg4TLUT") } },
    { 0x13, // SKYBOX_GORON_SHOP
      2,
      0.8f,
      { BG_TEX("vr_GLVR_static", "gGoronShopBgTex"), BG_TEX("vr_GLVR_static", "gGoronShop2BgTex") },
      { BG_TEX("vr_GLVR_pal_static", "gGoronShopBgTLUT"), BG_TEX("vr_GLVR_pal_static", "gGoronShopBg2TLUT") } },
    { 0x14, // SKYBOX_ZORA_SHOP
      2,
      0.8f,
      { BG_TEX("vr_ZRVR_static", "gZoraShopBgTex"), BG_TEX("vr_ZRVR_static", "gZoraShop2BgTex") },
      { BG_TEX("vr_ZRVR_pal_static", "gZoraShopBgTLUT"), BG_TEX("vr_ZRVR_pal_static", "gZoraShopBg2TLUT") } },
    { 0x16, // SKYBOX_POTION_SHOP_KAKARIKO
      2,
      0.8f,
      { BG_TEX("vr_DGVR_static", "gKakPotionShopBgTex"), BG_TEX("vr_DGVR_static", "gKakPotionShop2BgTex") },
      { BG_TEX("vr_DGVR_pal_static", "gKakPotionShopBgTLUT"), BG_TEX("vr_DGVR_pal_static", "gKakPotionShopBg2TLUT") } },
    { 0x0F, // SKYBOX_STABLES
      4,
      0.0f,
      { BG_TEX("vr_MLVR_static", "gStableBgTex"), BG_TEX("vr_MLVR_static", "gStable2BgTex"), BG_TEX("vr_MLVR_static", "gStable3BgTex"), BG_TEX("vr_MLVR_static", "gStable4BgTex") },
      { BG_TEX("vr_MLVR_pal_static", "gStableBgTLUT"), BG_TEX("vr_MLVR_pal_static", "gStableBg2TLUT"), BG_TEX("vr_MLVR_pal_static", "gStableBg3TLUT"), BG_TEX("vr_MLVR_pal_static", "gStableBg4TLUT") } },
    { 0x1B, // SKYBOX_HOUSE_IMPA
      4,
      0.0f,
      { BG_TEX("vr_LBVR_static", "gCowHouseBgTex"), BG_TEX("vr_LBVR_static", "gCowHouse2BgTex"), BG_TEX("vr_LBVR_static", "gCowHouse3BgTex"), BG_TEX("vr_LBVR_static", "gCowHouse4BgTex") },
      { BG_TEX("vr_LBVR_pal_static", "gCowHouseBgTLUT"), BG_TEX("vr_LBVR_pal_static", "gCowHouseBg2TLUT"), BG_TEX("vr_LBVR_pal_static", "gCowHouseBg3TLUT"), BG_TEX("vr_LBVR_pal_static", "gCowHouseBg4TLUT") } },
    { 0x1C, // SKYBOX_TENT
      3,
      0.0f,
      { BG_TEX("vr_TTVR_static", "gCarpentersTentBgTex"), BG_TEX("vr_TTVR_static", "gCarpentersTent2BgTex"), BG_TEX("vr_TTVR_static", "gCarpentersTent3BgTex") },
      { BG_TEX("vr_TTVR_pal_static", "gCarpentersTentBgTLUT"), BG_TEX("vr_TTVR_pal_static", "gCarpentersTentBg2TLUT"), BG_TEX("vr_TTVR_pal_static", "gCarpentersTentBg3TLUT") } },
    { 0x04, // SKYBOX_MARKET_ADULT
      4,
      0.0f,
      { BG_TEX("vr_RUVR_static", "gMarketRuinsBgTex"), BG_TEX("vr_RUVR_static", "gMarketRuins2BgTex"),
        BG_TEX("vr_RUVR_static", "gMarketRuins3BgTex"), BG_TEX("vr_RUVR_static", "gMarketRuins4BgTex") },
      { BG_TEX("vr_RUVR_pal_static", "gMarketRuinsBgTLUT"), BG_TEX("vr_RUVR_pal_static", "gMarketRuinsBg2TLUT"),
        BG_TEX("vr_RUVR_pal_static", "gMarketRuinsBg3TLUT"), BG_TEX("vr_RUVR_pal_static", "gMarketRuinsBg4TLUT") } },
    { 0x09, // SKYBOX_MARKET_CHILD_DAY
      4,
      0.0f,
      { BG_TEX("vr_MDVR_static", "gMarketDayBgTex"), BG_TEX("vr_MDVR_static", "gMarketDay2BgTex"),
        BG_TEX("vr_MDVR_static", "gMarketDay3BgTex"), BG_TEX("vr_MDVR_static", "gMarketDay4BgTex") },
      { BG_TEX("vr_MDVR_pal_static", "gMarketDayBgTLUT"), BG_TEX("vr_MDVR_pal_static", "gMarketDayBg2TLUT"),
        BG_TEX("vr_MDVR_pal_static", "gMarketDayBg3TLUT"), BG_TEX("vr_MDVR_pal_static", "gMarketDayBg4TLUT") } },
    { 0x0A, // SKYBOX_MARKET_CHILD_NIGHT
      4,
      0.0f,
      { BG_TEX("vr_MNVR_static", "gMarketNightBgTex"), BG_TEX("vr_MNVR_static", "gMarketNight2BgTex"),
        BG_TEX("vr_MNVR_static", "gMarketNight3BgTex"), BG_TEX("vr_MNVR_static", "gMarketNight4BgTex") },
      { BG_TEX("vr_MNVR_pal_static", "gMarketNightBgTLUT"), BG_TEX("vr_MNVR_pal_static", "gMarketNightBg2TLUT"),
        BG_TEX("vr_MNVR_pal_static", "gMarketNightBg3TLUT"), BG_TEX("vr_MNVR_pal_static", "gMarketNightBg4TLUT") } },
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
