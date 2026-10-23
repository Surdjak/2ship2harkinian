#include "HyruleScenes.h"
#include "HyruleIds.h"

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

namespace {

struct OotScene {
    s16 sceneId;
    s16 entranceSceneId;
    const char* path;
};

const OotScene sOotScenes[] = {
    { SCENE_KOKIRI_FOREST_Oot, ENTR_SCENE_KOKIRI_FOREST_Oot, "oot/scenes/shared/spot04_scene/spot04_scene" },
};

const OotScene* FindScene(s16 sceneId) {
    for (const OotScene& scene : sOotScenes) {
        if (scene.sceneId == sceneId) {
            return &scene;
        }
    }
    return nullptr;
}

// OoT exits are OoT entrance numbers. Until Hyrule's entrances exist, every exit leads back to the
// scene's first spawn.
u32 sExitList[32];

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
// save at that entrance, the same way map select does. HYRULE_BOOT_TIME=<hhmm> sets the time of day.
void BootToEntrance() {
    const char* value = std::getenv("HYRULE_BOOT_ENTRANCE");
    if (value == nullptr || value[0] == '\0') {
        return;
    }
    u32 entrance = (u32)std::strtoul(value, nullptr, 0);
    const char* timeValue = std::getenv("HYRULE_BOOT_TIME");
    s32 time = (timeValue != nullptr && timeValue[0] != '\0') ? std::atoi(timeValue) : -1;
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnConsoleLogoUpdate>([entrance, time]() {
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
    });
}

void RegisterHyruleScenes() {
    LoadOotArchive();
    BootToEntrance();
}

RegisterShipInitFunc sInitFunc(RegisterHyruleScenes, {});

} // namespace

extern "C" const char* Hyrule_GetScenePath(s16 sceneId) {
    const OotScene* scene = FindScene(sceneId);
    return scene != nullptr ? scene->path : nullptr;
}

extern "C" void Hyrule_AfterSceneCommands(PlayState* play) {
    const OotScene* scene = FindScene(play->sceneId);
    if (scene == nullptr) {
        return;
    }
    for (u32& exit : sExitList) {
        exit = Entrance_Create(scene->entranceSceneId, 0, 0);
    }
    play->setupExitList = sExitList;

    // Player spawn parameters name a camera. In OoT that indexes the collision's camera list; MM looks
    // it up in the actor cutscene camera list (command 0x02), which OoT scenes do not have. The two
    // lists share a layout, so point MM's at OoT's.
    play->actorCsCamList = (ActorCsCamInfo*)play->colCtx.colHeader->bgCamList;

    // MM sets up its cutscene manager from the actor cutscene list (command 0x1B), which OoT scenes do
    // not have. TODO: build one from OoT's camera cutscenes when OoT actors need them.
    CutsceneManager_Init(play, nullptr, 0);

    // The scene's sound settings hold OoT's sequence id; OoT's sequences are loaded at SEQ_OOT_BASE.
    if (play->sceneSequences.seqId != NA_BGM_NO_MUSIC) {
        play->sceneSequences.seqId = SEQ_OOT(play->sceneSequences.seqId);
    }
    // OoT's nature ambience 0x13 means none, so the music plays all day, as MM's AMBIENCE_ID_13 does.
    // TODO: OoT's other nature ambiences are not MM's ambience ids; they are left off until ported.
    if (play->sceneSequences.ambienceId != AMBIENCE_ID_13) {
        play->sceneSequences.ambienceId = AMBIENCE_ID_DISABLED;
    }
}
