# hyrule/

Ocarina of Time code ported into the 2Ship engine. See [docs/PLAN.md](../docs/PLAN.md).

Every `.c`, `.cpp`, `.h` and `.hpp` file under this directory is compiled into the `2ship` target
(`2s2h.elf`) with the same include paths, definitions and flags as `mm/`. This directory is also on
the include path, so headers here are included relative to `hyrule/`.

Re-run the CMake configure step after adding or removing files, because sources are gathered with
`file(GLOB_RECURSE)`.
