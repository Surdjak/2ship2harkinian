#!/usr/bin/env python3
"""Convert a SoH state log to this fork's unified ids, so it can drive and be compared with Hyrule scenes.

Scene ids are matched by scene file name between SoH's and 2Ship's scene tables; scenes this fork does
not have yet are dropped. Actor ids get ACTOR_OOT_BASE (hyrule/HyruleIds.h) added, except the ones the
scene converter maps to MM's own actors (OotToMm::ActorId in OTRExporter). SoH's eventInf flag block
becomes eventInfOot, the name this fork logs OoT's under.

--visit N keeps only the Nth visit (from 1) to --scene, so a puppet or diff uses that visit rather than
whichever one a frame lookup finds first or last.

    python3 harness/tools/ootlog.py record.state kf.state --scene SCENE_KOKIRI_FOREST_Oot --visit 2
"""
import argparse
import json
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")
SCENE = re.compile(r"^/\* (0x[0-9A-Fa-f]+) \*/ DEFINE_SCENE\((\w+),\s*(\w+)")
# OoT actors that the scene converter maps to MM's own: Player, and En_Holl to the room-loading plane.
MM_ACTORS = {0x0000: 0x0000, 0x0023: 0x0018}


def scene_table(path, name_arg):
    """Maps each scene's file name to (id, enum name)."""
    table = {}
    for line in open(path):
        m = SCENE.match(line.strip())
        if m:
            args = re.match(r".*DEFINE_SCENE\(([^)]*)\)", line).group(1).split(",")
            table[m.group(2)] = (int(m.group(1), 16), args[name_arg].strip())
    return table


def actor_base():
    text = open(os.path.join(ROOT, "hyrule", "HyruleIds.h")).read()
    return int(re.search(r"#define ACTOR_OOT_BASE (0x[0-9A-Fa-f]+)", text).group(1), 16)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("soh_log")
    parser.add_argument("output")
    parser.add_argument("--soh-tables", default=os.path.join(ROOT, "..", "Shipwright", "soh", "include", "tables"),
                        help="SoH's soh/include/tables directory")
    parser.add_argument("--scene", help="this fork's scene enum to keep, e.g. SCENE_KOKIRI_FOREST_Oot")
    parser.add_argument("--visit", type=int, help="keep only this visit (from 1) to --scene")
    args = parser.parse_args()
    if args.visit and not args.scene:
        parser.error("--visit needs --scene")

    soh = scene_table(os.path.join(args.soh_tables, "scene_table.h"), 2)
    ours = scene_table(os.path.join(ROOT, "mm", "include", "tables", "scene_table.h"), 1)
    scenes = {soh[name][0]: ours[name] for name in soh if name in ours and ours[name][1].endswith("_Oot")}
    keep = None
    if args.scene:
        keep = next((sid for sid, enum in scenes.values() if enum == args.scene), None)
        if keep is None:
            parser.error(f"{args.scene} is not a scene that both games have")
    base = actor_base()

    visits, previous, written = 0, None, 0
    with open(args.soh_log) as src, open(args.output, "w") as out:
        for line in src:
            if line.startswith("#"):
                out.write(line)
                continue
            if not line.strip():
                continue
            row = json.loads(line)
            if row["gs"] == "play" and row["scene"] in scenes:
                row["scene"] = scenes[row["scene"]][0]
            elif row["gs"] == "play":
                row["scene"] = -1
            if row["scene"] != previous and row["scene"] == keep:
                visits += 1
            previous = row["scene"]
            if keep is not None and (row["scene"] != keep or (args.visit and visits != args.visit)):
                continue
            for actor in row.get("actors", []):
                actor["id"] = MM_ACTORS.get(actor["id"], base + actor["id"])
            # This fork logs OoT's eventInf as eventInfOot, beside MM's own eventInf.
            for block in ("flags", "flagHashes"):
                if "eventInf" in row.get(block, {}):
                    row[block]["eventInfOot"] = row[block].pop("eventInf")
            # The hashes cover the original ids, so they no longer describe this row.
            row.pop("h", None)
            out.write(json.dumps(row, separators=(",", ":")) + "\n")
            written += 1
    print(f"wrote {written} frames to {args.output}")


if __name__ == "__main__":
    main()
