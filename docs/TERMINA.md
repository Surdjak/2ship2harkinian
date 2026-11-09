# From Hyrule to Termina

**Status: decisions T1-T6 taken 2026-11-06 (below); steps 1 to 7 done (hyrule/README.md, "Into Termina").** It follows [DESIGN.md](DESIGN.md) (connection between Hyrule and
Termina, Termina gate, Mask Shop, Time, Epona), which settles the rules; this file is how and in what order they come
into the game, and the few points DESIGN.md leaves open.

## Where we start

- A new save still opens on MM's intro (`ENTRANCE(CUTSCENE, 0)`, MM's sword and shield). Hyrule is reached only
  through the test boot (`HYRULE_BOOT_ENTRANCE`), although OoT's opening (Kokiri Forest's cutscene layer, Navi waking
  Link in his house) already plays when forced.
- OoT's Lost Woods sends Link back to Kokiri Forest on a wrong turn. Its first clearing (room 0), where Link arrives
  from Kokiri Forest, has such a wrong turn straight ahead, and on its left the Skull Kid's clearing (room 9: `En_Skj`
  type 0, whom MM's story is about, with the Saria's Song stump of type 5), a dead end with no exit of its own.
- Hyrule has no save: MM saves only through the Song of Time and owl statues, and 2Ship's pause save reloads in South
  Clock Town.
- OoT's Happy Mask Shop and its four sales (Keaton, Skull, Spooky, Bunny) are ported with vanilla rules; masks are
  owned once borrowed (INVENTORY.md). Nothing gives the Hero's Shield or closes the shop.
- MM's clock runs in Hyrule as in Termina: OoT's day and night are MM's `dayTime`.
- Epona's Song calls Epona in both worlds from the start (EPONA.md point 4 is not done).

## Proposal

1. **The opening.** A new save always starts as OoT's (T3): in Link's house, Kokiri Forest's opening (the Deku Tree sends
   Navi), then Navi waking Link. Link owns nothing: no sword, no shield, no ocarina (MM's default sword and shield
   go), and the first cycle has not begun (T3).
2. **OoT's time until Clock Town.** Until Link first enters Clock Town, the clock keeps OoT's rules: day follows
   night without a day count, no moon, no end of the third day. A permanent flag set on the first entry to Clock
   Town starts MM's three days, everywhere (DESIGN.md: the moon crash happens in Hyrule too). The pause menu's B saves
   in Hyrule, as in OoT, and the file reloads where OoT's would (T4).
3. **The Mask Shop and the gate.** After the 4th sale, the salesman gives the Goron Mask, Zora Mask, Gerudo Mask and
   Mask of Truth. If Link already owns the Ocarina of Time, the Kokiri Sword and Epona's Song, he also gives the
   Hero's Shield, sets the permanent **Termina unlocked** flag, and leaves for Termina: the shop closes for good.
   If Link lacks one, the salesman waits for him in the shop and gives the shield when he next talks to him with
   all three.
4. **The passage.** Once Termina is unlocked, the first clearing's straight-ahead wrong turn, beside the Skull Kid's
   clearing, leads to MM's Lost Woods (Intro) instead of Kokiri Forest, and MM's intro plays: Link on Epona, the Skull Kid
   steals Epona and the Ocarina of Time, the fall, the Deku curse, Tatl, the Clock Tower and the salesman, then
   Clock Town on the first day (step 2's flag). Navi stays in Hyrule (T1). Before the gate opens, the exit loops as in
   OoT.
5. **Epona's theft.** From the theft until Link recovers Epona at Romani Ranch, Epona's Song calls nothing in
   either world (DESIGN.md, EPONA.md point 4).
6. **The way back.** Once the Deku curse is lifted (the Song of Healing), Link walks back from the Clock Tower
   interior's back door through the opening dungeon's corridor to its landing spot (the big flower he fell onto),
   now a gold Deku Flower whose launch carries him back up the shaft to MM's Lost Woods, and out by the clearing's far
   side, where he woke up, into OoT's Lost Woods' first clearing (T5). From there the same exit leads into Termina the
   same way, walked without the intro's cutscenes, at either age (T6). Until the curse is lifted, the flower stays
   pink and that exit stays shut.
7. **The cycle.** From then on the Song of Time resets both worlds (already done, DESIGN.md "Cycle reset"), and the
   moon falls in Hyrule as in Termina.

Steps 1 to 5 make the game playable from OoT's opening into Termina; 6 and 7 close the loop.

## Decisions

| | Question | Decision |
|-|----------|----------|
| T1 | Navi at the passage | each world keeps its fairy: Navi is Link's fairy in every Hyrule scene, Tatl in every Termina scene; Navi takes her own cutscene cues in Hyrule (this replaces CUTSCENES.md N1's "Tatl takes Navi's cues") |
| T2 | Who takes the first passage | child Link only, as MM's intro is child Link's; for adult Link the exit loops until the way back is open (step 6), then he crosses too |
| T3 | Where a new save starts | always OoT's opening |
| T4 | Saving in Hyrule once MM's three days run everywhere | OoT's pause save stays a permanent save all game, at the time of the cycle it was made; from 2026-11-09 it works in Termina too once Link has reached Clock Town, reloading by OoT's rule (a temple at its entrance, elsewhere South Clock Town) |
| T5 | The way back, since Link cannot climb out of the opening dungeon's fall | through the opening dungeon: its landing spot carries Link up to MM's Lost Woods |
| T6 | Later trips from Hyrule into Termina | through MM's intro areas, walked without their cutscenes, at either age |
