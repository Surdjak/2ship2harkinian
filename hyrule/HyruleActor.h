#pragma once

// OoT's actor helpers from code (z_actor.c) that MM does not have, for ported actors. See hyrule/README.md.

#include "global.h"

// Fades alpha in or out by Player's distance (the camera's in cutscenes) and enables attention to match.
s16 Actor_UpdateAlphaByDistance(Actor* actor, PlayState* play, s16 alpha, f32 radius);

// OoT's brown dust puffs (z_effect_soft_sprite_old_init.c).
void func_800286CC(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep);

// OoT's last collectible type (ITEM00_BOMBS_SPECIAL in OoT).
#define ITEM00_MAX_OOT 0x19

// MM's collectible type (En_Item00) for one of OoT's, the same mapping the converter gives room
// collectibles (OotToMm::ActorParams). MM has no tunics or Deku shield (ITEM00_NOTHING); OoT's Deku Seeds
// are ITEM00_DEKU_SEEDS_Oot.
s16 Item00_FromOot(s16 ootType);

// Arrows and Deku Seeds drop as the ammo Link carries, in Termina and Hyrule alike (docs/OOT_ITEMS.md):
// seeds with the Slingshot only, arrows with the Bow only, and with both, the emptier one. With neither,
// arrows, which En_Item00 then drops as nothing.
s16 Item00_ArrowsOrSeeds(s16 dropId);
// OoT's Deku Seeds drop sprite.
TexturePtr Item00_GetDekuSeedsTex(void);

// MM keeps these from OoT in z_actor.c, unused there and so without a prototype in MM's headers.
s16 Npc_GetTrackingPresetMaxPlayerYaw(s16 presetIndex);
void Actor_UpdateFidgetTables(PlayState* play, s16* fidgetTableY, s16* fidgetTableZ, s32 tableLen);

// One of OoT's 23 preset damage tables (HyruleDamage.c), or NULL.
DamageTable* DamageTable_Get_Oot(s32 index);

// Whether a damage table is one of MM's, which has nothing for OoT's attacks (entries 32-63).
s32 DamageTable_IsMm(DamageTable* damageTable);
// OoT's Boomerang on an MM actor (docs/OOT_ITEMS.md): the MM attack whose entry does what OoT's boomerang does
// to that creature in OoT (DMG_DEKU_NUT's for a stun, DMG_ZORA_BOOMERANG's for damage), or -1 for nothing.
s32 DamageTable_GetOotBoomerangAttack(Actor* actor);

// OoT's dust variant 0 (z_effect_soft_sprite_old_init.c).
void func_8002836C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life);

// True while Player stands on a ladder.
s32 func_8002DDF4(PlayState* play);

// Sets up the specular highlight for the actor's opaque draw; with flag, also its segment 7 tile.
void func_8002EBCC(Actor* actor, PlayState* play, s32 flag);

// Draw a flex skeleton with the env colour's alpha set, opaque or translucent.
void func_80034BA0(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha);
void func_80034CC4(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha);
