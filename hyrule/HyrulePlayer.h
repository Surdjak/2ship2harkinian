#pragma once

// OoT's equipment on MM's Player (docs/PAUSE.md). The equipment values 3Ship adds (HyruleInventory.h) are
// past the end of MM's per-sword and per-shield tables, so Player asks here instead.

#include "PR/ultratypes.h"
#include "PR/gbi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Player;

// Player.currentShield is the equipped value, so OoT's shields follow MM's two.
#define PLAYER_SHIELD_DEKU_Oot 3
#define PLAYER_SHIELD_HYLIAN_Oot 4
#define PLAYER_SHIELD_MIRROR_Oot 5

// A right hand holding one of OoT's shields, as a near/far pair; NULL for MM's shields. Child Link holds only
// the Deku Shield: the Hylian Shield stays on his back, as in OoT.
Gfx** Hyrule_GetHandHoldingShieldDLs(s32 shield);
// One of OoT's shields on the back, drawn on the sheath limb; NULL for MM's shields.
Gfx* Hyrule_GetBackShieldDL(s32 shield);
// Sets up what Hyrule_GetBackShieldDL's lists expect and do not set themselves.
void Hyrule_SetupBackShieldDraw(struct PlayState* play);
u8 Hyrule_GetShieldColMaterial(s32 shield);
// OoT's Player_IsChildWithHylianShield: shields with it on his back instead of holding it.
s32 Hyrule_IsChildWithHylianShield(struct Player* player);

// The B-button item for an equipped sword value, MM's or 3Ship's (ITEM_NONE for none).
u16 Hyrule_SwordValueToItem(u8 value);
// MM's sword owned (Kokiri, Razor or Gilded), as an equip value; for its sheath while an OoT sword is worn.
u8 Hyrule_GetOwnedMmSwordValue(void);

#ifdef __cplusplus
}
#endif
