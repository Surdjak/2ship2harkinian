/*
 * File: z_en_eg.c
 * Overlay: ovl_En_Eg
 * Description: Triggers a void out (used in the tower collapse sequence?)
 */

#include "z_en_eg.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnEg_Init(Actor* thisx, PlayState* play);
static void EnEg_Destroy(Actor* thisx, PlayState* play);
static void EnEg_Update(Actor* thisx, PlayState* play);
static void EnEg_Draw(Actor* thisx, PlayState* play);

static void func_809FFDC8(EnEg* this, PlayState* play);

static s32 voided = false;

static EnEgActionFunc sActionFuncs[] = {
    func_809FFDC8,
};

ActorProfile En_Eg_Oot_Profile = {
    /**/ ACTOR_EN_EG_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_ZL2_Oot,
    /**/ sizeof(EnEg),
    /**/ EnEg_Init,
    /**/ EnEg_Destroy,
    /**/ EnEg_Update,
    /**/ EnEg_Draw,
};

static void EnEg_PlayVoidOutSFX() {
    Audio_PlaySfx_2(NA_SE_OC_ABYSS);
}

static void EnEg_Destroy(Actor* thisx, PlayState* play) {
    voided = false;
}

static void EnEg_Init(Actor* thisx, PlayState* play) {
    EnEg* this = (EnEg*)thisx;

    this->action = 0;
}

static void func_809FFDC8(EnEg* this, PlayState* play) {
    if (!voided && (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_2] / 100)) < 1) && Flags_GetSwitch(play, 0x36) && (kREG(0) == 0)) {
        // Void the player out
        Hyrule_TriggerRespawnOot(play);
        gSaveContext.respawnFlag = -2;
        AudioSeq_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | SEQ_OOT(0x100000FF));
        play->transitionType = TRANS_TYPE_FADE_BLACK;
        EnEg_PlayVoidOutSFX();
        voided = true;
    }
}

static void EnEg_Update(Actor* thisx, PlayState* play) {
    EnEg* this = (EnEg*)thisx;
    s32 action = this->action;

    if (((action < 0) || (0 < action)) || (sActionFuncs[action] == NULL)) {
    } else {
        sActionFuncs[action](this, play);
    }
}

static void EnEg_Draw(Actor* thisx, PlayState* play) {
}
