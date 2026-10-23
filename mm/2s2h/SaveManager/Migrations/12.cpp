#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"
#include "HyruleInventory.h"

// 3Ship [Hyrule] The Great Fairy's Sword is equipment (docs/INVENTORY.md): a sword in its MM item slot
// becomes owned, the slot empties, and C-buttons and the D-pad drop it. B keeps it, which is how it is worn.
void SaveManager_Migration_12(nlohmann::json& j) {
    nlohmann::json& saveInfo = j["save"]["saveInfo"];
    nlohmann::json& items = saveInfo["inventory"]["items"];

    if (items[SLOT_SWORD_GREAT_FAIRY].get<u8>() != ITEM_SWORD_GREAT_FAIRY) {
        return;
    }
    items[SLOT_SWORD_GREAT_FAIRY] = (u8)ITEM_NONE;
    saveInfo["inventoryOot"]["equipmentOwned"] = saveInfo["inventoryOot"]["equipmentOwned"].get<u32>() |
                                                 EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GREAT_FAIRY);

    nlohmann::json& equips = saveInfo["equips"];
    for (size_t form = 0; form < equips["buttonItems"].size(); form++) {
        for (size_t btn = EQUIP_SLOT_C_LEFT; btn < equips["buttonItems"][form].size(); btn++) {
            if (equips["buttonItems"][form][btn].get<u8>() == ITEM_SWORD_GREAT_FAIRY) {
                equips["buttonItems"][form][btn] = (u8)ITEM_NONE;
                equips["cButtonSlots"][form][btn] = (u8)SLOT_NONE;
            }
        }
    }
    nlohmann::json& dpad = j["save"]["shipSaveInfo"]["dpadEquips"];
    for (size_t form = 0; form < dpad["dpadItems"].size(); form++) {
        for (size_t btn = 0; btn < dpad["dpadItems"][form].size(); btn++) {
            if (dpad["dpadItems"][form][btn].get<u8>() == ITEM_SWORD_GREAT_FAIRY) {
                dpad["dpadItems"][form][btn] = (u8)ITEM_NONE;
                dpad["dpadSlots"][form][btn] = (u8)SLOT_NONE;
            }
        }
    }
}
