/*
 * File: z_en_ganon_organ.c
 * Overlay: ovl_En_Ganon_Organ
 * Description: The organ that Ganondorf plays in the cutscene before the fight. Includes carpet and scenery as well.
 */

#include "z_en_ganon_organ.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "../ovl_Boss_Ganon/z_boss_ganon.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EnGanonOrgan_Init(Actor* thisx, PlayState* play);
static void EnGanonOrgan_Destroy(Actor* thisx, PlayState* play);
static void EnGanonOrgan_Update(Actor* thisx, PlayState* play);
static void EnGanonOrgan_Draw(Actor* thisx, PlayState* play);

ActorProfile En_Ganon_Organ_Oot_Profile = {
    /**/ ACTOR_EN_GANON_ORGAN_Oot,
    /**/ ACTORCAT_BOSS,
    /**/ FLAGS,
    /**/ OBJECT_GANON_Oot,
    /**/ sizeof(EnGanonOrgan),
    /**/ EnGanonOrgan_Init,
    /**/ EnGanonOrgan_Destroy,
    /**/ EnGanonOrgan_Update,
    /**/ EnGanonOrgan_Draw,
};

// static u64 sForceAlignment = 0;

#include "assets/oot/overlays/ovl_En_Ganon_Organ/ovl_En_Ganon_Organ.h"

static void EnGanonOrgan_Init(Actor* thisx, PlayState* play) {
    thisx->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
}

static void EnGanonOrgan_Destroy(Actor* thisx, PlayState* play) {
}

static void EnGanonOrgan_Update(Actor* thisx, PlayState* play) {
    BossGanon* dorf;

    if (thisx->params == 1) {
        dorf = (BossGanon*)thisx->parent;
        if (dorf->organAlpha == 0) {
            Actor_Kill(thisx);
        }
    }
}

static Gfx* EnGanonOrgan_EmptyDList(GraphicsContext* gfxCtx) {
    Gfx* displayList;

    displayList = GRAPH_ALLOC(gfxCtx, sizeof(Gfx));
    gSPEndDisplayList(displayList);
    return displayList;
}

static Gfx* func_80A280BC(GraphicsContext* gfxCtx, BossGanon* dorf) {
    Gfx* displayList;
    Gfx* displayListHead;

    displayList = GRAPH_ALLOC(gfxCtx, 4 * sizeof(Gfx));
    displayListHead = displayList;
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 25, 20, 0, dorf->organAlpha);
    gDPSetRenderMode(displayListHead++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2);
    gSPEndDisplayList(displayListHead);
    return displayList;
}

static Gfx* func_80A28148(GraphicsContext* gfxCtx, BossGanon* dorf) {
    Gfx* displayList;
    Gfx* displayListHead;

    displayList = GRAPH_ALLOC(gfxCtx, 4 * sizeof(Gfx));
    displayListHead = displayList;

    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 0, 0, 0, dorf->organAlpha);
    gDPSetRenderMode(displayListHead++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2);
    gSPEndDisplayList(displayListHead);
    return displayList;
}

static void EnGanonOrgan_Draw(Actor* thisx, PlayState* play) {
    BossGanon* dorf = (BossGanon*)thisx->parent;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    if ((thisx->params == 1) && (dorf->organAlpha != 255)) {
        gSPSegment(POLY_OPA_DISP++, 0x08, func_80A280BC(play->state.gfxCtx, dorf));
        gSPSegment(POLY_OPA_DISP++, 0x09, func_80A28148(play->state.gfxCtx, dorf));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08, EnGanonOrgan_EmptyDList(play->state.gfxCtx));
        gSPSegment(POLY_OPA_DISP++, 0x09, EnGanonOrgan_EmptyDList(play->state.gfxCtx));
    }
    Matrix_Translate(0.0f, 0.0f, 0.0f, MTXMODE_NEW);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_OPA_DISP++, sRoomOrganAndFloorDL);
    gSPDisplayList(POLY_OPA_DISP++, sRoomStatuesDL);


    CLOSE_DISPS(play->state.gfxCtx);
}
