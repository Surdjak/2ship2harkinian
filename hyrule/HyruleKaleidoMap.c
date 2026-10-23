// OoT's world map page (KaleidoScope_DrawWorldMap in OoT's z_kaleido_map), the map page in Hyrule's scenes
// (docs/PAUSE.md, U7): Hyrule's map, clouds over the areas not visited yet, the current area boxed and named,
// and OoT's twelve points, which pulse where the story goes next. OoT's dungeon maps come with its dungeons.

#include "global.h"
#include "HyruleKaleidoMap.h"
#include "HyruleInventory.h"
#include "HyruleFlags.h"
#include "HyruleScenes.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/oot/textures/icon_item_static/icon_item_static.h"
#include "assets/oot/textures/icon_item_nes_static/icon_item_nes_static.h"
#include "assets/oot/textures/icon_item_field_static/icon_item_field_static.h"
#include "assets/oot/textures/map_name_static/map_name_static.h"

static TexturePtr sMapPageBgTextures[] = {
    // Column 0
    (TexturePtr)gPauseMap00Tex,
    (TexturePtr)gPauseMap01Tex,
    (TexturePtr)gPauseMap02Tex,
    (TexturePtr)gPauseMap03Tex,
    (TexturePtr)gPauseMap04Tex,
    // Column 1
    (TexturePtr)gPauseMap10ENGTex,
    (TexturePtr)gPauseMap11Tex,
    (TexturePtr)gPauseMap12Tex,
    (TexturePtr)gPauseMap13Tex,
    (TexturePtr)gPauseMap14Tex,
    // Column 2
    (TexturePtr)gPauseMap20Tex,
    (TexturePtr)gPauseMap21Tex,
    (TexturePtr)gPauseMap22Tex,
    (TexturePtr)gPauseMap23Tex,
    (TexturePtr)gPauseMap24Tex,
};

TexturePtr* Hyrule_GetMapPageBgTextures(void) {
    return sMapPageBgTextures;
}

s32 Hyrule_IsInHyrule(PlayState* play) {
    return Hyrule_GetScenePath(play->sceneId) != NULL;
}

#define WORLD_MAP_POINT_COUNT 12
#define WORLD_MAP_AREA_COUNT 22
#define WORLD_MAP_CLOUD_COUNT 16
#define WORLD_MAP_STRIP_COUNT 15 // 14 strips of 9 rows, then one of 2
#define WORLD_MAP_STRIP_HEIGHT 9

// Quads in worldMapOotVtx
#define QUAD_STRIP(i) (i)
#define QUAD_CLOUD(i) (WORLD_MAP_STRIP_COUNT + (i))
#define QUAD_POINT(i) (QUAD_CLOUD(WORLD_MAP_CLOUD_COUNT) + (i))
#define QUAD_AREA_BOX QUAD_POINT(WORLD_MAP_POINT_COUNT)
#define QUAD_AREA_NAME (QUAD_AREA_BOX + 1)
#define QUAD_TITLE (QUAD_AREA_NAME + 1)
#define QUAD_COUNT (QUAD_TITLE + 1)

// OoT's world map quads (D_8082AEC0, D_8082AF78, D_8082AAEC, D_8082AB2C in OoT's kaleido scope): clouds, then
// points, then the current area's name and the "current position" title.
static const s16 sCloudX[WORLD_MAP_CLOUD_COUNT] = { 47, -49, -17, -15, -9, 24, 43, 14,
                                                    9,  38, 82,  71,  -76, -87, -108, -54 };
static const s16 sCloudY[WORLD_MAP_CLOUD_COUNT] = { 15, 40, 11, 45, 52, 37, 36, 57, 54, 33, 31, 45, 32, 42, 49, -10 };
static const s16 sCloudWidths[WORLD_MAP_CLOUD_COUNT] = { 32, 112, 32, 48, 32, 32, 32, 48,
                                                         32, 64,  32, 48, 48, 48, 48, 64 };
static const s16 sCloudHeights[WORLD_MAP_CLOUD_COUNT] = { 24, 72, 13, 22, 19, 20, 19, 27,
                                                          14, 26, 22, 21, 49, 32, 45, 60 };
static const s16 sPointX[WORLD_MAP_POINT_COUNT] = { -93, -67, -56, -33, -10, 1, 14, 24, 35, 58, 74, 89 };
static const s16 sPointY[WORLD_MAP_POINT_COUNT] = { 31, 27, 15, -49, 8, 38, 7, 47, 30, 1, -9, 25 };
#define AREA_NAME_X 19
#define AREA_NAME_Y -32
#define TITLE_X 28
#define TITLE_Y -26

static TexturePtr sCloudTextures[WORLD_MAP_CLOUD_COUNT] = {
    (TexturePtr)gWorldMapCloud16Tex, (TexturePtr)gWorldMapCloud15Tex, (TexturePtr)gWorldMapCloud14Tex,
    (TexturePtr)gWorldMapCloud13Tex, (TexturePtr)gWorldMapCloud12Tex, (TexturePtr)gWorldMapCloud11Tex,
    (TexturePtr)gWorldMapCloud10Tex, (TexturePtr)gWorldMapCloud9Tex,  (TexturePtr)gWorldMapCloud8Tex,
    (TexturePtr)gWorldMapCloud7Tex,  (TexturePtr)gWorldMapCloud6Tex,  (TexturePtr)gWorldMapCloud5Tex,
    (TexturePtr)gWorldMapCloud4Tex,  (TexturePtr)gWorldMapCloud3Tex,  (TexturePtr)gWorldMapCloud2Tex,
    (TexturePtr)gWorldMapCloud1Tex,
};
// The area whose visit clears each cloud
static const u8 sCloudAreas[WORLD_MAP_CLOUD_COUNT] = { 0x05, 0x00, 0x13, 0x0E, 0x0F, 0x01, 0x02, 0x10,
                                                       0x12, 0x03, 0x07, 0x08, 0x09, 0x0C, 0x0B, 0x06 };

// The current area's box
static const s16 sAreaBoxX[WORLD_MAP_AREA_COUNT] = { -41, 19, 44, 40, 49, 51, -49, 83, 80, -67, 50,
                                                     -109, -76, -86, -10, -6, 19, 24, 11, -17, 37, -6 };
static const s16 sAreaBoxY[WORLD_MAP_AREA_COUNT] = { 30, 36, 35, 26, 7, 11, -31, 30, 38, 23, 2,
                                                     42, 40, 32, 38, 50, 57, 58, 56, 12, 36, 50 };
static const s16 sAreaBoxWidths[WORLD_MAP_AREA_COUNT] = { 96, 32, 32, 48, 48, 32, 48, 32, 32, 32, 16,
                                                          32, 32, 16, 32, 32, 32, 32, 32, 32, 16, 32 };
static const s16 sAreaBoxHeights[WORLD_MAP_AREA_COUNT] = { 59, 19, 13, 19, 38, 17, 38, 17, 13, 26, 16,
                                                           26, 26, 16, 19, 17, 26, 13, 17, 17, 16, 17 };
static TexturePtr sAreaBoxTextures[WORLD_MAP_AREA_COUNT] = {
    (TexturePtr)gWorldMapAreaBox7Tex, (TexturePtr)gWorldMapAreaBox1Tex, (TexturePtr)gWorldMapAreaBox4Tex,
    (TexturePtr)gWorldMapAreaBox6Tex, (TexturePtr)gWorldMapAreaBox2Tex, (TexturePtr)gWorldMapAreaBox3Tex,
    (TexturePtr)gWorldMapAreaBox2Tex, (TexturePtr)gWorldMapAreaBox3Tex, (TexturePtr)gWorldMapAreaBox4Tex,
    (TexturePtr)gWorldMapAreaBox5Tex, (TexturePtr)gWorldMapAreaBox8Tex, (TexturePtr)gWorldMapAreaBox5Tex,
    (TexturePtr)gWorldMapAreaBox5Tex, (TexturePtr)gWorldMapAreaBox8Tex, (TexturePtr)gWorldMapAreaBox1Tex,
    (TexturePtr)gWorldMapAreaBox3Tex, (TexturePtr)gWorldMapAreaBox5Tex, (TexturePtr)gWorldMapAreaBox4Tex,
    (TexturePtr)gWorldMapAreaBox3Tex, (TexturePtr)gWorldMapAreaBox3Tex, (TexturePtr)gWorldMapAreaBox8Tex,
    (TexturePtr)gWorldMapAreaBox3Tex,
};
static TexturePtr sAreaNames[WORLD_MAP_AREA_COUNT] = {
    (TexturePtr)gHyruleFieldPositionNameENGTex,        (TexturePtr)gKakarikoVillagePositionNameENGTex,
    (TexturePtr)gGraveyardPositionNameENGTex,          (TexturePtr)gZorasRiverPositionNameENGTex,
    (TexturePtr)gKokiriForestPositionNameENGTex,       (TexturePtr)gSacredForestMeadowPositionNameENGTex,
    (TexturePtr)gLakeHyliaPositionNameENGTex,          (TexturePtr)gZorasDomainPositionNameENGTex,
    (TexturePtr)gZorasFountainPositionNameENGTex,      (TexturePtr)gGerudoValleyPositionNameENGTex,
    (TexturePtr)gLostWoodsPositionNameENGTex,          (TexturePtr)gDesertColossusPositionNameENGTex,
    (TexturePtr)gGerudosFortressPositionNameENGTex,    (TexturePtr)gHauntedWastelandPositionNameENGTex,
    (TexturePtr)gMarketPositionNameENGTex,             (TexturePtr)gHyruleCastlePositionNameENGTex,
    (TexturePtr)gDeathMountainTrailPositionNameENGTex, (TexturePtr)gDeathMountainCraterPositionNameENGTex,
    (TexturePtr)gGoronCityPositionNameENGTex,          (TexturePtr)gLonLonRanchPositionNameENGTex,
    (TexturePtr)gQuestionMarkPositionNameENGTex,       (TexturePtr)gGanonsCastlePositionNameENGTex,
};
static TexturePtr sPointNames[WORLD_MAP_POINT_COUNT] = {
    (TexturePtr)gHauntedWastelandPointNameENGTex, (TexturePtr)gGerudosFortressPointNameENGTex,
    (TexturePtr)gGerudoValleyPointNameENGTex,     (TexturePtr)gHyliaLakesidePointNameENGTex,
    (TexturePtr)gLonLonRanchPointNameENGTex,      (TexturePtr)gMarketPointNameENGTex,
    (TexturePtr)gHyruleFieldPointNameENGTex,      (TexturePtr)gDeathMountainPointNameENGTex,
    (TexturePtr)gKakarikoVillagePointNameENGTex,  (TexturePtr)gLostWoodsPointNameENGTex,
    (TexturePtr)gKokiriForestPointNameENGTex,     (TexturePtr)gZorasDomainPointNameENGTex,
};

// The area of the current Hyrule scene (OoT's gSaveContext.worldMapArea), from its misc settings command.
static s16 sWorldMapArea = 0;

void Hyrule_SetWorldMapArea(PlayState* play, s32 area) {
    s32 ootSceneId = Hyrule_GetOotSceneId(play->sceneId);

    sWorldMapArea = area;
    // OoT's overworld scenes mark their area visited (Scene_CommandMiscSettings in OoT): Hyrule Field to
    // outside Ganon's Castle, and the market to the Temple of Time's exterior.
    if ((((ootSceneId >= 0x51) && (ootSceneId <= 0x64)) || ((ootSceneId >= 0x1B) && (ootSceneId <= 0x22))) &&
        (gSaveContext.save.cutsceneIndex < 0xFFF0) && (area < 32)) {
        gSaveContext.save.saveInfo.worldMapAreaDataOot |= 1u << area;
    }
}

TexturePtr Hyrule_GetWorldMapPointName(u16 point) {
    return (point < WORLD_MAP_POINT_COUNT) ? sPointNames[point] : NULL;
}

// OoT's points: 0 hidden, 1 shown, 2 pulsing where to go next (KaleidoScope_Update in OoT, on opening).
static u8 sPoints[WORLD_MAP_POINT_COUNT];

static void Hyrule_UpdateWorldMapPoints(void) {
    u32 visited = gSaveContext.save.saveInfo.worldMapAreaDataOot;
    s32 hookshot = Hyrule_InvContentOot(ITEM_HOOKSHOT_Oot) != ITEM_NONE;
    s32 longshot = Hyrule_InvContentOot(ITEM_LONGSHOT_Oot) == ITEM_LONGSHOT_Oot;
    s32 ironBoots = CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_BOOTS_Oot, EQUIP_OWNED_GORON_IRON);
    s32 i;

    for (i = 0; i < WORLD_MAP_POINT_COUNT; i++) {
        sPoints[i] = 0;
    }

    // Haunted Wasteland
    if (Hyrule_CheckQuestItemOot(QUEST_GERUDO_CARD_Oot)) {
        sPoints[0] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_MEDALLION_SPIRIT_Oot)) {
        sPoints[0] = 1;
    }
    // Gerudo's Fortress
    if (longshot) {
        sPoints[1] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_GERUDO_CARD_Oot)) {
        sPoints[1] = 1;
    }
    // Gerudo Valley
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_ENTERED_GERUDO_VALLEY_Oot)) {
        sPoints[2] = 1;
    }
    if (longshot) {
        sPoints[2] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_GERUDO_CARD_Oot)) {
        sPoints[2] = 1;
    }
    // Lake Hylia
    if (GET_CUR_UPG_VALUE(UPG_SCALE)) {
        sPoints[3] = 1;
    }
    if (ironBoots) {
        sPoints[3] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_MEDALLION_WATER_Oot)) {
        sPoints[3] = 1;
    }
    // Lon Lon Ranch
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DEKU_TREE_BLUE_WARP_Oot)) {
        sPoints[4] = 1;
    }
    if (Hyrule_InvContentOot(ITEM_OCARINA_FAIRY_Oot) != ITEM_NONE) {
        sPoints[4] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_EPONA_Oot)) {
        sPoints[4] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_WOKEN_IN_KAKARIKO_Oot)) {
        sPoints[4] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
        sPoints[4] = 1;
    }
    // Market
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DEKU_TREE_BLUE_WARP_Oot)) {
        sPoints[5] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
        sPoints[5] = 1;
    }
    if (Hyrule_InvContentOot(ITEM_OCARINA_TIME_Oot) == ITEM_OCARINA_TIME_Oot) {
        sPoints[5] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot)) {
        sPoints[5] = 1;
    }
    if (Hyrule_InvContentOot(ITEM_ARROW_LIGHT_Oot) != ITEM_NONE) {
        sPoints[5] = 2;
    }
    // Hyrule Field
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DEKU_TREE_BLUE_WARP_Oot)) {
        sPoints[6] = 1;
    }
    // Death Mountain
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
        sPoints[7] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
        sPoints[7] = 1;
    }
    if (hookshot) {
        sPoints[7] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FIRE_TEMPLE_BLUE_WARP_Oot)) {
        sPoints[7] = 1;
    }
    // Kakariko Village
    if (visited & (1 << 1)) {
        sPoints[8] = 1;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_LULLABY_Oot)) {
        sPoints[8] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_SUN_Oot)) {
        sPoints[8] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot)) {
        sPoints[8] = 2;
    }
    if (hookshot) {
        sPoints[8] = 1;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_STORMS_Oot)) {
        sPoints[8] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_DRAINED_WELL_IN_KAKARIKO_Oot)) {
        sPoints[8] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_BONGO_BONGO_ESCAPED_FROM_WELL_Oot)) {
        sPoints[8] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_MEDALLION_SHADOW_Oot)) {
        sPoints[8] = 1;
    }
    // Lost Woods
    if (visited & (1 << 10)) {
        sPoints[9] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_ABOUT_SARIA_Oot)) {
        sPoints[9] = 2;
    }
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_SARIA_Oot)) {
        sPoints[9] = 1;
    }
    if (hookshot) {
        sPoints[9] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP_Oot)) {
        sPoints[9] = 1;
    }
    // Kokiri Forest
    sPoints[10] = 2;
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DEKU_TREE_BLUE_WARP_Oot)) {
        sPoints[10] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_6E_Oot)) {
        sPoints[10] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_ABOUT_SARIA_Oot)) {
        sPoints[10] = 1;
    }
    // Zora's Domain
    if (Hyrule_CheckQuestItemOot(QUEST_SONG_LULLABY_Oot)) {
        sPoints[11] = 1;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
        sPoints[11] = 2;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_JABU_JABUS_BELLY_BLUE_WARP_Oot)) {
        sPoints[11] = 1;
    }
    if (hookshot) {
        sPoints[11] = 2;
    }
    if (ironBoots) {
        sPoints[11] = 1;
    }
}

static void Hyrule_SetMapQuad(Vtx* vtx, s16 x, s16 y, s16 width, s16 height, u8 alpha) {
    s32 i;

    for (i = 0; i < 4; i++) {
        vtx[i].v.ob[0] = x + ((i & 1) ? width : 0);
        vtx[i].v.ob[1] = y - ((i & 2) ? height : 0);
        vtx[i].v.ob[2] = 0;
        vtx[i].v.flag = 0;
        vtx[i].v.tc[0] = (i & 1) ? width << 5 : 0;
        vtx[i].v.tc[1] = (i & 2) ? height << 5 : 0;
        vtx[i].v.cn[0] = vtx[i].v.cn[1] = vtx[i].v.cn[2] = 255;
        vtx[i].v.cn[3] = alpha;
    }
}

void Hyrule_KaleidoSetWorldMapVertices(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 offsetY = pauseCtx->offsetY;
    s32 i;

    pauseCtx->worldMapOotVtx = GRAPH_ALLOC(play->state.gfxCtx, QUAD_COUNT * 4 * sizeof(Vtx));
    Vtx* vtx = pauseCtx->worldMapOotVtx;

    for (i = 0; i < WORLD_MAP_STRIP_COUNT; i++) {
        s16 height = (i < WORLD_MAP_STRIP_COUNT - 1) ? WORLD_MAP_STRIP_HEIGHT : 2;

        Hyrule_SetMapQuad(&vtx[QUAD_STRIP(i) * 4], -108, 58 - i * WORLD_MAP_STRIP_HEIGHT + offsetY, 216, height,
                          pauseCtx->alpha);
    }
    for (i = 0; i < WORLD_MAP_CLOUD_COUNT; i++) {
        Hyrule_SetMapQuad(&vtx[QUAD_CLOUD(i) * 4], sCloudX[i], sCloudY[i] + offsetY, sCloudWidths[i],
                          sCloudHeights[i], pauseCtx->alpha);
    }
    for (i = 0; i < WORLD_MAP_POINT_COUNT; i++) {
        Hyrule_SetMapQuad(&vtx[QUAD_POINT(i) * 4], sPointX[i], sPointY[i] + offsetY, 8, 8, pauseCtx->alpha);
    }
    if (sWorldMapArea < WORLD_MAP_AREA_COUNT) {
        Hyrule_SetMapQuad(&vtx[QUAD_AREA_BOX * 4], sAreaBoxX[sWorldMapArea], sAreaBoxY[sWorldMapArea] + offsetY,
                          sAreaBoxWidths[sWorldMapArea], sAreaBoxHeights[sWorldMapArea], pauseCtx->alpha);
    }
    Hyrule_SetMapQuad(&vtx[QUAD_AREA_NAME * 4], AREA_NAME_X, AREA_NAME_Y + offsetY, 80, 32, pauseCtx->alpha);
    Hyrule_SetMapQuad(&vtx[QUAD_TITLE * 4], TITLE_X, TITLE_Y + offsetY, 64, 8, pauseCtx->alpha);
}

void Hyrule_KaleidoDrawWorldMap(PlayState* play) {
    static s16 sPulseTimer = 20;
    static s16 sPulseStage = 1;
    static s16 sPulsePrim[3] = { 0, 0, 255 };
    static s16 sPulseEnv[3] = { 255, 255, 0 };
    static const s16 sPointPrims[2][3] = { { 0, 0, 255 }, { 255, 255, 0 } };
    static const s16 sPointEnvs[2][3] = { { 255, 255, 0 }, { 0, 0, 255 } };
    PauseContext* pauseCtx = &play->pauseCtx;
    Vtx* vtx = pauseCtx->worldMapOotVtx;
    u32 visited = gSaveContext.save.saveInfo.worldMapAreaDataOot;
    s32 i;

    Hyrule_UpdateWorldMapPoints();

    OPEN_DISPS(play->state.gfxCtx);

    // Hyrule's map, a CI8 image in strips
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_POINT);
    gDPLoadTLUT_pal256(POLY_OPA_DISP++, gWorldMapImageTLUT);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_RGBA16);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    for (i = 0; i < WORLD_MAP_STRIP_COUNT; i++) {
        s16 height = (i < WORLD_MAP_STRIP_COUNT - 1) ? WORLD_MAP_STRIP_HEIGHT : 2;

        gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_STRIP(i) * 4], 4, 0);
        gDPLoadMultiTile(POLY_OPA_DISP++, gWorldMapImageTex, 0, G_TX_RENDERTILE, G_IM_FMT_CI, G_IM_SIZ_8b, 216, 128, 0,
                         i * WORLD_MAP_STRIP_HEIGHT, 216 - 1, i * WORLD_MAP_STRIP_HEIGHT + height - 1, 0,
                         G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                         G_TX_NOLOD);
        gDPSetTileSize(POLY_OPA_DISP++, G_TX_RENDERTILE, 0, 0, (216 - 1) << G_TEXTURE_IMAGE_FRAC,
                       (height - 1) << G_TEXTURE_IMAGE_FRAC);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    }
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_NONE);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);

    // Clouds over the areas not visited
    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                      PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 235, 235, 235, pauseCtx->alpha);
    for (i = 0; i < WORLD_MAP_CLOUD_COUNT; i++) {
        if (!(visited & (1u << sCloudAreas[i]))) {
            gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_CLOUD(i) * 4], 4, 0);
            gDPLoadTextureBlock_4b(POLY_OPA_DISP++, sCloudTextures[i], G_IM_FMT_I, sCloudWidths[i], sCloudHeights[i], 0,
                                   G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    // The current area, boxed
    if (sWorldMapArea < WORLD_MAP_AREA_COUNT) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_POINT);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 255, 255, pauseCtx->alpha);
        gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_AREA_BOX * 4], 4, 0);
        gDPLoadTextureBlock_4b(POLY_OPA_DISP++, sAreaBoxTextures[sWorldMapArea], G_IM_FMT_IA,
                               sAreaBoxWidths[sWorldMapArea], sAreaBoxHeights[sWorldMapArea], 0,
                               G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);
    }

    // The points, pulsing between blue and yellow where the story goes next
    if (--sPulseTimer == 0) {
        sPulseStage ^= 1;
        sPulseTimer = 20;
    }
    for (i = 0; i < 3; i++) {
        sPulsePrim[i] += (sPointPrims[sPulseStage][i] - sPulsePrim[i]) / sPulseTimer;
        sPulseEnv[i] += (sPointEnvs[sPulseStage][i] - sPulseEnv[i]) / sPulseTimer;
    }
    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gWorldMapDotTex, G_IM_FMT_IA, G_IM_SIZ_8b, 8, 8, 0, G_TX_WRAP | G_TX_NOMIRROR,
                        G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (i = 0; i < WORLD_MAP_POINT_COUNT; i++) {
        if (sPoints[i] == 0) {
            continue;
        }
        gDPPipeSync(POLY_OPA_DISP++);
        if (sPoints[i] == 1) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sPointPrims[0][0], sPointPrims[0][1], sPointPrims[0][2],
                            pauseCtx->alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, sPointEnvs[0][0], sPointEnvs[0][1], sPointEnvs[0][2], 0);
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sPulsePrim[0], sPulsePrim[1], sPulsePrim[2], pauseCtx->alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, sPulseEnv[0], sPulseEnv[1], sPulseEnv[2], 0);
        }
        gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_POINT(i) * 4], 4, 0);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    }

    // The current area's name, under OoT's "current position"
    if (sWorldMapArea < WORLD_MAP_AREA_COUNT) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 150, 255, 255, pauseCtx->alpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_AREA_NAME * 4], 4, 0);
        POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, sAreaNames[sWorldMapArea], 80, 32, 0);
    }
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                      PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, pauseCtx->alpha);
    gSPVertex(POLY_OPA_DISP++, &vtx[QUAD_TITLE * 4], 4, 0);
    gDPLoadTextureBlock_4b(POLY_OPA_DISP++, gPauseCurrentPositionENGTex, G_IM_FMT_I, 64, 8, 0,
                           G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    CLOSE_DISPS(play->state.gfxCtx);
}

// The cursor moves along the shown points, OoT's way: left and right only, leaving the page at either end.
static s16 sCursorPoint = 10;

void Hyrule_KaleidoSetWorldMapCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;

    // The cursor's own view (KaleidoScope_DrawCursor), as for the other pages
    pauseCtx->cursorX = (sPointX[sCursorPoint] + 4) * 0.8125f + 3.0f;
    pauseCtx->cursorY = (sPointY[sCursorPoint] - 4) * 0.8125f - 3.125f;
    pauseCtx->cursorWidth = pauseCtx->cursorHeight = 10.0f;
}

static s32 Hyrule_FindWorldMapPoint(s32 from, s32 step) {
    s32 point;

    for (point = from; (point >= 0) && (point < WORLD_MAP_POINT_COUNT); point += step) {
        if (sPoints[point] != 0) {
            return point;
        }
    }
    return -1;
}

void Hyrule_KaleidoUpdateWorldMapCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 point;

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;

    if ((pauseCtx->state != PAUSE_STATE_MAIN) || (pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE) ||
        (pauseCtx->pageIndex != PAUSE_MAP)) {
        return;
    }
    Hyrule_UpdateWorldMapPoints();

    if (pauseCtx->cursorSpecialPos == 0) {
        if ((sPoints[sCursorPoint] == 0) && ((point = Hyrule_FindWorldMapPoint(0, 1)) >= 0)) {
            sCursorPoint = point;
        }
        if (pauseCtx->stickAdjX > 30) {
            point = Hyrule_FindWorldMapPoint(sCursorPoint + 1, 1);
            if (point < 0) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            } else {
                sCursorPoint = point;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        } else if (pauseCtx->stickAdjX < -30) {
            point = Hyrule_FindWorldMapPoint(sCursorPoint - 1, -1);
            if (point < 0) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            } else {
                sCursorPoint = point;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) && (pauseCtx->stickAdjX > 30)) {
        if ((point = Hyrule_FindWorldMapPoint(0, 1)) >= 0) {
            sCursorPoint = point;
            KaleidoScope_MoveCursorFromSpecialPos(play);
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT) && (pauseCtx->stickAdjX < -30)) {
        if ((point = Hyrule_FindWorldMapPoint(WORLD_MAP_POINT_COUNT - 1, -1)) >= 0) {
            sCursorPoint = point;
            KaleidoScope_MoveCursorFromSpecialPos(play);
        }
    }

    if ((pauseCtx->cursorSpecialPos != 0) || (sPoints[sCursorPoint] == 0)) {
        pauseCtx->cursorItem[PAUSE_MAP] = PAUSE_ITEM_NONE;
    } else {
        pauseCtx->cursorItem[PAUSE_MAP] = sCursorPoint;
        pauseCtx->cursorSlot[PAUSE_MAP] = sCursorPoint;
    }
}
