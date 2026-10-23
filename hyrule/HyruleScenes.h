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

// OoT's scene layers (SceneLayer in OoT). MM has no age or time layers.
#define SCENE_LAYER_CHILD_DAY_OOT 0
#define SCENE_LAYER_CHILD_NIGHT_OOT 1
#define SCENE_LAYER_ADULT_DAY_OOT 2
#define SCENE_LAYER_ADULT_NIGHT_OOT 3
#define SCENE_LAYER_CUTSCENE_FIRST_OOT 4

// An OoT entrance into a scene that is not in the scene table yet.
#define HYRULE_ENTRANCE_NONE 0xFFFFFFFF

// The MM entrance for an OoT entrance index, or HYRULE_ENTRANCE_NONE.
u32 Hyrule_EntranceFromOot(u16 ootEntrance);

// The MM entrance an OoT exit leads to. Exits into scenes that are not loaded yet lead back to the spawn
// Link came in at.
u32 Hyrule_ExitFromOot(u16 ootEntrance);

// OoT's scene camera type (cameraMovement in OoT's misc settings command), read for OoT scenes.
void Hyrule_SetCameraMovement(u8 cameraMovement);

// OoT's prerendered rooms show one of two scene cameras, the "viewpoint": 1 is the fixed camera the
// background was rendered from, 2 the pivoting one that shows the panorama, and 0 means the scene has
// none. Houses start on 2 and C-up toggles; shops start on 1 and their shopkeepers switch.
void Hyrule_InitViewpoint(struct PlayState* play);
void Hyrule_UpdateViewpoint(struct PlayState* play, struct Input* input);
void Hyrule_SetViewpoint(struct PlayState* play, s16 viewpoint);
// Whether C-up is the scene's viewpoint toggle (or disabled, in shops) rather than first-person view.
s32 Hyrule_CUpChangesViewpoint(void);

// MM's skybox for an OoT skybox id.
u8 Hyrule_SkyboxFromOot(u8 ootSkyboxId);

// The scene layer to load for an entrance: MM's unchanged, OoT's chosen as OoT does. mmSceneLayer is 0,
// or 1 + the cutscene index as MM computes it.
s32 Hyrule_GetSceneLayer(u32 entrance, s32 mmSceneLayer);

#ifdef __cplusplus
}
#endif
