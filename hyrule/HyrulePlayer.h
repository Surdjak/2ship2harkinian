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

// OoT's Slingshot (docs/OOT_ITEMS.md), on MM's human Link with OoT child Link's models.
// Whether Player holds the Slingshot, or is taking it out (heldItemId changes first).
s32 Hyrule_HoldsSlingshot(struct Player* player);
// Child Link's right hand holding the Slingshot, as a near/far pair.
Gfx** Hyrule_GetHandHoldingSlingshotDLs(void);
// His right arm holding the Slingshot out, in first person.
Gfx* Hyrule_GetSlingshotFirstPersonDL(void);
// The Slingshot's string, and where it sits in the right hand.
Gfx* Hyrule_GetSlingshotStringDL(void);
void Hyrule_GetSlingshotStringPos(f32* x, f32* y, f32* z);

// OoT's Boomerang (docs/OOT_ITEMS.md), on PLAYER_IA_BOOMERANG_Oot, which MM kept from OoT but left unused.
typedef enum HyruleBoomerangAnim {
    HYRULE_BOOMERANG_ANIM_READY, // OoT's link_boom_throw_wait2waitR
    HYRULE_BOOMERANG_ANIM_THROW_R,
    HYRULE_BOOMERANG_ANIM_THROW_L,
    HYRULE_BOOMERANG_ANIM_CATCH
} HyruleBoomerangAnim;
// OoT's animation for the Boomerang; MM kept only the aiming loops (gPlayerAnim_link_boom_throw_waitR/L).
void* Hyrule_GetBoomerangAnim(s32 anim);
// Throws OoT's Boomerang (En_Boom_Oot) as OoT's Player does, toward the lock-on target if any; NULL if none spawned.
struct Actor* Hyrule_ThrowBoomerang(struct PlayState* play, struct Player* player);
// Child Link's left fist holding the Boomerang, as a near/far pair.
Gfx** Hyrule_GetHandHoldingBoomerangDLs(void);

// OoT's masks (docs/OOT_ITEMS.md): worn by human Link, one face at a time with MM's, kept on across scenes as MM's
// are (gSaveContext.save.equippedMaskOot). OoT's PlayerMask values: 1 Keaton, 2 Skull, 3 Spooky, 4 Bunny Hood,
// 5 Goron, 6 Zora, 7 Gerudo, 8 Truth.
#define PLAYER_MASK_NONE_Oot 0
#define PLAYER_MASK_KEATON_Oot 1
#define PLAYER_MASK_BUNNY_Oot 4
#define PLAYER_MASK_TRUTH_Oot 8
// The OoT mask Link wears, for OoT's actors (OoT's Player_GetMask).
s32 Player_GetMaskOot(struct PlayState* play);
// Puts an OoT mask on, MM's off, or takes the OoT mask off if it is that one (OoT's Player_UseItem).
void Hyrule_ToggleMaskOot(struct Player* player, s32 mask);
// Each frame: the mask comes off when its item leaves the buttons or Link changes body (OoT's
// Player_ProcessItemButtons) and when MM's mask goes on; the Bunny Hood's ears swing (OoT's Player_UpdateBunnyEars).
void Hyrule_UpdateMaskOot(struct PlayState* play, struct Player* player);
// Draws the mask on child Link's head, after his skeleton (OoT's Player_DrawGameplay).
void Hyrule_DrawMaskOot(struct PlayState* play, struct Player* player);

// The Weird Egg hatches into the Cucco at dawn, as in OoT: noted with the new day, announced once a scene runs.
void Hyrule_OnNewDay(void);
void Hyrule_UpdateEggHatching(struct PlayState* play);

// The B-button item for an equipped sword value, MM's or 3Ship's (ITEM_NONE for none).
u16 Hyrule_SwordValueToItem(u8 value);
// MM's sword owned (Kokiri, Razor or Gilded), as an equip value; for its sheath while an OoT sword is worn.
u8 Hyrule_GetOwnedMmSwordValue(void);

#ifdef __cplusplus
}
#endif
