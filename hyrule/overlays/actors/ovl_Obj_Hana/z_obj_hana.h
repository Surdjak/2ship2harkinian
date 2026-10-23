#ifndef Z_OBJ_HANA_OOT_H
#define Z_OBJ_HANA_OOT_H

#include "global.h"

struct ObjHana;

typedef struct ObjHana {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ ColliderCylinder collider;
} ObjHana; // size = 0x0198

#endif
