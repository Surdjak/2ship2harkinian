#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] The other age's buttons and equipment (otherAgeEquipsOot). Older saves have never been the other
// age, so it starts unset.
void SaveManager_Migration_17(nlohmann::json& j) {
    j["save"]["saveInfo"]["otherAgeEquipsOot"] = {
        { "buttonItems", { 0xFF, 0xFF, 0xFF, 0xFF } }, { "cButtonSlots", { 0xFF, 0xFF, 0xFF, 0xFF } },
        { "dpadItems", { 0xFF, 0xFF, 0xFF, 0xFF } },   { "dpadSlots", { 0xFF, 0xFF, 0xFF, 0xFF } },
        { "equipment", 0 },                             { "set", 0 },
    };
}
