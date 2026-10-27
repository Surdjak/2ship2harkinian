# hyrule/

Ocarina of Time code ported into the 2Ship engine. See [docs/PLAN.md](../docs/PLAN.md).

Every `.c`, `.cpp`, `.h` and `.hpp` file under this directory is compiled into the `2ship` target
(`2s2h.elf`) with the same include paths, definitions and flags as `mm/`. This directory is also on
the include path, so headers here are included relative to `hyrule/`.

Re-run the CMake configure step after adding or removing files, because sources are gathered with
`file(GLOB_RECURSE)`.

## OoT assets

`assets/` holds the ZAPD inputs for extracting OoT assets. They are copied from SoH commit
`5a57a0cb`, the last one before SoH replaced ZAPD with Torch:

- `assets/extractor/` has the config, file list, symbol lists and texture pool.
- `assets/xml/N64_NTSC_11/` has XMLs for the scenes in the table below, the objects Kokiri Forest's rooms
  load, `gameplay_keep`, `gameplay_field_keep`, `link_animetion`, the audio and the overlays of ported
  actors.

Only the N64 NTSC 1.1 ROM is supported so far. To extract:

```bash
cmake -S . -B build-cmake -DOOT_ROM_PATH=/path/to/oot-ntsc-1.1.z64
cmake --build build-cmake --target ExtractOotAssets
```

This builds a second, OoT-mode copy of ZAPD (`ZAPD_Oot`) and writes `build-cmake/oot/oot.o2r`. Every
path in that archive, and every resource reference inside it, starts with `oot/`, so it can be
loaded beside `mm.o2r` without collisions.

## OoT to MM conversion

Extraction passes `--convertToMm`, so the archive holds MM formats that 2Ship's importers read. The
converters live in `OTRExporter/OTRExporter/OotToMm.cpp` and are skeletons:

- **Scenes:** room behavior, misc settings (0x19), the cutscene list (0x17 becomes 2Ship's 0x1F) and
  paths are rewritten in MM's layout. Other commands already match. MM's extra commands 0x1A-0x1E are
  not emitted. Actor and object ids are mapped to the unified tables; behavior types, camera type and
  world map area are not remapped.
- **Text:** OoT control codes become MM ones, item icons and text-ID jumps move into MM's message
  header, and codes without an MM equivalent are dropped and reported in the extraction log. Textbox
  types, sound effects and item IDs are not remapped.
- **Cutscenes:** stub. Each becomes an empty MM cutscene that only runs to its end frame, and the
  skipped OoT commands are reported in the extraction log.

## OoT scenes in game

`ExtractOotAssets` copies `oot.o2r` next to the game, and `HyruleScenes.cpp` loads it. OoT scenes follow
MM's in the scene table, suffixed `_Oot`. `tools/porting/gen_oot_scenes.py` generates them from SoH's
tables: the scene table entries (`mm/include/tables/scene_table_oot.h`), their entrance scenes
(`HyruleEntranceScenes.h`), their entrance tables (`HyruleEntranceTable.inc`) and the map from OoT
entrance indices to MM entrances (`HyruleOotEntrances.inc`). To add a scene, append it to the script's
`SCENES`, add its XML under `assets/xml` and rerun the script and `ExtractOotAssets`.

| Scene | Scene id | Entrance (spawn 0) |
|-------|----------|--------------------|
| Kokiri Forest | `SCENE_KOKIRI_FOREST_Oot` (0x71) | `0xDC00` |
| Link's House | `SCENE_LINKS_HOUSE_Oot` (0x72) | `0xDE00` |
| Kokiri Shop | `SCENE_KOKIRI_SHOP_Oot` (0x73) | `0xE000` |
| Know-It-All Brothers' House | `SCENE_KNOW_IT_ALL_BROS_HOUSE_Oot` (0x74) | `0xE200` |
| Twins' House | `SCENE_TWINS_HOUSE_Oot` (0x75) | `0xE400` |
| Mido's House | `SCENE_MIDOS_HOUSE_Oot` (0x76) | `0xE600` |
| Saria's House | `SCENE_SARIAS_HOUSE_Oot` (0x77) | `0xE800` |
| Lost Woods | `SCENE_LOST_WOODS_Oot` (0x78) | `0xEA00` |

To go there:

- start the game with `HYRULE_BOOT_ENTRANCE=<entrance>`, which skips the title screen and starts a debug
  save there as human Link (add `HYRULE_BOOT_TIME=<hhmm>` to choose the time of day);
- in the console, run `entrance <entrance>`;
- or pick the scene (for example "Kokiri Forest (OoT)") in Better Map Select.

What works: the scenes and their rooms load, render and collide; MM's Link spawns at OoT's spawn points
and walks between rooms through the room-loading planes (`En_Holl`, mapped to MM's). Collision camera
settings are mapped to MM's (`OotToMm::CameraSetting`), and a spawn's camera index refers to the
collision camera list as it does in OoT. Room actor lists spawn the ported OoT actors (see below).

### Entrances and exits

OoT groups its entrances by scene layer: child day, child night, adult day, adult night, then cutscenes.
Each group becomes one MM entrance of its scene, whose spawn bits are the group's OoT spawn (unique in
every OoT scene) and whose layers are OoT's. So `ENTRANCE(KOKIRI_FOREST_Oot, 3)` (0xDC30) is Kokiri
Forest's spawn 3, and an OoT entrance index past the start of its group is that layer of the group's
entrance. The flags keep OoT's layout, which is MM's; OoT's circle transitions do not exist in MM and
become fades of the same color and speed.

- **Exits:** OoT scenes' exit lists hold OoT entrance indices; `Scene_CommandExitList` turns them into
  MM entrances (`Hyrule_ExitFromOot`). An exit into a scene that is not loaded yet leads back to the
  spawn Link came in at.
- **Scene layers:** for OoT entrances, `Play_Init` picks the layer as OoT does
  (`Hyrule_GetSceneLayer`): child or adult (`LINK_IS_ADULT_OOT`), day or night (OoT's day is 6:30 to
  18:00, MM's `isNight`), and cutscenes from layer 4; then OoT's two exceptions
  (`Hyrule_GetSceneLayerException`): adult Kokiri Forest's layer 3 only after the Forest Temple, and child
  Hyrule Field's layer 1 with the three spiritual stones. The exit fade and the transition setup use the layer
  before the exceptions, as OoT does, and a missing adult night layer falls back to adult day.
- **Skyboxes:** OoT's normal sky and cutscene map use MM's, and the prerendered rooms' panoramas are
  drawn by `HyrulePanorama.c` (below). OoT's bazaar, market and overcast skies are not ported and show
  nothing (`Hyrule_SkyboxFromOot`).

### Prerendered rooms

The houses and the shop are OoT's prerendered rooms. Each has two scene cameras, which OoT calls
viewpoints:

- **Fixed (viewpoint 1):** `CAM_SET_PREREND_FIXED`. A JPEG background is drawn behind the room's depth
  geometry, from the camera it was rendered with.
- **Pivot (viewpoint 2):** `CAM_SET_PREREND_PIVOT`. The camera stays at one point and turns to follow
  Link, and a prerendered panorama of two to four 256x256 faces is drawn around it.

OoT's scene camera type (misc settings, 0x19) picks the viewpoint: houses (0x20) start on the pivot and
C-up toggles, while the shop (0x10) starts fixed and its shopkeeper switches. MM kept both camera
settings and the room drawing code but never used them, so 3Ship ports:

- `Camera_Fixed3` and `Camera_Unique7` (`z_camera.c`);
- the background draw under the fixed camera, decoding the JPEG on first use (`z_room.c`,
  `HyrulePrerender.cpp`, as SoH does);
- the panoramas, built and drawn as OoT's skybox code does (`HyrulePanorama.c`; the textures are in
  `assets/xml/N64_NTSC_11/textures/backgrounds.xml`);
- the viewpoints (`Hyrule_InitViewpoint`, `Hyrule_UpdateViewpoint`). In these rooms C-up is the toggle
  instead of first-person view, as in OoT. A prerendered room's camera otherwise holds still and does
  not change by floor (FREE0, as OoT's `func_80057FC4`).

The scene camera type travels in the 0x19 command: the converter writes it and 2Ship reads it for scenes
under `oot/` (`SetCameraSettingsFactory`). Checked against SoH booted at each house's entrance
(`HARNESS_ENTRANCE`): same camera, framing and panorama.

Not yet: the shopkeeper's browsing viewpoint (`En_Ossan` is not ported), and the 4:3 fixed backgrounds
leave the sides of a wider window to the depth geometry.

Not yet:

- **Actors:** only some waves are ported, so other OoT actors in room lists do not appear.
- **Scenes:** only the ones above; exits into others lead back to the spawn Link came in at.
- **OoT sound effects:** the OoT-only sound effects are not played yet (see Audio).
- **Scene effects:** OoT's per-scene animated textures are not drawn.
- **Cameras:** OoT settings with no MM counterpart (crawlspaces, magic beans, fishing, boss arenas,
  some temple rooms) use a general MM setting.
- **MM-only scene commands:** the actor cutscene list (0x1B) is started empty. The other MM-only
  commands are left out.

## Actor and object ids

OoT's actors and objects follow MM's in 2Ship's tables, in OoT's own order and suffixed `_Oot`
(`mm/include/tables/actor_table_oot.h` and `object_table_oot.h`, generated by
`tools/porting/gen_oot_tables.py`). A unified id is the OoT id plus `ACTOR_OOT_BASE` (0x2B2) or
`OBJECT_OOT_BASE` (0x283), from `HyruleIds.h`, which the exporter also uses to convert room data.

Actors that have not been ported are `DEFINE_ACTOR_UNSET`: their ids exist, but spawning one does
nothing. To port an actor, regenerate the tables with its OoT enum in `--ported`, which turns its line
into a `DEFINE_ACTOR` entry named `<Name>_Oot`. Player, the room-loading planes and collectibles
(`En_Item00`) still map to MM's own actors: the converter turns OoT's collectible types into MM's
(`OotToMm::ActorParams`) and gives these actors MM's spawn rotations. In Kokiri Forest every placed and
dropped collectible matches the SoH session. Ported actors that drop a collectible of a given OoT type
convert it the same way (`Item00_FromOot` in `HyruleActor.c`).

Chests (`En_Box`) map to MM's `En_Box` when MM has the item they hold: rupees, hearts, heart pieces and
containers, bombs, Bombchus, Deku nuts and sticks, arrows, magic, keys, map, compass and ice traps
(`OotToMm::MmGetItem`). The two games' chests share their parameter layout and chest types, so only the
get-item id changes; the chest's switch flag (its Z rotation) stays raw through MM's raw-rotation id
flags, and OoT's `object_box` becomes MM's. Chests holding anything else stay OoT actors, which are not
ported, until OoT's items are in the save. Mido's House's four chests sit where SoH's do with the same
flags, and one held in front of gives "You got a Blue Rupee!" with MM's chest and text.

Room actor lists keep OoT's rotations, since some OoT actors read parameters from them. MM keeps a
cutscene id and half-day bits in its rotations instead, so `Actor_SpawnEntry` spawns OoT ids with the
rotations as they are, no cutscene and every half-day.

The scene's sub-keep object (the special files command) is converted like room objects, so OoT's
`gameplay_field_keep` is loaded as `OBJECT_GAMEPLAY_FIELD_KEEP_Oot`.

## Ported actors

| Wave | Actors |
|------|--------|
| 1 | `Obj_Hana` (flowers, rocks, bush), `En_Kusa` (grass), `Obj_Mure2` (grass and rock rings) |
| 2 | `En_Ishi` (small and silver rocks) |
| 3 | `En_A_Obj` (signposts, blocks, grass clumps, tree stumps; part of `code` in OoT) |
| 4 | `En_Kanban` (wooden signs that can be cut apart) |
| 5 | `En_Wonder_Talk2` (invisible message spots), `En_Wonder_Item` (invisible item drops and switches) |
| 6 | `En_Ko` (the Kokiri children; Fado is left out) |
| 7 | `En_Md` (Mido) |
| 8 | `En_Goroiwa` (rolling boulder) |
| 9 | `En_Sw` (Skulltulas and Gold Skulltulas) |
| 10 | `Obj_Tsubo` (pots); chests (`En_Box`) map to MM's own, see Actor and object ids |
| 11 | Kokiri Forest's creatures: `En_Karebaba` (Withered Deku Babas), `En_Dekubaba` (Deku Babas), `En_Dekunuts` (Mad Scrubs) and their `En_Nutsball` nuts, `En_Okuta` (Octoroks) |
| 12 | `En_St` (Big Skulltula), `Obj_Mure3` (rupee circle), `En_Gs` (Gossip Stones), `Obj_Bombiwa` (bombable boulders), `En_Tana` (shop shelves) |

Ported actors live in `overlays/actors/ovl_<Name>/`, copied from SoH and changed as little as
possible:

- **Names:** functions are `static`, so they do not clash with MM actors of the same name. The profile
  is `<Name>_Oot_Profile`.
- **Ids:** actors and objects use the `_Oot` ids. OoT's `gameplay_keep` is always loaded in MM as
  `GAMEPLAY_KEEP`, so profiles and effects use that instead.
- **Assets:** headers come from `assets/oot/` (`tools/porting/gen_oot_asset_headers.py`), whose paths
  start with `oot/`. Data an actor keeps in its own overlay is extracted from `assets/xml/.../overlays/`.
- **Engine names:** MM's names replace OoT's: `ActorProfile`, `objectSlot` and `Object_GetSlot`,
  `cullingVolume*`, `terminalVelocity`, `speed`, `depthInWater`, `COL_MATERIAL_`, `ELEM_MATERIAL_`,
  `ATELEM_`, `ACELEM_`, and `ACTOR_FLAG_800` for `ACTOR_FLAG_GRASS_DESTROYED`. Sound effects use MM's
  ids of the same name.
- **Damage:** AC masks are written `DMG_FROM_OOT(<OoT mask>)` (see Damage types).
- **SoH additions:** enhancements, cvars, game-interactor hooks, stats and debug prints are dropped.
- **OoT code helpers:** the few `z_actor.c` functions MM lacks are in `HyruleActor.c`. MM keeps OoT's
  `Npc_UpdateTalking` and `Npc_TrackPoint`.
- **OoT state:** OoT's quest status, inventory and Gold Skulltula flags are in the shared inventory
  ([docs/INVENTORY.md](../docs/INVENTORY.md)), which `HyrulePort.h`'s `CHECK_QUEST_ITEM_OOT` (with
  `QUEST_*_Oot`), `INV_CONTENT_OOT` and `GET_GS_FLAGS_OOT` read. `HyrulePort.h` still stands in for
  what 2Ship does not have yet: age checks (Link is a child) and offering OoT trade items (never). Day
  and night are MM's. OoT scenes the table does not have yet are named in
  `HyruleScenesPending.h` (generated with the tables) with ids no scene uses, so checks for them never
  match.

`tools/porting/port_actor.py <SoH actor dir>` does the mechanical part of these steps and lists the lines
to review; the rest is done by hand.
- **Gaps:** anything that depends on a system not ported yet is marked with a `PORT:` comment.

Known differences:

- Grass drops come from MM's drop table of the same index, since OoT's tables and Deku seeds are not
  ported. Regrowing grass gives a recovery heart half the time and nothing instead of seeds.
- Bugs from grass and rocks wait for `En_Insect`.
- Cut signs (`En_Kanban`) are not repaired by Zelda's Lullaby, which MM's ocarina lacks; the pieces
  go when Link walks away, as in OoT. The cut direction comes from MM's melee animations, with Zora
  attacks cutting like sword swings and Goron punches like the hammer.
- OoT's age checks use `HyrulePort.h`, where Link is always a child until adult Link exists.
- OoT's background check flag 4 covers floor and water, while MM checks water only with 0x400, so
  ported `Actor_UpdateBgCheckInfo` calls with flag 4 also pass 0x400 (`port_actor.py` adds it).
- MM's Player recognises rocks by actor id, so `ACTOR_EN_ISHI_Oot` was added to its checks: silver
  rocks use the heavy lift and only Goron Link can lift them, as with MM's rocks.

Wave 1 was checked against the SoH session's second Kokiri Forest visit, with that run's Link
puppeting this fork's: every wave-1 actor matched in id, parameters, room and position on all 2631
scene frames, including spawning and despawning on room changes. Rotations differ because grass picks
a random yaw and the two engines draw different random numbers. Scripted replays with human Link
cut grass with a slash (dropping a collectible), and lift, throw and break it.

Wave 2 matches SoH the same way, except that `Obj_Mure2` spawns and removes its ring of rocks by its
distance from the camera, and MM's camera differs from OoT's: the ring appears and goes on different
frames (3 against 401 on entry), while on every frame both games have it, its rocks match. Human Link
lifts, throws and breaks a small rock in a scripted replay.

Wave 3's signposts match SoH's in position and parameters on every frame but one: on a room change,
MM removes an actor that was drawn that frame one frame later (`func_800BA798`), so the sign in view
outlives SoH's by a frame. In a scripted replay human Link reads the Know-it-All Brothers' sign: the
wooden textbox shows OoT's text and closes normally.

Wave 4's signs match SoH on all 2631 frames. A scripted replay reads "LINK's House" and cuts the top
off the sign with a slash.

Wave 5 matches SoH on all 2631 frames, including the two hidden rupees in room 2 that both Links
collect on the same frames. Walking out of the Kokiri Shop, human Link gets the forced "Hi, LINK! Look
this way!" message. OoT's Player cutscene actions 7 and 8 are MM's `PLAYER_CSACTION_END` and
`PLAYER_CSACTION_WAIT`; `En_Wonder_Item` drops MM's collectibles for OoT's (arrows come in MM's
bundles of 10 and 30).

Wave 6's eight Kokiri spawn, animate and despawn with SoH's on all 2631 frames. The one guarding the
forest exit aims at Link from about 1500 units away, so the puppet's few units of error move it by 1-3
units. Held in front of a Kokiri cutting grass, human Link talks to him ("That meanie, Mido, ..."). The
Kokiri also appear in the houses and the shop as in OoT (the Know-It-All Brothers, the twin, the shop's
Kokiri); Fado waits in the Lost Woods for the Odd Potion, which, like OoT's other items, is not in the
save yet. Until OoT's quest items exist, every Kokiri acts as at the start of the game.

Wave 7's Mido follows the SoH session within 1 unit on every frame (he shadows Link to block the path).
Human Link, carrying the debug save's Kokiri Sword and Hero's Shield, shows them to Mido, who sets
`EVENTCHKINF_SHOWED_MIDO_SWORD_SHIELD_Oot` and steps aside. OoT asks for the Deku Shield, which MM
lacks, so any shield counts. His Lost Woods scene (Saria's Song) waits for that scene and OoT's songs.

Wave 8's boulder rolls its path in step with SoH's until SoH's Link crawls into the tunnel (scene frame
1416): the boulder stops while Link is in a cutscene state, and the puppet copies Player's position but
not its state. Its attack mask uses `DMG_FROM_OOT` so that MM's Player, whose damage checks read only
MM's bits, registers it: with Link held on its path, the boulder knocks him down, bounces back, waits
and rolls on, as in OoT.

Wave 9's Gold Skulltula on the Know-it-All Brothers' house matches SoH on all 2631 frames (the debug
boot is at night). Held level with it, human Link kills it with sword slashes through its converted
OoT damage table. Its token (`En_Si`) is not ported yet, and OoT's Gold Skulltula flags are not in the
save, so it returns. OoT sound effects MM lacks use the nearest MM ones, marked `PORT:`.

Wave 10's pots, and the Kokiri, collectibles, message spots and item spots in the houses and the shop,
match SoH booted at each entrance (`HARNESS_ENTRANCE`) on every frame of the run, about 340 each. Held
next to a pot in the Know-It-All Brothers' house, human Link lifts it, throws it, and it breaks and drops
its green rupee. Pots use `object_tsubo`, or `gameplay_dangeon_keep` in dungeons.

Wave 11's creatures were checked against SoH booted at the same entrance with adult Link
(`HARNESS_ADULT`), its Link puppeting this fork's. The Octoroks surface, fire and sink, the Mad Scrubs
pop up and fire, and the Deku Babas wait and lunge on the same frames and at the same positions as in
SoH, until the two cameras differ: MM, like OoT, only updates an actor inside the camera's culling
volume, and MM's camera frames a knocked-back Link differently, so an Octorok or a Deku Baba pauses on
frames where SoH's runs, or the reverse. Held next to them, child Link cuts a Withered Deku Baba's head
off and adult Link kills a Deku Baba (it drops a Deku Nut); with his shield up, adult Link sends an
Octorok's rock back and kills it. A Mad Scrub's nut hits the same edge of the ground as in SoH.

- Their damage tables and AC masks are OoT's, through `DMG_TABLE_FROM_OOT` and `DMG_FROM_OOT`. The Deku
  Baba sets its Master jump slash entry by Link's age on each spawn: OoT changed the shared table once
  per overlay load, and 2Ship never reloads it.
- OoT's drop flags (`Actor_SetDropFlag_Oot`) also freeze them for magic ice and fire; their random drops
  come from MM's tables (see the grass above).
- MM's Hero's Shield, which takes the Deku Shield's place, sends rocks and nuts back as it does in MM;
  OoT's rule (the Deku Shield, or the Hylian Shield in adult Link's hand) is kept.
- MM has no ring of fire (`EffectSsFCircle`) for a Mad Scrub burnt by fire, so it is left out.
- Navi's enemy information is not given; MM's Tatl hints do not cover OoT enemies yet.

Wave 12 was checked with one hand-written puppet log driving both SoH and this fork through adult Kokiri
Forest: the Big Skulltula, the rupee circle and its seven rupees (MM's `En_Item00`), and the Gossip Stone
match SoH on every frame, apart from the circle removing its rupees by the camera's distance (ten frames
apart, as with `Obj_Mure2`). The shop shelves match SoH's shop run.

- Talking to a Gossip Stone shows OoT's text, and its own message with MM's or OoT's Mask of Truth.
- OoT's ocarina is not ported, so Gossip Stones hear MM's, as MM's own stones do: Epona's Song and the Song of
  Time bring a healing fairy and the Song of Storms a big one, MM's `En_Elf` with the same fairy types. OoT's
  Saria's Song, Zelda's Lullaby and Sun's Song wait for OoT's songs.
- Boulders break when one of MM's bombs explodes next to them (`func_80033684`, OoT's search for an
  exploding bomb, works on MM's `En_Bom`) or under the Megaton Hammer.

## Text

OoT's messages are added to MM's message table at startup (`HyruleText.cpp`), with `TEXT_OOT_BASE`
(0x8000) added to their ids and to the next-message id in their headers. Text ids stay 16-bit: MM's run
to 0x354C (plus 0xFFFC) and its credits from 0x4E20 to 0x4E4C, while OoT's run to 0x71B2. Ported code
writes `TEXT_OOT(<OoT id>)`. MM sends ids from 0x4E20 up to the credits table, so that check now stops
at `TEXT_OOT_BASE`.

The text itself is converted at extraction (`OotToMm::ConvertMessage`). Textbox types are kept, which
works for signs (type 1 is the wooden board in both games); OoT's other types, colours and icons are
not checked yet.

## Audio

`assets/xml/N64_NTSC_11/audio/Audio.xml` extracts OoT's 110 sequences, 38 soundfonts and 451 samples
into `oot.o2r` under `oot/audio/`. At startup (`mm/src/audio/lib/load.c`) OoT's soundfonts are given
the ids after MM's, custom soundfonts follow them, and OoT's sequences are loaded at `SEQ_OOT_BASE`
(0x400) + their OoT id with their font lists shifted to match. 0x400 is clear of MM's sequences (below
0x80) and of the ids the audio editor and custom sequences use; sequence ids are 16-bit in 2Ship.

Hyrule scenes name OoT sequences, so `Hyrule_AfterSceneCommands` adds `SEQ_OOT_BASE` to the scene's
music, and `AMBIENCE_OOT_BASE` (0x40) to its nature ambience. MM plays a scene's music by day and its
ambience at night, unless the ambience is 0x13 (none), as OoT does; OoT's 0x13 is left as MM's. For the
ids from 0x40, `Audio_PlayAmbience` plays OoT's ambience sequence (`SEQ_OOT_NATURE_AMBIENCE`) on MM's
ambience player with OoT's channel settings (`HyruleAudio.c`). MM's ambience grew out of OoT's, so the
channels, IO ports and rain and critter controls are the same; OoT played it on the main music player.
Not ported: OoT plays the Kokiri ambience on the Lost Woods bridge whatever the scene says. Sequence flags (resume, fanfare, enemy music...) come from OoT's table for OoT's
sequences (`HyruleAudio.c`, through `Audio_GetSeqFlags`); the two games use the same flag bits.

Kokiri Forest plays its theme by day (`HYRULE_BOOT_TIME=1200` with the boot shortcut) and its
night ambience at night (`HYRULE_BOOT_TIME=2200`). Checked against SoH's Kokiri Forest at 22:00
(`HARNESS_TIME=2200`): the ambience's spectrum matches SoH's (correlation 0.94 over 24 bands; against
the daytime music, 0.3).

## Event flags

OoT's event flags are in the save as `eventChkInfOot`, `itemGetInfOot` and `infTableOot` (OoT's sizes),
with the unsaved `eventInfOot` in `SaveContext`; save migration 10 adds them to older saves. Since
Hyrule resets completely with each cycle (docs/DESIGN.md), `Sram_SaveEndOfCycle` clears them all. The
states the design keeps across cycles (the rainbow bridge, the Mask Shop closure, Termina being
unlocked) will need flags outside these arrays.

`HyruleFlags.h`, generated by `tools/porting/gen_oot_flags.py` from SoH's `z64save.h`, names every flag
with an `_Oot` suffix and gives ported code `GET_EVENTCHKINF_OOT(flag)` and the matching `SET_`,
`CLEAR_` and other arrays' macros.

## Item ids

Item ids are 16-bit wherever they are stored: the inventory, C-button and D-pad items in saves,
`Player.heldItemId`, the item functions in `z64interface.h` and the item hooks. MM's items keep
their values, so saves need no migration. OoT's items follow MM's last one (`ITEM_NONE`, 0xFF) from
`ITEM_OOT_BASE` (0x100), suffixed `_Oot` (`mm/include/tables/item_id_oot.h`, generated with the tables).

OoT items cannot be given or shown yet: `Item_Give` and `Item_CheckObtainability` return `ITEM_NONE`
for them, and MM's item tables (slots, icons, prices, draw table) and get-item ids only cover MM's
items. Those are extended when OoT's items and interface are ported.

## Damage types

Collider damage flags (`dmgFlags`) are 64-bit and damage tables have 64 entries. MM's damage types keep
bits 0-31 and table entries 0-31; OoT's follow in bits 32-63 as `DMG_<NAME>_Oot`
(`mm/include/z64collision_check.h`) and use entries 32-63. MM's damage tables leave those entries at
zero, so MM actors ignore OoT damage until cross-play decides otherwise.

OoT actors are hit by MM's Player through `DMG_FROM_OOT`, which keeps an OoT mask in bits 32-63 and adds
the MM attacks that stand in for each OoT one:

| OoT | MM |
|-----|----|
| Deku nut, Deku stick, hookshot, normal/fire/ice/light arrows, unblockable | the same |
| Slingshot | Deku bubble |
| Explosive | explosives, powder keg |
| Boomerang | Zora boomerang |
| Hammer swing / hammer jump | Goron punch / Goron pound |
| Sword slashes | sword, Zora punch |
| Spin attacks | spin attack, Deku spin |
| Jump attacks | sword |
| Shield / mirror ray | normal shield / light ray |

OoT's spells and unknown arrows have no MM counterpart. MM-only attacks (Goron rolling, Deku launch,
the horse, thrown objects, sword beam, roll) are not added, so an OoT actor that should react to them
needs its own mask.

OoT enemies' damage tables are written `DMG_TABLE_FROM_OOT(<OoT's 32 entries>)`: OoT's entries fill
32-63, and each of MM's attacks takes the entry of the OoT attack it stands in for (sword and Zora punch:
Kokiri Sword; Goron punch: hammer; Goron pound and spikes: hammer jump; spin attack and Deku spin:
Kokiri spin; sword beam: Master spin; Deku bubble: slingshot; thrown objects: Deku stick; powder keg:
explosives). The horse, Deku launch, Zora barrier and roll do nothing to OoT enemies. OoT's 23 preset
tables are in `HyruleDamage.c` (`DamageTable_Get_Oot`).

MM's `DMG_` constants are 64-bit so that `flags &= ~DMG_X` clears only `DMG_X`, matching MM's integer
masks, and the top MM bit does not spill into OoT's. Variables that copy damage flags should be `u64`.
