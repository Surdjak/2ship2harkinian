/*
 * File: z_bg_gjyo_bridge.c
 * Overlay: ovl_Bg_Gjyo_Bridge
 * Description: Rainbow Bridge outside Ganon's Castle
 */

#include "z_bg_gjyo_bridge.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_gjyo_objects/object_gjyo_objects.h"
#include "assets/oot/scenes/dungeons/ganon_tou/ganon_tou_scene.h"

#define FLAGS 0

static void BgGjyoBridge_Init(Actor* thisx, PlayState* play);
static void BgGjyoBridge_Destroy(Actor* thisx, PlayState* play);
static void BgGjyoBridge_Update(Actor* thisx, PlayState* play);
static void BgGjyoBridge_Draw(Actor* thisx, PlayState* play);

static void func_808787A4(BgGjyoBridge* this, PlayState* play);
static void BgGjyoBridge_TriggerCutscene(BgGjyoBridge* this, PlayState* play);
static void BgGjyoBridge_SpawnBridge(BgGjyoBridge* this, PlayState* play);

ActorProfile Bg_Gjyo_Bridge_Oot_Profile = {
    /**/ ACTOR_BG_GJYO_BRIDGE_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_GJYO_OBJECTS_Oot,
    /**/ sizeof(BgGjyoBridge),
    /**/ BgGjyoBridge_Init,
    /**/ BgGjyoBridge_Destroy,
    /**/ BgGjyoBridge_Update,
    /**/ BgGjyoBridge_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeScale, 800, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgGjyoBridge_Init(Actor* thisx, PlayState* play) {
    BgGjyoBridge* this = (BgGjyoBridge*)thisx;
    s32 pad;
    CollisionHeader* colHeader;

    colHeader = NULL;

    Actor_ProcessInitChain(thisx, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gRainbowBridgeCol, &colHeader);

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_RAINBOW_BRIDGE_BUILT_Oot)) {
        this->actionFunc = func_808787A4;
    } else {
        this->dyna.actor.draw = NULL;
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        this->actionFunc = BgGjyoBridge_TriggerCutscene;
    }
}

static void BgGjyoBridge_Destroy(Actor* thisx, PlayState* play) {
    BgGjyoBridge* this = (BgGjyoBridge*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_808787A4(BgGjyoBridge* this, PlayState* play) {
}

void LaunchBridgeCutscene(BgGjyoBridge* this, PlayState* play) {
    if ((true)) {
        gCsCtxOot.segment = Lib_SegmentedToVirtual(gRainbowBridgeCs);
        Hyrule_TriggerCutsceneOot(1);
    }
    this->actionFunc = BgGjyoBridge_SpawnBridge;
}

u8 CheckPlayerPosition(Player* player, PlayState* play) {
    return (player->actor.world.pos.x > -70.0f) && (player->actor.world.pos.x < 300.0f) &&
           (player->actor.world.pos.y > 1340.0f) && (player->actor.world.pos.z > 1340.0f) &&
           (player->actor.world.pos.z < 1662.0f) && !Play_InCsMode(play);
}

static void BgGjyoBridge_TriggerCutscene(BgGjyoBridge* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    u8 vanillaBridgeCondition = CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_SPIRIT_Oot) && CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_SHADOW_Oot) &&
                                (INV_CONTENT_OOT(ITEM_ARROW_LIGHT_Oot) == ITEM_ARROW_LIGHT_Oot) &&
                                CheckPlayerPosition(player, play);

    if ((vanillaBridgeCondition)) {
        LaunchBridgeCutscene(this, play);
    }
}

static void BgGjyoBridge_SpawnBridge(BgGjyoBridge* this, PlayState* play) {
    u8 vanillaBridgeCondition = (gCsCtxOot.state != CS_STATE_IDLE_Oot) && (gCsCtxOot.npcActions[2] != NULL) &&
                                (gCsCtxOot.npcActions[2]->action == 2);

    if (!(true) || vanillaBridgeCondition) {
        this->dyna.actor.draw = BgGjyoBridge_Draw;
        DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        SET_EVENTCHKINF_OOT(EVENTCHKINF_RAINBOW_BRIDGE_BUILT_Oot);
    }
}

static void BgGjyoBridge_Update(Actor* thisx, PlayState* play) {
    BgGjyoBridge* this = (BgGjyoBridge*)thisx;

    this->actionFunc(this, play);
}

static void BgGjyoBridge_Draw(Actor* thisx, PlayState* play) {
    BgGjyoBridge* this = (BgGjyoBridge*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TexScrollEx(play->state.gfxCtx, play->gameplayFrames & 127, play->gameplayFrames * -3 & 127, 32, 32,
                               1, -3));

    gSPSegment(POLY_XLU_DISP++, 9,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -play->gameplayFrames & 127, 32, 32, 1, 0,
                                  play->gameplayFrames & 127, 32, 32, 0, -1, 0, 1));

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_XLU_DISP++, gRainbowBridgeDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
