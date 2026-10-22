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
- `assets/xml/N64_NTSC_11/` has XMLs for Kokiri Forest (`spot04`), the objects its rooms load,
  `gameplay_keep`, `gameplay_field_keep` and `link_animetion`.

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
  not emitted. Actor and object IDs, behavior types, camera type and world map area are not remapped.
- **Text:** OoT control codes become MM ones, item icons and text-ID jumps move into MM's message
  header, and codes without an MM equivalent are dropped and reported in the extraction log. Textbox
  types, sound effects and item IDs are not remapped.
- **Cutscenes:** stub. Each becomes an empty MM cutscene that only runs to its end frame, and the
  skipped OoT commands are reported in the extraction log.

## OoT scenes in game

`ExtractOotAssets` copies `oot.o2r` next to the game, and `HyruleScenes.cpp` loads it. OoT scenes follow
MM's in the scene table, suffixed `_Oot`. So far there is one:

| Scene | Scene id | Entrance (spawn 0) |
|-------|----------|--------------------|
| Kokiri Forest | `SCENE_KOKIRI_FOREST_Oot` (0x71) | `0xDC00` |

To go there:

- start the game with `HYRULE_BOOT_ENTRANCE=0xDC00`, which skips the title screen and starts a debug
  save there;
- in the console, run `entrance dc00`;
- or pick "Kokiri Forest (OoT)" in Better Map Select.

What works: the scene and its rooms load, render and collide; MM's Link spawns at OoT's spawn points
and walks between rooms through the room-loading planes (`En_Holl`, mapped to MM's). Collision camera
settings are mapped to MM's (`OotToMm::CameraSetting`), and a spawn's camera index refers to the
collision camera list as it does in OoT.

Not yet:

- **Actors:** OoT actors are not ported, so rooms are loaded without their actor lists.
- **Exits:** every exit leads back to the scene's first spawn.
- **Music:** none plays, since OoT's sequences are not extracted.
- **Scene effects:** OoT's per-scene animated textures are not drawn.
- **Cameras:** OoT settings with no MM counterpart (crawlspaces, magic beans, fishing, boss arenas,
  some temple rooms) use a general MM setting.
- **MM-only scene commands:** the actor cutscene list (0x1B) is started empty. The other MM-only
  commands are left out.
