// SoH adapter for the differential testing harness. See harness/core/Harness.h.
//
// Built into SoH when it is configured with -DHARNESS_DIR=<this repo>/harness. Fields are chosen to
// line up with the 2Ship adapter: Player's linearVelocity is logged as speedXZ, and form is linkAge.

#include "HarnessGame.h"

#include <cstdlib>
#include <cstring>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;
}

namespace {

uint32_t sSceneFrame = 0;

template <typename T> void AddFlags(HarnessFrameState& state, const char* name, const T& value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    state.flags.push_back({ name, std::vector<uint8_t>(bytes, bytes + sizeof(value)) });
}

void CaptureState(HarnessFrameState& state) {
    state.rng = Rand_GetState();
    AddFlags(state, "eventChkInf", gSaveContext.eventChkInf);
    AddFlags(state, "itemGetInf", gSaveContext.itemGetInf);
    AddFlags(state, "infTable", gSaveContext.infTable);
    AddFlags(state, "eventInf", gSaveContext.eventInf);
    AddFlags(state, "sceneFlags", gSaveContext.sceneFlags);

    if (gPlayState == nullptr) {
        state.gameState = "other";
        return;
    }

    PlayState* play = gPlayState;
    state.gameState = "play";
    state.scene = play->sceneNum;
    state.room = play->roomCtx.curRoom.num;
    state.sceneFrame = sSceneFrame;
    AddFlags(state, "liveSceneFlags", play->actorCtx.flags);

    HarnessEnv& e = state.env;
    e.valid = true;
    e.skyboxConfig = play->envCtx.unk_17;
    e.changeSkyboxNextConfig = play->envCtx.unk_18;
    e.changeSkyboxState = play->envCtx.unk_19;
    e.lightConfig = play->envCtx.unk_1F;
    e.changeLightNextConfig = play->envCtx.unk_20;
    e.changeLightEnabled = play->envCtx.unk_21;
    e.weatherMode = gWeatherMode;
    memcpy(e.precipitation, play->envCtx.unk_EE, sizeof(e.precipitation));
    e.lightning = play->envCtx.lightningMode;

    Player* player = GET_PLAYER(play);
    if (player != nullptr) {
        state.hasPlayer = true;
        HarnessPlayer& p = state.player;
        memcpy(p.pos, &player->actor.world.pos, sizeof(p.pos));
        p.rot[0] = player->actor.shape.rot.x;
        p.rot[1] = player->actor.shape.rot.y;
        p.rot[2] = player->actor.shape.rot.z;
        p.speedXZ = player->linearVelocity;
        p.yaw = player->yaw;
        p.stateFlags[0] = player->stateFlags1;
        p.stateFlags[1] = player->stateFlags2;
        p.stateFlags[2] = player->stateFlags3;
        p.form = gSaveContext.linkAge;
    }

    for (int32_t cat = 0; cat < ACTORCAT_MAX; cat++) {
        for (Actor* a = play->actorCtx.actorLists[cat].head; a != nullptr; a = a->next) {
            HarnessActor h;
            h.id = a->id;
            h.params = a->params;
            h.category = a->category;
            h.room = a->room;
            h.flags = a->flags;
            memcpy(h.pos, &a->world.pos, sizeof(h.pos));
            h.rot[0] = a->shape.rot.x;
            h.rot[1] = a->shape.rot.y;
            h.rot[2] = a->shape.rot.z;
            state.actors.push_back(h);
        }
    }
}

// Puppet mode: after Player updates, put it where the other run's Player was on the same scene
// frame, so the actors that update afterwards see the recorded Player.
void ApplyPuppet(void* refActor) {
    Actor* actor = static_cast<Actor*>(refActor);
    if (gPlayState == nullptr || actor != &GET_PLAYER(gPlayState)->actor) {
        return;
    }
    HarnessPlayer p;
    if (!Harness_PuppetPlayer(gPlayState->sceneNum, sSceneFrame, &p)) {
        return;
    }
    Player* player = (Player*)actor;
    memcpy(&player->actor.world.pos, p.pos, sizeof(p.pos));
    player->actor.shape.rot.x = p.rot[0];
    player->actor.shape.rot.y = p.rot[1];
    player->actor.shape.rot.z = p.rot[2];
    player->actor.world.rot.y = p.rot[1];
    player->linearVelocity = p.speedXZ;
    player->yaw = p.yaw;
}

void RegisterHarness() {
    Harness_Init("soh");
    if (!Harness_IsActive()) {
        return;
    }
    std::atexit(Harness_Shutdown);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](int16_t sceneNum) { sSceneFrame = 0; });

    // HARNESS_TIME=<hhmm> sets the time of day whenever a scene is entered (cutscenes such as the intro set
    // their own), to check night behavior. The run then differs from its recording.
    const char* timeValue = std::getenv("HARNESS_TIME");
    if (timeValue != nullptr && timeValue[0] != '\0') {
        int32_t time = std::atoi(timeValue);
        GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(
            [time](int16_t sceneNum) { gSaveContext.dayTime = CLOCK_TIME(time / 100, time % 100); });
    }

    // HARNESS_ENTRANCE=<OoT entrance> starts the loaded save there instead, outside any cutscene, to look at a
    // scene the recording does not reach. The run then differs from its recording.
    const char* entranceValue = std::getenv("HARNESS_ENTRANCE");
    if (entranceValue != nullptr && entranceValue[0] != '\0') {
        int32_t entrance = (int32_t)std::strtol(entranceValue, nullptr, 0);
        GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>([entrance](int32_t fileNum) {
            gSaveContext.entranceIndex = entrance;
            gSaveContext.cutsceneIndex = 0;
        });
    }

    // HARNESS_ADULT=1 makes the loaded save's Link an adult, his equipment swapped as the Temple of Time does, to
    // look at a scene's adult layer. The run then differs from its recording.
    const char* adultValue = std::getenv("HARNESS_ADULT");
    if (adultValue != nullptr && std::atoi(adultValue) != 0) {
        GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>([](int32_t fileNum) {
            if (LINK_IS_CHILD) {
                Inventory_SwapAgeEquipment();
                gSaveContext.linkAge = LINK_AGE_ADULT;
            }
        });
    }

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameFrameUpdate>([]() {
        HarnessFrameState state;
        CaptureState(state);
        Harness_LogState(state);
        if (gPlayState != nullptr) {
            sSceneFrame++;
        }
    });

    if (Harness_PuppetActive()) {
        GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorUpdate>(ApplyPuppet);
    }
}

RegisterShipInitFunc sInitFunc(RegisterHarness, {});

} // namespace

extern "C" void HarnessGame_OnPadRead(void* pads) {
    if (!Harness_IsActive()) {
        return;
    }
    OSContPad* pad = &static_cast<OSContPad*>(pads)[0];
    HarnessPad h = { pad->button, pad->stick_x, pad->stick_y, pad->right_stick_x, pad->right_stick_y,
                     pad->gyro_x,  pad->gyro_y };
    if (!Harness_ProcessPad(&h)) {
        // The replay ran out of input.
        Harness_Shutdown();
        std::_Exit(0);
    }
    pad->button = h.button;
    pad->stick_x = h.stickX;
    pad->stick_y = h.stickY;
    pad->right_stick_x = h.rightStickX;
    pad->right_stick_y = h.rightStickY;
    pad->gyro_x = h.gyroX;
    pad->gyro_y = h.gyroY;
    pad->err_no = 0;
}
