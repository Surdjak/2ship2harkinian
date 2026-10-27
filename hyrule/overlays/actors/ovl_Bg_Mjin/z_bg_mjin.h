#ifndef Z_BG_MJIN_OOT_H
#define Z_BG_MJIN_OOT_H

#include "global.h"

struct BgMjin;

typedef void (*BgMjinActionFunc)(struct BgMjin*, PlayState*);

typedef struct BgMjin {
    /* 0x0000 */ DynaPolyActor dyna;
    /* 0x0164 */ s8 objectSlot;
    /* 0x0168 */ BgMjinActionFunc actionFunc;
} BgMjin; // size = 0x016C

#endif
