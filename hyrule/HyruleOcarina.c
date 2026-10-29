// OoT's songs on MM's ocarina (docs/SONGS.md).

#include "global.h"
#include "HyruleOcarina.h"
#include "HyruleInventory.h"
#include "HyruleScenes.h"
#include "HyruleRestrictions.h"

s32 Hyrule_IsSongLearned(u8 song) {
    switch (song) {
        case OCARINA_SONG_ZELDAS_LULLABY:
            return Hyrule_CheckQuestItemOot(QUEST_SONG_LULLABY_Oot);
        case OCARINA_SONG_DOUBLE_TIME:
            return Hyrule_CheckQuestItemOot(QUEST_SONG_TIME_Oot);
        default:
            if ((song >= OCARINA_SONG_MINUET) && (song <= OCARINA_SONG_PRELUDE)) {
                return Hyrule_CheckQuestItemOot(QUEST_SONG_MINUET_Oot + song - OCARINA_SONG_MINUET);
            }
            // MM's songs, in the order of MM's quest bits from the Sonata to the Sun's Song.
            return CHECK_QUEST_ITEM(QUEST_SONG_SONATA + song);
    }
}

u32 Hyrule_GetLearnedSongBitsOot(void) {
    static const u8 sSongs[] = {
        OCARINA_SONG_ZELDAS_LULLABY, OCARINA_SONG_DOUBLE_TIME, OCARINA_SONG_MINUET,   OCARINA_SONG_BOLERO,
        OCARINA_SONG_SERENADE,       OCARINA_SONG_REQUIEM,     OCARINA_SONG_NOCTURNE, OCARINA_SONG_PRELUDE,
    };
    u32 bits = 0;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sSongs); i++) {
        if (Hyrule_IsSongLearned(sSongs[i])) {
            bits |= 1 << sSongs[i];
        }
    }
    return bits;
}

// OoT's song id (OoT's order, from the Minuet to the scarecrow's song) for one of MM's, or -1.
static s32 Hyrule_SongToOot(u8 song) {
    static const u8 sOotSongs[] = {
        OCARINA_SONG_MINUET,         OCARINA_SONG_BOLERO, OCARINA_SONG_SERENADE,   OCARINA_SONG_REQUIEM,
        OCARINA_SONG_NOCTURNE,       OCARINA_SONG_PRELUDE, OCARINA_SONG_SARIAS,    OCARINA_SONG_EPONAS,
        OCARINA_SONG_ZELDAS_LULLABY, OCARINA_SONG_SUNS,   OCARINA_SONG_DOUBLE_TIME, OCARINA_SONG_STORMS,
        OCARINA_SONG_SCARECROW_SPAWN,
    };
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sOotSongs); i++) {
        if (sOotSongs[i] == song) {
            return i;
        }
    }
    return -1;
}

// OoT's own modes that MM has no equivalent for are kept from this value up, clear of MM's.
#define OCARINA_MODE_OOT_ONLY 0x40

u8 Hyrule_GetOcarinaModeOot(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    u8 mode = msgCtx->ocarinaMode;
    s32 ootSong;

    switch (mode) {
        case OCARINA_MODE_NONE:
        case OCARINA_MODE_ACTIVE:
        case OCARINA_MODE_WARP:
        case OCARINA_MODE_EVENT:
        case OCARINA_MODE_END:
            return mode;
        case OCARINA_MODE_PLAYED_SCARECROW_SPAWN:
            return OCARINA_MODE_0B_Oot;
        case OCARINA_MODE_11:
            return OCARINA_MODE_0F_Oot;
        default:
            break;
    }
    if (mode >= OCARINA_MODE_OOT_ONLY) {
        return mode - OCARINA_MODE_OOT_ONLY;
    }
    // A wrong song for a song check: MM's "song - 1", as OoT's in OoT's ids.
    if ((msgCtx->ocarinaAction >= OCARINA_ACTION_CHECK_SONATA) && (msgCtx->lastPlayedSong != 0xFF) &&
        (mode == msgCtx->lastPlayedSong - 1)) {
        ootSong = Hyrule_SongToOot(msgCtx->lastPlayedSong);
        return (ootSong > 0) ? ootSong - 1 : OCARINA_MODE_04_Oot;
    }
    // MM's song modes (Epona's Song, the Song of Storms...) end the song; its time songs are still at work.
    return (mode <= OCARINA_MODE_PLAYED_SCARECROW_SPAWN) ? OCARINA_MODE_04_Oot : OCARINA_MODE_01_Oot;
}

void Hyrule_SetOcarinaModeOot(PlayState* play, u8 ootMode) {
    switch (ootMode) {
        case OCARINA_MODE_00_Oot:
        case OCARINA_MODE_01_Oot:
        case OCARINA_MODE_02_Oot:
        case OCARINA_MODE_03_Oot:
        case OCARINA_MODE_04_Oot:
            play->msgCtx.ocarinaMode = ootMode;
            break;
        case OCARINA_MODE_0B_Oot:
            play->msgCtx.ocarinaMode = OCARINA_MODE_PLAYED_SCARECROW_SPAWN;
            break;
        case OCARINA_MODE_0F_Oot:
            play->msgCtx.ocarinaMode = OCARINA_MODE_11;
            break;
        default:
            play->msgCtx.ocarinaMode = OCARINA_MODE_OOT_ONLY + ootMode;
            break;
    }
}

u8 gWarpSongsDisabledOot;

void Message_StartOcarinaOot(PlayState* play, u16 ocarinaAction) {
    gWarpSongsDisabledOot = false;
    Message_DisplayOcarinaStaff(play, ocarinaAction);
}

void Message_StartOcarinaNoWarpOot(PlayState* play, u16 ocarinaAction) {
    gWarpSongsDisabledOot = true;
    Message_DisplayOcarinaStaff(play, ocarinaAction);
}

u16 Hyrule_OcarinaActionFromOot(u16 ootAction) {
    // OoT's lessons of Zelda's Lullaby and the warp songs, which MM's lesson ranges cannot index: OoT's teach actions
    // (0x02-0x07 the warp songs, 0x0A Zelda's Lullaby) show OoT's staff text (0x86E + the action) and its playback
    // actions (0x0F-0x14, 0x17) prompt for the song, through Message_SetLessonOot.
    if (((ootAction >= 0x02) && (ootAction <= 0x07)) || (ootAction == 0x0A)) {
        Message_SetLessonOot((ootAction == 0x0A) ? OCARINA_SONG_ZELDAS_LULLABY : OCARINA_SONG_MINUET + ootAction - 0x02,
                             TEXT_OOT(0x86E + ootAction));
        return OCARINA_ACTION_11;
    }
    if (((ootAction >= 0x0F) && (ootAction <= 0x14)) || (ootAction == 0x17)) {
        Message_SetLessonOot((ootAction == 0x17) ? OCARINA_SONG_ZELDAS_LULLABY : OCARINA_SONG_MINUET + ootAction - 0x0F,
                             0);
        return OCARINA_ACTION_21;
    }
    switch (ootAction) {
        case 0x01:
            return OCARINA_ACTION_FREE_PLAY_Oot;
        case 0x08:
            return OCARINA_ACTION_TEACH_SARIA_Oot;
        case 0x09:
            return OCARINA_ACTION_TEACH_EPONA_Oot;
        case 0x0B:
            return OCARINA_ACTION_TEACH_SUNS_Oot;
        case 0x0C:
            return OCARINA_ACTION_TEACH_TIME_Oot;
        case 0x0D:
            return OCARINA_ACTION_TEACH_STORMS_Oot;
        case 0x15:
            return OCARINA_ACTION_PLAYBACK_SARIA_Oot;
        case 0x16:
            return OCARINA_ACTION_PLAYBACK_EPONA_Oot;
        case 0x18:
            return OCARINA_ACTION_PLAYBACK_SUNS_Oot;
        case 0x19:
            return OCARINA_ACTION_PLAYBACK_TIME_Oot;
        case 0x1A:
            return OCARINA_ACTION_PLAYBACK_STORMS_Oot;
        case 0x22:
            return OCARINA_ACTION_CHECK_SARIA_Oot;
        case 0x23:
            return OCARINA_ACTION_CHECK_EPONA_Oot;
        case 0x25:
            return OCARINA_ACTION_CHECK_SUNS_Oot;
        case 0x26:
            return OCARINA_ACTION_CHECK_TIME_Oot;
        case 0x27:
            return OCARINA_ACTION_CHECK_STORMS_Oot;
        case 0x28:
            return OCARINA_ACTION_CHECK_SCARECROW_Oot;
        case 0x29:
            return OCARINA_ACTION_FREE_PLAY_DONE_Oot;
        case 0x2A:
            return OCARINA_ACTION_SCARECROW_LONG_RECORDING_Oot;
        case 0x2B:
            return OCARINA_ACTION_SCARECROW_LONG_PLAYBACK_Oot;
        case 0x2C:
            return OCARINA_ACTION_SCARECROW_RECORDING_Oot;
        case 0x2D:
            return OCARINA_ACTION_SCARECROW_PLAYBACK_Oot;
        default:
            return OCARINA_ACTION_FREE_PLAY;
    }
}

// OoT's sWarpSongEntrances: the warp pads of the Sacred Forest Meadow, Death Mountain Crater, Lake Hylia, the Desert
// Colossus, the Graveyard and the Temple of Time.
static const u16 sWarpSongEntrancesOot[] = { 0x600, 0x4F6, 0x604, 0x1F1, 0x568, 0x5F4 };

s32 Hyrule_AreWarpSongsAllowedOot(PlayState* play) {
    if (gWarpSongsDisabledOot) {
        return false;
    }
    if (Hyrule_GetOotSceneId(play->sceneId) < 0) {
        return play->interfaceCtx.restrictions.songOfSoaring == 0;
    }
    // OoT refuses only at 3: Gerudo Training Ground, the windmill and Dampe's grave, and inside Ganon's Castle.
    return Hyrule_GetRestrictionOot(play, HYRULE_RESTRICTION_WARP_SONGS_OOT) != 3;
}

s32 Hyrule_IsWarpSongPadLoadedOot(u8 song) {
    return Hyrule_EntranceFromOot(sWarpSongEntrancesOot[song - OCARINA_SONG_MINUET]) != HYRULE_ENTRANCE_NONE;
}

static u32 sWarpSongEntranceOot;
static u8 sWarpSongArrivalOot;

void Hyrule_SetWarpSongDestinationOot(u8 song) {
    sWarpSongEntranceOot = Hyrule_EntranceFromOot(sWarpSongEntrancesOot[song - OCARINA_SONG_MINUET]);
    // The warp light's sparkle colors, by song (OoT's respawn[RESPAWN_MODE_RETURN].data).
    gSaveContext.respawn[RESPAWN_MODE_RETURN].data = song - OCARINA_SONG_MINUET;
}

// OoT's Environment_WarpSongLeave, into the pad's entrance. MM kept OoT's, but its respawn flag (-3) now means a grotto
// return, so Link comes in through the entrance and Player's init takes the arrival from Hyrule_TakeWarpSongArrivalOot.
void Hyrule_WarpSongLeaveOot(PlayState* play) {
    gWeatherMode = WEATHER_MODE_CLEAR;
    gSaveContext.save.cutsceneIndex = 0;
    gSaveContext.respawnFlag = 0;
    play->nextEntrance = sWarpSongEntranceOot;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
    sWarpSongArrivalOot = true;
}

s32 Hyrule_TakeWarpSongArrivalOot(void) {
    s32 arrival = sWarpSongArrivalOot;

    sWarpSongArrivalOot = false;
    return arrival;
}
