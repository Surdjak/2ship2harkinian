#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] Farore's Wind's warp point (faroresWindOot). Older saves have none.
void SaveManager_Migration_14(nlohmann::json& j) {
    j["save"]["saveInfo"]["faroresWindOot"] = {
        { "pos", { { "x", 0.0f }, { "y", 0.0f }, { "z", 0.0f } } },
        { "yaw", 0 },
        { "playerParams", 0 },
        { "entrance", 0 },
        { "roomIndex", 0 },
        { "data", 0 },
        { "tempSwitchFlags", 0 },
        { "unk_18", 0 },
        { "tempCollectFlags", 0 },
    };
}
