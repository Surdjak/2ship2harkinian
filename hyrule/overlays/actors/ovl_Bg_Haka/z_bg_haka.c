/*
 * File: z_bg_haka.c
 * Overlay: ovl_Bg_Haka
 * Description: Gravestone
 */

#include "z_bg_haka.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_haka/object_haka.h"

#define FLAGS 0

static void BgHaka_Init(Actor* thisx, PlayState* play);
static void BgHaka_Destroy(Actor* thisx, PlayState* play);
static void BgHaka_Update(Actor* thisx, PlayState* play);
static void BgHaka_Draw(Actor* thisx, PlayState* play);

static void BgHaka_CheckPlayerOnDirtPatch(BgHaka* this, Player* player);
static void BgHaka_IdleClosed(BgHaka* this, PlayState* play);
static void BgHaka_Pull(BgHaka* this, PlayState* play);
static void BgHaka_IdleOpened(BgHaka* this, PlayState* play);
static void BgHaka_IdleLockedClosed(BgHaka* this, PlayState* play);

ActorProfile Bg_Haka_Oot_Profile = {
    /**/ ACTOR_BG_HAKA_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_HAKA_Oot,
    /**/ sizeof(BgHaka),
    /**/ BgHaka_Init,
    /**/ BgHaka_Destroy,
    /**/ BgHaka_Update,
    /**/ BgHaka_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(terminalVelocity, 0, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgHaka_Init(Actor* thisx, PlayState* play) {
    BgHaka* this = (BgHaka*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gGravestoneCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    this->actionFunc = BgHaka_IdleClosed;
}

static void BgHaka_Destroy(Actor* thisx, PlayState* play) {
    BgHaka* this = (BgHaka*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgHaka_CheckPlayerOnDirtPatch(BgHaka* this, Player* player) {
    Vec3f sp1C;

    Actor_WorldToActorCoords(&this->dyna.actor, &sp1C, &player->actor.world.pos);
    if (fabsf(sp1C.x) < 34.6f && sp1C.z > -112.8f && sp1C.z < -36.0f) {
        player->stateFlags2 |= PLAYER_STATE2_FORCE_SAND_FLOOR_SOUND;
    }
}

static void BgHaka_IdleClosed(BgHaka* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->dyna.pushForce != 0.0f) {
        if (play->sceneId == SCENE_GRAVEYARD_Oot && !LINK_IS_ADULT_OOT && IS_DAY_OOT) {
            this->dyna.pushForce = 0.0f;
            player->stateFlags2 &= ~PLAYER_STATE2_10;
            if (!Play_InCsMode(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x5073), NULL);
                this->dyna.actor.params = 100;
                this->actionFunc = BgHaka_IdleLockedClosed;
            }
        } else if (0.0f < this->dyna.pushForce ||
                   (play->sceneId == SCENE_LAKE_HYLIA_Oot && !LINK_IS_ADULT_OOT && !Flags_GetSwitch(play, 0x23))) {
            this->dyna.pushForce = 0.0f;
            player->stateFlags2 &= ~PLAYER_STATE2_10;
        } else {
            this->dyna.actor.world.rot.y = this->dyna.actor.shape.rot.y + 0x8000;
            this->actionFunc = BgHaka_Pull;
        }
    }
    BgHaka_CheckPlayerOnDirtPatch(this, player);
}

static void BgHaka_Pull(BgHaka* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 sp38;

    this->dyna.actor.speed += 0.05f;
    this->dyna.actor.speed = CLAMP_MAX(this->dyna.actor.speed, 1.5f);
    sp38 = Math_StepToF(&this->dyna.actor.terminalVelocity, 60.0f, this->dyna.actor.speed);
    this->dyna.actor.world.pos.x =
        Math_SinS(this->dyna.actor.world.rot.y) * this->dyna.actor.terminalVelocity + this->dyna.actor.home.pos.x;
    this->dyna.actor.world.pos.z =
        Math_CosS(this->dyna.actor.world.rot.y) * this->dyna.actor.terminalVelocity + this->dyna.actor.home.pos.z;
    if (sp38 != 0) {
        this->dyna.pushForce = 0.0f;
        player->stateFlags2 &= ~PLAYER_STATE2_10;

        if (this->dyna.actor.params == 1) {
            Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        } else if ((!IS_DAY_OOT && play->sceneId == SCENE_GRAVEYARD_Oot)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_POH_Oot, this->dyna.actor.home.pos.x, this->dyna.actor.home.pos.y,
                        this->dyna.actor.home.pos.z, 0, this->dyna.actor.shape.rot.y, 0, 1);
        }

        this->actionFunc = BgHaka_IdleOpened;
    }
    Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_ROCK_SLIDE - SFX_FLAG);
}

static void BgHaka_IdleOpened(BgHaka* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->dyna.pushForce != 0.0f) {
        this->dyna.pushForce = 0.0f;
        player->stateFlags2 &= ~PLAYER_STATE2_10;
    }
}

static void BgHaka_IdleLockedClosed(BgHaka* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 pad;

    if (this->dyna.actor.params != 0) {
        this->dyna.actor.params -= 1;
    }
    if (this->dyna.pushForce != 0.0f) {
        this->dyna.pushForce = 0.0f;
        player->stateFlags2 &= ~PLAYER_STATE2_10;
    }
    if (this->dyna.actor.params == 0) {
        this->actionFunc = BgHaka_IdleClosed;
    }
    BgHaka_CheckPlayerOnDirtPatch(this, player);
}

static void BgHaka_Update(Actor* thisx, PlayState* play) {
    BgHaka* this = (BgHaka*)thisx;

    this->actionFunc(this, play);
}

static void BgHaka_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gGravestoneStoneDL);
    Matrix_Translate(0.0f, 0.0f, thisx->terminalVelocity * 10.0f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gGravestoneEarthDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
