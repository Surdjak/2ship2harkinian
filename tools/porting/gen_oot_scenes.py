#!/usr/bin/env python3
"""Generate the OoT scenes in 2Ship's scene table and their entrances from SoH's tables.

The scenes in SCENES are added after MM's, in that order, so a scene keeps its id when more are added
(append new scenes at the end). Each gets an entrance scene (ENTR_SCENE_<name>_Oot) whose entrance
table is built from OoT's entrance table:

- OoT groups its entrances by scene layer (child day, child night, adult day, adult night, then
  cutscenes). Each group whose first entry leads to one of these scenes becomes one MM entrance, with
  OoT's layers as the MM entrance's layers.
- The MM entrance's spawn bits are the group's OoT spawn number, which is unique per scene, so
  ENTRANCE(KOKIRI_FOREST_Oot, 3) is Kokiri Forest's spawn 3.
- The flags keep OoT's layout, which is MM's: continue BGM, title card, end and start transitions.
  OoT's circle transitions do not exist in MM and become fades of the same color and speed.

It also writes each scene's OoT id (for what OoT indexes by scene, such as dungeon keys), and the map
from each OoT entrance index to an MM entrance, for OoT exits; entrances into
scenes not listed here map to HYRULE_ENTRANCE_NONE. HyruleScenesPending.h is rewritten without the
listed scenes.

    python3 tools/porting/gen_oot_scenes.py ../Shipwright/soh/include/tables
"""
import argparse
import os
import re

from gen_oot_tables import gen_pending_scenes

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")

# OoT scenes in the scene table, in id order. Append only.
SCENES = [
    "SCENE_KOKIRI_FOREST",
    "SCENE_LINKS_HOUSE",
    "SCENE_KOKIRI_SHOP",
    "SCENE_KNOW_IT_ALL_BROS_HOUSE",
    "SCENE_TWINS_HOUSE",
    "SCENE_MIDOS_HOUSE",
    "SCENE_SARIAS_HOUSE",
    "SCENE_LOST_WOODS",
]

# Human-readable names that the enum name spells badly.
NAMES = {
    "SCENE_LINKS_HOUSE": "Link's House",
    "SCENE_KNOW_IT_ALL_BROS_HOUSE": "Know-It-All Brothers' House",
    "SCENE_TWINS_HOUSE": "Twins' House",
    "SCENE_MIDOS_HOUSE": "Mido's House",
    "SCENE_SARIAS_HOUSE": "Saria's House",
}

FIRST_SCENE_ID = 0x71  # after MM's SCENE_ALLEY
FIRST_ENTRANCE_SCENE_ID = 0x6E  # after MM's ENTR_SCENE_LAUNDRY_POOL
RESERVED_ENTRANCE_SCENE_ID = 0x7F  # entrances 0xFE00-0xFFFF are Player's "return" exits
FIRST_MAP_SELECT_INDEX = 102  # after MM's scenes in Better Map Select

SCENE_ENTRY = re.compile(r"^/\* (0x[0-9A-Fa-f]+) \*/ DEFINE_SCENE\((\w+),\s*\w+,\s*(\w+)")
ENTRANCE_ENTRY = re.compile(
    r"^/\* (0x[0-9A-Fa-f]+) \*/ DEFINE_ENTRANCE\((\w+), (\w+), (\d+), (true|false), (true|false), "
    r"(TRANS_TYPE_\w+(?:\([^)]*\))?), (TRANS_TYPE_\w+(?:\([^)]*\))?)\)$"
)

# OoT's transition types 0-19 have the same values in MM.
TRANS_TYPES = [
    "TRANS_TYPE_WIPE", "TRANS_TYPE_TRIFORCE", "TRANS_TYPE_FADE_BLACK", "TRANS_TYPE_FADE_WHITE",
    "TRANS_TYPE_FADE_BLACK_FAST", "TRANS_TYPE_FADE_WHITE_FAST", "TRANS_TYPE_FADE_BLACK_SLOW",
    "TRANS_TYPE_FADE_WHITE_SLOW", "TRANS_TYPE_WIPE_FAST", "TRANS_TYPE_FILL_WHITE2", "TRANS_TYPE_FILL_WHITE",
    "TRANS_TYPE_INSTANT", "TRANS_TYPE_FILL_BROWN", "TRANS_TYPE_FADE_WHITE_CS_DELAYED",
    "TRANS_TYPE_SANDSTORM_PERSIST", "TRANS_TYPE_SANDSTORM_END", "TRANS_TYPE_CS_BLACK_FILL",
    "TRANS_TYPE_FADE_WHITE_INSTANT", "TRANS_TYPE_FADE_GREEN", "TRANS_TYPE_FADE_BLUE",
]
# OoT circle (color, speed) -> MM type. Colors: TCC_BLACK, TCC_WHITE, TCC_GRAY; speeds: TCS_FAST, TCS_SLOW.
CIRCLE_STAND_INS = {
    ("TCC_BLACK", "TCS_FAST"): 4,  # TRANS_TYPE_FADE_BLACK_FAST
    ("TCC_BLACK", "TCS_SLOW"): 6,  # TRANS_TYPE_FADE_BLACK_SLOW
    ("TCC_WHITE", "TCS_FAST"): 5,  # TRANS_TYPE_FADE_WHITE_FAST
    ("TCC_WHITE", "TCS_SLOW"): 7,  # TRANS_TYPE_FADE_WHITE_SLOW
}


def trans_type(expr):
    m = re.match(r"TRANS_TYPE_CIRCLE\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)", expr)
    if m:
        return CIRCLE_STAND_INS[(m.group(2), m.group(3))]
    return TRANS_TYPES.index(expr)


def camel(enum):
    return "".join(w.capitalize() for w in enum[len("SCENE_"):].split("_"))


def human(enum):
    return NAMES.get(enum, " ".join(w.capitalize() for w in enum[len("SCENE_"):].split("_")))


def soh_scenes(soh_dir):
    """Maps each OoT scene enum to its file name and OoT id."""
    files = {}
    for line in open(os.path.join(soh_dir, "scene_table.h")):
        m = SCENE_ENTRY.match(line.strip())
        if m:
            files[m.group(3)] = (m.group(2), int(m.group(1), 16))
    return files


def entrance_groups(soh_dir):
    """OoT's entrance table as groups of layers: lists of (index, scene, spawn, flags)."""
    groups, group = [], []
    for raw in open(os.path.join(soh_dir, "entrance_table.h")):
        line = raw.strip()
        m = ENTRANCE_ENTRY.match(line)
        if m:
            continue_bgm, title_card = m.group(5) == "true", m.group(6) == "true"
            flags = ((continue_bgm << 15) | (title_card << 14) | (trans_type(m.group(7)) << 7) |
                     trans_type(m.group(8)))
            group.append((int(m.group(1), 16), m.group(3), int(m.group(4)), flags))
        elif line.startswith("/*") and "DEFINE_ENTRANCE" in line:
            raise SystemExit(f"unparsed entrance: {line}")
        elif not line and group:
            groups.append(group)
            group = []
    if group:
        groups.append(group)
    return groups


def entrance_scene_ids():
    ids, next_id = {}, FIRST_ENTRANCE_SCENE_ID
    for scene in SCENES:
        if next_id == RESERVED_ENTRANCE_SCENE_ID:
            next_id += 1
        ids[scene] = next_id
        next_id += 1
    return ids


GENERATED = "Generated by tools/porting/gen_oot_scenes.py from SoH's {}. Do not edit by hand."


def gen_scene_table(files):
    lines = [
        "/**",
        " * 3Ship [Hyrule] Scenes from Ocarina of Time, appended to MM's (see scene_table.h). Loaded from",
        " * oot.o2r; see hyrule/HyruleScenes.cpp.",
        " *",
        " * " + GENERATED.format("scene_table.h"),
        " */",
    ]
    for i, scene in enumerate(SCENES):
        lines.append(f"// {human(scene)}")
        lines.append(
            f"/* 0x{FIRST_SCENE_ID + i:02X} */ DEFINE_SCENE({files[scene][0]}, {scene}_Oot, 0, SCENE_DRAW_CFG_DEFAULT, "
            f"RESTRICTIONS_NONE, PERSISTENT_CYCLE_FLAGS_NONE, {scene[len('SCENE_'):]}_Oot, "
            f"{FIRST_MAP_SELECT_INDEX + i}, \"{human(scene)} (OoT)\")")
    return "\n".join(lines) + "\n"


def gen_entrance_scene_ids(ids):
    lines = [
        "// 3Ship [Hyrule] Entrance scenes of the OoT scenes, inside the EntranceSceneId enum (z64scene.h).",
        "// " + GENERATED.format("scene_table.h"),
        "// No include guard: this is part of an enum.",
    ]
    for scene, entr_id in ids.items():
        lines.append(f"/* 0x{entr_id:02X} */ ENTR_SCENE_{scene[len('SCENE_'):]}_Oot = 0x{entr_id:02X},")
    return "\n".join(lines) + "\n"


def gen_entrance_table(groups, ids):
    """Entrance tables for z_scene_table.c, and HYRULE_SCENE_ENTRANCES for sSceneEntranceTable."""
    lines = [
        "// 3Ship [Hyrule] Entrance tables of the OoT scenes, included by mm/src/code/z_scene_table.c.",
        "// " + GENERATED.format("entrance_table.h"),
        "// One entrance per OoT entrance group, at the group's spawn; its layers are OoT's scene layers.",
        "// Layers leading to scenes that are not loaded yet use the group's first layer.",
        "",
    ]
    tables = {}
    for scene in SCENES:
        by_spawn = {}
        for group in groups:
            if group[0][1] == scene:
                assert group[0][2] not in by_spawn, f"{scene} has two groups for spawn {group[0][2]}"
                by_spawn[group[0][2]] = group
        assert sorted(by_spawn) == list(range(len(by_spawn))), f"{scene}'s spawns are not contiguous"
        assert len(by_spawn) <= 32 and all(len(g) <= 16 for g in by_spawn.values())
        names = []
        for spawn in range(len(by_spawn)):
            group = by_spawn[spawn]
            name = f"s{camel(scene)}OotEntrance{spawn}"
            names.append(name)
            lines.append(f"static EntranceTableEntry {name}[] = {{")
            for index, target, target_spawn, flags in group:
                if target not in ids:
                    _, target, target_spawn, flags = group[0]
                lines.append(f"    /* 0x{index:03X} */ {{ {target}_Oot, {target_spawn}, 0x{flags:04X} }},")
            lines.append("};")
            lines.append("")
        table = f"s{camel(scene)}OotEntranceTable"
        tables[scene] = (table, len(names))
        lines.append(f"static EntranceTableEntry* {table}[] = {{")
        lines.extend(f"    {n}," for n in names)
        lines.append("};")
        lines.append("")
    lines.append("// The entrance scenes from ENTR_SCENE_" + SCENES[0][len("SCENE_"):] + "_Oot on, in order.")
    lines.append("#define HYRULE_SCENE_ENTRANCES \\")
    entries, expected = [], FIRST_ENTRANCE_SCENE_ID
    for scene, entr_id in ids.items():
        while expected < entr_id:
            entries.append(f"    /* 0x{expected:02X} */ SCENE_ENTRANCE_NONE(),")
            expected += 1
        entries.append(f"    /* 0x{entr_id:02X} */ SCENE_ENTRANCE({tables[scene][0]}, \"{scene}\"),")
        expected += 1
    lines.append(" \\\n".join(entries))
    return "\n".join(lines) + "\n"


def gen_oot_entrance_map(groups, ids):
    lines = [
        "// 3Ship [Hyrule] MM entrance for each OoT entrance index, included by hyrule/HyruleScenes.cpp.",
        "// " + GENERATED.format("entrance_table.h"),
        "// An OoT index past a group's first entry is that layer of the group's MM entrance.",
    ]
    total = sum(len(g) for g in groups)
    by_index = {}
    for group in groups:
        scene, spawn = group[0][1], group[0][2]
        for layer, (index, *_rest) in enumerate(group):
            if scene in ids:
                by_index[index] = f"ENTRANCE_MAKE(ENTR_SCENE_{scene[len('SCENE_'):]}_Oot, {spawn}, {layer})"
            else:
                by_index[index] = "HYRULE_ENTRANCE_NONE"
    assert sorted(by_index) == list(range(total)), "OoT entrance indices are not contiguous"
    lines.extend(f"/* 0x{i:03X} */ {by_index[i]}," for i in range(total))
    return "\n".join(lines) + "\n"


def gen_oot_scene_ids(files):
    lines = [
        "// 3Ship [Hyrule] OoT's scene id for each OoT scene in the scene table, from SCENE_KOKIRI_FOREST_Oot on;",
        "// included by hyrule/HyruleScenes.cpp. " + GENERATED.format("scene_table.h"),
    ]
    lines.extend(f"/* {scene}_Oot */ 0x{files[scene][1]:02X}," for scene in SCENES)
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("soh_tables", help="SoH's soh/include/tables directory")
    args = parser.parse_args()

    files = soh_scenes(args.soh_tables)
    groups = entrance_groups(args.soh_tables)
    ids = entrance_scene_ids()
    outputs = {
        os.path.join("mm", "include", "tables", "scene_table_oot.h"): gen_scene_table(files),
        os.path.join("hyrule", "HyruleEntranceScenes.h"): gen_entrance_scene_ids(ids),
        os.path.join("hyrule", "HyruleEntranceTable.inc"): gen_entrance_table(groups, ids),
        os.path.join("hyrule", "HyruleOotEntrances.inc"): gen_oot_entrance_map(groups, ids),
        os.path.join("hyrule", "HyruleOotSceneIds.inc"): gen_oot_scene_ids(files),
    }
    for path, text in outputs.items():
        with open(os.path.join(ROOT, path), "w") as f:
            f.write(text)
    pending = gen_pending_scenes(args.soh_tables)
    print(f"{len(SCENES)} scenes, {sum(len(g) for g in groups)} OoT entrances: " + ", ".join(outputs) +
          f"; {pending} scenes still pending")


if __name__ == "__main__":
    main()
