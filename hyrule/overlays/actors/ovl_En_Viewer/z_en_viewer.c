/*
 * File: z_en_viewer.c
 * Overlay: ovl_En_Viewer
 * Description: Cutscene Actors
 */

#include "z_en_viewer.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"
#include "../ovl_En_Ganon_Mant/z_en_ganon_mant.h"
#include "assets/oot/objects/object_zl4/object_zl4.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/object_horse_zelda/object_horse_zelda.h"
#include "assets/oot/objects/object_horse_ganon/object_horse_ganon.h"
#include "assets/oot/objects/object_im/object_im.h"
#include "assets/oot/objects/object_gndd/object_gndd.h"
#include "assets/oot/objects/object_ganon/object_ganon.h"
#include "assets/oot/objects/object_opening_demo1/object_opening_demo1.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include <assert.h>

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnViewer_Init(Actor* thisx, PlayState* play);
static void EnViewer_Destroy(Actor* thisx, PlayState* play);
static void EnViewer_Update(Actor* thisx, PlayState* play);
static void EnViewer_Draw(Actor* thisx, PlayState* play);

static void EnViewer_UpdatePosition(EnViewer* this, PlayState* play);
static void EnViewer_DrawFireEffects(EnViewer* this2, PlayState* play);
static void EnViewer_UpdateGanondorfCape(PlayState* play, EnViewer* this);
static void EnViewer_InitImpl(EnViewer* this, PlayState* play);
static void EnViewer_UpdateImpl(EnViewer* this, PlayState* play);

static u8 sHorseSfxPlayed = false;

ActorProfile En_Viewer_Oot_Profile = {
    /**/ ACTOR_EN_VIEWER_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnViewer),
    /**/ EnViewer_Init,
    /**/ EnViewer_Destroy,
    /**/ EnViewer_Update,
    /**/ EnViewer_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeScale, 300, ICHAIN_STOP),
};

static EnViewerInitData sInitData[] = {
    /* ENVIEWER_TYPE_0_HORSE_ZELDA */
    { OBJECT_HORSE_ZELDA_Oot, OBJECT_HORSE_ZELDA_Oot, 1, 0, ENVIEWER_SHADOW_HORSE, 20, ENVIEWER_DRAW_HORSE, &gHorseZeldaSkel,
      &gHorseZeldaGallopingAnim },
    /* ENVIEWER_TYPE_1_IMPA */
    { OBJECT_IM_Oot, OBJECT_OPENING_DEMO1_Oot, 1, 0, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_IMPA, &gImpaSkel,
      &object_opening_demo1_Anim_0029CC },
    /* ENVIEWER_TYPE_2_ZELDA */
    { OBJECT_ZL4_Oot, OBJECT_OPENING_DEMO1_Oot, 1, 0, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_ZELDA, &gChildZeldaSkel,
      &object_opening_demo1_Anim_000450 },
    /* ENVIEWER_TYPE_3_GANONDORF */
    { OBJECT_GNDD_Oot, OBJECT_GNDD_Oot, 1, -6, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_GANONDORF, &gYoungGanondorfSkel,
      &gYoungGanondorfHorsebackIdleAnim },
    /* ENVIEWER_TYPE_4_HORSE_GANONDORF */
    { OBJECT_HORSE_GANON_Oot, OBJECT_HORSE_GANON_Oot, 1, 0, ENVIEWER_SHADOW_HORSE, 20, ENVIEWER_DRAW_HORSE, &gHorseGanonSkel,
      &gHorseGanonRearingAnim },
    /* ENVIEWER_TYPE_5_GANONDORF */
    { OBJECT_GNDD_Oot, OBJECT_GNDD_Oot, 1, -6, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_GANONDORF, &gYoungGanondorfSkel,
      &gYoungGanondorfHorsebackRideAnim },
    /* ENVIEWER_TYPE_6_HORSE_GANONDORF */
    { OBJECT_HORSE_GANON_Oot, OBJECT_HORSE_GANON_Oot, 1, 0, ENVIEWER_SHADOW_HORSE, 20, ENVIEWER_DRAW_HORSE, &gHorseGanonSkel,
      &gHorseGanonGallopingAnim },
    /* ENVIEWER_TYPE_7_GANONDORF */
    { OBJECT_GNDD_Oot, OBJECT_GNDD_Oot, 1, -6, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_GANONDORF, &gYoungGanondorfSkel,
      &gYoungGanondorfArmsCrossedAnim },
    /* ENVIEWER_TYPE_8_GANONDORF */
    { OBJECT_GNDD_Oot, OBJECT_GNDD_Oot, 1, -6, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_GANONDORF, &gYoungGanondorfSkel,
      &gYoungGanondorfWalkAnim },
    /* ENVIEWER_TYPE_9_GANONDORF */
    { OBJECT_GANON_Oot, OBJECT_GANON_Oot, 1, -6, ENVIEWER_SHADOW_NONE, 10, ENVIEWER_DRAW_GANONDORF, &gGanondorfSkel,
      &gGanondorfEndingFloatAnim },
};

static EnGanonMant* sGanondorfCape;

static Vec3f sGanondorfNeckWorldPos;

static void EnViewer_SetupAction(EnViewer* this, EnViewerActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void EnViewer_Init(Actor* thisx, PlayState* play) {
    EnViewer* this = (EnViewer*)thisx;
    u8 type;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    EnViewer_SetupAction(this, EnViewer_InitImpl);
    sHorseSfxPlayed = false;
    type = this->actor.params >> 8;
    this->unused = 0;
    this->state = 0;
    this->isVisible = false;
    if (type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_5_GANONDORF || type == ENVIEWER_TYPE_7_GANONDORF ||
        type == ENVIEWER_TYPE_8_GANONDORF || type == ENVIEWER_TYPE_9_GANONDORF) {
        sGanondorfCape = (EnGanonMant*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_GANON_MANT_Oot,
                                                          0.0f, 0.0f, 0.0f, 0, 0, 0, 35);
    }
}

static void EnViewer_Destroy(Actor* thisx, PlayState* play) {
    EnViewer* this = (EnViewer*)thisx;

    Skin_Free(play, &this->skin);
}

static void EnViewer_InitAnimGanondorfOrZelda(EnViewer* this, PlayState* play, void* skeletonHeaderSeg,
                                       AnimationHeader* anim) {
    s16 type = this->actor.params >> 8;

    if (type == ENVIEWER_TYPE_2_ZELDA || type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_5_GANONDORF ||
        type == ENVIEWER_TYPE_7_GANONDORF || type == ENVIEWER_TYPE_8_GANONDORF || type == ENVIEWER_TYPE_9_GANONDORF) {
        SkelAnime_InitFlex(play, &this->skin.skelAnime, skeletonHeaderSeg, NULL, NULL, NULL, 0);
    } else {
        SkelAnime_Init(play, &this->skin.skelAnime, skeletonHeaderSeg, NULL, NULL, NULL, 0);
    }

    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->animObjBankIndex].segment);
    if (type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_7_GANONDORF || type == ENVIEWER_TYPE_8_GANONDORF ||
        type == ENVIEWER_TYPE_9_GANONDORF) {
        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, anim, 1.0f);
    } else {
        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, anim, 3.0f);
    }
}

static void EnViewer_InitAnimImpa(EnViewer* this, PlayState* play, void* skeletonHeaderSeg, AnimationHeader* anim) {
    SkelAnime_InitFlex(play, &this->skin.skelAnime, skeletonHeaderSeg, NULL, NULL, NULL, 0);
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->animObjBankIndex].segment);
    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, anim, 3.0f);
}

static void EnViewer_InitAnimHorse(EnViewer* this, PlayState* play, void* skeletonHeaderSeg, AnimationHeader* anim) {
    u8 type;

    Skin_Init(play, &this->skin, skeletonHeaderSeg, anim);
    type = this->actor.params >> 8;
    if (!(type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_4_HORSE_GANONDORF ||
          type == ENVIEWER_TYPE_7_GANONDORF || type == ENVIEWER_TYPE_8_GANONDORF ||
          type == ENVIEWER_TYPE_9_GANONDORF)) {
        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, anim, 3.0f);
    } else {
        Animation_PlayOnceSetSpeed(&this->skin.skelAnime, anim, 1.0f);
    }
}

static EnViewerInitAnimFunc sInitAnimFuncs[] = {
    EnViewer_InitAnimGanondorfOrZelda,
    EnViewer_InitAnimHorse,
    EnViewer_InitAnimGanondorfOrZelda,
    EnViewer_InitAnimImpa,
};

static ActorShadowFunc sShadowDrawFuncs[] = {
    NULL,
    ActorShadow_DrawCircle,
    ActorShadow_DrawHorse,
};

static void EnViewer_InitImpl(EnViewer* this, PlayState* play) {
    EnViewerInitData* initData = &sInitData[this->actor.params >> 8];
    s32 skelObjBankIndex = Object_GetSlot(&play->objectCtx, initData->skeletonObject);

    assert(skelObjBankIndex >= 0);

    this->animObjBankIndex = Object_GetSlot(&play->objectCtx, initData->animObject);
    assert(this->animObjBankIndex >= 0);

    if (!Object_IsLoaded(&play->objectCtx, skelObjBankIndex) ||
        !Object_IsLoaded(&play->objectCtx, this->animObjBankIndex)) {
        this->actor.flags &= ~ACTOR_FLAG_INSIDE_CULLING_VOLUME;
        return;
    }

    this->isVisible = true;
    this->actor.objectSlot = skelObjBankIndex;
    Actor_SetObjectDependency(play, &this->actor);
    Actor_SetScale(&this->actor, initData->scale / 100.0f);
    ActorShape_Init(&this->actor.shape, initData->yOffset * 100, sShadowDrawFuncs[initData->shadowType],
                    initData->shadowScale);
    this->drawFuncIndex = initData->drawType;
    sInitAnimFuncs[this->drawFuncIndex](this, play, initData->skeletonHeaderSeg, initData->anim);
    EnViewer_SetupAction(this, EnViewer_UpdateImpl);
}

static s16 sTimer = 0;

static void EnViewer_UpdateImpl(EnViewer* this, PlayState* play) {
    u8 type = this->actor.params >> 8;
    u16 csFrames;
    s32 animationEnded;

    if (type == ENVIEWER_TYPE_2_ZELDA) {
        if (gSaveContext.sceneLayer == 5) {
            csFrames = gCsCtxOot.frames;
            if (csFrames == 792) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_SURPRISE);
            } else if (csFrames == 845) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_THROW);
            }
        }
    } else if (type == ENVIEWER_TYPE_7_GANONDORF) {
        Actor_SetScale(&this->actor, 0.3f);
        this->actor.cullingVolumeDistance = 10000.0f;
        this->actor.cullingVolumeScale = 10000.0f;
        this->actor.cullingVolumeDownward = 10000.0f;
    } else if (type == ENVIEWER_TYPE_3_GANONDORF) {
        if (gSaveContext.sceneLayer == 4) {
            switch (gCsCtxOot.frames) {
                case 20:
                case 59:
                case 71:
                case 129:
                case 140:
                case 219:
                case 280:
                case 320:
                case 380:
                case 409:
                case 438:
                    Audio_PlaySfx(NA_SE_SY_DEMO_CUT);
                    break;
            }
        }
        if (gSaveContext.sceneLayer == 5) {
            if (gCsCtxOot.frames == 1508) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DAMAGE2_OLD);
            }
            if (gCsCtxOot.frames == 1545) {
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DEMO_6K_Oot, 32.0f, 101.0f, 1226.0f, 0, 0, 0,
                                   0xC);
            }
        }
        if (gCsCtxOot.frames == 1020) {
            SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_FANFARE, 0, SEQ_OOT(0x23)); // OoT's NA_BGM_OPENING_GANON
        }
        if (gCsCtxOot.frames == 960) {
            Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_EV_HORSE_GROAN);
        }
    } else if (type == ENVIEWER_TYPE_6_HORSE_GANONDORF) {
        if (gSaveContext.sceneLayer == 5 || gSaveContext.sceneLayer == 10) {
            Actor_PlaySfx(&this->actor, NA_SE_EV_AUTO_DOOR_CLOSE - SFX_FLAG);
        }
    } else if (type == ENVIEWER_TYPE_4_HORSE_GANONDORF) {
        s16 curFrame = this->skin.skelAnime.curFrame;

        if (this->skin.skelAnime.animation == &gHorseGanonRearingAnim) {
            if (curFrame == 8) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_WATERDROP_GRD);
            }
            if (curFrame == 30) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_KID_HORSE_LAND2);
            }
        } else if (this->skin.skelAnime.animation == &gHorseGanonIdleAnim) {
            if (curFrame == 25) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_KID_HORSE_SANDDUST);
            }
        } else if (this->skin.skelAnime.animation == &gHorseGanonGallopingAnim) {
            Actor_PlaySfx(&this->actor, NA_SE_EV_AUTO_DOOR_CLOSE - SFX_FLAG);
        }
    }

    if (sTimer != 0) {
        sTimer--;
    }

    EnViewer_UpdatePosition(this, play);
    Actor_MoveWithGravity(&this->actor); // has no effect, speed/velocity and gravity are 0

    animationEnded = SkelAnime_Update(&this->skin.skelAnime);
    if (type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_4_HORSE_GANONDORF) {
        if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[1] != NULL) {
            if (gCsCtxOot.npcActions[1]->action == 2 && sTimer == 0) {
                if (type == ENVIEWER_TYPE_3_GANONDORF) {
                    if (this->skin.skelAnime.animation != &gYoungGanondorfHorsebackIdleAnim) {
                        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gYoungGanondorfHorsebackIdleAnim, 1.0f);
                    }
                } else if (this->skin.skelAnime.animation != &gHorseGanonIdleAnim) {
                    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gHorseGanonIdleAnim, 1.0f);
                }
            } else if (gCsCtxOot.npcActions[1]->action == 1) {
                sTimer = 100;
                if (type == ENVIEWER_TYPE_3_GANONDORF) {
                    if (this->skin.skelAnime.animation != &gYoungGanondorfHorsebackRearAnim) {
                        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gYoungGanondorfHorsebackRearAnim, 1.0f);
                    }
                } else if (this->skin.skelAnime.animation != &gHorseGanonRearingAnim) {
                    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gHorseGanonRearingAnim, 1.0f);
                }
            } else if (type == ENVIEWER_TYPE_3_GANONDORF) {
                switch (this->state) {
                    case 0:
                        if (gCsCtxOot.npcActions[1]->action == 4) {
                            Animation_MorphToPlayOnce(&this->skin.skelAnime,
                                                      &gYoungGanondorfHorsebackLookSidewaysStartAnim, -5.0f);
                            this->state++;
                        }
                        break;
                    case 1:
                        if (animationEnded) {
                            Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfHorsebackLookSidewaysLoopAnim,
                                                  -5.0f);
                            this->state++;
                        }
                        break;
                    case 2:
                        if (gCsCtxOot.npcActions[1]->action == 5) {
                            Animation_MorphToPlayOnce(&this->skin.skelAnime,
                                                      &gYoungGanondorfHorsebackMagicChargeUpStartAnim, -5.0f);
                            this->state++;
                        }
                        break;
                    case 3:
                        if (animationEnded) {
                            Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfHorsebackMagicChargeUpLoopAnim,
                                                  -5.0f);
                            this->state++;
                        }
                        break;
                    case 4:
                        if (gCsCtxOot.npcActions[1]->action == 11) {
                            Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfHorsebackLookSidewaysLoopAnim,
                                                  -20.0f);
                            this->state++;
                        }
                        break;
                    case 5:
                        if (gCsCtxOot.npcActions[1]->action == 8) {
                            Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfHorsebackIdleAnim, -15.0f);
                            this->state++;
                        }
                        break;
                    case 6:
                        if (gCsCtxOot.npcActions[1]->action == 12) {
                            Actor_PlaySfx(&this->actor, NA_SE_EN_BAKUO_DEAD);
                            Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gYoungGanondorfHorsebackRideAnim, 3.0f);
                            this->state++;
                        }
                        break;
                    case 7:
                        this->state = 0;
                        break;
                }
            } else if (this->skin.skelAnime.animation != &gHorseGanonGallopingAnim &&
                       gCsCtxOot.npcActions[1]->action == 12) {
                Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gHorseGanonGallopingAnim, 3.0f);
            }
        }
    } else if (type == ENVIEWER_TYPE_1_IMPA) {
        if (gSaveContext.sceneLayer == 5) {
            if (gCsCtxOot.frames == 845) {
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_ITEM_OCARINA_Oot, 4.0f, 81.0f, 2600.0f, 0, 0,
                                   0, 0);
            }
        } else {
            if (gCsCtxOot.frames == 195) {
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_ITEM_OCARINA_Oot, 4.0f, 81.0f, 2035.0f, 0, 0,
                                   0, 1);
            }
        }
        switch (this->state) {
            case 0:
                if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[0] != NULL &&
                    gCsCtxOot.npcActions[0]->action == 6 &&
                    this->skin.skelAnime.animation != &object_opening_demo1_Anim_002574) {
                    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &object_opening_demo1_Anim_002574, 1.5f);
                    this->state++;
                }
                break;
            case 1:
                if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[0] != NULL &&
                    gCsCtxOot.npcActions[0]->action == 2 &&
                    this->skin.skelAnime.animation != &object_opening_demo1_Anim_0029CC) {
                    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &object_opening_demo1_Anim_0029CC, 3.0f);
                    this->state++;
                }
                break;
        }
    } else if (type == ENVIEWER_TYPE_2_ZELDA) {
        if (play->sceneId == SCENE_HYRULE_FIELD_Oot) { // Hyrule Field
            switch (this->state) {
                case 0:
                    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
                        if (gCsCtxOot.npcActions[0] != NULL && gCsCtxOot.npcActions[0]->action == 6 &&
                            this->skin.skelAnime.animation != &object_opening_demo1_Anim_001410) {
                            Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &object_opening_demo1_Anim_001410, 1.5f);
                            this->state++;
                        }
                    }
                    break;
                case 1:
                    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
                        if (gCsCtxOot.npcActions[0] != NULL && gCsCtxOot.npcActions[0]->action == 2 &&
                            this->skin.skelAnime.animation != &object_opening_demo1_Anim_000450) {
                            Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &object_opening_demo1_Anim_000450, 3.0f);
                            this->state++;
                        }
                    }
                    break;
            }
        } else {
            Audio_SetBaseFilter(0);
            switch (this->state) {
                case 0:
                    Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &object_opening_demo1_Anim_00504C, 1.0f);
                    this->state++;
                    break;
                case 1:
                    if (gCsCtxOot.npcActions[0]->action == 11) {
                        Animation_MorphToPlayOnce(&this->skin.skelAnime, &object_opening_demo1_Anim_00420C, -5.0f);
                        this->state++;
                    }
                    break;
                case 2:
                    if (animationEnded) {
                        Animation_MorphToLoop(&this->skin.skelAnime, &object_opening_demo1_Anim_0048FC, -5.0f);
                        this->state++;
                    }
                    break;
                case 3:
                    break;
            }
        }
    } else if (type == ENVIEWER_TYPE_7_GANONDORF) {
        switch (this->state) {
            case 0:
                if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[1] != NULL &&
                    gCsCtxOot.npcActions[1]->action == 7) {
                    Audio_PlaySfx(NA_SE_EN_ANSATSUSYA_SWORD);
                    Animation_MorphToPlayOnce(&this->skin.skelAnime, &gYoungGanondorfLaughStartAnim, -5.0f);
                    this->state++;
                }
                break;
            case 1:
                if (animationEnded) {
                    Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfLaughLoopAnim, -5.0f);
                    this->state++;
                }
                break;
        }
    } else if (type == ENVIEWER_TYPE_8_GANONDORF) {
        switch (this->state) {
            case 0:
                if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
                    if (gCsCtxOot.npcActions[1] != NULL && gCsCtxOot.npcActions[1]->action == 9) {
                        Animation_PlayLoopSetSpeed(&this->skin.skelAnime, &gYoungGanondorfWalkAnim, 1.0f);
                        this->state++;
                    }
                }
                break;
            case 1:
                if (gCsCtxOot.npcActions[1]->action == 10) {
                    Animation_MorphToPlayOnce(&this->skin.skelAnime, &gYoungGanondorfKneelStartAnim, -10.0f);
                    this->state++;
                }
                break;
            case 2:
                if (animationEnded) {
                    Animation_MorphToLoop(&this->skin.skelAnime, &gYoungGanondorfKneelLoopAnim, -5.0f);
                    this->state++;
                }
                break;
            case 3:
                if (gCsCtxOot.npcActions[1]->action == 4) {
                    Animation_MorphToPlayOnce(&this->skin.skelAnime, &gYoungGanondorfKneelLookSidewaysAnim, -5.0f);
                    this->state++;
                }
                break;
            default:
                this->state = 0;
                break;
        }
    }
}

static void EnViewer_Update(Actor* thisx, PlayState* play) {
    EnViewer* this = (EnViewer*)thisx;

    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->animObjBankIndex].segment);
    this->actionFunc(this, play);
}

static s32 EnViewer_Ganondorf3OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                        void* thisx) {
    if (gSaveContext.sceneLayer == 4) {
        if (gCsCtxOot.frames >= 400) {
            if (limbIndex == 5) {
                *dList = gYoungGanondorfOpenLeftHandDL;
            }
        }
    } else {
        if (gCsCtxOot.frames >= 1510 && gCsCtxOot.frames <= 1650) {
            if (limbIndex == 5) {
                *dList = gYoungGanondorfOpenLeftHandDL;
            }
        }
    }
    return false;
}

static void EnViewer_Ganondorf9PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    if (limbIndex == 11) {
        OPEN_DISPS(play->state.gfxCtx);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, Lib_SegmentedToVirtual(gGanondorfEyesDL));
        CLOSE_DISPS(play->state.gfxCtx);
    }
}

static void EnViewer_GanondorfPostLimbDrawUpdateCapeVec(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };

    if (limbIndex == 15) {
        Matrix_MultVec3f(&zeroVec, &sGanondorfNeckWorldPos);
    }
}

static void EnViewer_DrawGanondorf(EnViewer* this, PlayState* play) {
    s16 frames = 0;
    s16 type;

    OPEN_DISPS(play->state.gfxCtx);
    type = this->actor.params >> 8;
    if (type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_5_GANONDORF || type == ENVIEWER_TYPE_7_GANONDORF ||
        type == ENVIEWER_TYPE_8_GANONDORF) {
        if (gSaveContext.sceneLayer != 4) {
            frames = 149;
        }

        if (frames + 1127 >= gCsCtxOot.frames) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(&gYoungGanondorfEyeOpenTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(&gYoungGanondorfEyeOpenTex));

        } else if (frames + 1128 >= gCsCtxOot.frames) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(&gYoungGanondorfEyeHalfTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(&gYoungGanondorfEyeHalfTex));

        } else if (frames + 1129 >= gCsCtxOot.frames) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(&gYoungGanondorfEyeClosedTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(&gYoungGanondorfEyeClosedTex));

        } else {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(&gYoungGanondorfEyeLookingDownTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(&gYoungGanondorfEyeLookingDownTex));
        }
    } else if (type == ENVIEWER_TYPE_9_GANONDORF) {
        gSPSegment(POLY_XLU_DISP++, 0x08, Lib_SegmentedToVirtual(&gGanondorfCrazedEyeTex));
    }

    if (type == ENVIEWER_TYPE_9_GANONDORF) {
        SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, NULL, EnViewer_Ganondorf9PostLimbDraw, this);
    } else if (type == ENVIEWER_TYPE_3_GANONDORF) {
        SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, EnViewer_Ganondorf3OverrideLimbDraw,
                                  EnViewer_GanondorfPostLimbDrawUpdateCapeVec, this);
        EnViewer_UpdateGanondorfCape(play, this);
    } else if (type == ENVIEWER_TYPE_3_GANONDORF || type == ENVIEWER_TYPE_5_GANONDORF ||
               type == ENVIEWER_TYPE_7_GANONDORF || type == ENVIEWER_TYPE_8_GANONDORF) {
        if ((gCsCtxOot.state != CS_STATE_IDLE_Oot) && (gCsCtxOot.npcActions[1] != NULL)) {
            SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, NULL, EnViewer_GanondorfPostLimbDrawUpdateCapeVec,
                                      this);
            EnViewer_UpdateGanondorfCape(play, this);
        }
    } else {
        SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, NULL, NULL, this);
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnViewer_DrawHorse(EnViewer* this, PlayState* play) {
    func_80138228(&this->actor, play, &this->skin, NULL, true);
}

static s32 EnViewer_ZeldaOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    if (play->sceneId == SCENE_HYRULE_FIELD_Oot) { // Hyrule Field
        if (limbIndex == 2) {
            *dList = gChildZeldaCutsceneDressDL;
        }
        if (limbIndex == 7) {
            *dList = NULL;
        }
        if (limbIndex == 8) {
            *dList = NULL;
        }
        if (limbIndex == 9) {
            *dList = NULL;
        }
        if (limbIndex == 3) {
            *dList = NULL;
        }
        if (limbIndex == 5) {
            *dList = NULL;
        }
    }
    return false;
}

static void EnViewer_ZeldaPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    s32 pad;

    if (play->sceneId == SCENE_TEMPLE_OF_TIME_Oot) {
        if (limbIndex == 16) {
            OPEN_DISPS(play->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, gChildZeldaOcarinaOfTimeDL);
            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
}

static void EnViewer_DrawZelda(EnViewer* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);
    if (play->sceneId == SCENE_HYRULE_FIELD_Oot) { // Hyrule Field
        if (gCsCtxOot.frames < 771) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeInTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeOutTex));
        } else if (gCsCtxOot.frames < 772) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeBlinkTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeBlinkTex));
        } else if (gCsCtxOot.frames < 773) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
        } else if (gCsCtxOot.frames < 791) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeWideTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeWideTex));
        } else if (gCsCtxOot.frames < 792) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeBlinkTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeBlinkTex));
        } else if (gCsCtxOot.frames < 793) {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
        } else {
            gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeInTex));
            gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeOutTex));
        }

        if (gSaveContext.sceneLayer == 6) {
            gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gChildZeldaMouthSurprisedTex));
        } else {
            if (gCsCtxOot.frames < 758) {
                gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gChildZeldaMouthWorriedTex));
            } else if (gCsCtxOot.frames < 848) {
                gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gChildZeldaMouthSurprisedTex));
            } else {
                gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gChildZeldaMouthWorriedTex));
            }
        }
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
        gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gChildZeldaEyeShutTex));
        gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gChildZeldaMouthWorriedTex));
    }
    SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, EnViewer_ZeldaOverrideLimbDraw, EnViewer_ZeldaPostLimbDraw,
                              this);
    CLOSE_DISPS(play->state.gfxCtx);
}

static s32 EnViewer_ImpaOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    if (limbIndex == 16) {
        *dList = gImpaHeadMaskedDL;
    }
    return false;
}

static void EnViewer_DrawImpa(EnViewer* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gImpaEyeOpenTex));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gImpaEyeOpenTex));
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSPSegment(POLY_OPA_DISP++, 0x0C, &D_80116280_Oot[2]);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skin.skelAnime, EnViewer_ImpaOverrideLimbDraw, NULL, this);
    CLOSE_DISPS(play->state.gfxCtx);
}

static EnViewerDrawFunc sDrawFuncs[] = {
    EnViewer_DrawGanondorf,
    EnViewer_DrawHorse,
    EnViewer_DrawZelda,
    EnViewer_DrawImpa,
};

static void EnViewer_Draw(Actor* thisx, PlayState* play) {
    EnViewer* this = (EnViewer*)thisx;
    s32 pad;
    s16 type;

    OPEN_DISPS(play->state.gfxCtx);
    if (this->isVisible) {
        type = this->actor.params >> 8;
        if (type <= ENVIEWER_TYPE_2_ZELDA) { // zelda's horse, impa and zelda
            if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[0] != NULL) {
                Gfx_SetupDL25_Opa(play->state.gfxCtx);
                sDrawFuncs[this->drawFuncIndex](this, play);
            }
        } else if ((gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[1] != NULL) ||
                   type == ENVIEWER_TYPE_9_GANONDORF) {
            Gfx_SetupDL25_Opa(play->state.gfxCtx);
            sDrawFuncs[this->drawFuncIndex](this, play);
        }
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnViewer_UpdatePosition(EnViewer* this, PlayState* play) {
    Vec3f startPos;
    Vec3f endPos;
    f32 lerpFactor;
    s16 type = this->actor.params >> 8;

    if (type <= ENVIEWER_TYPE_2_ZELDA) { // zelda's horse, impa and zelda
        if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[0] != NULL &&
            gCsCtxOot.frames < gCsCtxOot.npcActions[0]->endFrame) {
            if (type == ENVIEWER_TYPE_0_HORSE_ZELDA) {
                if (!sHorseSfxPlayed) {
                    sHorseSfxPlayed = true;
                    AudioSfx_PlaySfx(NA_SE_EV_HORSE_NEIGH, &this->actor.projectedPos, 4,
                                         &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                }
                Actor_PlaySfx(&this->actor, NA_SE_EV_AUTO_DOOR_CLOSE - SFX_FLAG);
            }

            startPos.x = gCsCtxOot.npcActions[0]->startPos.x;
            startPos.y = gCsCtxOot.npcActions[0]->startPos.y;
            startPos.z = gCsCtxOot.npcActions[0]->startPos.z;
            endPos.x = gCsCtxOot.npcActions[0]->endPos.x;
            endPos.y = gCsCtxOot.npcActions[0]->endPos.y;
            endPos.z = gCsCtxOot.npcActions[0]->endPos.z;
            lerpFactor = Environment_LerpWeight(gCsCtxOot.npcActions[0]->endFrame,
                                                gCsCtxOot.npcActions[0]->startFrame, gCsCtxOot.frames);
            this->actor.world.pos.x = (endPos.x - startPos.x) * lerpFactor + startPos.x;
            this->actor.world.pos.y = (endPos.y - startPos.y) * lerpFactor + startPos.y;
            this->actor.world.pos.z = (endPos.z - startPos.z) * lerpFactor + startPos.z;
        }
    } else { // ganondorf and ganondorf's horse
        if (gCsCtxOot.state != CS_STATE_IDLE_Oot && gCsCtxOot.npcActions[1] != NULL &&
            gCsCtxOot.frames < gCsCtxOot.npcActions[1]->endFrame) {
            startPos.x = gCsCtxOot.npcActions[1]->startPos.x;
            startPos.y = gCsCtxOot.npcActions[1]->startPos.y;
            startPos.z = gCsCtxOot.npcActions[1]->startPos.z;
            endPos.x = gCsCtxOot.npcActions[1]->endPos.x;
            endPos.y = gCsCtxOot.npcActions[1]->endPos.y;
            endPos.z = gCsCtxOot.npcActions[1]->endPos.z;
            lerpFactor = Environment_LerpWeight(gCsCtxOot.npcActions[1]->endFrame,
                                                gCsCtxOot.npcActions[1]->startFrame, gCsCtxOot.frames);
            this->actor.world.pos.x = (endPos.x - startPos.x) * lerpFactor + startPos.x;
            this->actor.world.pos.y = (endPos.y - startPos.y) * lerpFactor + startPos.y;
            this->actor.world.pos.z = (endPos.z - startPos.z) * lerpFactor + startPos.z;

            if (gCsCtxOot.npcActions[1]->action == 12) {
                s16 yaw = Math_Vec3f_Yaw(&startPos, &endPos);

                Math_SmoothStepToS(&this->actor.world.rot.y, yaw, 0xA, 0x3E8, 1);
                Math_SmoothStepToS(&this->actor.shape.rot.y, yaw, 0xA, 0x3E8, 1);
            }

            if (type == ENVIEWER_TYPE_9_GANONDORF) {
                this->actor.world.rot.x = gCsCtxOot.npcActions[1]->urot.x;
                this->actor.world.rot.y = gCsCtxOot.npcActions[1]->urot.y;
                this->actor.world.rot.z = gCsCtxOot.npcActions[1]->urot.z;
                this->actor.shape.rot.x = gCsCtxOot.npcActions[1]->urot.x;
                this->actor.shape.rot.y = gCsCtxOot.npcActions[1]->urot.y;
                this->actor.shape.rot.z = gCsCtxOot.npcActions[1]->urot.z;
            }
        }
        if (type == ENVIEWER_TYPE_5_GANONDORF) {
            AudioSfx_PlaySfx(NA_SE_EV_BURNING - SFX_FLAG, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                 &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            EnViewer_DrawFireEffects(this, play);
        }
    }
}

static void EnViewer_InitFireEffect(EnViewer* this, PlayState* play, s16 i) {
    EnViewerFireEffect* eff;

    if ((i % 2) == 0) {
        eff = &this->fireEffects[i];
        eff->startPos.x = 100.0f;
        eff->startPos.y = -420.0f;
        eff->startPos.z = 400.0f;
        eff->endPos.x = 100.0f;
        eff->endPos.y = -420.0f;
        eff->endPos.z = -400.0f;
        eff->scale = (Rand_ZeroOne() * 5.0f + 12.0f) * 0.001f;
        eff->epoch++;
    } else {
        eff = &this->fireEffects[i];
        eff->startPos.x = -100.0f;
        eff->startPos.y = -420.0f;
        eff->startPos.z = 400.0f;
        eff->endPos.x = -100.0f;
        eff->endPos.y = -420.0f;
        eff->endPos.z = -400.0f;
        eff->scale = (Rand_ZeroOne() * 5.0f + 12.0f) * 0.001f;
        eff->epoch++;
    }
    if (this) {}
}

static void EnViewer_DrawFireEffects(EnViewer* this2, PlayState* play) {
    EnViewer* this = this2;
    s16 i;

    OPEN_DISPS(play->state.gfxCtx);
    for (i = 0; i < ARRAY_COUNT(this->fireEffects); i++) {
        switch (this->fireEffects[i].state) {
            case 0:
                EnViewer_InitFireEffect(this, play, i);
                this->fireEffects[i].lerpFactor = (i >> 1) * 0.1f;
                this->fireEffects[i].lerpFactorSpeed = 0.01f;
                this->fireEffects[i].state++;
                break;
            case 1:
                Math_SmoothStepToF(&this->fireEffects[i].lerpFactor, 1.0f, 1.0f, this->fireEffects[i].lerpFactorSpeed,
                                   this->fireEffects[i].lerpFactorSpeed);
                this->fireEffects[i].pos.x =
                    this->fireEffects[i].startPos.x +
                    (this->fireEffects[i].endPos.x - this->fireEffects[i].startPos.x) * this->fireEffects[i].lerpFactor;
                this->fireEffects[i].pos.y =
                    this->fireEffects[i].startPos.y +
                    (this->fireEffects[i].endPos.y - this->fireEffects[i].startPos.y) * this->fireEffects[i].lerpFactor;
                this->fireEffects[i].pos.z =
                    this->fireEffects[i].startPos.z +
                    (this->fireEffects[i].endPos.z - this->fireEffects[i].startPos.z) * this->fireEffects[i].lerpFactor;
                if (this->fireEffects[i].lerpFactor >= 1.0f) {
                    this->fireEffects[i].state++;
                }
                break;
            case 2:
                EnViewer_InitFireEffect(this, play, i);
                this->fireEffects[i].lerpFactor = 0.0f;
                this->fireEffects[i].lerpFactorSpeed = 0.01f;
                this->fireEffects[i].state--;
                break;
        }

        FrameInterpolation_RecordOpenChild(&this->fireEffects[i], this->fireEffects[i].epoch);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        Matrix_Translate(this->fireEffects[i].pos.x, this->fireEffects[i].pos.y, this->fireEffects[i].pos.z,
                         MTXMODE_NEW);
        Matrix_Scale(this->fireEffects[i].scale, this->fireEffects[i].scale, this->fireEffects[i].scale, MTXMODE_APPLY);
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0,
                                      (10 * i - 20 * play->state.frames) % 512, 32, 128, 0, 0, 0, -20));
        gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 170, 255);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 50, 00, 255);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, D_01000000_TO_SEGMENTED, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);
        FrameInterpolation_RecordCloseChild();
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnViewer_UpdateGanondorfCape(PlayState* play, EnViewer* this) {
    static s16 yOscillationPhase = 0;
    Vec3f forearmModelOffset;
    Vec3f forearmWorldOffset;

    if ((this->actor.params >> 8) == ENVIEWER_TYPE_5_GANONDORF) {
        sGanondorfCape->backPush = BREG(54) / 10.0f;
        sGanondorfCape->backSwayMagnitude = (BREG(60) + 25) / 100.0f;
        sGanondorfCape->sideSwayMagnitude = (BREG(55) - 45) / 10.0f;
        sGanondorfCape->minY = -10000.0f;
        sGanondorfCape->minDist = 0.0f;
        sGanondorfCape->gravity = (BREG(67) - 10) / 10.0f;
        forearmModelOffset.x = KREG(16) - 13.0f;
        forearmModelOffset.y = KREG(17) + 3.0f + Math_SinS(yOscillationPhase) * KREG(20);
        forearmModelOffset.z = KREG(18) - 10.0f;
        yOscillationPhase += KREG(19) * 0x1000 + 0x2000;

        Matrix_RotateYF((this->actor.shape.rot.y / (f32)0x8000) * M_PI, MTXMODE_NEW);
        Matrix_MultVec3f(&forearmModelOffset, &forearmWorldOffset);
        sGanondorfCape->rightForearmPos.x = sGanondorfNeckWorldPos.x + forearmWorldOffset.x;
        sGanondorfCape->rightForearmPos.y = sGanondorfNeckWorldPos.y + forearmWorldOffset.y;
        sGanondorfCape->rightForearmPos.z = sGanondorfNeckWorldPos.z + forearmWorldOffset.z;
        forearmModelOffset.x = -(KREG(16) - 13.0f);
        Matrix_MultVec3f(&forearmModelOffset, &forearmWorldOffset);
        sGanondorfCape->leftForearmPos.x = sGanondorfNeckWorldPos.x + forearmWorldOffset.x;
        sGanondorfCape->leftForearmPos.y = sGanondorfNeckWorldPos.y + forearmWorldOffset.y;
        sGanondorfCape->leftForearmPos.z = sGanondorfNeckWorldPos.z + forearmWorldOffset.z;
    }
}
