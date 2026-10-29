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
- **Cutscenes:** not converted. Scripts stay in OoT's format, as SoH writes them, and play with OoT's
  interpreter (Cutscenes, below); 2Ship's cutscene importer reads `oot/` scripts as they are.

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
| Sacred Forest Meadow | `SCENE_SACRED_FOREST_MEADOW_Oot` (0x79) | `0xEC00` |
| Grottos | `SCENE_GROTTOS_Oot` (0x7A) | `0xEE00` |
| Fairy's Fountain | `SCENE_FAIRYS_FOUNTAIN_Oot` (0x7B) | `0xF000` |
| Hyrule Field | `SCENE_HYRULE_FIELD_Oot` (0x7C) | `0xF200` |
| Kakariko Village | `SCENE_KAKARIKO_VILLAGE_Oot` (0x7D) | `0xF400` |
| Zora's River | `SCENE_ZORAS_RIVER_Oot` (0x7E) | `0xF600` |
| Lake Hylia | `SCENE_LAKE_HYLIA_Oot` (0x7F) | `0xF800` |
| Gerudo Valley | `SCENE_GERUDO_VALLEY_Oot` (0x80) | `0xFA00` |
| Lon Lon Ranch | `SCENE_LON_LON_RANCH_Oot` (0x81) | `0xFC00` |
| Death Mountain Trail | `SCENE_DEATH_MOUNTAIN_TRAIL_Oot` (0x82) | `0x10000` |

OoT's scenes take MM's `RESTRICTIONS_NONE`. OoT's own restrictions (its `sRestrictionFlags`) are in
`HyruleRestrictions.c`, read so far for the ocarina (its button greyed where OoT forbids it), the warp songs
and the spells; OoT's others (B, A, bottles, trade items, the hookshot, the Sun's Song and every item) are not
applied yet.

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
- **Scene effects:** OoT's scene draw configs (z_scene_table.c) run for OoT scenes through MM's
  `SCENE_DRAW_CFG_HYRULE_OOT` (`HyruleSceneDraw.c`), but only those of the scenes loaded so far (Kokiri Forest,
  the Lost Woods, the grottos, the fairy fountain, calm water); the others draw as MM's default.
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
| 13 | Sacred Forest Meadow: `En_Wf` (Wolfos), `En_Mb` (Moblins), `Bg_Spot05_Soko` (maze walls and gate), `Bg_Mjin` (warp pads); `En_Part` (OoT's flying enemy parts and flames) |
| 14 | Grottos: `Door_Ana` (grotto holes), `En_Torch` (the grotto chest) |
| 15 | Hyrule Field: `En_Wood02` (trees and bushes), `En_Peehat` (Peahats), `Obj_Hamishi` (adult rocks), `Bg_Spot00_Break` (broken fences) |
| 16 | `En_Encount1` (enemy spawner), `En_Skb` (Stalchildren), `En_Reeba` (Leevers), `En_Tite` (Tektites) |
| 17 | Kakariko to Death Mountain scenery: `En_River_Sound` (river, waterfall and other place sounds), `Obj_Kibako2` (big crates), `En_Hata` (flag), `Bg_Bombwall` (bombable walls), `Bg_Spot16_Doughnut` (Death Mountain's cloud ring), Kakariko's `Bg_Spot01_Fusya` (windmill), `Bg_Spot01_Idohashira`, `Bg_Spot01_Idomizu` and `Bg_Spot01_Idosoko` (the well's beam, water and stone), `Bg_Spot01_Objects2` (buildings under construction), `Bg_Gate_Shutter` (Death Mountain gate), `Bg_Spot03_Taki` (Zora's River waterfall), `Bg_Spot09_Obj` (Gerudo Valley's bridges and tent), `Bg_Umajump` (horse jumps), `Bg_Ingate` (Lon Lon Ranch gate), `Obj_Syokudai` (torches) |
| 18 | Their creatures and contraptions: `En_Niw` (cuccos) and `En_Attack_Niw` (the angry swarm), `Obj_Mure` (groups of fish, bugs and butterflies) with `En_Fish`, `En_Insect` and `En_Butte`, `Obj_Makekinsuta` (soft soil), `En_Crow` (Guays), `En_Cow`, `En_Bombf` (bomb flowers), `Bg_Spot16_Bombstone` (Dodongo's Cavern boulder), `En_Encount2` and `En_Fire_Rock` (Death Mountain's falling rocks), `Bg_Spot06_Objects` (Lake Hylia's water and lock), `Bg_Spot15_Rrbox` (pushable crates) |
| 19 | `En_Door` (doors with handles) and `En_Weather_Tag` (clouds, rain, snow and storms near Link) |
| 20 | Kakariko's people: `En_Heishi2` (the Death Mountain gate guard, and OoT's other guards), `En_Heishi4` (guards), `En_Toryo` (the boss carpenter), `En_Daiku_Kakariko` (carpenters), `En_Ani` (the man on the roof), `En_Niw_Lady` (the cucco lady), `En_Hy` (townsfolk); `Bg_Spot15_Saku` (Hyrule Castle's gate, which a guard opens) |
| 21 | The other people of the six scenes: `En_Ms` (the bean seller), `En_Hs2` (a sulking young man in Kakariko at night), `En_Tg` (the dancing couple), `En_Ge1` and `En_Ge2` (Gerudo guards), `En_Zo` (Lake Hylia's Zoras); `Item_Etcetera` (Ruto's Letter in its bottle, and other items set out), `Elf_Msg2` (Navi's message spots), `Bg_Haka` (gravestones) |
| 22 | `Obj_Bean` (soft soil for magic beans, and the plants Link rides as an adult), `En_Po_Field` (Hyrule Field's Poes), `Object_Kankyo` (Kokiri Forest's dust motes and OoT's other scene effects), `Bg_Spot00_Hanebasi` (Hyrule Castle's drawbridge) |

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
- Cut signs (`En_Kanban`) are repaired by Zelda's Lullaby; the pieces go when Link walks away, as in OoT. The cut direction comes from MM's melee animations, with Zora
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
- Gossip Stones hear Link's ocarina, which plays OoT's songs too (docs/SONGS.md): Zelda's Lullaby, Saria's Song,
  Epona's Song, the Sun's Song and the Song of Time bring a healing fairy and the Song of Storms a big one, MM's
  `En_Elf` with the same fairy types.
- Boulders break when one of MM's bombs explodes next to them (`func_80033684`, OoT's search for an
  exploding bomb, works on MM's `En_Bom`) or under the Megaton Hammer.

Wave 13 was checked with one puppet log driving SoH and this fork through the meadow at each age: the maze walls,
the gate and the warp pad match SoH on every frame. The Wolfos and the Moblins behave alike until their first
random wait, since OoT's and MM's random number generators draw different values. Adult Link kills a spear Moblin;
child Link's front slashes bounce off the Wolfos' guard as in OoT.

- OoT's `En_Part` is ported for the parts and flames enemies leave (`func_800358DC`, the Moblin's club flames); MM's
  own `En_Part` is a different actor.
- The miniboss music for the gate's Wolfos uses MM's `Audio_PlayBgm_StorePrevBgm`.
- OoT's camera glance at the opening gate (`OnePointCutscene_Attention`) is left out.

Wave 14 opens OoT's grottos. MM's grottos are OoT's design: a hole stores where Link fell in, and what the grotto
holds, as a return point (OoT's `RESPAWN_MODE_RETURN`, MM's `RESPAWN_MODE_UNK_3`), and the grotto's exit
(OoT's `ENTR_RETURN_GROTTO`, MM's 0xFFFF) brings him back up out of the hole. So OoT's `Door_Ana` keeps its
return point where MM's Player and `En_Torch` read it, its grotto entrances go through `Hyrule_EntranceFromOot`, and
the exit lists translate OoT's return exit to MM's. `En_Torch` spawns MM's chest (`En_Box`) with MM's get items.
In the Lost Woods, after a bomb breaks the boulder on it, child Link falls into the hole, lands in the grotto
with its chest (a blue rupee, as in OoT), grass and Gossip Stone, and walking into the exit light brings him back
up out of the hole. Holes opened by the Song of Storms open on MM's (OoT's storm flag is not ported).

Wave 15 was checked against SoH in Hyrule Field at each age: the trees, Peahats, rocks and fences match SoH's
positions on every frame, and with both Links puppeted to a Peahat, it rises and flies at Link within a unit of
SoH's for 900 frames. A Peahat larva bursts as MM's bomb (`En_Bom`, whose rotation and timer match OoT's), and
OoT's held death sound (`EffectSsDeadSound`) plays from its spot. The castle drawbridge (`Bg_Spot00_Hanebasi`)
waits for OoT's day and night: it closes on OoT's clock and puts Link out of the market at nightfall.

Wave 16's spawner raises Hyrule Field's Stalchildren at night as in SoH: with both Links puppeted along the same
path at 22:00, two rise beside Link at once, sink when he walks off and two more rise where he stops, on SoH's
frames (their spots differ, as they are random). OoT's `BodyBreak` (the parts an enemy bursts into) is in
`HyruleActor.c`, spawning OoT's `En_Part`; the Wolfos the spawner raises count back down when they die.

Not yet: the grottos' business scrubs, cows, fish, bees, Redeads, Tektites and Dampé's hole. The rainbow
hexagon under the exit light is OoT's own (SoH shows it too).

Wave 17 was checked against SoH at each of the six scenes' first spawn (child, and adult in Kakariko and
Gerudo Valley): with Link puppeted from SoH's run, the same ported actors are there in the same places on every
frame; Death Mountain's cloud ring turns on SoH's frames until it leaves the camera, which differs. Notes:

- `En_River_Sound` uses MM's names for OoT's river audio, which MM kept: `Audio_PlaySfx_River`,
  `Audio_LowerMainBgmVolume`, and `func_801A09D4` for the Lost Woods' Saria's Song that grows near the right path.
- The torches take MM's arrows (`En_Arrow`) and light them as MM's torches do; the Deku Stick flame is MM's
  Player `unk_B28`. OoT's camera glances (`OnePointCutscene_Attention`, the waterfall's `OnePointCutscene_Init`)
  are left out.
- MM's `LINK_AGE_IN_YEARS` reads MM's adult transformation, so ported code uses `LINK_AGE_IN_YEARS_OOT`
  (`HyrulePort.h`): the bridges and well water follow OoT's age.
- The well beam and the cloud ring's eruption follow OoT's cutscene cues (`gCsCtxOot.npcActions`), which only
  OoT's story cutscenes give; those are not triggered yet (docs/CUTSCENES.md).

Wave 18 was checked the same way, adding adult runs at Lake Hylia and Death Mountain Trail: the same actors are in
the same places as SoH's, except that the cuccos wander to random spots (RNG) and adult Lake Hylia's Guays
take off at once, where SoH holds them through its first-visit cutscene, which this check did not play (it plays
since docs/CUTSCENES.md step 2). A cucco
made angry (under gdb) brings up to six attack cuccos, which hit Link seven times in the next 770 frames. Notes:

- `En_Butte`'s fairy is MM's `En_Elf` (`FAIRY_TYPE_2`, OoT's `FAIRY_HEAL_TIMED`); OoT's `En_Elf` is not ported.
- `En_Cow` reads MM's `gHorsePlayedEponasSong`, OoT's `DREG(53)`; the Guays' Skull Mask is OoT's
  (`Player_GetMaskOot`).
- `Bg_Spot15_Rrbox`'s three crates at Lon Lon Ranch are in one of its cutscene layers, so they are built but not
  seen in play yet.

Wave 19 was checked the same way. The doors are SoH's, with the same params, in the same places, in child and adult
Kakariko, Lake Hylia and Lon Lon Ranch. Adult runs that walk Link into a weather tag's range give the same
environment as SoH's on every frame: in Kakariko, Death Mountain's clouds come at the same frame, with the same
skybox and light changes; at Lake Hylia, so does the rain. The rain count steps up every eighth frame since boot, so
it can be a frame apart. Under gdb, Link opens Lon Lon Ranch's stable door (the stable's exit then leads back to
the spawn, since the stable is not loaded yet), and a closed Kakariko door says "Granny's Potion Shop / Closed".
Notes:

- MM's Player opens a door through MM's `KnobDoorActor`, so OoT's `EnDoor` starts with one: Player sets
  `animIndex` (the transformation, plus `PLAYER_FORM_MAX` for a right-hand door) where OoT's sets `animStyle`,
  and `requestOpen` for `playerIsOpening`. The door picks OoT's child or adult animation by OoT's age. Like MM's
  doors, it queues MM's door cutscene (`func_80122F28`) when it offers itself to Player, which MM's Player waits for.
- MM's Player reads type 7 as MM's framed door, which loads no room, so OoT's `DOOR_ROOMLOAD7` becomes
  `DOOR_ROOMLOAD`, which OoT treats the same way. Locked doors use OoT's small keys
  (`Inventory_GetDungeonIndexOot`). Navi's "locked" line is left out: MM's `tatlTextId` cannot hold an OoT text id.
- OoT's and MM's `EnvironmentContext` share their layout, so `En_Weather_Tag` changes the skybox and light
  configs as OoT's does. OoT's weather modes go in MM's `gWeatherMode` past MM's own (`WEATHER_MODE_OOT`), so MM's
  `Environment_Init` does not take them for its own. As in OoT, a mode is kept through an exit
  (`retainWeatherMode`) into an OoT scene, which starts with its sky, light and rain or snow (checked under gdb:
  leaving Kakariko while cloudy starts Hyrule Field cloudy). MM draws rain only in its storm, so
  `Environment_DrawRain` draws it in OoT's scenes whenever there is some, as OoT does. The skies are MM's (MM's
  config 1 is its cloudy second day).

Wave 20 was checked the same way: the people are SoH's, in the same places, in child and adult Kakariko and at
Lon Lon Ranch, and adult Kakariko's match on every frame. In child Kakariko a guard turns toward Link before SoH's
does (the puppet does not copy Player's cutscene state) and the walking carpenters run up to 1.3 units apart (floor
heights). Under gdb, each one Link faces answers A with OoT's text, and child Link showing Zelda's Letter to the gate
guard gets OoT's reply and opens the Death Mountain gate. Notes:

- OoT's actors ask for and read exchange items (OoT's `ExchangeItemID`, `HyrulePlayer.h`), which map to MM's item
  actions both ways (`Hyrule_ExchangeItemToActionOot`, `Actor_GetPlayerExchangeItemIdOot`). MM's Player gives any
  item to an actor that asked for one; for OoT's actors it keeps OoT's rule (`Hyrule_IsExchangeAcceptedOot`): only
  the item asked for, a Big Poe for a Poe, and anything for Blue Fire. Ruto's Letter cannot be shown yet.
- OoT's people react to OoT's masks with OoT's texts (`Text_GetFaceReactionOot`); the Kokiri and Mido used MM's
  function before, which reads MM's masks.
- SoH's `SkelAnime_DrawSkeletonOpa` draws flexible skeletons as such; the port tool had made it MM's
  `SkelAnime_DrawOpa`, which draws the townsfolk's bodies as torn triangles. Ported actors now use
  `SkelAnime_DrawSkeletonOpaOot` (`HyruleActor.h`), which picks by the skeleton (earlier waves' cuccos, cows, Guays,
  fish, Wolfos and Moblins included).
- The beggar's bottles are MM's (`Inventory_HasItemInBottle`); the lost dog uses MM's leftover `dogParams` and
  `dogIsLost`.

Wave 21 was checked the same way: the new people are SoH's, in the same places, and the six scenes diverge from SoH
as before. The one new divergence is Gerudo Valley's gate guard, which SoH updates once on its first frame (OoT
updates a new actor before the camera can cull it) and so stands 15 units higher on its floor, and the patrols,
which start walking a frame later; the guard reaches its floor as soon as Link comes near. Under gdb: Link buys
beans from the bean seller at OoT's rising prices; the Gerudo gate guard turns child Link away and a patrol catches
him and throws him out of the valley; Lake Hylia's Zoras surface and talk; with the Silver Scale, Link dives for the
bottle with Ruto's Letter; Tatl reads Navi's message spot (C-up); adult Link pulls the gravestone; the young man at
night and the couple talk. Notes:

- MM numbers Player's cutscene actions differently: OoT's 7 (end) and 8 (wait) are MM's `PLAYER_CSACTION_END` and
  `PLAYER_CSACTION_WAIT`, and OoT's 95 (hands up, caught) is MM's 26. Ported actors call
  `Player_SetCsActionWithHaltedActorsOot` (`HyrulePlayer.h`). Wave 20's guards passed OoT's numbers to MM, which
  kept Link waiting after the Death Mountain gate opened; they now release him.
- OoT's message system closes a choice once it is made unless the actor goes on to another text that frame, and
  OoT's actors rely on it (the bean seller hands his bean over as the box closes). MM's actors close their choices
  themselves, so `Message_Update` does it for OoT's texts only.
- MM's Player and `Actor_OfferGetItem` took get-item ids below `GI_MAX` only, so an OoT actor could hand Link
  nothing but chests (whose negative ids take another path); they now take OoT's (`GI_OOT_BASE` up).
- OoT's exits by entrance name go through `Hyrule_ExitFromOot` (the port tool turns
  `play->nextEntranceIndex = ENTR_X;` into it); exits to scenes that are not loaded yet lead back to the spawn.
- MM's circle transition has no starburst, so the Gerudo's capture is MM's plain circle.

Wave 22 was checked the same way: the six scenes diverge from SoH as before, and the regressions match but for the
random numbers in Kokiri Forest, which the dust motes draw from the first frame. Under gdb: child Link plants a bean
from the bean seller in Zora's River's soil (the soil takes MM's Magic Beans under OoT's rule), and adult Link rides
the grown plant up the river and back; a Big Poe rises near adult Link in Hyrule Field at night and flees; the
drawbridge is up at night for child Link and comes down at dawn; Kokiri Forest has its 64 dust motes. Notes:

- OoT's and MM's `EnvironmentContext` share their layout under other names; the port tool now renames OoT's
  (`gloomySkyMode` is MM's `stormRequest`, `unk_EE` its `precipitation`...), and OoT's clock (`dayTime`,
  `gTimeSpeed`) is MM's.
- A bean is watered by the Song of Storms, which MM's ocarina mode reports (`OCARINA_MODE_PLAYED_STORMS`), as for
  the grottos. OoT's sprout camera cutscene and bean-plant camera settings are not in MM's camera; the ride keeps
  Link's camera.
- `SkelAnime_DrawSkeleton2Oot` and OoT's `D_80116280` (`HyruleActor.h`) draw the Poes' translucent skeletons.
- Not checked yet: killing a Poe and bottling its soul, which needs arrows from Epona for a Big Poe.

## Cutscenes

OoT's scripts play with OoT's interpreter (`HyruleCutscene.c`, from SoH's `z_demo.c`, docs/CUTSCENES.md), in OoT's
`CutsceneContext` (`gCsCtxOot`) and with OoT's trigger, apart from MM's. While one plays, MM's
`play->csCtx.state` reads `CS_STATE_RUN` and `curFrame` follows OoT's frame, so MM's Player, interface and
actors see a cutscene, and `Play_Update` runs OoT's interpreter instead of MM's. OoT's script camera (spline
points relative to the world or to Link) drives MM's cutscene sub-camera through `Play_SetCameraAtEye`, its FOV
and roll. OoT's environment flags (`Flags_GetEnvOot`) are kept here and cleared on scene load, as in OoT.
`HyruleCutscene.c` is generated (the port tool's renames plus fix-ups); the Link actions of story cutscenes are
not ported yet.

Scene cutscenes start as in OoT (`HyruleSceneCutscene.c`, from Play_Init): a cutscene layer (4 and up) plays its
scene's script, and otherwise OoT's entrance cutscene table plays a first-visit introduction (or Epona's fence
jumps, or one of its story scenes) by OoT entrance, age and event flag. The introductions' flags
(`EVENTCHKINF_ENTERED_*`) survive the cycle reset, so each plays once per save (docs/CUTSCENES.md C2). OoT's
conditional triggers (Saria on the bridge, the Nocturne...) are not ported yet.

Played so far: the warp songs' warp-out and warp-in (`Demo_Kankyo`), from both worlds; the introductions of Death
Mountain Trail, Hyrule Field, Kakariko, Lake Hylia, Gerudo Valley and Lon Lon Ranch; and Kokiri Forest's
cutscene layers (the opening narration, the Deku Tree), reached only from story cutscenes. Link leaves through the
pad's entrance (`Hyrule_WarpSongLeaveOot`) with respawn flag 0, since MM gave OoT's -3 to grotto returns, and
Player starts in MM's warp-song arrival, which now spawns OoT's `Demo_Kankyo` (MM's leftover spawned MM's).

## Text

OoT's messages are added to MM's message table at startup (`HyruleText.cpp`), with `TEXT_OOT_BASE`
(0x8000) added to their ids and to the next-message id in their headers. Text ids stay 16-bit: MM's run
to 0x354C (plus 0xFFFC) and its credits from 0x4E20 to 0x4E4C, while OoT's run to 0x71B2. Ported code
writes `TEXT_OOT(<OoT id>)`. MM sends ids from 0x4E20 up to the credits table, so that check now stops
at `TEXT_OOT_BASE`.

The text itself is converted at extraction (`OotToMm::ConvertMessage`). Textbox types are kept, which
works for signs (type 1 is the wooden board in both games); OoT's other types, colours and icons are
not checked yet.

Scene wind: MM's Player is blown by the scene's environment wind (scene command 0x05) from 50 up; OoT's only
sways flags with it (En_Hata). Player ignores it in OoT scenes, or Death Mountain Trail's (130) blows Link back
down into Kakariko.

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

Hyrule Field's music (OoT sequence 0x02) is a program that copies its segments (OoT sequences 0x03-0x17:
intro, segments, enemy and idle) into two buffers of its own data with the seq player's "load sequence"
instruction (0xB0), choosing them by the mode Player writes to IO port 2 (moving, still or enemy). MM still has
OoT's code for that mode (`Audio_SetSequenceMode`). The seq player adds `SEQ_OOT_BASE` to the sequence ids of
that instruction and of "start sequence" (0xC4) when the running sequence is OoT's, and `AudioLoad_SlowLoadSeq`
now checks the sequence map instead of the table count the port never sets (as SoH does). Checked: standing
still plays the idle segments and running the field segments; at night the field plays its ambience.

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
