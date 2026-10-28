#pragma once

// OoT's equipment on MM's Player (docs/PAUSE.md). The equipment values 3Ship adds (HyruleInventory.h) are
// past the end of MM's per-sword and per-shield tables, so Player asks here instead.

#include "PR/ultratypes.h"
#include "PR/gbi.h"
#include "z64player.h"

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
#define PLAYER_MASK_SKULL_Oot 2
#define PLAYER_MASK_SPOOKY_Oot 3
#define PLAYER_MASK_BUNNY_Oot 4
#define PLAYER_MASK_GORON_Oot 5
#define PLAYER_MASK_ZORA_Oot 6
#define PLAYER_MASK_GERUDO_Oot 7
#define PLAYER_MASK_TRUTH_Oot 8
// The OoT mask Link wears, for OoT's actors (OoT's Player_GetMask).
s32 Player_GetMaskOot(struct PlayState* play);
// OoT's Text_GetFaceReaction: what OoT's people say to the OoT mask Link wears, for one of OoT's reaction sets
// (an OoT text id with TEXT_OOT_BASE), or 0. MM's function reads MM's masks and returns MM's texts.
u16 Text_GetFaceReactionOot(struct PlayState* play, u32 reactionSet);
// Takes the OoT mask off (OoT's Player_UnsetMask).
void Player_UnsetMaskOot(struct PlayState* play);
// Puts an OoT mask on, MM's off, or takes the OoT mask off if it is that one (OoT's Player_UseItem).
void Hyrule_ToggleMaskOot(struct Player* player, s32 mask);
// Each frame: the mask comes off when its item leaves the buttons or Link changes body (OoT's
// Player_ProcessItemButtons) and when MM's mask goes on; the Bunny Hood's ears swing (OoT's Player_UpdateBunnyEars).
void Hyrule_UpdateMaskOot(struct PlayState* play, struct Player* player);
// Draws the mask on Link's head, after his skeleton (OoT's Player_DrawGameplay).
void Hyrule_DrawMaskOot(struct PlayState* play, struct Player* player);

// The Weird Egg and the Pocket Egg hatch into their Cuccos at dawn, as in OoT: noted with the new day, announced
// once a scene runs.
void Hyrule_OnNewDay(void);
void Hyrule_UpdateEggHatching(struct PlayState* play);
// When Link dies, the adult trade items that spoil go back to what they were: the Odd Mushroom to Cojiro, the
// Eyeball Frog and the Eyedrops to the Prescription (OoT's GameOver_Update).
void Hyrule_RevertSpoilingItemsOot(struct PlayState* play);

// Adult Link (docs/ADULT_LINK.md): human Link at adult age (LINK_IS_ADULT_OOT), with OoT's adult model.
s32 Hyrule_IsAdultPlayer(struct Player* player);
FlexSkeletonHeader* Hyrule_GetAdultSkeleton(void);
// His eyes, mouth (segments 8 and 9) and tunic colour, before his skeleton is drawn (OoT's Player_DrawImpl).
Gfx* Hyrule_SetupAdultDraw(Gfx* gfx, s32 eyeIndex, s32 mouthIndex);
// His hands, sheath and waist (OoT's Player_OverrideLimbDrawGameplayDefault), for MM's model types; true when the
// limb is one of those. The hand types are updated as MM's are, for Player's post-limb draw.
s32 Hyrule_OverrideLimbDrawAdult(struct Player* player, s32 limbIndex, Gfx** dList, s32 lod,
                                 PlayerModelType* leftHandType, PlayerModelType* rightHandType);
// His arms in first person (OoT's Player_OverrideLimbDrawGameplayFirstPerson).
Gfx* Hyrule_GetAdultFirstPersonDL(struct Player* player, s32 limbIndex);
// Masks are made for child Link's head. On his head's post-limb draw: keeps its matrix (for OoT's masks, drawn
// after his skeleton), and fits a mask to it by changing the current matrix (for MM's, drawn there).
void Hyrule_SetAdultHeadMatrix(void);
void Hyrule_FitMaskToAdultHead(void);
// His bow string, and where it sits in the right hand (OoT's sBowStringData).
Gfx* Hyrule_GetAdultBowStringDL(void);
void Hyrule_GetAdultBowStringPos(f32* x, f32* y, f32* z);

// The swords' damage (docs/ADULT_LINK.md, A7): to OoT's creatures, OoT's damage type for each sword's class
// (Kokiri, Master or Giant's), in OoT's slash or jump form; to MM's, adult Link's swords by OoT's ratios (the
// Master Sword twice the Kokiri Sword, Biggoron's four times). Changes a human sword attack's flags and damage.
void Hyrule_SetSwordDamage(struct Player* player, s32 strong, u64* dmgFlags, s32* damage);
// Boots that walk on the bottom of water: Zora Link's underwater and the Iron Boots (OoT's PLAYER_BOOTS_IRON checks).
#define PLAYER_BOOTS_WALKS_UNDERWATER(boots) \
    (((boots) == PLAYER_BOOTS_ZORA_UNDERWATER) || ((boots) == PLAYER_BOOTS_IRON_Oot))

// OoT's heavy_run animation group for the Iron Boots, by PlayerAnimType.
void* Hyrule_GetHeavyRunAnim(s32 modelAnimType);
// Adult Link's Iron or Hover Boots over his feet, after his skeleton (OoT's Player_DrawImpl).
void Hyrule_DrawAdultBoots(struct PlayState* play);
// The Hover Boots' glowing ring under Link while they float (OoT's Player_Draw).
void Hyrule_DrawHoverBootsGlow(struct PlayState* play, struct Player* player);
// One of OoT's player animations in gameplay_keep, by name: z_player.c cannot include OoT's gameplay_keep.h,
// whose names are MM's too.
#define HYRULE_ANIM_OOT(name) ("__OTR__oot/objects/gameplay_keep/" name)
// Where the Megaton Hammer strikes the ground in front of Link, if the ground is there (OoT's
// Player_Action_808502D0, which then quakes and spawns a shockwave).
s32 Hyrule_GetHammerGroundPos(struct PlayState* play, struct Player* player, Vec3f* pos);
// The hookshot's reach in frames: OoT's Hookshot 13, the Longshot and MM's Hookshot 26 (OoT's ArmsHook_Wait).
s32 Hyrule_GetHookshotReach(struct Player* player);
// Adult Link's PlayerBoots, from the equipment page's boots (OoT's BOOTS_EQUIP_TO_PLAYER).
u8 Hyrule_GetAdultBoots(void);
// Whether Link holds the Giant's Knife broken (OoT's Player_HoldsBrokenKnife).
s32 Hyrule_HoldsBrokenKnife(struct Player* player);
// A sword hit wears the Giant's Knife down; after 8, it breaks (OoT's func_80842B7C). True if he holds it.
s32 Hyrule_WearGiantsKnife(struct PlayState* play, struct Player* player);

// OoT's exchange items (OoT's ExchangeItemID): what an OoT actor asks Link to show, and what he showed it. MM's
// Player keeps item actions in exchangeItemAction instead.
typedef enum ExchangeItemIdOot {
    /* 0x00 */ EXCH_ITEM_NONE,
    /* 0x01 */ EXCH_ITEM_LETTER_ZELDA,
    /* 0x02 */ EXCH_ITEM_WEIRD_EGG,
    /* 0x03 */ EXCH_ITEM_CHICKEN,
    /* 0x04 */ EXCH_ITEM_BEAN,
    /* 0x05 */ EXCH_ITEM_POCKET_EGG,
    /* 0x06 */ EXCH_ITEM_POCKET_CUCCO,
    /* 0x07 */ EXCH_ITEM_COJIRO,
    /* 0x08 */ EXCH_ITEM_ODD_MUSHROOM,
    /* 0x09 */ EXCH_ITEM_ODD_POTION,
    /* 0x0A */ EXCH_ITEM_SAW,
    /* 0x0B */ EXCH_ITEM_SWORD_BROKEN,
    /* 0x0C */ EXCH_ITEM_PRESCRIPTION,
    /* 0x0D */ EXCH_ITEM_FROG,
    /* 0x0E */ EXCH_ITEM_EYEDROPS,
    /* 0x0F */ EXCH_ITEM_CLAIM_CHECK,
    /* 0x10 */ EXCH_ITEM_MASK_SKULL,
    /* 0x11 */ EXCH_ITEM_MASK_SPOOKY,
    /* 0x12 */ EXCH_ITEM_MASK_KEATON,
    /* 0x13 */ EXCH_ITEM_MASK_BUNNY,
    /* 0x14 */ EXCH_ITEM_MASK_TRUTH,
    /* 0x15 */ EXCH_ITEM_MASK_GORON,
    /* 0x16 */ EXCH_ITEM_MASK_ZORA,
    /* 0x17 */ EXCH_ITEM_MASK_GERUDO,
    /* 0x18 */ EXCH_ITEM_FISH,
    /* 0x19 */ EXCH_ITEM_BLUE_FIRE,
    /* 0x1A */ EXCH_ITEM_BUG,
    /* 0x1B */ EXCH_ITEM_POE,
    /* 0x1C */ EXCH_ITEM_BIG_POE,
    /* 0x1D */ EXCH_ITEM_LETTER_RUTO,
    /* 0x1E */ EXCH_ITEM_MAX
} ExchangeItemIdOot;
// The item action Link shows for an OoT exchange item (PLAYER_IA_NONE for none or Ruto's Letter, which Link cannot
// show yet), and back (EXCH_ITEM_NONE for an item action OoT has no exchange item for).
PlayerItemAction Hyrule_ExchangeItemToActionOot(s32 exchangeItem);
s32 Hyrule_ActionToExchangeItemOot(PlayerItemAction itemAction);
// OoT's Actor_GetPlayerExchangeItemId: the exchange item Link showed.
s32 Actor_GetPlayerExchangeItemIdOot(struct PlayState* play);
// Whether an OoT actor that asked for `offer` takes the item Link shows, by OoT's rule: only the item it asked for,
// a Big Poe for a Poe, and anything when it asked for Blue Fire. MM's actors take any item.
s32 Hyrule_IsExchangeAcceptedOot(PlayerItemAction offer, PlayerItemAction shown);

// The B-button item for an equipped sword value, MM's or 3Ship's (ITEM_NONE for none).
u16 Hyrule_SwordValueToItem(u8 value);
// MM's sword owned (Kokiri, Razor or Gilded), as an equip value; for its sheath while an OoT sword is worn.
u8 Hyrule_GetOwnedMmSwordValue(void);

#ifdef __cplusplus
}
#endif
