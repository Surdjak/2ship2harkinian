#pragma once

// OoT's spells on MM's Player (docs/OOT_ITEMS.md): Din's Fire, Farore's Wind and Nayru's Love, their scene rules,
// and Farore's Wind's warp point.

#include "PR/ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;
struct Player;
struct Actor;

// OoT's spells, in the order of OoT's sMagicSpellActorIds and Player's spell animations.
typedef enum HyruleSpell {
    /* 0 */ HYRULE_SPELL_FARORES_WIND,
    /* 1 */ HYRULE_SPELL_NAYRUS_LOVE,
    /* 2 */ HYRULE_SPELL_DINS_FIRE,
    /* 3 */ HYRULE_SPELL_MAX
} HyruleSpell;

// The spell an item is, or -1.
s32 Hyrule_GetSpell(u16 item);
// OoT's magic cost of a spell.
s32 Hyrule_GetSpellCost(s32 spell);
// Whether the current scene allows a spell: Hyrule's scenes by OoT's table; in Termina, Farore's Wind in MM's
// temples only, and Din's Fire and Nayru's Love outside Clock Town (houses and shops grey every OoT item).
s32 Hyrule_IsSpellAllowed(struct PlayState* play, s32 spell);
// Whether an item can be used in the current scene as far as spells go: false for a spell the scene forbids.
s32 Hyrule_IsSpellItemAllowed(struct PlayState* play, u16 item);
// Spawns the spell's actor on Player (OoT's Player_SpawnMagicSpell).
struct Actor* Hyrule_SpawnSpell(struct PlayState* play, struct Player* player, s32 spell);

// Farore's Wind. The point is saved (SaveInfo.faroresWindOot, set while its data is non-zero); its pointer is
// drawn from a copy kept for the scene, as OoT keeps it in respawn[RESPAWN_MODE_TOP], which MM uses for itself.
s32 Hyrule_IsFaroresWindSet(void);
// At scene start (OoT's func_8002FA60): the pointer's light, and the pointer from the saved point.
void Hyrule_InitFaroresWind(struct PlayState* play);
void Hyrule_DestroyFaroresWind(struct PlayState* play);
// The pointer and its sparkles, each frame (OoT's Actor_DrawFaroresWindPointer).
void Hyrule_DrawFaroresWind(struct PlayState* play);
// Saves Link's room entry point as the warp point; the pointer flies there from Link.
void Hyrule_SetFaroresWind(struct PlayState* play);
// Erases the point; the pointer shrinks away.
void Hyrule_DispelFaroresWind(void);
// Erases the point without effects, at the end of a cycle.
void Hyrule_ClearFaroresWind(void);
// Warps to the point (OoT's Player_Action_8085063C, choice 0).
void Hyrule_ReturnToFaroresWind(struct PlayState* play);
// Whether Player is arriving by Farore's Wind; true once per warp.
s32 Hyrule_TakeFaroresWindArrival(void);
// Link appearing at the point: the pointer, once there, flies off and the point is used up.
void Hyrule_ArriveAtFaroresWind(struct PlayState* play);

#ifdef __cplusplus
}
#endif
