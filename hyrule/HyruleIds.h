#pragma once

// Where OoT's actors and objects start in 2Ship's unified tables (mm/include/tables/*_oot.h). A unified
// id is the base plus the OoT id. This header has no dependencies so OTRExporter can use it too;
// HyruleScenes.cpp checks the values against the tables at compile time.

#define ACTOR_OOT_BASE 0x2B2
#define OBJECT_OOT_BASE 0x283
// Item ids are not a table: OoT's start above MM's last one (ITEM_NONE, 0xFF), so MM's keep their values.
#define ITEM_OOT_BASE 0x100
// Get-item ids are 16-bit where they are kept (Player.getItemId); MM's run to GI_MAX (0xBB), OoT's start at 0x100.
#define GI_OOT_BASE 0x100
// Get-item draw ids: MM's run to GID_MAXIMUM (0x76), OoT's start at 0x80 and run past 0xFF, so they are 16-bit
// wherever they are kept (GetItemEntry.gid, Player.getItemDrawIdPlusOne).
#define GID_OOT_BASE 0x80
// Text ids stay 16-bit: MM's run to 0x354C (and 0xFFFC), OoT's to 0x71B2, so OoT's fit above 0x8000.
#define TEXT_OOT_BASE 0x8000
#define TEXT_OOT(ootTextId) (TEXT_OOT_BASE + (ootTextId))
// Sequence ids are 16-bit in 2Ship. OoT's start at 0x400, clear of MM's (below 0x80) and of the ids the
// audio editor and custom sequences use above them. OoT's soundfonts follow MM's, so their ids are
// assigned at load time (mm/src/audio/lib/load.c).
#define SEQ_OOT_BASE 0x400
#define SEQ_OOT(ootSeqId) (SEQ_OOT_BASE + (ootSeqId))
// OoT's NA_BGM_NATURE_AMBIENCE, which plays OoT's nature ambiences.
#define SEQ_OOT_NATURE_AMBIENCE SEQ_OOT(0x1)
// Ambience ids are 8-bit and MM's run to 0x13. OoT's nature ambiences start at 0x40; OoT's 0x13 (none) is
// MM's AMBIENCE_ID_13, which means the same.
#define AMBIENCE_OOT_BASE 0x40
#define AMBIENCE_OOT(ootAmbienceId) (AMBIENCE_OOT_BASE + (ootAmbienceId))
