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
// collectibles (OotToMm::ActorParams). MM has no Deku seeds, tunics or Deku shield (ITEM00_NOTHING).
s16 Item00_FromOot(s16 ootType);

// One of OoT's 23 preset damage tables (HyruleDamage.c), or NULL.
DamageTable* DamageTable_Get_Oot(s32 index);

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
