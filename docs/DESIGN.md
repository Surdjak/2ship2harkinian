# Design: Ocarina of Time + Majora's Mask as one game

This fork of 2 Ship 2 Harkinian (2Ship) merges Ocarina of Time (OoT) into Majora's Mask (MM) as a
single game: one world, one save, one inventory. 2Ship is the engine. Ship of Harkinian (SoH) is the
unmodified OoT reference used for porting and differential testing.

Hyrule refers to the OoT world and Termina to the MM world. See [PLAN.md](PLAN.md) for staging.

## Player bodies

Link has six bodies:

| Body | Source |
|------|--------|
| Child (human) | MM human Link |
| Adult | OoT's adult `PlayerAgeProperties` row |
| Deku | MM |
| Goron | MM |
| Zora | MM |
| Fierce Deity | MM |

- Biggoron's Sword uses MM's `PLAYER_IA_SWORD_TWO_HANDED` path. The sword beam stays exclusive to
  Fierce Deity.

## Movement

- There is one movement set. OoT child physics and MM human physics are identical.
- Crawlspaces are child Link only.
- Crouch stab is patched.
- The Silver and Golden Scales work for human Link in both regions.

## Connection between Hyrule and Termina

- A looping exit in OoT's Lost Woods (the Skull Kid clearing) leads to MM's Lost Woods (Intro), then
  Before Clock Town, then the Clock Tower interior.
- The route is made two-way, with a free-roam variant of the intro areas.
- The way back to Hyrule stays blocked until the Deku curse is lifted.

## Termina gate

Termina opens after Link owns, in this order:

1. Ocarina of Time
2. Kokiri Sword
3. Epona's Song
4. Hero's Shield, which is always obtained last

Handing over the Hero's Shield sets a permanent **Termina unlocked** flag. The exit checks only that
flag.

## Mask Shop

- Borrowed masks become owned and are kept, even after they are sold.
- After the 4th sale, the salesman gives the Goron Mask, Zora Mask, Gerudo Mask and Mask of Truth.
- If the other three gate prerequisites (Ocarina of Time, Kokiri Sword, Epona's Song) are already
  owned, he also gives the Hero's Shield and leaves for Termina. The shop then closes for good.
- OoT masks live in the OoT trade slot and reuse SoH's Mask Select cycling.
- OoT and MM masks are separate items, each keeping its home-game behavior. The OoT Bunny Hood is
  cosmetic. The MM Bunny Hood gives the 1.5x speed boost.

## Time

- OoT time rules apply until Link first enters Clock Town.
- From then on, the MM clock runs everywhere at Termina speed, including the day counter.
- The moon crash happens in Hyrule too.

## Songs

| Song | Function |
|------|----------|
| Song of Time | Cycle reset only. Learned in MM's night-3 flashback. |
| Song of Double Time | Performs OoT's Song of Time functions. Taught by Zelda, with its own learned flag. |
| Sun's Song | Performs Song of Double Time's job. |
| Inverted Song of Time | Unchanged. |

## Cycle reset

- Hyrule resets completely.
- Link wakes in Clock Town at dawn of day 1, with his age unchanged.
- Unique rewards that were already claimed give filler instead.
- The Gold Skulltula token count is permanent.
- World states keyed to medallions and spiritual stones (19 actor types) move to per-cycle flags.
- These never reset:
  - the rainbow bridge, which stays keyed to medallions
  - the Mask Shop closure
  - the Termina unlocked flag

## Epona

- Child Link can ride Epona once he knows Epona's Song. The song works in both eras.
- Epona is stolen in MM's intro, so she is unavailable everywhere until she is recovered at Romani
  Ranch.
- Ingo's race is reworked around freeing the ranch and Talon's return.

## Shields

- The shields are Deku, Hylian, Hero's, OoT Mirror and MM Mirror, stored as OoT-style ownership
  bits.
- Adult Link can use the Hero's Shield.
- The equipment page shows only what the current body can equip.
