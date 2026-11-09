#pragma once

// Stand-ins that ported OoT code uses for OoT state 2Ship does not have yet. See hyrule/README.md.

#include "HyruleScenesPending.h"
#include "HyruleInventory.h"
#include "HyrulePlayer.h" // OoT's exchange items
#include "HyruleElfMessage.h"
#include "HyruleScenes.h" // Navi reads OoT scene cameras
#include "z64shrink_window.h" // OoT's Letterbox_ calls are MM's ShrinkWindow_Letterbox_

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
#define LINK_AGE_ADULT 0
#define LINK_AGE_CHILD 1
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

// OoT's play->shootingGalleryStatus and bombchuBowlingStatus are MM's play->bButtonAmmoPlusOne and unk_1887D (Honey and
// Darling's Bombchus), which MM's Player handles as OoT's does.

// Whether a game of Bombchu Bowling is on: the B button then throws its Bombchus (HyruleActor.c).
s32 Hyrule_IsBombchuBowlingOot(struct PlayState* play);

// OoT's one-point camera cutscenes (z_onepointdemo.c). The attention shots are ported (HyruleAttention.c); an actor that
// starts any other one keeps the main camera and gets CAM_ID_NONE for the sub camera (HyruleCamera.c).
s16 OnePointCutscene_Init(struct PlayState* play, s16 csId, s16 timer, struct Actor* actor, s16 parentCamId);
s16 OnePointCutscene_EndCutscene(struct PlayState* play, s16 subCamId);
s32 OnePointCutscene_Attention(struct PlayState* play, struct Actor* actor);
s32 OnePointCutscene_CheckForCategory(struct PlayState* play, s32 actorCategory);
// OoT's func_8005B198: the category of the actor the last attention shot turned to, or -1 before the first.
s32 Hyrule_GetAttentionCategoryOot(void);

// OoT's Audio_PlaySoundTransposed: a sound effect at pos, raised or lowered by semitones (HyruleAudio.c).
void Audio_PlaySoundTransposedOot(Vec3f* pos, u16 sfxId, s8 semitone);
// OoT's Audio_SetFastTempoForTimedMinigame: the timed minigame music speeds up near the end (HyruleAudio.c).
void Audio_SetFastTempoForTimedMinigameOot(void);

// OoT's "play for an actor" (HyruleOcarina.c): an actor that wants Link to play for it asks every frame (OoT's
// PLAYER_STATE2_NEAR_OCARINA_ACTOR), and learns when Link takes out the ocarina for it (OoT's
// PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR), through MM's ocarina offer.
void Hyrule_OfferOcarinaOot(struct PlayState* play, struct Actor* actor);
s32 Hyrule_OcarinaAttemptedOot(struct PlayState* play, struct Actor* actor);

// OoT's Audio_PlayWindmillBgm: the windmill's music, unless it is already playing (HyruleAudio.c).
void Audio_PlayWindmillBgmOot(void);

// OoT's chamber cutscenes (gSaveContext.chamberCutsceneNum, which MM's save context kept): the sage the Chamber of the
// Sages awakens.
typedef enum ChamberCutsceneOot {
    /* 0 */ CHAMBER_CS_FOREST,
    /* 1 */ CHAMBER_CS_FIRE,
    /* 2 */ CHAMBER_CS_WATER,
    /* 3 */ CHAMBER_CS_SPIRIT,
    /* 4 */ CHAMBER_CS_SHADOW,
    /* 5 */ CHAMBER_CS_LIGHT
} ChamberCutsceneOot;

// OoT's Vec2f, whose second member is y (MM's is z).
typedef struct {
    f32 x, y;
} Vec2fOot;

// OoT's AnimationMinimalInfo: an animation with its mode and morph frames.
typedef struct {
    /* 0x00 */ AnimationHeader* animation;
    /* 0x04 */ u8 mode;
    /* 0x08 */ f32 morphFrames;
} AnimationMinimalInfo;

// OoT's func_800F483C: the main BGM's volume (HyruleAudio.c).
void Audio_SetBgmMainVolumeOot(u8 targetVol, u8 volFadeTimer);
// OoT's Audio_PlaySoundIncreasinglyTransposed and its reset, and Morpha's table for it (HyruleAudio.c).
extern u8 gMorphaTransposeTableOot[16];
void Audio_PlaySoundIncreasinglyTransposedOot(Vec3f* pos, s16 sfxId, u8* semitones);
void Audio_ResetIncreasingTransposeOot(void);
// MM's custom lens flare (z_kankyo.c): its scale, color intensity and glare (OoT's screen fill alpha).
extern f32 D_801F4E44;
extern f32 D_801F4E48;
extern s16 D_801F4E4C;

// OoT's func_800F3F3C: the cutscene effects sequence, one of its effects (HyruleAudio.c).
void Hyrule_PlayCutsceneEffectsSeqOot(u8 effect);

#ifdef __cplusplus
}
#endif
