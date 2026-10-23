#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] Room for OoT's scenes: scene flags grew from 120 to 256 scenes, and scenesVisible from
// 7 to 8 words. Pad older saves with empty entries.
void SaveManager_Migration_9(nlohmann::json& j) {
    nlohmann::json& saveInfo = j["save"]["saveInfo"];

    nlohmann::json& sceneFlags = saveInfo["permanentSceneFlags"];
    while (sceneFlags.size() < sizeof(SaveInfo::permanentSceneFlags) / sizeof(PermanentSceneFlags)) {
        sceneFlags.push_back({ { "chest", 0 },
                               { "switch0", 0 },
                               { "switch1", 0 },
                               { "clearedRoom", 0 },
                               { "collectible", 0 },
                               { "unk_14", 0 },
                               { "rooms", 0 } });
    }

    nlohmann::json& scenesVisible = saveInfo["scenesVisible"];
    while (scenesVisible.size() < sizeof(SaveInfo::scenesVisible) / sizeof(u32)) {
        scenesVisible.push_back(0);
    }
}
