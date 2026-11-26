#pragma once

// A new file's start in OoT's opening, and saving in Hyrule. See docs/TERMINA.md (step 1, T3, T4).

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// After Sram_InitNewSave on file creation: Link starts as in OoT, in his house with OoT's opening, owning nothing.
void Hyrule_InitNewFileOot(void);

// On opening a file whose last permanent save was made in Hyrule (or a new file): Link reloads where OoT puts him
// (Link's house, the Temple of Time or the dungeon's entrance), at the saved day and time. False for MM's saves.
s32 Hyrule_OpenSaveOot(void);

// Whether the pause menu's B opens the save prompt: always in Hyrule, as in OoT, and in Termina once Link has reached
// Clock Town.
s32 Hyrule_CanPauseSaveOot(struct PlayState* play);

// Whether this pause save is OoT's permanent save rather than 2Ship's owl-style pause save (T4).
s32 Hyrule_IsPermanentPauseSaveOot(struct PlayState* play);

// Before a permanent pause save is written: marks one made in Termina, which reloads by OoT's rule (Hyrule_OpenSaveOot).
void Hyrule_OnPermanentPauseSaveOot(struct PlayState* play);

// For MM's own permanent saves (the Song of Time, the first entry to Clock Town...), which reload in Termina.
void Hyrule_ForgetSavedSceneOot(void);

// Where a 2Ship owl-style save (the autosave, or 2Ship's pause save) made in an OoT scene reloads: OoT's rule, as for
// a permanent save there. HYRULE_ENTRANCE_NONE outside OoT's scenes.
u32 Hyrule_GetOwlSaveEntranceOot(struct PlayState* play);

// After a permanent pause save: deletes the file's owl save, which would load in its place (2Ship keeps the autosave's
// and its pause save's owl saves when the file is opened).
void Hyrule_DeleteOwlSaveOot(struct PlayState* play);

#ifdef __cplusplus
}
#endif
