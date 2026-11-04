/*
 * File: z_en_okarina_tag.c
 * Overlay: ovl_En_Okarina_Tag
 * Description: Music Staff (Ocarina) spot
 */

#include "z_en_okarina_tag.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "HyruleCutscene.h"
#include "assets/oot/scenes/misc/hakaana_ouke/hakaana_ouke_scene.h"
#include "assets/oot/scenes/overworld/spot02/spot02_scene.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

static void EnOkarinaTag_Init(Actor* thisx, PlayState* play);
static void EnOkarinaTag_Destroy(Actor* thisx, PlayState* play);
static void EnOkarinaTag_Update(Actor* thisx, PlayState* play);

static void func_80ABEF2C(EnOkarinaTag* this, PlayState* play);
static void func_80ABF708(EnOkarinaTag* this, PlayState* play);
static void func_80ABF28C(EnOkarinaTag* this, PlayState* play);
static void func_80ABF0CC(EnOkarinaTag* this, PlayState* play);
static void func_80ABF4C8(EnOkarinaTag* this, PlayState* play);
static void func_80ABF7CC(EnOkarinaTag* this, PlayState* play);

ActorProfile En_Okarina_Tag_Oot_Profile = {
    /**/ ACTOR_EN_OKARINA_TAG_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnOkarinaTag),
    /**/ EnOkarinaTag_Init,
    /**/ EnOkarinaTag_Destroy,
    /**/ EnOkarinaTag_Update,
    /**/ NULL,
};

extern CutsceneData D_80ABF9D0[];
extern CutsceneData D_80ABFB40[];

static void EnOkarinaTag_Destroy(Actor* thisx, PlayState* play) {
}

static void EnOkarinaTag_Init(Actor* thisx, PlayState* play) {
    EnOkarinaTag* this = (EnOkarinaTag*)thisx;

    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->type = (this->actor.params >> 0xA) & 0x3F;
    this->ocarinaSong = (this->actor.params >> 6) & 0xF;
    this->switchFlag = this->actor.params & 0x3F;
    if (this->switchFlag == 0x3F) {
        this->switchFlag = -1;
    }
    if (this->ocarinaSong == 0xF) {
        this->ocarinaSong = 0;
        this->unk_158 = 1;
    }
    this->actor.attentionRangeType = 1;
    if (this->actor.world.rot.z > 0) {
        this->interactRange = this->actor.world.rot.z * 40.0f;
    }


    if (((this->switchFlag >= 0) && (Flags_GetSwitch(play, this->switchFlag)))) {
        Actor_Kill(&this->actor);
    } else {
        switch (this->type) {
            case 7:
                this->actionFunc = func_80ABEF2C;
                break;
            case 2:
                if (LINK_IS_ADULT_OOT) {
                    Actor_Kill(&this->actor);
                    break;
                }
            case 1:
            case 4:
            case 6:
                this->actionFunc = func_80ABF28C;
                break;
            case 5:
                this->actor.textId = TEXT_OOT(0x5021);
                this->actionFunc = func_80ABF708;
                break;
            default:
                Actor_Kill(&this->actor);
                break;
        }
    }
}

static void func_80ABEF2C(EnOkarinaTag* this, PlayState* play) {
    Player* player;
    u16 ocarinaSong;

    player = GET_PLAYER(play);
    this->unk_15A++;
    if (((this->switchFlag >= 0) && (Flags_GetSwitch(play, this->switchFlag)))) {
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    } else {
        if ((this->ocarinaSong != 6) || (gSaveContext.save.saveInfo.scarecrowSpawnSongSet)) {
            // PORT: MM's Player offers the ocarina to an actor that offers itself, where OoT's plays for an actor
            // that asks (PLAYER_STATE2_PLAY_FOR_ACTOR).
            if ((this->actor.xzDistToPlayer < (90.0f + this->interactRange)) &&
                (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) < 80.0f)) {
                if (Actor_OcarinaInteractionAccepted(&this->actor, &play->state)) {
                    ocarinaSong = this->ocarinaSong;
                    if (ocarinaSong == 6) {
                        ocarinaSong = 0xA;
                    }
                    Message_StartOcarinaOot(play, ocarinaSong + OCARINA_ACTION_CHECK_SARIA_Oot);
                    this->actionFunc = func_80ABF0CC;
                } else {
                    Actor_OfferOcarinaInteraction(&this->actor, play, 90.0f + this->interactRange, 80.0f);
                }
            }
        }
    }
}

static void func_80ABF0CC(EnOkarinaTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_04_Oot) {
        this->actionFunc = func_80ABEF2C;
    } else {
        if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
            if ((this->switchFlag >= 0)) {
                Flags_SetSwitch(play, this->switchFlag);
            }
            if (play->sceneId == SCENE_WATER_TEMPLE_Oot) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
            }
            if ((play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) &&
                (play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS_Oot)) {
                Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
            }
            Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
            this->actionFunc = func_80ABEF2C;
            return;
        }
        if (this->unk_158 != 0) {
            if ((Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_05_Oot) || (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_06_Oot) ||
                (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_07_Oot) || (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_08_Oot) ||
                (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_09_Oot) || (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_0A_Oot) ||
                (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_0D_Oot)) {
                if ((this->switchFlag >= 0)) {
                    Flags_SetSwitch(play, this->switchFlag);
                }
                Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
                Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                this->actionFunc = func_80ABEF2C;
                return;
            }
        }
        if ((Hyrule_GetOcarinaModeOot(play) >= OCARINA_MODE_05_Oot) &&
            (Hyrule_GetOcarinaModeOot(play) < OCARINA_MODE_0E_Oot)) {
            Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
            this->actionFunc = func_80ABEF2C;
        }
    }
}

static void func_80ABF28C(EnOkarinaTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->unk_15A++;
    if ((this->ocarinaSong != 6) || (gSaveContext.save.saveInfo.scarecrowSpawnSongSet)) {
        if (((this->switchFlag >= 0) && (Flags_GetSwitch(play, this->switchFlag)))) {
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        } else if (((this->type != 4) ||
                    (!GET_EVENTCHKINF_OOT(EVENTCHKINF_OPENED_THE_DOOR_OF_TIME_Oot))) &&
                   ((this->type != 6) || !GET_EVENTCHKINF_OOT(EVENTCHKINF_DESTROYED_ROYAL_FAMILY_TOMB_Oot)) &&
                   (this->actor.xzDistToPlayer < (90.0f + this->interactRange)) &&
                   (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) < 80.0f)) {
            // PORT: as above, the spot offers itself to MM's ocarina.
            if (Actor_OcarinaInteractionAccepted(&this->actor, &play->state)) {
                switch (this->type) {
                    case 1:
                        Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_LULLABY_Oot);
                        break;
                    case 2:
                        Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_STORMS_Oot);
                        break;
                    case 4:
                        Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_TIME_Oot);
                        break;
                    case 6:
                        Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_LULLABY_Oot);
                        break;
                    default:
                        Actor_Kill(&this->actor);
                        break;
                }
                this->actionFunc = func_80ABF4C8;
            } else {
                Actor_OfferOcarinaInteraction(&this->actor, play, 90.0f + this->interactRange, 80.0f);
            }
        }
    }
}

static void func_80ABF4C8(EnOkarinaTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_04_Oot) {
        this->actionFunc = func_80ABF28C;
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        if ((this->switchFlag >= 0)) {
            Flags_SetSwitch(play, this->switchFlag);
        }
        switch (this->type) {
            case 1: // Zora's River Waterfall
                Flags_SetSwitch(play, this->switchFlag);
                SET_EVENTCHKINF_OOT(EVENTCHKINF_OPENED_ZORAS_DOMAIN_Oot);
                break;
            case 2: // Kakariko Windmill
                if ((true)) {
                    gCsCtxOot.segment = D_80ABF9D0;
                    Hyrule_TriggerCutsceneOot(1);
                }
                // PORT: OoT's Audio_SetMainBgmTempoFreqAfterFanfare (the windmill's music speeds up) is not in MM.
                break;
            case 4: // Door of Time
                if ((true)) {
                    gCsCtxOot.segment = D_80ABFB40;
                    Hyrule_TriggerCutsceneOot(1);
                }
                break;
            case 6: // Royal Family Tomb
                if ((true)) {
                    gCsCtxOot.segment = LINK_IS_ADULT_OOT ? Lib_SegmentedToVirtual(&spot02_scene_Cs_003C80)
                                                        : Lib_SegmentedToVirtual(&spot02_scene_Cs_005020);
                    Hyrule_TriggerCutsceneOot(1);
                }
                SET_EVENTCHKINF_OOT(EVENTCHKINF_DESTROYED_ROYAL_FAMILY_TOMB_Oot);
                Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                break;
            default:
                break;
        }
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
        this->actionFunc = func_80ABF28C;
    } else {
        if (Hyrule_GetOcarinaModeOot(play) >= OCARINA_MODE_05_Oot) {
            if (Hyrule_GetOcarinaModeOot(play) < OCARINA_MODE_0E_Oot) {
                Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
                this->actionFunc = func_80ABF28C;
                return;
            }
        }
    }
}

static void func_80ABF708(EnOkarinaTag* this, PlayState* play) {
    s16 yawDiff;
    s16 yawDiffNew;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = func_80ABF7CC;
    } else {
        yawDiff = this->actor.yawTowardsPlayer - this->actor.world.rot.y;
        this->unk_15A++;
        if (!(this->actor.xzDistToPlayer > 120.0f)) {
            if (CHECK_QUEST_ITEM_OOT(QUEST_SONG_SUN_Oot)) {
                this->actor.textId = TEXT_OOT(0x5021);
            }
            yawDiffNew = ABS(yawDiff);
            if (yawDiffNew < 0x4300) {
                this->unk_15A = 0;
                Actor_OfferTalk(&this->actor, play, 70.0f);
            }
        }
    }
}

static void func_80ABF7CC(EnOkarinaTag* this, PlayState* play) {

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        if ((!CHECK_QUEST_ITEM_OOT(QUEST_SONG_SUN_Oot))) {
            gCsCtxOot.segment = Lib_SegmentedToVirtual(&gSunSongGraveSunSongTeachCs);
            Hyrule_TriggerCutsceneOot(1);
        }
        this->actionFunc = func_80ABF708;
    }
}

static void EnOkarinaTag_Update(Actor* thisx, PlayState* play) {
    EnOkarinaTag* this = (EnOkarinaTag*)thisx;

    this->actionFunc(this, play);
    if (BREG(0) != 0) {
        if (this->unk_15A != 0) {
            if (!(this->unk_15A & 1)) {
                DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                       this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, 1.0f,
                                       1.0f, 1.0f, 120, 120, 120, 255, 4, play->state.gfxCtx);
            }
        } else {
            DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                   this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, 1.0f,
                                   1.0f, 1.0f, 255, 0, 0, 255, 4, play->state.gfxCtx);
        }
    }
}
