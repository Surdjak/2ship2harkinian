# OoT's items on MM's Player

**Status: Slingshot, Boomerang, spells, OoT's masks and child trade items done (2026-10-26, decisions below).** It follows [PAUSE.md](PAUSE.md): OoT's items can go
on C-buttons and the D-pad, but MM's Player does nothing with them yet (`Player_ItemToItemAction` maps
every id from `ITEM_FD` up to `PLAYER_IA_NONE`). This adds them one by one, the Slingshot first.

## Approach for every OoT item

1. **One item action per OoT item**, appended after MM's (`PLAYER_IA_SLINGSHOT_Oot` = 0x53, then
   `PLAYER_IA_MAX` = 0x54...). MM's Player is mostly driven by tables indexed by item action (model group,
   upper-body action, init function, get-item id); each new action takes the entries of the MM action it
   behaves like, so MM's code does the work. Appending keeps MM's ranges (bows, explosives, bottles,
   masks) unchanged.
2. **`Player_ItemToItemAction` maps OoT ids** (`ITEM_*_Oot`) to those actions; OoT items still map to
   `PLAYER_IA_NONE` until their turn.
3. **Projectiles and effects reuse MM's actors where MM kept OoT's code** (`En_Arrow` still has OoT's seed;
   `En_Boom` is OoT's boomerang turned Zora's), and are ported from OoT where it did not (Din's Fire,
   Farore's Wind, Nayru's Love).
4. **Models come from OoT's child Link** (`object_link_child`), drawn on MM's human Link as the Deku Shield
   already is (`HyrulePlayer.c`).
5. **Ammo** lives where INVENTORY.md put it; MM's ammo calls (`AMMO`, `Inventory_ChangeAmmo`) learn OoT's
   ids, as the HUD and the item page already have.
6. **Bodies:** human Link only (DESIGN.md: OoT's items are child Link's); adult Link's items come with the
   adult body.

## The Slingshot

OoT's Slingshot is its Bow with seeds: the same item action functions, first-person aiming and Z-targeted
shots, with `ARROW_SEED` and child Link's own hand and string. MM's Player and `En_Arrow` kept most of that:

| Part | MM today | Change |
|------|----------|--------|
| Item action | none | `PLAYER_IA_SLINGSHOT_Oot`, with the Bow's model group, upper-body action and init |
| Projectile | `En_Arrow`'s `ARROW_TYPE_SLINGSHOT` (OoT's seed, its sound and its sparkle), reached only from an unused Deku Nut path | Player fires it for the Slingshot (`func_808305BC`) |
| Ammo | `AMMO(ITEM_BOW)` | Deku Seeds (`AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT)`): checked before shooting, spent by `Inventory_ChangeAmmo`, refilled up to the Bullet Bag |
| Damage | `ARROW_TYPE_SLINGSHOT` hits as MM's Deku Nut, which stuns | OoT's slingshot on OoT's actors, and MM's Deku bubble on MM's, as DMG_FROM_OOT already has it: damage, no stun (S1) |
| Model | MM's bow hand | OoT child Link's right hand holding the Slingshot, and its string, which stretches as Link pulls (OoT's `sBowStringData` for the child) |
| Seeds to pick up | OoT's seed drops become nothing (`Item00_FromOot`) | a seed collectible in MM's `En_Item00` (OoT's seed sprite, 5 seeds); one drop rule for Termina and Hyrule: arrows or seeds by the ammo Link carries (S3) |

Not changed: aiming, the reticle, the camera and Z-targeting are MM's Bow's, which are OoT's; shooting
galleries and other minigames keep the Bow.

### Checks

- Fire seeds with the Slingshot on C and on B-free Z-targeting; ammo goes down, the HUD and item page
  follow, an empty bag refuses as the Bow does.
- Seeds hit OoT's actors ported so far (Gold Skulltulas, Deku Babas when ported) as in SoH, and MM's
  enemies in Termina per S1; MM's Bow is unchanged (Termina combat replay).
- Seed drops appear, in Termina and Hyrule, only with the Slingshot, and refill it; arrows still drop with the Bow.

## Implementation

- **Player** (`z_player.c`, `z_player_lib.c`): `PLAYER_IA_SLINGSHOT_Oot` (the first of `PLAYER_IA_OOT_MIN`)
  takes the Bow's model group, upper-body action and init. Player's button code read every id from `ITEM_FD`
  up as "no item"; it now uses `ITEM_IS_NONE`, which leaves OoT's ids out. OoT's items Player does not use
  yet play the error sound, and none can be shown to anyone yet (no get-item model for them in MM's table).
- **Firing** (`func_808305BC`): the Slingshot fires `ARROW_TYPE_SLINGSHOT` with the Deku Seeds
  (`Inventory_ChangeAmmo(ITEM_SLINGSHOT_Oot, ...)`), with OoT's draw and flick sounds.
- **Models** (`hyrule/HyrulePlayer.c`): while Link holds or takes out the Slingshot, his right hand, his
  first-person arm and the bow string are OoT child Link's, the string placed and turned as OoT does.
- **Damage** (`sArrowDmgFlags`, now u64): the seed is `DMG_DEKU_BUBBLE | DMG_SLINGSHOT_Oot`. MM's tables give
  the Deku bubble damage and no stun, except Odolwa and his bugs, which MM stuns with it. MM picks a table
  entry from the attack's highest bit, which for an attack with OoT's bits is in the empty OoT half of MM's
  tables; MM's tables now read MM's bits only (`CollisionCheck_GetDamageAndEffectOnElementAC`), without which
  seeds did nothing to MM's creatures.
- **Drops** (`z_en_item00.c`, `hyrule/HyruleActor.c`): `ITEM00_DEKU_SEEDS_Oot` (OoT's sprite, 5 seeds), also
  for OoT's room collectibles (OTRExporter's `OotToMm`). Hyrule's actors already roll MM's drop tables; now
  every arrow or seed drop, in either world, becomes seeds with the Slingshot only, arrows with the Bow only,
  and with both the emptier one (`Item00_ArrowsOrSeeds`). A flexible drop gives seeds when the Bow is not
  low and the seeds are.

## The Boomerang

MM's Player still has OoT's boomerang item action (`PLAYER_IA_11`, now `PLAYER_IA_BOOMERANG_Oot`), with OoT's
model group, upper-body actions and item change; MM uses those upper-body actions for Zora Link's fins.

- **Player** (`z_player.c`): `ITEM_BOOMERANG_Oot` maps to it. The aim, throw and catch are MM's Zora fin
  steps (`Player_UpperAction_11` to `16`), which are OoT's, with OoT's animations
  (`link_boom_throw_wait2waitR`, `throwR`/`throwL` by the side Link leans to, `catch`) and one boomerang thrown
  as OoT's Player throws it (`Hyrule_ThrowBoomerang`). Aiming uses MM's Zora fin camera, OoT's boomerang camera.
- **The boomerang** (`hyrule/overlays/actors/ovl_En_Boom`): OoT's `En_Boom`, ported (`En_Boom_Oot`). It homes
  on the lock-on target, bounces off walls, and brings back collectibles (MM's `En_Item00`), and Gold Skulltula
  tokens once their actor is ported; as in OoT, a fetched item falls where the boomerang is caught, near Link. Player's Zora boomerang
  flags and pointer stand in for OoT's. OoT's pass-through walls in Jabu-Jabu come with Jabu-Jabu.
- **Model** (`z_player_lib.c`): child Link's left fist holding the Boomerang, open while it is out.
- **On OoT's creatures**, OoT's damage tables apply, as in OoT. **On MM's** (`DamageTable_GetOotBoomerangAttack`),
  it does what OoT's boomerang does to that creature in OoT (B1): it stuns, through the creature's own Deku
  Nut entry; it deals damage, through its Zora fin entry, to those OoT's boomerang damages (Guay, Deku Baba,
  Wilted Deku Baba, Mad Scrub, Keese, Octorok, Skullwalltula and Gold Skulltula, Big Poe, Leever); and it does
  nothing to those it does nothing to in OoT (Red Bubble, Freezard, ReDead and Gibdos, Shellblade, the Poe
  Sisters, Iron Knuckle). MM-only creatures are stunned.

## The spells

Din's Fire, Farore's Wind and Nayru's Love are OoT's, for human Link, with OoT's magic costs (12, 12, 24).
MM's Player kept OoT's spell animations; the cast is OoT's, with MM's item camera.

- **Player** (`z_player.c`): `PLAYER_IA_FARORES_WIND_Oot`, `PLAYER_IA_NAYRUS_LOVE_Oot` and `PLAYER_IA_DINS_FIRE_Oot`.
  Using one checks the scene and the magic, then OoT's cast (`Player_Action_CastSpell_Oot`): the charge, the
  spell's actor and animation, and the magic spent.
- **The actors** (`hyrule/overlays/actors`): OoT's `Magic_Fire` (Din's Fire: the fire dome, its damage and the
  screen tint, across the whole widescreen), `Magic_Dark` (Nayru's Love: the orb, the dimmed scene, and the
  diamond that keeps damage off Link for 1200 frames, across scenes, through MM's negative
  `invincibilityTimer`, which works as OoT's) and `Magic_Wind` (Farore's Wind's wind).
- **Farore's Wind** (`hyrule/HyruleMagic.c`): the point is Link's room entry point (his void-out respawn), as
  in OoT, saved in `SaveInfo.faroresWindOot` (save version 14) and shown by OoT's floating pointer. Casting
  again offers OoT's "Return to the Warp Point / Dispel the Warp Point / Exit"; returning warps there, and
  arriving uses the point up. OoT keeps the point in `respawn[RESPAWN_MODE_TOP]`, which MM uses for itself on
  every scene entry, so the pointer has its own copy and the point goes into `RESPAWN_MODE_TOP` only for the warp.
  The point is erased at the end of a cycle (F2).
- **Where** (`Hyrule_IsSpellAllowed`): in Hyrule, OoT's own table (`HyruleRestrictions.c`; Farore's Wind in
  eleven dungeons, not their boss rooms; Din's Fire and Nayru's Love not in towns, houses and shops). In Termina,
  Farore's Wind in the four temples (F1), and Din's Fire and Nayru's Love outside Clock Town (F3); MM greys every
  OoT item in houses and shops already. A forbidden spell's button is greyed.
- **OoT's text**: OoT's message positions went into the flags half of MM's message header, which made every
  message placed at the top, middle or bottom unskippable, and the bottom ones invisible. OTRExporter's
  converter now puts them where MM reads its own (`OotToMm::ConvertMessage`).

## OoT's masks and the child trade slot

The child trade slot holds the Weird Egg, the Cucco or Zelda's Letter, then OoT's masks, which A cycles on the
item page (PAUSE.md). OoT's and MM's masks stay separate items, each behaving as in its own game (DESIGN.md).

- **Masks** (`hyrule/HyrulePlayer.c`): `PLAYER_IA_MASK_KEATON_Oot` to `PLAYER_IA_MASK_TRUTH_Oot`. Using one puts it on
  at once, or takes it off, as in OoT; it is drawn from OoT child Link's model on MM's Link's head, the Bunny Hood
  with OoT's swinging ears (cosmetic, DESIGN.md). One face at a time: an OoT mask takes MM's off, and MM's takes
  the OoT one off. It comes off when its item leaves the buttons or Link changes body, as in OoT, and stays on
  across scenes and saves, as MM's do (M1; `Save.equippedMaskOot`, save version 15). OoT's actors ask
  `Player_GetMaskOot` (OoT's `Player_GetMask`); each reacts to masks as it is ported.
- **Trade items**: `PLAYER_IA_WEIRD_EGG_Oot`, `PLAYER_IA_CHICKEN_Oot` and `PLAYER_IA_LETTER_ZELDA_Oot` are shown as MM
  shows its trade items: Link holds the item up (the Cucco cries, as in OoT), and with no one to take it, MM's
  "This item doesn't seem to work here". Talon, Malon, the castle guard and Zelda take them as they are ported.
- **The Weird Egg** hatches into the Cucco at the next dawn (MM's new day, or a song's), with OoT's message.
- **OoT's get-item table and models** (M2): OoT's get-item ids (`GI_*_Oot`, from `GI_OOT_BASE` 0x100) and draw ids
  (`GID_*_Oot`, from `GID_OOT_BASE` 0x80) follow MM's. `GetItemEntry` moved to `z64item.h` and widened (item, text
  and draw ids 16-bit, `Player.getItemDrawIdPlusOne` too); `Player_GetItemEntry` finds MM's or OoT's entry
  (`hyrule/HyruleGetItem.c`, generated by `tools/porting/gen_oot_get_items.py`), and `GetItem_Draw` hands OoT's
  draw ids to OoT's draw table (`hyrule/HyruleGetItemDraw.c`, ported by `tools/porting/port_oot_get_item_draw.py`,
  with all of OoT's `object_gi_*` models). OoT's chests and gifts can use them as they are ported.

## Later items, in order

1. **Slingshot** (done).
2. **Boomerang** (done).
3. **Megaton Hammer:** a melee item action (adult only in OoT: waits for adult Link).
4. **Din's Fire, Farore's Wind, Nayru's Love** (done).
5. **OoT's masks and child trade items** (done). The adult trade items come with adult Link.

Each gets its own short proposal when its turn comes.

## Decisions

| | Question | Decision |
|-|----------|----------|
| S1 | What seeds do to MM's enemies | the Deku bubble's damage, and seeds do not stun (MM's leftover entry made them a stunning Deku Nut) |
| S2 | Who uses the Slingshot | human (child) Link only, as in OoT; adult Link uses the Bow |
| S3 | Where seeds come from | anywhere: one drop rule for Termina and Hyrule, so seeds drop in Termina and MM's drops (arrows, magic, MM's per-form drops) in Hyrule |
| S4 | The model | OoT child Link's hand-and-slingshot on MM's Link, as with the Deku Shield |
| M1 | An OoT mask through loading zones | stays on, as MM's masks do (vanilla OoT takes it off) |
| M2 | OoT's get-item models | now, with OoT's whole get-item table, so the trade items are shown |
| F1 | Where Farore's Wind sets a point | dungeons of both worlds: OoT's, as OoT allows, and MM's four temples |
| F2 | The point at the end of a cycle | erased, as the dungeons reset |
| F3 | Din's Fire and Nayru's Love in Termina | OoT's rule: not in Clock Town, houses or shops |
| B1 | What the Boomerang does to MM's creatures | stun, as in OoT; damage to the creatures OoT's boomerang damages, and nothing to those it does nothing to |
