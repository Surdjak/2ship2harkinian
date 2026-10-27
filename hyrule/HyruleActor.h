#pragma once

// OoT's actor helpers from code (z_actor.c) that MM does not have, for ported actors. See hyrule/README.md.

#include "global.h"

// OoT's BodyBreak (z64actor.h): an enemy's limbs, recorded as it draws, burst into OoT's En_Part pieces when it dies.
typedef struct BodyBreak {
    /* 0x00 */ MtxF* matrices;
    /* 0x04 */ s16* objectIds;
    /* 0x08 */ s16 count;
    /* 0x0C */ Gfx** dLists;
    /* 0x10 */ s32 val; // used for various purposes: both a status indicator and counter
    /* 0x14 */ s32 prevLimbIndex;
} BodyBreak;

#define BODYBREAK_OBJECT_DEFAULT -1 // use the same object as the actor
#define BODYBREAK_STATUS_READY -1
#define BODYBREAK_STATUS_FINISHED 0

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

// OoT's Actor_SetDropFlag and Actor_SetDropFlagJntSph: OoT's drop flags, and with freezeFlag, magic ice and fire
// freeze the actor (its freezeTimer).
void Actor_SetDropFlag_Oot(Actor* actor, ColliderElement* elem, s32 freezeFlag);
void Actor_SetDropFlagJntSph_Oot(Actor* actor, ColliderJntSph* jntSph, s32 freezeFlag);

// OoT's EffectSsDeadDb_Spawn (an enemy's death flame), with OoT's colour components and optional sound.
void EffectSsDeadDb_Spawn_Oot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep,
                              s16 primR, s16 primG, s16 primB, s16 primA, s16 envR, s16 envG, s16 envB, s16 unused,
                              s32 life, s16 playSound);

// OoT's flying ice shards on a frozen actor (MM's En_Ice effect, flying type).
void EffectSsEnIce_SpawnFlyingVec3f(PlayState* play, Actor* actor, Vec3f* pos, s16 primR, s16 primG, s16 primB,
                                    s16 primA, s16 envR, s16 envG, s16 envB, f32 scale);

// OoT's dust variant 0 with a 10-frame life (z_effect_soft_sprite_old_init.c).
void func_8002829C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep);

// An exploding bomb (MM's En_Bom with params 1, as OoT's) whose blast reaches the actor, or NULL.
Actor* func_80033684(PlayState* play, Actor* explosiveActor);

// OoT's helpers from z_actor.c for ported enemies, unchanged but for MM's names.
// Whether Player's footstep flag is set.
s32 func_8002DDE4(PlayState* play);
// Whether Player swings a sword within range, facing the actor, inside the given angles.
s32 func_800354B4(PlayState* play, Actor* actor, f32 range, s16 arg3, s16 arg4, s16 arg5);
// OoT's small falling dust sparkle.
void func_800355B8(PlayState* play, Vec3f* pos);
// Spawns one of OoT's En_Part pieces (flying parts, flames) as the actor's child.
Actor* func_800358DC(Actor* actor, Vec3f* spawnPos, Vec3s* spawnRot, f32* arg3, s32 timer, s16* unused, PlayState* play,
                     s16 params, s32 arg8);
// Tilts a rotation to the actor's floor polygon.
void func_800359B8(Actor* actor, s16 arg1, Vec3s* arg2);
// A Hookshot or arrow about to hit the actor within radius, or NULL.
Actor* Actor_GetProjectileActor(PlayState* play, Actor* refActor, f32 radius);
// EffectSsEnIce_SpawnFlyingVec3f for a Vec3s position.
void EffectSsEnIce_SpawnFlyingVec3s(PlayState* play, Actor* actor, Vec3s* pos, s16 primR, s16 primG, s16 primB,
                                    s16 primA, s16 envR, s16 envG, s16 envB, f32 scale);

// OoT's EffectSsDeadSound (a sound held at a projected position): MM has no such effect, so the sound plays once
// from a copy of that position.
#define DEADSOUND_REPEAT_MODE_OFF 0
#define DEADSOUND_REPEAT_MODE_ON 1
void EffectSsDeadSound_SpawnStationary(PlayState* play, Vec3f* pos, u16 sfxId, s16 lowerPriority, s16 repeatMode,
                                       s32 life);

// OoT's BodyBreak functions (z_actor.c).
void BodyBreak_Alloc(BodyBreak* bodyBreak, s32 count, PlayState* play);
void BodyBreak_SetInfo(BodyBreak* bodyBreak, s32 limbIndex, s32 minLimbIndex, s32 maxLimbIndex, u32 count, Gfx** dList,
                       s16 objectId);
s32 BodyBreak_SpawnParts(Actor* actor, BodyBreak* bodyBreak, PlayState* play, s16 type);
