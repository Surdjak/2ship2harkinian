#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] The dawns since Biggoron took the broken Giant's Knife (inventoryOot.bgsDayCount). Older saves lack it.
void SaveManager_Migration_20(nlohmann::json& j) {
    j["save"]["saveInfo"]["inventoryOot"]["bgsDayCount"] = 0;
}
