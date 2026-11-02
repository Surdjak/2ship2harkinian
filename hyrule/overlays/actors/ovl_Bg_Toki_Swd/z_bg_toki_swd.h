#ifndef Z_BG_TOKI_SWD_OOT_H
#define Z_BG_TOKI_SWD_OOT_H

#include "global.h"

struct BgTokiSwd;

typedef void (*BgTokiSwdActionFunc)(struct BgTokiSwd*, PlayState*);

typedef struct BgTokiSwd {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ BgTokiSwdActionFunc actionFunc;
    /* 0x0150 */ ColliderCylinder collider;
} BgTokiSwd; // size = 0x019C

#endif
