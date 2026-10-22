#!/usr/bin/env python3
"""Build a harness input file from a short script, for replays that need no recording session.

Script lines (one per line, # starts a comment):
  wait N                    N frames with nothing pressed
  press BUTTON[+BUTTON] [N] press for N frames (default 1), then release for 1 frame
  hold BUTTON[+BUTTON] N    press for N frames without the release frame
  stick X Y N               hold the control stick at (X, Y) for N frames

Buttons: A B Z START L R CUP CDOWN CLEFT CRIGHT DUP DDOWN DLEFT DRIGHT
"""
import argparse
import sys

BUTTONS = {
    "CRIGHT": 0x1, "CLEFT": 0x2, "CDOWN": 0x4, "CUP": 0x8, "R": 0x10, "L": 0x20,
    "DRIGHT": 0x100, "DLEFT": 0x200, "DDOWN": 0x400, "DUP": 0x800,
    "START": 0x1000, "Z": 0x2000, "B": 0x4000, "A": 0x8000,
}


def parse_buttons(spec):
    mask = 0
    for name in spec.upper().split("+"):
        if name not in BUTTONS:
            sys.exit(f"unknown button {name}")
        mask |= BUTTONS[name]
    return mask


def build(lines):
    frames = []
    for lineno, raw in enumerate(lines, 1):
        words = raw.split("#", 1)[0].split()
        if not words:
            continue
        cmd, args = words[0].lower(), words[1:]
        try:
            if cmd == "wait":
                frames += [(0, 0, 0)] * int(args[0])
            elif cmd == "press":
                n = int(args[1]) if len(args) > 1 else 1
                frames += [(parse_buttons(args[0]), 0, 0)] * n + [(0, 0, 0)]
            elif cmd == "hold":
                frames += [(parse_buttons(args[0]), 0, 0)] * int(args[1])
            elif cmd == "stick":
                frames += [(0, int(args[0]), int(args[1]))] * int(args[2])
            else:
                sys.exit(f"line {lineno}: unknown command {cmd}")
        except (IndexError, ValueError):
            sys.exit(f"line {lineno}: bad arguments: {raw.strip()}")
    return frames


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("script", help="input script, or - for stdin")
    parser.add_argument("output", help="harness input file to write")
    args = parser.parse_args()

    source = sys.stdin if args.script == "-" else open(args.script)
    frames = build(source.readlines())
    with open(args.output, "w") as out:
        out.write("# harness-inputs 1 game=script seed=0\n")
        out.write("# frame button stickX stickY rightStickX rightStickY gyroX gyroY\n")
        for i, (button, sx, sy) in enumerate(frames):
            out.write(f"{i} {button:x} {sx} {sy} 0 0 0 0\n")
    print(f"wrote {len(frames)} frames to {args.output}")


if __name__ == "__main__":
    main()
