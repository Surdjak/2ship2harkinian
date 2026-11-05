/*
 * File: z_bg_jya_block.c
 * Overlay: ovl_Bg_Jya_Block
 * Description: Silver Block (Child Link)
 */

#include "z_bg_jya_block.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_dangeon_keep/gameplay_dangeon_keep.h"

#define FLAGS 0

static void BgJyaBlock_Init(Actor* thisx, PlayState* play);
static void BgJyaBlock_Destroy(Actor* thisx, PlayState* play);
static void BgJyaBlock_Update(Actor* thisx, PlayState* play);
static void BgJyaBlock_Draw(Actor* thisx, PlayState* play);

ActorProfile Bg_Jya_Block_Oot_Profile = {
    /**/ ACTOR_BG_JYA_BLOCK_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_GAMEPLAY_DANGEON_KEEP_Oot,
    /**/ sizeof(BgJyaBlock),
    /**/ BgJyaBlock_Init,
    /**/ BgJyaBlock_Destroy,
    /**/ BgJyaBlock_Update,
    /**/ BgJyaBlock_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 333, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1800, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1500, ICHAIN_STOP),
};

static void BgJyaBlock_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgJyaBlock* this = (BgJyaBlock*)thisx;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gPushBlockCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);

    if ((LINK_AGE_IN_YEARS_OOT != YEARS_CHILD) || !Flags_GetSwitch(play, thisx->params & 0x3F)) {
        Actor_Kill(&this->dyna.actor);
    }
}

static void BgJyaBlock_Destroy(Actor* thisx, PlayState* play) {
    BgJyaBlock* this = (BgJyaBlock*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgJyaBlock_Update(Actor* thisx, PlayState* play) {
    BgJyaBlock* this = (BgJyaBlock*)thisx;
    Player* player = GET_PLAYER(play);

    player->stateFlags2 &= ~PLAYER_STATE2_10;
    this->dyna.pushForce = 0.0f;
}

static void BgJyaBlock_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gPushBlockGrayTex));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 232, 210, 176, 255);
    gSPDisplayList(POLY_OPA_DISP++, gPushBlockDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
