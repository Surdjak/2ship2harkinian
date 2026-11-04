#include "z_en_syateki_man.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "../ovl_En_Syateki_Itm/z_en_syateki_itm.h"
#include "assets/oot/objects/object_ossan/object_ossan.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_LOCK_ON_DISABLED)

typedef enum {
    /* 0 */ SYATEKI_RESULT_NONE,
    /* 1 */ SYATEKI_RESULT_WINNER,
    /* 2 */ SYATEKI_RESULT_ALMOST,
    /* 3 */ SYATEKI_RESULT_FAILURE,
    /* 4 */ SYATEKI_RESULT_REFUSE
} EnSyatekiManGameResult;

typedef enum {
    /* 0 */ SYATEKI_TEXT_CHOICE,
    /* 1 */ SYATEKI_TEXT_START_GAME,
    /* 2 */ SYATEKI_TEXT_NO_RUPEES,
    /* 3 */ SYATEKI_TEXT_REFUSE
} EnSyatekiManTextIdx;

static void EnSyatekiMan_Init(Actor* thisx, PlayState* play);
static void EnSyatekiMan_Destroy(Actor* thisx, PlayState* play);
static void EnSyatekiMan_Update(Actor* thisx, PlayState* play);
static void EnSyatekiMan_Draw(Actor* thisx, PlayState* play);

static void EnSyatekiMan_Start(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_SetupIdle(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_Idle(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_Talk(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_StopTalk(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_StartGame(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_WaitForGame(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_EndGame(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_GivePrize(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_FinishPrize(EnSyatekiMan* this, PlayState* play);
static void EnSyatekiMan_RestartGame(EnSyatekiMan* this, PlayState* play);

static void EnSyatekiMan_BlinkWait(EnSyatekiMan* this);
static void EnSyatekiMan_Blink(EnSyatekiMan* this);


ActorProfile En_Syateki_Man_Oot_Profile = {
    /**/ ACTOR_EN_SYATEKI_MAN_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_OSSAN_Oot,
    /**/ sizeof(EnSyatekiMan),
    /**/ EnSyatekiMan_Init,
    /**/ EnSyatekiMan_Destroy,
    /**/ EnSyatekiMan_Update,
    /**/ EnSyatekiMan_Draw,
};


static s16 sTextIds[] = { TEXT_OOT(0x2B), TEXT_OOT(0x2E), TEXT_OOT(0xC8), TEXT_OOT(0x2D) };

static s16 sTextBoxCount[] = { TEXT_STATE_CHOICE, TEXT_STATE_EVENT, TEXT_STATE_EVENT, TEXT_STATE_EVENT };

static void EnSyatekiMan_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnSyatekiMan* this = (EnSyatekiMan*)thisx;

    this->actor.attentionRangeType = 1;
    Actor_SetScale(&this->actor, 0.01f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gObjectOssanSkel, &gObjectOssanAnim_000338, this->jointTable,
                       this->morphTable, 9);
    if (!LINK_IS_ADULT_OOT) {
        this->headRot.z = 20;
    }
    this->blinkTimer = 20;
    this->eyeState = 0;
    this->blinkFunc = EnSyatekiMan_BlinkWait;
    this->actor.colChkInfo.cylRadius = 100;
    this->actionFunc = EnSyatekiMan_Start;
}

static void EnSyatekiMan_Destroy(Actor* thisx, PlayState* play) {
}

static void EnSyatekiMan_Start(EnSyatekiMan* this, PlayState* play) {
    f32 lastFrame = Animation_GetLastFrame(&gObjectOssanAnim_000338);

    Animation_Change(&this->skelAnime, &gObjectOssanAnim_000338, 1.0f, 0.0f, (s16)lastFrame, ANIMMODE_LOOP, -10.0f);
    this->actionFunc = EnSyatekiMan_SetupIdle;
}

static void EnSyatekiMan_SetupIdle(EnSyatekiMan* this, PlayState* play) {
    if (this->gameResult == SYATEKI_RESULT_REFUSE) {
        this->textIdx = SYATEKI_TEXT_REFUSE;
    }

    this->actor.textId = sTextIds[this->textIdx];
    this->numTextBox = sTextBoxCount[this->textIdx];
    this->actionFunc = EnSyatekiMan_Idle;
}

static void EnSyatekiMan_Idle(EnSyatekiMan* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnSyatekiMan_Talk;
    } else {
        Actor_OfferTalk(&this->actor, play, 100.0f);
    }
}

static void EnSyatekiMan_Talk(EnSyatekiMan* this, PlayState* play) {
    s16 nextState = 0;

    SkelAnime_Update(&this->skelAnime);
    if (this->cameraHold) {
        play->bButtonAmmoPlusOne = -2;
    }
    if ((this->numTextBox == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        if (this->textIdx == SYATEKI_TEXT_CHOICE) {
            switch (play->msgCtx.choiceIndex) {
                case 0:
                    if (gSaveContext.save.saveInfo.playerData.rupees >= 20) {
                        Rupees_ChangeBy(-20);
                        this->textIdx = SYATEKI_TEXT_START_GAME;
                        nextState = 1;
                    } else {
                        this->textIdx = SYATEKI_TEXT_NO_RUPEES;
                        nextState = 2;
                    }
                    this->actor.textId = sTextIds[this->textIdx];
                    this->numTextBox = sTextBoxCount[this->textIdx];
                    break;
                case 1:
                    this->actor.textId = sTextIds[SYATEKI_TEXT_REFUSE];
                    this->numTextBox = sTextBoxCount[SYATEKI_TEXT_REFUSE];
                    nextState = 2;
                    break;
            }
            Message_ContinueTextbox(play, this->actor.textId);
        } else {
            Message_CloseTextbox(play);
        }
        switch (nextState) {
            case 0:
                this->actionFunc = EnSyatekiMan_SetupIdle;
                break;
            case 1:
                this->actionFunc = EnSyatekiMan_StartGame;
                break;
            case 2:
                this->actionFunc = EnSyatekiMan_StopTalk;
                break;
        }
    }
}

static void EnSyatekiMan_StopTalk(EnSyatekiMan* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (this->cameraHold) {
        play->bButtonAmmoPlusOne = -2;
    }
    if ((this->numTextBox == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        if (this->cameraHold) {
            OnePointCutscene_EndCutscene(play, this->csCam);
            this->csCam = CAM_ID_NONE;
            this->cameraHold = false;
        }
        Message_CloseTextbox(play);
        this->actionFunc = EnSyatekiMan_SetupIdle;
    }
}

static void EnSyatekiMan_StartGame(EnSyatekiMan* this, PlayState* play) {
    EnSyatekiItm* gallery;

    SkelAnime_Update(&this->skelAnime);
    if (this->cameraHold) {
        play->bButtonAmmoPlusOne = -2;
    }
    if ((this->numTextBox == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        if (this->cameraHold) {
            OnePointCutscene_EndCutscene(play, this->csCam);
            this->csCam = CAM_ID_NONE;
            this->cameraHold = false;
        }
        Message_CloseTextbox(play);
        gallery = ((EnSyatekiItm*)this->actor.parent);
        if (gallery->actor.update != NULL) {
            if ((true)) {
                gallery->signal = ENSYATEKI_START;
            }
            this->actionFunc = EnSyatekiMan_WaitForGame;
        }
    }
}

static void EnSyatekiMan_WaitForGame(EnSyatekiMan* this, PlayState* play) {
    EnSyatekiItm* gallery;

    SkelAnime_Update(&this->skelAnime);
    gallery = ((EnSyatekiItm*)this->actor.parent);
    if ((gallery->actor.update != NULL) && (gallery->signal == ENSYATEKI_END)) {
        this->csCam = OnePointCutscene_Init(play, 8002, -99, &this->actor, CAM_ID_MAIN);
        switch (gallery->hitCount) {
            case 10:
                this->gameResult = SYATEKI_RESULT_WINNER;
                this->actor.textId = TEXT_OOT(0x71AF);
                break;
            case 8:
            case 9:
                this->gameResult = SYATEKI_RESULT_ALMOST;
                this->actor.textId = TEXT_OOT(0x71AE);
                break;
            default:
                this->gameResult = SYATEKI_RESULT_FAILURE;
                this->actor.textId = TEXT_OOT(0x71AD);
                if (play->bButtonAmmoPlusOne == 15 + 1) {
                    this->gameResult = SYATEKI_RESULT_REFUSE;
                    this->actor.textId = TEXT_OOT(0x2D);
                }
                break;
        }
        play->bButtonAmmoPlusOne = -2;
        Message_StartTextbox(play, this->actor.textId, NULL);
        this->actionFunc = EnSyatekiMan_EndGame;
    }
}

static void EnSyatekiMan_EndGame(EnSyatekiMan* this, PlayState* play) {
    EnSyatekiItm* gallery;

    SkelAnime_Update(&this->skelAnime);
    if ((this->numTextBox == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        if (this->gameResult != SYATEKI_RESULT_FAILURE) {
            OnePointCutscene_EndCutscene(play, this->csCam);
            this->csCam = CAM_ID_NONE;
        }
        Message_CloseTextbox(play);
        gallery = ((EnSyatekiItm*)this->actor.parent);
        if (gallery->actor.update != NULL) {
            gallery->signal = ENSYATEKI_RESULTS;
            this->textIdx = 0;
            switch (this->gameResult) {
                case SYATEKI_RESULT_WINNER:
                    this->tempGallery = this->actor.parent;
                    this->actor.parent = NULL;
                    if (!LINK_IS_ADULT_OOT) {
                        if (!GET_ITEMGETINF_OOT(ITEMGETINF_0D_Oot)) {
                            if (CUR_UPG_VALUE(UPG_BULLET_BAG) == 1) {
                                this->getItemId = GI_BULLET_BAG_40_Oot;
                            } else {
                                this->getItemId = GI_BULLET_BAG_50_Oot;
                            }
                        } else {
                            this->getItemId = GI_RUPEE_PURPLE_Oot;
                        }
                    } else {
                        // Only give the adult rando reward when the player has a quiver
                        if (!GET_ITEMGETINF_OOT(ITEMGETINF_0E_Oot)) {
                            switch (CUR_UPG_VALUE(UPG_QUIVER)) {
                                case 0:
                                    this->getItemId = GI_RUPEE_PURPLE_Oot;
                                    break;
                                case 1:
                                    this->getItemId = GI_QUIVER_40_Oot;
                                    break;
                                case 2:
                                    this->getItemId = GI_QUIVER_50_Oot;
                                    break;
                            }
                        } else {
                            this->getItemId = GI_RUPEE_PURPLE_Oot;
                        }
                    }
                    if ((true)) {
                        Actor_OfferGetItem(&this->actor, play, this->getItemId, 2000.0f, 1000.0f);
                    }
                    this->actionFunc = EnSyatekiMan_GivePrize;
                    break;
                case SYATEKI_RESULT_ALMOST:
                    this->timer = 20;
                    s32 ammunition = 15;
                    (true);
                    Player_SetBButtonAmmo(play, ammunition);
                    this->actionFunc = EnSyatekiMan_RestartGame;
                    break;
                default:
                    if (this->gameResult == SYATEKI_RESULT_REFUSE) {
                        this->actionFunc = EnSyatekiMan_SetupIdle;
                    } else {
                        this->cameraHold = true;
                        this->actor.textId = sTextIds[this->textIdx];
                        this->numTextBox = sTextBoxCount[this->textIdx];
                        Message_StartTextbox(play, this->actor.textId, NULL);
                        this->actionFunc = EnSyatekiMan_Talk;
                    }
                    break;
            }
        }
    }
}

static void EnSyatekiMan_GivePrize(EnSyatekiMan* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actionFunc = EnSyatekiMan_FinishPrize;
    } else {
        Actor_OfferGetItem(&this->actor, play, this->getItemId, 2000.0f, 1000.0f);
    }
}

static void EnSyatekiMan_FinishPrize(EnSyatekiMan* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) ||
        !(true)) {
        if (!LINK_IS_ADULT_OOT) {
            SET_ITEMGETINF_OOT(ITEMGETINF_0D_Oot);
        } else if (((this->getItemId == GI_QUIVER_40_Oot) || (this->getItemId == GI_QUIVER_50_Oot))) {
            SET_ITEMGETINF_OOT(ITEMGETINF_0E_Oot);
        }
        this->gameResult = SYATEKI_RESULT_NONE;
        this->actor.parent = this->tempGallery;
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
        this->actionFunc = EnSyatekiMan_SetupIdle;
    }
}

static void EnSyatekiMan_RestartGame(EnSyatekiMan* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (this->timer == 0) {
        EnSyatekiItm* gallery = ((EnSyatekiItm*)this->actor.parent);

        if (gallery->actor.update != NULL) {
            gallery->signal = ENSYATEKI_START;
            this->gameResult = SYATEKI_RESULT_NONE;
            this->actionFunc = EnSyatekiMan_WaitForGame;
        }
    }
}

static void EnSyatekiMan_BlinkWait(EnSyatekiMan* this) {
    s16 decrBlinkTimer = this->blinkTimer - 1;

    if (decrBlinkTimer != 0) {
        this->blinkTimer = decrBlinkTimer;
    } else {
        this->blinkFunc = EnSyatekiMan_Blink;
    }
}

static void EnSyatekiMan_Blink(EnSyatekiMan* this) {
    s16 decrBlinkTimer = this->blinkTimer - 1;

    if (decrBlinkTimer != 0) {
        this->blinkTimer = decrBlinkTimer;
    } else {
        s16 nextEyeState = this->eyeState + 1;

        if (nextEyeState >= 3) {
            this->eyeState = 0;
            this->blinkTimer = 20 + (s32)(Rand_ZeroOne() * 60.0f);
            this->blinkFunc = EnSyatekiMan_BlinkWait;
        } else {
            this->eyeState = nextEyeState;
            this->blinkTimer = 1;
        }
    }
}

static void EnSyatekiMan_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnSyatekiMan* this = (EnSyatekiMan*)thisx;

    if (this->timer != 0) {
        this->timer--;
    }
    this->actionFunc(this, play);
    this->blinkFunc(this);
    this->actor.focus.pos.y = 70.0f;
    Actor_SetFocus(&this->actor, 70.0f);
    Actor_TrackPlayer(play, &this->actor, &this->headRot, &this->bodyRot, this->actor.focus.pos);
}

static s32 EnSyatekiMan_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnSyatekiMan* this = (EnSyatekiMan*)thisx;
    s32 turnDirection;

    if (limbIndex == 1) {
        rot->x += this->bodyRot.y;
    }
    if (limbIndex == 8) {
        *dList = gObjectOssanEnSyatekiManDL_007E28;
        turnDirection = 1;
        if (this->gameResult == SYATEKI_RESULT_REFUSE) {
            turnDirection = -1;
        }
        rot->x += this->headRot.y * turnDirection;
        rot->z += this->headRot.z;
    }
    return 0;
}

static void EnSyatekiMan_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    EnSyatekiMan* this = (EnSyatekiMan*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnSyatekiMan_OverrideLimbDraw, NULL, this);
}

