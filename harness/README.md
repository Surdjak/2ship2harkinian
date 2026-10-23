# Differential testing harness

The harness records controller input, replays it, and logs game state every logic frame, in both
unmodified-gameplay SoH and this fork. Comparing two logs shows the first frame where the runs
diverge. It is how OoT ports are checked against SoH (see [docs/PLAN.md](../docs/PLAN.md)).

| Path | Contents |
|------|----------|
| `core/` | Game-independent recorder, replayer, state logger and puppet reader. |
| `2ship/` | Adapter compiled into this fork (`harness/CMakeLists.txt`). |
| `soh/` | Adapter compiled into SoH when it is configured with `-DHARNESS_DIR=<this directory>`. |
| `tools/make_inputs.py` | Builds an input file from a short script, for replays without a recording session. |
| `tools/statediff.py` | Compares two state logs and reports the first divergent frame. |
| `tools/ootlog.py` | Converts a SoH state log to this fork's scene and actor ids, optionally one scene visit. |
| `tools/session.py` | Records and replays sessions together with the saves and settings they started from. |

## Running

The harness is off unless one of these environment variables is set:

| Variable | Effect |
|----------|--------|
| `HARNESS_RECORD=<file>` | Record controller 1, one line per logic frame. |
| `HARNESS_REPLAY=<file>` | Replay a recording. The game exits when the input runs out. |
| `HARNESS_STATELOG=<file>` | Write per-frame state as JSON lines. |
| `HARNESS_PUPPET=<file>` | Drive Player from another run's state log (see below). |
| `HARNESS_SEED=<n>` | RNG seed used instead of the clock, default 0. Record and replay with the same seed. |

### Recording and replaying a session

`tools/session.py` keeps everything a replay needs in one session folder: the input, both state logs,
and a copy of the saves and settings the recording started from. A replay only matches if the game
starts from those same files, so replay sets your current saves and settings aside, puts the
session's copies in place, and restores yours afterwards.

Record on the desktop with the window focused, playing with keyboard or controller 1 (only controller
1 is recorded). Closing the window ends the recording.

```bash
python3 harness/tools/session.py record sessions/kokiri-walk
```

```bash
python3 harness/tools/session.py replay sessions/kokiri-walk
```

Recording uses the SoH build in `../Shipwright/build-cmake/soh` by default; pass
`--game-dir build-cmake/mm` for this fork. `sessions/` is git-ignored.

Replays run under a virtual display, because on a desktop a hidden or unfocused window can be
throttled by the compositor to a few frames per second; `--window` shows the game instead. After the
replay, `statediff.py` compares it with the recording.

If a replay is interrupted, your saves and settings stay in the session's `backup/` folder and the next
replay refuses to run until you have put them back.

Leave the mouse option, network features and simulated input lag off while recording.

### Reading a diff

`statediff.py` prints the first frame where the runs disagree and the fields that differ there:

- **`flags.*` first:** the runs started from different saves or took a different menu path.
- **`rng` first:** something consumed random numbers differently; the actors that differ next show what.
- **`player` or `actors` first:** game logic diverged on that frame.

Later differences usually follow from the first one, so fix the first and compare again.

Use `--align scene` to compare runs whose boot sequences differ, such as SoH against this fork: frames
are then matched by scene and by frames since that scene was entered. `--ignore` leaves out sections
(`rng`, `actors`, ...), single flag blocks (`flags.liveSceneFlags`) or actor fields (`actors.rot`).
`--actors` compares only the listed actor ids, and `--unordered` compares actor lists as sets.

### Checking an OoT actor port against SoH

A SoH log uses OoT's scene and actor ids. `ootlog.py` rewrites them to this fork's unified ids and can
keep a single visit to a scene, so this fork can be started at that visit's entrance with SoH's Player
as its puppet:

```bash
python3 harness/tools/ootlog.py sessions/kokiri-walk/record.state kf.state --scene SCENE_KOKIRI_FOREST_Oot --visit 2
```

```bash
HYRULE_BOOT_ENTRANCE=0xDC30 HARNESS_PUPPET=kf.state HARNESS_STATELOG=ours.state ./2s2h.elf
```

```bash
python3 harness/tools/statediff.py kf.state ours.state --align scene --unordered --actors 0x3D7,0x401,0x403 --ignore rng,player,flags,actors.flags,actors.rot
```

The two Players differ, as do the engines' random numbers, cameras and actor list order, so compare the
ported actors only and leave out rotations that come from random numbers and flags that depend on
the camera. The puppet moves Player but cannot attack, so check attacks with a scripted replay in this
fork (`make_inputs.py`).

The puppet does not copy Player's state flags, so actors that react to Player's state (such as pausing
while Link is in a cutscene) can part from the recording at that point.

A puppet log can also be written by hand to hold Player in one place, such as in front of an NPC,
while `HARNESS_REPLAY` presses buttons: one line per scene frame with `gs`, `scene`, `sf` and a
`player` object (`pos`, `rot`, `speedXZ`, `yaw`).

## State log

Each line is one logic frame:

- **`f`:** the logic frame since boot.
- **`gs`, `scene`, `room`, `sf`:** where the game is, with `sf` counting frames since the scene was entered.
- **`rng`:** the game's PRNG state.
- **`player`:** position, shape rotation, forward speed, yaw, state flags and form (MM transformation; OoT `linkAge`).
- **`actors`:** every actor with category, id, params, room, flags, position and shape rotation.
- **`flagHashes`:** a hash of each save flag block every frame. The block's bytes are written under
  `flags` only on frames where it changes. This fork logs OoT's flags under SoH's names (`eventChkInf`,
  `itemGetInf`, `infTable`, and `eventInfOot` for OoT's `eventInf`), so a converted SoH log can be
  compared flag by flag.
- **`h`:** hashes of the whole frame and of each section, for quick comparison.

## Determinism

While the harness is active, both games:

- seed the RNG with `HARNESS_SEED` instead of the clock, where `Play_Init` reseeds on every scene
  load (and, in 2Ship, when skipping to file select);
- read controller 1 from the recording, right after the pads are read from the device, so gameplay,
  the ocarina and timers all see it.

2Ship also replaces `func_80173B48`, whose wall-clock frame timing changes how many random numbers
player dust effects use, with a fixed value.

Replays of the title sequences and of idle recordings match frame for frame in both games. Known
sources of divergence that remain:

- minigame, postman and bottle timers in MM, which run on `osGetTime()`;
- audio sample counts per frame, which follow the real output buffer, for actors that read sequence
  state;
- SoH enhancements that inject input or randomness (mouse, emulated buttons, network effects,
  randomized enemies). Keep them off.

## Puppet mode

With `HARNESS_PUPPET`, after Player updates on a scene frame that the log also has, Player's position,
rotation, speed and yaw are replaced with the logged ones, and Player's body collider follows them. Actors that update afterwards see the
recorded Player, so a ported actor can be tested against SoH's Player behavior. Collision and
cutscenes later in the frame can still move Player, so the end-of-frame position can differ slightly.
Frames are matched by scene id, so a SoH log needs `ootlog.py` first. When a log visits a scene more
than once, the last visit wins, so keep only the visit to replay.
