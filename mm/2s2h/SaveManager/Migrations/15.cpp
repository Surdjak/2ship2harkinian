#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] The OoT mask worn (equippedMaskOot). Older saves wear none.
void SaveManager_Migration_15(nlohmann::json& j) {
    j["save"]["equippedMaskOot"] = 0;
}
