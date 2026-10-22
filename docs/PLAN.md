# Plan

This is the staged plan for merging Ocarina of Time into Majora's Mask on the 2Ship engine. Gameplay
decisions are in [DESIGN.md](DESIGN.md).

## Phase 0: foundations

1. Fork 2ship2harkinian, clone it with submodules on branch `phase0`, and build vanilla 2Ship as the
   baseline.
2. Write `docs/DESIGN.md` and `docs/PLAN.md`.
3. Add an empty `hyrule/` source tree wired into CMake. It compiles to nothing yet.
4. Extract OoT assets: extend the ZAPDTR/OTRExporter pipeline to read the OoT ROM into an `oot/` path
   namespace in its own archive, starting with Kokiri Forest and its objects. Namespacing is
   mandatory because 636 paths in `gameplay_keep` alone collide between the games.
5. Write converter skeletons:
   - OoT scene to MM scene format. Most commands map directly. Cutscene data, misc settings and MM's 5
     extra commands need handling.
   - OoT text to MM text control codes.
   - OoT cutscenes to MM, as a stub.
6. Build a differential testing harness:
   - input recording and replay
   - a per-frame state hash covering player position and action, the actor list (id, params,
     position, state) and flags
   - support in both unmodified SoH and this fork
   - a diff tool that reports the first divergent frame
   - a puppet mode where this fork replays SoH's recorded Player state, so actor ports can be tested
     in isolation
7. List the engine functions whose signatures differ between SoH's and 2Ship's headers (about 182 of
   the 1,187 shared names). Start Coccinelle or clang scripts for mechanical translation.

### Exit criteria

- Vanilla MM still builds and runs unchanged.
- Kokiri Forest extracts into `oot/`.
- The harness records a SoH session and replays it with a state-hash log.

## Stage 1: make the engines twins

- Widen MM's engine:
  - 64-bit damage flags and 64-entry damage tables
  - 16-bit item IDs
  - wider entrance encoding
  - unified actor, object and scene tables, with OoT variants suffixed `_Oot`
  - a unified save
- Load Hyrule scenes.
- Port OoT systems.
- Port the 428 OoT actors in waves, each validated by the harness.
- Add adult Link as a 6th Player body.
- Add the OoT interface.

## Stage 2: merge

- One world
- One save
- One inventory
- A unified pause menu

## Stage 3: cross-play

- Transformations work in Hyrule.
- Adult Link and OoT items work in Termina.
