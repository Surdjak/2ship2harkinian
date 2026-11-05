/*
 * File: z_bg_mori_idomizu.c
 * Overlay: ovl_Bg_Mori_Idomizu
 * Description: Square of water in Forest Temple well
 */

#include "z_bg_mori_idomizu.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_mori_objects/object_mori_objects.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgMoriIdomizu_Init(Actor* thisx, PlayState* play);
static void BgMoriIdomizu_Destroy(Actor* thisx, PlayState* play);
static void BgMoriIdomizu_Update(Actor* thisx, PlayState* play);
static void BgMoriIdomizu_Draw(Actor* thisx, PlayState* play);

static void BgMoriIdomizu_SetupWaitForMoriTex(BgMoriIdomizu* this);
static void BgMoriIdomizu_WaitForMoriTex(BgMoriIdomizu* this, PlayState* play);
static void BgMoriIdomizu_SetupMain(BgMoriIdomizu* this);
static void BgMoriIdomizu_Main(BgMoriIdomizu* this, PlayState* play);

static s16 sKankyoIsSpawned = false;

ActorProfile Bg_Mori_Idomizu_Oot_Profile = {
    /**/ ACTOR_BG_MORI_IDOMIZU_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_MORI_OBJECTS_Oot,
    /**/ sizeof(BgMoriIdomizu),
    /**/ BgMoriIdomizu_Init,
    /**/ BgMoriIdomizu_Destroy,
    /**/ BgMoriIdomizu_Update,
    /**/ NULL,
};

static void BgMoriIdomizu_SetupAction(BgMoriIdomizu* this, BgMoriIdomizuActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgMoriIdomizu_SetWaterLevel(PlayState* play, s16 waterLevel) {
    WaterBox* waterBox = play->colCtx.colHeader->waterBoxes;

    waterBox[2].minPos.y = waterLevel;
    waterBox[3].minPos.y = waterLevel;
    waterBox[4].minPos.y = waterLevel;
}

static void BgMoriIdomizu_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriIdomizu* this = (BgMoriIdomizu*)thisx;

    if (sKankyoIsSpawned) {
        Actor_Kill(&this->actor);
        return;
    }
    this->actor.scale.x = 1.1f;
    this->actor.scale.y = 1.0f;
    this->actor.scale.z = 1.0f;
    this->actor.world.pos.x = 119.0f;
    this->actor.world.pos.z = -1820.0f;
    this->prevSwitchFlagSet = Flags_GetSwitch(play, this->actor.params & 0x3F);
    if (this->prevSwitchFlagSet != 0) {
        this->actor.world.pos.y = -282.0f;
        BgMoriIdomizu_SetWaterLevel(play, -282);
    } else {
        this->actor.world.pos.y = 184.0f;
        BgMoriIdomizu_SetWaterLevel(play, 184);
    }
    this->moriTexObjIndex = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (this->moriTexObjIndex < 0) {
        Actor_Kill(&this->actor);
        return;
    }
    BgMoriIdomizu_SetupWaitForMoriTex(this);
    sKankyoIsSpawned = true;
    this->isLoaded = true;
    this->actor.room = -1;
}

static void BgMoriIdomizu_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriIdomizu* this = (BgMoriIdomizu*)thisx;

    if (this->isLoaded) {
        sKankyoIsSpawned = false;
    }
}

static void BgMoriIdomizu_SetupWaitForMoriTex(BgMoriIdomizu* this) {
    BgMoriIdomizu_SetupAction(this, BgMoriIdomizu_WaitForMoriTex);
}

static void BgMoriIdomizu_WaitForMoriTex(BgMoriIdomizu* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->moriTexObjIndex)) {
        BgMoriIdomizu_SetupMain(this);
        this->actor.draw = BgMoriIdomizu_Draw;
    }
}

static void BgMoriIdomizu_SetupMain(BgMoriIdomizu* this) {
    BgMoriIdomizu_SetupAction(this, BgMoriIdomizu_Main);
}

static void BgMoriIdomizu_Main(BgMoriIdomizu* this, PlayState* play) {
    s8 roomNum;
    Actor* thisx = &this->actor;
    s32 switchFlagSet;

    roomNum = play->roomCtx.curRoom.num;
    switchFlagSet = Flags_GetSwitch(play, thisx->params & 0x3F);
    if (switchFlagSet) {
        this->targetWaterLevel = -282.0f;
    } else {
        this->targetWaterLevel = 184.0f;
    }
    if (switchFlagSet && !this->prevSwitchFlagSet) {
        OnePointCutscene_Init(play, 3240, 70, thisx, CAM_ID_MAIN);
        this->drainTimer = 90;
    } else if (!switchFlagSet && this->prevSwitchFlagSet) {
        OnePointCutscene_Init(play, 3240, 70, thisx, CAM_ID_MAIN);
        this->drainTimer = 90;
        thisx->world.pos.y = 0.0f;
    }
    this->drainTimer--;
    if ((roomNum == 7) || (roomNum == 8) || (roomNum == 9)) {
        if (this->drainTimer < 70) {
            Math_StepToF(&thisx->world.pos.y, this->targetWaterLevel, 3.5f);
            BgMoriIdomizu_SetWaterLevel(play, thisx->world.pos.y);
            if (this->drainTimer > 0) {
                if (switchFlagSet) {
                    Audio_PlaySfx_2(NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
                } else {
                    Audio_PlaySfx_2(NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
                }
            }
        }
    } else {
        thisx->world.pos.y = this->targetWaterLevel;
        BgMoriIdomizu_SetWaterLevel(play, thisx->world.pos.y);
        Actor_Kill(thisx);
        return;
    }
    this->prevSwitchFlagSet = switchFlagSet;
}

static void BgMoriIdomizu_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriIdomizu* this = (BgMoriIdomizu*)thisx;

    if (this->actionFunc != NULL) {
        this->actionFunc(this, play);
    }
}

static void BgMoriIdomizu_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriIdomizu* this = (BgMoriIdomizu*)thisx;
    u32 gameplayFrames = play->gameplayFrames;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIndex].segment);

    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, 128);

    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0x7F - (gameplayFrames & 0x7F), gameplayFrames % 0x80, 0x20,
                                  0x20, 1, gameplayFrames & 0x7F, gameplayFrames % 0x80, 0x20, 0x20, -1, 1, 1, 1));

    gSPDisplayList(POLY_XLU_DISP++, gMoriIdomizuWaterDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
