// OoT's get-item table (docs/OOT_ITEMS.md), after MM's: get-item ids from GI_OOT_BASE.

#include "global.h"

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
