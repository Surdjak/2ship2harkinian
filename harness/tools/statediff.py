#!/usr/bin/env python3
"""Compare two harness state logs and report the first frame where they diverge.

Frames are matched by logic frame (--align frame, the default, for two runs of the same game) or by
scene and frames since the scene was entered (--align scene, for SoH against our fork, whose boot
sequences differ). Run tools/ootlog.py on a SoH log first to give it this fork's ids. Exit status is 0
when no compared frame differs and 1 otherwise.
"""
import argparse
import json
import sys

SECTIONS = ("gs", "scene", "room", "rng", "player", "actors", "flags")


def load(path):
    """Yields frames with flag blocks expanded, since logs only carry a block's data when it changes."""
    flags = {}
    with open(path) as f:
        for line in f:
            if line.startswith("#") or not line.strip():
                continue
            row = json.loads(line)
            flags.update(row.get("flags", {}))
            row["flags"] = {name: flags.get(name) for name in row.get("flagHashes", {})}
            yield row


def key(row, align):
    if align == "frame":
        return row["f"]
    if row["gs"] != "play":
        return None
    return (row["scene"], row["sf"])


def diff_values(a, b, path, out, limit):
    if len(out) >= limit:
        return
    if isinstance(a, dict) and isinstance(b, dict):
        for k in sorted(set(a) | set(b)):
            diff_values(a.get(k), b.get(k), f"{path}.{k}", out, limit)
    elif isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b):
            out.append(f"{path}: length {len(a)} != {len(b)}")
        for i, (x, y) in enumerate(zip(a, b)):
            diff_values(x, y, f"{path}[{i}]", out, limit)
    elif a != b:
        out.append(f"{path}: {a!r} != {b!r}")


def diff_flags(a, b, out, limit):
    for name in sorted(set(a) | set(b)):
        x, y = a.get(name), b.get(name)
        if x == y:
            continue
        if x is None or y is None or len(x) != len(y):
            out.append(f"flags.{name}: present or size differs")
            continue
        xb, yb = bytes.fromhex(x), bytes.fromhex(y)
        offsets = [i for i in range(len(xb)) if xb[i] != yb[i]]
        shown = ", ".join(f"0x{i:X}: {xb[i]:02x} != {yb[i]:02x}" for i in offsets[:8])
        more = f" (+{len(offsets) - 8} more)" if len(offsets) > 8 else ""
        out.append(f"flags.{name}: {len(offsets)} bytes differ: {shown}{more}")
        if len(out) >= limit:
            return


def actor_view(actors, ids, ignore, ordered):
    """Keeps the actors whose id is in ids (all when ids is empty), without ignored fields."""
    fields = {s.split(".", 1)[1] for s in ignore if s.startswith("actors.")}
    kept = [{k: v for k, v in actor.items() if k not in fields} for actor in actors or []
            if not ids or actor["id"] in ids]
    return kept if ordered else sorted(kept, key=lambda actor: json.dumps(actor, sort_keys=True))


def compare(a, b, ignore, ids, ordered, limit):
    diffs = []
    for section in SECTIONS:
        if section in ignore:
            continue
        if section == "actors":
            diff_values(actor_view(a.get("actors"), ids, ignore, ordered),
                        actor_view(b.get("actors"), ids, ignore, ordered), "actors", diffs, limit)
        elif section == "flags":
            diff_flags({k: v for k, v in a["flags"].items() if f"flags.{k}" not in ignore},
                       {k: v for k, v in b["flags"].items() if f"flags.{k}" not in ignore}, diffs, limit)
        else:
            diff_values(a.get(section), b.get(section), section, diffs, limit)
    return diffs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("a")
    parser.add_argument("b")
    parser.add_argument("--align", choices=("frame", "scene"), default="frame")
    parser.add_argument("--ignore", default="",
                        help="comma-separated sections (gs, scene, room, rng, player, actors, flags), "
                             "flag blocks (flags.NAME) or actor fields (actors.FIELD) to leave out")
    parser.add_argument("--actors", default="",
                        help="comma-separated actor ids (decimal or 0x hex) to compare, leaving out the others")
    parser.add_argument("--unordered", action="store_true",
                        help="compare actor lists as sets; OoT and MM order actors of a category differently")
    parser.add_argument("--limit", type=int, default=20, help="maximum field differences to print")
    args = parser.parse_args()
    ignore = {s.strip() for s in args.ignore.split(",") if s.strip()}
    ids = {int(s, 0) for s in args.actors.split(",") if s.strip()}

    b_rows = {}
    for row in load(args.b):
        k = key(row, args.align)
        if k is not None:
            b_rows.setdefault(k, row)

    compared = 0
    for row in load(args.a):
        k = key(row, args.align)
        other = b_rows.get(k) if k is not None else None
        if other is None:
            continue
        compared += 1
        diffs = compare(row, other, ignore, ids, not args.unordered, args.limit)
        if diffs:
            where = f"frame {k}" if args.align == "frame" else f"scene {k[0]} scene frame {k[1]}"
            print(f"first divergence at {where} (a: frame {row['f']}, b: frame {other['f']})")
            for d in diffs:
                print(f"  {d}")
            return 1

    if compared == 0:
        print("no frames in common")
        return 1
    print(f"{compared} frames compared, no divergence")
    return 0


if __name__ == "__main__":
    sys.exit(main())
