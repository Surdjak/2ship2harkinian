#pragma once

// Where OoT's actors and objects start in 2Ship's unified tables (mm/include/tables/*_oot.h). A unified
// id is the base plus the OoT id. This header has no dependencies so OTRExporter can use it too;
// HyruleScenes.cpp checks the values against the tables at compile time.

#define ACTOR_OOT_BASE 0x2B2
#define OBJECT_OOT_BASE 0x283
