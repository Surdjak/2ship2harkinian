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

// OoT's animation table entry (z64.h); MM's AnimationInfo also has a play speed and a start frame.
typedef struct AnimationFrameCountInfo {
    /* 0x00 */ AnimationHeader* animation;
    /* 0x04 */ f32 frameCount;
    /* 0x08 */ u8 mode;
    /* 0x0C */ f32 morphFrames;
} AnimationFrameCountInfo;

#define BODYBREAK_OBJECT_DEFAULT -1 // use the same object as the actor
#define BODYBREAK_STATUS_READY -1
#define BODYBREAK_STATUS_FINISHED 0

// SoH's SkelAnime_DrawSkeletonOpa: draws a normal or a flexible skeleton (MM's SkelAnime keeps dListCount only for
// flexible ones), so that ported actors need not know which theirs is.
void SkelAnime_DrawSkeletonOpaOot(PlayState* play, SkelAnime* skelAnime, OverrideLimbDrawOpa overrideLimbDraw,
                                  PostLimbDrawOpa postLimbDraw, void* arg);
// SoH's SkelAnime_DrawSkeleton2: the same, into a display list given and returned.
Gfx* SkelAnime_DrawSkeleton2Oot(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                                PostLimbDraw postLimbDraw, void* arg, Gfx* gfx);
// OoT's D_80116280: the render mode translucent skeletons draw with (Poes'), for segment 0x0C.
extern Gfx D_80116280_Oot[];

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

// OoT's Actor_Find: the first actor of this id in a category, or NULL.
Actor* Actor_FindOot(ActorContext* actorCtx, s32 actorId, s32 actorCategory);

// OoT's actor helpers that MM lacks or names differently (the port tool renames their calls).
// OoT's func_8002843C: dust drawn with flag 2.
void EffectSsDust_SpawnDrawFlag2Oot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                                    Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life);
// OoT's func_80035844: the yaw and pitch from one point to another.
void Math_Vec3fDiffToRotOot(Vec3f* from, Vec3f* to, Vec3s* rot, s32 upIsPositive);
Actor* Actor_GetCollidedExplosiveOot(PlayState* play, Collider* collider);
// The text id for a scene's group of texts (OoT's Actor_SetTextWithPrefix), as an OoT text.
void Actor_SetTextWithPrefixOot(PlayState* play, Actor* actor, s16 baseTextId);
void EffectSsDeadDd_SpawnOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep,
                             s16 primR, s16 primG, s16 primB, s16 primA, s16 envR, s16 envG, s16 envB, s16 alphaStep,
                             s32 life);
void EffectSsDeadDs_SpawnStationaryOot(PlayState* play, Vec3f* pos, s16 scale, s16 scaleStep, s16 alpha, s32 life);
// MM has no fire-circle effect: does nothing.
void EffectSsFCircle_SpawnOot(PlayState* play, Actor* actor, Vec3f* pos, s16 radius, s16 height);
// OoT's func_80094E78.
Gfx* Gfx_TexScrollOot(GraphicsContext* gfxCtx, u32 x, u32 y);
// OoT's Play_TriggerRespawn.
void Hyrule_TriggerRespawnOot(PlayState* play);
// MM's, which its header leaves out.
Hilite* Hilite_DrawXlu(Vec3f* object, Vec3f* eye, Vec3f* lightDir, GraphicsContext* gfxCtx);
// OoT's CollisionCheck_GetSwordDamage: a sword hit's damage from OoT's damage flags.
u8 CollisionCheck_GetSwordDamageOot(u64 dmgFlags, PlayState* play);
// OoT's func_80064520 and func_80064534: an actor's manual cutscene, in OoT's cutscene context (HyruleCutscene.c).
void Cutscene_StartManualOot(PlayState* play);
void Cutscene_StopManualOot(PlayState* play);
// OoT's OnePointCutscene_AttentionSetSfx: an attention shot (HyruleOnePoint.c) that plays sfxId as it starts.
s32 OnePointCutscene_AttentionSetSfxOot(PlayState* play, Actor* actor, s32 sfxId);
// OoT's EffectSsFhgFlash_SpawnLightBall (Phantom Ganon's light balls); MM's effect has only the shock spawner.
void EffectSsFhgFlash_SpawnLightBallOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, u8 param);
// OoT's func_80093C80: Gfx_SetupDL25_Opa, without color dither in rooms of OoT's behavior type 3.
void Gfx_SetupDL25_OpaRoomOot(PlayState* play);
// OoT's func_800355E4: whether a Megaton Hammer swing forward hit this collider.
s32 Actor_IsHitByHammerForwardOot(PlayState* play, Collider* collider);
// OoT's Actor_SwapHookshotAttachment: the hookshot lets go of actorA and holds actorB.
void Actor_SwapHookshotAttachmentOot(PlayState* play, Actor* actorA, Actor* actorB);
// MM's, which its header leaves out.
void SkelAnime_InterpFrameTable(s32 limbCount, Vec3s* dst, Vec3s* start, Vec3s* target, f32 weight);
// OoT's Math_StepUntilS.
s32 Math_StepUntilSOot(s16* pValue, s16 limit, s16 step);
// MM's, which its headers leave out.
void Skin_DrawImpl(Actor* actor, PlayState* play, Skin* skin, SkinPostDraw postDraw,
                   SkinOverrideLimbDraw overrideLimbDraw, s32 setTranslation, s32 arg6, s32 drawFlags);
void Play_ClearAllSubCameras(PlayState* play);
// OoT's dust and sparkle spawners.
void func_8003555C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel);
void func_800283D4(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life);
// OoT's func_80033AEC: steps arg1 toward arg0 unless it is between arg4 and arg5 away.
f32 func_80033AEC(Vec3f* arg0, Vec3f* arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5);
// OoT's Dodongo fire, through MM's D_Fire effect (which draws from MM's object_dodongo; the actor loads it).
void EffectSsDFire_SpawnFixedScaleOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 alpha,
                                      s16 fadeDelay);
// OoT's lava bubbles and splashes (King Dodongo's lava), which MM lacks: not drawn.
void EffectSsGMagma_SpawnOot(PlayState* play, Vec3f* pos);
void EffectSsGMagma2_SpawnOot(PlayState* play, Vec3f* pos, Color_RGBA8* primColor, Color_RGBA8* envColor,
                              s16 updateRate, s16 drawMode, s16 scale);
// MM's Player limb override for the gameplay skeleton, which its headers leave out (Dark Link draws with it).
s32 Player_OverrideLimbDrawGameplayCommon(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                          Actor* thisx);
// OoT's generic NPC talk (func_80037C30 picks the text, func_80037D98 offers it and follows it), for the NPCs ported
// so far: OoT's npc id 0xF (the Deku Sprout). Other ids get no text.
u16 func_80037C30(PlayState* play, s16 npcId);
s32 func_80037D98(PlayState* play, Actor* actor, s16 npcId, s32* talkState);
// OoT's func_800C0808: the camera follows Link with the given setting.
s32 func_800C0808(PlayState* play, s16 camId, Player* player, s16 setting);
