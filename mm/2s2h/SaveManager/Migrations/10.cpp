#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] OoT's event flags (eventChkInfOot, itemGetInfOot, infTableOot). Older saves start
// with them clear.
void SaveManager_Migration_10(nlohmann::json& j) {
    nlohmann::json& saveInfo = j["save"]["saveInfo"];

    saveInfo["eventChkInfOot"] = std::vector<u16>(ARRAY_COUNT(SaveInfo::eventChkInfOot), 0);
    saveInfo["itemGetInfOot"] = std::vector<u16>(ARRAY_COUNT(SaveInfo::itemGetInfOot), 0);
    saveInfo["infTableOot"] = std::vector<u16>(ARRAY_COUNT(SaveInfo::infTableOot), 0);
}
