#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] Link's age (isAdultOot). Older saves are child Link's.
void SaveManager_Migration_16(nlohmann::json& j) {
    j["save"]["saveInfo"]["isAdultOot"] = 0;
}
