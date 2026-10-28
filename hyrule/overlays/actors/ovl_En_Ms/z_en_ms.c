/*
 * File: z_en_ms.c
 * Overlay: ovl_En_Ms
 * Description: Magic Bean Salesman
 */

#include "z_en_ms.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "assets/oot/objects/object_ms/object_ms.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnMs_Init(Actor* thisx, PlayState* play);
static void EnMs_Destroy(Actor* thisx, PlayState* play);
static void EnMs_Update(Actor* thisx, PlayState* play);
static void EnMs_Draw(Actor* thisx, PlayState* play);

static void EnMs_SetOfferText(EnMs* this, PlayState* play);
static void EnMs_Wait(EnMs* this, PlayState* play);
static void EnMs_Talk(EnMs* this, PlayState* play);
static void EnMs_Sell(EnMs* this, PlayState* play);
static void EnMs_TalkAfterPurchase(EnMs* this, PlayState* play);

ActorProfile En_Ms_Oot_Profile = {
    /**/ ACTOR_EN_MS_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_MS_Oot,
    /**/ sizeof(EnMs),
    /**/ EnMs_Init,
    /**/ EnMs_Destroy,
    /**/ EnMs_Update,
    /**/ EnMs_Draw,
};

static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        COLSHAPE_CYLINDER,
    },
    { 0x00, { 0x00000000, 0x00, 0x00 }, { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 }, 0x00, 0x01, 0x01 },
    { 22, 37, 0, { 0 } },
};

static s16 sPrices[] = {
    10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
};

static u16 sOfferTextIDs[] = {
    0x405E, 0x405F, 0x4060, 0x4061, 0x4062, 0x4063, 0x4064, 0x4065, 0x4066, 0x4067,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 2, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 500, ICHAIN_STOP),
};

static void EnMs_SetOfferText(EnMs* this, PlayState* play) {
    this->actor.textId = Text_GetFaceReactionOot(play, 0x1B);
    if (this->actor.textId == 0) {
        if (BEANS_BOUGHT_OOT >= 10) {
            this->actor.textId = TEXT_OOT(0x406B);
        } else {
            this->actor.textId = TEXT_OOT(sOfferTextIDs[BEANS_BOUGHT_OOT]);
        }
    }
}

static void EnMs_Init(Actor* thisx, PlayState* play) {
    EnMs* this = (EnMs*)thisx;
    s32 pad;

    if (LINK_AGE_IN_YEARS_OOT != YEARS_CHILD) {
        Actor_Kill(&this->actor);
        return;
    }
    Actor_ProcessInitChain(&this->actor, sInitChain);
    SkelAnime_InitFlex(play, &this->skelAnime, &gBeanSalesmanSkel, &gBeanSalesmanEatingAnim, this->jointTable,
                       this->morphTable, 9);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 35.0f);
    Actor_SetScale(&this->actor, 0.015f);

    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->actor.speed = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = -1.0f;

    EnMs_SetOfferText(this, play);

    this->actionFunc = EnMs_Wait;
}

static void EnMs_Destroy(Actor* thisx, PlayState* play) {
    EnMs* this = (EnMs*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnMs_Wait(EnMs* this, PlayState* play) {
    s16 yawDiff;

    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    EnMs_SetOfferText(this, play);

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) { // if talk is initiated
        this->actionFunc = EnMs_Talk;
    } else if ((this->actor.xzDistToPlayer < 90.0f) && (ABS(yawDiff) < 0x2000)) { // talk range
        Actor_OfferTalk(&this->actor, play, 90.0f);
    }
}

static void EnMs_Talk(EnMs* this, PlayState* play) {
    u8 dialogState;

    dialogState = Message_GetState(&play->msgCtx);
    if (dialogState != TEXT_STATE_CHOICE) {
        if ((dialogState == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) { // advanced final textbox
            this->actionFunc = EnMs_Wait;
        }
    } else if (Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0: // yes
                if (!((gSaveContext.save.saveInfo.playerData.rupees >= sPrices[BEANS_BOUGHT_OOT]))) {
                    Message_ContinueTextbox(play, TEXT_OOT(0x4069)); // not enough rupees text
                    return;
                }

                if ((true)) {
                    Actor_OfferGetItem(&this->actor, play, GI_BEAN_Oot, 90.0f, 10.0f);
                    this->actionFunc = EnMs_Sell;
                }
                return;
            case 1: // no
                Message_ContinueTextbox(play, TEXT_OOT(0x4068));
            default:
                return;
        }
    }
}

static void EnMs_Sell(EnMs* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play)) {
        if ((true)) {
            Rupees_ChangeBy(-sPrices[BEANS_BOUGHT_OOT]);
        }
        this->actor.parent = NULL;
        this->actionFunc = EnMs_TalkAfterPurchase;
    } else {
        Actor_OfferGetItem(&this->actor, play, GI_BEAN_Oot, 90.0f, 10.0f);
    }
}

static void EnMs_TalkAfterPurchase(EnMs* this, PlayState* play) {
    // if dialog state is 6 and player responded to textbox
    if ((Message_GetState(&play->msgCtx)) == TEXT_STATE_DONE && Message_ShouldAdvance(play)) {
        Message_ContinueTextbox(play, TEXT_OOT(0x406C));
        this->actionFunc = EnMs_Talk;
    }
}

static void EnMs_Update(Actor* thisx, PlayState* play) {
    EnMs* this = (EnMs*)thisx;
    s32 pad;

    this->activeTimer += 1;
    Actor_SetFocus(&this->actor, 20.0f);
    this->actor.lockOnArrowOffset = 500.0f;
    Actor_SetScale(&this->actor, 0.015f);
    SkelAnime_Update(&this->skelAnime);
    this->actionFunc(this, play);

    if (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x157 /* ENTR_LON_LON_RANCH_ENTRANCE */) &&
        gSaveContext.sceneLayer == 8) { // ride carpet if in credits
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    }
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static void EnMs_Draw(Actor* thisx, PlayState* play) {
    EnMs* this = (EnMs*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, this);
}
