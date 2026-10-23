#pragma once

// OoT's pages in MM's pause menu (docs/PAUSE.md): equipment (PAUSE_EQUIP) and quest status
// (PAUSE_QUEST_OOT). They live here because OoT's and MM's pause textures share names, so OoT's headers
// cannot be included beside MM's in ovl_kaleido_scope.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// A page's background (15 quads, in KaleidoScope_DrawPageSections' order) and its "To <page>" label (IA8,
// 128x16).
TexturePtr* Hyrule_GetPageBgTextures(s16 page);
TexturePtr Hyrule_GetToPageTexture(s16 page);

// Called from ovl_kaleido_scope for these pages, as MM's own pages are.
void Hyrule_KaleidoSetPageVertices(struct PlayState* play, s16 page);
void Hyrule_KaleidoDrawPage(struct PlayState* play, s16 page);
void Hyrule_KaleidoSetCursor(struct PlayState* play, s16 page);
void Hyrule_KaleidoUpdateCursor(struct PlayState* play, s16 page);

// OoT's name texture for an OoT item id (IA4, 128x16), or NULL.
TexturePtr Hyrule_GetItemNameTexture(u16 item);
// An item's icon (RGBA32, 32x32 for items), MM's or OoT's, or NULL.
TexturePtr Hyrule_GetItemIcon(u16 item);

#ifdef __cplusplus
}
#endif
