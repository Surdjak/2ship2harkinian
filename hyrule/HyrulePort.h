#pragma once

// Stand-ins that ported OoT code uses for OoT state 2Ship does not have yet. See hyrule/README.md.

#include "HyruleScenesPending.h"
#include "HyruleInventory.h"
#include "HyrulePlayer.h" // OoT's exchange items
#include "HyruleElfMessage.h"
#include "HyruleScenes.h" // Navi reads OoT scene cameras

#ifdef __cplusplus
extern "C" {
#endif

// Link's age (docs/ADULT_LINK.md): adult Link is human Link at adult age, as in OoT.
#define LINK_IS_ADULT_OOT (gSaveContext.save.saveInfo.isAdultOot != 0)
#define LINK_IS_CHILD_OOT (!LINK_IS_ADULT_OOT)
// MM's LINK_AGE_IN_YEARS reads MM's LINK_IS_ADULT (a transformation), not OoT's age.
#define LINK_AGE_IN_YEARS_OOT (LINK_IS_ADULT_OOT ? YEARS_ADULT : YEARS_CHILD)
// OoT's gSaveContext.linkAge: LINK_AGE_ADULT 0, LINK_AGE_CHILD 1.
#define LINK_AGE_OOT (LINK_IS_ADULT_OOT ? 0 : 1)
// OoT's sub-timer (the escape and minigame countdowns), which MM's timers do not run yet.
#define Interface_SetSubTimerToFinalSecond(play) ((void)0)

// OoT's quest status and inventory, in the shared inventory (HyruleInventory.h).
#define CHECK_QUEST_ITEM_OOT(item) Hyrule_CheckQuestItemOot(item)
#define INV_CONTENT_OOT(item) Hyrule_InvContentOot(item)
// The magic beans bought from the bean seller (OoT keeps the count in a spare ammo slot).
#define BEANS_BOUGHT_OOT (gSaveContext.save.saveInfo.inventoryOot.beansBought)

// OoT's weather modes (En_Weather_Tag's, 1 to 5) in MM's gWeatherMode, past MM's own: MM's Environment_Init
// would act on them as its own when it keeps the weather (retainWeatherMode or a respawn), and clears them
// otherwise, as OoT's clears its own.
#define WEATHER_MODE_OOT(mode) (0x10 + (mode))

// OoT's day and night are MM's.
#define IS_DAY_OOT (!gSaveContext.save.isNight)
#define IS_NIGHT_OOT (gSaveContext.save.isNight)

// OoT's Gold Skulltula flags, in the shared inventory.
#define GET_GS_FLAGS_OOT(index) Hyrule_GetGsFlagsOot(index)
#define SET_GS_FLAGS_OOT(index, value) Hyrule_SetGsFlagsOot(index, value)

// Navi's hint timer (OoT's saved gSaveContext.naviTimer), not saved yet: after a reload Navi offers her C-up hint
// again.
extern u16 gNaviTimerOot;

// OoT's play->shootingGalleryStatus (HyruleActor.c): its archery minigames' state, cleared on each scene's entry.
extern s8 gShootingGalleryStatusOot;

// OoT's func_800F3F3C: the cutscene effects sequence, one of its effects (HyruleAudio.c).
void Hyrule_PlayCutsceneEffectsSeqOot(u8 effect);

#ifdef __cplusplus
}
#endif
