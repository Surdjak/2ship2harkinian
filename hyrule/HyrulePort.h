#pragma once

// Stand-ins that ported OoT code uses for OoT state 2Ship does not have yet. See hyrule/README.md.

#include "HyruleScenesPending.h"
#include "HyruleInventory.h"

// There is no adult Link yet, and MM's Link is child-sized, so OoT's age checks see a child.
#define LINK_IS_ADULT_OOT false
#define LINK_IS_CHILD_OOT true

// OoT's quest status and inventory, in the shared inventory (HyruleInventory.h).
#define CHECK_QUEST_ITEM_OOT(item) Hyrule_CheckQuestItemOot(item)
#define INV_CONTENT_OOT(item) Hyrule_InvContentOot(item)
// Link cannot offer OoT's trade items yet: they need a C-button, which needs the pause menu (EXCH_ITEM_NONE).
#define ACTOR_GET_PLAYER_EXCHANGE_ITEM_ID_OOT(play) 0

// OoT's day and night are MM's.
#define IS_DAY_OOT (!gSaveContext.save.isNight)
#define IS_NIGHT_OOT (gSaveContext.save.isNight)

// OoT's Gold Skulltula flags, in the shared inventory.
#define GET_GS_FLAGS_OOT(index) Hyrule_GetGsFlagsOot(index)
#define SET_GS_FLAGS_OOT(index, value) Hyrule_SetGsFlagsOot(index, value)
