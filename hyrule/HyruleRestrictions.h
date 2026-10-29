#pragma once

// OoT's per-scene restrictions (OoT's sRestrictionFlags and Interface_SetSceneRestrictions), for OoT's scenes. MM's
// restrictions (interfaceCtx.restrictions) cover the rest; OoT's scenes get MM's RESTRICTIONS_NONE.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// OoT's restrictions read so far, by their bit position in OoT's flags (flags2, then flags3).
typedef enum HyruleRestrictionOot {
    /* 0 */ HYRULE_RESTRICTION_OCARINA_OOT,     // 0 allowed
    /* 1 */ HYRULE_RESTRICTION_WARP_SONGS_OOT,  // 3 refused (the only value OoT checks)
    /* 2 */ HYRULE_RESTRICTION_FARORES_OOT,     // 0 allowed
    /* 3 */ HYRULE_RESTRICTION_DINS_NAYRUS_OOT, // 0 allowed
} HyruleRestrictionOot;

// OoT's value for a restriction in the current scene; -1 in MM's scenes, and 0 in OoT's scenes OoT's table leaves out.
s32 Hyrule_GetRestrictionOot(struct PlayState* play, HyruleRestrictionOot restriction);

#ifdef __cplusplus
}
#endif
