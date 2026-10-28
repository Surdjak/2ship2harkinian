# OoT's items in the shared inventory

**Status: approved 2026-10-23, with the changes recorded under Decisions.** It follows
[DESIGN.md](DESIGN.md): one save, one inventory, OoT and MM masks kept apart, five shields stored as
ownership bits, a permanent Gold Skulltula count.

## Principles

1. **One item, one place.** Where both games have the same item (bombs, the bow, the hookshot...),
   Hyrule and Termina share its slot, its ammo and its upgrades. Picking it up in either world is the
   same pickup.
2. **OoT-only items get new slots** after MM's, so MM's slots, ids and save offsets inside them do not
   move. OoT item ids stay where they are (`ITEM_OOT_BASE` + OoT id, already in the item table).
3. **Use MM's spare fields first.** MM kept OoT's upgrade bit layout and leaves five of its eight
   upgrades unused, and two of its four equipment nibbles.
4. **Hyrule's world resets with each cycle; Link's belongings mostly do not**, as in MM: major items,
   equipment, upgrades, songs and quest items stay; ammo, rupees, bottle contents and trade items go.

## Where each OoT item lives

### Shared with MM (MM's slot)

| OoT item | Shared as |
|----------|-----------|
| Deku Sticks, Deku Nuts, Bombs, Bombchus, Bow, Fire/Ice/Light Arrows | MM's item, ammo and capacity upgrade |
| Fairy Ocarina, Ocarina of Time | MM's ocarina slot: the Fairy Ocarina first, replaced by the Ocarina of Time (D2) |
| Hookshot, Longshot | MM's hookshot slot: child Link uses MM's Hookshot, adult Link OoT's Hookshot, then Longshot (D3) |
| Lens of Truth, Magic Beans | MM's |
| Bottles and their contents | the bottle slots (D1); same contents use MM's ids (potions, fairy, fish, bugs, blue fire, poe, big poe, milk); Ruto's Letter is an OoT content |
| Kokiri Sword | MM's sword: the Razor and Gilded upgrades upgrade it (D4) |
| Quiver, Bomb Bag, Wallet, stick and nut capacity | MM's upgrade fields |
| Heart pieces and containers, magic, double defense, rupees | shared |
| Epona's Song, Song of Storms, Sun's Song, Saria's Song, Song of Time | MM's song bits (MM already has bits for Saria's and the Sun's Song) |

### New item slots (appended after MM's 48)

| Slot | Holds |
|------|-------|
| `SLOT_OOT_SLINGSHOT` | Slingshot, with Deku Seeds as its ammo |
| `SLOT_OOT_BOOMERANG` | Boomerang |
| `SLOT_OOT_HAMMER` | Megaton Hammer |
| `SLOT_OOT_DINS_FIRE`, `_FARORES_WIND`, `_NAYRUS_LOVE` | the three spells |
| `SLOT_OOT_TRADE_CHILD` | Weird Egg, Cucco, Zelda's Letter, each lost when traded, then OoT's masks, which are kept (cycled with SoH's Mask Select, per DESIGN.md) |
| `SLOT_OOT_TRADE_ADULT` | Pocket Egg through Claim Check |
| `SLOT_BOTTLE_7`-`_10` | four more bottles, so OoT's four bottle rewards stay bottles (D1) |

These live in a new `InventoryOot` beside MM's `Inventory`, so MM's arrays and loops keep their sizes.
Their slot ids continue MM's (0x30 and up), so C-buttons and the pause menu can address both the same
way; ported code reads them with `INV_CONTENT_OOT`.

### Equipment

- **Ownership:** a new `u32 equipmentOwned`, one byte per type: swords (Kokiri, Razor, Gilded, Master,
  Great Fairy's, Giant's Knife, Biggoron's, broken Knife), shields (Deku, Hero's, MM Mirror, Hylian,
  OoT Mirror), tunics (Kokiri, Goron, Zora), boots (Kokiri, Iron, Hover).
- **Equipped:** MM's `equipment` keeps its sword and shield nibbles, with new values for the Master
  Sword, the Great Fairy's Sword, the Giant's Knife, Biggoron's Sword and the Deku, Hylian and OoT
  Mirror shields; OoT's tunic and boots go in MM's unused third and fourth nibbles, with OoT's values.
- MM's Kokiri, Razor and Gilded swords keep MM's rule that only one of them exists at a time.
- **The Great Fairy's Sword** becomes a sword instead of a C-button item: it shares the Master Sword's
  equipment slot, as a two-handed sword for child Link and an alternative to the Master Sword for
  adult Link. Its item slot (`SLOT_SWORD_GREAT_FAIRY`) stays empty (save version 12).
- **Shields:** the Deku Shield, Hero's Shield and MM Mirror Shield share one equipment slot; the
  Hylian and OoT Mirror shields have their own, as in OoT. How the shared slot picks among them is part
  of the pause menu proposal.

### Upgrades

OoT's strength (Goron's Bracelet, Silver and Golden Gauntlets), scale and bullet bag fit MM's unused
upgrade fields as they are: same bits, same values. Nothing new is needed.

### Quest status

- A new `u32 questItemsOot`: the six medallions, three spiritual stones, Stone of Agony, Gerudo's
  Card, Zelda's Lullaby, the six warp songs, and the Song of Double Time's learned flag (DESIGN.md:
  taught by Zelda).
- MM's `questItems` keeps the shared songs, heart pieces and MM's own bits.

### Dungeon items

OoT's maps, compasses, boss keys and small keys are per OoT dungeon, which MM's 9- and 10-entry arrays
cannot hold: new `u8 dungeonItemsOot[20]` and `s8 dungeonKeysOot[19]`, indexed as in OoT.

### Gold Skulltulas

- New `u32 gsFlagsOot[6]` (OoT's `gsFlags`) and `s16 gsTokensOot`, apart from MM's swamp and ocean
  counts.
- Both are permanent (D5): the token count is permanent per DESIGN.md, so each Gold Skulltula has to
  stay collected for its token to count once.

## Cycle reset

At the dawn of the first day (`Sram_SaveEndOfCycle`), as MM does for its own items:

| Kept | Reset |
|------|-------|
| OoT's major items, spells, equipment, upgrades | ammo, including Deku Seeds |
| songs, medallions, stones, Agony, Gerudo's Card | rupees (outside the bank), bottle contents |
| OoT's masks (DESIGN.md: owned once borrowed) | the adult trade items, and the child ones other than masks (D6) |
| Gold Skulltula flags and tokens | OoT dungeon keys, maps, compasses and boss keys (D7) |
| | Hyrule's world states keyed to medallions and stones (DESIGN.md: per-cycle flags) |

## Implementation

Done (`hyrule/HyruleInventory.c`, save version 11):

- `SaveInfo.inventoryOot` holds OoT's slots and ammo, adult Link's hookshot, owned OoT masks, equipment
  ownership, OoT's quest bits, dungeon items and keys, and Gold Skulltula flags and tokens. Migration
  11 adds it to older saves: no OoT items, Kokiri tunic and boots worn, MM's sword and shield owned.
- `Item_Give` and `Item_CheckObtainability` take OoT ids: shared items go to MM's under MM's id (so MM's
  ammo, capacities and HUD apply), the rest to `inventoryOot`, following OoT's rules (bottles, trade
  slots, the Longshot, Biggoron's Sword replacing the Giant's Knife, seeds against the bullet bag).
  OoT's dungeon items and keys go to the current OoT dungeon, by its OoT scene id.
- `HyrulePort.h`'s stand-ins read the save: `CHECK_QUEST_ITEM_OOT`, `INV_CONTENT_OOT` (always in OoT
  ids, for shared slots too) and the Gold Skulltula flags. Ported code names OoT's quest items
  `QUEST_*_Oot`.
- MM's own sword and shield are recorded as owned whenever MM's `Item_Give` runs.
- The cycle reset follows the table above.

Not yet:

- **Get-item entries:** OoT's get-item table and models follow MM's ([OOT_ITEMS.md](OOT_ITEMS.md)), but MM's
  `En_Box` keeps its get-item id in 7 bits, so chests holding OoT-only items stay OoT actors.
- **Using OoT items:** the equipment page ([PAUSE.md](PAUSE.md)) equips OoT's swords, shields, tunics and
  boots, and child Link uses the Deku and Hylian Shields and the Great Fairy's Sword; adult pieces wait
  for the adult body. Player using the Slingshot, Boomerang, Hammer and spells comes item by item after
  the item page. Mido still accepts any shield. Link shows OoT's trade items to OoT's people, who take only
  what they ask for, as in OoT (hyrule/README.md, wave 20); Ruto's Letter cannot be shown yet.
- **Losing MM's shield** (stolen, eaten) does not clear its ownership yet; MM's sword follows the one worn.

**Next:** the unified pause menu, in [PAUSE.md](PAUSE.md).

## Decisions

| | Question | Decision |
|-|----------|----------|
| D1 | OoT's four bottles: extra slots, or MM's 6 only | 10 bottles in all |
| D2 | One ocarina slot: the Fairy Ocarina, then the Ocarina of Time | yes |
| D3 | The hookshot slot | MM's Hookshot for child Link; OoT's Hookshot, then Longshot, for adult Link; all in the same slot |
| D4 | One Kokiri Sword for both worlds, upgraded by MM's smithy | yes |
| D5 | Gold Skulltula flags permanent | yes |
| D6 | Trade items other than masks reset each cycle; finished trade rewards stay | yes; child trade items other than masks are also lost when traded |
| D7 | OoT dungeon keys and items reset each cycle | yes |
| | Great Fairy's Sword | moves to the Master Sword's slot (see Equipment) |
| | Shields | Hero's and MM Mirror share the Deku Shield's slot (see Equipment) |

