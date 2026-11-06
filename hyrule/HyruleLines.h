#pragma once

// 3Ship's own lines, and OoT lines that the merge rewrites, through 2Ship's custom messages (CustomMessage.h).
// Rewritten OoT lines keep their ids, so actors that wait on them are unchanged.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

typedef enum HyruleLineId {
    // The Happy Mask Salesman (docs/TERMINA.md step 3).
    HYRULE_LINE_MASK_GIFT,           // the Goron, Zora and Gerudo Masks are Link's
    HYRULE_LINE_MASK_GIFT_WITH_HINT, // the same, and come back with the Termina gate's items
    HYRULE_LINE_HERO_SHIELD_OFFER,   // Link has them: the Hero's Shield
    HYRULE_LINE_SALESMAN_FAREWELL,   // he leaves for Termina
} HyruleLineId;

// Continues the open textbox with one of 3Ship's lines, as Message_ContinueTextbox does with a text id.
void Hyrule_ContinueTextbox(struct PlayState* play, HyruleLineId id);

#ifdef __cplusplus
}
#endif
