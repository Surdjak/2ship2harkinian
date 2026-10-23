#pragma once

// Where OoT's actors and objects start in 2Ship's unified tables (mm/include/tables/*_oot.h). A unified
// id is the base plus the OoT id. This header has no dependencies so OTRExporter can use it too;
// HyruleScenes.cpp checks the values against the tables at compile time.

#define ACTOR_OOT_BASE 0x2B2
#define OBJECT_OOT_BASE 0x283
// Item ids are not a table: OoT's start above MM's last one (ITEM_NONE, 0xFF), so MM's keep their values.
#define ITEM_OOT_BASE 0x100
// Text ids stay 16-bit: MM's run to 0x354C (and 0xFFFC), OoT's to 0x71B2, so OoT's fit above 0x8000.
#define TEXT_OOT_BASE 0x8000
#define TEXT_OOT(ootTextId) (TEXT_OOT_BASE + (ootTextId))
