// OoT's actor helpers from code (z_actor.c) that MM does not have. Ported from SoH.

#include "HyruleActor.h"
#include "HyruleInventory.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"

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
