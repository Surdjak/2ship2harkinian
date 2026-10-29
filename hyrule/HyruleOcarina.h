#pragma once

// OoT's songs on MM's ocarina (docs/SONGS.md).

#include "ultra64.h"

#ifdef __cplusplus
extern "C" {
#endif

// Whether Link has learned a song, by MM's song id: MM's quest bits for the songs MM teaches (Saria's and the Sun's
// Song, Epona's Song and the Song of Storms included), OoT's for Zelda's Lullaby and the warp songs, and OoT's Song of
// Time bit for the Song of Double Time, which only Zelda teaches (DESIGN.md).
s32 Hyrule_IsSongLearned(u8 song);
// The playable-songs bits (one per MM song id) of the songs MM's own learned bits leave out: Zelda's Lullaby, the warp
// songs and the Song of Double Time.
u32 Hyrule_GetLearnedSongBitsOot(void);

// OoT's song ids, as ported OoT code names them, are MM's: the warp songs are MM's songs 24 to 29, and OoT's Song of
// Time is MM's Song of Double Time, which does its work (DESIGN.md).
#define OCARINA_SONG_MINUET_Oot OCARINA_SONG_MINUET
#define OCARINA_SONG_BOLERO_Oot OCARINA_SONG_BOLERO
#define OCARINA_SONG_SERENADE_Oot OCARINA_SONG_SERENADE
#define OCARINA_SONG_REQUIEM_Oot OCARINA_SONG_REQUIEM
#define OCARINA_SONG_NOCTURNE_Oot OCARINA_SONG_NOCTURNE
#define OCARINA_SONG_PRELUDE_Oot OCARINA_SONG_PRELUDE
#define OCARINA_SONG_SARIAS_Oot OCARINA_SONG_SARIAS
#define OCARINA_SONG_EPONAS_Oot OCARINA_SONG_EPONAS
#define OCARINA_SONG_LULLABY_Oot OCARINA_SONG_ZELDAS_LULLABY
#define OCARINA_SONG_SUNS_Oot OCARINA_SONG_SUNS
#define OCARINA_SONG_TIME_Oot OCARINA_SONG_DOUBLE_TIME
#define OCARINA_SONG_STORMS_Oot OCARINA_SONG_STORMS
#define OCARINA_SONG_SCARECROW_SPAWN_Oot OCARINA_SONG_SCARECROW_SPAWN

// OoT's ocarina actions MM's staff has. Teaching, prompting or checking Zelda's Lullaby or a warp song, the memory game
// and the frogs' songs are not here yet (docs/SONGS.md, step 5), so a port that uses one does not build.
#define OCARINA_ACTION_FREE_PLAY_Oot OCARINA_ACTION_FREE_PLAY
#define OCARINA_ACTION_FREE_PLAY_DONE_Oot OCARINA_ACTION_FREE_PLAY_DONE
#define OCARINA_ACTION_TEACH_SARIA_Oot OCARINA_ACTION_DEMONSTRATE_SARIAS
#define OCARINA_ACTION_TEACH_EPONA_Oot OCARINA_ACTION_DEMONSTRATE_EPONAS
#define OCARINA_ACTION_TEACH_SUNS_Oot OCARINA_ACTION_DEMONSTRATE_SUNS
#define OCARINA_ACTION_TEACH_TIME_Oot OCARINA_ACTION_DEMONSTRATE_DOUBLE_TIME
#define OCARINA_ACTION_TEACH_STORMS_Oot OCARINA_ACTION_DEMONSTRATE_STORMS
#define OCARINA_ACTION_PLAYBACK_SARIA_Oot OCARINA_ACTION_PROMPT_SARIAS
#define OCARINA_ACTION_PLAYBACK_EPONA_Oot OCARINA_ACTION_PROMPT_EPONAS
#define OCARINA_ACTION_PLAYBACK_SUNS_Oot OCARINA_ACTION_PROMPT_SUNS
#define OCARINA_ACTION_PLAYBACK_TIME_Oot OCARINA_ACTION_PROMPT_DOUBLE_TIME
#define OCARINA_ACTION_PLAYBACK_STORMS_Oot OCARINA_ACTION_PROMPT_STORMS
#define OCARINA_ACTION_CHECK_SARIA_Oot OCARINA_ACTION_CHECK_SARIAS
#define OCARINA_ACTION_CHECK_EPONA_Oot OCARINA_ACTION_CHECK_EPONAS
#define OCARINA_ACTION_CHECK_SUNS_Oot OCARINA_ACTION_CHECK_SUNS
#define OCARINA_ACTION_CHECK_TIME_Oot OCARINA_ACTION_CHECK_DOUBLE_TIME
#define OCARINA_ACTION_CHECK_STORMS_Oot OCARINA_ACTION_CHECK_STORMS
#define OCARINA_ACTION_CHECK_SCARECROW_Oot OCARINA_ACTION_CHECK_SCARECROW_SPAWN
#define OCARINA_ACTION_SCARECROW_LONG_RECORDING_Oot OCARINA_ACTION_SCARECROW_LONG_RECORDING
#define OCARINA_ACTION_SCARECROW_LONG_PLAYBACK_Oot OCARINA_ACTION_SCARECROW_LONG_DEMONSTRATION
#define OCARINA_ACTION_SCARECROW_RECORDING_Oot OCARINA_ACTION_SCARECROW_SPAWN_RECORDING
#define OCARINA_ACTION_SCARECROW_PLAYBACK_Oot OCARINA_ACTION_SCARECROW_SPAWN_DEMONSTRATION

// OoT's ocarina modes. MM's share OoT's first five (none, playing, warp, the song asked for was played, done) but number
// the rest differently, and MM ends a free-played song with its own mode for some songs; so ported code reads and sets
// the mode through these, in OoT's values:
// - read: MM's mode as OoT's: after a free-played song, "done" (04) with lastPlayedSong set, as OoT; a wrong song for a
//   song check, OoT's "song - 1" (05 to 0A); the scarecrow's song 0B; the end of a long scarecrow recording 0F; MM's
//   time-song processing as still playing (01).
// - set: OoT's mode as MM's; OoT's actors' own modes MM has no equivalent for are kept apart from MM's.
typedef enum OcarinaModeOot {
    /* 0x00 */ OCARINA_MODE_00_Oot,
    /* 0x01 */ OCARINA_MODE_01_Oot,
    /* 0x02 */ OCARINA_MODE_02_Oot,
    /* 0x03 */ OCARINA_MODE_03_Oot,
    /* 0x04 */ OCARINA_MODE_04_Oot,
    /* 0x05 */ OCARINA_MODE_05_Oot,
    /* 0x06 */ OCARINA_MODE_06_Oot,
    /* 0x07 */ OCARINA_MODE_07_Oot,
    /* 0x08 */ OCARINA_MODE_08_Oot,
    /* 0x09 */ OCARINA_MODE_09_Oot,
    /* 0x0A */ OCARINA_MODE_0A_Oot,
    /* 0x0B */ OCARINA_MODE_0B_Oot,
    /* 0x0C */ OCARINA_MODE_0C_Oot,
    /* 0x0D */ OCARINA_MODE_0D_Oot,
    /* 0x0E */ OCARINA_MODE_0E_Oot,
    /* 0x0F */ OCARINA_MODE_0F_Oot
} OcarinaModeOot;

struct PlayState;
u8 Hyrule_GetOcarinaModeOot(struct PlayState* play);
void Hyrule_SetOcarinaModeOot(struct PlayState* play, u8 ootMode);

// The song and OoT staff text of an OoT lesson that MM's lesson ranges cannot index (z_message.c).
void Message_SetLessonOot(u8 song, u16 textId);

// OoT's func_8010BD58 and func_8010BD88: bring up the ocarina staff for an action; the second also refuses warp songs
// (OoT's unk_E40E), which the warp songs will read (docs/SONGS.md, step 4).
void Message_StartOcarinaOot(struct PlayState* play, u16 ocarinaAction);
void Message_StartOcarinaNoWarpOot(struct PlayState* play, u16 ocarinaAction);
extern u8 gWarpSongsDisabledOot;
// One of OoT's ocarina action ids (OoT's OcarinaSongActionId, as OoT's cutscene scripts hold them) as MM's; the
// actions MM's staff does not have yet (docs/SONGS.md, step 5) bring up the free-play staff.
u16 Hyrule_OcarinaActionFromOot(u16 ootAction);

// The warp songs (docs/SONGS.md, W1): played where OoT allows them in Hyrule (all but Gerudo Training Ground, the
// windmill and Dampe's grave, and inside Ganon's Castle) and where MM allows the Song of Soaring in Termina, unless the
// actor that brought up the staff refused them (Message_StartOcarinaNoWarpOot).
s32 Hyrule_AreWarpSongsAllowedOot(struct PlayState* play);
// Whether a warp song's pad (OoT's six warp pads) is in a scene loaded so far; until then its song does nothing.
s32 Hyrule_IsWarpSongPadLoadedOot(u8 song);
// Link leaves by a warp song (OoT's Player, after "yes"): the pad's entrance and the song, for the warp light
// (Demo_Kankyo_Oot), which ends with Hyrule_WarpSongLeaveOot.
void Hyrule_SetWarpSongDestinationOot(u8 song);
void Hyrule_WarpSongLeaveOot(struct PlayState* play);
// Whether Link arrives by a warp song (read once, by Player's init), to come in with the warp light.
s32 Hyrule_TakeWarpSongArrivalOot(void);

#ifdef __cplusplus
}
#endif
