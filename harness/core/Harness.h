#pragma once

// Differential testing harness shared by SoH and 2Ship. Each game supplies an adapter that feeds pad
// input and per-frame state to these functions; the core handles files, hashing and replay.
//
// Configuration comes from environment variables, read once by Harness_Init:
//   HARNESS_RECORD=<file>    record controller 1 input, one line per logic frame
//   HARNESS_REPLAY=<file>    replay recorded input; the game exits when the file runs out
//   HARNESS_STATELOG=<file>  write per-frame state as JSON lines
//   HARNESS_PUPPET=<file>    drive Player from another run's state log, matched by scene and frame
//   HARNESS_SEED=<n>         RNG seed used instead of the clock while the harness is active (default 0)

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t button;
    int8_t stickX;
    int8_t stickY;
    int8_t rightStickX;
    int8_t rightStickY;
    float gyroX;
    float gyroY;
} HarnessPad;

void Harness_Init(const char* game);
void Harness_Shutdown(void);

// True when any mode is enabled; adapters use it to make the game deterministic.
bool Harness_IsActive(void);
uint32_t Harness_Seed(void);

// Called once per logic frame with controller 1 as read from the device. Records it, or replaces it
// with the recorded input. Returns false when a replay has run out of input.
bool Harness_ProcessPad(HarnessPad* pad);
uint64_t Harness_Frame(void);

#ifdef __cplusplus
}

#include <string>
#include <vector>

struct HarnessPlayer {
    float pos[3];
    int16_t rot[3];
    float speedXZ;
    int16_t yaw;
    uint32_t stateFlags[3];
    int32_t form;
};

struct HarnessActor {
    int16_t id;
    int16_t params;
    uint8_t category;
    int8_t room;
    uint32_t flags;
    float pos[3];
    int16_t rot[3];
};

// The weather, as OoT's EnvironmentContext names it (MM kept the names and layout). Logged when the adapter sets
// valid, so logs from before it existed still compare.
struct HarnessEnv {
    bool valid = false;
    uint8_t skyboxConfig;
    uint8_t changeSkyboxNextConfig;
    uint8_t changeSkyboxState;
    uint8_t lightConfig;
    uint8_t changeLightNextConfig;
    uint8_t changeLightEnabled;
    uint8_t weatherMode; // OoT's gWeatherMode
    uint8_t precipitation[4]; // rain max and current, snow current and max
    uint8_t lightning; // OoT's lightningMode, MM's lightningState
};

struct HarnessFlagBlock {
    std::string name;
    std::vector<uint8_t> data;
};

struct HarnessFrameState {
    std::string gameState; // "play" or another game state name
    int32_t scene = -1;
    int32_t room = -1;
    uint32_t sceneFrame = 0; // logic frames since the scene was entered
    uint32_t rng = 0;
    bool hasPlayer = false;
    HarnessPlayer player = {};
    HarnessEnv env = {};
    std::vector<HarnessActor> actors;
    std::vector<HarnessFlagBlock> flags;
};

void Harness_LogState(const HarnessFrameState& state);

// Looks up the recorded Player for a scene frame in the HARNESS_PUPPET log.
bool Harness_PuppetPlayer(int32_t scene, uint32_t sceneFrame, HarnessPlayer* out);
bool Harness_PuppetActive();

#endif
