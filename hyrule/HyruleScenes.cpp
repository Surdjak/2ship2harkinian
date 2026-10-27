#include "HyruleScenes.h"
#include "HyruleIds.h"
#include "HyrulePort.h"
#include "HyrulePanorama.h"
#include "HyruleInventory.h"
#include "HyruleFlags.h"

#include <cstdlib>
#include <filesystem>
#include <ship/Context.h>

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "overlays/gamestates/ovl_select/z_select.h"
}

static_assert(ACTOR_PLAYER_Oot == ACTOR_OOT_BASE, "ACTOR_OOT_BASE must be the first OoT actor");
static_assert(ACTOR_ID_MAX == ACTOR_OOT_BASE + 0x1D7, "OoT has 0x1D7 actors");
static_assert(OBJECT_INVALID_Oot == OBJECT_OOT_BASE, "OBJECT_OOT_BASE must be the first OoT object");
static_assert(OBJECT_ID_MAX == OBJECT_OOT_BASE + 0x192, "OoT has 0x192 objects");
static_assert(ITEM_STICK_Oot == ITEM_OOT_BASE && ITEM_OOT_BASE > ITEM_NONE, "OoT items must follow MM's");
// Spawn entries keep the actor id in 13 bits, and Actor_SpawnSetupActors reads bit 11 (0x800) as a flag.
static_assert(ACTOR_ID_MAX <= 0x800, "actor ids must stay below 0x800");

static_assert(SCENE_KOKIRI_FOREST_Oot == SCENE_ALLEY + 1, "OoT scenes must follow MM's");
static_assert(ENTR_SCENE_KOKIRI_FOREST_Oot == ENTR_SCENE_LAUNDRY_POOL + 1, "OoT entrance scenes must follow MM's");

namespace {

// The MM entrance for each OoT entrance index (tools/porting/gen_oot_scenes.py).
const u32 sOotEntrances[] = {
#include "HyruleOotEntrances.inc"
};

// The scene's OoT skybox id, kept for its panorama (Hyrule_SkyboxFromOot).
u8 sOotSkyboxId = 0;

// OoT's scene camera type and prerendered room viewpoint (YREG(15) and play->unk_1242B in OoT).
#define CAMERA_MOVEMENT_SHOP 0x10
#define CAMERA_MOVEMENT_HOUSE 0x20
u8 sCameraMovement = 0;
s16 sViewpoint = 0;

// OoT's scene id for each OoT scene in the table (tools/porting/gen_oot_scenes.py).
const u8 sOotSceneIds[] = {
#include "HyruleOotSceneIds.inc"
};
static_assert(ARRAY_COUNT(sOotSceneIds) == SCENE_MAX - SCENE_KOKIRI_FOREST_Oot, "one OoT id per OoT scene");

bool IsOotScene(s16 sceneId) {
    return (sceneId >= SCENE_KOKIRI_FOREST_Oot) && (sceneId < SCENE_MAX);
}

bool IsOotEntrance(u32 entrance) {
    u32 entranceSceneId = ENTRANCE_GET_SCENE(entrance);
    return (entranceSceneId >= ENTR_SCENE_KOKIRI_FOREST_Oot) && (entranceSceneId < ENTR_SCENE_MAX);
}

void LoadOotArchive() {
    std::string path = Ship::Context::LocateFileAcrossAppDirs("oot.o2r");
    if (!std::filesystem::exists(path)) {
        return;
    }
    // Added after 2Ship has checked its archives' game versions, which would reject OoT's, and after
    // mm.o2r, whose version is the one the game reads.
    Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(path);
}

// For tests and quick checks: HYRULE_BOOT_ENTRANCE=<entrance> skips the title screen and starts a debug
// save at that entrance, the same way map select does. HYRULE_BOOT_TIME=<hhmm> sets the time of day, and
// HYRULE_BOOT_ADULT=1 starts as adult Link.
void BootToEntrance() {
    const char* value = std::getenv("HYRULE_BOOT_ENTRANCE");
    if (value == nullptr || value[0] == '\0') {
        return;
    }
    u32 entrance = (u32)std::strtoul(value, nullptr, 0);
    const char* timeValue = std::getenv("HYRULE_BOOT_TIME");
    s32 time = (timeValue != nullptr && timeValue[0] != '\0') ? std::atoi(timeValue) : -1;
    const char* adultValue = std::getenv("HYRULE_BOOT_ADULT");
    bool adult = (adultValue != nullptr) && (adultValue[0] == '1');
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnConsoleLogoUpdate>([entrance, time, adult]() {
        static bool sBooted = false;
        if (sBooted) {
            return;
        }
        sBooted = true;
        gSaveContext.fileNum = 0xFF;
        // Map select's init sets this; the debug save leaves it at 0, which is Fierce Deity.
        gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
        // ConsoleLogoState and MapSelectState both start with their GameState, which is all
        // MapSelect_LoadGame uses.
        MapSelect_LoadGame((MapSelectState*)gGameState, entrance, 0);
        if (time >= 0) {
            gSaveContext.save.time = CLOCK_TIME(time / 100, time % 100);
        }
        if (adult) {
            Hyrule_ItemGive(nullptr, ITEM_SWORD_MASTER_Oot);
            Hyrule_SetAge(true);
        }
    });
}

void RegisterHyruleScenes() {
    LoadOotArchive();
    BootToEntrance();
}

RegisterShipInitFunc sInitFunc(RegisterHyruleScenes, {});

} // namespace

extern "C" const char* Hyrule_GetScenePath(s16 sceneId) {
    static std::string sPath;
    if (!IsOotScene(sceneId)) {
        return nullptr;
    }
    const char* fileName = gSceneTable[sceneId].segment.fileName;
    sPath = std::string("oot/scenes/shared/") + fileName + "/" + fileName;
    return sPath.c_str();
}

extern "C" s32 Hyrule_GetOotSceneId(s16 sceneId) {
    return IsOotScene(sceneId) ? sOotSceneIds[sceneId - SCENE_KOKIRI_FOREST_Oot] : -1;
}

extern "C" u32 Hyrule_EntranceFromOot(u16 ootEntrance) {
    return (ootEntrance < ARRAY_COUNT(sOotEntrances)) ? sOotEntrances[ootEntrance] : HYRULE_ENTRANCE_NONE;
}

extern "C" u32 Hyrule_ExitFromOot(u16 ootEntrance) {
    // OoT's ENTR_RETURN_GROTTO, the exit out of grottos and fairy fountains, is MM's 0xFFFF: back to the hole Link
    // fell in (RESPAWN_MODE_UNK_3).
    static constexpr u16 kOotReturnGrotto = 0x7FFF;
    if (ootEntrance == kOotReturnGrotto) {
        return 0xFFFF;
    }
    u32 entrance = Hyrule_EntranceFromOot(ootEntrance);
    // An exit into a scene that is not loaded yet leads back to the spawn Link came in at.
    return (entrance != HYRULE_ENTRANCE_NONE) ? entrance : (gSaveContext.save.entrance & ~0xF);
}

// OoT picks the scene layer from Link's age and the time of day, and puts cutscenes from layer 4 (Play_Init
// in OoT). MM's layer is 0, or 1 + the cutscene.
extern "C" s32 Hyrule_GetSceneLayer(u32 entrance, s32 mmSceneLayer) {
    if (!IsOotEntrance(entrance)) {
        return mmSceneLayer;
    }
    if (mmSceneLayer != 0) {
        return SCENE_LAYER_CUTSCENE_FIRST_OOT + mmSceneLayer - 1;
    }
    if (LINK_IS_ADULT_OOT) {
        return IS_DAY_OOT ? SCENE_LAYER_ADULT_DAY_OOT : SCENE_LAYER_ADULT_NIGHT_OOT;
    }
    return IS_DAY_OOT ? SCENE_LAYER_CHILD_DAY_OOT : SCENE_LAYER_CHILD_NIGHT_OOT;
}

extern "C" s32 Hyrule_GetSceneLayerException(u32 entrance, s32 sceneLayer) {
    // OoT's scene ids.
    static constexpr s32 kHyruleField = 0x51;
    static constexpr s32 kKokiriForest = 0x55;
    s32 ootScene;

    if (!IsOotEntrance(entrance) || (sceneLayer >= SCENE_LAYER_CUTSCENE_FIRST_OOT)) {
        return sceneLayer;
    }
    ootScene = Hyrule_GetOotSceneId(Entrance_GetSceneIdAbsolute(entrance));
    if ((ootScene == kHyruleField) && LINK_IS_CHILD_OOT) {
        // Child Link sees Hyrule Field's night layer, Zelda's escape, once he has the three spiritual stones.
        return (Hyrule_CheckQuestItemOot(QUEST_KOKIRI_EMERALD_Oot) && Hyrule_CheckQuestItemOot(QUEST_GORON_RUBY_Oot) &&
                Hyrule_CheckQuestItemOot(QUEST_ZORA_SAPPHIRE_Oot))
                   ? SCENE_LAYER_CHILD_NIGHT_OOT
                   : SCENE_LAYER_CHILD_DAY_OOT;
    }
    if ((ootScene == kKokiriForest) && LINK_IS_ADULT_OOT) {
        // Adult Link's Kokiri Forest is overrun until the Forest Temple is cleared, day and night.
        return GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP_Oot) ? SCENE_LAYER_ADULT_NIGHT_OOT
                                                                                 : SCENE_LAYER_ADULT_DAY_OOT;
    }
    return sceneLayer;
}

extern "C" u8 Hyrule_SkyboxFromOot(u8 ootSkyboxId) {
    sOotSkyboxId = ootSkyboxId;
    switch (ootSkyboxId) {
        case 0x01: // SKYBOX_NORMAL_SKY
            return SKYBOX_NORMAL_SKY;
        case 0x05: // SKYBOX_CUTSCENE_MAP
            return SKYBOX_CUTSCENE_MAP;
        default:
            // The prerendered rooms' panoramas (SKYBOX_HOUSE_LINK...) are drawn by HyrulePanorama.c.
            // TODO: OoT's bazaar, market and overcast skies are not ported.
            return SKYBOX_NONE;
    }
}

extern "C" void Hyrule_SetCameraMovement(u8 cameraMovement) {
    sCameraMovement = cameraMovement;
}

extern "C" void Hyrule_InitViewpoint(PlayState* play) {
    if (sCameraMovement == CAMERA_MOVEMENT_HOUSE) {
        sViewpoint = 2;
    } else if (sCameraMovement == CAMERA_MOVEMENT_SHOP) {
        sViewpoint = 1;
    } else {
        sViewpoint = 0;
    }
}

extern "C" void Hyrule_SetViewpoint(PlayState* play, s16 viewpoint) {
    sViewpoint = viewpoint;
    if ((sCameraMovement != CAMERA_MOVEMENT_SHOP) && (gSaveContext.save.cutsceneIndex < 0xFFF0)) {
        Audio_PlaySfx((viewpoint == 1) ? NA_SE_SY_CAMERA_ZOOM_DOWN : NA_SE_SY_CAMERA_ZOOM_UP);
    }
    Camera_ChangeActorCsCamIndex(GET_ACTIVE_CAM(play), sViewpoint - 1);
}

// Play_Update in OoT, after the rooms update.
extern "C" void Hyrule_UpdateViewpoint(PlayState* play, Input* input) {
    if (sViewpoint == 0) {
        return;
    }
    if (CHECK_BTN_ALL(input->press.button, BTN_CUP) && !IS_PAUSED(&play->pauseCtx) && !Player_InCsMode(play)) {
        if (sCameraMovement == CAMERA_MOVEMENT_SHOP) {
            Audio_PlaySfx(NA_SE_SY_ERROR);
        } else {
            Hyrule_SetViewpoint(play, sViewpoint ^ 3);
        }
    }
    Camera_ChangeActorCsCamIndex(GET_ACTIVE_CAM(play), sViewpoint - 1);
}

extern "C" s32 Hyrule_CUpChangesViewpoint(void) {
    return (sCameraMovement == CAMERA_MOVEMENT_SHOP) || (sCameraMovement == CAMERA_MOVEMENT_HOUSE);
}

extern "C" void Hyrule_AfterSceneCommands(PlayState* play) {
    Hyrule_InitPanorama(IsOotScene(play->sceneId) ? sOotSkyboxId : 0);
    sOotSkyboxId = 0;
    if (!IsOotScene(play->sceneId)) {
        sCameraMovement = 0;
        return;
    }

    // Player spawn parameters name a camera. In OoT that indexes the collision's camera list; MM looks
    // it up in the actor cutscene camera list (command 0x02), which OoT scenes do not have. The two
    // lists share a layout, so point MM's at OoT's.
    play->actorCsCamList = (ActorCsCamInfo*)play->colCtx.colHeader->bgCamList;

    // MM sets up its cutscene manager from the actor cutscene list (command 0x1B), which OoT scenes do
    // not have. TODO: build one from OoT's camera cutscenes when OoT actors need them.
    CutsceneManager_Init(play, nullptr, 0);

    // OoT's Hyrule Field music (sequence 0x02) is a logic sequence that OoT's audio code steers; MM's sequence player
    // jumps out of it and crashes. TODO: port OoT's field music driver. Until then Hyrule Field plays its nature
    // ambience day and night, as OoT does at night.
    static constexpr u8 kOotFieldLogicSeq = 0x02;
    if (play->sceneSequences.seqId == kOotFieldLogicSeq) {
        play->sceneSequences.seqId = NA_BGM_NO_MUSIC;
    }
    // The scene's sound settings hold OoT's sequence id; OoT's sequences are loaded at SEQ_OOT_BASE.
    if (play->sceneSequences.seqId != NA_BGM_NO_MUSIC) {
        play->sceneSequences.seqId = SEQ_OOT(play->sceneSequences.seqId);
    }
    // The same for OoT's nature ambiences, except 0x13 (none), which means MM's AMBIENCE_ID_13: the music
    // plays all day.
    if (play->sceneSequences.ambienceId != AMBIENCE_ID_13) {
        play->sceneSequences.ambienceId = AMBIENCE_OOT(play->sceneSequences.ambienceId);
    }
}
