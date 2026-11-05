// OoT's scene draw configs (z_scene_table.c), for OoT scenes. Ported from SoH, without its Master Quest textures;
// OoT's SDC_DEFAULT is MM's default.

#include "global.h"
#include "HyruleSceneDraw.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"
#include "z64quake.h"
#include "assets/oot/scenes/dungeons/Bmori1/Bmori1_scene.h"
#include "assets/oot/scenes/dungeons/MIZUsin/MIZUsin_scene.h"
#include "assets/oot/scenes/dungeons/ddan/ddan_scene.h"
#include "assets/oot/scenes/dungeons/gerudoway/gerudoway_scene.h"
#include "assets/oot/scenes/dungeons/ice_doukutu/ice_doukutu_scene.h"
#include "assets/oot/scenes/dungeons/jyasinzou/jyasinzou_scene.h"
#include "assets/oot/scenes/dungeons/men/men_scene.h"
#include "assets/oot/scenes/dungeons/ydan/ydan_scene.h"
#include "assets/oot/scenes/indoors/miharigoya/miharigoya_scene.h"
#include "assets/oot/scenes/overworld/souko/souko_scene.h"
#include "assets/oot/scenes/overworld/spot00/spot00_room_0.h"
#include "assets/oot/scenes/overworld/spot01/spot01_scene.h"
#include "assets/oot/scenes/overworld/spot07/spot07_scene.h"
#include "assets/oot/scenes/overworld/spot12/spot12_scene.h"
#include "assets/oot/scenes/overworld/spot16/spot16_room_0.h"
#include "assets/oot/scenes/overworld/spot18/spot18_scene.h"
#include "assets/oot/scenes/overworld/spot20/spot20_scene.h"

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

// SDC_HYRULE_FIELD (OoT's func_8009DA30): a room display list fades in after dusk and out at dawn, through OoT's
// roomCtx.unk_74[0].
static void Hyrule_SceneDrawHyruleField(PlayState* play) {
    u32 gameplayFrames;
    Gfx* displayListHead;

    displayListHead = GRAPH_ALLOC(play->state.gfxCtx, 3 * sizeof(Gfx));

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 10) % 128, 32,
                                  32, 1, gameplayFrames % 128, (gameplayFrames * 10) % 128, 32, 32, -1, 10, 1, 10));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_XLU_DISP++, 0x0A, displayListHead);

    if ((gSaveContext.save.time >= 0x4AAC) && (gSaveContext.save.time <= 0xC555)) {
        gSPEndDisplayList(displayListHead);
    } else {
        if (gSaveContext.save.time > 0xC555) {
            if (play->roomCtx.unk7A[0] != 255) {
                Math_StepToS(&play->roomCtx.unk7A[0], 255, 5);
            }
        } else if (gSaveContext.save.time >= 0x4000) {
            if (play->roomCtx.unk7A[0] != 0) {
                Math_StepToS(&play->roomCtx.unk7A[0], 0, 10);
            }
        }

        gDPSetPrimColor(displayListHead++, 0, 0, 255, 255, 255, play->roomCtx.unk7A[0]);
        gSPDisplayList(displayListHead++, spot00_room_0DL_012B20);
        gSPEndDisplayList(displayListHead);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_KAKARIKO_VILLAGE (OoT's func_8009DD5C): the windows are lit at night.
static void* sKakarikoWindowTexturesOot[] = {
    gKakarikoVillageDayWindowTex,
    gKakarikoVillageNightWindowTex,
};

static void Hyrule_SceneDrawKakarikoVillage(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sKakarikoWindowTexturesOot[gSaveContext.save.isNight]));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_ZORAS_RIVER (OoT's func_8009DE78).
static void Hyrule_SceneDrawZorasRiver(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 6) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 6) % 128, 32, 32, -1, 6, 1, 6));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 3) % 128, 32, 32, -1, 3, 1, 3));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_LAKE_HYLIA (OoT's func_8009E54C): the water is opaque while the lake is drained (adult Link before the Water
// Temple, and the cutscene layers), through OoT's roomCtx.unk_74[0].
static void Hyrule_SceneDrawLakeHylia(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    if ((gSaveContext.sceneLayer > 3) ||
        (LINK_IS_ADULT_OOT && !GET_EVENTCHKINF_OOT(EVENTCHKINF_RAISED_LAKE_HYLIA_WATER_Oot))) {
        play->roomCtx.unk7A[0] = 87;
    }

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames, gameplayFrames, 32, 32, 1, 0, 0, 32,
                                          32, 0, 0, 0, play->roomCtx.unk7A[0] + 168, 1, 1, 0, 0));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, -gameplayFrames, -gameplayFrames, 32, 32, 1, 0, 0, 16,
                                          64, 0, 0, 0, play->roomCtx.unk7A[0] + 168, -1, -1, 0, 0));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_ZORAS_DOMAIN (OoT's func_8009E730): the waterfall stops for adult Link, the domain being frozen.
static void* sZorasDomainEntranceTexturesOot[] = {
    gZorasDomainDayEntranceTex,
    gZorasDomainNightEntranceTex,
};

static void Hyrule_SceneDrawZorasDomain(PlayState* play) {
    u32 gameplayFrames;
    u32 var;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    var = 127 - (gameplayFrames * 1) % 128;
    if (LINK_IS_ADULT_OOT) {
        var = 0;
    }
    gSPSegment(POLY_OPA_DISP++, 0x0C,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 64, 32, 1, 0, var, 64, 32, 0, 0, 0, -1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sZorasDomainEntranceTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_ZORAS_FOUNTAIN (OoT's func_8009E8C0).
static void Hyrule_SceneDrawZorasFountain(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(
        POLY_OPA_DISP++, 0x08,
        Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 128, 0, 32, 32, 1, 0, 0, 32, 32, 1, 0, 0, 0));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 255 - (gameplayFrames * 2) % 256, 64, 64, 1, 0,
                                  255 - (gameplayFrames * 2) % 256, 64, 64, 0, -2, 0, -2));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 128, 32, 32, 1, 0,
                                  (gameplayFrames * 1) % 128, 32, 32, 0, 1, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GERUDO_VALLEY (OoT's func_8009EAD8).
static void Hyrule_SceneDrawGerudoValley(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 3) % 1024, 32, 256, 1, 0,
                                  (gameplayFrames * 3) % 1024, 32, 256, 0, 3, 0, 3));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 256, 64, 64, 1, 0,
                                  (gameplayFrames * 1) % 256, 64, 64, 0, 1, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 2) % 128, 32, 32, 1, 0,
                                  (gameplayFrames * 2) % 128, 32, 32, 0, 2, 0, 2));
    gSPSegment(POLY_OPA_DISP++, 0x0B,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 32, 1, 0, 127 - (gameplayFrames * 3) % 128, 32, 32,
                                  0, 0, 0, -3));
    gSPSegment(POLY_XLU_DISP++, 0x0C,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 128, 32, 32, 1, 0,
                                  (gameplayFrames * 1) % 128, 32, 32, 0, 1, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0D,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 64, 16, 16, 1, 0,
                                  (gameplayFrames * 1) % 64, 16, 16, 0, 1, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DESERT_COLOSSUS (OoT's func_8009F074).
static void Hyrule_SceneDrawDesertColossus(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(
        POLY_OPA_DISP++, 0x08,
        Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 32, 1, 0, 127 - gameplayFrames % 128, 32, 32, 0, 0, 0, -1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GERUDOS_FORTRESS (OoT's func_8009F1B4): the walls of the cell the Gerudo throw Link in, by day or night.
static void* sGerudosFortressCellTexturesOot[] = {
    gSpot12_009678Tex,
    gSpot12_00DE78Tex,
};

static void Hyrule_SceneDrawGerudosFortress(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08,
               Lib_SegmentedToVirtual(sGerudosFortressCellTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_HAUNTED_WASTELAND (OoT's func_8009F270).
static void Hyrule_SceneDrawHauntedWasteland(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, gameplayFrames % 128, 32, 32, 1, 0, gameplayFrames % 128,
                                  32, 32, 0, 1, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, gameplayFrames % 128, 32, 32, 1, 0, gameplayFrames % 128,
                                  32, 32, 0, 1, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DEATH_MOUNTAIN_TRAIL (OoT's func_8009F5D4): a room display list fades in after dusk and out at dawn, through
// OoT's roomCtx.unk_74[0].
static void Hyrule_SceneDrawDeathMountainTrail(PlayState* play) {
    Gfx* displayListHead = GRAPH_ALLOC(play->state.gfxCtx, 3 * sizeof(Gfx));

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08, displayListHead);

    if ((gSaveContext.save.time >= 0x4AAC) && (gSaveContext.save.time <= 0xC000)) {
        gSPEndDisplayList(displayListHead);
    } else {
        if (gSaveContext.save.time > 0xC000) {
            if (play->roomCtx.unk7A[0] != 255) {
                Math_StepToS(&play->roomCtx.unk7A[0], 255, 5);
            }
        } else if (gSaveContext.save.time >= 0x4000) {
            if (play->roomCtx.unk7A[0] != 0) {
                Math_StepToS(&play->roomCtx.unk7A[0], 0, 10);
            }
        }

        gDPSetPrimColor(displayListHead++, 0, 0, 255, 255, 255, play->roomCtx.unk7A[0]);
        gSPDisplayList(displayListHead++, spot16_room_0DL_00AA48);
        gSPEndDisplayList(displayListHead);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DEATH_MOUNTAIN_CRATER (OoT's func_8009F7D4): the lava's glow pulses.
static void Hyrule_SceneDrawDeathMountainCrater(PlayState* play) {
    s8 envR = coss((play->gameplayFrames * 1500) & 0xFFFF) >> 8;
    s8 envG = coss((play->gameplayFrames * 1500) & 0xFFFF) >> 8;
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    envR = (envR >> 1) + 192;
    envG = (envG >> 1) + 192;

    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, gameplayFrames % 128, 32, 32, 1, 0, gameplayFrames % 128,
                                  32, 32, 0, 1, 0, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, envR, envG, 255, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GORON_CITY (OoT's func_8009F9D0).
static void* sGoronCityEntranceTexturesOot[] = {
    gGoronCityDayEntranceTex,
    gGoronCityNightEntranceTex,
};

static void Hyrule_SceneDrawGoronCity(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 127 - gameplayFrames % 128, 32, 32, 1, gameplayFrames % 128,
                                  0, 32, 32, 0, -1, 1, 0));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sGoronCityEntranceTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_LON_LON_RANCH (OoT's func_8009FB74): the windows are lit at night.
static void* sLonLonRanchWindowTexturesOot[] = {
    gLonLonRanchDayWindowTex,
    gLonLonRangeNightWindowsTex,
};

static void Hyrule_SceneDrawLonLonRanch(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08,
               Lib_SegmentedToVirtual(sLonLonRanchWindowTexturesOot[gSaveContext.save.isNight]));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_FIRE_TEMPLE (OoT's func_8009FC90).
static void Hyrule_SceneDrawFireTemple(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 127 - gameplayFrames % 128, 32, 32, 1,
                                  127 - gameplayFrames % 128, 0, 32, 32, 0, -1, -1, 0));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 3) % 128, 127 - (gameplayFrames * 6) % 128,
                                  32, 32, 1, (gameplayFrames * 6) % 128, 127 - (gameplayFrames * 3) % 128, 32, 32, 3,
                                  -6, 6, -3));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 64);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 64);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DEKU_TREE (OoT's func_800995DC).
static void* sDekuTreeEntranceTexturesOot[] = {
    gYdanTex_00BA18,
    gYdanTex_00CA18,
};

static void Hyrule_SceneDrawDekuTree(PlayState* play) {
    u32 gameplayFrames = play->gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - (gameplayFrames % 128), (gameplayFrames * 1) % 128, 32,
                                  32, 1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sDekuTreeEntranceTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DODONGOS_CAVERN (OoT's func_80099878): the lava floor's frames, and the skull's eyes that Bg_Dodoago lights
// through OoT's roomCtx.unk_74 (BGDODOAGO_EYE_LEFT 0, BGDODOAGO_EYE_RIGHT 1).
static void* sDodongosCavernEntranceTexturesOot[] = {
    gDCDayEntranceTex,
    gDCNightEntranceTex,
};
static void* sDodongosCavernLavaFloorTexturesOot[] = {
    gDCLavaFloor1Tex, gDCLavaFloor2Tex, gDCLavaFloor3Tex, gDCLavaFloor4Tex,
    gDCLavaFloor5Tex, gDCLavaFloor6Tex, gDCLavaFloor7Tex, gDCLavaFloor8Tex,
};

static void Hyrule_SceneDrawDodongosCavern(PlayState* play) {
    u32 gameplayFrames;
    Gfx* displayListHead = GRAPH_ALLOC(play->state.gfxCtx, 2 * sizeof(Gfx[3]));

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Lib_SegmentedToVirtual(sDodongosCavernEntranceTexturesOot[gSaveContext.save.isNight]));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Lib_SegmentedToVirtual(sDodongosCavernLavaFloorTexturesOot[(s32)(gameplayFrames & 14) >> 1]));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 256, 0, 64, 32, 1, 0,
                                  (gameplayFrames * 1) % 128, 64, 32, 1, 0, 0, 1));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 128, 32, 32, 1, 0,
                                  (gameplayFrames * 2) % 128, 32, 32, 0, 1, 0, 2));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gSPSegment(POLY_OPA_DISP++, 0x0B, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 255, 255, 255, play->roomCtx.unk7A[0]);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_OPA_DISP++, 0x0C, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 255, 255, 255, play->roomCtx.unk7A[1]);
    gSPEndDisplayList(displayListHead);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_JABU_JABU (OoT's func_8009FE58): the walls wobble, through the view's distortion and a scale matrix in
// segment 0x0D; Jabu-Jabu's swallow swells them through OoT's roomCtx.unk_74 (0: state, 1: strength).
static f32 sJabuJabuWobbleOot = 0.0f; // OoT's D_8012A398

static void Hyrule_SceneDrawJabuJabu(PlayState* play) {
    static s16 sDistortionAngle1 = 538;  // OoT's D_8012A39C
    static s16 sDistortionAngle2 = 4272; // OoT's D_8012A3A0
    u32 gameplayFrames;
    f32 temp;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    if (play->sceneId == SCENE_JABU_JABU_Oot) {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, gameplayFrames % 128, (gameplayFrames * 2) % 128, 32, 32,
                                      1, 127 - gameplayFrames % 128, (gameplayFrames * 2) % 128, 32, 32, 1, 2, -1, 2));
        gSPSegment(POLY_OPA_DISP++, 0x0B,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 255 - (gameplayFrames * 4) % 256, 32, 64, 1, 0,
                                      255 - (gameplayFrames * 4) % 256, 32, 64, 0, -4, 0, -4));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TexScrollEx(play->state.gfxCtx, (127 - (gameplayFrames * 1)) % 128, (gameplayFrames * 1) % 128,
                                   32, 32, -1, 1));
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    if (FrameAdvance_IsEnabled(play) != true) {
        sDistortionAngle1 += 1820;
        sDistortionAngle2 += 1820;

        temp = 0.020000001f;

        // OoT stops drawing the scene once the pause menu's background is captured, so the wobble holds still while
        // paused; the port keeps drawing it (as SoH, which checks the pause state likewise).
        if (play->pauseCtx.state == PAUSE_STATE_OFF) {
            View_SetDistortionOrientation(&play->view,
                                          ((360.00018f / 65535.0f) * (M_PI / 180.0f)) * temp *
                                              Math_CosS(sDistortionAngle1),
                                          ((360.00018f / 65535.0f) * (M_PI / 180.0f)) * temp *
                                              Math_SinS(sDistortionAngle1),
                                          ((360.00018f / 65535.0f) * (M_PI / 180.0f)) * temp *
                                              Math_SinS(sDistortionAngle2));
            View_SetDistortionScale(&play->view, 1.f + (0.79999995f * temp * Math_SinS(sDistortionAngle2)),
                                    1.f + (0.39999998f * temp * Math_CosS(sDistortionAngle2)),
                                    1.f + (1 * temp * Math_CosS(sDistortionAngle1)));
            View_SetDistortionSpeed(&play->view, 0.95f);
        }

        switch (play->roomCtx.unk7A[0]) {
            case 0:
                break;
            case 1:
                if (play->roomCtx.unk7A[1] < 1200) {
                    play->roomCtx.unk7A[1] += 200;
                } else {
                    play->roomCtx.unk7A[0]++;
                }
                break;
            case 2:
                if (play->roomCtx.unk7A[1] > 0) {
                    play->roomCtx.unk7A[1] -= 30;
                } else {
                    play->roomCtx.unk7A[1] = 0;
                    play->roomCtx.unk7A[0] = 0;
                }
                break;
        }

        if (play->pauseCtx.state == PAUSE_STATE_OFF) {
            sJabuJabuWobbleOot += 0.15f + (play->roomCtx.unk7A[1] * 0.001f);
        }
    }

    if (play->roomCtx.curRoom.num == 2) {
        Matrix_Scale(1.0f, sinf(sJabuJabuWobbleOot) * 0.8f, 1.0f, MTXMODE_NEW);
    } else {
        Matrix_Scale(1.005f, sinf(sJabuJabuWobbleOot) * 0.8f, 1.005f, MTXMODE_NEW);
    }

    gSPSegment(POLY_OPA_DISP++, 0x0D, Matrix_Finalize(play->state.gfxCtx));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_FOREST_TEMPLE (OoT's func_8009D758).
static void* sForestTempleEntranceTexturesOot[] = {
    gForestTempleDayEntranceTex,
    gForestTempleNightEntranceTex,
};

static void Hyrule_SceneDrawForestTemple(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sForestTempleEntranceTexturesOot[gSaveContext.save.isNight]));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_WATER_TEMPLE (OoT's func_8009B0FC): the water's surfaces at each level, from OoT's roomCtx.unk_74[1] (the
// level in the high byte, its fade in the low byte).
static void* sWaterTempleEntranceTexturesOot[] = {
    gWaterTempleDayEntranceTex,
    gWaterTempleNightEntranceTex,
};

static void Hyrule_SceneDrawWaterTemple(PlayState* play) {
    u32 gameplayFrames;
    s32 level;
    s32 fade;

    OPEN_DISPS(play->state.gfxCtx);

    level = (play->roomCtx.unk7A[1] >> 8) & 0xFF;
    fade = play->roomCtx.unk7A[1] & 0xFF;
    gameplayFrames = play->gameplayFrames;

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sWaterTempleEntranceTexturesOot[gSaveContext.save.isNight]));

    if (level == 1) {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, fade, 1, 0, 0, 0));
    } else if (level < 1) {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 255, 1, 0, 0, 0));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 160, 1, 0, 0, 0));
    }

    if (level == 2) {
        gSPSegment(POLY_OPA_DISP++, 0x09,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, fade, 1, 0, 0, 0));
    } else if (level < 2) {
        gSPSegment(POLY_OPA_DISP++, 0x09,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 255, 1, 0, 0, 0));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x09,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 160, 1, 0, 0, 0));
    }

    if (level != 0) {
        gSPSegment(POLY_OPA_DISP++, 0x0A,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 160, 1, 0, 0, 0));
        gSPSegment(POLY_OPA_DISP++, 0x0B,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 3, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 180, 3, 0, 0, 0));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x0A,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 128, 0, 32, 32, 1, 0, 0, 32,
                                              32, 0, 0, 0, 160 + (s32)((fade / 200.0f) * 95.0f), 1, 0, 0, 0));
        gSPSegment(POLY_OPA_DISP++, 0x0B,
                   Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 3, 0, 32, 32, 1, 0, 0, 32, 32, 0,
                                              0, 0, 185 + (s32)((fade / 200.0f) * 70.0f), 3, 0, 0, 0));
    }

    gSPSegment(POLY_XLU_DISP++, 0x0C,
               Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 1, gameplayFrames * 1, 32, 32, 1, 0,
                                          127 - (gameplayFrames * 1), 32, 32, 0, 0, 0, 128, 1, 1, 0, -1));
    gSPSegment(POLY_XLU_DISP++, 0x0D,
               Gfx_TwoTexScrollEnvColorEx(play->state.gfxCtx, 0, gameplayFrames * 4, 0, 32, 32, 1, gameplayFrames * 4,
                                          0, 32, 32, 0, 0, 0, 128, 4, 0, 4, 0));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_SHADOW_TEMPLE_AND_WELL (OoT's func_8009AE30): Bongo Bongo's room scrolls its opaque layer.
static void Hyrule_SceneDrawShadowTempleAndWell(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;

    if (play->sceneId == SCENE_SHADOW_TEMPLE_BOSS_Oot) {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 2) % 128, 0, 32, 32, 1,
                                      (gameplayFrames * 2) % 128, 0, 32, 32, 2, 0, 2, 0));
    } else {
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 2) % 128, 0, 32, 32, 1,
                                      (gameplayFrames * 2) % 128, 0, 32, 32, 2, 0, 2, 0));
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_SPIRIT_TEMPLE (OoT's func_8009D974).
static void* sSpiritTempleEntranceTexturesOot[] = {
    gSpiritTempleDayEntranceTex,
    gSpiritTempleNightEntranceTex,
};

static void Hyrule_SceneDrawSpiritTemple(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sSpiritTempleEntranceTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_INSIDE_GANONS_CASTLE (OoT's func_800A0334).
static void Hyrule_SceneDrawInsideGanonsCastle(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 512, 32,
                                  128, 1, gameplayFrames % 128, (gameplayFrames * 1) % 512, 32, 128, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GERUDO_TRAINING_GROUND (OoT's func_8009CC00).
static void* sGerudoTrainingGroundEntranceTexturesOot[] = {
    gGTGDayEntranceTex,
    gGTGNightEntranceTex,
};

static void Hyrule_SceneDrawGerudoTrainingGround(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sGerudoTrainingGroundEntranceTexturesOot[gSaveContext.save.isNight]));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_DEKU_TREE_BOSS (OoT's func_80099760).
static void Hyrule_SceneDrawDekuTreeBoss(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 2) % 256, 0, 64, 32, 1, 0,
                                  (gameplayFrames * 2) % 128, 64, 32, 0, 2, 0, 2));

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_WATER_TEMPLE_BOSS (OoT's func_8009B86C): Morpha's room fades its floor through OoT's roomCtx.unk_74[0].
static void Hyrule_SceneDrawWaterTempleBoss(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, gameplayFrames * 1, 0, 32, 32, 1, 0, 0, 32, 32, 1, 0, 0, 0));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, play->roomCtx.unk7A[0]);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 145);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_ICE_CAVERN (OoT's func_8009C3EC).
static void* sIceCavernEntranceTexturesOot[] = {
    gIceCavernDayEntranceTex,
    gIceCavernNightEntranceTex,
};

static void Hyrule_SceneDrawIceCavern(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sIceCavernEntranceTexturesOot[gSaveContext.save.isNight]));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// OoT's func_8009BEEC: the collapsing tower shakes the camera now and then.
static void Hyrule_SceneQuakeGanonsTowerCollapse(PlayState* play) {
    s32 quakeIndex;

    if (play->gameplayFrames % 128 == 13) {
        quakeIndex = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_2);
        Quake_SetSpeed(quakeIndex, 10000);
        Quake_SetPerturbations(quakeIndex, 4, 0, 0, 0);
        Quake_SetDuration(quakeIndex, 127);
    }

    if ((play->gameplayFrames % 64 == 0) && (Rand_ZeroOne() > 0.6f)) {
        quakeIndex = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);
        // Through s32, so a speed past 0x7FFF wraps as OoT's does.
        Quake_SetSpeed(quakeIndex, (s32)(32000.0f + (Rand_ZeroOne() * 3000.0f)));
        Quake_SetPerturbations(quakeIndex, 10.0f - (Rand_ZeroOne() * 9.0f), 0, 0, 0);
        Quake_SetDuration(quakeIndex, 48.0f - (Rand_ZeroOne() * 15.0f));
    }
}

// SDC_GANONS_TOWER_COLLAPSE_EXTERIOR (OoT's func_8009C0AC): the tower shakes once it starts to collapse (switch flag
// 0x37).
static void Hyrule_SceneDrawGanonsTowerCollapseExterior(PlayState* play) {
    u32 gameplayFrames;
    s8 envShade = coss((play->gameplayFrames * 1500) & 0xFFFF) >> 8;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 512, 64, 128, 1, 0,
                                  511 - (gameplayFrames * 1) % 512, 64, 128, 0, 1, 0, -1));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 1) % 256, 32, 64, 1, 0,
                                  255 - (gameplayFrames * 1) % 256, 32, 64, 0, 1, 0, -1));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 20) % 2048, 16, 512, 1, 0,
                                  (gameplayFrames * 30) % 2048, 16, 512, 0, 20, 0, 30));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    envShade = (envShade >> 1) + 192;
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, envShade, envShade, envShade, 128);

    CLOSE_DISPS(play->state.gfxCtx);

    if (Flags_GetSwitch(play, 0x37)) {
        if ((play->sceneId == SCENE_GANON_BOSS_Oot) || (play->sceneId == SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR_Oot) ||
            (play->sceneId == SCENE_GANONS_TOWER_COLLAPSE_INTERIOR_Oot) ||
            (play->sceneId == SCENE_INSIDE_GANONS_CASTLE_COLLAPSE_Oot)) {
            Hyrule_SceneQuakeGanonsTowerCollapse(play);
        }
    }
}

// SDC_THIEVES_HIDEOUT (OoT's func_8009AFE0).
static void* sThievesHideoutEntranceTexturesOot[] = {
    gThievesHideoutDayEntranceTex,
    gThievesHideoutNightEntranceTex,
};

static void Hyrule_SceneDrawThievesHideout(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x09, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 3) % 128, 32, 32, 0, 3));

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sThievesHideoutEntranceTexturesOot[gSaveContext.save.isNight]));

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_ROYAL_FAMILYS_TOMB (OoT's func_8009C608).
static void Hyrule_SceneDrawRoyalFamilysTomb(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 1) % 64, 256, 16, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (gameplayFrames * 60) % 2048, 8, 512, 1, 0,
                                  (gameplayFrames * 50) % 2048, 8, 512, 0, 60, 0, 50));
    gSPSegment(POLY_OPA_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - (gameplayFrames * 1) % 128, 0, 32, 32, 1,
                                  (gameplayFrames * 1) % 128, 0, 32, 32, -1, 0, 1, 0));
    gSPSegment(POLY_XLU_DISP++, 0x0B,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 1023 - (gameplayFrames * 6) % 1024, 16, 256, 1, 0,
                                  1023 - (gameplayFrames * 3) % 1024, 16, 256, 0, -6, 0, -3));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_LAKESIDE_LABORATORY (OoT's func_8009C8B8).
static void Hyrule_SceneDrawLakesideLaboratory(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(
        POLY_OPA_DISP++, 0x08,
        Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 32, 1, 0, (gameplayFrames * 1) % 128, 32, 32, 0, 0, 0, 1));
    gSPSegment(POLY_XLU_DISP++, 0x0A,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 127 - gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32,
                                  1, gameplayFrames % 128, (gameplayFrames * 1) % 128, 32, 32, -1, 1, 1, 1));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TexScrollEx(play->state.gfxCtx, 0, 255 - (gameplayFrames * 10) % 256, 32, 64, 0, -10));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_LON_LON_BUILDINGS (OoT's func_8009D31C).
static void* sLonLonHouseEntranceTexturesOot[] = {
    gLonLonHouseDayEntranceTex,
    gLonLonHouseNightEntranceTex,
};

static void Hyrule_SceneDrawLonLonBuildings(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Lib_SegmentedToVirtual(sLonLonHouseEntranceTexturesOot[gSaveContext.save.isNight]));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_POTION_SHOP_GRANNY (OoT's func_8009D5B4).
static void Hyrule_SceneDrawPotionShopGranny(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 3) % 128, 32, 32, 0, 3));
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 1023 - (gameplayFrames * 3) % 1024, 16, 256, 1, 0,
                                  1023 - (gameplayFrames * 6) % 1024, 16, 256, 0, -3, 0, -6));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_GRAVE_EXIT_LIGHT_SHINING (OoT's func_8009AB98).
static void Hyrule_SceneDrawGraveExitLightShining(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08, Gfx_TexScrollEx(play->state.gfxCtx, 0, gameplayFrames % 64, 256, 16, 0, 1));

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// SDC_BESITU (OoT's func_800A059C).
static void Hyrule_SceneDrawBesitu(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TexScrollEx(play->state.gfxCtx, 127 - (gameplayFrames * 2) % 128, 0, 32, 64, -2, 0));
    gSPSegment(POLY_OPA_DISP++, 0x09,
               Gfx_TexScrollEx(play->state.gfxCtx, 0, (gameplayFrames * 2) % 512, 128, 128, 0, 2));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

// PORT: OoT's Gfx_TwoTexScrollPrimColor (z_scene_table.c), which MM does not have: two tile scrolls and a primitive
// color.
static Gfx* Gfx_TwoTexScrollPrimColorOot(GraphicsContext* gfxCtx, s32 tile1, u32 x1, u32 y1, s32 width1, s32 height1,
                                         s32 tile2, u32 x2, u32 y2, s32 width2, s32 height2, s32 r, s32 g, s32 b,
                                         s32 a) {
    Gfx* displayList = GRAPH_ALLOC(gfxCtx, 10 * sizeof(Gfx));

    x1 %= 2048;
    y1 %= 2048;
    x2 %= 2048;
    y2 %= 2048;

    gDPTileSync(displayList);
    gDPSetTileSize(displayList + 1, tile1, x1, y1, (x1 + ((width1 - 1) << 2)), (y1 + ((height1 - 1) << 2)));
    gDPTileSync(displayList + 2);
    gDPSetTileSize(displayList + 3, tile2, x2, y2, (x2 + ((width2 - 1) << 2)), (y2 + ((height2 - 1) << 2)));
    gDPSetPrimColor(displayList + 4, 0, 0, r, g, b, a);
    gSPEndDisplayList(displayList + 5);

    return displayList;
}

// SDC_FISHING_POND (OoT's func_8009CF84): the water's alpha follows OoT's roomCtx.unk_74[0].
static void Hyrule_SceneDrawFishingPond(PlayState* play) {
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollPrimColorOot(play->state.gfxCtx, 0, 127 - gameplayFrames % 128,
                                            (gameplayFrames * 1) % 128, 32, 32, 1, gameplayFrames % 128,
                                            (gameplayFrames * 1) % 128, 32, 32, 255, 255, 255,
                                            play->roomCtx.unk7A[0] + 127));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_SceneDrawOot(PlayState* play) {
    s32 index = play->sceneId - SCENE_KOKIRI_FOREST_Oot;
    s32 config = ((index >= 0) && (index < ARRAY_COUNT(sOotSceneDrawConfigs))) ? sOotSceneDrawConfigs[index] : SDC_DEFAULT;

    switch (config) {
        case SDC_HYRULE_FIELD:
            Hyrule_SceneDrawHyruleField(play);
            break;
        case SDC_KAKARIKO_VILLAGE:
            Hyrule_SceneDrawKakarikoVillage(play);
            break;
        case SDC_ZORAS_RIVER:
            Hyrule_SceneDrawZorasRiver(play);
            break;
        case SDC_KOKIRI_FOREST:
            Hyrule_SceneDrawKokiriForest(play);
            break;
        case SDC_LAKE_HYLIA:
            Hyrule_SceneDrawLakeHylia(play);
            break;
        case SDC_ZORAS_DOMAIN:
            Hyrule_SceneDrawZorasDomain(play);
            break;
        case SDC_ZORAS_FOUNTAIN:
            Hyrule_SceneDrawZorasFountain(play);
            break;
        case SDC_GERUDO_VALLEY:
            Hyrule_SceneDrawGerudoValley(play);
            break;
        case SDC_LOST_WOODS:
            Hyrule_SceneDrawLostWoods(play);
            break;
        case SDC_DESERT_COLOSSUS:
            Hyrule_SceneDrawDesertColossus(play);
            break;
        case SDC_GERUDOS_FORTRESS:
            Hyrule_SceneDrawGerudosFortress(play);
            break;
        case SDC_HAUNTED_WASTELAND:
            Hyrule_SceneDrawHauntedWasteland(play);
            break;
        case SDC_HYRULE_CASTLE:
            Hyrule_SceneDrawHyruleCastle(play);
            break;
        case SDC_DEATH_MOUNTAIN_TRAIL:
            Hyrule_SceneDrawDeathMountainTrail(play);
            break;
        case SDC_DEATH_MOUNTAIN_CRATER:
            Hyrule_SceneDrawDeathMountainCrater(play);
            break;
        case SDC_GORON_CITY:
            Hyrule_SceneDrawGoronCity(play);
            break;
        case SDC_LON_LON_RANCH:
            Hyrule_SceneDrawLonLonRanch(play);
            break;
        case SDC_FIRE_TEMPLE:
            Hyrule_SceneDrawFireTemple(play);
            break;
        case SDC_DEKU_TREE:
            Hyrule_SceneDrawDekuTree(play);
            break;
        case SDC_DODONGOS_CAVERN:
            Hyrule_SceneDrawDodongosCavern(play);
            break;
        case SDC_JABU_JABU:
            Hyrule_SceneDrawJabuJabu(play);
            break;
        case SDC_FOREST_TEMPLE:
            Hyrule_SceneDrawForestTemple(play);
            break;
        case SDC_WATER_TEMPLE:
            Hyrule_SceneDrawWaterTemple(play);
            break;
        case SDC_SHADOW_TEMPLE_AND_WELL:
            Hyrule_SceneDrawShadowTempleAndWell(play);
            break;
        case SDC_SPIRIT_TEMPLE:
            Hyrule_SceneDrawSpiritTemple(play);
            break;
        case SDC_INSIDE_GANONS_CASTLE:
            Hyrule_SceneDrawInsideGanonsCastle(play);
            break;
        case SDC_GERUDO_TRAINING_GROUND:
            Hyrule_SceneDrawGerudoTrainingGround(play);
            break;
        case SDC_DEKU_TREE_BOSS:
            Hyrule_SceneDrawDekuTreeBoss(play);
            break;
        case SDC_WATER_TEMPLE_BOSS:
            Hyrule_SceneDrawWaterTempleBoss(play);
            break;
        case SDC_TEMPLE_OF_TIME:
            Hyrule_SceneDrawTempleOfTime(play);
            break;
        case SDC_GROTTOS:
            Hyrule_SceneDrawGrottos(play);
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
        case SDC_CASTLE_COURTYARD_GUARDS:
            Hyrule_SceneDrawCastleCourtyardGuards(play);
            break;
        case SDC_OUTSIDE_GANONS_CASTLE:
            Hyrule_SceneDrawOutsideGanonsCastle(play);
            break;
        case SDC_ICE_CAVERN:
            Hyrule_SceneDrawIceCavern(play);
            break;
        case SDC_GANONS_TOWER_COLLAPSE_EXTERIOR:
            Hyrule_SceneDrawGanonsTowerCollapseExterior(play);
            break;
        case SDC_FAIRYS_FOUNTAIN:
            Hyrule_SceneDrawFairysFountain(play);
            break;
        case SDC_THIEVES_HIDEOUT:
            Hyrule_SceneDrawThievesHideout(play);
            break;
        case SDC_BOMBCHU_BOWLING_ALLEY:
            Hyrule_SceneDrawBombchuBowlingAlley(play);
            break;
        case SDC_ROYAL_FAMILYS_TOMB:
            Hyrule_SceneDrawRoyalFamilysTomb(play);
            break;
        case SDC_LAKESIDE_LABORATORY:
            Hyrule_SceneDrawLakesideLaboratory(play);
            break;
        case SDC_LON_LON_BUILDINGS:
            Hyrule_SceneDrawLonLonBuildings(play);
            break;
        case SDC_MARKET_GUARD_HOUSE:
            Hyrule_SceneDrawMarketGuardHouse(play);
            break;
        case SDC_POTION_SHOP_GRANNY:
            Hyrule_SceneDrawPotionShopGranny(play);
            break;
        case SDC_CALM_WATER:
            Hyrule_SceneDrawCalmWater(play);
            break;
        case SDC_GRAVE_EXIT_LIGHT_SHINING:
            Hyrule_SceneDrawGraveExitLightShining(play);
            break;
        case SDC_BESITU:
            Hyrule_SceneDrawBesitu(play);
            break;
        case SDC_FISHING_POND:
            Hyrule_SceneDrawFishingPond(play);
            break;
        case SDC_GANONS_TOWER_COLLAPSE_INTERIOR:
        case SDC_INSIDE_GANONS_CASTLE_COLLAPSE:
            // OoT's func_800A057C and func_800A055C: only the collapse's shaking.
            Hyrule_SceneQuakeGanonsTowerCollapse(play);
            break;
        default:
            // OoT's SDC_DEFAULT is MM's default.
            Scene_DrawConfigDefault(play);
            break;
    }
}
