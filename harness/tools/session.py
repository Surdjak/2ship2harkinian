#!/usr/bin/env python3
"""Record and replay harness sessions with the saves and settings they started from.

A replay only matches its recording if the game starts from the same save files and settings, so a
session folder keeps a copy of both from the moment recording began:

  session.py record SESSION [--game-dir DIR]   play normally; close the window to stop
  session.py replay SESSION [--game-dir DIR]   replay headless and compare with the recording

Replay sets the game's current saves and settings aside, puts the session's copies in place, runs the
replay, and then puts the originals back, so playing and replaying never disturb each other.

Session folder:
  session.json    game, seed and when it was recorded
  start/          saves and settings as they were when recording started
  inputs          recorded controller input
  record.state    state log written while recording
  replay.state    state log of the last replay, and replay.log its game output
"""
import argparse
import datetime
import json
import os
import shutil
import signal
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_GAME_DIRS = {
    "soh": os.path.join(HERE, "..", "..", "..", "Shipwright", "build-cmake", "soh"),
    "2ship": os.path.join(HERE, "..", "..", "build-cmake", "mm"),
}
GAMES = {
    "soh": {"exe": "soh.elf", "saves": "Save", "settings": "shipofharkinian.json"},
    "2ship": {"exe": "2s2h.elf", "saves": "saves", "settings": "2ship2harkinian.json"},
}


def detect_game(game_dir):
    for name, game in GAMES.items():
        if os.path.isfile(os.path.join(game_dir, game["exe"])):
            return name
    sys.exit(f"no soh.elf or 2s2h.elf in {game_dir}")


def copy_profile(game, src_dir, dst_dir):
    """Copies the saves folder and settings file from src_dir to dst_dir, removing what src lacks."""
    for item in (game["saves"], game["settings"]):
        src, dst = os.path.join(src_dir, item), os.path.join(dst_dir, item)
        if os.path.isdir(dst):
            shutil.rmtree(dst)
        elif os.path.exists(dst):
            os.remove(dst)
        if os.path.isdir(src):
            shutil.copytree(src, dst)
        elif os.path.exists(src):
            shutil.copy2(src, dst)


def run_game(game_dir, game, env_vars, headless, log=None, timeout=None):
    """Runs the game and returns its exit status, killing it and xvfb-run together on timeout."""
    env = dict(os.environ, **env_vars)
    cmd = [os.path.join(".", game["exe"])]
    if headless:
        cmd = ["xvfb-run", "-a", "-s", "-screen 0 1280x720x24"] + cmd
    proc = subprocess.Popen(cmd, cwd=game_dir, env=env, stdout=log, stderr=subprocess.STDOUT,
                            start_new_session=True)
    try:
        return proc.wait(timeout=timeout)
    except (subprocess.TimeoutExpired, KeyboardInterrupt):
        os.killpg(proc.pid, signal.SIGKILL)
        proc.wait()
        raise


def record(args):
    session = os.path.abspath(args.session)
    if os.path.exists(os.path.join(session, "inputs")):
        sys.exit(f"{session} already holds a recording")
    game_dir = os.path.abspath(args.game_dir)
    name = detect_game(game_dir)
    game = GAMES[name]

    os.makedirs(os.path.join(session, "start"), exist_ok=True)
    copy_profile(game, game_dir, os.path.join(session, "start"))
    with open(os.path.join(session, "session.json"), "w") as f:
        json.dump({"game": name, "seed": args.seed, "recorded": datetime.datetime.now().isoformat(timespec="seconds"),
                   "game_dir": game_dir}, f, indent=1)

    print(f"Recording {name}. Play, then close the game window to stop.", flush=True)
    run_game(game_dir, game, {
        "HARNESS_RECORD": os.path.join(session, "inputs"),
        "HARNESS_STATELOG": os.path.join(session, "record.state"),
        "HARNESS_SEED": str(args.seed),
    }, headless=False)
    print(f"Recorded to {session}")


def replay(args):
    session = os.path.abspath(args.session)
    with open(os.path.join(session, "session.json")) as f:
        meta = json.load(f)
    game_dir = os.path.abspath(args.game_dir or meta["game_dir"])
    name = detect_game(game_dir)
    if name != meta["game"]:
        sys.exit(f"session was recorded in {meta['game']}, but {game_dir} holds {name}")
    game = GAMES[name]

    backup = os.path.join(session, "backup")
    if os.path.exists(backup):
        sys.exit(f"{backup} exists: an earlier replay was interrupted before it put your saves and settings "
                 f"back. Copy them from there into {game_dir}, then delete it.")
    os.makedirs(backup)
    copy_profile(game, game_dir, backup)
    try:
        copy_profile(game, os.path.join(session, "start"), game_dir)
        with open(os.path.join(session, "replay.log"), "w") as log:
            env = {
                "HARNESS_REPLAY": os.path.join(session, "inputs"),
                "HARNESS_STATELOG": os.path.join(session, "replay.state"),
                "HARNESS_SEED": str(meta["seed"]),
            }
            status = run_game(game_dir, game, env, headless=not args.window, log=log, timeout=args.timeout)
        if status != 0:
            print(f"game exited with status {status}; see {session}/replay.log")
    finally:
        copy_profile(game, backup, game_dir)
        shutil.rmtree(backup)

    return subprocess.run([sys.executable, os.path.join(HERE, "statediff.py"), os.path.join(session, "record.state"),
                           os.path.join(session, "replay.state")]).returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)

    rec = sub.add_parser("record", help="play and record a session")
    rec.add_argument("session")
    rec.add_argument("--game-dir", default=DEFAULT_GAME_DIRS["soh"], help="folder holding the game (default: SoH)")
    rec.add_argument("--seed", type=int, default=0)

    rep = sub.add_parser("replay", help="replay a session and compare it with the recording")
    rep.add_argument("session")
    rep.add_argument("--game-dir", help="folder holding the game (default: where it was recorded)")
    rep.add_argument("--window", action="store_true", help="show the game instead of running headless")
    rep.add_argument("--timeout", type=int, help="seconds before the replay is stopped")

    args = parser.parse_args()
    sys.exit(record(args) if args.command == "record" else replay(args))


if __name__ == "__main__":
    main()
