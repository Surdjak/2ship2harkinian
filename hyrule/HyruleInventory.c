// OoT's items in the shared inventory. See docs/INVENTORY.md and HyruleInventory.h.
//
// Item_Give for OoT ids follows OoT's (z_parameter.c in OoT): items both games have are handed to MM's
// Item_Give under MM's id, so MM's ammo, capacities and HUD handle them; OoT's own items, equipment, quest
// status and Gold Skulltulas are written to SaveInfo.inventoryOot.

#include "global.h"
#include "HyruleInventory.h"
#include "HyruleIds.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"
#include "HyrulePlayer.h"

#define INVENTORY_OOT (gSaveContext.save.saveInfo.inventoryOot)
#define OOT_ITEM(ootItem) ((ootItem) - ITEM_OOT_BASE)

// MM's item for each OoT item both games have, by OoT id; 0 for the rest. The Ocarina of Time, which is MM's
// item 0, is handled on its own (Inventory_GetMmItem).
static const u16 sMmItems[] = {
    [OOT_ITEM(ITEM_STICK_Oot)] = ITEM_DEKU_STICK,
    [OOT_ITEM(ITEM_NUT_Oot)] = ITEM_DEKU_NUT,
    [OOT_ITEM(ITEM_BOMB_Oot)] = ITEM_BOMB,
    [OOT_ITEM(ITEM_BOW_Oot)] = ITEM_BOW,
    [OOT_ITEM(ITEM_ARROW_FIRE_Oot)] = ITEM_ARROW_FIRE,
    [OOT_ITEM(ITEM_BOMBCHU_Oot)] = ITEM_BOMBCHU,
    [OOT_ITEM(ITEM_ARROW_ICE_Oot)] = ITEM_ARROW_ICE,
    [OOT_ITEM(ITEM_LENS_Oot)] = ITEM_LENS_OF_TRUTH,
    [OOT_ITEM(ITEM_BEAN_Oot)] = ITEM_MAGIC_BEANS,
    [OOT_ITEM(ITEM_ARROW_LIGHT_Oot)] = ITEM_ARROW_LIGHT,
    [OOT_ITEM(ITEM_SWORD_KOKIRI_Oot)] = ITEM_SWORD_KOKIRI,
    [OOT_ITEM(ITEM_QUIVER_30_Oot)] = ITEM_QUIVER_30,
    [OOT_ITEM(ITEM_QUIVER_40_Oot)] = ITEM_QUIVER_40,
    [OOT_ITEM(ITEM_QUIVER_50_Oot)] = ITEM_QUIVER_50,
    [OOT_ITEM(ITEM_BOMB_BAG_20_Oot)] = ITEM_BOMB_BAG_20,
    [OOT_ITEM(ITEM_BOMB_BAG_30_Oot)] = ITEM_BOMB_BAG_30,
    [OOT_ITEM(ITEM_BOMB_BAG_40_Oot)] = ITEM_BOMB_BAG_40,
    [OOT_ITEM(ITEM_WALLET_ADULT_Oot)] = ITEM_WALLET_ADULT,
    [OOT_ITEM(ITEM_WALLET_GIANT_Oot)] = ITEM_WALLET_GIANT,
    [OOT_ITEM(ITEM_SONG_EPONA_Oot)] = ITEM_SONG_EPONA,
    [OOT_ITEM(ITEM_SONG_SARIA_Oot)] = ITEM_SONG_SARIA,
    [OOT_ITEM(ITEM_SONG_SUN_Oot)] = ITEM_SONG_SUN,
    [OOT_ITEM(ITEM_SONG_STORMS_Oot)] = ITEM_SONG_STORMS,
    [OOT_ITEM(ITEM_HEART_CONTAINER_Oot)] = ITEM_HEART_CONTAINER,
    [OOT_ITEM(ITEM_HEART_PIECE_Oot)] = ITEM_HEART_PIECE,
    [OOT_ITEM(ITEM_MAGIC_SMALL_Oot)] = ITEM_MAGIC_JAR_SMALL,
    [OOT_ITEM(ITEM_MAGIC_LARGE_Oot)] = ITEM_MAGIC_JAR_BIG,
    [OOT_ITEM(ITEM_HEART_PIECE_2_Oot)] = ITEM_HEART_PIECE_2,
    [OOT_ITEM(ITEM_HEART_Oot)] = ITEM_RECOVERY_HEART,
    [OOT_ITEM(ITEM_RUPEE_GREEN_Oot)] = ITEM_RUPEE_GREEN,
    [OOT_ITEM(ITEM_RUPEE_BLUE_Oot)] = ITEM_RUPEE_BLUE,
    [OOT_ITEM(ITEM_RUPEE_RED_Oot)] = ITEM_RUPEE_RED,
    [OOT_ITEM(ITEM_RUPEE_PURPLE_Oot)] = ITEM_RUPEE_PURPLE,
    [OOT_ITEM(ITEM_RUPEE_GOLD_Oot)] = ITEM_RUPEE_HUGE, // 200 in both
    [OOT_ITEM(ITEM_STICKS_5_Oot)] = ITEM_DEKU_STICKS_5,
    [OOT_ITEM(ITEM_STICKS_10_Oot)] = ITEM_DEKU_STICKS_10,
    [OOT_ITEM(ITEM_NUTS_5_Oot)] = ITEM_DEKU_NUTS_5,
    [OOT_ITEM(ITEM_NUTS_10_Oot)] = ITEM_DEKU_NUTS_10,
    [OOT_ITEM(ITEM_BOMBS_5_Oot)] = ITEM_BOMBS_5,
    [OOT_ITEM(ITEM_BOMBS_10_Oot)] = ITEM_BOMBS_10,
    [OOT_ITEM(ITEM_BOMBS_20_Oot)] = ITEM_BOMBS_20,
    [OOT_ITEM(ITEM_BOMBS_30_Oot)] = ITEM_BOMBS_30,
    [OOT_ITEM(ITEM_BOMBCHUS_5_Oot)] = ITEM_BOMBCHUS_5,
    [OOT_ITEM(ITEM_BOMBCHUS_20_Oot)] = ITEM_BOMBCHUS_20,
    [OOT_ITEM(ITEM_STICK_UPGRADE_20_Oot)] = ITEM_DEKU_STICK_UPGRADE_20,
    [OOT_ITEM(ITEM_STICK_UPGRADE_30_Oot)] = ITEM_DEKU_STICK_UPGRADE_30,
    [OOT_ITEM(ITEM_NUT_UPGRADE_30_Oot)] = ITEM_DEKU_NUT_UPGRADE_30,
    [OOT_ITEM(ITEM_NUT_UPGRADE_40_Oot)] = ITEM_DEKU_NUT_UPGRADE_40,
};

// MM's id for the bottle contents both games have, by OoT id; Ruto's Letter stays an OoT id.
static const u16 sMmBottleContents[] = {
    [OOT_ITEM(ITEM_POTION_RED_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_POTION_RED,
    [OOT_ITEM(ITEM_POTION_GREEN_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_POTION_GREEN,
    [OOT_ITEM(ITEM_POTION_BLUE_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_POTION_BLUE,
    [OOT_ITEM(ITEM_FAIRY_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_FAIRY,
    [OOT_ITEM(ITEM_FISH_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_FISH,
    [OOT_ITEM(ITEM_MILK_BOTTLE_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_MILK_BOTTLE,
    [OOT_ITEM(ITEM_LETTER_RUTO_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_LETTER_RUTO_Oot,
    [OOT_ITEM(ITEM_BLUE_FIRE_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_BLUE_FIRE,
    [OOT_ITEM(ITEM_BUG_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_BUG,
    [OOT_ITEM(ITEM_BIG_POE_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_BIG_POE,
    [OOT_ITEM(ITEM_MILK_HALF_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_MILK_HALF,
    [OOT_ITEM(ITEM_POE_Oot) - OOT_ITEM(ITEM_POTION_RED_Oot)] = ITEM_POE,
};

// The item slot of each OoT-only item that has one.
static s32 Inventory_GetSlotOot(u16 item) {
    switch (item) {
        case ITEM_SLINGSHOT_Oot:
            return SLOT_OOT_SLINGSHOT;
        case ITEM_BOOMERANG_Oot:
            return SLOT_OOT_BOOMERANG;
        case ITEM_HAMMER_Oot:
            return SLOT_OOT_HAMMER;
        case ITEM_DINS_FIRE_Oot:
            return SLOT_OOT_DINS_FIRE;
        case ITEM_FARORES_WIND_Oot:
            return SLOT_OOT_FARORES_WIND;
        case ITEM_NAYRUS_LOVE_Oot:
            return SLOT_OOT_NAYRUS_LOVE;
        default:
            if ((item >= ITEM_WEIRD_EGG_Oot) && (item <= ITEM_SOLD_OUT_Oot)) {
                return SLOT_OOT_TRADE_CHILD;
            }
            if ((item >= ITEM_POCKET_EGG_Oot) && (item <= ITEM_CLAIM_CHECK_Oot)) {
                return SLOT_OOT_TRADE_ADULT;
            }
            return SLOT_NONE;
    }
}

static u16 Inventory_GetMmItem(u16 item) {
    u16 mmItem;

    if (item == ITEM_OCARINA_TIME_Oot) {
        return ITEM_OCARINA_OF_TIME;
    }
    mmItem = (OOT_ITEM(item) < ARRAY_COUNT(sMmItems)) ? sMmItems[OOT_ITEM(item)] : 0;
    return (mmItem != 0) ? mmItem : ITEM_NONE;
}

// OoT indexes dungeon items by dungeon scene; boss rooms count as their dungeon.
static s32 Inventory_GetDungeonIndexOot(PlayState* play) {
    s32 ootScene = (play != NULL) ? Hyrule_GetOotSceneId(play->sceneId) : -1;

    if ((ootScene >= 0x00) && (ootScene <= 0x0D)) { // SCENE_DEKU_TREE to SCENE_INSIDE_GANONS_CASTLE
        return ootScene;
    }
    if ((ootScene >= 0x11) && (ootScene <= 0x18)) { // SCENE_DEKU_TREE_BOSS to SCENE_SHADOW_TEMPLE_BOSS
        return ootScene - 0x11;
    }
    return -1;
}

// Replaces an item on Link's C and D-pad buttons, as OoT does when an item in a slot changes.
static void Inventory_ReplaceButtonItem(PlayState* play, u16 oldItem, u16 newItem) {
    s32 btn;

    if (oldItem == ITEM_NONE) {
        return;
    }
    for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
        if (GET_CUR_FORM_BTN_ITEM(btn) == oldItem) {
            SET_CUR_FORM_BTN_ITEM(btn, newItem);
            if (play != NULL) {
                Interface_LoadItemIconImpl(play, btn);
            }
        }
    }
    for (btn = EQUIP_SLOT_D_RIGHT; btn <= EQUIP_SLOT_D_UP; btn++) {
        if (DPAD_GET_CUR_FORM_BTN_ITEM(btn) == oldItem) {
            DPAD_SET_CUR_FORM_BTN_ITEM(btn, newItem);
            if (play != NULL) {
                Interface_Dpad_LoadItemIconImpl(play, btn);
            }
        }
    }
}

// The bottle slots in order: MM's six, then the four after OoT's slots.
static u16* Inventory_GetBottle(s32 i) {
    return (i < 6) ? &gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + i]
                   : &INV_CONTENT_SLOT_OOT(SLOT_BOTTLE_7 + i - 6);
}

void Hyrule_InitInventoryOot(void) {
    s32 i;

    memset(&INVENTORY_OOT, 0, sizeof(INVENTORY_OOT));
    for (i = 0; i < ARRAY_COUNT(INVENTORY_OOT.items); i++) {
        INVENTORY_OOT.items[i] = ITEM_NONE;
    }
    INVENTORY_OOT.hookshotAdult = ITEM_NONE;
    for (i = 0; i < ARRAY_COUNT(INVENTORY_OOT.dungeonKeys); i++) {
        INVENTORY_OOT.dungeonKeys[i] = -1;
    }
    // OoT starts Link in the Kokiri tunic and boots.
    INVENTORY_OOT.equipmentOwned = EQUIP_OWNED_BIT(EQUIP_TYPE_TUNIC, EQUIP_OWNED_KOKIRI) |
                                   EQUIP_OWNED_BIT(EQUIP_TYPE_BOOTS_Oot, EQUIP_OWNED_KOKIRI);
    gSaveContext.save.saveInfo.equips.equipment &= ~(0xFF << 8);
    gSaveContext.save.saveInfo.equips.equipment |= (EQUIP_VALUE_TUNIC_KOKIRI << 8) | (EQUIP_VALUE_BOOTS_KOKIRI_Oot << 12);
    Hyrule_SyncEquipmentOwnership();
}

void Hyrule_SyncEquipmentOwnership(void) {
    static const u8 sMmSwords[] = { EQUIP_OWNED_SWORD_KOKIRI, EQUIP_OWNED_SWORD_RAZOR, EQUIP_OWNED_SWORD_GILDED };
    u32 mmSwordBits = EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI) |
                      EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR) |
                      EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GILDED);
    u8 sword = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD);

    // MM owns one sword at a time (Kokiri, Razor or Gilded; Fierce Deity's is a mask's): the one worn, or none
    // while the smithy or a thief has it. While an OoT sword is worn, the MM one owned stays as it was.
    if (sword <= EQUIP_VALUE_SWORD_GILDED) {
        INVENTORY_OOT.equipmentOwned &= ~mmSwordBits;
        if (sword != EQUIP_VALUE_SWORD_NONE) {
            INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, sMmSwords[sword - EQUIP_VALUE_SWORD_KOKIRI]);
        }
    }
    // The Great Fairy's Sword is owned while MM's item slot holds it (docs/PAUSE.md frees that slot later).
    if (INV_CONTENT(ITEM_SWORD_GREAT_FAIRY) == ITEM_SWORD_GREAT_FAIRY) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);
    }
    switch (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD)) {
        case EQUIP_VALUE_SHIELD_HERO:
            INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_HERO);
            break;
        case EQUIP_VALUE_SHIELD_MIRROR:
            INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_MIRROR_MM);
            break;
        default:
            break;
    }
}

// Puts a new empty bottle, or fills an empty one, among the ten bottle slots.
static u16 Inventory_GiveBottle(PlayState* play, u16 content) {
    s32 i;
    u16 search = (content == ITEM_BOTTLE) ? ITEM_NONE : ITEM_BOTTLE;

    for (i = 0; i < 10; i++) {
        u16* bottle = Inventory_GetBottle(i);

        if (*bottle == search) {
            *bottle = content;
            if (search == ITEM_BOTTLE) {
                Inventory_ReplaceButtonItem(play, ITEM_BOTTLE, content);
            }
            return ITEM_NONE;
        }
    }
    return content;
}

u16 Hyrule_ItemGive(PlayState* play, u16 item) {
    u16 mmItem = Inventory_GetMmItem(item);
    s32 slot = Inventory_GetSlotOot(item);
    s32 dungeon;

    if (mmItem != ITEM_NONE) {
        // One Kokiri Sword for both worlds (D4): a Razor or Gilded Sword is already it, upgraded.
        if ((item == ITEM_SWORD_KOKIRI_Oot) && (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) != EQUIP_VALUE_SWORD_NONE)) {
            INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI);
            return ITEM_NONE;
        }
        return Item_Give(play, mmItem);
    }

    if ((item >= ITEM_MEDALLION_FOREST_Oot) && (item <= ITEM_MEDALLION_LIGHT_Oot)) {
        INVENTORY_OOT.questItems |= 1 << (item - ITEM_MEDALLION_FOREST_Oot + QUEST_MEDALLION_FOREST_Oot);
    } else if ((item >= ITEM_SONG_MINUET_Oot) && (item <= ITEM_SONG_LULLABY_Oot)) {
        INVENTORY_OOT.questItems |= 1 << (item - ITEM_SONG_MINUET_Oot + QUEST_SONG_MINUET_Oot);
    } else if (item == ITEM_SONG_TIME_Oot) {
        // Zelda's Song of Time is the Song of Double Time (DESIGN.md).
        INVENTORY_OOT.questItems |= 1 << QUEST_SONG_TIME_Oot;
    } else if ((item >= ITEM_KOKIRI_EMERALD_Oot) && (item <= ITEM_GERUDO_CARD_Oot)) {
        INVENTORY_OOT.questItems |= 1 << (item - ITEM_KOKIRI_EMERALD_Oot + QUEST_KOKIRI_EMERALD_Oot);
    } else if (item == ITEM_SKULL_TOKEN_Oot) {
        INVENTORY_OOT.questItems |= 1 << QUEST_SKULL_TOKEN_Oot;
        INVENTORY_OOT.gsTokens++;
    } else if (item == ITEM_SWORD_MASTER_Oot) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_MASTER);
    } else if (item == ITEM_SWORD_KNIFE_Oot) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE);
    } else if (item == ITEM_SWORD_BGS_Oot) {
        // Biggoron's Sword replaces the Giant's Knife, broken or not.
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_BIGGORON);
        INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE_BROKEN);
    } else if (item == ITEM_SHIELD_DEKU_Oot) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_DEKU);
    } else if (item == ITEM_SHIELD_HYLIAN_Oot) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_HYLIAN);
    } else if (item == ITEM_SHIELD_MIRROR_Oot) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_MIRROR_OOT);
    } else if ((item >= ITEM_TUNIC_KOKIRI_Oot) && (item <= ITEM_TUNIC_ZORA_Oot)) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_TUNIC, item - ITEM_TUNIC_KOKIRI_Oot);
    } else if ((item >= ITEM_BOOTS_KOKIRI_Oot) && (item <= ITEM_BOOTS_HOVER_Oot)) {
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_BOOTS_Oot, item - ITEM_BOOTS_KOKIRI_Oot);
    } else if ((item >= ITEM_BRACELET_Oot) && (item <= ITEM_GAUNTLETS_GOLD_Oot)) {
        Inventory_ChangeUpgrade(UPG_STRENGTH, item - ITEM_BRACELET_Oot + 1);
    } else if ((item == ITEM_SCALE_SILVER_Oot) || (item == ITEM_SCALE_GOLDEN_Oot)) {
        Inventory_ChangeUpgrade(UPG_SCALE, item - ITEM_SCALE_SILVER_Oot + 1);
    } else if (item == ITEM_SLINGSHOT_Oot) {
        Inventory_ChangeUpgrade(UPG_BULLET_BAG, 1);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_SLINGSHOT) = ITEM_SLINGSHOT_Oot;
        AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT) = 30;
    } else if ((item >= ITEM_BULLET_BAG_30_Oot) && (item <= ITEM_BULLET_BAG_50_Oot)) {
        Inventory_ChangeUpgrade(UPG_BULLET_BAG, item - ITEM_BULLET_BAG_30_Oot + 1);
        AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT) = CUR_CAPACITY(UPG_BULLET_BAG);
    } else if ((item == ITEM_SEEDS_Oot) || (item == ITEM_SEEDS_30_Oot)) {
        AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT) =
            CLAMP_MAX(AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT) + ((item == ITEM_SEEDS_Oot) ? 5 : 30),
                      CUR_CAPACITY(UPG_BULLET_BAG));
    } else if ((item >= ITEM_ARROWS_SMALL_Oot) && (item <= ITEM_ARROWS_LARGE_Oot)) {
        static const s16 sArrows[] = { 5, 10, 30 };

        Inventory_ChangeAmmo(ITEM_BOW, sArrows[item - ITEM_ARROWS_SMALL_Oot]);
    } else if (item == ITEM_OCARINA_FAIRY_Oot) {
        // One ocarina slot (D2): the Fairy Ocarina, until the Ocarina of Time replaces it.
        if (INV_CONTENT(ITEM_OCARINA_OF_TIME) == ITEM_NONE) {
            INV_CONTENT(ITEM_OCARINA_OF_TIME) = ITEM_OCARINA_FAIRY;
        }
    } else if ((item == ITEM_HOOKSHOT_Oot) || (item == ITEM_LONGSHOT_Oot)) {
        // Adult Link's hookshot (D3); the Longshot replaces OoT's Hookshot.
        if (INVENTORY_OOT.hookshotAdult != ITEM_LONGSHOT_Oot) {
            INVENTORY_OOT.hookshotAdult = item;
        }
    } else if (item == ITEM_BOTTLE_Oot) {
        return (Inventory_GiveBottle(play, ITEM_BOTTLE) == ITEM_NONE) ? ITEM_NONE : item;
    } else if (((item >= ITEM_POTION_RED_Oot) && (item <= ITEM_POE_Oot)) || (item == ITEM_MILK_Oot)) {
        u16 content = (item == ITEM_MILK_Oot) ? ITEM_MILK_BOTTLE
                                              : sMmBottleContents[item - ITEM_POTION_RED_Oot];

        // A bottle that comes filled (Talon's milk, Ruto's Letter) is a new bottle; other contents fill one.
        if ((item == ITEM_MILK_BOTTLE_Oot) || (item == ITEM_LETTER_RUTO_Oot)) {
            if (Inventory_GiveBottle(play, ITEM_BOTTLE) != ITEM_NONE) {
                return item;
            }
        }
        return (Inventory_GiveBottle(play, content) == ITEM_NONE) ? ITEM_NONE : item;
    } else if ((item >= ITEM_MASK_KEATON_Oot) && (item <= ITEM_MASK_TRUTH_Oot)) {
        // OoT's masks are kept once owned (DESIGN.md), and shown in the child trade slot.
        INVENTORY_OOT.masksOwned |= 1 << (item - ITEM_MASK_KEATON_Oot);
        Inventory_ReplaceButtonItem(play, INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD), item);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) = item;
    } else if (slot == SLOT_OOT_TRADE_CHILD || slot == SLOT_OOT_TRADE_ADULT) {
        if (item == ITEM_SAW_Oot) {
            SET_ITEMGETINF_OOT(ITEMGETINF_OBTAINED_NUT_UPGRADE_FROM_STAGE_Oot);
        }
        Inventory_ReplaceButtonItem(play, INV_CONTENT_SLOT_OOT(slot), (item != ITEM_SOLD_OUT_Oot) ? item : ITEM_NONE);
        INV_CONTENT_SLOT_OOT(slot) = item;
    } else if (slot != SLOT_NONE) {
        INV_CONTENT_SLOT_OOT(slot) = item;
    } else if ((item == ITEM_KEY_BOSS_Oot) || (item == ITEM_COMPASS_Oot) || (item == ITEM_DUNGEON_MAP_Oot)) {
        if ((dungeon = Inventory_GetDungeonIndexOot(play)) < 0) {
            return item;
        }
        INVENTORY_OOT.dungeonItems[dungeon] |= 1 << (item - ITEM_KEY_BOSS_Oot);
    } else if (item == ITEM_KEY_SMALL_Oot) {
        if ((dungeon = Inventory_GetDungeonIndexOot(play)) < 0) {
            return item;
        }
        INVENTORY_OOT.dungeonKeys[dungeon] = (INVENTORY_OOT.dungeonKeys[dungeon] < 0) ? 1 : INVENTORY_OOT.dungeonKeys[dungeon] + 1;
    } else if (item == ITEM_SINGLE_MAGIC_Oot) {
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
        gSaveContext.magicFillTarget = MAGIC_NORMAL_METER;
    } else if (item == ITEM_DOUBLE_MAGIC_Oot) {
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
        gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
        gSaveContext.magicFillTarget = MAGIC_DOUBLE_METER;
    } else if (item == ITEM_DOUBLE_DEFENSE_Oot) {
        gSaveContext.save.saveInfo.playerData.doubleDefense = true;
    } else {
        // The C-button arrows, the fishing pole and OoT's invalid ids are not items to keep.
        return item;
    }
    return ITEM_NONE;
}

u16 Hyrule_ItemCheckObtainability(u16 item) {
    u16 mmItem = Inventory_GetMmItem(item);
    s32 slot = Inventory_GetSlotOot(item);

    if (mmItem != ITEM_NONE) {
        return (Item_CheckObtainability(mmItem) == ITEM_NONE) ? ITEM_NONE : item;
    }
    if (slot != SLOT_NONE) {
        return INV_CONTENT_SLOT_OOT(slot);
    }
    return ITEM_NONE;
}

u16 Hyrule_InvContentOot(u16 item) {
    s32 slot = Inventory_GetSlotOot(item);
    u16 mmItem;

    if (slot != SLOT_NONE) {
        return INV_CONTENT_SLOT_OOT(slot);
    }
    if ((item == ITEM_HOOKSHOT_Oot) || (item == ITEM_LONGSHOT_Oot)) {
        return INVENTORY_OOT.hookshotAdult;
    }
    if (item == ITEM_OCARINA_FAIRY_Oot) {
        mmItem = INV_CONTENT(ITEM_OCARINA_OF_TIME);
        return (mmItem == ITEM_OCARINA_FAIRY) ? ITEM_OCARINA_FAIRY_Oot
                                              : ((mmItem == ITEM_OCARINA_OF_TIME) ? ITEM_OCARINA_TIME_Oot : ITEM_NONE);
    }
    // A shared item: OoT's id if MM's slot holds it.
    mmItem = Inventory_GetMmItem(item);
    if ((mmItem == ITEM_NONE) || (mmItem >= ARRAY_COUNT(gItemSlots))) {
        return ITEM_NONE;
    }
    return (INV_CONTENT(mmItem) == mmItem) ? item : ITEM_NONE;
}

s32 Hyrule_CheckQuestItemOot(s32 quest) {
    switch (quest) {
        case QUEST_SONG_EPONA_Oot:
            return CHECK_QUEST_ITEM(QUEST_SONG_EPONA);
        case QUEST_SONG_SARIA_Oot:
            return CHECK_QUEST_ITEM(QUEST_SONG_SARIA);
        case QUEST_SONG_SUN_Oot:
            return CHECK_QUEST_ITEM(QUEST_SONG_SUN);
        case QUEST_SONG_STORMS_Oot:
            return CHECK_QUEST_ITEM(QUEST_SONG_STORMS);
        default:
            return (INVENTORY_OOT.questItems >> quest) & 1;
    }
}

u8 Hyrule_GetGsFlagsOot(s32 index) {
    return (INVENTORY_OOT.gsFlags[index >> 2] >> ((index & 3) * 8)) & 0xFF;
}

void Hyrule_SetGsFlagsOot(s32 index, u8 value) {
    INVENTORY_OOT.gsFlags[index >> 2] |= (u32)value << ((index & 3) * 8);
}

void Hyrule_ResetInventoryOotForCycle(PlayState* play) {
    s32 i;
    u16 item;

    // Ammo, as MM empties its own.
    for (i = 0; i < ARRAY_COUNT(INVENTORY_OOT.ammo); i++) {
        INVENTORY_OOT.ammo[i] = 0;
    }
    // Bottle contents, as MM does for its six (Sram_SaveEndOfCycle).
    for (i = SLOT_BOTTLE_7; i <= SLOT_BOTTLE_10; i++) {
        item = INV_CONTENT_SLOT_OOT(i);
        if (((item >= ITEM_POTION_RED) && (item <= ITEM_OBABA_DRINK)) || (item == ITEM_LETTER_RUTO_Oot)) {
            Inventory_ReplaceButtonItem(play, item, ITEM_BOTTLE);
            INV_CONTENT_SLOT_OOT(i) = ITEM_BOTTLE;
        }
    }
    // Trade items, except OoT's masks (D6).
    item = INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD);
    if ((item != ITEM_NONE) && !((item >= ITEM_MASK_KEATON_Oot) && (item <= ITEM_MASK_TRUTH_Oot))) {
        u16 mask = ITEM_NONE;

        for (i = 0; i <= ITEM_MASK_TRUTH_Oot - ITEM_MASK_KEATON_Oot; i++) {
            if (INVENTORY_OOT.masksOwned & (1 << i)) {
                mask = ITEM_MASK_KEATON_Oot + i;
            }
        }
        Inventory_ReplaceButtonItem(play, item, mask);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) = mask;
    }
    item = INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT);
    if (item != ITEM_NONE) {
        Inventory_ReplaceButtonItem(play, item, ITEM_NONE);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) = ITEM_NONE;
    }
    // OoT's dungeon items and keys (D7).
    memset(INVENTORY_OOT.dungeonItems, 0, sizeof(INVENTORY_OOT.dungeonItems));
    for (i = 0; i < ARRAY_COUNT(INVENTORY_OOT.dungeonKeys); i++) {
        INVENTORY_OOT.dungeonKeys[i] = -1;
    }
    // MM turns the Razor Sword back into the Kokiri Sword (Sram_SaveEndOfCycle) when it is worn; while an OoT
    // sword is worn, ownership is the only record of it.
    if ((GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) > EQUIP_VALUE_SWORD_GILDED) &&
        CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR)) {
        INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR);
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI);
    }
}

void Hyrule_LoseGreatFairySword(PlayState* play) {
    INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);

    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot) {
        u8 sword = Hyrule_GetOwnedMmSwordValue();

        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, sword);
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = Hyrule_SwordValueToItem(sword); // human Link's B
        if (CUR_FORM == 0) {
            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        }
    }
}
