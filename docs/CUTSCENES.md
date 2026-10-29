# OoT's cutscenes

**Status: approved 2026-10-28 with decisions C1 and C2, and N1 on 2026-10-29 (below); steps 1 (the interpreter, camera and warp-song
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
3. Story cutscenes, in progress:
   - Done: Link follows an OoT script's cues with OoT's own Player cutscene actions (`HyrulePlayerCsOot.inc`,
     generated from SoH's `z_player.c` and included by MM's): OoT's cue map, its two action tables and their
     actions, on MM's Player helpers and MM's copies of OoT's Link animations. MM's "follow cues" action
     (`Player_CsAction_48`, OoT's `func_80852C50`) hands over while an OoT script plays. This replaces point 2's
     mapping of OoT's ids to MM's actions, which would have lost the ~60 OoT actions MM's table lacks. MM's camera
     now keeps OoT's letterbox and hides the interface while an OoT script plays, as OoT's camera does, and the
     script's sub-camera takes MM's `CAM_SET_FREE0` (OoT's `CAM_SET_CS_0`), as MM's own scripts do. Checked under
     gdb: Link's house's first cutscene layer plays Link asleep, waking and getting up on Navi's lines.
   - Done: Navi (N1). OoT's `En_Elf` and `Elf_Msg` are ported, and MM's Player spawns OoT's `En_Elf` as Navi in an
     OoT scene while Link has no Tatl (`hasTatl`), in Player's fairy slot (`tatlActor`, OoT's `naviActor`): MM's
     Player talks to her on C-up with its fairy text (`tatlTextId`, OoT's `naviTextId`), and she hovers by MM's
     attention context (`attention.tatlHoverPos`...), which is OoT's targeting context field for field. Her C-up
     quest hint is none, as in SoH, whose scenes load no hint table; her hint timer is not saved yet. What she says
     after Saria's Song is OoT's (`HyruleElfMessage.c`, generated from SoH's `z_elf_message.c`). Checked under gdb:
     in Link's house's cutscene she follows her cues (slot 8) to the window, the bed and Link's side, and in Kokiri
     Forest she follows Link.
   - Done: Kokiri Forest's opening and Saria on the bridge. `Bg_Treemouth` (the Deku Tree's mouth), `Demo_Effect`
     (the cutscenes' lights and jewels, here the Kokiri Emerald) and `Demo_Sa` (Saria in cutscenes) are ported, and
     OoT's conditional triggers (`Hyrule_HandleConditionalTriggersOot`, after MM's in Play_Init, before the layer
     is chosen) start Saria on the bridge (with the Fairy Ocarina), and, once their scenes come, the Requiem, the
     Nocturne, the Light Arrows and Ganon's collapse. The interpreter's music commands now play OoT's sequences
     (`SEQ_OOT`; they played MM's of the same number), and OoT's cutscene effects sequence is
     `Hyrule_PlayCutsceneEffectsSeqOot`. Checked under gdb with screenshots: Kokiri Forest's cutscene layers (forced)
     play the opening (the Deku Tree sends Navi), the Deku Tree's death to the Kokiri Emerald, then Link back in
     Kokiri Forest; entering the bridge plays Saria's farewell (Link takes the ocarina, "You received the Fairy
     Ocarina!"), then Hyrule Field's introduction, then normal play.
   - Next: Tatl taking Navi's cues once she has joined (N1); Sheik's Minuet (Sacred Forest Meadow, `En_Xc`),
     Zelda's escape (Hyrule Field, `Demo_Im`, `En_Viewer`), Kakariko's story layers. The credits' layers (Lon Lon
     Ranch's and others) wait for the ending.

## Decisions

| | Question | Decision |
|-|----------|----------|
| C1 | Play OoT's scripts natively, or convert them to MM's commands | natively, with OoT's interpreter |
| C2 | OoT's first-visit introductions (Death Mountain Trail's...) once the cycle resets Hyrule | once per save, though Hyrule's world resets |
| N1 | Who plays Navi's part (her cues, lines and companion role) in Hyrule | Navi, ported, until Link first enters Termina (where Tatl joins him); then Tatl, in both worlds, takes Navi's later cues and lines |
