# OoT's songs on MM's ocarina

**Status: approved 2026-10-28 with decisions W1-W4 (below); steps 1 to 4 done, step 5 in progress.** It follows [DESIGN.md](DESIGN.md) (Songs) and
[INVENTORY.md](INVENTORY.md) (song bits). Most actors left in the scenes already loaded wait on it: the
scarecrows, the frogs, the Sun's Song spots, the song teachers (Malon, the windmill man, Saria, Sheik), the
Gossip Stones' fairies, OoT's chests and signs that answer Zelda's Lullaby, the Door of Time and the blue time
blocks.

## Where we start

- Link plays MM's ocarina in both worlds. MM's song table already holds most of OoT's songs, unused or shared:
  Zelda's Lullaby (`OCARINA_SONG_ZELDAS_LULLABY`, id 21), Saria's Song, the Sun's Song, the Song of Storms,
  Epona's Song, the Song of Time and the scarecrow's song. MM lacks OoT's six warp songs: the Minuet of Forest,
  Bolero of Fire, Serenade of Water, Requiem of Spirit, Nocturne of Shadow and Prelude of Light.
- Learned songs are bits: MM's `questItems` for the shared ones (MM has bits for Saria's and the Sun's Song),
  `questItemsOot` for Zelda's Lullaby, the warp songs and the Song of Double Time (INVENTORY.md). The pause
  menu's quest page already shows all of them.
- Ported OoT actors that listen to songs hear MM's ocarina only for the songs MM plays: the Gossip Stones
  answer Epona's Song, the Song of Time and the Song of Storms; the grottos open to the Song of Storms. A sign
  that Zelda's Lullaby should repair, and Mido, who waits for Saria's Song, have `PORT:` notes.

## Proposal

1. **One song table.** OoT's six warp songs join MM's table after its last song (ids 24 to 29), with OoT's
   notes, so MM's ocarina recognises, demonstrates and prompts them like its own. The playable-songs mask
   grows from 24 bits to 30, under the two control bits MM keeps at the top. MM's long scarecrow song, which
   MM numbers just past the last song, moves with it.
2. **What Link can play** comes from the learned bits: MM's for the shared songs, `questItemsOot` for Zelda's
   Lullaby and the warp songs. The Song of Double Time gets its own bit (DESIGN.md: Zelda teaches it), so
   learning the Song of Time no longer grants it; the Inverted Song of Time stays MM's.
3. **OoT's actors hear OoT's song names.** OoT's `OCARINA_SONG_*`, `OCARINA_ACTION_*` and `OCARINA_MODE_*`
   values map to MM's (`HyruleOcarina.h`), and the port tool renames them, so a ported actor checks "the
   Minuet was played" or "teach Epona's Song" as in OoT.
4. **What each song does**, by DESIGN.md:
   - Zelda's Lullaby, Saria's Song and the warp songs act only on OoT's actors and places; elsewhere they are
     played and nothing answers, like any song nothing listens to.
   - The Song of Double Time does what OoT's Song of Time did (the Door of Time, the blue blocks); the Song
     of Time only resets the cycle; the Sun's Song skips to the next dawn or dusk in both worlds (Double
     Time's job) and keeps OoT's other uses (the Sun's Song spots, freezing ReDeads).
   - Epona's Song and the Song of Storms do what they do in either game, wherever they are played.
5. **Warp songs** take Link to OoT's six warp pads with OoT's "warp?" prompt and blue light: the Sacred Forest
   Meadow, Death Mountain Crater, Lake Hylia, the Desert Colossus, the Graveyard and the Temple of Time. Until a
   pad's scene is loaded, its song plays and does nothing. They work in both worlds: in Hyrule as in OoT, and in
   Termina wherever the Song of Soaring works (W1).
6. **Teachers** teach as in OoT: they demonstrate, Link plays it back, and the bit is set. A song taught in
   both games (Epona's Song by Malon and Romani, the Song of Storms by the windmill man and Flat) is one bit:
   the second teacher's lesson is a refresher, and their reward follows DESIGN.md's rule (filler once
   claimed). Sheik's lessons come with Sheik's cutscenes.
7. **Scarecrows.** OoT's Pierre and MM's scarecrows share one recorded song (W3).
8. **The Song of Soaring** works outdoors in Hyrule too, to the owl statues Link has activated (W2).

## Implementation

1. Done: the warp songs are MM's songs 24 to 29 (`OCARINA_SONG_MINUET`...), with OoT's notes and buttons and
   reserved from scarecrow recordings; the playable mask is 30 bits (`code_8019AF00.c`). Played, OoT's songs
   (Saria's Song, the Sun's Song, Zelda's Lullaby and the warp songs) show OoT's "You played..." texts, which MM
   lacks (`Message_GetSongPlayedTextId`), and play OoT's fanfares; Saria's Song and Zelda's Lullaby spawn OoT's
   effects, which MM kept (`Oceff_Wipe3`, `Oceff_Wipe`'s Zelda's Lullaby type), and the warp songs none, as in
   OoT. MM's fanfare table ended before Zelda's Lullaby and now covers every song. A played song gives actors
   MM's `OCARINA_MODE_EVENT` with `lastPlayedSong`, as MM's own songs do. Checked under gdb: each warp song and
   Zelda's Lullaby, once learned (`Item_Give`), plays on the free-play staff.
   Teaching a warp song (its demonstrate, prompt and check staff actions) comes with the first teacher (step 5):
   MM finds those actions by adding the song id to a base action, which runs into other actions past MM's songs.
2. Done: `Hyrule_IsSongLearned` and `Hyrule_GetLearnedSongBitsOot` (`HyruleOcarina.h`) read MM's bits and
   `questItemsOot`; no save change was needed. The Song of Time no longer brings the Song of Double Time (W4),
   which plays once Zelda's bit is set. MM's Goron Lullaby mask (`(u16)~0x4000`) clears every bit past 15, so OoT's
   bits are added after it.
3. Done: `HyruleOcarina.h` names OoT's songs and staff actions as MM's (OoT's Song of Time is MM's Song of
   Double Time), and ported code reads and sets the ocarina mode in OoT's values (`Hyrule_GetOcarinaModeOot`,
   `Hyrule_SetOcarinaModeOot`): MM shares OoT's first five modes but numbers the rest differently, and ends a
   free-played song as OoT does only once its text is closed. OoT's `func_8010BD58` and `func_8010BD88` (the
   latter refusing warp songs) are `Message_StartOcarinaOot` and `Message_StartOcarinaNoWarpOot`. The port tool
   renames all of these. MM's staff checks for the songs before the Song of Time (Sonata to Saria's Song) were
   broken and unused in MM; they now work, as Mido's check for Saria's Song needs. Checked under gdb: a Kokiri
   Forest Gossip Stone gives a fairy for Zelda's Lullaby, a cut-down sign mends itself to it, and adult Link's
   Saria's Song lets Mido step aside in the Lost Woods (OoT's Player plays for an actor that asks; MM's offers
   the ocarina to an actor that offers itself, `Actor_OfferOcarinaInteraction`, which the port does by hand).
4. Done, with OoT's warp light, which is a cutscene ([CUTSCENES.md](CUTSCENES.md) step 1); it was chosen over
   MM's Song of Soaring effect or a plain fade. A warp song played on the free-play staff (or for an actor's
   check) asks OoT's "Warp to...?" once its "You played..." text closes, as OoT's played-song mode does; MM's own
   yes/no for an open ocarina makes the mode `OCARINA_MODE_WARP` or `END`. On "yes", Player puts Link in a wait
   and spawns `Demo_Kankyo`'s warp-out, which leaves for the pad (`Hyrule_WarpSongLeaveOot`, through
   `Hyrule_EntranceFromOot`); on arrival Player starts in MM's warp-song mode, which spawns the warp-in. A song
   whose pad's scene is not loaded plays and does nothing. Where warping is refused, OoT's "You can't warp here!"
   ends the song. The rule built: in Hyrule, OoT's own (refused only in Gerudo Training Ground, the windmill and
   Dampe's grave, and inside Ganon's Castle, or by an actor that asks, `Message_StartOcarinaNoWarpOot`); in
   Termina, wherever MM allows the Song of Soaring (`restrictions.songOfSoaring`), which includes indoors.
   Dungeons allow the warp songs, as in OoT; boss rooms, minigames, some shops and Ganon's Castle's collapse
   forbid the ocarina itself, as OoT's scenes do: OoT's restrictions are `HyruleRestrictions.c` (OoT's
   `sRestrictionFlags`, which Farore's Wind, Din's Fire and Nayru's Love also read), and MM's interface greys the
   ocarina's button there. Checked under gdb: the Minuet and the Serenade warp from Kokiri Forest and South Clock
   Town, "No" ends the song with Link free, the Astral Observatory refuses, and Kokiri Forest given a boss room's
   restrictions greys the ocarina, and the Minuet's inputs play nothing. Other songs keep MM's played-song effects
   (`Oceff_*`).
5. The actor waves that need songs: Shot_Sun and the frogs (Zora's River), the scarecrows (Lake Hylia), then
   the teachers as their scenes come. Lessons: done for the songs past MM's lesson ranges. MM finds a lesson's song
   by its offset in its demonstrate and prompt ranges, which end before Zelda's Lullaby and the warp songs, so
   OoT's teach and playback actions for those (`Hyrule_OcarinaActionFromOot`) take the ranges' unused last slots
   (`OCARINA_ACTION_11`, `OCARINA_ACTION_21`) with the song and OoT's staff text kept aside
   (`Message_SetLessonOot`); a correct playback gives OoT's song, as OoT's does. Sheik's Minuet is the first
   teacher (docs/CUTSCENES.md step 3); their check actions are not ported yet.

## Decisions

| | Question | Decision |
|-|----------|----------|
| W1 | Where the warp songs work | in Hyrule exactly as in OoT (refused only in Gerudo Training Ground, the windmill and Dampe's grave, and inside Ganon's Castle; no ocarina where OoT has none); in Termina wherever the Song of Soaring works |
| W2 | The Song of Soaring in Hyrule | it works there too, to the owl statues Link has activated |
| W3 | The scarecrow's song | one song, recorded once, for OoT's Pierre and MM's scarecrows |
| W4 | Double Time for players who learned the Song of Time | DESIGN.md as written: only Zelda teaches it |
