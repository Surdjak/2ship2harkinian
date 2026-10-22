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
