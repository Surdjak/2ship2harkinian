#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"
#include "HyruleInventory.h"

// 3Ship [Hyrule] OoT's part of the shared inventory (inventoryOot, docs/INVENTORY.md). Older saves start with
// no OoT items, wearing the Kokiri tunic and boots, and owning MM's sword and shield.
void SaveManager_Migration_11(nlohmann::json& j) {
    nlohmann::json& saveInfo = j["save"]["saveInfo"];
    u16 equipment = saveInfo["equips"]["equipment"].get<u16>();
    u32 owned = EQUIP_OWNED_BIT(EQUIP_TYPE_TUNIC, EQUIP_OWNED_KOKIRI) |
                EQUIP_OWNED_BIT(EQUIP_TYPE_BOOTS_Oot, EQUIP_OWNED_KOKIRI);

    switch (equipment & 0xF) {
        case EQUIP_VALUE_SWORD_KOKIRI:
            owned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI);
            break;
        case EQUIP_VALUE_SWORD_RAZOR:
            owned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR);
            break;
        case EQUIP_VALUE_SWORD_GILDED:
            owned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GILDED);
            break;
        default:
            break;
    }
    switch ((equipment >> 4) & 0xF) {
        case EQUIP_VALUE_SHIELD_HERO:
            owned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_HERO);
            break;
        case EQUIP_VALUE_SHIELD_MIRROR:
            owned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SHIELD, EQUIP_OWNED_SHIELD_MIRROR_MM);
            break;
        default:
            break;
    }
    saveInfo["equips"]["equipment"] =
        (u16)((equipment & 0xFF) | (EQUIP_VALUE_TUNIC_KOKIRI << 8) | (EQUIP_VALUE_BOOTS_KOKIRI_Oot << 12));

    saveInfo["inventoryOot"] = {
        { "items", std::vector<u16>(SLOT_OOT_COUNT, ITEM_NONE) },
        { "ammo", std::vector<s8>(SLOT_OOT_COUNT, 0) },
        { "hookshotAdult", (u16)ITEM_NONE },
        { "masksOwned", (u8)0 },
        { "equipmentOwned", owned },
        { "questItems", (u32)0 },
        { "dungeonItems", std::vector<u8>(ARRAY_COUNT(InventoryOot::dungeonItems), 0) },
        { "dungeonKeys", std::vector<s8>(ARRAY_COUNT(InventoryOot::dungeonKeys), -1) },
        { "gsFlags", std::vector<u32>(ARRAY_COUNT(InventoryOot::gsFlags), 0) },
        { "gsTokens", (s16)0 },
    };
}
