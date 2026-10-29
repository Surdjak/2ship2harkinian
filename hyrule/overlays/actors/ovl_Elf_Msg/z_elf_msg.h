#ifndef Z_ELF_MSG_OOT_H
#define Z_ELF_MSG_OOT_H

#include "global.h"

struct ElfMsg;

typedef void (*ElfMsgActionFunc)(struct ElfMsg*, PlayState*);

typedef struct ElfMsg {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ ElfMsgActionFunc actionFunc;
} ElfMsg; // size = 0x0150

#endif
