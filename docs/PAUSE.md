# The unified pause menu

**Status: approved 2026-10-23 (decisions below).** It builds on
[INVENTORY.md](INVENTORY.md), whose items, equipment and quest status need a place on the pause screen.

## Where we start

MM's pause screen (`ovl_kaleido_scope`) is OoT's: four pages on a rotating cube, switched with Z and R.
MM swapped OoT's equipment page for its masks:

| OoT | MM |
|-----|----|
| Items (6x4) | Items (6x4) |
| Map | Map (and the world map) |
| Quest status | Quest status |
| Equipment (4x4, with Link) | Masks (6x4) |

The shared inventory needs room for 35 items (MM's 24, minus the Great Fairy's Sword, plus OoT's 12),
an equipment page, OoT's quest status, and Hyrule's maps.

## Proposed pages

Five pages on a pentagon instead of a cube, in this order (Z and R still step between neighbours):

**Items → Map → Quest → Equipment → Masks → (Items)**

### Items: one 7x5 grid (35 slots)

Cells shrink from 32 to 28 units, so the grid (196 x 140) still fits the page (240 x 160).

```
 Ocarina      Bow          Fire Arrow   Ice Arrow    Light Arrow  Slingshot    Boomerang
 Bombs        Bombchus     Deku Sticks  Deku Nuts    Magic Beans  Powder Keg   Megaton Hammer
 Pictograph   Lens         Hookshot*    Din's Fire   Farore's     Nayru's      Child trade**
 Deed trade   Key/Mama     Couple trade Adult trade  Bottle 1     Bottle 2     Bottle 3
 Bottle 4     Bottle 5     Bottle 6     Bottle 7     Bottle 8     Bottle 9     Bottle 10
```

\* MM's Hookshot for child Link; OoT's Hookshot or Longshot for adult Link (INVENTORY.md, D3).
\** Weird Egg, Cucco, Zelda's Letter, then OoT's masks; A cycles the owned masks (SoH's Mask Select).

Weapons and tools first, then consumables, magic, trade items and bottles. Items the current body
cannot use are greyed out, as MM already does for its transformations.

### Map: by region

In Termina, MM's map page and world map, unchanged. In Hyrule, OoT's: its dungeon map, and Hyrule's
world map with OoT's point markers. Each region's page needs only its own game's assets and code
(OoT's `z_kaleido_map` and `z_lmap_mark`).

### Quest: one merged page

```
  Remains (4)          Medallions (6) around      Spiritual Stones (3)
  Odolwa Goht ...      the Triforce, as in OoT    Emerald Ruby Sapphire

  Songs, two staves of 10:
   Termina: Sonata, Goron Lullaby, Bossa Nova, Elegy, Oath, Time, Healing, Epona's, Soaring, Storms
   Hyrule:  Zelda's Lullaby, Saria's, Sun's, Double Time, Minuet, Bolero, Serenade, Requiem, Nocturne, Prelude

  Bombers' Notebook   Stone of Agony   Gerudo's Card   Quiver   Bomb Bag   Heart pieces   Gold Skulltulas
```

Epona's Song and the Song of Storms belong to both games but are shown once.

### Equipment: OoT's page, with MM's pieces

```
  Upgrades       Swords                        Shields                       Tunics / Boots
  Bullet Bag     Kokiri / Razor / Gilded       Deku / Hero's / MM Mirror     Kokiri    Goron    Zora
  Strength       Master / Great Fairy's        Hylian                        Kokiri    Iron     Hover
  Scale          Giant's Knife / Biggoron's    OoT Mirror
  Wallet
```

(OoT's 4x4 grid: an upgrade column, then three cells each for swords, shields, tunics and boots.)

- **Swords:** MM's Kokiri, Razor and Gilded Swords share a cell (only one exists at a time). The Master
  Sword's cell shows the Great Fairy's Sword for child Link; adult Link sees the Master Sword, and A
  switches to the Great Fairy's Sword (INVENTORY.md).
- **Shields:** the Deku, Hero's and MM Mirror shields share a cell: it shows the one worn, or the best
  owned, and A cycles the owned ones and wears the one shown (U5).
- Only what the current body can wear is shown (DESIGN.md).
- The quiver and bomb bag stay on the quest page, as in MM.

### Masks: unchanged

MM's 24 masks. OoT's masks live in the child trade slot (DESIGN.md).

## Work, in order

1. **Pentagon and equipment page:** five page positions instead of four (the eye moves between them as
   now), and the equipment page with ownership, wearing, and the restrictions per body. Player draws
   and uses the new swords, shields, tunics and boots: this is when OoT's equipment becomes usable.
2. **Link's figure** on the equipment page (U6), in the current body and equipment.
3. **Item page:** the 7x5 grid, OoT's item icons and names (OoT's `icon_item_static` and
   `item_name_static`), C-button assignment for the new slots, and the child trade slot's cycling.
   Player using the Slingshot, Boomerang, Hammer and spells is separate work, item by item.
4. **Quest page:** the merged layout, with OoT's quest icons.
5. **Hyrule's maps:** OoT's map page and world map assets.

Each step leaves the menu working; until step 3, OoT's items stay hidden.

## Implementation

Step 1 is done, for child Link:

- **The ring** (`z_kaleido_scope_NES.c`, `z_kaleido_setup.c`): pages stand on a pentagon
  (`PAUSE_PAGE_DIST` 129, the eye `PAUSE_EYE_DIST` 28 from the centre, so a page is as far from the eye as
  on MM's cube). Only the current page and its two neighbours are built and drawn. The world map follows
  the map page's angle. The equipment page (`PAUSE_EQUIP`) has no C-buttons.
- **The equipment page** (`hyrule/HyruleKaleido.c`, with OoT's `icon_item_static`, `icon_item_nes_static`
  and `item_name_static`): OoT's background, the grid above, OoT's names, the worn piece outlined. Each
  cell lists its pieces in priority order and shows the worn one, else the first the current body can
  wear; A wears it, or the next one if it is already worn. Deku, Goron, Zora and Fierce Deity Link see
  only the upgrades. The quest page shows the sword and shield worn, OoT's included.
- **Ownership:** MM's sword cell follows MM's one-sword rule (the worn MM sword replaces the others; none
  while the smithy or a thief has it). The Great Fairy's Sword is owned while its MM item slot holds it,
  until step 3 frees that slot; the thief bird can steal it while worn, and MM's own sword goes back on B.
- **Player** (`hyrule/HyrulePlayer.c`, `z_player_lib.c`): the Deku Shield is drawn on the back and in the
  hand from OoT's child Link; the Hylian Shield stays on child Link's back and blocks from there while he
  crouches, as in OoT (its shield-only display list is cut from OoT's shield-and-sheath one in 3Ship's
  `object_link_child.xml`). The Great Fairy's Sword is a two-handed sword on B, with MM's own sword's
  sheath shown empty. MM's sword and shield tables are no longer read past their end, and the places
  that rebuild the B item from the sword value (minigames, Fierce Deity, the item tracker) use
  `Hyrule_SwordValueToItem`.

Not yet:

- **Adult pieces** (Master Sword, Giant's Knife, Biggoron's Sword, OoT Mirror Shield, Goron and Zora
  Tunics, Iron and Hover Boots) are on the page but only adult Link can wear them, so Player's models and
  effects for them come with the adult body.
- **Link's figure** (step 2): its area is black.
- The Deku Shield does not burn yet, and MM's Hero's and Mirror Shields still are not unowned when lost
  (INVENTORY.md).

## Decisions

| | Question | Decision |
|-|----------|----------|
| U1 | Five pages on a pentagon, or six (separate Termina and Hyrule quest pages) | five |
| U2 | One 7x5 item grid with slightly smaller cells, or MM's 6x4 grid plus a second item page | one 7x5 grid |
| U3 | The item layout above (tools, consumables, magic, trade, bottles) | as above |
| U4 | Equipment's upgrade column: Bullet Bag, Strength, Scale, Wallet (quiver and bomb bag stay on the quest page) | yes |
| U5 | The shared shield cell shows the one worn (or best owned); A cycles owned shields | yes |
| U6 | Link's figure on the equipment page, as in OoT | yes, right after the equipment page |
| U7 | The map page by region (Termina's in Termina, Hyrule's in Hyrule) | yes |
