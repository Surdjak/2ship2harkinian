/*
 * File: z_bg_ydan_hasi.c
 * Overlay: ovl_Bg_Ydan_Hasi
 * Description: Deku Tree Puzzle elements. Water plane and floating block in B1, and 3 blocks on 2F
 */

#include "z_bg_ydan_hasi.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_ydan_objects/object_ydan_objects.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgYdanHasi_Init(Actor* thisx, PlayState* play);
static void BgYdanHasi_Destroy(Actor* thisx, PlayState* play);
static void BgYdanHasi_Update(Actor* thisx, PlayState* play);
static void BgYdanHasi_Draw(Actor* thisx, PlayState* play);

static void BgYdanHasi_InitWater(BgYdanHasi* this, PlayState* play);
static void BgYdanHasi_UpdateFloatingBlock(BgYdanHasi* this, PlayState* play);
static void BgYdanHasi_SetupThreeBlocks(BgYdanHasi* this, PlayState* play);
static void BgYdanHasi_MoveWater(BgYdanHasi* this, PlayState* play);
static void BgYdanHasi_DecWaterTimer(BgYdanHasi* this, PlayState* play);
static void BgYdanHasi_UpdateThreeBlocks(BgYdanHasi* this, PlayState* play);

ActorProfile Bg_Ydan_Hasi_Oot_Profile = {
    /**/ ACTOR_BG_YDAN_HASI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_YDAN_OBJECTS_Oot,
    /**/ sizeof(BgYdanHasi),
    /**/ BgYdanHasi_Init,
    /**/ BgYdanHasi_Destroy,
    /**/ BgYdanHasi_Update,
    /**/ BgYdanHasi_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgYdanHasi_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgYdanHasi* this = (BgYdanHasi*)thisx;
    CollisionHeader* colHeader = NULL;
    WaterBox* waterBox;

    Actor_ProcessInitChain(thisx, sInitChain);
    this->type = ((thisx->params >> 8) & 0x3F);
    thisx->params = thisx->params & 0xFF;
    waterBox = &play->colCtx.colHeader->waterBoxes[1];
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    if (thisx->params == HASI_WATER) {
        // Water the moving platform floats on in B1. Never runs in Master Quest
        waterBox->minPos.y = thisx->world.pos.y = thisx->home.pos.y += -5.0f;
        this->actionFunc = BgYdanHasi_InitWater;
    } else {
        if (thisx->params == HASI_WATER_BLOCK) {
            // Moving platform on the water in B1
            CollisionHeader_GetVirtual(&gDTSlidingPlatformCol, &colHeader);
            thisx->scale.z = 0.15f;
            thisx->scale.x = 0.15f;
            thisx->world.pos.y = (waterBox->minPos.y + 20.0f);
            this->actionFunc = BgYdanHasi_UpdateFloatingBlock;
        } else {
            // 3 platforms on 2F
            CollisionHeader_GetVirtual(&gDTRisingPlatformsCol, &colHeader);
            thisx->draw = NULL;
            this->actionFunc = BgYdanHasi_SetupThreeBlocks;
            Actor_SetFocus(&this->dyna.actor, 40.0f);
        }
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    }
    this->timer = 0;
}

static void BgYdanHasi_Destroy(Actor* thisx, PlayState* play) {
    BgYdanHasi* this = (BgYdanHasi*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgYdanHasi_UpdateFloatingBlock(BgYdanHasi* this, PlayState* play) {
    WaterBox* waterBox;
    f32 framesAfterMath;

    framesAfterMath = sinf((play->gameplayFrames & 0xFF) * (M_PI / 128)) * 165.0f;
    this->dyna.actor.world.pos.x =
        ((Math_SinS(this->dyna.actor.world.rot.y) * framesAfterMath) + this->dyna.actor.home.pos.x);
    this->dyna.actor.world.pos.z =
        ((Math_CosS(this->dyna.actor.world.rot.y) * framesAfterMath) + this->dyna.actor.home.pos.z);
    waterBox = &play->colCtx.colHeader->waterBoxes[1];
    this->dyna.actor.world.pos.y = waterBox->minPos.y + 20.0f;
    if (this->timer != 0) {
        this->timer--;
    }
    if (this->timer == 0) {
        this->timer = 50;
    }
    this->dyna.actor.world.pos.y += 2.0f * sinf(this->timer * (M_PI / 25));
}

static void BgYdanHasi_InitWater(BgYdanHasi* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->type)) {
        this->timer = 600;
        this->actionFunc = BgYdanHasi_MoveWater;
    }
}

static void BgYdanHasi_MoveWater(BgYdanHasi* this, PlayState* play) {
    WaterBox* waterBox;

    if (this->timer == 0) {
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, 1.0f) != 0) {
            Flags_UnsetSwitch(play, this->type);
            this->actionFunc = BgYdanHasi_InitWater;
        }
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
    } else {
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y - 47.0f, 0.5f)) {
            this->actionFunc = BgYdanHasi_DecWaterTimer;
        }
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
    }
    waterBox = &play->colCtx.colHeader->waterBoxes[1];
    waterBox->minPos.y = this->dyna.actor.world.pos.y;
}

static void BgYdanHasi_DecWaterTimer(BgYdanHasi* this, PlayState* play) {
    if ((this->timer != 0)) {
        this->timer--;
    }

    Actor_PlaySfx_FlaggedTimer(&this->dyna.actor, this->timer);
    if (this->timer == 0) {
        this->actionFunc = BgYdanHasi_MoveWater;
    }
}

static void BgYdanHasi_SetupThreeBlocks(BgYdanHasi* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->type)) {
        this->timer = 260;
        this->dyna.actor.draw = BgYdanHasi_Draw;
        this->actionFunc = BgYdanHasi_UpdateThreeBlocks;
        OnePointCutscene_Init(play, 3040, 30, &this->dyna.actor, CAM_ID_MAIN);
    }
}

static void BgYdanHasi_UpdateThreeBlocks(BgYdanHasi* this, PlayState* play) {
    if ((this->timer != 0)) {
        this->timer--;
    }

    if (this->timer == 0) {
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, 3.0f) != 0) {
            Flags_UnsetSwitch(play, this->type);
            this->dyna.actor.draw = NULL;
            this->actionFunc = BgYdanHasi_SetupThreeBlocks;
        } else {
            Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_ELEVATOR_MOVE - SFX_FLAG);
        }
    } else if (!Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 120.0f, 3.0f)) {
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_ELEVATOR_MOVE - SFX_FLAG);

    } else {
        Actor_PlaySfx_FlaggedTimer(&this->dyna.actor, this->timer);
    }
}

static void BgYdanHasi_Update(Actor* thisx, PlayState* play) {
    BgYdanHasi* this = (BgYdanHasi*)thisx;

    this->actionFunc(this, play);
}

static void BgYdanHasi_Draw(Actor* thisx, PlayState* play) {
    static Gfx* dLists[] = { gDTSlidingPlatformDL, gDTWaterPlaneDL, gDTRisingPlatformsDL };
    BgYdanHasi* this = (BgYdanHasi*)thisx;

    if (this->dyna.actor.params == HASI_WATER_BLOCK || this->dyna.actor.params == HASI_THREE_BLOCKS) {
        Gfx_DrawDListOpa(play, dLists[this->dyna.actor.params]);
    } else {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, -play->gameplayFrames % 128, play->gameplayFrames % 128,
                                      0x20, 0x20, 1, play->gameplayFrames % 128, play->gameplayFrames % 128, 0x20, 0x20,
                                      -1, 1, 1, 1));
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gDTWaterPlaneDL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
