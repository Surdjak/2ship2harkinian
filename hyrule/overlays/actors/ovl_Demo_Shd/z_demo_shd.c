/*
 * File: z_demo_shd.c
 * Overlay: Demo_Shd
 * Description: Bongo Bongo's Shadow
 */

#include "z_demo_shd.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void DemoShd_Init(Actor* thisx, PlayState* play);
static void DemoShd_Destroy(Actor* thisx, PlayState* play);
static void DemoShd_Update(Actor* thisx, PlayState* play);
static void DemoShd_Draw(Actor* thisx, PlayState* play);

static void func_80991298(DemoShd* this, PlayState* play);

ActorProfile Demo_Shd_Oot_Profile = {
    /**/ ACTOR_DEMO_SHD_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(DemoShd),
    /**/ DemoShd_Init,
    /**/ DemoShd_Destroy,
    /**/ DemoShd_Update,
    /**/ DemoShd_Draw,
};

#include "assets/oot/overlays/ovl_Demo_Shd/ovl_Demo_Shd.h"

static void DemoShd_SetupAction(DemoShd* this, DemoShdActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void DemoShd_Init(Actor* thisx, PlayState* play) {
    DemoShd* this = (DemoShd*)thisx;

    this->unk_14C = 0;
    DemoShd_SetupAction(this, func_80991298);
    Actor_SetScale(&this->actor, 0.4f);
    this->actor.world.pos.y = 0.0f;
    this->actor.world.pos.x = 0.0f;
}

static void DemoShd_Destroy(Actor* thisx, PlayState* play) {
}

static void func_80991298(DemoShd* this, PlayState* play) {
    if ((gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[0] != NULL) ||
        (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[1] != NULL)) {
        if (gCsCtxOot.frames == 800) {
            Hyrule_PlayCutsceneEffectsSeqOot(9);
        }
        if (gCsCtxOot.frames == 1069) {
            Hyrule_PlayCutsceneEffectsSeqOot(8);
        }
    }

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction0 = gCsCtxOot.npcActions[0];

        if (npcAction0 != NULL) {
            if (npcAction0->action == 2) {
                if (!(this->unk_14C & 1)) {
                    this->unk_14E = npcAction0->startPos.x;
                }
                this->unk_14C |= 1;
            } else {
                this->unk_14C &= ~1;
            }
        }
    }

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction1 = gCsCtxOot.npcActions[1];

        if (npcAction1 != NULL) {
            if (npcAction1->action == 2) {
                if (!(this->unk_14C & 2)) {
                    this->unk_14E = npcAction1->startPos.x;
                }
                this->unk_14C |= 2;
            } else {
                this->unk_14C &= ~2;
            }
        }
    }

    this->unk_14E++;
}

static void DemoShd_Update(Actor* thisx, PlayState* play) {
    DemoShd* this = (DemoShd*)thisx;

    this->actionFunc(this, play);
}

static void DemoShd_Draw(Actor* thisx, PlayState* play) {
    DemoShd* this = (DemoShd*)thisx;
    s32 pad;
    u32 unk_14E = this->unk_14E;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, D_809932D0);

    if (this->unk_14C & 1) {
        gSPDisplayList(POLY_XLU_DISP++,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0x3FF - ((unk_14E * 5) & 0x3FF), 16, 256, 1, 0,
                                          255 - ((unk_14E * 5) & 255), 32, 32, 0, -5, 0, -5));
        gSPDisplayList(POLY_XLU_DISP++, D_80993390);
    } else if (this->unk_14C & 2) {
        gSPDisplayList(POLY_XLU_DISP++,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0x3FF - ((unk_14E * 5) & 0x3FF), 16, 256, 1, 0,
                                          255 - ((unk_14E * 5) & 255), 32, 32, 0, -5, 0, -5));
        gSPDisplayList(POLY_XLU_DISP++, D_809934B8);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
