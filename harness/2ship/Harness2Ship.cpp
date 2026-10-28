// 2Ship adapter for the differential testing harness. See harness/core/Harness.h.

#include "HarnessGame.h"

#include <cstdlib>
#include <cstring>

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "rand.h"
#include "HyrulePort.h"
}

namespace {

uint32_t sSceneFrame = 0;

template <typename T> void AddFlags(HarnessFrameState& state, const char* name, const T& value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    state.flags.push_back({ name, std::vector<uint8_t>(bytes, bytes + sizeof(value)) });
}

void CaptureState(HarnessFrameState& state) {
    state.rng = Rand_GetState();
    AddFlags(state, "weekEventReg", gSaveContext.save.saveInfo.weekEventReg);
    AddFlags(state, "eventInf", gSaveContext.eventInf);
    AddFlags(state, "permanentSceneFlags", gSaveContext.save.saveInfo.permanentSceneFlags);
    AddFlags(state, "cycleSceneFlags", gSaveContext.cycleSceneFlags);
    // 3Ship [Hyrule] OoT's flags, under the names SoH's adapter uses (tools/ootlog.py renames SoH's
    // eventInf, which MM's would clash with).
    AddFlags(state, "eventChkInf", gSaveContext.save.saveInfo.eventChkInfOot);
    AddFlags(state, "itemGetInf", gSaveContext.save.saveInfo.itemGetInfOot);
    AddFlags(state, "infTable", gSaveContext.save.saveInfo.infTableOot);
    AddFlags(state, "eventInfOot", gSaveContext.eventInfOot);

    if (gPlayState == nullptr) {
        state.gameState = "other";
        return;
    }

    PlayState* play = gPlayState;
    state.gameState = "play";
    state.scene = play->sceneId;
    state.room = play->roomCtx.curRoom.num;
    state.sceneFrame = sSceneFrame;
    AddFlags(state, "liveSceneFlags", play->actorCtx.sceneFlags);

    HarnessEnv& e = state.env;
    e.valid = true;
    e.skyboxConfig = play->envCtx.skyboxConfig;
    e.changeSkyboxNextConfig = play->envCtx.changeSkyboxNextConfig;
    e.changeSkyboxState = play->envCtx.changeSkyboxState;
    e.lightConfig = play->envCtx.lightConfig;
    e.changeLightNextConfig = play->envCtx.changeLightNextConfig;
    e.changeLightEnabled = play->envCtx.changeLightEnabled;
    // 3Ship [Hyrule] OoT's weather modes are kept past MM's (WEATHER_MODE_OOT); log them as OoT's.
    e.weatherMode = (gWeatherMode >= WEATHER_MODE_OOT(0)) ? gWeatherMode - WEATHER_MODE_OOT(0) : gWeatherMode;
    memcpy(e.precipitation, play->envCtx.precipitation, sizeof(e.precipitation));
    e.lightning = play->envCtx.lightningState;

    Player* player = GET_PLAYER(play);
    if (player != nullptr) {
        state.hasPlayer = true;
        HarnessPlayer& p = state.player;
        memcpy(p.pos, &player->actor.world.pos, sizeof(p.pos));
        p.rot[0] = player->actor.shape.rot.x;
        p.rot[1] = player->actor.shape.rot.y;
        p.rot[2] = player->actor.shape.rot.z;
        p.speedXZ = player->speedXZ;
        p.yaw = player->yaw;
        p.stateFlags[0] = player->stateFlags1;
        p.stateFlags[1] = player->stateFlags2;
        p.stateFlags[2] = player->stateFlags3;
        p.form = player->transformation;
    }

    for (int32_t cat = 0; cat < ACTORCAT_MAX; cat++) {
        for (Actor* a = play->actorCtx.actorLists[cat].first; a != nullptr; a = a->next) {
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
void ApplyPuppet(Actor* actor) {
    if (gPlayState == nullptr || actor != &GET_PLAYER(gPlayState)->actor) {
        return;
    }
    HarnessPlayer p;
    if (!Harness_PuppetPlayer(gPlayState->sceneId, sSceneFrame, &p)) {
        return;
    }
    Player* player = (Player*)actor;
    memcpy(&player->actor.world.pos, p.pos, sizeof(p.pos));
    player->actor.shape.rot.x = p.rot[0];
    player->actor.shape.rot.y = p.rot[1];
    player->actor.shape.rot.z = p.rot[2];
    player->actor.world.rot.y = p.rot[1];
    player->speedXZ = p.speedXZ;
    player->yaw = p.yaw;
    // Player placed its hurtbox during its update; move it too, since collisions are checked after all
    // actors update.
    Collider_UpdateCylinder(&player->actor, &player->cylinder);
}

void RegisterHarness() {
    Harness_Init("2ship");
    if (!Harness_IsActive()) {
        return;
    }
    std::atexit(Harness_Shutdown);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(
        [](s16 sceneId, s8 spawnNum) { sSceneFrame = 0; });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateMainFinish>([]() {
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
