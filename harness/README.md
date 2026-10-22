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

## Running

The harness is off unless one of these environment variables is set:

| Variable | Effect |
|----------|--------|
| `HARNESS_RECORD=<file>` | Record controller 1, one line per logic frame. |
| `HARNESS_REPLAY=<file>` | Replay a recording. The game exits when the input runs out. |
| `HARNESS_STATELOG=<file>` | Write per-frame state as JSON lines. |
| `HARNESS_PUPPET=<file>` | Drive Player from another run's state log (see below). |
| `HARNESS_SEED=<n>` | RNG seed used instead of the clock, default 0. Record and replay with the same seed. |

Run replays under a virtual display. On a desktop, a window that is hidden or unfocused can be
throttled by the compositor, which slows a run to a few frames per second:

```bash
HARNESS_REPLAY=session.inputs HARNESS_STATELOG=replay.state xvfb-run -a -s "-screen 0 1280x720x24" ./2s2h.elf
```

```bash
python3 harness/tools/statediff.py record.state replay.state
```

Use `--align scene` to compare runs whose boot sequences differ, such as SoH against this fork: frames
are then matched by scene and by frames since that scene was entered. `--ignore` leaves out sections
(`rng`, `actors`, ...) or single flag blocks (`flags.liveSceneFlags`).

## State log

Each line is one logic frame:

- **`f`:** the logic frame since boot.
- **`gs`, `scene`, `room`, `sf`:** where the game is, with `sf` counting frames since the scene was entered.
- **`rng`:** the game's PRNG state.
- **`player`:** position, shape rotation, forward speed, yaw, state flags and form (MM transformation; OoT `linkAge`).
- **`actors`:** every actor with category, id, params, room, flags, position and shape rotation.
- **`flagHashes`:** a hash of each save flag block every frame. The block's bytes are written under
  `flags` only on frames where it changes.
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
rotation, speed and yaw are replaced with the logged ones. Actors that update afterwards see the
recorded Player, so a ported actor can be tested against SoH's Player behavior. Collision and
cutscenes later in the frame can still move Player, so the end-of-frame position can differ slightly.
Frames are matched by scene id, so SoH logs only drive this fork once Hyrule scenes load here.
