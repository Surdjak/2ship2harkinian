#ifndef Z_EN_STREAM_OOT_H
#define Z_EN_STREAM_OOT_H

#include "global.h"

struct EnStream;

typedef void (*EnStreamActionFunc)(struct EnStream*, PlayState*);

typedef struct EnStream {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ EnStreamActionFunc actionFunc;
    /* 0x0150 */ s32 unk_150;
    /* 0x0154 */ char unk_154[0x4];
} EnStream; // size = 0x0158

#endif
