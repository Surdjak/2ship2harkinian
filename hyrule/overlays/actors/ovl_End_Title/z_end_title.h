#ifndef Z_END_TITLE_OOT_H
#define Z_END_TITLE_OOT_H

#include "global.h"

struct EndTitle;

typedef struct EndTitle {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ u8 endAlpha;
    /* 0x014D */ u8 tlozAlpha;
    /* 0x014E */ u8 ootAlpha;
} EndTitle; // size = 0x0150

#endif
