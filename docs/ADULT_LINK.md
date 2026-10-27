# Adult Link

**Status: approved 2026-10-26 (decisions below).** DESIGN.md gives Link six bodies, the sixth being OoT's
adult Link; PLAN.md puts him in Stage 1. The pause menu already lists his equipment (PAUSE.md, "Adult pieces"),
and his items wait for him (OOT_ITEMS.md: the Megaton Hammer, the adult trade items, OoT's Hookshot and Longshot).

## Where we start

- **MM's Player has five bodies** (`PlayerTransformation`): Fierce Deity, Goron, Zora, Deku, and human (child)
  Link. About 50 tables are sized by body, and the code checks the body some 430 times, mostly "is human".
- **Fierce Deity is OoT's adult Link, scaled up.** MM made his body row (`sPlayerAgeProperties`) from OoT's adult
  one ×1.5 (ceiling 84 for OoT's 56, and so on), with OoT's adult voice. MM's Player still moves, climbs and
  fights at adult proportions.
- **OoT treats age as a property of one body, not a body of its own.** Child and adult share Player's code and
  its animations; age picks the body row, the model, the voice, and a few branches (about 20, plus the Iron and
  Hover Boots, the Megaton Hammer, the tunics and the Longshot).
- **MM's Player dropped some adult pieces.** MM kept the Bow, the Hookshot and the heavy sinking of the Iron
  Boots (Zora Link's), but not the Hover Boots or the tunics' effects.
- **3Ship stands in for age** with `LINK_IS_ADULT_OOT`, always false (`hyrule/HyrulePort.h`).

## Proposal

**Adult Link is human Link at adult age** (A1), as OoT does it: same transformation (`PLAYER_FORM_HUMAN`), with an
age saved alongside it. Everything MM's human Link does, adult Link does, unless his age says otherwise. That
keeps MM's 280 "is human" paths working for him without being reviewed one by one, and it is where ported OoT
actors expect him (`LINK_IS_ADULT`).

| Part | Child (today) | Adult |
|------|---------------|-------|
| Body row | MM's human row | OoT's adult row (MM's Fierce Deity row ÷1.5) |
| Model | MM's `object_link_child` | OoT's `object_link_boy`: skeleton, face, hands, sheaths, shields, swords, bow, hookshot, bottles, first-person arms |
| Voice | MM's child voice | OoT's adult voice (MM's Fierce Deity voice bank) |
| Tunic | green | Kokiri, Goron or Zora, by the equipment page |
| Buttons and equipment | the human row | their own, swapped when age changes, as OoT's `childEquips`/`adultEquips` (A3) |
| Items | MM's and OoT's child items | by OoT's age rules (A4); masks too (A5) |

### Work, in order

1. **The body**: the saved age (`LINK_IS_ADULT_OOT` becomes real), the adult row, model, voice and tunic colour,
   and Link's figure on the pause page. Age changes with a developer toggle first (A2).
2. **Per-age buttons, equipment and items**: the swap on age change, the items each age can use (greyed on the
   item page as for MM's bodies), and the adult pieces of the equipment page becoming wearable. Masks fit to
   his head, and transformations return to his age.
3. **Adult equipment**: the Master Sword, Giant's Knife (it breaks) and Biggoron's Sword (MM's two-handed path,
   DESIGN.md), the Hylian Shield in hand and OoT's Mirror Shield (it reflects light), the Goron Tunic (heat) and
   Zora Tunic (breathing underwater), the Iron Boots and the Hover Boots.
4. **Adult items**: the Megaton Hammer, OoT's Hookshot and Longshot (INVENTORY.md D3), the Bow in his hands, and the
   adult trade items shown as the child ones are.
5. **Hyrule at adult age**: OoT's scenes load their adult layers for adult Link, as OoT does (only the actors
   ported so far appear). Termina is unchanged: he is the same Link there (Stage 3).

Each step ends with harness tests of both ages and the usual regressions. Changing age the OoT way (the Master
Sword in the Temple of Time) comes with the Temple of Time.

## Implementation

Step 1, the body, is done:

- **Age** (`SaveInfo.isAdultOot`, save version 16): `LINK_IS_ADULT_OOT` reads it, so ported OoT actors see his age.
  It changes with the developer console's `age [child|adult]` (reloads the map) and the harness's
  `HYRULE_BOOT_ADULT=1`.
- **Body** (`z_player.c`): adult Link (`Hyrule_IsAdultPlayer`) takes `sAdultAgeProperties_Oot`, OoT's adult row, and
  OoT's adult skeleton (`object_link_boy`, from OoT's archive); the row carries OoT's adult voice.
- **Model** (`hyrule/HyrulePlayer.c`, `z_player_lib.c`): his eyes, mouth and tunic colour (OoT's `Player_DrawImpl`),
  and his hands, sheath and waist for each of MM's model types: the Master Sword or Biggoron's Sword in hand, the
  Hylian Shield (also standing in for MM's Hero's Shield, the same design) or OoT's Mirror Shield in hand or on
  his back with the sheath, as OoT's single display lists draw them, the bow (with OoT's adult string), the
  Ocarina of Time, the hookshot and bottles; and his arms in first person.
- **Link's figure** on the equipment page stands as OoT stands adult Link.

Step 2, per-age buttons, equipment and items, is done:

- **Buttons and equipment by age** (`Hyrule_SetAge`, `hyrule/HyruleInventory.c`): human Link's row (B, the
  C-buttons MM's other bodies share, the D-pad) and the equipment swap with `SaveInfo.otherAgeEquipsOot` (save
  version 17), as OoT's `Inventory_SwapAgeEquipment` swaps `childEquips` and `adultEquips`. An age's first buttons
  are OoT's: adult Link gets the Master Sword, the Hylian Shield (else the Hero's Shield), the Kokiri tunic and
  boots, and Deku Nuts, Bombs and the ocarina on C. Coming back to an age, each button shows what its slot now
  holds (bottles, trade items, the end of a cycle). The developer toggles give the Master Sword, as the pedestal
  will.
- **Swords**: adult Link's take MM's sword paths in Player (the Master Sword the one-handed one, the Giant's Knife
  and Biggoron's Sword the two-handed one) and OoT's hands, the broken knife included; their damage and the
  knife breaking are step 3. MM's swords are child Link's: their ownership is kept while he is adult, and the
  end of a cycle turns child Link's stored Razor Sword back into the Kokiri Sword.
- **Items by age** (`Hyrule_IsItemUsableAtAge`, A4): greyed on the HUD and the item page as for MM's bodies.
  The Hookshot's slot shows MM's Hookshot to child Link and OoT's Hookshot or Longshot to adult Link (using them
  is step 4).
- **Masks** fit adult Link's head (`Hyrule_FitMaskToAdultHead`): MM's are drawn with his head's matrix, stretched
  to his longer face (measured from the two head models); OoT's, which load the head's matrix from the skeleton's,
  get the same fitted matrix. A transformation mask taken off returns him to his age (Player_Init picks the body
  by the saved age).

Step 3, adult equipment, is done:

- **Swords** (`Hyrule_SetSwordDamage`, A7): every human sword attack also carries OoT's damage type for its
  class, the slash or, for jump attacks, the jump one, so OoT's creatures take OoT's damage; OoT's damage tables
  now give the damage itself, as OoT reads them (they were read as MM's multipliers, which broke values 3 and
  4). To MM's creatures the Master Sword hits as the Razor Sword (2, 4 jumping) and the Giant's Knife and
  Biggoron's Sword as the Great Fairy's Sword (4, 8).
- **The Giant's Knife breaks** (`Hyrule_WearGiantsKnife`, save version 18 `inventoryOot.knifeHealth`): 8 hits,
  counted where MM wears the Razor Sword down; then it is the broken knife, on B and owned, which hits as the
  Kokiri Sword, swings lightly and cannot spin, as in OoT.
- **OoT's Mirror Shield** reflects light and counts as a mirror shield for MM's actors
  (`Player_HasMirrorShieldEquipped`).
- **Tunics**: MM kept OoT's hazard timer and its tunic checks; OoT's hot rooms keep their "hot" value. Adult Link
  follows OoT's underwater rule (in the Iron Boots, 80 frames; otherwise 300), the timer runs on the bottom too,
  and OoT's warnings show ("it's so hot", "take off those boots").
- **Boots** (`PlayerBoots`, `z_player_lib.c`): adult Link's Kokiri, Hover and Iron Boots have OoT's movement rows
  (adult Link now runs at OoT's adult speed). The Iron Boots are heavy boots for MM's Player, sink and walk on the
  bottom as Zora Link does, are not moved by currents or conveyors, run with OoT's heavy-run animation and step
  heavily. The Hover Boots float for 19 frames off an edge or over water and soft ground, hum and glow while
  they do, and slide (OoT's `Player_UpdateHoverBoots`, ported into MM's Deku hovering). Both are drawn on his feet.

Step 4, adult items, is done:

- **The Megaton Hammer** (`PLAYER_IA_HAMMER_Oot`): MM's two-handed weapon path, as the Great Fairy's Sword is,
  taken out and swung with its C button. It swings with OoT's forward and side animations (in MM's two unused
  backslash attacks, `PLAYER_MWA_HAMMER_*_Oot`), never in combos or spins; hits with OoT's hammer damage types and
  MM's Goron punch and pound, which stand in for them (2, 4 jumping); and strikes walls and the ground in front of
  him with OoT's quake, ground pound (`actorCtx.unk2`) and shockwave, through the wall-hit code Goron Link's
  punches took over from OoT's hammer.
- **OoT's Hookshot and Longshot** are MM's hookshot, with OoT's reach: 13 frames for the Hookshot, 26 for the
  Longshot (MM's Hookshot's).
- **The adult trade items** (`PLAYER_IA_POCKET_EGG_Oot` to `PLAYER_IA_CLAIM_CHECK_Oot`) are shown to people as the
  child ones are. The Pocket Egg hatches at dawn as the Weird Egg does, and dying turns the Odd Mushroom back into
  Cojiro and the Eyeball Frog and Eyedrops back into the Prescription (OoT's spoiling items; their timers come with
  the people who start them).
- **The Giant's Knife's item ids** now follow OoT's: `ITEM_SWORD_BGS_Oot` is the whole knife and Biggoron's Sword
  (told apart by the get-item id, `Hyrule_GiveGiantsKnife`), `ITEM_SWORD_KNIFE_Oot` the broken knife, and
  `ITEM_SWORD_BROKEN_Oot` the Broken Goron's Sword, a trade item. The equipment page names Biggoron's Sword as OoT
  does.

Step 5, Hyrule at adult age, is done: OoT's scenes already chose their adult day and night layers by Link's age
(`Hyrule_GetSceneLayer`); OoT's two exceptions now follow (`Hyrule_GetSceneLayerException`): adult Link's Kokiri
Forest keeps its overrun layer day and night until the Forest Temple is cleared, and child Link's Hyrule Field
shows Zelda's escape layer once he has the three spiritual stones. Every ported scene (Kokiri Forest, its houses
and shop, the Lost Woods) loads its adult layers by day and night; their ported actors behave by age as in OoT
(the Kokiri hide in their houses until adult Link has the Forest Medallion). Adult Kokiri Forest's Octoroks,
Deku Babas and Mad Scrubs came with actor wave 11 (hyrule/README.md); its Gossip Stone is not ported yet.

## Decisions

| | Question | Proposal |
|-|----------|----------|
| A1 | Adult Link: an age of human Link, or a sixth transformation | an age of human Link, as in OoT; to the player, still a body of his own |
| A2 | How age changes | OoT's Master Sword pedestal once the Temple of Time is ported; until then a developer toggle (2Ship's developer menu and the harness) |
| A3 | Buttons and equipment | each age has its own, kept when he changes age, as in OoT |
| A4 | Items by age | OoT's items by OoT's rules: the Slingshot, Boomerang, Deku Sticks and Kokiri Sword are child Link's; the Megaton Hammer, Iron and Hover Boots, Goron and Zora Tunics, adult swords, OoT's Mirror Shield and Longshot are adult Link's. MM's items work for both, masks included (A5) |
| A5 | Masks on adult Link | he wears them, MM's and OoT's, and transforms; taking a transformation mask off returns him to his age (not in OoT) |
| A6 | Age at the cycle reset | unchanged (DESIGN.md already says so) |
| A7 | Swords against OoT's creatures | OoT's three classes: the Kokiri Sword and the broken knife are Kokiri, the Razor and Gilded Swords and the Master Sword are Master, the Great Fairy's Sword, Giant's Knife and Biggoron's Sword are Giant's (step 3) |
