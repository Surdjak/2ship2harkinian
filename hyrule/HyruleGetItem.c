// OoT's get-item table (docs/OOT_ITEMS.md), after MM's: get-item ids from GI_OOT_BASE.

#include "global.h"
#include "HyruleInventory.h"
#include "HyruleSfx.h" // OoT's fanfares and box item sound

extern GetItemEntry sGetItemTable[GI_MAX - 1];

static GetItemEntry sGetItemTableOot[GI_TEXT_0_Oot - GI_OOT_BASE + 1] = {
#include "HyruleGetItemTable.inc"
};

GetItemEntry* Player_GetItemEntry(s32 getItemId) {
    if (getItemId > GI_OOT_BASE) {
        return &sGetItemTableOot[getItemId - GI_OOT_BASE];
    }
    return &sGetItemTable[getItemId - 1];
}

void Hyrule_PlayItemFanfareOot(s32 itemId) {
    // OoT's Player_Action_8084E3C4. Its fanfares' "| 0x900" arguments are left out: 2Ship's fanfare takes none.
    if (((itemId >= ITEM_RUPEE_GREEN_Oot) && (itemId <= ITEM_RUPEE_RED_Oot)) ||
        ((itemId >= ITEM_RUPEE_PURPLE_Oot) && (itemId <= ITEM_RUPEE_GOLD_Oot)) || (itemId == ITEM_HEART_Oot)) {
        Audio_PlaySfx(NA_SE_SY_GET_BOXITEM);
    } else if ((itemId == ITEM_HEART_CONTAINER_Oot) ||
               ((itemId == ITEM_HEART_PIECE_2_Oot) && EQ_MAX_QUEST_HEART_PIECE_COUNT)) {
        Audio_PlayFanfare(NA_BGM_HEART_GET);
    } else if (itemId == ITEM_HEART_PIECE_2_Oot) {
        Audio_PlayFanfare(NA_BGM_SMALL_ITEM_GET);
    } else {
        Audio_PlayFanfare(NA_BGM_ITEM_GET);
    }
}
