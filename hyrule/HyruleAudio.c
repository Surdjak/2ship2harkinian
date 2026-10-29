// OoT audio helpers for 2Ship's audio code. See hyrule/README.md.

#include "global.h"
#include "HyruleIds.h"

// OoT's sequence flags (code_800EC960.c in OoT), which use the same bits as MM's. Ported from SoH.
static u8 sSeqFlagsOot[] = {
    0x2,  // NA_BGM_GENERAL_SFX
    0x1,  // NA_BGM_NATURE_BACKGROUND
    0,    // NA_BGM_FIELD_LOGIC
    0,    // NA_BGM_FIELD_INIT
    0,    // NA_BGM_FIELD_DEFAULT_1
    0,    // NA_BGM_FIELD_DEFAULT_2
    0,    // NA_BGM_FIELD_DEFAULT_3
    0,    // NA_BGM_FIELD_DEFAULT_4
    0,    // NA_BGM_FIELD_DEFAULT_5
    0,    // NA_BGM_FIELD_DEFAULT_6
    0,    // NA_BGM_FIELD_DEFAULT_7
    0,    // NA_BGM_FIELD_DEFAULT_8
    0,    // NA_BGM_FIELD_DEFAULT_9
    0,    // NA_BGM_FIELD_DEFAULT_A
    0,    // NA_BGM_FIELD_DEFAULT_B
    0,    // NA_BGM_FIELD_ENEMY_INIT
    0,    // NA_BGM_FIELD_ENEMY_1
    0,    // NA_BGM_FIELD_ENEMY_2
    0,    // NA_BGM_FIELD_ENEMY_3
    0,    // NA_BGM_FIELD_ENEMY_4
    0,    // NA_BGM_FIELD_STILL_1
    0,    // NA_BGM_FIELD_STILL_2
    0,    // NA_BGM_FIELD_STILL_3
    0,    // NA_BGM_FIELD_STILL_4
    0x21, // NA_BGM_DUNGEON
    0x10, // NA_BGM_KAKARIKO_ADULT
    0,    // NA_BGM_ENEMY
    0x88, // NA_BGM_BOSS
    0x1,  // NA_BGM_INSIDE_DEKU_TREE
    0,    // NA_BGM_MARKET
    0,    // NA_BGM_TITLE
    0x20, // NA_BGM_LINK_HOUSE
    0,    // NA_BGM_GAME_OVER
    0,    // NA_BGM_BOSS_CLEAR
    0x2,  // NA_BGM_ITEM_GET
    0x4,  // NA_BGM_OPENING_GANON
    0x2,  // NA_BGM_HEART_GET
    0x2,  // NA_BGM_OCA_LIGHT
    0x1,  // NA_BGM_JABU_JABU
    0x10, // NA_BGM_KAKARIKO_KID
    0,    // NA_BGM_GREAT_FAIRY
    0,    // NA_BGM_ZELDA_THEME
    0x1,  // NA_BGM_FIRE_TEMPLE
    0x2,  // NA_BGM_OPEN_TRE_BOX
    0x1,  // NA_BGM_FOREST_TEMPLE
    0,    // NA_BGM_COURTYARD
    0x80, // NA_BGM_GANON_TOWER
    0,    // NA_BGM_LONLON
    0x80, // NA_BGM_GORON_CITY
    0,    // NA_BGM_FIELD_MORNING
    0x2,  // NA_BGM_SPIRITUAL_STONE
    0x2,  // NA_BGM_OCA_BOLERO
    0x2,  // NA_BGM_OCA_MINUET
    0x2,  // NA_BGM_OCA_SERENADE
    0x2,  // NA_BGM_OCA_REQUIEM
    0x2,  // NA_BGM_OCA_NOCTURNE
    0x88, // NA_BGM_MINI_BOSS
    0x2,  // NA_BGM_SMALL_ITEM_GET
    0,    // NA_BGM_TEMPLE_OF_TIME
    0x2,  // NA_BGM_EVENT_CLEAR
    0x11, // NA_BGM_KOKIRI
    0x2,  // NA_BGM_OCA_FAIRY_GET
    0x1,  // NA_BGM_SARIA_THEME
    0x1,  // NA_BGM_SPIRIT_TEMPLE
    0,    // NA_BGM_HORSE
    0,    // NA_BGM_HORSE_GOAL
    0,    // NA_BGM_INGO
    0x2,  // NA_BGM_MEDALLION_GET
    0x2,  // NA_BGM_OCA_SARIA
    0x2,  // NA_BGM_OCA_EPONA
    0x2,  // NA_BGM_OCA_ZELDA
    0x2,  // NA_BGM_OCA_SUNS
    0x2,  // NA_BGM_OCA_TIME
    0x2,  // NA_BGM_OCA_STORM
    0,    // NA_BGM_NAVI_OPENING
    0,    // NA_BGM_DEKU_TREE_CS
    0,    // NA_BGM_WINDMILL
    0,    // NA_BGM_HYRULE_CS
    0x20, // NA_BGM_MINI_GAME
    0,    // NA_BGM_SHEIK
    0x10, // NA_BGM_ZORA_DOMAIN
    0x2,  // NA_BGM_APPEAR
    0,    // NA_BGM_ADULT_LINK
    0,    // NA_BGM_MASTER_SWORD
    0x4,  // NA_BGM_INTRO_GANON
    0x20, // NA_BGM_SHOP
    0x40, // NA_BGM_CHAMBER_OF_SAGES
    0x40, // NA_BGM_FILE_SELECT
    0x1,  // NA_BGM_ICE_CAVERN
    0x2,  // NA_BGM_DOOR_OF_TIME
    0x2,  // NA_BGM_OWL
    0x1,  // NA_BGM_SHADOW_TEMPLE
    0x1,  // NA_BGM_WATER_TEMPLE
    0x2,  // NA_BGM_BRIDGE_TO_GANONS
    0,    // NA_BGM_SEAL_OF_SAGES
    0x11, // NA_BGM_GERUDO_VALLEY
    0,    // NA_BGM_POTION_SHOP
    0,    // NA_BGM_KOTAKE_KOUME
    0x80, // NA_BGM_ESCAPE
    0,    // NA_BGM_UNDERGROUND
    0x80, // NA_BGM_GANON_BATTLE_1
    0x80, // NA_BGM_GANON_BATTLE_2
    0,    // NA_BGM_OCARINA_OF_TIME
    0,    // NA_BGM_STAFF_1
    0,    // NA_BGM_STAFF_2
    0,    // NA_BGM_STAFF_3
    0,    // NA_BGM_STAFF_4
    0,    // NA_BGM_FIRE_BOSS
    0x8,  // NA_BGM_TIMED_MINI_GAME
    0,    // NA_BGM_VARIOUS_SFX
};

u8 Hyrule_GetOotSeqFlags(u16 ootSeqId) {
    return (ootSeqId < ARRAY_COUNT(sSeqFlagsOot)) ? sSeqFlagsOot[ootSeqId] : 0;
}

// OoT's nature ambiences (sequence.h and code_800EC960.c in OoT), ported from SoH. MM's ambiences grew out of
// these: the channels and IO ports are the same, but the sound types differ, so OoT's ambience sequence
// needs OoT's values.
typedef enum {
    /* 0x0 */ NATURE_CHANNEL_STREAM_0,
    /* 0x1 */ NATURE_CHANNEL_CRITTER_0,
    /* 0x2 */ NATURE_CHANNEL_CRITTER_1,
    /* 0x3 */ NATURE_CHANNEL_CRITTER_2,
    /* 0x4 */ NATURE_CHANNEL_CRITTER_3,
    /* 0x5 */ NATURE_CHANNEL_CRITTER_4,
    /* 0x6 */ NATURE_CHANNEL_CRITTER_5,
    /* 0x7 */ NATURE_CHANNEL_CRITTER_6,
    /* 0x8 */ NATURE_CHANNEL_CRITTER_7,
    /* 0xC */ NATURE_CHANNEL_STREAM_1 = 12,
    /* 0xD */ NATURE_CHANNEL_UNK,
    /* 0xE */ NATURE_CHANNEL_RAIN,
    /* 0xF */ NATURE_CHANNEL_LIGHTNING
} NatureChannelIdx; // playerIdx = 0 (Overlaps with main bgm)

typedef enum {
    /* 0x00 */ NATURE_ID_GENERAL_NIGHT,
    /* 0x01 */ NATURE_ID_MARKET_ENTRANCE,
    /* 0x02 */ NATURE_ID_KAKARIKO_REGION,
    /* 0x03 */ NATURE_ID_MARKET_RUINS,
    /* 0x04 */ NATURE_ID_KOKIRI_REGION,
    /* 0x05 */ NATURE_ID_MARKET_NIGHT,
    /* 0x06 */ NATURE_ID_06,
    /* 0x07 */ NATURE_ID_GANONS_LAIR,
    /* 0x08 */ NATURE_ID_08,
    /* 0x09 */ NATURE_ID_09,
    /* 0x0A */ NATURE_ID_WASTELAND,
    /* 0x0B */ NATURE_ID_COLOSSUS,
    /* 0x0C */ NATURE_ID_DEATH_MOUNTAIN_TRAIL,
    /* 0x0D */ NATURE_ID_0D,
    /* 0x0E */ NATURE_ID_0E,
    /* 0x0F */ NATURE_ID_0F,
    /* 0x10 */ NATURE_ID_10,
    /* 0x11 */ NATURE_ID_11,
    /* 0x12 */ NATURE_ID_12,
    /* 0x13 */ NATURE_ID_NONE,
    /* 0xFF */ NATURE_ID_DISABLED = 0xFF
} NatureAmbienceId;

typedef enum {
    /* 0x00 */ NATURE_STREAM_RUSHING_WATER,
    /* 0x01 */ NATURE_STREAM_HOWLING_WIND,
    /* 0x02 */ NATURE_STREAM_SCREECHING_WIND,
    /* 0x03 */ NATURE_STREAM_SCREECHING_WIND_ALT1
} NatureStreamId;

typedef enum {
    /* 0x00 */ NATURE_CRITTER_BIRD_CHIRP_1,
    /* 0x01 */ NATURE_CRITTER_TAP,
    /* 0x02 */ NATURE_CRITTER_BIRD_CHIRP_2,
    /* 0x03 */ NATURE_CRITTER_BIRD_CHIRP_1_ALT1,
    /* 0x04 */ NATURE_CRITTER_CRICKETS,
    /* 0x05 */ NATURE_CRITTER_BIRD_CHIRP_1_ALT2,
    /* 0x06 */ NATURE_CRITTER_LOUD_CHIRPING,
    /* 0x07 */ NATURE_CRITTER_BIRD_CHIRP_1_ALT3,
    /* 0x08 */ NATURE_CRITTER_BIRD_CHIRP_1_ALT4,
    /* 0x09 */ NATURE_CRITTER_CROWS_CAWS,
    /* 0x0A */ NATURE_CRITTER_SMALL_BIRD_CHIRPS,
    /* 0x0B */ NATURE_CRITTER_BIRD_SCREECH,
    /* 0x0C */ NATURE_CRITTER_BIRD_SONG,
    /* 0x0D */ NATURE_CRITTER_OWL_HOOT,
    /* 0x0E */ NATURE_CRITTER_HAWK_SCREECH,
    /* 0x0F */ NATURE_CRITTER_BIRD_CALL,
    /* 0x10 */ NATURE_CRITTER_CAWING_BIRD,
    /* 0x11 */ NATURE_CRITTER_CUCCO_CROWS,
    /* 0x12 */ NATURE_CRITTER_BIRD_CHIRP_2_ALT1,
    /* 0x13 */ NATURE_CRITTER_BIRD_CHIRP_1_ALT5
} NatureAmimalId;

#define NATURE_IO_CRITTER_0_TYPE(type)        NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_0_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_0_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_0_PORT5(reverb)     NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_1_TYPE(type)        NATURE_CHANNEL_CRITTER_1, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_1_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_1, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_1_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_1, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_1_PORT5(reverb)     NATURE_CHANNEL_CRITTER_1, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_2_TYPE(type)        NATURE_CHANNEL_CRITTER_2, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_2_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_2, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_2_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_2, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_2_PORT5(reverb)     NATURE_CHANNEL_CRITTER_2, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_3_TYPE(type)        NATURE_CHANNEL_CRITTER_3, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_3_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_3, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_3_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_3, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_3_PORT5(reverb)     NATURE_CHANNEL_CRITTER_3, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_4_TYPE(type)        NATURE_CHANNEL_CRITTER_4, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_4_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_4, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_4_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_4, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_4_PORT5(reverb)     NATURE_CHANNEL_CRITTER_4, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_5_TYPE(type)        NATURE_CHANNEL_CRITTER_5, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_5_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_5, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_5_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_5, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_5_PORT5(reverb)     NATURE_CHANNEL_CRITTER_5, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_CRITTER_6_TYPE(type)        NATURE_CHANNEL_CRITTER_6, CHANNEL_IO_PORT_2, type
#define NATURE_IO_CRITTER_6_BEND_PITCH(bend)  NATURE_CHANNEL_CRITTER_6, CHANNEL_IO_PORT_3, bend
#define NATURE_IO_CRITTER_6_NUM_LAYERS(num)   NATURE_CHANNEL_CRITTER_6, CHANNEL_IO_PORT_4, num
#define NATURE_IO_CRITTER_6_PORT5(reverb)     NATURE_CHANNEL_CRITTER_6, CHANNEL_IO_PORT_5, reverb

#define NATURE_IO_STREAM_0_TYPE(type)         NATURE_CHANNEL_STREAM_0, CHANNEL_IO_PORT_2, type
#define NATURE_IO_STREAM_0_PORT3(data)        NATURE_CHANNEL_STREAM_0, CHANNEL_IO_PORT_3, data
#define NATURE_IO_STREAM_0_PORT4(data)        NATURE_CHANNEL_STREAM_0, CHANNEL_IO_PORT_4, data

#define NATURE_IO_STREAM_1_TYPE(type)         NATURE_CHANNEL_STREAM_1, CHANNEL_IO_PORT_2, type
#define NATURE_IO_STREAM_1_PORT3(data)        NATURE_CHANNEL_STREAM_1, CHANNEL_IO_PORT_3, data
#define NATURE_IO_STREAM_1_PORT4(data)        NATURE_CHANNEL_STREAM_1, CHANNEL_IO_PORT_4, data

#define NATURE_IO_ENTRIES_END 0xFF

// The same layout as MM's AmbienceDataIO.
typedef struct {
    /* 0x0 */ u16 playerIO;
    /* 0x2 */ u16 channelMask;
    /* 0x4 */ u8 channelIO[3 * 33 + 1];
} NatureAmbienceDataIO; // size = 0x68

static NatureAmbienceDataIO sNatureAmbienceDataIO[20] = {
    // NATURE_ID_GENERAL_NIGHT
    {
        0xC0FF, // PlayerIO Data
        0xC0FE, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CROWS_CAWS),
            NATURE_IO_CRITTER_0_BEND_PITCH(64),
            NATURE_IO_CRITTER_0_NUM_LAYERS(0),
            NATURE_IO_CRITTER_0_PORT5(32),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_1_BEND_PITCH(0),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(16),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_SMALL_BIRD_CHIRPS),
            NATURE_IO_CRITTER_2_BEND_PITCH(112),
            NATURE_IO_CRITTER_2_NUM_LAYERS(1),
            NATURE_IO_CRITTER_2_PORT5(48),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_HAWK_SCREECH),
            NATURE_IO_CRITTER_3_BEND_PITCH(127),
            NATURE_IO_CRITTER_3_NUM_LAYERS(0),
            NATURE_IO_CRITTER_3_PORT5(16),

            // Channel 5
            NATURE_IO_CRITTER_4_TYPE(NATURE_CRITTER_BIRD_CHIRP_1),
            NATURE_IO_CRITTER_4_BEND_PITCH(127),
            NATURE_IO_CRITTER_4_NUM_LAYERS(1),
            NATURE_IO_CRITTER_4_PORT5(16),

            // Channel 6
            NATURE_IO_CRITTER_5_TYPE(NATURE_CRITTER_TAP),
            NATURE_IO_CRITTER_5_BEND_PITCH(127),
            NATURE_IO_CRITTER_5_NUM_LAYERS(3),
            NATURE_IO_CRITTER_5_PORT5(16),

            // Channel 7
            NATURE_IO_CRITTER_6_TYPE(NATURE_CRITTER_CUCCO_CROWS),
            NATURE_IO_CRITTER_6_BEND_PITCH(127),
            NATURE_IO_CRITTER_6_NUM_LAYERS(1),
            NATURE_IO_CRITTER_6_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_MARKET_ENTRANCE
    {
        0xC0FB, // PlayerIO Data
        0xC0FA, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_2_BEND_PITCH(112),
            NATURE_IO_CRITTER_2_NUM_LAYERS(1),
            NATURE_IO_CRITTER_2_PORT5(48),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_HAWK_SCREECH),
            NATURE_IO_CRITTER_3_BEND_PITCH(127),
            NATURE_IO_CRITTER_3_NUM_LAYERS(0),
            NATURE_IO_CRITTER_3_PORT5(16),

            // Channel 5
            NATURE_IO_CRITTER_4_TYPE(NATURE_CRITTER_BIRD_CHIRP_1),
            NATURE_IO_CRITTER_4_BEND_PITCH(127),
            NATURE_IO_CRITTER_4_NUM_LAYERS(1),
            NATURE_IO_CRITTER_4_PORT5(16),

            // Channel 6
            NATURE_IO_CRITTER_5_TYPE(NATURE_CRITTER_TAP),
            NATURE_IO_CRITTER_5_BEND_PITCH(127),
            NATURE_IO_CRITTER_5_NUM_LAYERS(3),
            NATURE_IO_CRITTER_5_PORT5(16),

            // Channel 7
            NATURE_IO_CRITTER_6_TYPE(NATURE_CRITTER_CUCCO_CROWS),
            NATURE_IO_CRITTER_6_BEND_PITCH(127),
            NATURE_IO_CRITTER_6_NUM_LAYERS(1),
            NATURE_IO_CRITTER_6_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_KAKARIKO_REGION
    {
        0xC001, // PlayerIO Data
        0x4000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_1_BEND_PITCH(48),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(32),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_MARKET_RUINS
    {
        0xC005, // PlayerIO Data
        0x4000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_HOWLING_WIND),
            NATURE_IO_STREAM_0_PORT3(32),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_1_BEND_PITCH(48),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(32),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_KOKIRI_REGION
    {
        0xC01F, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(47),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_OWL_HOOT),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_CAWING_BIRD),
            NATURE_IO_CRITTER_1_BEND_PITCH(0),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(32),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_HAWK_SCREECH),
            NATURE_IO_CRITTER_2_BEND_PITCH(0),
            NATURE_IO_CRITTER_2_NUM_LAYERS(0),
            NATURE_IO_CRITTER_2_PORT5(44),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_3_BEND_PITCH(63),
            NATURE_IO_CRITTER_3_NUM_LAYERS(1),
            NATURE_IO_CRITTER_3_PORT5(44),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_MARKET_NIGHT
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_06
    {
        0xC0FB, // PlayerIO Data
        0xC0FA, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_2_BEND_PITCH(112),
            NATURE_IO_CRITTER_2_NUM_LAYERS(1),
            NATURE_IO_CRITTER_2_PORT5(48),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_HAWK_SCREECH),
            NATURE_IO_CRITTER_3_BEND_PITCH(127),
            NATURE_IO_CRITTER_3_NUM_LAYERS(0),
            NATURE_IO_CRITTER_3_PORT5(16),

            // Channel 5
            NATURE_IO_CRITTER_4_TYPE(NATURE_CRITTER_BIRD_CHIRP_1),
            NATURE_IO_CRITTER_4_BEND_PITCH(127),
            NATURE_IO_CRITTER_4_NUM_LAYERS(1),
            NATURE_IO_CRITTER_4_PORT5(16),

            // Channel 6
            NATURE_IO_CRITTER_5_TYPE(NATURE_CRITTER_TAP),
            NATURE_IO_CRITTER_5_BEND_PITCH(127),
            NATURE_IO_CRITTER_5_NUM_LAYERS(3),
            NATURE_IO_CRITTER_5_PORT5(16),

            // Channel 7
            NATURE_IO_CRITTER_6_TYPE(NATURE_CRITTER_CUCCO_CROWS),
            NATURE_IO_CRITTER_6_BEND_PITCH(127),
            NATURE_IO_CRITTER_6_NUM_LAYERS(1),
            NATURE_IO_CRITTER_6_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_GANONS_LAIR
    {
        0x8001, // PlayerIO Data
        0x0,    // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_HOWLING_WIND),
            NATURE_IO_STREAM_0_PORT3(32),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_08
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_09
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_WASTELAND
    {
        0xC001, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_SCREECHING_WIND),
            NATURE_IO_STREAM_0_PORT3(0),
            NATURE_IO_STREAM_0_PORT4(0),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_COLOSSUS
    {
        0xC02F, // PlayerIO Data
        0xC02E, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_SCREECHING_WIND),
            NATURE_IO_STREAM_0_PORT3(0),
            NATURE_IO_STREAM_0_PORT4(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_SMALL_BIRD_CHIRPS),
            NATURE_IO_CRITTER_0_BEND_PITCH(64),
            NATURE_IO_CRITTER_0_NUM_LAYERS(0),
            NATURE_IO_CRITTER_0_PORT5(32),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_BIRD_CALL),
            NATURE_IO_CRITTER_1_BEND_PITCH(112),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(48),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_HAWK_SCREECH),
            NATURE_IO_CRITTER_2_BEND_PITCH(127),
            NATURE_IO_CRITTER_2_NUM_LAYERS(0),
            NATURE_IO_CRITTER_2_PORT5(16),

            // Channel 5
            NATURE_IO_CRITTER_4_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_4_BEND_PITCH(127),
            NATURE_IO_CRITTER_4_NUM_LAYERS(0),
            NATURE_IO_CRITTER_4_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_DEATH_MOUNTAIN_TRAIL
    {
        0xC07F, // PlayerIO Data
        0xC07E, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),
            NATURE_IO_STREAM_0_PORT4(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_SMALL_BIRD_CHIRPS),
            NATURE_IO_CRITTER_0_BEND_PITCH(64),
            NATURE_IO_CRITTER_0_NUM_LAYERS(0),
            NATURE_IO_CRITTER_0_PORT5(32),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_1_BEND_PITCH(112),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(48),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_BIRD_SONG),
            NATURE_IO_CRITTER_2_BEND_PITCH(127),
            NATURE_IO_CRITTER_2_NUM_LAYERS(0),
            NATURE_IO_CRITTER_2_PORT5(16),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_LOUD_CHIRPING),
            NATURE_IO_CRITTER_3_BEND_PITCH(0),
            NATURE_IO_CRITTER_3_NUM_LAYERS(0),
            NATURE_IO_CRITTER_3_PORT5(16),

            // Channel 5
            NATURE_IO_CRITTER_4_TYPE(NATURE_CRITTER_BIRD_CHIRP_1),
            NATURE_IO_CRITTER_4_BEND_PITCH(0),
            NATURE_IO_CRITTER_4_NUM_LAYERS(0),
            NATURE_IO_CRITTER_4_PORT5(16),

            // Channel 6
            NATURE_IO_CRITTER_5_TYPE(NATURE_CRITTER_TAP),
            NATURE_IO_CRITTER_5_BEND_PITCH(0),
            NATURE_IO_CRITTER_5_NUM_LAYERS(0),
            NATURE_IO_CRITTER_5_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_0D
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_0E
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_0F
    {
        0xC01F, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_BIRD_CHIRP_1),
            NATURE_IO_CRITTER_0_BEND_PITCH(80),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(8),

            // Channel 2
            NATURE_IO_CRITTER_1_TYPE(NATURE_CRITTER_SMALL_BIRD_CHIRPS),
            NATURE_IO_CRITTER_1_BEND_PITCH(80),
            NATURE_IO_CRITTER_1_NUM_LAYERS(1),
            NATURE_IO_CRITTER_1_PORT5(48),

            // Channel 3
            NATURE_IO_CRITTER_2_TYPE(NATURE_CRITTER_LOUD_CHIRPING),
            NATURE_IO_CRITTER_2_BEND_PITCH(0),
            NATURE_IO_CRITTER_2_NUM_LAYERS(0),
            NATURE_IO_CRITTER_2_PORT5(0),

            // Channel 4
            NATURE_IO_CRITTER_3_TYPE(NATURE_CRITTER_BIRD_SCREECH),
            NATURE_IO_CRITTER_3_BEND_PITCH(96),
            NATURE_IO_CRITTER_3_NUM_LAYERS(0),
            NATURE_IO_CRITTER_3_PORT5(32),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_10
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_11
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_12
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },

    // NATURE_ID_NONE
    // While there is data for this natureAmbienceId, it is identical to previous entries
    // and the game treats it as no nature ambience
    {
        0xC003, // PlayerIO Data
        0xC000, // Channel Mask
        {
            // Channel 0
            NATURE_IO_STREAM_0_TYPE(NATURE_STREAM_RUSHING_WATER),
            NATURE_IO_STREAM_0_PORT3(0),

            // Channel 1
            NATURE_IO_CRITTER_0_TYPE(NATURE_CRITTER_CRICKETS),
            NATURE_IO_CRITTER_0_BEND_PITCH(0),
            NATURE_IO_CRITTER_0_NUM_LAYERS(1),
            NATURE_IO_CRITTER_0_PORT5(16),

            // End
            NATURE_IO_ENTRIES_END,
        },
    },
};

const void* Hyrule_GetNatureAmbienceData(u8 ootAmbienceId) {
    return (ootAmbienceId < ARRAY_COUNT(sNatureAmbienceDataIO)) ? &sNatureAmbienceDataIO[ootAmbienceId]
                                                                : &sNatureAmbienceDataIO[NATURE_ID_GENERAL_NIGHT];
}

// OoT's func_800F3F3C: the cutscene effects sequence (OoT's NA_BGM_VARIOUS_SFX) on the sub-BGM player, its effect
// chosen by an IO port.
void Hyrule_PlayCutsceneEffectsSeqOot(u8 effect) {
    SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_SUB, 0, SEQ_OOT(0x6D));
    SEQCMD_SET_SEQPLAYER_IO(SEQ_PLAYER_BGM_SUB, 0, effect);
}
