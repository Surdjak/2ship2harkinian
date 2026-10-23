#pragma once

// OoT's prerendered room backgrounds. See hyrule/README.md.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// The decoded image (RGBA16, as the room shape's fmt and siz say) of a prerendered room's background,
// given its resource path. NULL if the path is not a background.
void* Hyrule_GetPrerenderImage(const char* path);

#ifdef __cplusplus
}
#endif
