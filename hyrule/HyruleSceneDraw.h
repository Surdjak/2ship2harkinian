#pragma once

// OoT's scene draw configs (z_scene_table.c), for OoT scenes: MM's SCENE_DRAW_CFG_HYRULE_OOT. See hyrule/README.md.

struct PlayState;

#ifdef __cplusplus
extern "C" {
#endif

// Scrolls the scene's textures and sets its colors as OoT's draw config for the current OoT scene does.
void Hyrule_SceneDrawOot(struct PlayState* play);

#ifdef __cplusplus
}
#endif
