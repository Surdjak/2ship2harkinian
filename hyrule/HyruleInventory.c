// OoT's items in the shared inventory. See docs/INVENTORY.md and HyruleInventory.h.
//
// Item_Give for OoT ids follows OoT's (z_parameter.c in OoT): items both games have are handed to MM's
// Item_Give under MM's id, so MM's ammo, capacities and HUD handle them; OoT's own items, equipment, quest
// status and Gold Skulltulas are written to SaveInfo.inventoryOot.

#include "global.h"
#include "HyruleInventory.h"
#include "HyruleKaleido.h"
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

s32 Inventory_GetDungeonIndexOot(PlayState* play) {
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
void Inventory_ReplaceButtonItem(PlayState* play, u16 oldItem, u16 newItem) {
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
    // while the smithy or a thief has it. While an OoT sword is worn, or Link is adult (MM's swords are child
    // Link's), the MM one owned stays as it was.
    if ((sword <= EQUIP_VALUE_SWORD_GILDED) && !LINK_IS_ADULT_OOT) {
        INVENTORY_OOT.equipmentOwned &= ~mmSwordBits;
        if (sword != EQUIP_VALUE_SWORD_NONE) {
            INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, sMmSwords[sword - EQUIP_VALUE_SWORD_KOKIRI]);
        }
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
        // OoT's beans come from the bean seller, who counts them (MM's own seller does not).
        if (item == ITEM_BEAN_Oot) {
            INVENTORY_OOT.beansBought++;
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
        // OoT's ITEM_SWORD_KNIFE is the broken Giant's Knife.
        INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE);
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE_BROKEN);
    } else if (item == ITEM_SWORD_BGS_Oot) {
        // Biggoron's Sword replaces the Giant's Knife, broken or not. Medigoron's Giant's Knife is the same item
        // in OoT (Hyrule_GiveGiantsKnife).
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_BIGGORON);
        INVENTORY_OOT.equipmentOwned &= ~(EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE) |
                                          EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE_BROKEN));
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

u16 Hyrule_MessageIconFromOot(u8 ootItem) {
    u16 item = ITEM_OOT_BASE + ootItem;
    u16 mmItem;

    if (ootItem == 0xFE) {
        return 0xFE;
    }
    if ((item >= ITEM_SONG_MINUET_Oot) && (item <= ITEM_SONG_PRELUDE_Oot)) {
        // MM's first six song notes have OoT's warp song colors, in OoT's order.
        return ITEM_SONG_SONATA + (item - ITEM_SONG_MINUET_Oot);
    }
    if ((item >= ITEM_SONG_LULLABY_Oot) && (item <= ITEM_SONG_STORMS_Oot)) {
        return ITEM_SONG_TIME; // a white note, as OoT's other songs have
    }
    mmItem = Inventory_GetMmItem(item);
    if ((mmItem != ITEM_NONE) && (mmItem < ITEM_B8)) {
        return mmItem;
    }
    // OoT's own items (32x32) and quest items (24x24), up to the magic jars.
    return ((item <= ITEM_MAGIC_LARGE_Oot) && (Hyrule_GetItemIcon(item) != NULL)) ? item : 0xFE;
}

u16 Hyrule_ItemCheckObtainability(u16 item) {
    u16 mmItem = Inventory_GetMmItem(item);
    s32 slot = Inventory_GetSlotOot(item);

    // The Ocarina of Time replaces the Fairy Ocarina in MM's slot (INVENTORY.md D2): it is new until the slot holds it.
    // MM's check counts any ocarina there.
    if (item == ITEM_OCARINA_TIME_Oot) {
        return (INV_CONTENT(ITEM_OCARINA_OF_TIME) == ITEM_OCARINA_OF_TIME) ? item : ITEM_NONE;
    }
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
    // The bean seller restocks with the rest of Hyrule's world.
    INVENTORY_OOT.beansBought = 0;
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

u16* Hyrule_GetSlotContent(s16 slot) {
    // The Hookshot's slot holds MM's Hookshot for child Link and OoT's Hookshot or Longshot for adult Link (D3).
    if ((slot == SLOT_HOOKSHOT) && LINK_IS_ADULT_OOT) {
        return &INVENTORY_OOT.hookshotAdult;
    }
    if ((slot >= 0) && (slot < ARRAY_COUNT(gSaveContext.save.saveInfo.inventory.items))) {
        return &gSaveContext.save.saveInfo.inventory.items[slot];
    }
    if ((slot >= SLOT_OOT_FIRST) && (slot < SLOT_OOT_END)) {
        return &INV_CONTENT_SLOT_OOT(slot);
    }
    return NULL;
}

u16 Hyrule_GetNextChildTradeMask(void) {
    u16 item = INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD);
    s32 count = ITEM_MASK_TRUTH_Oot - ITEM_MASK_KEATON_Oot + 1;
    s32 i;

    if ((item < ITEM_MASK_KEATON_Oot) || (item > ITEM_MASK_TRUTH_Oot)) {
        return ITEM_NONE;
    }
    for (i = 1; i < count; i++) {
        s32 mask = (item - ITEM_MASK_KEATON_Oot + i) % count;

        if (INVENTORY_OOT.masksOwned & (1 << mask)) {
            return ITEM_MASK_KEATON_Oot + mask;
        }
    }
    return ITEM_NONE;
}

s32 Hyrule_IsOotItemUsableByForm(s32 form, u16 item) {
    return form == PLAYER_FORM_HUMAN;
}

s32 Hyrule_IsItemUsableAtAge(u16 item) {
    switch (item) {
        case ITEM_SLINGSHOT_Oot:
        case ITEM_BOOMERANG_Oot:
        case ITEM_DEKU_STICK:
        case ITEM_WEIRD_EGG_Oot:
        case ITEM_CHICKEN_Oot:
        case ITEM_LETTER_ZELDA_Oot:
            return LINK_IS_CHILD_OOT;
        case ITEM_HAMMER_Oot:
        case ITEM_HOOKSHOT_Oot:
        case ITEM_LONGSHOT_Oot:
            return LINK_IS_ADULT_OOT;
        default:
            if ((item >= ITEM_POCKET_EGG_Oot) && (item <= ITEM_CLAIM_CHECK_Oot)) {
                return LINK_IS_ADULT_OOT;
            }
            return true;
    }
}

// The sword an age wears by default: the Master Sword for adult Link, MM's sword owned for child Link.
static u8 Hyrule_GetDefaultSwordValue(s32 adult) {
    if (!adult) {
        return Hyrule_GetOwnedMmSwordValue();
    }
    return CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_MASTER) ? EQUIP_VALUE_SWORD_MASTER_Oot
                                                                              : EQUIP_VALUE_SWORD_NONE;
}

// An age's first buttons and equipment, as OoT's Inventory_SwapAgeEquipment gives adult Link his: his sword,
// the Hylian Shield (or the Hero's Shield, the one both ages wear), the Kokiri tunic and boots, and Deku Nuts,
// Bombs and the ocarina on C-left, C-down and C-right; nothing on the D-pad.
static void Hyrule_InitAgeEquips(s32 adult) {
    static const struct {
        u8 button;
        u8 slot;
    } sCItems[] = { { EQUIP_SLOT_C_LEFT, SLOT_DEKU_NUT }, { EQUIP_SLOT_C_DOWN, SLOT_BOMB },
                    { EQUIP_SLOT_C_RIGHT, SLOT_OCARINA } };
    u8 sword = Hyrule_GetDefaultSwordValue(adult);
    u8 shield = EQUIP_VALUE_SHIELD_NONE;
    s32 i;

    for (i = 0; i < 4; i++) {
        BUTTON_ITEM_EQUIP(0, i) = ITEM_NONE;
        C_SLOT_EQUIP(0, i) = SLOT_NONE;
        DPAD_BUTTON_ITEM_EQUIP(0, i) = ITEM_NONE;
        DPAD_SLOT_EQUIP(0, i) = SLOT_NONE;
    }
    for (i = 0; i < ARRAY_COUNT(sCItems); i++) {
        u16 item = *Hyrule_GetSlotContent(sCItems[i].slot);

        if (item != ITEM_NONE) {
            BUTTON_ITEM_EQUIP(0, sCItems[i].button) = item;
            C_SLOT_EQUIP(0, sCItems[i].button) = sCItems[i].slot;
        }
    }
    if (CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_HYLIAN)) {
        shield = EQUIP_VALUE_SHIELD_HYLIAN_Oot;
    } else if (CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_HERO)) {
        shield = EQUIP_VALUE_SHIELD_HERO;
    }
    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = Hyrule_SwordValueToItem(sword);
    gSaveContext.save.saveInfo.equips.equipment = (sword << (EQUIP_TYPE_SWORD * 4)) |
                                                  (shield << (EQUIP_TYPE_SHIELD * 4)) |
                                                  (EQUIP_VALUE_TUNIC_KOKIRI << (EQUIP_TYPE_TUNIC * 4)) |
                                                  (EQUIP_VALUE_BOOTS_KOKIRI_Oot << (EQUIP_TYPE_BOOTS_Oot * 4));
}

void Hyrule_SetAge(s32 adult) {
    AgeEquipsOot* other = &gSaveContext.save.saveInfo.otherAgeEquipsOot;
    AgeEquipsOot current;
    s32 i;

    adult = (adult != 0);
    if (adult == LINK_IS_ADULT_OOT) {
        return;
    }
    // Human Link's row: B and the C-buttons (which MM's other bodies share), the D-pad and the equipment.
    Hyrule_SyncEquipmentOwnership();
    for (i = 0; i < 4; i++) {
        current.buttonItems[i] = BUTTON_ITEM_EQUIP(0, i);
        current.cButtonSlots[i] = C_SLOT_EQUIP(0, i);
        current.dpadItems[i] = DPAD_BUTTON_ITEM_EQUIP(0, i);
        current.dpadSlots[i] = DPAD_SLOT_EQUIP(0, i);
    }
    current.equipment = gSaveContext.save.saveInfo.equips.equipment;
    current.set = true;

    gSaveContext.save.saveInfo.isAdultOot = adult;
    if (!other->set) {
        Hyrule_InitAgeEquips(adult);
    } else {
        for (i = 0; i < 4; i++) {
            BUTTON_ITEM_EQUIP(0, i) = other->buttonItems[i];
            C_SLOT_EQUIP(0, i) = other->cButtonSlots[i];
            DPAD_BUTTON_ITEM_EQUIP(0, i) = other->dpadItems[i];
            DPAD_SLOT_EQUIP(0, i) = other->dpadSlots[i];
        }
        gSaveContext.save.saveInfo.equips.equipment = other->equipment;
        // What a slot holds may have changed at the other age (bottles, trade items, the end of a cycle), and the
        // Hookshot's slot holds each age's own: the buttons show it now, as OoT refreshes its bottles and trade
        // items.
        for (i = 0; i < 4; i++) {
            u16* content = (i != EQUIP_SLOT_B) ? Hyrule_GetSlotContent(C_SLOT_EQUIP(0, i)) : NULL;

            if (content != NULL) {
                BUTTON_ITEM_EQUIP(0, i) = *content;
                if (*content == ITEM_NONE) {
                    C_SLOT_EQUIP(0, i) = SLOT_NONE;
                }
            }
            content = Hyrule_GetSlotContent(DPAD_SLOT_EQUIP(0, i));
            if (content != NULL) {
                DPAD_BUTTON_ITEM_EQUIP(0, i) = *content;
                if (*content == ITEM_NONE) {
                    DPAD_SLOT_EQUIP(0, i) = SLOT_NONE;
                }
            }
        }
    }
    *other = current;
}

void Hyrule_ResetOtherAgeSwordForCycle(void) {
    AgeEquipsOot* child = &gSaveContext.save.saveInfo.otherAgeEquipsOot;
    u8 sword;

    if (!LINK_IS_ADULT_OOT || !child->set) {
        return;
    }
    // Sram_SaveEndOfCycle's sword fix-up on child Link's stored row; MM's sword owned follows it, as
    // Hyrule_SyncEquipmentOwnership has it.
    sword = child->equipment & 0xF;
    if (sword <= EQUIP_VALUE_SWORD_RAZOR) {
        sword = ((STOLEN_ITEM_1 >= ITEM_SWORD_GILDED) || (STOLEN_ITEM_2 >= ITEM_SWORD_GILDED))
                    ? EQUIP_VALUE_SWORD_GILDED
                    : EQUIP_VALUE_SWORD_KOKIRI;
        child->equipment = (child->equipment & ~0xF) | sword;
        child->buttonItems[EQUIP_SLOT_B] = Hyrule_SwordValueToItem(sword);
        INVENTORY_OOT.equipmentOwned &= ~(EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI) |
                                          EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR) |
                                          EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GILDED));
        INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, (sword == EQUIP_VALUE_SWORD_GILDED)
                                                                              ? EQUIP_OWNED_SWORD_GILDED
                                                                              : EQUIP_OWNED_SWORD_KOKIRI);
    }
}

void Hyrule_GiveGreatFairySword(void) {
    INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);
}

s32 Hyrule_OwnsGreatFairySword(void) {
    return CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);
}

void Hyrule_LoseGreatFairySword(PlayState* play) {
    INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);

    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot) {
        u8 sword = Hyrule_GetDefaultSwordValue(LINK_IS_ADULT_OOT);

        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, sword);
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = Hyrule_SwordValueToItem(sword); // human Link's B
        if (CUR_FORM == 0) {
            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        }
    }
}

void Hyrule_GiveGiantsKnife(void) {
    INVENTORY_OOT.equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE_BROKEN);
    INVENTORY_OOT.equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE);
    INVENTORY_OOT.knifeHealth = 8;
}
