#pragma once

// OoT's items in the shared inventory. See docs/INVENTORY.md. The data is SaveInfo.inventoryOot (z64save.h);
// items both games have stay in MM's Inventory.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// Item slots after MM's 48 (items 24 and masks 24), kept in InventoryOot.items by SLOT - SLOT_OOT_FIRST.
typedef enum SlotOot {
    /* 0x30 */ SLOT_OOT_SLINGSHOT = 0x30,
    /* 0x31 */ SLOT_OOT_BOOMERANG,
    /* 0x32 */ SLOT_OOT_HAMMER,
    /* 0x33 */ SLOT_OOT_DINS_FIRE,
    /* 0x34 */ SLOT_OOT_FARORES_WIND,
    /* 0x35 */ SLOT_OOT_NAYRUS_LOVE,
    /* 0x36 */ SLOT_OOT_TRADE_CHILD,
    /* 0x37 */ SLOT_OOT_TRADE_ADULT,
    /* 0x38 */ SLOT_BOTTLE_7,
    /* 0x39 */ SLOT_BOTTLE_8,
    /* 0x3A */ SLOT_BOTTLE_9,
    /* 0x3B */ SLOT_BOTTLE_10,
    /* 0x3C */ SLOT_OOT_END
} SlotOot;

#define SLOT_OOT_FIRST SLOT_OOT_SLINGSHOT
#define SLOT_OOT_COUNT (SLOT_OOT_END - SLOT_OOT_FIRST)
#define INV_CONTENT_SLOT_OOT(slot) (gSaveContext.save.saveInfo.inventoryOot.items[(slot) - SLOT_OOT_FIRST])
#define AMMO_SLOT_OOT(slot) (gSaveContext.save.saveInfo.inventoryOot.ammo[(slot) - SLOT_OOT_FIRST])

// OoT's trade slots, by their first items as in OoT (z64item.h).
#define ITEM_TRADE_CHILD_Oot ITEM_WEIRD_EGG_Oot
#define ITEM_TRADE_ADULT_Oot ITEM_POCKET_EGG_Oot

// OoT's quest status, with OoT's bit numbers (QuestItem in OoT). Epona's, Saria's and the Sun's Song and
// the Song of Storms are MM's bits (Hyrule_CheckQuestItemOot); OoT's Song of Time bit is the Song of Double
// Time, which does its job (DESIGN.md).
typedef enum QuestItemOot {
    /* 0x00 */ QUEST_MEDALLION_FOREST_Oot,
    /* 0x01 */ QUEST_MEDALLION_FIRE_Oot,
    /* 0x02 */ QUEST_MEDALLION_WATER_Oot,
    /* 0x03 */ QUEST_MEDALLION_SPIRIT_Oot,
    /* 0x04 */ QUEST_MEDALLION_SHADOW_Oot,
    /* 0x05 */ QUEST_MEDALLION_LIGHT_Oot,
    /* 0x06 */ QUEST_SONG_MINUET_Oot,
    /* 0x07 */ QUEST_SONG_BOLERO_Oot,
    /* 0x08 */ QUEST_SONG_SERENADE_Oot,
    /* 0x09 */ QUEST_SONG_REQUIEM_Oot,
    /* 0x0A */ QUEST_SONG_NOCTURNE_Oot,
    /* 0x0B */ QUEST_SONG_PRELUDE_Oot,
    /* 0x0C */ QUEST_SONG_LULLABY_Oot,
    /* 0x0D */ QUEST_SONG_EPONA_Oot,
    /* 0x0E */ QUEST_SONG_SARIA_Oot,
    /* 0x0F */ QUEST_SONG_SUN_Oot,
    /* 0x10 */ QUEST_SONG_TIME_Oot,
    /* 0x11 */ QUEST_SONG_STORMS_Oot,
    /* 0x12 */ QUEST_KOKIRI_EMERALD_Oot,
    /* 0x13 */ QUEST_GORON_RUBY_Oot,
    /* 0x14 */ QUEST_ZORA_SAPPHIRE_Oot,
    /* 0x15 */ QUEST_STONE_OF_AGONY_Oot,
    /* 0x16 */ QUEST_GERUDO_CARD_Oot,
    /* 0x17 */ QUEST_SKULL_TOKEN_Oot
} QuestItemOot;

// Equipment ownership (InventoryOot.equipmentOwned): a byte per type (EQUIP_TYPE_*), a bit per piece.
typedef enum EquipOwnedSword {
    EQUIP_OWNED_SWORD_KOKIRI,
    EQUIP_OWNED_SWORD_RAZOR,
    EQUIP_OWNED_SWORD_GILDED,
    EQUIP_OWNED_SWORD_MASTER,
    EQUIP_OWNED_SWORD_GREAT_FAIRY,
    EQUIP_OWNED_SWORD_KNIFE,
    EQUIP_OWNED_SWORD_BIGGORON,
    EQUIP_OWNED_SWORD_KNIFE_BROKEN
} EquipOwnedSword;

typedef enum EquipOwnedShield {
    EQUIP_OWNED_SHIELD_DEKU,
    EQUIP_OWNED_SHIELD_HERO,
    EQUIP_OWNED_SHIELD_MIRROR_MM,
    EQUIP_OWNED_SHIELD_HYLIAN,
    EQUIP_OWNED_SHIELD_MIRROR_OOT
} EquipOwnedShield;

typedef enum EquipOwnedTunicBoots {
    EQUIP_OWNED_KOKIRI, // tunic or boots
    EQUIP_OWNED_GORON_IRON,
    EQUIP_OWNED_ZORA_HOVER
} EquipOwnedTunicBoots;

#define EQUIP_TYPE_BOOTS_Oot 3
#define EQUIP_OWNED_BIT(type, piece) (1u << ((type) * 8 + (piece)))
#define CHECK_OWNED_EQUIP_OOT(type, piece) \
    ((gSaveContext.save.saveInfo.inventoryOot.equipmentOwned & EQUIP_OWNED_BIT(type, piece)) != 0)

// Equipped values 3Ship adds to MM's (ItemEquips.equipment). The tunic and boots nibbles, which MM leaves
// unused, take OoT's values: MM kept OoT's tunic values (EQUIP_VALUE_TUNIC_*).
#define EQUIP_VALUE_SWORD_MASTER_Oot 5
#define EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot 6
#define EQUIP_VALUE_SWORD_KNIFE_Oot 7
#define EQUIP_VALUE_SWORD_BIGGORON_Oot 8
#define EQUIP_VALUE_SHIELD_DEKU_Oot 3
#define EQUIP_VALUE_SHIELD_HYLIAN_Oot 4
#define EQUIP_VALUE_SHIELD_MIRROR_Oot 5
#define EQUIP_VALUE_BOOTS_KOKIRI_Oot 1
#define EQUIP_VALUE_BOOTS_IRON_Oot 2
#define EQUIP_VALUE_BOOTS_HOVER_Oot 3

// Sets up OoT's part of a new or debug save: Kokiri tunic and boots, worn; no keys.
void Hyrule_InitInventoryOot(void);

// Item_Give and Item_CheckObtainability for OoT item ids (ITEM_*_Oot). Items both games have go to MM's.
u16 Hyrule_ItemGive(struct PlayState* play, u16 item);
u16 Hyrule_ItemCheckObtainability(u16 item);

// What an item slot holds, for MM's slots (items and masks) and OoT's (SLOT_OOT_*) alike; NULL for others. The
// Hookshot's slot is adult Link's own (InventoryOot.hookshotAdult) while he is adult.
u16* Hyrule_GetSlotContent(s16 slot);
// The child trade slot's next OoT mask after the one it holds, among those owned (SoH's Mask Select); ITEM_NONE
// if it holds no mask or no other is owned.
u16 Hyrule_GetNextChildTradeMask(void);

// Whether a body can use an OoT item: human Link only, until Player uses them per body.
s32 Hyrule_IsOotItemUsableByForm(s32 form, u16 item);

// OoT's items by age (docs/ADULT_LINK.md, A4): the Slingshot, Boomerang, Deku Sticks and child trade items are
// child Link's; the Megaton Hammer, OoT's Hookshot and Longshot and the adult trade items are adult Link's. MM's
// items, masks included, and the rest fit both.
s32 Hyrule_IsItemUsableAtAge(u16 item);

// Makes Link a child or an adult (SaveInfo.isAdultOot). Each age keeps its own B, C and D-pad buttons and
// equipment (SaveInfo.otherAgeEquipsOot holds the other's), as OoT's Inventory_SwapAgeEquipment does; an age's
// first ones are OoT's. Reload the scene after.
void Hyrule_SetAge(s32 adult);

// At the end of a cycle while Link is adult: MM's sword fix-up (the Razor Sword back to the Kokiri Sword) on
// child Link's stored equipment.
void Hyrule_ResetOtherAgeSwordForCycle(void);

// OoT's INV_CONTENT: what is in an OoT item's slot, as an OoT item id (ITEM_NONE if empty).
u16 Hyrule_InvContentOot(u16 item);

// OoT's CHECK_QUEST_ITEM, for QUEST_*_Oot.
s32 Hyrule_CheckQuestItemOot(s32 quest);

// OoT's GET_GS_FLAGS and SET_GS_FLAGS.
u8 Hyrule_GetGsFlagsOot(s32 index);
void Hyrule_SetGsFlagsOot(s32 index, u8 value);

// Records MM's sword and shield, which MM keeps only as what is worn, as owned (MM's one sword replacing the
// others).
void Hyrule_SyncEquipmentOwnership(void);

// Medigoron's Giant's Knife (GI_SWORD_KNIFE_Oot), which OoT gives as ITEM_SWORD_BGS with Biggoron's Sword and tells
// apart by its get-item id: owned whole, with OoT's 8 hits (swordHealth).
void Hyrule_GiveGiantsKnife(void);

// The Great Fairy's Sword is equipment rather than an item (docs/INVENTORY.md): Item_Give records it as owned,
// and it is worn from the equipment page.
void Hyrule_GiveGreatFairySword(void);
s32 Hyrule_OwnsGreatFairySword(void);

// The Great Fairy's Sword stolen: no longer owned, and if worn, MM's own sword (or none) goes back on B.
void Hyrule_LoseGreatFairySword(struct PlayState* play);

// Replaces an item on Link's C and D-pad buttons, as OoT does when an item in a slot changes.
void Inventory_ReplaceButtonItem(struct PlayState* play, u16 oldItem, u16 newItem);

// OoT's items at the end of a cycle (docs/INVENTORY.md): ammo, bottle contents, trade items other than
// masks and OoT's dungeon items reset; the rest is kept.
void Hyrule_ResetInventoryOotForCycle(struct PlayState* play);

#ifdef __cplusplus
}
#endif
