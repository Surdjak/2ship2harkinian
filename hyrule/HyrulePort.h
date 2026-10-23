#pragma once

// Stand-ins that ported OoT code uses for OoT state 2Ship does not have yet. See hyrule/README.md.

// There is no adult Link yet, and MM's Link is child-sized, so OoT's age checks see a child.
#define LINK_IS_ADULT_OOT false
#define LINK_IS_CHILD_OOT true

// OoT's quest items and inventory are not in the save yet: ported checks see none of them.
#define CHECK_QUEST_ITEM_OOT(item) false
#define INV_CONTENT_OOT(slotItem) ITEM_NONE
