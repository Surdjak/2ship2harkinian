#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

// 3Ship [Hyrule] OoT's minigame records (highScoresOot) and the beans bought from the bean seller
// (inventoryOot.beansBought). Older saves have neither.
void SaveManager_Migration_19(nlohmann::json& j) {
    j["save"]["saveInfo"]["highScoresOot"] = { 0, 0, 0, 0, 0, 0, 0 };
    j["save"]["saveInfo"]["inventoryOot"]["beansBought"] = 0;
}
