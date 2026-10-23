#pragma once

// OoT's equipment page in MM's pause menu (docs/PAUSE.md). It lives here because OoT's and MM's pause
// textures share names, so OoT's headers cannot be included beside MM's in ovl_kaleido_scope.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// The page's background (15 quads, in KaleidoScope_DrawPageSections' order) and its "To Equipment" label
// (IA8, 128x16).
TexturePtr* Hyrule_GetEquipPageBgTextures(void);
TexturePtr Hyrule_GetToEquipmentTexture(void);

// Called from ovl_kaleido_scope for the equipment page, as MM's own pages are.
void Hyrule_KaleidoSetEquipmentVertices(struct PlayState* play);
void Hyrule_KaleidoDrawEquipment(struct PlayState* play);
void Hyrule_KaleidoSetEquipCursor(struct PlayState* play);
void Hyrule_KaleidoUpdateEquipCursor(struct PlayState* play);

// OoT's name texture for an OoT item id (IA4, 128x16), or NULL.
TexturePtr Hyrule_GetItemNameTexture(u16 item);

// The piece of an equipment type (EQUIP_TYPE_*) worn, as an item id or its icon; ITEM_NONE or NULL if none.
u16 Hyrule_GetWornEquipItem(u8 type);
TexturePtr Hyrule_GetWornEquipIcon(u8 type);

#ifdef __cplusplus
}
#endif
