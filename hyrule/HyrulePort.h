#pragma once

// Stand-ins that ported OoT code uses for OoT state 2Ship does not have yet. See hyrule/README.md.

#include "HyruleScenesPending.h"

// There is no adult Link yet, and MM's Link is child-sized, so OoT's age checks see a child.
#define LINK_IS_ADULT_OOT false
#define LINK_IS_CHILD_OOT true

// OoT's quest items and inventory are not in the save yet: ported checks see none of them.
#define CHECK_QUEST_ITEM_OOT(item) false
#define INV_CONTENT_OOT(slotItem) ITEM_NONE
// Nor are OoT's trade items, so Link never offers one (EXCH_ITEM_NONE).
#define ACTOR_GET_PLAYER_EXCHANGE_ITEM_ID_OOT(play) 0

// OoT's day and night are MM's.
#define IS_DAY_OOT (!gSaveContext.save.isNight)
#define IS_NIGHT_OOT (gSaveContext.save.isNight)

// OoT's Gold Skulltula flags are not in the save yet: none are collected, and collecting one is not kept.
#define GET_GS_FLAGS_OOT(index) 0
#define SET_GS_FLAGS_OOT(index, value) ((void)0)
