// OoT's 23 preset damage tables (z_collision_btltbls.c), as 64-entry tables. Ported from SoH.

#include "HyruleActor.h"

static DamageTable sDamageTablePresets[] = {
    // 0
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0xE), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0xF), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0x2),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x2), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 1 Used by En_Karebaba
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0xE), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0xF), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0x2),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x2), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 2 Used by En_St, En_Ssh
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(2, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(3, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(8, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 3
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 4 Used by En_Dodojr
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(2, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(8, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 5
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x3), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x3), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 6
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(3, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(6, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(6, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(6, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 7
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(3, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(6, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(6, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(6, 0x0),
                       DMG_ENTRY(6, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 8
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(3, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 9 Used by En_Bubble
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(8, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 10
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0xE),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0x2),
                       DMG_ENTRY(2, 0x3), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x2), DMG_ENTRY(2, 0x3), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 11 Used by En_Horse
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(9, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 12
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 13
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x2),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x2), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 14 Used by En_Sw
    DMG_TABLE_FROM_OOT(DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(8, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 15 Used by En_Fd
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 16 Used by En_Fw
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(4, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(4, 0x0), DMG_ENTRY(8, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 17
    DMG_TABLE_FROM_OOT(DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 18
    DMG_TABLE_FROM_OOT(DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 19
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0xF), DMG_ENTRY(2, 0x2),
                       DMG_ENTRY(2, 0x3), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x2), DMG_ENTRY(2, 0x3), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 20
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0xF),
                       DMG_ENTRY(0, 0xE), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0xD), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x3), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x3), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0),
                       DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 21
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0xF),
                       DMG_ENTRY(0, 0x1), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0xF), DMG_ENTRY(0, 0x1),
                       DMG_ENTRY(1, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(2, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(2, 0x1), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(2, 0x1), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(1, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
    // 22 Used by En_Du, En_Go, En_Ma1, En_Ma2, En_Ma3
    DMG_TABLE_FROM_OOT(DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0),
                       DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0), DMG_ENTRY(0, 0x0)),
};

DamageTable* DamageTable_Get_Oot(s32 index) {
    if (!(0 <= index && index < ARRAY_COUNT(sDamageTablePresets))) {
        return NULL;
    }
    return &sDamageTablePresets[index];
}

s32 DamageTable_IsMm(DamageTable* damageTable) {
    s32 i;

    for (i = 32; i < ARRAY_COUNT(damageTable->attack); i++) {
        if (damageTable->attack[i] != 0) {
            return false;
        }
    }
    return true;
}

// MM's creatures that are OoT's too, by what OoT's boomerang does to them there. The others, and every
// MM-only creature, are stunned, as OoT's boomerang stuns most enemies.
s32 DamageTable_GetOotBoomerangAttack(Actor* actor) {
    switch (actor->id) {
        // Damaged: Guay, Deku Baba, Wilted Deku Baba (dies to any hit), Mad Scrub, Keese, Octorok, Skulltulas
        // on walls and Gold Skulltulas, Big Poe (OoT's Poes), Leever.
        case ACTOR_EN_CROW:
        case ACTOR_EN_DEKUBABA:
        case ACTOR_EN_KAREBABA:
        case ACTOR_EN_DEKUNUTS:
        case ACTOR_EN_FIREFLY:
        case ACTOR_EN_OKUTA:
        case ACTOR_EN_SW:
        case ACTOR_EN_BIGPO:
        case ACTOR_EN_NEO_REEBA:
            return 4; // DMG_ZORA_BOOMERANG

        // Unaffected: Red Bubble, Freezard, ReDead and Gibdos, Shellblade, the Poe Sisters, Iron Knuckle.
        case ACTOR_EN_BBFALL:
        case ACTOR_EN_FZ:
        case ACTOR_EN_RD:
        case ACTOR_EN_RAILGIBUD:
        case ACTOR_EN_TALK_GIBUD:
        case ACTOR_EN_SB:
        case ACTOR_EN_PO_SISTERS:
        case ACTOR_EN_IK:
            return -1;

        default:
            return 0; // DMG_DEKU_NUT
    }
}
