/*
 * File: z_en_ge1.c
 * Overlay: ovl_En_Ge1
 * Description: White-clothed Gerudo
 */

#include "z_en_ge1.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "z64horse.h"
#include "assets/oot/objects/object_ge1/object_ge1.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

#define GE1_STATE_TALKING (1 << 0)
#define GE1_STATE_GIVE_QUIVER (1 << 1)
#define GE1_STATE_IDLE_ANIM (1 << 2)
#define GE1_STATE_STOP_FIDGET (1 << 3)

typedef enum {
    /* 00 */ GE1_HAIR_BOB,
    /* 01 */ GE1_HAIR_STRAIGHT,
    /* 02 */ GE1_HAIR_SPIKY
} EnGe1Hairstyle;

static void EnGe1_Init(Actor* thisx, PlayState* play);
static void EnGe1_Destroy(Actor* thisx, PlayState* play);
static void EnGe1_Update(Actor* thisx, PlayState* play);
static void EnGe1_Draw(Actor* thisx, PlayState* play);

static void EnGe1_WaitTillItemGiven_Archery(EnGe1* this, PlayState* play);
static void EnGe1_BeginGiveItem_Archery(EnGe1* this, PlayState* play);

static s32 EnGe1_CheckCarpentersFreed(void);
static void EnGe1_WatchForPlayerFrontOnly(EnGe1* this, PlayState* play);
static void EnGe1_SetNormalText(EnGe1* this, PlayState* play);
static void EnGe1_WatchForAndSensePlayer(EnGe1* this, PlayState* play);
static void EnGe1_GetReaction_ValleyFloor(EnGe1* this, PlayState* play);
static void EnGe1_CheckForCard_GTGGuard(EnGe1* this, PlayState* play);
static void EnGe1_CheckGate_GateOp(EnGe1* this, PlayState* play);
static void EnGe1_GetReaction_GateGuard(EnGe1* this, PlayState* play);
static void EnGe1_TalkAfterGame_Archery(EnGe1* this, PlayState* play);
static void EnGe1_Wait_Archery(EnGe1* this, PlayState* play);
static void EnGe1_CueUpAnimation(EnGe1* this);
static void EnGe1_StopFidget(EnGe1* this);

ActorProfile En_Ge1_Oot_Profile = {
    /**/ ACTOR_EN_GE1_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_GE1_Oot,
    /**/ sizeof(EnGe1),
    /**/ EnGe1_Init,
    /**/ EnGe1_Destroy,
    /**/ EnGe1_Update,
    /**/ EnGe1_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_ENEMY,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0x00000702), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 20, 40, 0, { 0, 0, 0 } },
};

static Gfx* sHairstyleDLists[] = {
    gGerudoWhiteHairstyleBobDL,
    gGerudoWhiteHairstyleStraightFringeDL,
    gGerudoWhiteHairstyleSpikyDL,
};

static Vec3f D_80A327A8 = { 600.0f, 700.0f, 0.0f };

static void* sEyeTextures[] = {
    gGerudoWhiteEyeOpenTex,
    gGerudoWhiteEyeHalfTex,
    gGerudoWhiteEyeClosedTex,
};

static void EnGe1_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnGe1* this = (EnGe1*)thisx;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 30.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gGerudoWhiteSkel, &gGerudoWhiteIdleAnim, this->jointTable,
                       this->morphTable, GE1_LIMB_MAX);
    Animation_PlayOnce(&this->skelAnime, &gGerudoWhiteIdleAnim);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->animation = &gGerudoWhiteIdleAnim;
    this->animFunc = EnGe1_CueUpAnimation;
    this->actor.attentionRangeType = 6;
    Actor_SetScale(&this->actor, 0.01f);

    // In Gerudo Valley
    this->actor.cullingVolumeDistance = ((play->sceneId == SCENE_GERUDO_VALLEY_Oot) ? 1000.0f : 1200.0f);

    switch (this->actor.params & 0xFF) {

        case GE1_TYPE_GATE_GUARD:
            this->hairstyle = GE1_HAIR_SPIKY;
            this->actionFunc = EnGe1_GetReaction_GateGuard;
            break;

        case GE1_TYPE_GATE_OPERATOR:
            this->hairstyle = GE1_HAIR_STRAIGHT;

            if ((EnGe1_CheckCarpentersFreed())) {
                this->actionFunc = EnGe1_CheckGate_GateOp;
            } else {
                this->actionFunc = EnGe1_WatchForPlayerFrontOnly;
            }
            break;

        case GE1_TYPE_NORMAL:
            this->hairstyle = GE1_HAIR_STRAIGHT;

            if ((EnGe1_CheckCarpentersFreed())) {
                this->actionFunc = EnGe1_SetNormalText;
            } else {
                this->actionFunc = EnGe1_WatchForAndSensePlayer;
            }
            break;

        case GE1_TYPE_VALLEY_FLOOR:
            if (LINK_IS_ADULT_OOT) {
                Actor_Kill(&this->actor);
                return;
            }
            this->hairstyle = GE1_HAIR_BOB;
            this->actionFunc = EnGe1_GetReaction_ValleyFloor;
            break;

        case GE1_TYPE_HORSEBACK_ARCHERY:
            if (INV_CONTENT_OOT(SLOT_BOW) == ITEM_NONE) {
                Actor_Kill(&this->actor);
                return;
            }
            this->actor.attentionRangeType = 3;
            this->hairstyle = GE1_HAIR_BOB;

            if (gSaveContext.eventInfOot[0] & 0x100) {
                this->actionFunc = EnGe1_TalkAfterGame_Archery;
            } else if ((EnGe1_CheckCarpentersFreed())) {
                this->actionFunc = EnGe1_Wait_Archery;
            } else {
                this->actionFunc = EnGe1_WatchForPlayerFrontOnly;
            }
            break;

        case GE1_TYPE_TRAINING_GROUND_GUARD:
            this->hairstyle = GE1_HAIR_STRAIGHT;

            if ((EnGe1_CheckCarpentersFreed())) {
                this->actionFunc = EnGe1_CheckForCard_GTGGuard;
            } else {
                this->actionFunc = EnGe1_WatchForPlayerFrontOnly;
            }
            break;
    }

    this->stateFlags = 0;
}

static void EnGe1_Destroy(Actor* thisx, PlayState* play) {
    EnGe1* this = (EnGe1*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 EnGe1_SetTalkAction(EnGe1* this, PlayState* play, u16 textId, f32 arg3, EnGe1ActionFunc actionFunc) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = actionFunc;
        this->animFunc = EnGe1_StopFidget;
        this->stateFlags &= ~GE1_STATE_IDLE_ANIM;
        this->animation = &gGerudoWhiteIdleAnim;
        Animation_Change(&this->skelAnime, &gGerudoWhiteIdleAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gGerudoWhiteIdleAnim), ANIMMODE_ONCE, -8.0f);
        return true;
    }

    this->actor.textId = textId;

    if (this->actor.xzDistToPlayer < arg3) {
        Actor_OfferTalk(&this->actor, play, arg3);
    }

    return false;
}

static void EnGe1_SetAnimationIdle(EnGe1* this) {
    Animation_Change(&this->skelAnime, &gGerudoWhiteIdleAnim, -1.0f, Animation_GetLastFrame(&gGerudoWhiteIdleAnim),
                     0.0f, ANIMMODE_ONCE, 8.0f);
    this->animation = &gGerudoWhiteIdleAnim;
    this->animFunc = EnGe1_CueUpAnimation;
}

static s32 EnGe1_CheckCarpentersFreed(void) {
    u16 carpenterFlags = gSaveContext.save.saveInfo.eventChkInfOot[9];
    if (!((carpenterFlags & 1) && (carpenterFlags & 2) && (carpenterFlags & 4) && (carpenterFlags & 8))) {
        return 0;
    }
    return 1;
}

/**
 * Sends player to different places depending on if has hookshot, and if this is the first time captured
 */
static void EnGe1_KickPlayer(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;

    if (this->cutsceneTimer > 0) {
        this->cutsceneTimer--;
    } else {
        Horse_ResetHorseData(play);

        if ((INV_CONTENT_OOT(ITEM_HOOKSHOT_Oot) == ITEM_NONE) || (INV_CONTENT_OOT(ITEM_LONGSHOT_Oot) == ITEM_NONE)) {
            play->nextEntrance = Hyrule_ExitFromOot(0x1A5); // OoT's ENTR_GERUDO_VALLEY_1
        } else if (Flags_GetEventChkInf(
                       EVENTCHKINF_WATCHED_GANONS_CASTLE_COLLAPSE_CAUGHT_BY_GERUDO_Oot)) { // Caught previously
            play->nextEntrance = Hyrule_ExitFromOot(0x5F8); // OoT's ENTR_GERUDOS_FORTRESS_18
        } else {
            play->nextEntrance = Hyrule_ExitFromOot(0x3B4); // OoT's ENTR_GERUDOS_FORTRESS_17
        }


        play->transitionType = TRANS_TYPE_CIRCLE;
        play->transitionTrigger = TRANS_TRIGGER_START;
    }
}

static void EnGe1_SpotPlayer(EnGe1* this, PlayState* play) {
    this->cutsceneTimer = 30;
    this->actionFunc = EnGe1_KickPlayer;
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 0x5F);
    Audio_PlaySfx(NA_SE_SY_FOUND);
    Message_StartTextbox(play, TEXT_OOT(0x6000), &this->actor);
}

static void EnGe1_WatchForPlayerFrontOnly(EnGe1* this, PlayState* play) {
    s16 angleDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if ((ABS(angleDiff) <= 0x4300) && (this->actor.xzDistToPlayer < 100.0f)) {
        EnGe1_SpotPlayer(this, play);
    }

    if (this->collider.base.acFlags & AC_HIT) {
        EnGe1_SpotPlayer(this, play);
    }

    CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
}

static void EnGe1_ChooseActionFromTextId(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;

    if (Actor_TextboxIsClosing(&this->actor, play)) {
        switch (this->actor.textId) {
            case TEXT_OOT(0x6001):
                this->actionFunc = EnGe1_SetNormalText;
                break;

            case TEXT_OOT(0x601A):
            case TEXT_OOT(0x6019):
                this->actionFunc = EnGe1_GetReaction_ValleyFloor;
                break;

            case TEXT_OOT(0x6018):
                this->actionFunc = EnGe1_CheckGate_GateOp;
                break;

            default:
                this->actionFunc = EnGe1_GetReaction_ValleyFloor;
                break;
        }
    }
}

static void EnGe1_SetNormalText(EnGe1* this, PlayState* play) {
    EnGe1_SetTalkAction(this, play, TEXT_OOT(0x6001), 100.0f, EnGe1_ChooseActionFromTextId);
}

static void EnGe1_WatchForAndSensePlayer(EnGe1* this, PlayState* play) {
    s16 angleDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if ((this->actor.xzDistToPlayer < 50.0f) || ((ABS(angleDiff) <= 0x4300) && (this->actor.xzDistToPlayer < 400.0f))) {
        EnGe1_SpotPlayer(this, play);
    }

    if (this->collider.base.acFlags & AC_HIT) {
        EnGe1_SpotPlayer(this, play);
    }
    CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
}

static void EnGe1_GetReaction_ValleyFloor(EnGe1* this, PlayState* play) {
    u16 reactionText = Text_GetFaceReactionOot(play, 0x22);

    if (reactionText == 0) {
        reactionText = TEXT_OOT(0x6019);
    }

    EnGe1_SetTalkAction(this, play, reactionText, 100.0f, EnGe1_ChooseActionFromTextId);
}

// Gerudo Training Ground Guard functions

static void EnGe1_WaitTillOpened_GTGGuard(EnGe1* this, PlayState* play) {
    if (this->cutsceneTimer > 0) {
        this->cutsceneTimer--;
    } else {
        EnGe1_SetAnimationIdle(this);
        this->actionFunc = EnGe1_SetNormalText;
    }

    this->stateFlags |= GE1_STATE_STOP_FIDGET;
}

static void EnGe1_Open_GTGGuard(EnGe1* this, PlayState* play) {
    if (this->stateFlags & GE1_STATE_IDLE_ANIM) {
        this->actionFunc = EnGe1_WaitTillOpened_GTGGuard;
        Flags_SetSwitch(play, (this->actor.params >> 8) & 0x3F);
        this->cutsceneTimer = 50;
        Message_CloseTextbox(play);
    } else if ((this->skelAnime.curFrame == 15.0f) || (this->skelAnime.curFrame == 19.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_IT_HAND_CLAP);
    }
}

static void EnGe1_SetupOpen_GTGGuard(EnGe1* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        this->actionFunc = EnGe1_Open_GTGGuard;
        Animation_Change(&this->skelAnime, &gGerudoWhiteClapAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gGerudoWhiteClapAnim), ANIMMODE_ONCE, -3.0f);
        this->animation = &gGerudoWhiteClapAnim;
        this->animFunc = EnGe1_StopFidget;
        this->stateFlags &= ~GE1_STATE_IDLE_ANIM;
    }
}

static void EnGe1_RefuseEntryTooPoor_GTGGuard(EnGe1* this, PlayState* play) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
        this->actionFunc = EnGe1_CheckForCard_GTGGuard;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_OfferOpen_GTGGuard(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);

        switch (play->msgCtx.choiceIndex) {
            case 0:
                if (gSaveContext.save.saveInfo.playerData.rupees < 10) {
                    Message_ContinueTextbox(play, TEXT_OOT(0x6016));
                    this->actionFunc = EnGe1_RefuseEntryTooPoor_GTGGuard;
                } else {
                    Rupees_ChangeBy(-10);
                    Message_ContinueTextbox(play, TEXT_OOT(0x6015));
                    this->actionFunc = EnGe1_SetupOpen_GTGGuard;
                }
                break;
            case 1:
                this->actionFunc = EnGe1_CheckForCard_GTGGuard;
                EnGe1_SetAnimationIdle(this);
                break;
        }
    }
}

static void EnGe1_RefuseOpenNoCard_GTGGuard(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = EnGe1_CheckForCard_GTGGuard;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_CheckForCard_GTGGuard(EnGe1* this, PlayState* play) {
    if (CHECK_QUEST_ITEM_OOT(QUEST_GERUDO_CARD_Oot)) {
        EnGe1_SetTalkAction(this, play, TEXT_OOT(0x6014), 100.0f, EnGe1_OfferOpen_GTGGuard);
    } else {
        //! @bug This outcome is inaccessible in normal gameplay since this function it is unreachable without
        //! obtaining the card in the first place.
        EnGe1_SetTalkAction(this, play, TEXT_OOT(0x6013), 100.0f, EnGe1_RefuseOpenNoCard_GTGGuard);
    }
}

// Gate Operator functions

static void EnGe1_WaitGateOpen_GateOp(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        this->actionFunc = EnGe1_CheckGate_GateOp;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_WaitUntilGateOpened_GateOp(EnGe1* this, PlayState* play) {
    if (this->cutsceneTimer > 0) {
        this->cutsceneTimer--;
    } else {
        EnGe1_SetAnimationIdle(this);
        this->actionFunc = EnGe1_CheckGate_GateOp;
    }
    this->stateFlags |= GE1_STATE_STOP_FIDGET;
}

static void EnGe1_OpenGate_GateOp(EnGe1* this, PlayState* play) {
    if (this->stateFlags & GE1_STATE_IDLE_ANIM) {
        this->actionFunc = EnGe1_WaitUntilGateOpened_GateOp;
        Flags_SetSwitch(play, (this->actor.params >> 8) & 0x3F);
        this->cutsceneTimer = 50;
        Message_CloseTextbox(play);
    } else if ((this->skelAnime.curFrame == 15.0f) || (this->skelAnime.curFrame == 19.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_IT_HAND_CLAP);
    }
}

static void EnGe1_SetupOpenGate_GateOp(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        this->actionFunc = EnGe1_OpenGate_GateOp;
        Animation_Change(&this->skelAnime, &gGerudoWhiteClapAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gGerudoWhiteClapAnim), ANIMMODE_ONCE, -3.0f);
        this->animation = &gGerudoWhiteClapAnim;
        this->animFunc = EnGe1_StopFidget;
        this->stateFlags &= ~GE1_STATE_IDLE_ANIM;
    }
}

static void EnGe1_CheckGate_GateOp(EnGe1* this, PlayState* play) {
    if (Flags_GetSwitch(play, (this->actor.params >> 8) & 0x3F)) {
        EnGe1_SetTalkAction(this, play, TEXT_OOT(0x6018), 100.0f, EnGe1_WaitGateOpen_GateOp);
    } else {
        EnGe1_SetTalkAction(this, play, TEXT_OOT(0x6017), 100.0f, EnGe1_SetupOpenGate_GateOp);
    }
}

// Gate guard functions

static void EnGe1_Talk_GateGuard(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;

    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = EnGe1_GetReaction_GateGuard;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_GetReaction_GateGuard(EnGe1* this, PlayState* play) {
    u16 reactionText;

    reactionText = Text_GetFaceReactionOot(play, 0x22);

    if (reactionText == 0) {
        reactionText = TEXT_OOT(0x6069);
    }

    if (EnGe1_SetTalkAction(this, play, reactionText, 100.0f, EnGe1_Talk_GateGuard)) {
        this->animFunc = EnGe1_CueUpAnimation;
        this->animation = &gGerudoWhiteDismissiveAnim;
        Animation_Change(&this->skelAnime, &gGerudoWhiteDismissiveAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gGerudoWhiteDismissiveAnim), ANIMMODE_ONCE, -8.0f);
    }
}

// Archery functions

static void EnGe1_SetupWait_Archery(EnGe1* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = EnGe1_Wait_Archery;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_WaitTillItemGiven_Archery(EnGe1* this, PlayState* play) {
    s32 getItemId;

    if (!(true)) {
        return;
    }
    if (Actor_HasParent(&this->actor, play)) {
        if ((true)) {
            this->actionFunc = EnGe1_SetupWait_Archery;
        }

        if (this->stateFlags & GE1_STATE_GIVE_QUIVER) {
            SET_ITEMGETINF_OOT(ITEMGETINF_0F_Oot);
        } else {
            SET_INFTABLE_OOT(INFTABLE_190_Oot);
        }
    } else {
        if (this->stateFlags & GE1_STATE_GIVE_QUIVER) {
            switch (CUR_UPG_VALUE(UPG_QUIVER)) {
                //! @bug Asschest. See next function for details
                case 1:
                    getItemId = GI_QUIVER_40_Oot;
                    break;
                case 2:
                    getItemId = GI_QUIVER_50_Oot;
                    break;
            }
        } else {
            getItemId = GI_HEART_PIECE_Oot;
        }
        Actor_OfferGetItem(&this->actor, play, getItemId, 10000.0f, 50.0f);
    }
}

static void EnGe1_BeginGiveItem_Archery(EnGe1* this, PlayState* play) {
    s32 getItemId;

    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->actionFunc = EnGe1_WaitTillItemGiven_Archery;
    }

    if (this->stateFlags & GE1_STATE_GIVE_QUIVER) {
        switch (CUR_UPG_VALUE(UPG_QUIVER)) {
            //! @bug Asschest: the compiler inserts a default assigning *(sp+0x24) to getItemId, which is junk data left
            //! over from the previous function run in EnGe1_Update, namely EnGe1_CueUpAnimation. The top stack variable
            //! in that function is &this->skelAnime = thisx + 198, and depending on where this loads in memory, the
            //! getItemId changes.
            case 1:
                getItemId = GI_QUIVER_40_Oot;
                break;
            case 2:
                getItemId = GI_QUIVER_50_Oot;
                break;
        }
    } else {
        getItemId = GI_HEART_PIECE_Oot;
    }

    if ((true)) {
        Actor_OfferGetItem(&this->actor, play, getItemId, 10000.0f, 50.0f);
    }
}

static void EnGe1_TalkWinPrize_Archery(EnGe1* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnGe1_BeginGiveItem_Archery;
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
    } else {
        Actor_OfferTalk(&this->actor, play, 200.0f);
    }
}

static void EnGe1_TalkTooPoor_Archery(EnGe1* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        this->actionFunc = EnGe1_Wait_Archery;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_WaitDoNothing(EnGe1* this, PlayState* play) {
}

static void EnGe1_BeginGame_Archery(EnGe1* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Actor* horse;

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;

        switch (play->msgCtx.choiceIndex) {
            case 0:
                if (gSaveContext.save.saveInfo.playerData.rupees < 20) {
                    Message_ContinueTextbox(play, 0x85);
                    this->actionFunc = EnGe1_TalkTooPoor_Archery;
                } else if ((true)) {
                    Rupees_ChangeBy(-20);
                    play->nextEntrance = Hyrule_ExitFromOot(0x129); // OoT's ENTR_GERUDOS_FORTRESS_EAST_EXIT
                    gSaveContext.nextCutsceneIndex = 0xFFF0;
                    play->transitionType = TRANS_TYPE_CIRCLE;
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    gSaveContext.eventInfOot[0] |= 0x100;
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_HORSEBACK_ARCHERY_Oot);

                    if (!(player->stateFlags1 & PLAYER_STATE1_800000)) {
                        Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
                    } else {
                        horse = Actor_FindNearby(play, &player->actor, ACTOR_EN_HORSE_Oot, ACTORCAT_BG, 1200.0f);
                        player->actor.freezeTimer = 1200;

                        if (horse != NULL) {
                            horse->freezeTimer = 1200;
                        }
                    }

                    this->actionFunc = EnGe1_WaitDoNothing;
                }
                break;

            case 1:
                this->actionFunc = EnGe1_Wait_Archery;
                Message_CloseTextbox(play);
                break;
        }
    }
}

static void EnGe1_TalkOfferPlay_Archery(EnGe1* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_ContinueTextbox(play, TEXT_OOT(0x6041));
        this->actionFunc = EnGe1_BeginGame_Archery;
    }
}

static void EnGe1_TalkNoPrize_Archery(EnGe1* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnGe1_TalkOfferPlay_Archery;
    } else {
        Actor_OfferTalk(&this->actor, play, 300.0f);
    }
}

static void EnGe1_TalkAfterGame_Archery(EnGe1* this, PlayState* play) {
    gSaveContext.eventInfOot[0] &= ~0x100;
    this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;

    if (HIGH_SCORE_OOT(HS_HBA_Oot) < gSaveContext.minigameScore) {
        HIGH_SCORE_OOT(HS_HBA_Oot) = gSaveContext.minigameScore;
    }

    if (gSaveContext.minigameScore < 1000) {
        this->actor.textId = TEXT_OOT(0x6045);
        this->actionFunc = EnGe1_TalkNoPrize_Archery;
    } else if (!GET_INFTABLE_OOT(INFTABLE_190_Oot)) {
        this->actor.textId = TEXT_OOT(0x6046);
        this->actionFunc = EnGe1_TalkWinPrize_Archery;
        this->stateFlags &= ~GE1_STATE_GIVE_QUIVER;
    } else if (gSaveContext.minigameScore < 1500) {
        this->actor.textId = TEXT_OOT(0x6047);
        this->actionFunc = EnGe1_TalkNoPrize_Archery;
    } else if (GET_ITEMGETINF_OOT(ITEMGETINF_0F_Oot)) {
        this->actor.textId = TEXT_OOT(0x6047);
        this->actionFunc = EnGe1_TalkNoPrize_Archery;
    } else {
        this->actor.textId = TEXT_OOT(0x6044);
        this->actionFunc = EnGe1_TalkWinPrize_Archery;
        this->stateFlags |= GE1_STATE_GIVE_QUIVER;
    }
}

static void EnGe1_TalkNoHorse_Archery(EnGe1* this, PlayState* play) {
    this->stateFlags |= GE1_STATE_TALKING;
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = EnGe1_Wait_Archery;
        EnGe1_SetAnimationIdle(this);
    }
}

static void EnGe1_Wait_Archery(EnGe1* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    u16 textId;

    if (!(player->stateFlags1 & PLAYER_STATE1_800000)) {
        EnGe1_SetTalkAction(this, play, TEXT_OOT(0x603F), 100.0f, EnGe1_TalkNoHorse_Archery);
    } else {
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_HORSEBACK_ARCHERY_Oot)) {
            if (GET_INFTABLE_OOT(INFTABLE_190_Oot)) {
                textId = TEXT_OOT(0x6042);
            } else {
                textId = TEXT_OOT(0x6043);
            }
        } else {
            textId = TEXT_OOT(0x6040);
        }
        EnGe1_SetTalkAction(this, play, textId, 200.0f, EnGe1_TalkOfferPlay_Archery);
    }
}

// General functions

static void EnGe1_TurnToFacePlayer(EnGe1* this, PlayState* play) {
    s32 pad;
    s16 angleDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if (ABS(angleDiff) <= 0x4000) {
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 6, 4000, 100);
        this->actor.world.rot.y = this->actor.shape.rot.y;
        Actor_TrackPlayer(play, &this->actor, &this->headRot, &this->unk_2A2, this->actor.focus.pos);
    } else {
        if (angleDiff < 0) {
            Math_SmoothStepToS(&this->headRot.y, -0x2000, 6, 6200, 0x100);
        } else {
            Math_SmoothStepToS(&this->headRot.y, 0x2000, 6, 6200, 0x100);
        }

        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 12, 1000, 100);
        this->actor.world.rot.y = this->actor.shape.rot.y;
    }
}

static void EnGe1_LookAtPlayer(EnGe1* this, PlayState* play) {
    s16 angleDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if ((ABS(angleDiff) <= 0x4300) && (this->actor.xzDistToPlayer < 100.0f)) {
        Actor_TrackPlayer(play, &this->actor, &this->headRot, &this->unk_2A2, this->actor.focus.pos);
    } else {
        Math_SmoothStepToS(&this->headRot.x, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->headRot.y, 0, 6, 6200, 100);
    }
}

static void EnGe1_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnGe1* this = (EnGe1*)thisx;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 40.0f, 25.0f, 40.0f, 0x405);
    this->animFunc(this);
    this->actionFunc(this, play);

    if (this->stateFlags & GE1_STATE_TALKING) {
        EnGe1_TurnToFacePlayer(this, play);
        this->stateFlags &= ~GE1_STATE_TALKING;
    } else {
        EnGe1_LookAtPlayer(this, play);
    }
    this->unk_2A2.x = this->unk_2A2.y = this->unk_2A2.z = 0;

    if (DECR(this->blinkTimer) == 0) {
        this->blinkTimer = Rand_S16Offset(60, 60);
    }
    this->eyeIndex = this->blinkTimer;

    if (this->eyeIndex >= 3) {
        this->eyeIndex = 0;
    }
}

// Animation functions

static void EnGe1_CueUpAnimation(EnGe1* this) {
    if (SkelAnime_Update(&this->skelAnime)) {
        Animation_PlayOnce(&this->skelAnime, this->animation);
    }
}

static void EnGe1_StopFidget(EnGe1* this) {
    if (!(this->stateFlags & GE1_STATE_IDLE_ANIM)) {
        if (SkelAnime_Update(&this->skelAnime)) {
            this->stateFlags |= GE1_STATE_IDLE_ANIM;
        }
        this->stateFlags |= GE1_STATE_STOP_FIDGET;
    }
}

static s32 EnGe1_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    s32 pad;
    EnGe1* this = (EnGe1*)thisx;

    if (limbIndex == GE1_LIMB_HEAD) {
        rot->x += this->headRot.y;
        rot->z += this->headRot.x;
    }

    if (this->stateFlags & GE1_STATE_STOP_FIDGET) {
        this->stateFlags &= ~GE1_STATE_STOP_FIDGET;
        return 0;
    }

    // The purpose of the state flag GE1_STATE_STOP_FIDGET is to skip this code, which this actor has in lieu of an idle
    // animation.
    if ((limbIndex == GE1_LIMB_TORSO) || (limbIndex == GE1_LIMB_L_FOREARM) || (limbIndex == GE1_LIMB_R_FOREARM)) {
        rot->y += Math_SinS(play->state.frames * (limbIndex * 50 + 0x814)) * 200.0f;
        rot->z += Math_CosS(play->state.frames * (limbIndex * 50 + 0x940)) * 200.0f;
    }
    return 0;
}

static void EnGe1_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnGe1* this = (EnGe1*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == GE1_LIMB_HEAD) {
        gSPDisplayList(POLY_OPA_DISP++, sHairstyleDLists[this->hairstyle]);
        Matrix_MultVec3f(&D_80A327A8, &this->actor.focus.pos);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnGe1_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    EnGe1* this = (EnGe1*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL37_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIndex]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnGe1_OverrideLimbDraw, EnGe1_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
