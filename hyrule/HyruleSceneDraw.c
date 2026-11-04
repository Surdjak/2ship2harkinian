// OoT's scene draw configs (z_scene_table.c), for OoT scenes. Ported from SoH; the draw configs of scenes not ported
// yet fall back to MM's default.

#include "global.h"
#include "HyruleSceneDraw.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"
#include "assets/oot/scenes/indoors/miharigoya/miharigoya_scene.h"

// OoT's SceneDrawConfig.
typedef enum OotSceneDrawConfig {
    /*  0 */ SDC_DEFAULT,
    /*  1 */ SDC_HYRULE_FIELD,
    /*  2 */ SDC_KAKARIKO_VILLAGE,
    /*  3 */ SDC_ZORAS_RIVER,
    /*  4 */ SDC_KOKIRI_FOREST,
    /*  5 */ SDC_LAKE_HYLIA,
    /*  6 */ SDC_ZORAS_DOMAIN,
    /*  7 */ SDC_ZORAS_FOUNTAIN,
    /*  8 */ SDC_GERUDO_VALLEY,
    /*  9 */ SDC_LOST_WOODS,
    /* 10 */ SDC_DESERT_COLOSSUS,
    /* 11 */ SDC_GERUDOS_FORTRESS,
    /* 12 */ SDC_HAUNTED_WASTELAND,
    /* 13 */ SDC_HYRULE_CASTLE,
    /* 14 */ SDC_DEATH_MOUNTAIN_TRAIL,
    /* 15 */ SDC_DEATH_MOUNTAIN_CRATER,
    /* 16 */ SDC_GORON_CITY,
    /* 17 */ SDC_LON_LON_RANCH,
    /* 18 */ SDC_FIRE_TEMPLE,
    /* 19 */ SDC_DEKU_TREE,
    /* 20 */ SDC_DODONGOS_CAVERN,
    /* 21 */ SDC_JABU_JABU,
    /* 22 */ SDC_FOREST_TEMPLE,
    /* 23 */ SDC_WATER_TEMPLE,
    /* 24 */ SDC_SHADOW_TEMPLE_AND_WELL,
    /* 25 */ SDC_SPIRIT_TEMPLE,
    /* 26 */ SDC_INSIDE_GANONS_CASTLE,
    /* 27 */ SDC_GERUDO_TRAINING_GROUND,
    /* 28 */ SDC_DEKU_TREE_BOSS,
    /* 29 */ SDC_WATER_TEMPLE_BOSS,
    /* 30 */ SDC_TEMPLE_OF_TIME,
    /* 31 */ SDC_GROTTOS,
    /* 32 */ SDC_CHAMBER_OF_THE_SAGES,
    /* 33 */ SDC_GREAT_FAIRYS_FOUNTAIN,
    /* 34 */ SDC_SHOOTING_GALLERY,
    /* 35 */ SDC_CASTLE_COURTYARD_GUARDS,
    /* 36 */ SDC_OUTSIDE_GANONS_CASTLE,
    /* 37 */ SDC_ICE_CAVERN,
    /* 38 */ SDC_GANONS_TOWER_COLLAPSE_EXTERIOR,
    /* 39 */ SDC_FAIRYS_FOUNTAIN,
    /* 40 */ SDC_THIEVES_HIDEOUT,
    /* 41 */ SDC_BOMBCHU_BOWLING_ALLEY,
    /* 42 */ SDC_ROYAL_FAMILYS_TOMB,
    /* 43 */ SDC_LAKESIDE_LABORATORY,
    /* 44 */ SDC_LON_LON_BUILDINGS,
    /* 45 */ SDC_MARKET_GUARD_HOUSE,
    /* 46 */ SDC_POTION_SHOP_GRANNY,
    /* 47 */ SDC_CALM_WATER,
    /* 48 */ SDC_GRAVE_EXIT_LIGHT_SHINING,
    /* 49 */ SDC_BESITU,
    /* 50 */ SDC_FISHING_POND,
    /* 51 */ SDC_GANONS_TOWER_COLLAPSE_INTERIOR,
    /* 52 */ SDC_INSIDE_GANONS_CASTLE_COLLAPSE,
} OotSceneDrawConfig;

static const u8 sOotSceneDrawConfigs[] = {
#include "HyruleOotSceneDrawConfigs.inc"
};

// OoT's roomCtx.unk_74, the draw config's data that cutscenes change, is MM's roomCtx.unk7A.

// SDC_KOKIRI_FOREST (OoT's func_8009E0B8).
static void Hyrule_SceneDrawKokiriForest(PlayState* play) {
    u32 gameplayFrames;
    u8 spA3;
    u16 spA0;
    Gfx* displayListHead;

    spA3 = 128;
    spA0 = 500;
    displayListHead = GRAPH_ALLOC(play->state.gfxCtx, 6 * sizeof(Gfx));

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 10) % 128, 32,
                                  32, 1, gameplayFrames % 128, (gameplayFrames * 10) % 128, 32, 32, -1, 10, 1, 10));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    if (gSaveContext.sceneLayer == 4) {
        spA3 = 255 - (u8)play->roomCtx.unk7A[0];
    } else if (gSaveContext.sceneLayer == 6) {
        spA0 = play->roomCtx.unk7A[0] + 500;
    } else if (((gSaveContext.sceneLayer < 4) || LINK_IS_ADULT_OOT) &&
               GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_KOKIRI_EMERALD_DEKU_TREE_DEAD_Oot)) {
        spA0 = 2150;
    }

    gSPSegment(POLY_OPA_DISP++, 0x0A, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 128, 128, 128, spA3);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_XLU_DISP++, 0x0B, displayListHead);
    gSPSegment(POLY_OPA_DISP++, 0x0B, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 128, 128, 128, spA0 * 0.1f);
    gSPEndDisplayList(displayListHead);

    gSPSegment(POLY_OPA_DISP++, 0x0C,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (s16)(-play->roomCtx.unk7A[0] * 0.02f), 32, 16, 1, 0,
                                  (s16)(-play->roomCtx.unk7A[0] * 0.02f), 32, 16, 0,
                                  play->roomCtx.unk7A[0] == 0 ? 0 : -1, 0, play->roomCtx.unk7A[0] == 0 ? 0 : 1));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_LOST_WOODS (OoT's func_8009EE44): Cojiro crows once in the Lost Woods.
static void Hyrule_SceneDrawLostWoods(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, gameplayFrames % 128, 0, 32, 16, 1, gameplayFrames % 128, 0,
                                  32, 16, 1, 0, 1, 0));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, gameplayFrames % 128, 32, 32, 1,
                                  gameplayFrames % 128, gameplayFrames % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    if ((play->roomCtx.unk7A[0] == 0) && (INV_CONTENT_OOT(ITEM_COJIRO_Oot) == ITEM_COJIRO_Oot)) {
        if (play->roomCtx.unk7A[1] == 50) {
            Player_PlaySfx(GET_PLAYER(play), NA_SE_EV_CHICKEN_CRY_M);
            play->roomCtx.unk7A[0] = 1;
        }
        play->roomCtx.unk7A[1]++;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GROTTOS (OoT's func_8009A45C).
static void Hyrule_SceneDrawGrottos(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 64, 256, 16, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - (gameplayFrames % 128), (gameplayFrames * 1) % 128, 32,
                                  32, 1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 32, 1, 0, 127 - (gameplayFrames * 1) % 128, 32, 32,
                                  0, 0, 0, -1));
    gSPSegment(POLY_OPA_DISP++, 0x0B, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 128, 32, 32, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0C,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 50) % 2048, 8, 512, 1, 0,
                                  (gameplayFrames * 60) % 2048, 8, 512, 0, 50, 0, 60));
    gSPSegment(POLY_OPA_DISP++, 0x0D,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0, (gameplayFrames * 1) % 128, 32, 32, 0, 0,
                                  0, 1));

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_FAIRYS_FOUNTAIN (OoT's func_8009ACA8).
static void Hyrule_SceneDrawFairysFountain(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));
    gSPSegment(POLY_XLU_DISP++, 0x09, Gfx_TexScrollEx(play->state.gfxCtx, 0, gameplayFrames % 64, 256, 16, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_CALM_WATER (OoT's func_8009CAC0).
static void Hyrule_SceneDrawCalmWater(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_HYRULE_CASTLE (OoT's func_8009F40C).
static void Hyrule_SceneDrawHyruleCastle(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 10) % 128, 32,
                                  32, 1, gameplayFrames % 128, (gameplayFrames * 10) % 128, 32, 32, -1, 10, 1, 10));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_TEMPLE_OF_TIME (OoT's func_80099BD8): the light through the windows, which the Door of Time's cutscenes
// brighten through OoT's roomCtx.unk_74 (MM's unk7A).
static void Hyrule_SceneDrawTempleOfTime(PlayState* play) {
    f32 temp;
    Gfx* displayListHead = GRAPH_ALLOC(play->state.gfxCtx, 18 * sizeof(Gfx));

    OPEN_DISPS(play->state.gfxCtx);

    temp = play->roomCtx.unk7A[0] / 255.0f;

    gSPSegment(POLY_XLU_DISP++, 0x08, displayListHead);
    gSPSegment(POLY_OPA_DISP++, 0x08, displayListHead);
    gDPSetPrimColor(displayListHead++, 0, 0, 255 - (u8)(185.0f * temp), 255 - (u8)(145.0f * temp),
                    255 - (u8)(105.0f * temp), 255);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_XLU_DISP++, 0x09, displayListHead);
    gSPSegment(POLY_OPA_DISP++, 0x09, displayListHead);
    gDPSetPrimColor(displayListHead++, 0, 0, 76 + (u8)(6.0f * temp), 76 + (u8)(34.0f * temp), 76 + (u8)(74.0f * temp),
                    255);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_OPA_DISP++, 0x0A, displayListHead);
    gSPSegment(POLY_XLU_DISP++, 0x0A, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 0, 0, 0, play->roomCtx.unk7A[0]);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_OPA_DISP++, 0x0B, displayListHead);
    gSPSegment(POLY_XLU_DISP++, 0x0B, displayListHead);
    gDPSetPrimColor(displayListHead++, 0, 0, 89 + (u8)(166.0f * temp), 89 + (u8)(166.0f * temp),
                    89 + (u8)(166.0f * temp), 255);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 0, 0, 0, play->roomCtx.unk7A[0]);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_OPA_DISP++, 0x0C, displayListHead);
    gSPSegment(POLY_XLU_DISP++, 0x0C, displayListHead);
    gDPSetPrimColor(displayListHead++, 0, 0, 255 + (u8)(179.0f * temp), 255 + (u8)(179.0f * temp),
                    255 + (u8)(179.0f * temp), 255);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 0, 0, 0, play->roomCtx.unk7A[0]);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_OPA_DISP++, 0x0D, displayListHead);
    gSPSegment(POLY_XLU_DISP++, 0x0D, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 0, 0, 0, play->roomCtx.unk7A[1]);
    gSPEndDisplayList(displayListHead);

    CLOSE_DISPS(play->state.gfxCtx);

    // The Master Sword's glint in layer 5 (OoT's gCustomLensFlareOn and its scale, intensity and fill).
    if (gSaveContext.sceneLayer == 5) {
        gCustomLensFlare1On = true;
        gCustomLensFlare1Pos.x = -20.0f;
        gCustomLensFlare1Pos.y = 1220.0f;
        gCustomLensFlare1Pos.z = -684.0f;
        D_801F4E44 = 10.0f;
        D_801F4E48 = 8.0f;
        D_801F4E4C = 200;
    }
}

// SDC_CHAMBER_OF_THE_SAGES (OoT's func_8009A798).
static void Hyrule_SceneDrawChamberOfTheSages(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 2) % 256, 64, 64, 0, 2));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - (gameplayFrames * 1) % 128, (gameplayFrames * 1) % 256,
                                  32, 64, 1, 0, 0, 32, 128, -1, 1, 0, 0));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GREAT_FAIRYS_FOUNTAIN (OoT's func_8009A9DC).
static void Hyrule_SceneDrawGreatFairysFountain(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 256, 32, 64,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 256, 32, 64, -1, 3, 1, 3));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_SHOOTING_GALLERY (OoT's func_8009B9BC).
static void Hyrule_SceneDrawShootingGallery(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, gameplayFrames % 64, 4, 16, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_BOMBCHU_BOWLING_ALLEY (OoT's func_8009D0E8).
static void Hyrule_SceneDrawBombchuBowlingAlley(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TexScrollEx(play->state.gfxCtx, 127 - (gameplayFrames * 4) % 128, 0, 32, 32, -4, 0));
    gSPSegment(POLY_OPA_DISP++, 0x09, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 5) % 64, 16, 16, 0, 5));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TexScrollEx(play->state.gfxCtx, 0, 63 - (gameplayFrames * 2) % 64, 16, 16, 0, -2));
    gSPSegment(POLY_XLU_DISP++, 0x0B,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 127 - (gameplayFrames * 3) % 128, 32, 32, 1, 0, 0, 32, 32,
                                  0, -1, 0, 0));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_MARKET_GUARD_HOUSE (OoT's func_8009D438): the windows show the market by day or night, and the ruins to adult
// Link.
static void* sGuardHouseView2TexturesOot[] = {
    gGuardHouseOutSideView1DayTex,
    gGuardHouseOutSideView1NightTex,
};
static void* sGuardHouseView1TexturesOot[] = {
    gGuardHouseOutSideView2DayTex,
    gGuardHouseOutSideView2NightTex,
};

static void Hyrule_SceneDrawMarketGuardHouse(PlayState* play) {
    s32 var;

    OPEN_DISPS(play->state.gfxCtx);

    if (LINK_IS_ADULT_OOT) {
        var = 1;
    } else {
        var = gSaveContext.save.isNight;
    }

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sGuardHouseView1TexturesOot[var]));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(sGuardHouseView2TexturesOot[var]));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_CASTLE_COURTYARD_GUARDS (OoT's func_8009BAA4).
static void Hyrule_SceneDrawCastleCourtyardGuards(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));

    if (play->sceneId == SCENE_CASTLE_COURTYARD_GUARDS_DAY_Oot) {
        gSPSegment(POLY_XLU_DISP++, 0x09,
                   Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 10) % 256, 32, 64, 0, 10));
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_OUTSIDE_GANONS_CASTLE (OoT's func_8009BC44).
static void Hyrule_SceneDrawOutsideGanonsCastle(PlayState* play) {
    u32 gameplayFrames;
    s8 envShade = Math_CosS((play->gameplayFrames * 1500) & 0xFFFF) * 0x7F;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;

    if (play->sceneId == SCENE_OUTSIDE_GANONS_CASTLE_Oot) {
        gSPSegment(POLY_XLU_DISP++, 0x09,
                   Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 256, 64, 64, 0, 1));
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 255 - (gameplayFrames * 1) % 256, 64, 64, 1, 0,
                                      (gameplayFrames * 1) % 256, 64, 64, 0, -1, 0, 1));
    }

    gSPSegment(POLY_OPA_DISP++, 0x0B,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 255 - (gameplayFrames * 1) % 128, (gameplayFrames * 1) % 128,
                                  32, 32, 1, (gameplayFrames * 1) % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1,
                                  1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    envShade = (envShade >> 1) + 192;
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, envShade, envShade, envShade, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_SceneDrawOot(PlayState* play) {
    s32 index = play->sceneId - SCENE_KOKIRI_FOREST_Oot;
    s32 config = ((index >= 0) && (index < ARRAY_COUNT(sOotSceneDrawConfigs))) ? sOotSceneDrawConfigs[index] : SDC_DEFAULT;

    switch (config) {
        case SDC_KOKIRI_FOREST:
            Hyrule_SceneDrawKokiriForest(play);
            break;
        case SDC_LOST_WOODS:
            Hyrule_SceneDrawLostWoods(play);
            break;
        case SDC_GROTTOS:
            Hyrule_SceneDrawGrottos(play);
            break;
        case SDC_FAIRYS_FOUNTAIN:
            Hyrule_SceneDrawFairysFountain(play);
            break;
        case SDC_CALM_WATER:
            Hyrule_SceneDrawCalmWater(play);
            break;
        case SDC_HYRULE_CASTLE:
            Hyrule_SceneDrawHyruleCastle(play);
            break;
        case SDC_TEMPLE_OF_TIME:
            Hyrule_SceneDrawTempleOfTime(play);
            break;
        case SDC_CHAMBER_OF_THE_SAGES:
            Hyrule_SceneDrawChamberOfTheSages(play);
            break;
        case SDC_GREAT_FAIRYS_FOUNTAIN:
            Hyrule_SceneDrawGreatFairysFountain(play);
            break;
        case SDC_SHOOTING_GALLERY:
            Hyrule_SceneDrawShootingGallery(play);
            break;
        case SDC_BOMBCHU_BOWLING_ALLEY:
            Hyrule_SceneDrawBombchuBowlingAlley(play);
            break;
        case SDC_MARKET_GUARD_HOUSE:
            Hyrule_SceneDrawMarketGuardHouse(play);
            break;
        case SDC_CASTLE_COURTYARD_GUARDS:
            Hyrule_SceneDrawCastleCourtyardGuards(play);
            break;
        case SDC_OUTSIDE_GANONS_CASTLE:
            Hyrule_SceneDrawOutsideGanonsCastle(play);
            break;
        default:
            // OoT's SDC_DEFAULT is MM's default; the others wait for their scenes.
            Scene_DrawConfigDefault(play);
            break;
    }
}
