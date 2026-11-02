/*
 * File: z_en_rl.c
 * Overlay: En_Rl
 * Description: Rauru
 */

#include "z_en_rl.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_rl/object_rl.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnRl_Init(Actor* thisx, PlayState* play);
static void EnRl_Destroy(Actor* thisx, PlayState* play);
static void EnRl_Update(Actor* thisx, PlayState* play);
static void EnRl_Draw(Actor* thisx, PlayState* play);

static void func_80AE7798(EnRl* this, PlayState* play);
static void func_80AE77B8(EnRl* this, PlayState* play);
static void func_80AE77F8(EnRl* this, PlayState* play);
static void func_80AE7838(EnRl* this, PlayState* play);
static void func_80AE7C64(EnRl* this, PlayState* play);
static void func_80AE7C94(EnRl* this, PlayState* play);
static void func_80AE7CE8(EnRl* this, PlayState* play);
static void func_80AE7D40(EnRl* this, PlayState* play);
static void func_80AE7FD0(EnRl* this, PlayState* play);
static void func_80AE7FDC(EnRl* this, PlayState* play);
static void func_80AE7D94(EnRl* this, PlayState* play);

static void* D_80AE81A0[] = { object_rl_Tex_003620, object_rl_Tex_003960, object_rl_Tex_003B60 };
static s32 D_80AE81AC = 0;

static void EnRl_Destroy(Actor* thisx, PlayState* play) {
    EnRl* this = (EnRl*)thisx;
    D_80AE81AC = 0;
    SkelAnime_Free(&this->skelAnime, play);
}

static void func_80AE72D0(EnRl* this) {
    s32 pad[3];
    s16* timer = &this->timer;
    s16* eyeTextureIndex = &this->eyeTextureIndex;

    if (DECR(*timer) == 0) {
        *timer = Rand_S16Offset(60, 60);
    }

    *eyeTextureIndex = *timer;
    if (*eyeTextureIndex > 2) {
        *eyeTextureIndex = 0;
    }
}

static void func_80AE7358(EnRl* this) {
    Animation_Change(&this->skelAnime, &object_rl_Anim_000A3C, 1.0f, 0.0f,
                     Animation_GetLastFrame(&object_rl_Anim_000A3C), ANIMMODE_LOOP, 0.0f);
    this->action = 4;
    this->drawConfig = 0;
    this->alpha = 0;
    this->lightBallSpawned = 0;
    this->actor.shape.shadowAlpha = 0;
    this->unk_19C = 0.0f;
}

static void func_80AE73D8(EnRl* this, PlayState* play) {

    if (gCsCtxOot.state == CS_STATE_IDLE_Oot) {
        if (D_80AE81AC) {
            if (this->actor.params == 2) {
                func_80AE7358(this);
            }
            D_80AE81AC = 0;
        }
    } else if (!D_80AE81AC) {
        D_80AE81AC = 1;
    }
}

static void func_80AE744C(EnRl* this, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f, 0x405);
}

static s32 EnRl_UpdateSkelAnime(EnRl* this) {
    return SkelAnime_Update(&this->skelAnime);
}

static s32 func_80AE74B4(EnRl* this, PlayState* play, u16 arg2, s32 arg3) {
    CsCmdActorCueOot* csCmdActorAction;

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        csCmdActorAction = gCsCtxOot.npcActions[arg3];
        if (csCmdActorAction != NULL && csCmdActorAction->action == arg2) {
            return 1;
        }
    }
    return 0;
}

static s32 func_80AE74FC(EnRl* this, PlayState* play, u16 arg2, s32 arg3) {
    CsCmdActorCueOot* csCmdActorAction;

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        csCmdActorAction = gCsCtxOot.npcActions[arg3];
        if (csCmdActorAction != NULL && csCmdActorAction->action != arg2) {
            return 1;
        }
    }
    return 0;
}

static void func_80AE7544(EnRl* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &object_rl_Skel_007B38, &object_rl_Anim_000A3C, NULL, NULL, 0);
}

static void func_80AE7590(EnRl* this, PlayState* play) {
    s32 pad;
    Player* player;
    Vec3f pos;
    s16 sceneNum = play->sceneId;

    if (gSaveContext.sceneLayer == 4 && sceneNum == SCENE_CHAMBER_OF_THE_SAGES_Oot && gCsCtxOot.state != CS_STATE_IDLE_Oot &&
        gCsCtxOot.npcActions[6] != NULL && gCsCtxOot.npcActions[6]->action == 2 && !this->lightMedallionGiven) {
        player = GET_PLAYER(play);
        pos.x = player->actor.world.pos.x;
        pos.y = player->actor.world.pos.y + 80.0f;
        pos.z = player->actor.world.pos.z;
        Actor_Spawn(&play->actorCtx, play, ACTOR_DEMO_EFFECT_Oot, pos.x, pos.y, pos.z, 0, 0, 0, 0xE);
        if ((true)) {
            Item_Give(play, ITEM_MEDALLION_LIGHT_Oot);
        }
        this->lightMedallionGiven = 1;
    }
}

static void func_80AE7668(EnRl* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->drawConfig = 1;
    this->action = 1;
    player->actor.world.rot.y = player->actor.shape.rot.y = this->actor.world.rot.y + 0x8000;
}

static void func_80AE7698(EnRl* this, PlayState* play) {
    CsCmdActorCueOot* csCmdActorAction;

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        csCmdActorAction = gCsCtxOot.npcActions[0];
        if (csCmdActorAction != NULL && csCmdActorAction->action == 3) {
            Animation_Change(&this->skelAnime, &object_rl_Anim_00040C, 1.0f, 0.0f,
                             Animation_GetLastFrame(&object_rl_Anim_00040C), ANIMMODE_ONCE, 0.0f);
            this->action = 2;
        }
    }
}

static void func_80AE772C(EnRl* this, s32 arg1) {
    if (arg1) {
        Animation_Change(&this->skelAnime, &object_rl_Anim_000830, 1.0f, 0.0f,
                         Animation_GetLastFrame(&object_rl_Anim_000830), ANIMMODE_LOOP, 0.0f);
        this->action = 3;
    }
}

static void func_80AE7798(EnRl* this, PlayState* play) {
    func_80AE7668(this, play);
}

static void func_80AE77B8(EnRl* this, PlayState* play) {
    func_80AE744C(this, play);
    EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE7698(this, play);
}

static void func_80AE77F8(EnRl* this, PlayState* play) {
    s32 temp;

    func_80AE744C(this, play);
    temp = EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE772C(this, temp);
}

static void func_80AE7838(EnRl* this, PlayState* play) {
    func_80AE744C(this, play);
    EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE7590(this, play);
}

static void func_80AE7878(EnRl* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &object_rl_Skel_007B38, &object_rl_Anim_000A3C, NULL, NULL, 0);
    this->action = 4;
    this->actor.shape.shadowAlpha = 0;
}

static void func_80AE78D4(EnRl* this, PlayState* play) {
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DEMO_6K_Oot, this->actor.world.pos.x,
                       kREG(18) + 22.0f + this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 5);
}

static void func_80AE7954(EnRl* this, PlayState* play) {
    if (func_80AE74B4(this, play, 4, 0)) {
        this->action = 5;
        this->drawConfig = 2;
        this->alpha = 0;
        this->actor.shape.shadowAlpha = 0;
        this->unk_19C = 0.0f;
    }
}

static void func_80AE79A4(EnRl* this, PlayState* play) {
    f32* unk_19C = &this->unk_19C;
    s32 alpha = 255;

    if (func_80AE74B4(this, play, 4, 0)) {
        *unk_19C += 1.0f;
        if (*unk_19C >= kREG(5) + 10.0f) {
            this->action = 7;
            this->drawConfig = 1;
            *unk_19C = kREG(5) + 10.0f;
            this->alpha = alpha;
            this->actor.shape.shadowAlpha = alpha;
            return;
        }
    } else {
        *unk_19C -= 1.0f;
        if (*unk_19C <= 0.0f) {
            this->action = 4;
            this->drawConfig = 0;
            *unk_19C = 0.0f;
            this->alpha = 0;
            this->actor.shape.shadowAlpha = 0;
            return;
        }
    }
    alpha = (*unk_19C / (kREG(5) + 10.0f)) * 255.0f;
    this->alpha = alpha;
    this->actor.shape.shadowAlpha = alpha;
}

static void func_80AE7AF8(EnRl* this, PlayState* play) {
    if (func_80AE74B4(this, play, 3, 0)) {
        Animation_Change(&this->skelAnime, &object_rl_Anim_00040C, 1.0f, 0.0f,
                         Animation_GetLastFrame(&object_rl_Anim_00040C), ANIMMODE_ONCE, -8.0f);
        this->action = 6;
    } else if (func_80AE74FC(this, play, 4, 0)) {
        this->action = 5;
        this->drawConfig = 2;
        this->unk_19C = kREG(5) + 10.0f;
        this->alpha = 255;
        if (!this->lightBallSpawned) {
            func_80AE78D4(this, play);
            this->lightBallSpawned = 1;
        }
        this->actor.shape.shadowAlpha = 0xFF;
    }
}

static void func_80AE7BF8(EnRl* this, s32 arg1) {
    if (arg1 != 0) {
        Animation_Change(&this->skelAnime, &object_rl_Anim_000830, 1.0f, 0.0f,
                         Animation_GetLastFrame(&object_rl_Anim_000830), ANIMMODE_LOOP, 0.0f);
        this->action = 7;
    }
}

static void func_80AE7C64(EnRl* this, PlayState* play) {
    func_80AE7954(this, play);
    func_80AE73D8(this, play);
}

static void func_80AE7C94(EnRl* this, PlayState* play) {
    func_80AE744C(this, play);
    EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE79A4(this, play);
    func_80AE73D8(this, play);
}

static void func_80AE7CE8(EnRl* this, PlayState* play) {
    s32 temp;

    func_80AE744C(this, play);
    temp = EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE7BF8(this, temp);
    func_80AE73D8(this, play);
}

static void func_80AE7D40(EnRl* this, PlayState* play) {
    func_80AE744C(this, play);
    EnRl_UpdateSkelAnime(this);
    func_80AE72D0(this);
    func_80AE7AF8(this, play);
    func_80AE73D8(this, play);
}

static void func_80AE7D94(EnRl* this, PlayState* play) {
    s32 pad[2];
    s16 temp = this->eyeTextureIndex;
    void* tex = D_80AE81A0[temp];
    SkelAnime* skelAnime = &this->skelAnime;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08, Lib_SegmentedToVirtual(tex));
    gSPSegment(POLY_XLU_DISP++, 0x09, Lib_SegmentedToVirtual(tex));
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, this->alpha);
    gSPSegment(POLY_XLU_DISP++, 0x0C, D_80116280_Oot);

    POLY_XLU_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount, NULL,
                                       NULL, NULL, POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

static EnRlActionFunc sActionFuncs[] = {
    func_80AE7798, func_80AE77B8, func_80AE77F8, func_80AE7838,
    func_80AE7C64, func_80AE7C94, func_80AE7CE8, func_80AE7D40,
};

static void EnRl_Update(Actor* thisx, PlayState* play) {
    EnRl* this = (EnRl*)thisx;

    if ((this->action < 0) || (this->action > 7) || (sActionFuncs[this->action] == NULL)) {
        return;
    }
    sActionFuncs[this->action](this, play);
}

static void EnRl_Init(Actor* thisx, PlayState* play) {
    EnRl* this = (EnRl*)thisx;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 50.0f);
    if (this->actor.params == 2) {
        func_80AE7878(this, play);
    } else {
        func_80AE7544(this, play);
    }
}
static void func_80AE7FD0(EnRl* this, PlayState* play) {
}

static void func_80AE7FDC(EnRl* this, PlayState* play) {
    s32 pad[2];
    s16 temp = this->eyeTextureIndex;
    void* tex = D_80AE81A0[temp];
    SkelAnime* skelAnime = &this->skelAnime;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(tex));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(tex));
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSPSegment(POLY_OPA_DISP++, 0x0C, &D_80116280_Oot[2]);

    SkelAnime_DrawSkeletonOpaOot(play, skelAnime, NULL, NULL, &this->actor);
    CLOSE_DISPS(play->state.gfxCtx);
}

static EnRlDrawFunc sDrawFuncs[] = {
    func_80AE7FD0,
    func_80AE7FDC,
    func_80AE7D94,
};

static void EnRl_Draw(Actor* thisx, PlayState* play) {
    EnRl* this = (EnRl*)thisx;

    if (this->drawConfig < 0 || this->drawConfig >= 3 || sDrawFuncs[this->drawConfig] == NULL) {
        return;
    }
    sDrawFuncs[this->drawConfig](this, play);
}

ActorProfile En_Rl_Oot_Profile = {
    /**/ ACTOR_EN_RL_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_RL_Oot,
    /**/ sizeof(EnRl),
    /**/ EnRl_Init,
    /**/ EnRl_Destroy,
    /**/ EnRl_Update,
    /**/ EnRl_Draw,
};
