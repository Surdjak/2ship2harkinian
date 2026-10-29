# OoT's cutscenes

**Status: approved 2026-10-28 with decisions C1 and C2 (below); steps 1 (the interpreter, camera and warp-song
cutscenes) and 2 (scene cutscenes and the entrance introductions) done.** Asked for by [SONGS.md](SONGS.md) step 4: OoT's warp-song
light is a cutscene, and the fork plays none of OoT's yet.

## Where we start

- **Scene cutscenes** (about 50 in the scenes loaded so far: Kokiri Forest 9, Hyrule Field 8, Lon Lon Ranch 8,
  Kakariko 5, Death Mountain Trail 5, Gerudo Valley 3, Lake Hylia 2, Link's house 2, the Lost Woods 1) go
  through OTRExporter's `OotToMm` converter, which is a stub: each becomes an empty MM cutscene that runs to its
  end frame. Entrance cutscenes (Death Mountain Trail's introduction) do not play (hyrule/README.md).
- **Actor cutscenes** (the warp songs' eight in `Demo_Kankyo`, and most story scenes) are C arrays in the
  actor, built with OoT's cutscene macros, so they are in OoT's format whatever the extractor does.
- OoT and MM script cutscenes differently:
  - **Commands:** different ids and data. OoT gives each actor-action command one of ten fixed slots,
    `npcActions[0-9]`, by command id; MM gives its ten channels to whichever cue types come first, and actors
    look theirs up (`Cutscene_GetCueChannel`).
  - **Link:** OoT's Link action ids are OoT's Player cutscene actions, which MM numbers differently (as
    `Player_SetCsActionWithHaltedActorsOot` already maps for the few actors use).
  - **Camera:** OoT's script camera is spline points through a camera setting (`CAM_SET_CS_0`,
    `Camera_SetCSParams`, OoT's `Camera_Demo1`); MM's is its own cutscene camera with other interpolations.
    MM's `Camera_Demo1` is MM's.

## Proposal

1. **OoT's cutscenes play in OoT's format, with OoT's interpreter.** `HyruleCutscene.c` ports OoT's
   `z_demo.c` for OoT scripts: OoT's commands and timings, its camera splines (OoT's `Camera_Demo1` and spline
   interpolation, driving one of MM's sub-cameras), lighting, music, text and transitions. MM's cutscenes keep
   MM's interpreter; a script is OoT's when an OoT actor starts it or it belongs to an OoT scene. Converting
   OoT's scripts to MM's commands instead would lose OoT's camera and timing wherever MM's commands differ.
2. **Shared state stays MM's** where MM's code reads it: the frame counter and state live in MM's
   `CutsceneContext` (`curFrame`, `state`), and Link follows OoT's Link actions through MM's `playerCue`, with
   OoT's ids mapped to MM's Player cutscene actions. OoT's actor slots are kept by the OoT interpreter, and
   ported OoT actors read them by slot as in OoT (the port tool renames `csCtx.npcActions[n]`, `csCtx.frames`,
   `csCtx.segment` and `gSaveContext.cutsceneTrigger` uses).
3. **Scene scripts stay in OoT's format** in `oot.o2r`: `OotToMm` stops converting them and writes OoT's
   command stream as SoH does, and OoT scenes start them as OoT does (scene layers 4 and up, and OoT's
   entrance cutscene table for first visits).
4. **Order:** the interpreter, camera and Link actions with the warp-song cutscenes first (camera relative to
   Link, misc commands, transition effect), which completes SONGS.md step 4; then scene cutscenes, the
   entrance introductions first; then story cutscenes with their actors (`Demo_*`), each needing its slots and
   Link actions.

## Implementation

1. Done: `HyruleCutscene.c` is OoT's interpreter, generated from SoH's `z_demo.c` by the port tool's renames and
   fix-ups, with OoT's `CutsceneContext` (`gCsCtxOot`) and its own trigger. Point 2 changed in the doing: MM's
   `csCtx.state` and `curFrame` mirror OoT's while a script plays, which is what MM's Player and interface read,
   but Link's actions stay MM's (`playerCue` is left empty) until story cutscenes need OoT's Link actions; the
   warp light needs none. `Play_Update` runs OoT's interpreter instead of MM's while an OoT script plays or is
   triggered. The script camera (OoT's spline interpolation, points absolute or relative to Link) drives MM's
   cutscene sub-camera. OoT's environment flags and the age to load with (`gLinkAgeOnLoadOot`, set by OoT's
   terminators) are here too. Ported actors read OoT's slots (`gCsCtxOot.npcActions`), as the port tool now
   renames; actors ported earlier were moved off MM's `actorCues`.
   `Demo_Kankyo` is ported, and with it the warp-song light (SONGS.md step 4). Checked under gdb: the Minuet and
   the Serenade from Kokiri Forest and from South Clock Town play OoT's warp-out (Link fades into the light, OoT's
   camera circles him), enter the Sacred Forest Meadow and Lake Hylia, and play the warp-in, after which Link is
   drawn and free; the cutscene plays once.
2. Done: OTRExporter writes OoT's scripts in OoT's format (its stub converter is gone, and SoH's writer does it), and
   2Ship's cutscene importer reads `oot/` scripts as they are. `HyruleSceneCutscene.c` starts them as OoT's Play_Init
   does: a cutscene layer (4 and up) plays its scene's script, kept at the head of MM's script list, with the
   cutscene index kept at 0xFFF0 as OoT keeps it; otherwise OoT's entrance cutscene table (by OoT entrance, age and
   event flag) plays its script from `oot.o2r`. C2: the cycle reset keeps the eighteen first-visit introductions'
   flags (`EVENTCHKINF_ENTERED_*`), and clears the rest as before. The cutscene camera now hands back as OoT's does
   (OoT's `func_8005B1A4` is MM's `Camera_SetFinishedFlag`; MM's `func_800E0348`, used before, starves a camera that
   has not run yet of its settings, which an introduction at scene start showed). Checked under gdb with
   screenshots: Death Mountain Trail's, Hyrule Field's, Kakariko's, Lake Hylia's, Gerudo Valley's and Lon Lon
   Ranch's introductions play once on entry, with OoT's camera, and end with Link free on the scene's camera; the
   Deku Tree Sprout's (adult, Kokiri Forest) plays to its first text and stops at frame 80 as OoT's does; Kokiri
   Forest's cutscene layer (forced under gdb) plays OoT's opening narration and its terminator chains into the Deku
   Tree's layer; the cycle reset keeps Death Mountain Trail's flag and clears Mido's. OoT's conditional triggers
   (Saria on the bridge, the Nocturne, the Light Arrows, Ganon's collapse) are story cutscenes (step 3).
3. Next: story cutscenes with their `Demo_*` actors and OoT's Link actions, and OoT's conditional triggers.

## Decisions

| | Question | Decision |
|-|----------|----------|
| C1 | Play OoT's scripts natively, or convert them to MM's commands | natively, with OoT's interpreter |
| C2 | OoT's first-visit introductions (Death Mountain Trail's...) once the cycle resets Hyrule | once per save, though Hyrule's world resets |
