#pragma once

// OoT's world map page, the map page in Hyrule's scenes (HyruleKaleidoMap.c).

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// Whether the current scene is one of Hyrule's, where the map page shows OoT's world map.
s32 Hyrule_IsInHyrule(struct PlayState* play);
// The current area, from an OoT scene's misc settings command, which also marks it visited as OoT does.
void Hyrule_SetWorldMapArea(struct PlayState* play, s32 area);

TexturePtr* Hyrule_GetMapPageBgTextures(void);
// A point's name (IA4, 128x16), for the name panel.
TexturePtr Hyrule_GetWorldMapPointName(u16 point);

void Hyrule_KaleidoSetWorldMapVertices(struct PlayState* play);
void Hyrule_KaleidoDrawWorldMap(struct PlayState* play);
void Hyrule_KaleidoSetWorldMapCursor(struct PlayState* play);
void Hyrule_KaleidoUpdateWorldMapCursor(struct PlayState* play);

#ifdef __cplusplus
}
#endif
