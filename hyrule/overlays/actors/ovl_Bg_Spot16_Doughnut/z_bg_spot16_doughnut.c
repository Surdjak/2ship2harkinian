/*
 * File: z_bg_spot16_doughnut.c
 * Overlay: ovl_Bg_Spot16_Doughnut
 * Description: Death Mountain cloud circle
 */

#include "z_bg_spot16_doughnut.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_efc_doughnut/object_efc_doughnut.h"

#define FLAGS 0

static void BgSpot16Doughnut_Init(Actor* thisx, PlayState* play);
static void BgSpot16Doughnut_Destroy(Actor* thisx, PlayState* play);
static void BgSpot16Doughnut_Update(Actor* thisx, PlayState* play);
static void BgSpot16Doughnut_Draw(Actor* thisx, PlayState* play);

static void BgSpot16Doughnut_UpdateExpanding(Actor* thisx, PlayState* play);
static void BgSpot16Doughnut_DrawExpanding(Actor* thisx, PlayState* play);

ActorProfile Bg_Spot16_Doughnut_Oot_Profile = {
    /**/ ACTOR_BG_SPOT16_DOUGHNUT_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_EFC_DOUGHNUT_Oot,
    /**/ sizeof(BgSpot16Doughnut),
    /**/ BgSpot16Doughnut_Init,
    /**/ BgSpot16Doughnut_Destroy,
    /**/ BgSpot16Doughnut_Update,
    /**/ BgSpot16Doughnut_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeDistance, 5500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 5000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 5000, ICHAIN_STOP),
};

static s16 sScales[] = {
    0, 0, 70, 210, 300,
};

static void BgSpot16Doughnut_Init(Actor* thisx, PlayState* play) {
    BgSpot16Doughnut* this = (BgSpot16Doughnut*)thisx;
    s32 params;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Actor_SetScale(&this->actor, 0.1f);
    this->fireFlag = 0;
    this->envColorAlpha = 255;
    params = this->actor.params;
    if (params == 1 || params == 2 || params == 3 || params == 4) {
        Actor_SetScale(&this->actor, sScales[this->actor.params] * 1.0e-4f);
        this->actor.draw = BgSpot16Doughnut_DrawExpanding;
        this->actor.update = BgSpot16Doughnut_UpdateExpanding;
    } else {
        // Scales this actor for scenes where it is featured in the background,
        // Death Mountain itself falls into the default case.
        switch (play->sceneId) {
            case SCENE_KAKARIKO_VILLAGE_Oot:
                Actor_SetScale(&this->actor, 0.04f);
                break;
            case SCENE_TEMPLE_OF_TIME_EXTERIOR_DAY_Oot:
            case SCENE_TEMPLE_OF_TIME_EXTERIOR_NIGHT_Oot:
            case SCENE_TEMPLE_OF_TIME_EXTERIOR_RUINS_Oot:
                Actor_SetScale(&this->actor, 0.018f);
                break;
            default:
                Actor_SetScale(&this->actor, 0.1f);
                break;
        }
        if (!LINK_IS_ADULT_OOT || GET_EVENTCHKINF_OOT(EVENTCHKINF_DEATH_MOUNTAIN_ERUPTED_Oot)) {
            this->fireFlag &= ~1;
        } else {
            this->fireFlag |= 1;
        }
    }
}

static void BgSpot16Doughnut_Destroy(Actor* thisx, PlayState* play) {
}

static void BgSpot16Doughnut_Update(Actor* thisx, PlayState* play) {
    BgSpot16Doughnut* this = (BgSpot16Doughnut*)thisx;

    if (!(this->fireFlag & 1)) {
        this->actor.shape.rot.y -= 0x20;
        if (this->envColorAlpha < 255) {
            this->envColorAlpha += 5;
        } else {
            this->envColorAlpha = 255;
        }
    } else if (play->csCtx.state != CS_STATE_IDLE && play->csCtx.actorCues[2] != NULL &&
               play->csCtx.actorCues[2]->id == 2) {
        if (this->envColorAlpha >= 6) {
            this->envColorAlpha -= 5;
        } else {
            this->envColorAlpha = 0;
            this->fireFlag &= ~1;
        }
    }
}

// Update function for outwardly expanding and dissipating
static void BgSpot16Doughnut_UpdateExpanding(Actor* thisx, PlayState* play) {
    BgSpot16Doughnut* this = (BgSpot16Doughnut*)thisx;

    if (this->envColorAlpha >= 6) {
        this->envColorAlpha -= 5;
    } else {
        Actor_Kill(&this->actor);
    }
    this->actor.shape.rot.y -= 0x20;
    Actor_SetScale(&this->actor, this->actor.scale.x + 0.0019999998f);
}

static void BgSpot16Doughnut_Draw(Actor* thisx, PlayState* play) {
    BgSpot16Doughnut* this = (BgSpot16Doughnut*)thisx;
    u32 scroll = play->gameplayFrames & 0xFFFF;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    if (this->fireFlag & 1) {
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, scroll * (-1), 0, 16, 32, 1, scroll, scroll * (-2), 16, 32,
                                      -1, 0, 1, -2));
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, this->envColorAlpha);
        gSPDisplayList(POLY_XLU_DISP++, gDeathMountainCloudCircleFieryDL);
    } else {
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, this->envColorAlpha);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
        gSPDisplayList(POLY_XLU_DISP++, gDeathMountainCloudCircleNormalDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// Draw function for outwardly expanding and dissipating
static void BgSpot16Doughnut_DrawExpanding(Actor* thisx, PlayState* play) {
    BgSpot16Doughnut* this = (BgSpot16Doughnut*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, this->envColorAlpha);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    gSPDisplayList(POLY_XLU_DISP++, gDeathMountainCloudCircleNormalDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
