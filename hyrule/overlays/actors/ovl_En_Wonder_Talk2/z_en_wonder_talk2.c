/*
 * File: z_en_wonder_talk2.c
 * Overlay: ovl_En_Wonder_Talk2
 * Description: Dialog spot
 *
 * 3Ship [Hyrule] Ported from SoH. See hyrule/README.md for the porting conventions.
 */

#include "z_en_wonder_talk2.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_LOCK_ON_DISABLED)

static void EnWonderTalk2_Init(Actor* thisx, PlayState* play);
static void EnWonderTalk2_Destroy(Actor* thisx, PlayState* play);
static void EnWonderTalk2_Update(Actor* thisx, PlayState* play);

static void func_80B3A10C(EnWonderTalk2* this, PlayState* play);
static void func_80B3A4F8(EnWonderTalk2* this, PlayState* play);
static void func_80B3A15C(EnWonderTalk2* this, PlayState* play);
static void func_80B3A3D4(EnWonderTalk2* this, PlayState* play);
static void EnWonderTalk2_DoNothing(EnWonderTalk2* this, PlayState* play);

ActorProfile En_Wonder_Talk2_Oot_Profile = {
    /**/ ACTOR_EN_WONDER_TALK2_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnWonderTalk2),
    /**/ EnWonderTalk2_Init,
    /**/ EnWonderTalk2_Destroy,
    /**/ EnWonderTalk2_Update,
    /**/ NULL,
};

static s16 D_80B3A8E0[] = { 6, 0, 1, 2, 3, 4, 5 };

static void EnWonderTalk2_Destroy(Actor* thisx, PlayState* play) {
}

static void EnWonderTalk2_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnWonderTalk2* this = (EnWonderTalk2*)thisx;

    this->baseMsgId = (this->actor.params >> 6) & 0xFF;
    if (this->actor.world.rot.z > 0) {
        s32 rangeIndex = 0;
        s16 rotZmod10 = this->actor.world.rot.z;

        while (rotZmod10 > 10) {
            rotZmod10 -= 10;
            rangeIndex++;
        }
        // rangeIndex = rot.z/10 here
        this->triggerRange = rotZmod10 * 40.0f;
        if (rangeIndex > 6) {
            rangeIndex = 0;
        }

        this->actor.attentionRangeType = D_80B3A8E0[rangeIndex];
    }

    this->initPos = this->actor.world.pos;
    this->switchFlag = (this->actor.params & 0x3F);
    this->talkMode = ((this->actor.params >> 0xE) & 3);

    if (this->switchFlag == 0x3F) {
        this->switchFlag = -1;
    }
    if (this->switchFlag >= 0 && Flags_GetSwitch(play, this->switchFlag)) {
        Actor_Kill(&this->actor);
        return;
    }
    // PORT: in the Gerudo Training Ground, forced messages other than switch flags 0x08, 0x16 and 0x2F only
    // check (talkMode 4). That scene is not in this fork yet.
    if (this->talkMode == 3) {
        this->actor.flags &= ~ACTOR_FLAG_LOCK_ON_DISABLED;
        this->actionFunc = EnWonderTalk2_DoNothing;
    } else {
        this->actionFunc = func_80B3A10C;
    }
}

static void func_80B3A10C(EnWonderTalk2* this, PlayState* play) {
    this->actor.textId = TEXT_OOT(0x200 | this->baseMsgId);
    if (this->talkMode == 1 || this->talkMode == 4) {
        this->actionFunc = func_80B3A4F8;
    } else {
        this->actionFunc = func_80B3A15C;
    }
}

static void func_80B3A15C(EnWonderTalk2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->unk_158++;
    if ((this->switchFlag >= 0) && Flags_GetSwitch(play, this->switchFlag)) {
        if (!this->unk_15A) {
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            this->unk_15A = true;
        }
    } else if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if ((this->switchFlag >= 0) && (this->talkMode != 2)) {
            Flags_SetSwitch(play, this->switchFlag);
        }

        this->actionFunc = func_80B3A10C;
    } else {
        s16 yawDiff = ABS((s16)(this->actor.yawTowardsPlayer - this->actor.world.rot.y));

        if (!((this->actor.xzDistToPlayer > 40.0f + this->triggerRange) ||
              (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) > 100.0f) || (yawDiff >= 0x4000))) {
            this->unk_158 = 0;
            Actor_OfferTalkExchange(&this->actor, play, this->triggerRange + 50.0f, 100.0f, PLAYER_IA_NONE);
        }
    }
}

static void func_80B3A3D4(EnWonderTalk2* this, PlayState* play) {
    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_EVENT:
        case TEXT_STATE_DONE:
            if (Message_ShouldAdvance(play)) {
                if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) {
                    Message_CloseTextbox(play);
                }
            } else {
                break;
            }
        case TEXT_STATE_NONE:
            if ((this->switchFlag >= 0) && (this->talkMode != 4)) {
                Flags_SetSwitch(play, this->switchFlag);
            }

            if (this->talkMode == 4) {
                this->unk_15A = true;
            }
            this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_UPDATE_CULLING_DISABLED);
            // OoT's Player cutscene action 7 is MM's PLAYER_CSACTION_END.
            Player_SetCsActionWithHaltedActors(play, NULL, PLAYER_CSACTION_END);
            this->unk_156 = true;
            this->actionFunc = func_80B3A4F8;
            break;
    }
}

static void func_80B3A4F8(EnWonderTalk2* this, PlayState* play) {
    Player* player;

    player = GET_PLAYER(play);
    this->unk_158++;
    if (this->switchFlag >= 0 && Flags_GetSwitch(play, this->switchFlag)) {
        if (!this->unk_15A) {
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            this->unk_15A = true;
        }
    } else if ((this->talkMode != 4) || !this->unk_15A) {
        if (((this->actor.xzDistToPlayer < (40.0f + this->triggerRange)) &&
             (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) < 100.0f)) &&
            !Play_InCsMode(play)) {
            this->unk_158 = 0;
            if (!this->unk_156) {
                Message_StartTextbox(play, this->actor.textId, NULL);
                // OoT's Player cutscene action 8 is MM's PLAYER_CSACTION_WAIT.
                Player_SetCsActionWithHaltedActors(play, NULL, PLAYER_CSACTION_WAIT);
                this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_UPDATE_CULLING_DISABLED;
                this->actionFunc = func_80B3A3D4;
            }

        } else {
            this->unk_156 = false;
        }
    }
}

static void EnWonderTalk2_DoNothing(EnWonderTalk2* this, PlayState* play) {
}

static void EnWonderTalk2_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnWonderTalk2* this = (EnWonderTalk2*)thisx;

    this->actionFunc(this, play);
    this->actor.world.pos.y = this->initPos.y;

    Actor_SetFocus(&this->actor, this->height);
}
