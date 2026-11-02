/*
 * File: z_en_wonder_talk.c
 * Overlay: ovl_En_Wonder_Talk
 * Description: Checkable spot (Green Navi)
 */

#include "z_en_wonder_talk.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_LOCK_ON_DISABLED)

static void EnWonderTalk_Init(Actor* thisx, PlayState* play);
static void EnWonderTalk_Destroy(Actor* thisx, PlayState* play);
static void EnWonderTalk_Update(Actor* thisx, PlayState* play);

static void func_80B391CC(EnWonderTalk* this, PlayState* play);
static void func_80B395F0(EnWonderTalk* this, PlayState* play);
static void func_80B3943C(EnWonderTalk* this, PlayState* play);

ActorProfile En_Wonder_Talk_Oot_Profile = {
    /**/ ACTOR_EN_WONDER_TALK_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnWonderTalk),
    /**/ EnWonderTalk_Init,
    /**/ EnWonderTalk_Destroy,
    /**/ EnWonderTalk_Update,
    /**/ NULL,
};

static void EnWonderTalk_Destroy(Actor* thisx, PlayState* play) {
}

static void EnWonderTalk_Init(Actor* thisx, PlayState* play) {
    EnWonderTalk* this = (EnWonderTalk*)thisx;


    this->unk_150 = (this->actor.params >> 0xB) & 0x1F;
    this->unk_152 = (this->actor.params >> 6) & 0x1F;
    this->switchFlag = this->actor.params & 0x3F;
    if (this->switchFlag == 0x3F) {
        this->switchFlag = -1;
    }
    this->actor.attentionRangeType = 1;
    if (this->switchFlag >= 0) {
        if (Flags_GetSwitch(play, this->switchFlag)) {
            Actor_Kill(&this->actor);
            return;
        }
    }
    this->actionFunc = func_80B391CC;
    this->unk_15C = 40.0f;
}

static void func_80B391CC(EnWonderTalk* this, PlayState* play) {
    if (this->switchFlag < 0 || !Flags_GetSwitch(play, this->switchFlag)) {
        switch (this->unk_150) {
            case 1:
                this->height = 0.0f;
                this->unk_15C = 80.0f;
                if (!LINK_IS_ADULT_OOT) {
                    this->actor.textId = TEXT_OOT(0x7040);
                } else {
                    this->actor.textId = TEXT_OOT(0x7088);
                }

                this->unk_156 = TEXT_STATE_EVENT;
                break;
            case 2:
                this->actor.textId = TEXT_OOT(0x5002);
                this->unk_156 = TEXT_STATE_CHOICE;
                this->height = 30.0f;
                this->unk_15C = 40.0f;
                break;
            case 3:
                this->actor.textId = TEXT_OOT(0x501E);
                this->unk_156 = TEXT_STATE_EVENT;
                this->height = 0.0f;
                this->unk_15C = 110.0f;
                break;
            case 4:
                this->actor.textId = TEXT_OOT(0x5020);
                this->unk_156 = TEXT_STATE_DONE;
                this->height = 0.0f;
                this->unk_15C = 120.0f;
                if (GET_EVENTCHKINF_OOT(EVENTCHKINF_DESTROYED_ROYAL_FAMILY_TOMB_Oot)) {
                    Actor_Kill(&this->actor);
                }
                break;
            case 5:
                this->actor.textId = TEXT_OOT(0x501F);
                this->unk_156 = TEXT_STATE_EVENT;
                this->height = 0.0f;
                this->unk_15C = 110.0f;
                break;
            default:
                this->actor.textId = TEXT_OOT(0x7072);
                this->unk_156 = TEXT_STATE_EVENT;
                break;
        }

        this->actionFunc = func_80B3943C;
    }
}

static void func_80B3943C(EnWonderTalk* this, PlayState* play) {
    s16 yawDiff;
    s16 yawDiffTemp;

    this->unk_15A++;
    if (this->unk_150 == 4 && (GET_EVENTCHKINF_OOT(EVENTCHKINF_DESTROYED_ROYAL_FAMILY_TOMB_Oot))) {
        Actor_Kill(&this->actor);
        return;
    }
    if (this->switchFlag < 0 || !Flags_GetSwitch(play, this->switchFlag)) {
        if ((Actor_TalkOfferAccepted(&this->actor, &play->state))) {
            if (this->unk_156 != TEXT_STATE_DONE) {
                this->actionFunc = func_80B395F0;
            } else {
                if (this->switchFlag >= 0) {
                    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
                    Flags_SetSwitch(play, this->switchFlag);
                }
                this->actionFunc = func_80B391CC;
            }
        } else if (!(this->unk_15C < this->actor.xzDistToPlayer)) {
            yawDiffTemp = (this->actor.yawTowardsPlayer - this->actor.world.rot.y);
            yawDiff = ABS(yawDiffTemp);

            if (yawDiff < 0x4000) {
                if (this->unk_15A >= 2) {
                }
                this->unk_15A = 0;
                Actor_OfferTalk(&this->actor, play, this->unk_15C);
            }
        }
    }
}

static void func_80B395F0(EnWonderTalk* this, PlayState* play) {
    if (this->unk_156 == Message_GetState(&play->msgCtx) && Message_ShouldAdvance(play)) {
        if (this->switchFlag >= 0) {
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            Flags_SetSwitch(play, this->switchFlag);
        }
        switch (this->unk_150) {
            case 1:
                Message_CloseTextbox(play);
                this->actionFunc = func_80B391CC;
                break;
            case 2:
                switch (play->msgCtx.choiceIndex) {
                    case 0:
                        if (!LINK_IS_ADULT_OOT) {
                            this->actor.textId = TEXT_OOT(0x5001);
                        } else {
                            this->actor.textId = TEXT_OOT(0x5003);
                        }
                        break;
                    case 1:
                        this->actor.textId = TEXT_OOT(0x5004);
                        break;
                }

                this->unk_156 = TEXT_STATE_DONE;
                Message_ContinueTextbox(play, this->actor.textId);
                this->actionFunc = func_80B391CC;
                break;
            case 3:
                Message_CloseTextbox(play);
                if (this->unk_164 == 0) {
                    Actor_Spawn(&play->actorCtx, play, ACTOR_EN_POH_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                                this->actor.world.pos.z, 0, 0, 0, 2);
                    this->unk_164 = 1;
                }

                this->actionFunc = func_80B391CC;
                break;
            case 5:
                Message_CloseTextbox(play);
                if (this->unk_164 == 0) {
                    Actor_Spawn(&play->actorCtx, play, ACTOR_EN_POH_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                                this->actor.world.pos.z, 0, 0, 0, 3);
                    this->unk_164 = 1;
                }
                this->actionFunc = func_80B391CC;
                break;
        }
    }
}

static void EnWonderTalk_Update(Actor* thisx, PlayState* play) {
    EnWonderTalk* this = (EnWonderTalk*)thisx;

    if (this->unk_158 != 0) {
        this->unk_158--;
    }
    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, this->height);

    if (BREG(0) != 0) {
        if (this->unk_15A != 0) {
            if ((this->unk_15A & 1) == 0) {
                DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                       this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, 1.0f,
                                       1.0f, 1.0f, 10, 10, 10, 255, 4, play->state.gfxCtx);
            }
        } else {
            DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                   this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, 1.0f,
                                   1.0f, 1.0f, 0, 255, 0, 255, 4, play->state.gfxCtx);
        }
    }
}
