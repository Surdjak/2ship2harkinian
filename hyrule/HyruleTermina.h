#pragma once

// The way from Hyrule to Termina: the Termina gate, the Happy Mask Salesman's part in it, and the Lost Woods passage.
// See docs/TERMINA.md and docs/DESIGN.md (Connection between Hyrule and Termina, Termina gate, Mask Shop).

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;
struct Actor;

// The permanent Termina unlocked flag (DESIGN.md): an OoT event flag OoT never uses, which the cycle reset keeps
// (sKeptFlagsOot). Setting it also closes the Happy Mask Shop for good.
#define EVENTCHKINF_TERMINA_UNLOCKED_3SHIP 0xCF

// Epona stolen in MM's intro and not yet recovered at Romani Ranch (DESIGN.md: Epona), kept through the cycle reset.
#define EVENTCHKINF_EPONA_STOLEN_3SHIP 0xCE

s32 Hyrule_IsTerminaUnlocked(void);
void Hyrule_UnlockTermina(void);

// Whether Link owns the gate's first three items: the Ocarina of Time, the Kokiri Sword and Epona's Song. The
// Hero's Shield, the fourth, comes from the Happy Mask Salesman once Link owns them.
s32 Hyrule_OwnsTerminaGateItems(void);

// The Happy Mask Salesman's gift after the fourth sale (DESIGN.md: Mask Shop): the Goron, Zora and Gerudo Masks,
// with the Mask of Truth he lends as in OoT.
void Hyrule_GiveMaskShopGift(struct PlayState* play);

// Whether Epona answers her song: MM's "knows Epona's Song", unless the Skull Kid has her. Romani's lesson, which
// gives the song again, recovers her (Hyrule_RecoverEpona).
s32 Hyrule_HasEpona(void);
void Hyrule_RecoverEpona(void);

// The way back to Hyrule (docs/TERMINA.md step 6): open once Link has reached Clock Town and the Deku curse is lifted.
s32 Hyrule_IsWayBackOpen(void);
// The opening dungeon's landing flower, gold once the way back is open: Deku Link launched high from it rises back to
// MM's Lost Woods.
s32 Hyrule_IsWayBackLandingFlower(struct PlayState* play, struct Actor* actor);
// From Player's scene exits: OoT's Lost Woods' first clearing (room 0), whose straight-ahead exit beside the Skull Kid's
// clearing loops back to Kokiri Forest in OoT, leads child Link into MM's intro once Termina is unlocked. True when
// it took the exit.
s32 Hyrule_TakeTerminaPassage(struct PlayState* play, s32 exitIndex);

// MM's intro areas once Link has reached Clock Town (docs/TERMINA.md step 6, T5, T6): MM's Lost Woods, the opening
// dungeon ("Before Clock Town") and the Clock Tower entered from its corridor are walked without their cutscenes.
s32 Hyrule_SkipsIntroCutscene(struct PlayState* play);

// Each frame: the way back's two exits, from the opening dungeon's landing spot up to MM's Lost Woods, and from the
// end of Epona's path in MM's Lost Woods into OoT's Lost Woods.
void Hyrule_UpdateTerminaPassage(struct PlayState* play);

#ifdef __cplusplus
}
#endif
