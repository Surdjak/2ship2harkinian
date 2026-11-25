#pragma once

// OoT's scene draw configs (z_scene_table.c), for OoT scenes: MM's SCENE_DRAW_CFG_HYRULE_OOT. See hyrule/README.md.

struct PlayState;

#ifdef __cplusplus
extern "C" {
#endif

// Scrolls the scene's textures and sets its colors as OoT's draw config for the current OoT scene does.
void Hyrule_SceneDrawOot(struct PlayState* play);

struct SkyboxContext;
// OoT's cutscene map sky (SKYBOX_CUTSCENE_MAP, the goddesses' sky in the Great Deku Tree's story), which MM never
// used: its two skies, holy0 and holy1, with their palettes as one (holy1's indices are its upper half).
void Hyrule_SetupCutsceneMapSkybox(struct SkyboxContext* skyboxCtx);
// The blend between them, by the time of day, as OoT's skybox config 3 (Environment_UpdateSkybox).
u8 Hyrule_GetCutsceneMapSkyboxBlend(void);

#ifdef __cplusplus
}
#endif
