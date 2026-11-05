/*
 * File: z_en_blkobj.c
 * Overlay: ovl_En_Blkobj
 * Description: Dark Link's Illusion Room
 */

#include "z_en_blkobj.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_blkobj/object_blkobj.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EnBlkobj_Init(Actor* thisx, PlayState* play);
static void EnBlkobj_Destroy(Actor* thisx, PlayState* play);
static void EnBlkobj_Update(Actor* thisx, PlayState* play);
static void EnBlkobj_Draw(Actor* thisx, PlayState* play);

static void EnBlkobj_Wait(EnBlkobj* this, PlayState* play);
static void EnBlkobj_SpawnDarkLink(EnBlkobj* this, PlayState* play);
static void EnBlkobj_DarkLinkFight(EnBlkobj* this, PlayState* play);
static void EnBlkobj_DoNothing(EnBlkobj* this, PlayState* play);

ActorProfile En_Blkobj_Oot_Profile = {
    /**/ ACTOR_EN_BLKOBJ_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_BLKOBJ_Oot,
    /**/ sizeof(EnBlkobj),
    /**/ EnBlkobj_Init,
    /**/ EnBlkobj_Destroy,
    /**/ EnBlkobj_Update,
    /**/ EnBlkobj_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F(scale, 1, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 800, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 300, ICHAIN_STOP),
};

static Gfx sSetupOpaDL[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPEndDisplayList(),
};

static Gfx sSetupXluDL[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2),
    gsSPEndDisplayList(),
};

static void EnBlkobj_SetupAction(EnBlkobj* this, EnBlkobjActionFunc actionFunc) {
    this->actionFunc = actionFunc;
    this->timer = 0;
}

static void EnBlkobj_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnBlkobj* this = (EnBlkobj*)thisx;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    if (Flags_GetClear(play, this->dyna.actor.room)) {
        this->alpha = 255;
        EnBlkobj_SetupAction(this, EnBlkobj_DoNothing);
    } else {
        CollisionHeader_GetVirtual(&gIllusionRoomCol, &colHeader);
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
        EnBlkobj_SetupAction(this, EnBlkobj_Wait);
    }
}

static void EnBlkobj_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    EnBlkobj* this = (EnBlkobj*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void EnBlkobj_Wait(EnBlkobj* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->dyna.actor.xzDistToPlayer < 120.0f) {
        EnBlkobj_SetupAction(this, EnBlkobj_SpawnDarkLink);
    }
    player->stateFlags2 |= PLAYER_STATE2_4000000;
}

static void EnBlkobj_SpawnDarkLink(EnBlkobj* this, PlayState* play) {
    if ((!(this->dyna.actor.flags & ACTOR_FLAG_INSIDE_CULLING_VOLUME))) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_TORCH2_Oot, this->dyna.actor.world.pos.x, this->dyna.actor.world.pos.y,
                    this->dyna.actor.world.pos.z, 0, this->dyna.actor.yawTowardsPlayer, 0, 0);
        EnBlkobj_SetupAction(this, EnBlkobj_DarkLinkFight);
    }
}

static void EnBlkobj_DarkLinkFight(EnBlkobj* this, PlayState* play) {
    s32 alphaMod;

    if (this->timer == 0) {
        // Dark Link room completed.
        // Check for if Dark Link is defeated in authentic gameplay.
        // Check for if all enemies are defeated with enemy randomizer or crowd control on.
        uint8_t roomCleared = (!0 &&
                               !(0) &&
                               Actor_FindOot(&play->actorCtx, ACTOR_EN_TORCH2_Oot, ACTORCAT_BOSS) == NULL) ||
                              ((0 ||
                                (0)) &&
                               Flags_GetClearTemp(play, this->dyna.actor.room));
        if (roomCleared) {
            Flags_SetClear(play, this->dyna.actor.room);
            this->timer++;
        }
    } else if (this->timer++ > 100) {
        alphaMod = (this->timer - 100) >> 2;
        if (alphaMod > 5) {
            alphaMod = 5;
        }
        this->alpha += alphaMod;
        if (this->alpha > 255) {
            this->alpha = 255;
            EnBlkobj_SetupAction(this, EnBlkobj_DoNothing);
            DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
        }
    }
}

static void EnBlkobj_DoNothing(EnBlkobj* this, PlayState* play) {
}

static void EnBlkobj_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnBlkobj* this = (EnBlkobj*)thisx;

    this->actionFunc(this, play);
}

static void EnBlkobj_DrawAlpha(PlayState* play, Gfx* dList, s32 alpha) {
    Gfx* segment;

    OPEN_DISPS(play->state.gfxCtx);

    if (alpha == 255) {
        segment = sSetupOpaDL;
    } else {
        segment = sSetupXluDL;
    }

    gSPSegment(POLY_XLU_DISP++, 0x08, segment);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, alpha);
    gSPDisplayList(POLY_XLU_DISP++, dList);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnBlkobj_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    EnBlkobj* this = (EnBlkobj*)thisx;
    s32 illusionAlpha;
    u32 gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gameplayFrames = play->gameplayFrames % 128;

    gSPSegment(
        POLY_XLU_DISP++, 0x0D,
        Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, gameplayFrames, 0, 32, 32, 1, gameplayFrames, 0, 32, 32, 1, 0, 1, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    if (this->alpha != 0) {
        EnBlkobj_DrawAlpha(play, gIllusionRoomNormalDL, this->alpha);
    }
    illusionAlpha = 255 - this->alpha;
    if (illusionAlpha != 0) {
        EnBlkobj_DrawAlpha(play, gIllusionRoomIllusionDL, illusionAlpha);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
