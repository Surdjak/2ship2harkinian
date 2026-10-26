#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] The Giant's Knife's health (inventoryOot.knifeHealth): a whole knife, if one is owned, as
// OoT's swordHealth when it is bought.
void SaveManager_Migration_18(nlohmann::json& j) {
    auto& inventory = j["save"]["saveInfo"]["inventoryOot"];
    uint32_t owned = inventory["equipmentOwned"].get<uint32_t>();

    inventory["knifeHealth"] = (owned & (1u << 5)) ? 8 : 0; // EQUIP_OWNED_SWORD_KNIFE
}
