// OoT's actor helpers from code (z_actor.c) that MM does not have. Ported from SoH.

#include "HyruleActor.h"
#include "HyruleFlags.h"
#include "HyruleInventory.h"
#include "HyruleIds.h"
#include "HyruleScenes.h"
#include "HyruleCutscene.h"
#include "overlays/effects/ovl_Effect_Ss_Fhg_Flash/z_eff_ss_fhg_flash.h"
#include "overlays/actors/ovl_Arms_Hook/z_arms_hook.h"
#include "overlays/actors/ovl_En_Part/z_en_part.h" // OoT's, in hyrule/
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"

s32 Hyrule_IsBombchuBowlingOot(PlayState* play) {
    // OoT's bowling alley sets switch 0x38 for the length of a game.
    return (play->sceneId == SCENE_BOMBCHU_BOWLING_ALLEY_Oot) && Flags_GetSwitch(play, 0x38);
}

static Gfx* func_80034B28(GraphicsContext* gfxCtx) {
    Gfx* displayList;

    displayList = GRAPH_ALLOC(gfxCtx, sizeof(Gfx));
    gSPEndDisplayList(displayList);

    return displayList;
}

static Gfx* func_80034B54(GraphicsContext* gfxCtx) {
    Gfx* displayListHead;
    Gfx* displayList;

    displayList = displayListHead = GRAPH_ALLOC(gfxCtx, 2 * sizeof(Gfx));

    gDPSetRenderMode(displayListHead++, G_RM_FOG_SHADE_A,
                     AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU | FORCE_BL |
                         GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA));

    gSPEndDisplayList(displayListHead++);

    return displayList;
}

void func_80034BA0(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, alpha);
    gDPPipeSync(POLY_OPA_DISP++);
    gSPSegment(POLY_OPA_DISP++, 0x0C, func_80034B28(play->state.gfxCtx));

    POLY_OPA_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                                       overrideLimbDraw, postLimbDraw, actor, POLY_OPA_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80034CC4(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                   PostLimbDraw postLimbDraw, Actor* actor, s16 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, alpha);
    gSPSegment(POLY_XLU_DISP++, 0x0C, func_80034B54(play->state.gfxCtx));

    POLY_XLU_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                                       overrideLimbDraw, postLimbDraw, actor, POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

s16 Actor_UpdateAlphaByDistance(Actor* actor, PlayState* play, s16 alpha, f32 radius) {
    Player* player = GET_PLAYER(play);
    f32 distance;

    if ((play->csCtx.state != CS_STATE_IDLE) || (gDbgCamEnabled)) {
        distance = Math_Vec3f_DistXYZ(&actor->world.pos, &play->view.eye) * 0.25f;
    } else {
        distance = Math_Vec3f_DistXYZ(&actor->world.pos, &player->actor.world.pos);
    }

    if (radius < distance) {
        actor->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        Math_SmoothStepToS(&alpha, 0, 6, 0x14, 1);
    } else {
        actor->flags |= ACTOR_FLAG_ATTENTION_ENABLED;
        Math_SmoothStepToS(&alpha, 0xFF, 6, 0x14, 1);
    }

    return alpha;
}

static Color_RGBA8 sDustBrownPrim = { 170, 130, 90, 255 };
static Color_RGBA8 sDustBrownEnv = { 100, 60, 20, 255 };

void func_800286CC(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep) {
    EffectSsDust_Spawn(play, 5, pos, velocity, accel, &sDustBrownPrim, &sDustBrownEnv, scale, scaleStep, 10, 0);
}

void func_8002836C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life) {
    EffectSsDust_Spawn(play, 0, pos, velocity, accel, primColor, envColor, scale, scaleStep, life, 0);
}

s32 func_8002DDF4(PlayState* play) {
    Player* player = GET_PLAYER(play);

    // OoT's PLAYER_STATE2_STATIONARY_LADDER, the same bit in MM.
    return player->stateFlags2 & PLAYER_STATE2_1000;
}

void func_8002EBCC(Actor* actor, PlayState* play, s32 flag) {
    Hilite* hilite;
    Vec3f lightDir;
    Gfx* displayListHead;
    Gfx* displayList;

    lightDir.x = play->envCtx.dirLight1.params.dir.x;
    lightDir.y = play->envCtx.dirLight1.params.dir.y;
    lightDir.z = play->envCtx.dirLight1.params.dir.z;

    hilite = Hilite_DrawOpa(&actor->world.pos, &play->view.eye, &lightDir, play->state.gfxCtx);

    if (flag != 0) {
        displayList = GRAPH_ALLOC(play->state.gfxCtx, 2 * sizeof(Gfx));
        displayListHead = displayList;

        OPEN_DISPS(play->state.gfxCtx);

        gDPSetHilite1Tile(displayListHead++, 1, hilite, 0x10, 0x10);
        gSPEndDisplayList(displayListHead);
        gSPSegment(POLY_OPA_DISP++, 0x07, displayList);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

s16 Item00_FromOot(s16 ootType) {
    static const u8 sMmItem00[ITEM00_MAX_OOT + 1] = {
        /* RUPEE_GREEN */ 0x00,  /* RUPEE_BLUE */ 0x01,    /* RUPEE_RED */ 0x02,       /* HEART */ 0x03,
        /* BOMBS_A */ 0x04,      /* ARROWS_SINGLE */ 0x05, /* HEART_PIECE */ 0x06,     /* HEART_CONTAINER */ 0x07,
        /* ARROWS_SMALL */ 0x05, /* ARROWS_MEDIUM */ 0x05, /* ARROWS_LARGE */ 0x08,    /* BOMBS_B */ 0x0B,
        /* NUTS */ 0x0C,         /* STICK */ 0x0D,         /* MAGIC_LARGE */ 0x0E,     /* MAGIC_SMALL */ 0x0F,
        /* SEEDS */ 0x1E,        /* SMALL_KEY */ 0x11,     /* FLEXIBLE */ 0x12,        /* RUPEE_ORANGE */ 0x13,
        /* RUPEE_PURPLE */ 0x14, /* SHIELD_DEKU */ 0x16,   /* SHIELD_HYLIAN */ 0x16,   /* TUNIC_ZORA */ 0x18,
        /* TUNIC_GORON */ 0x18,  /* BOMBS_SPECIAL */ 0x04,
    };

    return ((ootType >= 0) && (ootType <= ITEM00_MAX_OOT)) ? sMmItem00[ootType] : ITEM00_NOTHING;
}

s16 Item00_ArrowsOrSeeds(s16 dropId) {
    s32 hasBow = INV_CONTENT(ITEM_BOW) != ITEM_NONE;
    s32 hasSlingshot = INV_CONTENT_SLOT_OOT(SLOT_OOT_SLINGSHOT) != ITEM_NONE;
    s32 seeds;

    if (hasBow && hasSlingshot) {
        // The fuller one, by the share of its capacity.
        seeds = AMMO_SLOT_OOT(SLOT_OOT_SLINGSHOT) * CUR_CAPACITY(UPG_QUIVER) <
                AMMO(ITEM_BOW) * CUR_CAPACITY(UPG_BULLET_BAG);
    } else {
        seeds = hasSlingshot;
    }

    if (seeds) {
        return ITEM00_DEKU_SEEDS_Oot;
    }
    return (dropId == ITEM00_DEKU_SEEDS_Oot) ? ITEM00_ARROWS_10 : dropId;
}

TexturePtr Item00_GetDekuSeedsTex(void) {
    return (TexturePtr)gDropDekuSeedsTex;
}

// OoT's drop flag for one hit element: also freezes the actor for magic ice and fire (with freezeFlag), and
// tells OoT's other arrows and Din's apart. Masks go through DMG_FROM_OOT, so MM's own arrows count too.
static s32 Actor_GetDropFlag_Oot(Actor* actor, ColliderElement* elem, s32 freezeFlag) {
    ColliderElement* acHitElem = elem->acHitElem;
    u64 dmgFlags;

    if (acHitElem == NULL) {
        return 0x00;
    }
    dmgFlags = acHitElem->atDmgInfo.dmgFlags;
    if (freezeFlag && (dmgFlags & DMG_FROM_OOT(0x10060000))) {
        actor->freezeTimer = acHitElem->atDmgInfo.damage;
        return 0x00;
    }
    if (dmgFlags & DMG_FROM_OOT(0x0800)) {
        return 0x01;
    }
    if (dmgFlags & DMG_FROM_OOT(0x1000)) {
        return 0x02;
    }
    if (dmgFlags & DMG_FROM_OOT(0x4000)) {
        return 0x04;
    }
    if (dmgFlags & DMG_FROM_OOT(0x8000)) {
        return 0x08;
    }
    if (dmgFlags & DMG_FROM_OOT(0x10000)) {
        return 0x10;
    }
    if (dmgFlags & DMG_FROM_OOT(0x2000)) {
        return 0x20;
    }
    if (dmgFlags & DMG_FROM_OOT(0x80000)) {
        if (freezeFlag) {
            actor->freezeTimer = acHitElem->atDmgInfo.damage;
        }
        return 0x40;
    }
    return 0x00;
}

void Actor_SetDropFlag_Oot(Actor* actor, ColliderElement* elem, s32 freezeFlag) {
    actor->dropFlag = Actor_GetDropFlag_Oot(actor, elem, freezeFlag);
}

void Actor_SetDropFlagJntSph_Oot(Actor* actor, ColliderJntSph* jntSph, s32 freezeFlag) {
    s32 i;

    actor->dropFlag = 0x00;
    for (i = jntSph->count - 1; i >= 0; i--) {
        actor->dropFlag |= Actor_GetDropFlag_Oot(actor, &jntSph->elements[i].base, freezeFlag);
    }
}

void EffectSsDeadDb_Spawn_Oot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep,
                              s16 primR, s16 primG, s16 primB, s16 primA, s16 envR, s16 envG, s16 envB, s16 unused,
                              s32 life, s16 playSound) {
    Color_RGBA8 prim = { primR, primG, primB, primA };
    Color_RGBA8 env = { envR, envG, envB, 0 };

    EffectSsDeadDb_Spawn(play, pos, velocity, accel, &prim, &env, scale, scaleStep, life);
    if (playSound) {
        // OoT's flame plays it on its second frame; MM's actors play it with the flame.
        SoundSource_PlaySfxAtFixedWorldPos(play, pos, life, NA_SE_EN_EXTINCT);
    }
}

// MM's, in z_effect_soft_sprite_old_init.c without a prototype in MM's headers.
void EffectSsEnIce_SpawnFlying(PlayState* play, Actor* actor, Vec3f* pos, Color_RGBA8* prim, Color_RGBA8* env,
                               f32 scale);

void EffectSsEnIce_SpawnFlyingVec3f(PlayState* play, Actor* actor, Vec3f* pos, s16 primR, s16 primG, s16 primB,
                                    s16 primA, s16 envR, s16 envG, s16 envB, f32 scale) {
    Color_RGBA8 prim = { primR, primG, primB, primA };
    Color_RGBA8 env = { envR, envG, envB, 0 };

    EffectSsEnIce_SpawnFlying(play, actor, pos, &prim, &env, scale);
}

void func_8002829C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep) {
    EffectSsDust_Spawn(play, 0, pos, velocity, accel, primColor, envColor, scale, scaleStep, 10, 0);
}

Actor* func_80033684(PlayState* play, Actor* explosiveActor) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_EXPLOSIVES].first;

    while (actor != NULL) {
        if ((actor == explosiveActor) || (actor->params != 1)) {
            actor = actor->next;
        } else {
            if (Actor_WorldDistXYZToActor(explosiveActor, actor) <= (actor->shape.rot.z * 10) + 80.0f) {
                return actor;
            } else {
                actor = actor->next;
            }
        }
    }

    return NULL;
}

s32 func_8002DDE4(PlayState* play) {
    Player* player = GET_PLAYER(play);

    return player->stateFlags2 & PLAYER_STATE2_8;
}

s32 func_800354B4(PlayState* play, Actor* actor, f32 range, s16 arg3, s16 arg4, s16 arg5) {
    Player* player = GET_PLAYER(play);
    s16 var1;
    s16 var2;

    var1 = (s16)(actor->yawTowardsPlayer + 0x8000) - player->actor.shape.rot.y;
    var2 = actor->yawTowardsPlayer - arg5;

    if ((actor->xzDistToPlayer <= range) && (player->meleeWeaponState != 0) && (arg4 >= ABS(var1)) &&
        (arg3 >= ABS(var2))) {
        return true;
    } else {
        return false;
    }
}

void func_800355B8(PlayState* play, Vec3f* pos) {
    static Vec3f sVelocity = { 0.0f, -1.5f, 0.0f };
    static Vec3f sAccel = { 0.0f, -0.2f, 0.0f };
    Color_RGBA8 color1 = { 200, 160, 120, 0 };
    Color_RGBA8 color2 = { 130, 90, 50, 0 };

    EffectSsKirakira_SpawnSmall(play, pos, &sVelocity, &sAccel, &color1, &color2);
}

Actor* func_800358DC(Actor* actor, Vec3f* spawnPos, Vec3s* spawnRot, f32* arg3, s32 timer, s16* unused, PlayState* play,
                     s16 params, s32 arg8) {
    EnPart* spawnedEnPart;

    spawnedEnPart = (EnPart*)Actor_SpawnAsChild(&play->actorCtx, actor, play, ACTOR_EN_PART_Oot, spawnPos->x,
                                                spawnPos->y, spawnPos->z, spawnRot->x, spawnRot->y, actor->objectSlot,
                                                params);
    if (spawnedEnPart != NULL) {
        spawnedEnPart->actor.scale = actor->scale;
        spawnedEnPart->actor.speed = arg3[0];
        spawnedEnPart->displayList = arg8;
        spawnedEnPart->action = 2;
        spawnedEnPart->timer = timer;
        spawnedEnPart->rotZ = arg3[1];
        spawnedEnPart->rotZSpeed = arg3[2];
        return &spawnedEnPart->actor;
    }

    return NULL;
}

void func_800359B8(Actor* actor, s16 arg1, Vec3s* arg2) {
    f32 sp44;
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    CollisionPoly* floorPoly;

    if (actor->floorPoly != NULL) {
        floorPoly = actor->floorPoly;
        sp44 = COLPOLY_GET_NORMAL(floorPoly->normal.x);
        sp40 = COLPOLY_GET_NORMAL(floorPoly->normal.y);
        sp3C = COLPOLY_GET_NORMAL(floorPoly->normal.z);

        sp38 = Math_SinS(arg1);
        sp34 = Math_CosS(arg1);
        sp28 = (-(sp44 * sp38) - (sp3C * sp34));
        arg2->x = -(s16)(Math_FAtan2F(sp28 * sp40, 1.0f) * (32768 / M_PI));

        sp2C = Math_SinS(arg1 - 16375);
        sp30 = Math_CosS(arg1 - 16375);
        sp24 = (-(sp44 * sp2C) - (sp3C * sp30));
        arg2->z = -(s16)(Math_FAtan2F(sp24 * sp40, 1.0f) * (32768 / M_PI));
    }
}

Actor* Actor_GetProjectileActor(PlayState* play, Actor* refActor, f32 radius) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
    Vec3f spA8;
    Vec3f sp90;
    Vec3f sp84;

    // MM's Hookshot and arrows. OoT also skipped an arrow whose unk_210.x is 0, which (SoH notes) is about never.
    while (actor != NULL) {
        if (((actor->id != ACTOR_ARMS_HOOK) && (actor->id != ACTOR_EN_ARROW)) || (actor == refActor) ||
            (Math_Vec3f_DistXYZ(&refActor->world.pos, &actor->world.pos) > radius) ||
            ((actor->id == ACTOR_ARMS_HOOK) && (((ArmsHook*)actor)->timer == 0))) {
            actor = actor->next;
            continue;
        }
        spA8.x = actor->world.pos.x + Math_SinS(actor->world.rot.y) * (actor->speed * 10.0f);
        spA8.y = actor->world.pos.y + actor->velocity.y + (actor->gravity * 10.0f);
        spA8.z = actor->world.pos.z + Math_CosS(actor->world.rot.y) * (actor->speed * 10.0f);
        if (CollisionCheck_CylSideVsLineSeg(refActor->colChkInfo.cylRadius, refActor->colChkInfo.cylHeight, 0.0f,
                                            &refActor->world.pos, &actor->world.pos, &spA8, &sp90, &sp84)) {
            return actor;
        }
        actor = actor->next;
    }

    return NULL;
}

void EffectSsEnIce_SpawnFlyingVec3s(PlayState* play, Actor* actor, Vec3s* pos, s16 primR, s16 primG, s16 primB,
                                    s16 primA, s16 envR, s16 envG, s16 envB, f32 scale) {
    Vec3f posF = { pos->x, pos->y, pos->z };

    EffectSsEnIce_SpawnFlyingVec3f(play, actor, &posF, primR, primG, primB, primA, envR, envG, envB, scale);
}

void EffectSsDeadSound_SpawnStationary(PlayState* play, Vec3f* pos, u16 sfxId, s16 lowerPriority, s16 repeatMode,
                                       s32 life) {
    // OoT's callers pass the actor's projected position, which the effect keeps after the actor is gone; the sound
    // system holds on to the pointer, so it plays from a copy.
    static Vec3f sPositions[8];
    static s32 sNext = 0;
    Vec3f* copy = &sPositions[sNext];

    sNext = (sNext + 1) % ARRAY_COUNT(sPositions);
    Math_Vec3f_Copy(copy, pos);
    Audio_PlaySfx_AtPos(copy, sfxId);
}

void BodyBreak_Alloc(BodyBreak* bodyBreak, s32 count, PlayState* play) {
    u32 matricesSize;
    u32 dListsSize;
    u32 objectIdsSize;

    matricesSize = (count + 1) * sizeof(*bodyBreak->matrices);
    bodyBreak->matrices = ZeldaArena_Malloc(matricesSize);

    if (bodyBreak->matrices != NULL) {
        dListsSize = (count + 1) * sizeof(*bodyBreak->dLists);
        bodyBreak->dLists = ZeldaArena_Malloc(dListsSize);

        if (bodyBreak->dLists != NULL) {
            objectIdsSize = (count + 1) * sizeof(*bodyBreak->objectIds);
            bodyBreak->objectIds = ZeldaArena_Malloc(objectIdsSize);

            if (bodyBreak->objectIds != NULL) {
                memset((u8*)bodyBreak->matrices, 0, matricesSize);
                memset((u8*)bodyBreak->dLists, 0, dListsSize);
                memset((u8*)bodyBreak->objectIds, 0, objectIdsSize);
                bodyBreak->val = 1;
                return;
            }
        }
    }

    if (bodyBreak->matrices != NULL) {
        ZeldaArena_Free(bodyBreak->matrices);
    }
    if (bodyBreak->dLists != NULL) {
        ZeldaArena_Free(bodyBreak->dLists);
    }
    if (bodyBreak->objectIds != NULL) {
        ZeldaArena_Free(bodyBreak->objectIds);
    }
}

void BodyBreak_SetInfo(BodyBreak* bodyBreak, s32 limbIndex, s32 minLimbIndex, s32 maxLimbIndex, u32 count, Gfx** dList,
                       s16 objectId) {
    PlayState* play = Effect_GetPlayState();

    if ((play->actorCtx.freezeFlashTimer == 0) && (bodyBreak->val > 0)) {
        if ((limbIndex >= minLimbIndex) && (limbIndex <= maxLimbIndex) && (*dList != NULL)) {
            bodyBreak->dLists[bodyBreak->val] = *dList;
            Matrix_Get(&bodyBreak->matrices[bodyBreak->val]);
            bodyBreak->objectIds[bodyBreak->val] = objectId;
            bodyBreak->val++;
        }

        if (limbIndex != bodyBreak->prevLimbIndex) {
            bodyBreak->count++;
        }

        if ((u32)bodyBreak->count >= count) {
            bodyBreak->count = bodyBreak->val - 1;
            bodyBreak->val = BODYBREAK_STATUS_READY;
        }
    }

    bodyBreak->prevLimbIndex = limbIndex;
}

s32 BodyBreak_SpawnParts(Actor* actor, BodyBreak* bodyBreak, PlayState* play, s16 type) {
    EnPart* spawnedEnPart;
    MtxF* mtx;
    s16 objectSlot;

    if (bodyBreak->val != BODYBREAK_STATUS_READY) {
        return false;
    }

    while (bodyBreak->count > 0) {
        Matrix_Put(&bodyBreak->matrices[bodyBreak->count]);
        Matrix_Scale(1.0f / actor->scale.x, 1.0f / actor->scale.y, 1.0f / actor->scale.z, MTXMODE_APPLY);
        Matrix_Get(&bodyBreak->matrices[bodyBreak->count]);

        if (bodyBreak->objectIds[bodyBreak->count] >= 0) {
            objectSlot = bodyBreak->objectIds[bodyBreak->count];
        } else {
            objectSlot = actor->objectSlot;
        }

        mtx = &bodyBreak->matrices[bodyBreak->count];

        spawnedEnPart = (EnPart*)Actor_SpawnAsChild(&play->actorCtx, actor, play, ACTOR_EN_PART_Oot, mtx->xw, mtx->yw,
                                                    mtx->zw, 0, 0, objectSlot, type);

        if (spawnedEnPart != NULL) {
            Matrix_MtxFToYXZRot(&bodyBreak->matrices[bodyBreak->count], &spawnedEnPart->actor.shape.rot, false);
            spawnedEnPart->displayList = bodyBreak->dLists[bodyBreak->count];
            spawnedEnPart->actor.scale = actor->scale;
        }

        bodyBreak->count--;
    }

    bodyBreak->val = BODYBREAK_STATUS_FINISHED;

    ZeldaArena_Free(bodyBreak->matrices);
    ZeldaArena_Free(bodyBreak->dLists);
    ZeldaArena_Free(bodyBreak->objectIds);

    return true;
}

void SkelAnime_DrawSkeletonOpaOot(PlayState* play, SkelAnime* skelAnime, OverrideLimbDrawOpa overrideLimbDraw,
                                  PostLimbDrawOpa postLimbDraw, void* arg) {
    if (skelAnime->dListCount != 0) {
        SkelAnime_DrawFlexOpa(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                              overrideLimbDraw, postLimbDraw, arg);
    } else {
        SkelAnime_DrawOpa(play, skelAnime->skeleton, skelAnime->jointTable, overrideLimbDraw, postLimbDraw, arg);
    }
}

Gfx* SkelAnime_DrawSkeleton2Oot(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                                PostLimbDraw postLimbDraw, void* arg, Gfx* gfx) {
    if (skelAnime->dListCount != 0) {
        return SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                                  overrideLimbDraw, postLimbDraw, arg, gfx);
    }
    return SkelAnime_Draw(play, skelAnime->skeleton, skelAnime->jointTable, overrideLimbDraw, postLimbDraw, arg, gfx);
}

Gfx D_80116280_Oot[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU |
                                            FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
};

Actor* Actor_FindOot(ActorContext* actorCtx, s32 actorId, s32 actorCategory) {
    Actor* actor = actorCtx->actorLists[actorCategory].first;

    while (actor != NULL) {
        if (actorId == actor->id) {
            return actor;
        }
        actor = actor->next;
    }
    return NULL;
}

void EffectSsDust_SpawnDrawFlag2Oot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                                    Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life) {
    EffectSsDust_Spawn(play, 2, pos, velocity, accel, primColor, envColor, scale, scaleStep, life, 0);
}

void Math_Vec3fDiffToRotOot(Vec3f* from, Vec3f* to, Vec3s* rot, s32 upIsPositive) {
    f32 dx = to->x - from->x;
    f32 dz = to->z - from->z;
    f32 dy = upIsPositive ? (to->y - from->y) : (from->y - to->y);

    rot->y = Math_Atan2S(dz, dx);
    rot->x = Math_Atan2S(sqrtf(SQ(dx) + SQ(dz)), dy);
}

Actor* Actor_GetCollidedExplosiveOot(PlayState* play, Collider* collider) {
    if ((collider->acFlags & AC_HIT) && (collider->ac->category == ACTORCAT_EXPLOSIVES)) {
        collider->acFlags &= ~AC_HIT;
        return collider->ac;
    }
    return NULL;
}

void Actor_SetTextWithPrefixOot(PlayState* play, Actor* actor, s16 baseTextId) {
    s16 prefix;

    // By OoT's scene id.
    switch (Hyrule_GetOotSceneId(play->sceneId)) {
        case 0x00: // SCENE_DEKU_TREE
        case 0x11: // SCENE_DEKU_TREE_BOSS
        case 0x14: // SCENE_FOREST_TEMPLE_BOSS
        case 0x26: // SCENE_KNOW_IT_ALL_BROS_HOUSE
        case 0x27: // SCENE_TWINS_HOUSE
        case 0x28: // SCENE_MIDOS_HOUSE
        case 0x29: // SCENE_SARIAS_HOUSE
        case 0x2D: // SCENE_KOKIRI_SHOP
        case 0x34: // SCENE_LINKS_HOUSE
        case 0x55: // SCENE_KOKIRI_FOREST
        case 0x56: // SCENE_SACRED_FOREST_MEADOW
        case 0x5B: // SCENE_LOST_WOODS
        case 112:
            prefix = 0x1000;
            break;
        case 0x36: // SCENE_STABLE
        case 0x51: // SCENE_HYRULE_FIELD
        case 0x63: // SCENE_LON_LON_RANCH
            prefix = 0x2000;
            break;
        case 0x04: // SCENE_FIRE_TEMPLE
        case 0x12: // SCENE_DODONGOS_CAVERN_BOSS
        case 0x15: // SCENE_FIRE_TEMPLE_BOSS
        case 0x60: // SCENE_DEATH_MOUNTAIN_TRAIL
        case 0x61: // SCENE_DEATH_MOUNTAIN_CRATER
        case 0x62: // SCENE_GORON_CITY
            prefix = 0x3000;
            break;
        case 0x02: // SCENE_JABU_JABU
        case 0x13: // SCENE_JABU_JABU_BOSS
        case 0x54: // SCENE_ZORAS_RIVER
        case 0x58: // SCENE_ZORAS_DOMAIN
        case 0x59: // SCENE_ZORAS_FOUNTAIN
            prefix = 0x4000;
            break;
        case 0x07: // SCENE_SHADOW_TEMPLE
        case 0x18: // SCENE_SHADOW_TEMPLE_BOSS
        case 0x2A: // SCENE_KAKARIKO_CENTER_GUEST_HOUSE
        case 0x2B: // SCENE_BACK_ALLEY_HOUSE
        case 0x35: // SCENE_DOG_LADY_HOUSE
        case 0x3A: // SCENE_GRAVEKEEPERS_HUT
        case 0x3F: // SCENE_REDEAD_GRAVE
        case 0x48: // SCENE_WINDMILL_AND_DAMPES_GRAVE
        case 0x52: // SCENE_KAKARIKO_VILLAGE
        case 0x53: // SCENE_GRAVEYARD
            prefix = 0x5000;
            break;
        case 0x06: // SCENE_SPIRIT_TEMPLE
        case 0x17: // SCENE_SPIRIT_TEMPLE_BOSS
        case 0x37: // SCENE_IMPAS_HOUSE
        case 0x39: // SCENE_CARPENTERS_TENT
        case 0x57: // SCENE_LAKE_HYLIA
        case 0x5A: // SCENE_GERUDO_VALLEY
        case 0x5C: // SCENE_DESERT_COLOSSUS
            prefix = 0x6000;
            break;
        case 0x1B: // SCENE_MARKET_ENTRANCE_DAY
        case 0x1E: // SCENE_BACK_ALLEY_DAY
        case 0x1F: // SCENE_BACK_ALLEY_NIGHT
        case 0x20: // SCENE_MARKET_DAY
        case 0x21: // SCENE_MARKET_NIGHT
        case 0x22: // SCENE_MARKET_RUINS
        case 0x5F: // SCENE_HYRULE_CASTLE
            prefix = 0x7000;
            break;
        default:
            prefix = 0x0000;
            break;
    }
    actor->textId = TEXT_OOT(prefix | baseTextId);
}

void EffectSsDeadDd_SpawnOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale, s16 scaleStep,
                             s16 primR, s16 primG, s16 primB, s16 primA, s16 envR, s16 envG, s16 envB, s16 alphaStep,
                             s32 life) {
    Color_RGBA8 prim = { primR, primG, primB, primA };
    Color_RGBA8 env = { envR, envG, envB, 255 };

    EffectSsDeadDd_Spawn(play, pos, velocity, accel, &prim, &env, scale, scaleStep, alphaStep, life);
}

void EffectSsDeadDs_SpawnStationaryOot(PlayState* play, Vec3f* pos, s16 scale, s16 scaleStep, s16 alpha, s32 life) {
    static Vec3f sZero = { 0.0f, 0.0f, 0.0f };

    EffectSsDeadDs_Spawn(play, pos, &sZero, &sZero, scale, scaleStep, alpha, life);
}

void EffectSsFCircle_SpawnOot(PlayState* play, Actor* actor, Vec3f* pos, s16 radius, s16 height) {
}

Gfx* Gfx_TexScrollOot(GraphicsContext* gfxCtx, u32 x, u32 y) {
    return Gfx_TexScroll(gfxCtx, x, y, 0, 0);
}

void Hyrule_TriggerRespawnOot(PlayState* play) {
    // OoT sends Link back to where he entered the scene; MM's respawn is where he entered the room.
    func_80169EFC(play);
}

u8 CollisionCheck_GetSwordDamageOot(u64 dmgFlags, PlayState* play) {
    // OoT's bits are the upper half of 3Ship's flags (z64collision_check.h).
    u32 ootFlags = (u32)(dmgFlags >> 32);

    if (ootFlags & 0x00400100) {
        return 1;
    } else if (ootFlags & 0x03000242) {
        return 2;
    } else if (ootFlags & 0x48800400) {
        return 4;
    } else if (ootFlags & 0x04000000) {
        return 8;
    }
    return 0;
}

void Cutscene_StartManualOot(PlayState* play) {
    gCsCtxOot.state = CS_STATE_SKIPPABLE_INIT_Oot;
    gCsCtxOot.linkAction = NULL;
}

void Cutscene_StopManualOot(PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_UNSKIPPABLE_EXEC_Oot) {
        gCsCtxOot.state = CS_STATE_UNSKIPPABLE_INIT_Oot;
    }
}

s32 OnePointCutscene_AttentionSetSfxOot(PlayState* play, Actor* actor, s32 sfxId) {
    // OoT plays the sound as its attention camera reaches the actor; without that camera, it plays now.
    Audio_PlaySfx(sfxId);
    return CAM_ID_NONE;
}

void EffectSsFhgFlash_SpawnLightBallOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale,
                                        u8 param) {
    // MM keeps OoT's effect, but only its shock spawner.
    EffectSsFhgFlashInitParams initParams;

    Math_Vec3f_Copy(&initParams.pos, pos);
    Math_Vec3f_Copy(&initParams.velocity, velocity);
    Math_Vec3f_Copy(&initParams.accel, accel);
    initParams.scale = scale;
    initParams.params = param;
    initParams.type = FHGFLASH_LIGHTBALL;
    EffectSs_Spawn(play, EFFECT_SS_FHG_FLASH, 128, &initParams);
}

void Gfx_SetupDL25_OpaRoomOot(PlayState* play) {
    GraphicsContext* gfxCtx = play->state.gfxCtx;

    Gfx_SetupDL25_Opa(gfxCtx);
    // OoT's room behavior type 3 (MM's Room.type) turns color dither off.
    if (play->roomCtx.curRoom.type == 3) {
        OPEN_DISPS(gfxCtx);
        gDPSetColorDither(POLY_OPA_DISP++, G_CD_DISABLE);
        CLOSE_DISPS(gfxCtx);
    }
}

s32 Actor_IsHitByHammerForwardOot(PlayState* play, Collider* collider) {
    // OoT's func_800355E4: a forward Megaton Hammer swing (3Ship's swing is MM's backslash, HyrulePlayer.c).
    Player* player = GET_PLAYER(play);

    return (collider->acFlags & AC_TYPE_PLAYER) && (player->meleeWeaponState != 0) &&
           (player->heldItemAction == PLAYER_IA_HAMMER_Oot) &&
           (player->meleeWeaponAnimation == PLAYER_MWA_HAMMER_FORWARD_Oot);
}

void Actor_SwapHookshotAttachmentOot(PlayState* play, Actor* actorA, Actor* actorB) {
    ArmsHook* hookshot = (ArmsHook*)Actor_FindOot(&play->actorCtx, ACTOR_ARMS_HOOK, ACTORCAT_ITEMACTION);

    if (hookshot != NULL) {
        hookshot->attachedActor = actorB;
        hookshot->attachPointOffset.x = hookshot->attachPointOffset.y = hookshot->attachPointOffset.z = 0.0f;
    }
    actorB->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED;
    actorA->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
}

s32 Math_StepUntilSOot(s16* pValue, s16 limit, s16 step) {
    s16 orig = *pValue;

    *pValue += step;
    if (((*pValue - limit) * (orig - limit)) <= 0) {
        *pValue = limit;
        return true;
    }
    return false;
}

void func_8003555C(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel) {
    // OoT leaves the alphas unset (a bug); opaque here.
    Color_RGBA8 primColor = { 200, 160, 120, 255 };
    Color_RGBA8 envColor = { 130, 90, 50, 255 };

    EffectSsKirakira_SpawnSmall(play, pos, velocity, accel, &primColor, &envColor);
}

void func_800283D4(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, Color_RGBA8* primColor,
                   Color_RGBA8* envColor, s16 scale, s16 scaleStep, s16 life) {
    EffectSsDust_Spawn(play, 1, pos, velocity, accel, primColor, envColor, scale, scaleStep, life, 0);
}

f32 func_80033AEC(Vec3f* arg0, Vec3f* arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5) {
    f32 ret = 0.0f;

    if ((arg4 <= Math_Vec3f_DistXYZ(arg0, arg1)) || (arg5 < Math_Vec3f_DistXYZ(arg0, arg1))) {
        ret = Math_SmoothStepToF(&arg1->x, arg0->x, arg2, arg3, 0.0f);
        ret += Math_SmoothStepToF(&arg1->y, arg0->y, arg2, arg3, 0.0f);
        ret += Math_SmoothStepToF(&arg1->z, arg0->z, arg2, arg3, 0.0f);
    }
    return ret;
}

void EffectSsDFire_SpawnFixedScaleOot(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 alpha,
                                      s16 fadeDelay) {
    // OoT's fades by 5 a frame once its life is down to fadeDelay; MM's once fewer than (life - fadeDelay)
    // frames are left. OoT's also fades in by 15 a frame before that, which MM's does not.
    EffectSsDFire_Spawn(play, pos, velocity, accel, 100, 35, alpha, 5, 8 - fadeDelay - 1, 8);
}

void EffectSsGMagma_SpawnOot(PlayState* play, Vec3f* pos) {
}

void EffectSsGMagma2_SpawnOot(PlayState* play, Vec3f* pos, Color_RGBA8* primColor, Color_RGBA8* envColor,
                              s16 updateRate, s16 drawMode, s16 scale) {
}

u16 func_80037C30(PlayState* play, s16 npcId) {
    switch (npcId) {
        case 0xF: // the Deku Sprout
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_5C_Oot)) {
                return TEXT_OOT(0x1078);
            }
            return GET_INFTABLE_OOT(INFTABLE_66_Oot) ? TEXT_OOT(0x1066) : TEXT_OOT(0x1062);
        default:
            return 0;
    }
}

// OoT's func_80036E50: what a closed text leaves set.
static void Npc_OnTextClosedOot(u16 textId, s16 npcId) {
    if ((npcId == 0xF) && (textId == TEXT_OOT(0x1062))) {
        SET_INFTABLE_OOT(INFTABLE_66_Oot);
    }
}

s32 func_80037D98(PlayState* play, Actor* actor, s16 npcId, s32* talkState) {
    s16 screenX;
    s16 screenY;
    s16 yawDiff;

    if (Actor_TalkOfferAccepted(actor, &play->state)) {
        *talkState = 1;
        return true;
    }

    if (*talkState == 1) {
        // OoT's func_80037CB8: the talk ends when the text closes (the Deku Sprout's texts ask nothing).
        if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
            Npc_OnTextClosedOot(actor->textId, npcId);
            *talkState = 0;
        }
        return false;
    }

    Actor_GetScreenPos(play, actor, &screenX, &screenY);
    if ((screenX < 0) || (screenX > SCREEN_WIDTH) || (screenY < 0) || (screenY > SCREEN_HEIGHT)) {
        return false;
    }
    yawDiff = actor->yawTowardsPlayer - actor->shape.rot.y;
    if (ABS(yawDiff) >= 0x4300) {
        return false;
    }
    if ((actor->xyzDistToPlayerSq > SQ(160.0f)) && !actor->isLockedOn) {
        return false;
    }
    if (actor->xyzDistToPlayerSq <= SQ(80.0f)) {
        if (Actor_OfferTalk(actor, play, 80.0f)) {
            actor->textId = func_80037C30(play, npcId);
        }
    } else if (Actor_OfferTalkNearColChkInfoCylinder(actor, play)) {
        actor->textId = func_80037C30(play, npcId);
    }
    return false;
}

s32 func_800C0808(PlayState* play, s16 camId, Player* player, s16 setting) {
    Camera* camera = Play_GetCamera(play, camId);

    Camera_InitFocalActorSettings(camera, &player->actor);
    return Camera_ChangeSetting(camera, setting);
}
