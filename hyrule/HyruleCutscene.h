#pragma once

// OoT's cutscenes, played by OoT's interpreter (HyruleCutscene.c, docs/CUTSCENES.md). OoT's types, states and command
// ids, from SoH's z64cutscene.h, with an Oot suffix: MM has a CsCmdActorCue and CS_CMD_MISC of its own.

#include "ultra64.h"
#include "z64math.h"
#include "z64cutscene.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* 0x00 */ u16 entrance;       // entrance index upon which the cutscene should trigger
    /* 0x02 */ u8  ageRestriction; // 0 for adult only, 1 for child only, 2 for both ages
    /* 0x03 */ u8  flag;           // eventChkInf flag bound to the entrance cutscene
    /* 0x04 */ const char* script; // the cutscene's path in oot.o2r (OoT's segment address)
} EntranceCutsceneOot;

typedef struct {
    /* 0x00 */ s8    continueFlag;
    /* 0x01 */ s8    cameraRoll;
    /* 0x02 */ u16   nextPointFrame;
    /* 0x04 */ f32   viewAngle; // in degrees
    /* 0x08 */ Vec3s pos;
} CutsceneCameraPointOot; // size = 0x10

typedef struct {
    /* 0x00 */ Vec3f at;
    /* 0x0C */ Vec3f eye;
    /* 0x18 */ s16 roll;
    /* 0x1A */ s16 fov;
} CutsceneCameraAngleOot; // size = 0x1C

typedef struct {
    /* 0x0 */ CutsceneCameraPointOot* atPoints;
    /* 0x4 */ CutsceneCameraPointOot* eyePoints;
    /* 0x8 */ s16 relativeToPlayer;
} CutsceneCameraMoveOot; // size = 0xC

typedef struct {
    /* 0x00 */ u16 base;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
} CsCmdBaseOot; // size = 0x6

typedef struct {
    /* 0x00 */ u16 setting;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
} CsCmdEnvLightingOot; // size = 0x6

typedef struct {
    /* 0x00 */ u16  sequence;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
} CsCmdMusicChangeOot; // size = 0x6

typedef struct {
    /* 0x00 */ u16 type;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
} CsCmdMusicFadeOot; // size = 0x6

typedef struct {
    /* 0x00 */ u16 unk_00;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
    /* 0x06 */ u8  unk_06;
    /* 0x07 */ u8  unk_07;
    /* 0x08 */ u8  unk_08;
} CsCmdUnknown9Oot; // size = 0xA

typedef struct {
    /* 0x00 */ u16 unk_00;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
    /* 0x06 */ u8  hour;
    /* 0x07 */ u8  minute;
} CsCmdDayTimeOot; // size = 0x8

typedef struct {
    /* 0x00 */ u16 base;
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
    /* 0x06 */ u16 type;
    /* 0x08 */ u16 textId1;
    /* 0x0A */ u16 textId2;
} CsCmdTextboxOot; // size = 0xC

typedef struct {
    /* 0x00 */ u16 action; // "dousa"
    /* 0x02 */ u16 startFrame;
    /* 0x04 */ u16 endFrame;
    union {
        /* 0x06 */ Vec3s rot;
        /* 0x06 */ Vec3us urot;
    };
    /* 0x0C */ Vec3i startPos;
    /* 0x18 */ Vec3i endPos;
    /* 0x24 */ Vec3i normal;
} CsCmdActorCueOot; // size = 0x30

typedef enum {
    CS_STATE_IDLE_Oot,
    CS_STATE_SKIPPABLE_INIT_Oot,
    CS_STATE_SKIPPABLE_EXEC_Oot,
    CS_STATE_UNSKIPPABLE_INIT_Oot,
    CS_STATE_UNSKIPPABLE_EXEC_Oot
} CutsceneStateOot;

typedef enum {
    CS_CMD_00_Oot = 0x0000,
    CS_CMD_CAM_EYE_Oot = 0x0001,
    CS_CMD_CAM_AT_Oot = 0x0002,
    CS_CMD_MISC_Oot = 0x0003,
    CS_CMD_SET_LIGHTING_Oot = 0x0004,
    CS_CMD_CAM_EYE_REL_TO_PLAYER_Oot = 0x0005,
    CS_CMD_CAM_AT_REL_TO_PLAYER_Oot = 0x0006,
    CS_CMD_07_Oot = 0x0007,
    CS_CMD_08_Oot = 0x0008,
    CS_CMD_09_Oot = 0x0009,
    CS_CMD_TEXTBOX_Oot = 0x0013,
    CS_CMD_SET_PLAYER_ACTION_Oot = 0x000A,
    CS_CMD_SET_ACTOR_ACTION_1_Oot = 0x000F,
    CS_CMD_SET_ACTOR_ACTION_2_Oot = 0x000E,
    CS_CMD_SET_ACTOR_ACTION_3_Oot = 0x0019,
    CS_CMD_SET_ACTOR_ACTION_4_Oot = 0x001D,
    CS_CMD_SET_ACTOR_ACTION_5_Oot = 0x001E,
    CS_CMD_SET_ACTOR_ACTION_6_Oot = 0x002C,
    CS_CMD_SET_ACTOR_ACTION_7_Oot = 0x001F,
    CS_CMD_SET_ACTOR_ACTION_8_Oot = 0x0031,
    CS_CMD_SET_ACTOR_ACTION_9_Oot = 0x003E,
    CS_CMD_SET_ACTOR_ACTION_10_Oot = 0x008F,
    CS_CMD_SCENE_TRANS_FX_Oot = 0x002D,
    CS_CMD_NOP_Oot = 0x000B,
    CS_CMD_PLAYBGM_Oot = 0x0056,
    CS_CMD_STOPBGM_Oot = 0x0057,
    CS_CMD_FADEBGM_Oot = 0x007C,
    CS_CMD_SETTIME_Oot = 0x008C,
    CS_CMD_TERMINATOR_Oot = 0x03E8,
    CS_CMD_END_Oot = 0xFFFF
} CutsceneCmdOot;

// TODO confirm correctness, clarify names
typedef enum {
    /* 0x00 */ INVALID_DESTINATION_0_Oot,
    /* 0x01 */ CUTSCENE_MAP_GANON_HORSE_Oot,
    /* 0x02 */ CUTSCENE_MAP_THREE_GODESSES_POST_DEKU_TREE_Oot,
    /* 0x03 */ GERUDO_VALLEY_DIN_Oot,
    /* 0x04 */ DEATH_MOUNTAIN_TRAIL_NAYRU_Oot,
    /* 0x05 */ KOKIRI_FOREST_FARORE_Oot,
    /* 0x06 */ CUTSCENE_MAP_TRIFORCE_CREATION_Oot,
    /* 0x07 */ KOKIRI_FOREST_RECEIVE_KOKIRI_EMERALD_Oot,
    /* 0x08 */ TEMPLE_OF_TIME_AFTER_USE_MS_Oot,
    /* 0x09 */ GERUDO_VALLEY_DIN_2_Oot,
    /* 0x0A */ LINKS_HOUSE_INTRO_Oot,
    /* 0x0B */ KOKIRI_FOREST_INTRO_Oot,
    /* 0x0C */ DEATH_MOUNTAIN_TRAIL_AFTER_GORON_RUBY_Oot,
    /* 0x0D */ ZORAS_FOUNTAIN_AFTER_ZORAS_SAPPHIRE_Oot,
    /* 0x0E */ KOKIRI_FOREST_AFTER_KOKIRI_EMERALD_Oot,
    /* 0x0F */ TEMPLE_OF_TIME_KOKIRI_EMERALD_Oot, //unused
    /* 0x10 */ TEMPLE_OF_TIME_GORON_RUBY_Oot, //unused
    /* 0x11 */ TEMPLE_OF_TIME_ZORAS_SAPPHIRE_Oot, //unused
    /* 0x12 */ TEMPLE_OF_TIME_AFTER_USE_MS_FIRST_Oot,
    /* 0x13 */ DEATH_MOUNTAIN_TRAIL_AFTER_INTRO_Oot,
    /* 0x14 */ INVALID_DESTINATION_14_Oot,
    /* 0x15 */ LAKE_HYLIA_WATER_RISES_Oot,
    /* 0x16 */ DESERT_COLOSSUS_REQUIEM_Oot,
    /* 0x17 */ CUTSCENE_MAP_CURSE_YOU_Oot,
    /* 0x18 */ JABU_JABU_INTRO_Oot,
    /* 0x19 */ CHAMBER_OF_SAGES_LIGHT_MEDALLION_Oot,
    /* 0x1A */ TEMPLE_OF_TIME_KOKIRI_EMERALD_2_Oot, //duplicate of 0x000F
    /* 0x1B */ TEMPLE_OF_TIME_GORON_RUBY_2_Oot, //duplicate of 0x0010
    /* 0x1C */ TEMPLE_OF_TIME_ZORAS_SAPPHIRE_2_Oot, //duplicate of 0x0011
    /* 0x1D */ CHAMBER_OF_SAGES_FOREST_MEDALLION_Oot,
    /* 0x1E */ CHAMBER_OF_SAGES_FIRE_MEDALLION_Oot,
    /* 0x1F */ CHAMBER_OF_SAGES_WATER_MEDALLION_Oot,
    /* 0x20 */ HYRULE_FIELD_FLASHBACK_Oot, //lacs part 4
    /* 0x21 */ HYRULE_FIELD_AFTER_LAKE_HYLIA_OWL_Oot,
    /* 0x22 */ CUTSCENE_MAP_GANON_AFTER_USE_MS_Oot,
    /* 0x23 */ HYRULE_FIELD_INTRO_ZELDA_ESCAPE_Oot,
    /* 0x24 */ INVALID_DESTINATION_24_Oot,
    /* 0x25 */ INVALID_DESTINATION_25_Oot,
    /* 0x26 */ CUTSCENE_MAP_SHEIKAH_LEGEND_Oot, //lacs part 2
    /* 0x27 */ TEMPLE_OF_TIME_ZELDA_REVEAL_Oot, //lacs part 3
    /* 0x28 */ TEMPLE_OF_TIME_GET_LIGHT_ARROWS_Oot, //lacs part 5
    /* 0x29 */ LAKE_HYLIA_AFTER_BLUE_WARP_Oot,
    /* 0x2A */ KAKARIKO_VILLAGE_DRAIN_WELL_Oot,
    /* 0x2B */ WINDMILL_AFTER_DRAIN_WELL_Oot,
    /* 0x2C */ TEMPLE_OF_TIME_AFTER_DOOR_OF_TIME_OPENS_Oot,
    /* 0x2D */ INVALID_DESTINATION_2D_Oot,
    /* 0x2E */ TEMPLE_OF_TIME_AFTER_USE_MS_FIRST_2_Oot, // duplicate of 0x0012
    /* 0x2F */ KAKARIKO_VILLAGE_NOCTURNE_PART_2_Oot,
    /* 0x30 */ DESERT_COLOSSUS_AFTER_REQUIEM_Oot,
    /* 0x31 */ TEMPLE_OF_TIME_AFTER_LIGHT_ARROWS_Oot,
    /* 0x32 */ KAKARIKO_VILLAGE_AFTER_NOCTURNE_Oot,
    /* 0x33 */ HYRULE_FIELD_IMPA_ESCORT_CS_Oot,
    /* 0x34 */ TEMPLE_OF_TIME_SONG_OF_TIME_Oot,
    /* 0x35 */ HYRULE_FIELD_AFTER_SONG_OF_TIME_Oot,
    /* 0x36 */ GERUDO_VALLEY_CREDITS_Oot,
    /* 0x37 */ GERUDO_FORTRESS_CREDITS_Oot,
    /* 0x38 */ KAKARIKO_VILLAGE_CREDITS_Oot,
    /* 0x39 */ DEATH_MOUNTAIN_TRAIL_CREDITS_1_Oot,
    /* 0x3A */ GORON_CITY_CREDITS_Oot, // unused?
    /* 0x3B */ LAKE_HYLIA_CREDITS_Oot,
    /* 0x3C */ ZORAS_FOUNTAIN_CREDITS_Oot, // unused
    /* 0x3D */ ZORAS_DOMAIN_CREDITS_Oot,
    /* 0x3E */ KOKIRI_FOREST_CREDITS_1_Oot,
    /* 0x3F */ KOKIRI_FOREST_CREDITS_2_Oot,
    /* 0x40 */ HYRULE_FIELD_CREDITS_Oot,
    /* 0x41 */ LON_LON_RANCH_CREDITS_1_Oot,
    /* 0x42 */ KAKARIKO_VILLAGE_AFTER_TRAIL_OWL_Oot,
    /* 0x43 */ HTRULE_FIELD_UNUSED_ENTRANCE_Oot,
    /* 0x44 */ CUTSCENE_MAP_FIRE_Oot,
    /* 0x45 */ KOKIRI_FOREST_POST_FOREST_MEDALLION_Oot,
    /* 0x46 */ DEATH_MOUNTAIN_TRAIL_CREDITS_2_Oot,
    /* 0x47 */ TEMPLE_OF_TIME_CREDITS_Oot,
    /* 0x48 */ ZELDAS_COURTYARD_CREDITS_Oot,
    /* 0x49 */ LON_LON_RANCH_CREDITS_1_2_Oot, // duplicate of 0x0041
    /* 0x4A */ LON_LON_RANCH_CREDITS_2_Oot,
    /* 0x4B */ LON_LON_RANCH_CREDITS_3_Oot,
    /* 0x4C */ LON_LON_RANCH_CREDITS_4_Oot,
    /* 0x4D */ LON_LON_RANCH_CREDITS_5_Oot,
    /* 0x4E */ LON_LON_RANCH_CREDITS_6_Oot,
    /* 0x4F */ LON_LON_RANCH_NO_CS_1_Oot,
    /* 0x50 */ LON_LON_RANCH_NO_CS_2_Oot,
    /* 0x51 */ LON_LON_RANCH_NO_CS_3_Oot,
    /* 0x52 */ LON_LON_RANCH_NO_CS_4_Oot,
    /* 0x53 */ LON_LON_RANCH_NO_CS_5_Oot,
    /* 0x54 */ LON_LON_RANCH_NO_CS_6_Oot,
    /* 0x55 */ LON_LON_RANCH_NO_CS_7_Oot,
    /* 0x56 */ LON_LON_RANCH_NO_CS_8_Oot,
    /* 0x57 */ LON_LON_RANCH_NO_CS_9_Oot,
    /* 0x58 */ LON_LON_RANCH_NO_CS_10_Oot,
    /* 0x59 */ LON_LON_RANCH_NO_CS_11_Oot,
    /* 0x5A */ LON_LON_RANCH_NO_CS_12_Oot,
    /* 0x5B */ LON_LON_RANCH_NO_CS_13_Oot,
    /* 0x5C */ LON_LON_RANCH_NO_CS_14_Oot,
    /* 0x5D */ LON_LON_RANCH_NO_CS_15_Oot,
    /* 0x5E */ LON_LON_RANCH_NO_CS_EPONAS_SONG_Oot,
    /* 0x5F */ CONDITIONAL_DESTINATION_Oot, // TODO more descriptive name?
    /* 0x60 */ DESERT_COLOSSUS_SPIRIT_BLUE_WARP_Oot,
    /* 0x61 */ GRAVEYARD_AFTER_SHADOW_BLUE_WARP_Oot,
    /* 0x62 */ DEATH_MOUNTAIN_CRATER_AFTER_FIRE_BLUE_WARP_Oot,
    /* 0x63 */ SACRED_FOREST_MEADOW_AFTER_FOREST_BLUE_WARP_Oot,
    /* 0x64 */ KOKIRI_FOREST_AFTER_FOREST_BLUE_WARP_Oot,
    /* 0x65 */ DESERT_COLOSSUS_AFTER_SILVER_GAUNTLETS_Oot,
    /* 0x66 */ TEMPLE_OF_TIME_FRONT_OF_PEDESTAL_Oot,
    /* 0x67 */ HYRULE_FIELD_TITLE_SCREEN_Oot,
    /* 0x68 */ SPIRIT_TEMPLE_BOSS_TITLE_SCREEN_Oot,
    /* 0x69 */ GRAVEYARD_SUNS_SONG_Oot,
    /* 0x6A */ ROYAL_FAMILYS_TOMB_SUNS_SONG_Oot,
    /* 0x6B */ GANONS_CASTLE_AFTER_FOREST_TRIAL_Oot,
    /* 0x6C */ GANONS_CASTLE_AFTER_WATER_TRIAL_Oot,
    /* 0x6D */ GANONS_CASTLE_AFTER_SHADOW_TRIAL_Oot,
    /* 0x6E */ GANONS_CASTLE_AFTER_FIRE_TRIAL_Oot,
    /* 0x6F */ GANONS_CASTLE_AFTER_LIGHT_TRIAL_Oot,
    /* 0x70 */ GANONS_CASTLE_AFTER_SPIRIT_TRIAL_Oot,
    /* 0x71 */ GANONS_CASTLE_DISPEL_BARRIER_IF_CONDITIONS_Oot,
    /* 0x72 */ HYRULE_FIELD_INTRO_Oot,
    /* 0x73 */ HYRULE_FIELD_AFTER_IMPA_ESCORT_Oot,
    /* 0x74 */ DESERT_COLOSSUS_SPIRIT_BLUE_WARP_2_Oot,
    /* 0x75 */ HYRULE_FIELD_SKY_Oot,
    /* 0x76 */ GANON_BATTLE_TOWER_COLLAPSE_Oot,
    /* 0x77 */ ZELDAS_COURTYARD_RECEIVE_LETTER_Oot
} CutsceneTerminatorDestinationOot;


#define CS_CMD_CONTINUE_Oot 0
#define CS_CMD_STOP_Oot -1

// OoT's CutsceneContext, which MM's PlayState does not have.
typedef struct CutsceneContextOot {
    /* 0x00 */ char unk_00[0x4];
    /* 0x04 */ void* segment;
    /* 0x08 */ u8 state;
    /* 0x0C */ f32 unk_0C;
    /* 0x10 */ u16 frames;
    /* 0x12 */ u16 unk_12;
    /* 0x14 */ s32 unk_14; // the cutscene's sub-camera
    /* 0x18 */ u16 unk_18;
    /* 0x1A */ u8 unk_1A;
    /* 0x1B */ u8 unk_1B;
    /* 0x1C */ CutsceneCameraPointOot* cameraFocus;
    /* 0x20 */ CutsceneCameraPointOot* cameraPosition;
    /* 0x24 */ CsCmdActorCueOot* linkAction;
    /* 0x28 */ CsCmdActorCueOot* npcActions[10]; // "npcdemopnt"
} CutsceneContextOot;

extern CutsceneContextOot gCsCtxOot;
// OoT's z_demo.c frame markers of the camera and text commands, which Bg_Treemouth resets when it restarts a script.
extern u16 D_8015FCC0;
extern u16 D_8015FCC2;
extern u16 D_8015FCC4;
// OoT's play->linkAgeOnLoad: Link's age for the next scene (OoT's LINK_AGE_ADULT 0, LINK_AGE_CHILD 1), or -1.
extern s8 gLinkAgeOnLoadOot;
void Hyrule_ApplyAgeOnLoadOot(void);

struct PlayState;

// Requests OoT's script in gCsCtxOot.segment, as OoT's actors set gSaveContext.cutsceneTrigger (1, or 2 for an entrance
// cutscene); kept apart from MM's trigger.
void Hyrule_TriggerCutsceneOot(u8 trigger);
// Starts an OoT script, as OoT's actors do (play->csCtx.segment = script; gSaveContext.cutsceneTrigger = 1).
void Hyrule_StartCutsceneOot(struct PlayState* play, void* script);
// Starts a scene's cutscene-layer script, as OoT does for a cutscene index of 0xFFF0 and up.
void Hyrule_StartSceneCutsceneOot(struct PlayState* play, void* script);
// OoT's scene cutscenes at the end of Play_Init (HyruleSceneCutscene.c): the cutscene layer's script, or OoT's entrance
// cutscene (OoT's Cutscene_HandleEntranceTriggers).
void Hyrule_HandleSceneCutscenesOot(struct PlayState* play);
// OoT's Cutscene_HandleConditionalTriggers, in Play_Init before the scene layer is chosen: story cutscenes that replace
// the entrance Link takes (Saria on the bridge...).
void Hyrule_HandleConditionalTriggersOot(struct PlayState* play);
// The cycle reset of OoT's event flags, which keeps the first-visit introductions' (C2: once per save).
void Hyrule_ResetEventChkInfOotForCycle(void);
// Whether an OoT script is playing, during which MM's cutscene update stands aside.
s32 Hyrule_IsCutsceneOotPlaying(void);
// Called each frame from Play_Update in place of MM's cutscene update while an OoT script plays or waits to start.
void Hyrule_UpdateCutsceneOot(struct PlayState* play);
// OoT's func_800BB2B4: a point on a cutscene camera spline, then a step along it; true at its end (the warp light's
// sparkles follow one).
s32 Hyrule_CsSplineStepOot(Vec3f* pos, f32* roll, f32* fov, CutsceneCameraPointOot* point, s16* keyFrame,
                           f32* curFrame);
// Called on scene load (OoT's Cutscene_Init, through Play_Init).
void Hyrule_InitCutsceneOot(struct PlayState* play);

// OoT's per-scene environment flags (OoT's play->envFlags), cleared on scene load: cutscenes set them, and actors wait
// on them.
s32 Flags_GetEnvOot(struct PlayState* play, s16 flag);
void Flags_SetEnvOot(struct PlayState* play, s16 flag);
void Flags_UnsetEnvOot(struct PlayState* play, s16 flag);

#ifdef __cplusplus
}
#endif
