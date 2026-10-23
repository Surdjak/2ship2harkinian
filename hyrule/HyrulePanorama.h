#pragma once

// OoT's prerendered room panoramas. See hyrule/README.md.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// Builds the panorama for an OoT skybox id, or turns it off if the id has none (MM scenes pass 0).
void Hyrule_InitPanorama(u8 ootSkyboxId);

// Draws the panorama around (x, y, z) unless the camera is the prerendered room's fixed one, as OoT's
// Play_Draw does after the rooms.
void Hyrule_DrawPanorama(struct PlayState* play, f32 x, f32 y, f32 z);

#ifdef __cplusplus
}
#endif
