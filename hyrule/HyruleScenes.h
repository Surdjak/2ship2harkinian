#pragma once

// OoT scenes loaded from oot.o2r. See hyrule/README.md.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// The scene file's path in oot.o2r, or NULL for an MM scene.
const char* Hyrule_GetScenePath(s16 sceneId);

// Called once the scene file's commands have run, before the first room loads.
void Hyrule_AfterSceneCommands(struct PlayState* play);

#ifdef __cplusplus
}
#endif
