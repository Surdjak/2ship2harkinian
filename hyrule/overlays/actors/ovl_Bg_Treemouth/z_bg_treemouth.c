/*
 * File: z_bg_treemouth.c
 * Overlay: ovl_Bg_Treemouth
 * Description: Great Deku Tree's Mouth
 */

#include "z_bg_treemouth.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_spot04_objects/object_spot04_objects.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgTreemouth_Init(Actor* thisx, PlayState* play);
static void BgTreemouth_Destroy(Actor* thisx, PlayState* play);
static void BgTreemouth_Update(Actor* thisx, PlayState* play);
static void BgTreemouth_Draw(Actor* thisx, PlayState* play);

static void func_808BC65C(BgTreemouth* this, PlayState* play);
static void func_808BC6F8(BgTreemouth* this, PlayState* play);
static void func_808BC80C(BgTreemouth* this, PlayState* play);
static void func_808BC864(BgTreemouth* this, PlayState* play);
static void BgTreemouth_DoNothing(BgTreemouth* this, PlayState* play);
static void func_808BC8B8(BgTreemouth* this, PlayState* play);
static void func_808BC9EC(BgTreemouth* this, PlayState* play);
static void func_808BCAF0(BgTreemouth* this, PlayState* play);

extern CutsceneData D_808BCE20[];
extern CutsceneData D_808BD2A0[];
extern CutsceneData D_808BD520[];
extern CutsceneData D_808BD790[];

ActorProfile Bg_Treemouth_Oot_Profile = {
    /**/ ACTOR_BG_TREEMOUTH_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SPOT04_OBJECTS_Oot,
    /**/ sizeof(BgTreemouth),
    /**/ BgTreemouth_Init,
    /**/ BgTreemouth_Destroy,
    /**/ BgTreemouth_Update,
    /**/ BgTreemouth_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 5, ICHAIN_CONTINUE),
    ICHAIN_VEC3F(scale, 1, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 8000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 300, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 300, ICHAIN_STOP),
};

// unused
static f32 D_808BD9C4[] = {
    -2746.0f, 545.0f, 4694.0f, -2654.0f, 146.0f, 4534.0f,
};

static void BgTreemouth_SetupAction(BgTreemouth* this, BgTreemouthActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgTreemouth_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgTreemouth* this = (BgTreemouth*)thisx;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(thisx, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gDekuTreeMouthCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    ActorShape_Init(&thisx->shape, 0.0f, NULL, 0.0f);
    Actor_SetFocus(thisx, 50.0f);

    if ((gSaveContext.sceneLayer < 4) && !LINK_IS_ADULT_OOT) {
        BgTreemouth_SetupAction(this, func_808BC8B8);
    } else if (LINK_IS_ADULT_OOT || (gSaveContext.sceneLayer == 7)) {
        this->unk_168 = 0.0f;
        BgTreemouth_SetupAction(this, BgTreemouth_DoNothing);
    } else {
        this->unk_168 = 1.0f;
        BgTreemouth_SetupAction(this, func_808BC6F8);
    }

    thisx->textId = 0x905;
}

static void BgTreemouth_Destroy(Actor* thisx, PlayState* play) {
    BgTreemouth* this = (BgTreemouth*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_808BC65C(BgTreemouth* this, PlayState* play) {
    CsCmdActorCueOot* npcAction;

    if ((gCsCtxOot.state != CS_STATE_IDLE_Oot)) {
        npcAction = gCsCtxOot.npcActions[0];
        if (npcAction != NULL) {
            if (npcAction->action == 2) {
                BgTreemouth_SetupAction(this, func_808BC80C);
            } else if (npcAction->action == 3) {
                Audio_PlaySfx(NA_SE_EV_WOODDOOR_OPEN);
                BgTreemouth_SetupAction(this, func_808BC6F8);
            }
        }
    }
}

static void func_808BC6F8(BgTreemouth* this, PlayState* play) {
    Vec3f sp34;

    if (this->unk_168 < 1.0f) {
        this->unk_168 += 0.01f;
    } else {
        this->unk_168 = 1.0f;
    }

    if ((gSaveContext.sceneLayer == 6) && (gCsCtxOot.frames >= 0x2BD) && (play->state.frames % 8 == 0)) {
        sp34.x = (Rand_ZeroOne() * 1158.0f) + 3407.0f;
        sp34.y = 970.0f;
        sp34.z = (Rand_ZeroOne() * 2026.0f) + -2163.0f;
        EffectSsHahen_SpawnBurst(play, &sp34, 0.8f, 0, 50, 30, 1, HAHEN_OBJECT_DEFAULT, 10, NULL);
    }
}

static void func_808BC80C(BgTreemouth* this, PlayState* play) {
    this->unk_168 += 0.05f;
    if (this->unk_168 >= 0.8f) {
        BgTreemouth_SetupAction(this, func_808BC864);
    }
}

static void func_808BC864(BgTreemouth* this, PlayState* play) {
    this->unk_168 -= 0.03f;
    if (this->unk_168 <= 0.0f) {
        BgTreemouth_SetupAction(this, func_808BC65C);
    }
}

static void func_808BC8B8(BgTreemouth* this, PlayState* play) {
    if ((!GET_EVENTCHKINF_OOT(EVENTCHKINF_DEKU_TREE_OPENED_MOUTH_Oot)) || LINK_IS_ADULT_OOT) {
        if (!LINK_IS_ADULT_OOT) {
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_MET_DEKU_TREE_Oot)) {
                if (Actor_IsFacingAndNearPlayer(&this->dyna.actor, 1658.0f, 0x7530)) {
                    this->dyna.actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
                    if (this->dyna.actor.isLockedOn) {
                        this->dyna.actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
                        gCsCtxOot.segment = D_808BD2A0;
                        Hyrule_TriggerCutsceneOot(1);
                        BgTreemouth_SetupAction(this, func_808BC9EC);
                    }
                }
            } else if (Actor_IsFacingAndNearPlayer(&this->dyna.actor, 1658.0f, 0x4E20)) {
                SET_EVENTCHKINF_OOT(EVENTCHKINF_MET_DEKU_TREE_Oot);
                if ((true)) {
                    gCsCtxOot.segment = D_808BCE20;
                    Hyrule_TriggerCutsceneOot(1);
                    BgTreemouth_SetupAction(this, func_808BC9EC);
                }
            }
        }
    } else {
        this->unk_168 = 1.0f;
    }
}

static void func_808BC9EC(BgTreemouth* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (gCsCtxOot.state == CS_STATE_UNSKIPPABLE_INIT_Oot) {
        if (Actor_IsFacingAndNearPlayer(&this->dyna.actor, 350.0f, 0x7530)) {
            player->actor.world.pos.x = 3827.0f;
            player->actor.world.pos.y = -161.0f;
            player->actor.world.pos.z = -1142.0f;
        }

        gCsCtxOot.frames = 0;
        gCsCtxOot.unk_18 = 0xFFFF;
        D_8015FCC0 = 0xFFFF;
        D_8015FCC2 = 0xFFFF;
        D_8015FCC4 = 0xFFFF;
        gCsCtxOot.unk_1A = 0;
        gCsCtxOot.unk_1B = 0;
        gCsCtxOot.state = CS_STATE_SKIPPABLE_EXEC_Oot;

        if (play->msgCtx.choiceIndex == 0) {
            gCsCtxOot.segment = D_808BD520;
            SET_EVENTCHKINF_OOT(EVENTCHKINF_DEKU_TREE_OPENED_MOUTH_Oot);
            BgTreemouth_SetupAction(this, func_808BCAF0);
        } else {
            gCsCtxOot.segment = D_808BD790;
            gCsCtxOot.frames = 0;
            BgTreemouth_SetupAction(this, func_808BC8B8);
        }
    }
}

static void func_808BCAF0(BgTreemouth* this, PlayState* play) {
    CsCmdActorCueOot* npcAction;

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        npcAction = gCsCtxOot.npcActions[0];
        if (npcAction != NULL) {
            if (npcAction->action == 2) {
                BgTreemouth_SetupAction(this, func_808BC80C);
            } else if (npcAction->action == 3) {
                Audio_PlaySfx(NA_SE_EV_WOODDOOR_OPEN);
                BgTreemouth_SetupAction(this, func_808BC6F8);
            }
        }
    }
}

static void BgTreemouth_DoNothing(BgTreemouth* this, PlayState* play) {
}

static void BgTreemouth_Update(Actor* thisx, PlayState* play) {
    BgTreemouth* this = (BgTreemouth*)thisx;
    f32 unk_168;

    this->actionFunc(this, play);
    unk_168 = this->unk_168;
    thisx->world.pos.x = (unk_168 * -160.0f) + 4029.0f;
    thisx->world.pos.y = (unk_168 * -399.0f) + 136.0f;
    thisx->world.pos.z = (unk_168 * 92.0f) + -1255.0f;
}

static void BgTreemouth_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    u16 alpha = 500;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    if ((gSaveContext.sceneLayer < 4) || LINK_IS_ADULT_OOT) {
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_KOKIRI_EMERALD_DEKU_TREE_DEAD_Oot)) {
            alpha = 2150;
        }
    } else { // neeeded to match
    }

    if (gSaveContext.sceneLayer == 6) {
        alpha = (play->roomCtx.unk7A[0] + 0x1F4);
    }

    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, alpha * 0.1f);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gDekuTreeMouthDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
