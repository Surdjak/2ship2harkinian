#ifndef Z_EN_ISHI_OOT_H
#define Z_EN_ISHI_OOT_H

#include "global.h"

typedef enum {
    /* 0x00 */ ROCK_SMALL,
    /* 0x01 */ ROCK_LARGE
} EnIshiType;

struct EnIshi;

typedef void (*EnIshiActionFunc)(struct EnIshi*, PlayState*);
typedef void (*EnIshiEffectSpawnFunc)(struct EnIshi*, PlayState*);
typedef void (*EnIshiDrawFunc)(struct EnIshi*, PlayState*);

typedef struct EnIshi {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ EnIshiActionFunc actionFunc;
    /* 0x0150 */ ColliderCylinder collider;
} EnIshi; // size = 0x019C

#endif
