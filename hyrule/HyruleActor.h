#pragma once

// OoT's actor helpers from code (z_actor.c) that MM does not have, for ported actors. See hyrule/README.md.

#include "global.h"

// Fades alpha in or out by Player's distance (the camera's in cutscenes) and enables attention to match.
s16 Actor_UpdateAlphaByDistance(Actor* actor, PlayState* play, s16 alpha, f32 radius);

// OoT's brown dust puffs (z_effect_soft_sprite_old_init.c).
void func_800286CC(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep);

// Draw a flex skeleton with the env colour's alpha set, opaque or translucent.
void func_80034BA0(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha);
void func_80034CC4(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha);
