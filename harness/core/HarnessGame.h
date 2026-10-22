#pragma once

// Entry points each game adapter implements for the few decomp call sites that need them.

#include "Harness.h"

#ifdef __cplusplus
extern "C" {
#endif

// Called right after the pads are read from the device, with the game's OSContPad array.
void HarnessGame_OnPadRead(void* pads);

#ifdef __cplusplus
}
#endif
