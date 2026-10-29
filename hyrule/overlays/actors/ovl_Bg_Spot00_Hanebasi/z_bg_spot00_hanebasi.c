/*
 * File: z_bg_spot00_hanebasi.c
 * Overlay: ovl_Bg_Spot00_Hanebasi
 * Description: Hyrule Field Drawbridge and Torches
 */

#include "z_bg_spot00_hanebasi.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "assets/oot/objects/object_spot00_objects/object_spot00_objects.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

typedef enum {
    /* -1 */ DT_DRAWBRIDGE = -1,
    /*  0 */ DT_CHAIN_1,
    /*  1 */ DT_CHAIN_2
} DrawbridgeType;

static void BgSpot00Hanebasi_Init(Actor* thisx, PlayState* play);
static void BgSpot00Hanebasi_Destroy(Actor* thisx, PlayState* play);
static void BgSpot00Hanebasi_Update(Actor* thisx, PlayState* play);
static void BgSpot00Hanebasi_Draw(Actor* thisx, PlayState* play);

static void BgSpot00Hanebasi_DrawbridgeWait(BgSpot00Hanebasi* this, PlayState* play);
static void BgSpot00Hanebasi_DrawbridgeRiseAndFall(BgSpot00Hanebasi* this, PlayState* play);
static void BgSpot00Hanebasi_SetTorchLightInfo(BgSpot00Hanebasi* this, PlayState* play);

ActorProfile Bg_Spot00_Hanebasi_Oot_Profile = {
    /**/ ACTOR_BG_SPOT00_HANEBASI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SPOT00_OBJECTS_Oot,
    /**/ sizeof(BgSpot00Hanebasi),
    /**/ BgSpot00Hanebasi_Init,
    /**/ BgSpot00Hanebasi_Destroy,
    /**/ BgSpot00Hanebasi_Update,
    /**/ BgSpot00Hanebasi_Draw,
};

static f32 sTorchFlameScale = 0.0f;

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeScale, 550, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 2000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 5000, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static void BgSpot00Hanebasi_Init(Actor* thisx, PlayState* play) {
    BgSpot00Hanebasi* this = (BgSpot00Hanebasi*)thisx;
    s32 pad;
    Vec3f chainPos;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, 1);

    if (this->dyna.actor.params == DT_DRAWBRIDGE) {
        CollisionHeader_GetVirtual(&gHyruleFieldCastleDrawbridgeCol, &colHeader);
    } else {
        CollisionHeader_GetVirtual(&gHyruleFieldCastleDrawbridgeChainsCol, &colHeader);
    }

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if (this->dyna.actor.params == DT_DRAWBRIDGE) {
        if (LINK_IS_ADULT_OOT && (gSaveContext.sceneLayer < 4)) {
            Actor_Kill(&this->dyna.actor);
            return;
        }

        if ((gSaveContext.sceneLayer != 6) &&
            ((gSaveContext.sceneLayer == 4) || (gSaveContext.sceneLayer == 5) || (!LINK_IS_ADULT_OOT && !IS_DAY_OOT))) {
            this->dyna.actor.shape.rot.x = -0x4000;
        } else {
            this->dyna.actor.shape.rot.x = 0;
        }

        if (gSaveContext.sceneLayer != 6) {
            // Don't close the bridge in rando to accommodate hyrule castle exit
            if (CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot) && CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot) &&
                CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot)) {
                this->dyna.actor.shape.rot.x = -0x4000;
            }
        }

        chainPos.y =
            (10.0f * Math_CosS(this->dyna.actor.shape.rot.x)) - (Math_SinS(this->dyna.actor.shape.rot.x) * 400.0f);
        chainPos.z =
            (10.0f * Math_SinS(this->dyna.actor.shape.rot.x)) - (Math_CosS(this->dyna.actor.shape.rot.x) * 400.0f);
        chainPos.x =
            (158.0f * Math_CosS(this->dyna.actor.shape.rot.y)) + (Math_SinS(this->dyna.actor.shape.rot.y) * chainPos.z);
        chainPos.z = (-158.0f * Math_SinS(this->dyna.actor.shape.rot.y)) +
                     (Math_CosS(this->dyna.actor.shape.rot.y) * chainPos.z);

        if (Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_BG_SPOT00_HANEBASI_Oot,
                               this->dyna.actor.world.pos.x + chainPos.x, this->dyna.actor.world.pos.y + chainPos.y,
                               this->dyna.actor.world.pos.z + chainPos.z,
                               ((this->dyna.actor.shape.rot.x == 0) ? 0 : 0xF020), this->dyna.actor.shape.rot.y, 0,
                               DT_CHAIN_1) == NULL) {
            Actor_Kill(&this->dyna.actor);
        }

        this->actionFunc = BgSpot00Hanebasi_DrawbridgeWait;
        this->destAngle = 40;
    } else if (this->dyna.actor.params == DT_CHAIN_1) {
        if (Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_BG_SPOT00_HANEBASI_Oot,
                               this->dyna.actor.world.pos.x - (Math_CosS(this->dyna.actor.shape.rot.y) * 316.0f),
                               this->dyna.actor.world.pos.y,
                               this->dyna.actor.world.pos.z + (Math_SinS(this->dyna.actor.shape.rot.y) * 316.0f),
                               this->dyna.actor.shape.rot.x, this->dyna.actor.shape.rot.y, 0, DT_CHAIN_2) == NULL) {
            Actor_Kill(&this->dyna.actor);
            Actor_Kill(this->dyna.actor.parent);
        }

        this->actionFunc = BgSpot00Hanebasi_SetTorchLightInfo;
    } else {
        this->actionFunc = BgSpot00Hanebasi_SetTorchLightInfo;
    }

    if (this->dyna.actor.params >= DT_CHAIN_1) {
        this->lightNode = LightContext_InsertLight(play, &play->lightCtx, &this->lightInfo);
        Lights_PointGlowSetInfo(&this->lightInfo, ((this->dyna.actor.params == DT_CHAIN_1) ? 260.0f : -260.0f), 168,
                                690, 255, 255, 0, 0);
    }
}

static void BgSpot00Hanebasi_Destroy(Actor* thisx, PlayState* play) {
    BgSpot00Hanebasi* this = (BgSpot00Hanebasi*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);

    if (this->dyna.actor.params >= DT_CHAIN_1) {
        LightContext_RemoveLight(play, &play->lightCtx, this->lightNode);
    }
}

static void BgSpot00Hanebasi_DrawbridgeWait(BgSpot00Hanebasi* this, PlayState* play) {
    BgSpot00Hanebasi* child = (BgSpot00Hanebasi*)this->dyna.actor.child;

    if ((gSaveContext.sceneLayer >= 4) || !CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot) ||
        !CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot) || !CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot) ||
        (GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot))) {
        if (this->dyna.actor.shape.rot.x != 0) {
            if (Flags_GetEnvOot(play, 0) || ((gSaveContext.sceneLayer < 4) && IS_DAY_OOT)) {
                this->actionFunc = BgSpot00Hanebasi_DrawbridgeRiseAndFall;
                this->destAngle = 0;
                child->destAngle = 0;
                return;
            }

            if (this) {} // required to match
        }
        if ((this->dyna.actor.shape.rot.x == 0) && (gSaveContext.sceneLayer < 4) && !LINK_IS_ADULT_OOT && !IS_DAY_OOT) {
            this->actionFunc = BgSpot00Hanebasi_DrawbridgeRiseAndFall;
            this->destAngle = -0x4000;
            child->destAngle = -0xFE0;
        }
    }
}

static void BgSpot00Hanebasi_DoNothing(BgSpot00Hanebasi* this, PlayState* play) {
}

static void BgSpot00Hanebasi_DrawbridgeRiseAndFall(BgSpot00Hanebasi* this, PlayState* play) {
    BgSpot00Hanebasi* child;
    Actor* childsChild;
    s16 angle = 80;

    if (Math_ScaledStepToS(&this->dyna.actor.shape.rot.x, this->destAngle, 80)) {
        this->actionFunc = BgSpot00Hanebasi_DrawbridgeWait;
    }

    if (this->dyna.actor.shape.rot.x >= -0x27D8) {
        child = (BgSpot00Hanebasi*)this->dyna.actor.child;
        angle *= 0.4f;
        Math_ScaledStepToS(&child->dyna.actor.shape.rot.x, child->destAngle, angle);
        childsChild = child->dyna.actor.child;
        Math_ScaledStepToS(&childsChild->shape.rot.x, child->destAngle, angle);
    }

    if (this->destAngle < 0) {
        if (this->actionFunc == BgSpot00Hanebasi_DrawbridgeWait) {
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BRIDGE_CLOSE_STOP);
        } else {
            Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BRIDGE_CLOSE - SFX_FLAG);
        }
    } else {
        if (this->actionFunc == BgSpot00Hanebasi_DrawbridgeWait) {
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BRIDGE_OPEN_STOP);
        } else {
            Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BRIDGE_OPEN - SFX_FLAG);
        }
    }
}

static void BgSpot00Hanebasi_SetTorchLightInfo(BgSpot00Hanebasi* this, PlayState* play) {
    u8 lightColor = (u8)(Rand_ZeroOne() * 127.0f) + 128; // intensity of the red and green channels

    Lights_PointGlowSetInfo(&this->lightInfo, (this->dyna.actor.params == DT_CHAIN_1) ? 260.0f : -260.0f,
                            (5000.0f * sTorchFlameScale) + 128.0f, 690, lightColor, lightColor, 0,
                            sTorchFlameScale * 37500.0f);
}

static void BgSpot00Hanebasi_Update(Actor* thisx, PlayState* play) {
    BgSpot00Hanebasi* this = (BgSpot00Hanebasi*)thisx;
    s32 pad;

    this->actionFunc(this, play);

    if (this->dyna.actor.params == DT_DRAWBRIDGE) {
        if (play->sceneId == SCENE_HYRULE_FIELD_Oot) {
            if (CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot) && CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot) &&
                CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot) &&
                LINK_IS_CHILD_OOT) {
                Player* player = GET_PLAYER(play);

                if ((player->actor.world.pos.x > -450.0f) && (player->actor.world.pos.x < 450.0f) &&
                    (player->actor.world.pos.z > 1080.0f) && (player->actor.world.pos.z < 1700.0f) &&
                    (!(Play_InCsMode(play)))) {
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot);
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_DRAWBRIDGE_OPENED_AFTER_ZELDA_FLED_Oot);
                    this->actionFunc = BgSpot00Hanebasi_DoNothing;
                    Player_SetCsActionWithHaltedActorsOot(play, &player->actor, 8);
                    play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                    gSaveContext.nextCutsceneIndex = 0xFFF1;
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                } else if (Actor_IsFacingAndNearPlayer(&this->dyna.actor, 3000.0f, 0x7530)) {
                    play->envCtx.stormRequest = 1;
                }
            }
        }

        if (gSaveContext.sceneLayer == 5) {
            u16 dayTime;
            s32 tmp;

            if (R_TIME_SPEED == 50) {
                tmp = 0xD556;

                if (gSaveContext.save.time >= 0xD557) {
                    tmp = 0x1D556;
                }

                R_TIME_SPEED = (tmp - gSaveContext.save.time) * (1.0f / 350.0f);
            }

            dayTime = gSaveContext.save.time;

            if ((dayTime >= 0x2AAC) && (dayTime < 0x3000) && (gSaveContext.sceneLayer == 5)) {
                R_TIME_SPEED = 0;
            }
        }
    }
}

static void BgSpot00Hanebasi_DrawTorches(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    f32 angle;
    s32 i;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    if (gSaveContext.sceneLayer >= 4) {
        sTorchFlameScale = 0.008f;
    } else {
        sTorchFlameScale = ((thisx->shape.rot.x * -1) - 0x2000) * (1.0f / 1024000.0f);
    }

    angle = (s16)(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000) * (M_PI / 32768.0f);
    gDPSetPrimColor(POLY_XLU_DISP++, 128, 128, 255, 255, 0, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, 0);

    for (i = 0; i < 2; i++) {
        FrameInterpolation_RecordOpenChild("Hanebasi Torch", i);

        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0,
                                      ((play->gameplayFrames + i) * -20) & 0x1FF, 32, 128, 0, 0, 0, -20));

        Matrix_Translate((i == 0) ? 260.0f : -260.0f, 128.0f, 690.0f, MTXMODE_NEW);
        Matrix_RotateYF(angle, MTXMODE_APPLY);
        Matrix_Scale(sTorchFlameScale, sTorchFlameScale, sTorchFlameScale, MTXMODE_APPLY);

        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);

        FrameInterpolation_RecordCloseChild();
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BgSpot00Hanebasi_Draw(Actor* thisx, PlayState* play) {
    Vec3f basePos = { 158.0f, 10.0f, 400.0f };
    Vec3f newPos;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    if (thisx->params == DT_DRAWBRIDGE) {
        gSPDisplayList(POLY_OPA_DISP++, gHyruleFieldCastleDrawbridgeDL);

        Matrix_MultVec3f(&basePos, &newPos);
        thisx->child->world.pos.x = newPos.x;
        thisx->child->world.pos.y = newPos.y;
        thisx->child->world.pos.z = newPos.z;
        basePos.x *= -1.0f;

        Matrix_MultVec3f(&basePos, &newPos);
        thisx->child->child->world.pos.x = newPos.x;
        thisx->child->child->world.pos.y = newPos.y;
        thisx->child->child->world.pos.z = newPos.z;

        if (gSaveContext.sceneLayer != 12) {
            if ((gSaveContext.sceneLayer >= 4) || (!LINK_IS_ADULT_OOT && (thisx->shape.rot.x < -0x2000))) {
                BgSpot00Hanebasi_DrawTorches(thisx, play);
            } else {
                sTorchFlameScale = 0.0f;
            }
        }
    } else {
        gSPDisplayList(POLY_OPA_DISP++, gHyruleFieldCastleDrawbridgeChainsDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
