#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] OoT's areas visited (worldMapAreaDataOot), for Hyrule's map page. Older saves have seen none.
void SaveManager_Migration_13(nlohmann::json& j) {
    j["save"]["saveInfo"]["worldMapAreaDataOot"] = (u32)0;
}
