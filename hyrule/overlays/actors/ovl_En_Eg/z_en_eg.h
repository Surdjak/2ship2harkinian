#ifndef Z_EN_EG_OOT_H
#define Z_EN_EG_OOT_H

#include "global.h"

struct EnEg;

typedef void (*EnEgActionFunc)(struct EnEg*, PlayState*);

typedef struct EnEg {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ s32 action;
} EnEg; // size = 0x0154
#endif
