#include "z_en_in.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "HyruleCutscene.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"
#include "assets/oot/objects/object_in/object_in.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnIn_Init(Actor* thisx, PlayState* play);
static void EnIn_Destroy(Actor* thisx, PlayState* play);
static void EnIn_Update(Actor* thisx, PlayState* play);
static void EnIn_Draw(Actor* thisx, PlayState* play);
static void EnIn_Reset(void);

static void EnIn_WaitForObject(EnIn* this, PlayState* play);
static void func_80A7A304(EnIn* this, PlayState* play);
static void func_80A7A4C8(EnIn* this, PlayState* play);
static void func_80A7A568(EnIn* this, PlayState* play);
static void func_80A7A848(EnIn* this, PlayState* play);
static void func_80A7ABD4(EnIn* this, PlayState* play);
static void func_80A7AEF0(EnIn* this, PlayState* play);
static void func_80A7B018(EnIn* this, PlayState* play);
static void func_80A7B024(EnIn* this, PlayState* play);
static void func_80A7AE84(EnIn* this, PlayState* play);
static void func_80A7A770(EnIn* this, PlayState* play);
static void func_80A7A940(EnIn* this, PlayState* play);
static void func_80A7AA40(EnIn* this, PlayState* play);
static void func_80A7A4BC(EnIn* this, PlayState* play);

ActorProfile En_In_Oot_Profile = {
    /**/ ACTOR_EN_IN_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_IN_Oot,
    /**/ sizeof(EnIn),
    /**/ EnIn_Init,
    /**/ EnIn_Destroy,
    /**/ EnIn_Update,
    /**/ EnIn_Draw,
    /**/ EnIn_Reset,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 18, 46, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = {
    0, 0, 0, 0, MASS_IMMOVABLE,
};

typedef enum {
    /* 0 */ ENIN_ANIM_0,
    /* 1 */ ENIN_ANIM_1,
    /* 2 */ ENIN_ANIM_2,
    /* 3 */ ENIN_ANIM_3,
    /* 4 */ ENIN_ANIM_4,
    /* 5 */ ENIN_ANIM_5,
    /* 6 */ ENIN_ANIM_6,
    /* 7 */ ENIN_ANIM_7,
    /* 8 */ ENIN_ANIM_8,
    /* 9 */ ENIN_ANIM_9
} EnInAnimation;

static AnimationFrameCountInfo sAnimationInfo[] = {
    { &object_in_Anim_001CC0, 1.0f, ANIMMODE_LOOP, 0.0f }, { &object_in_Anim_001CC0, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &object_in_Anim_013C6C, 1.0f, ANIMMODE_LOOP, 0.0f }, { &object_in_Anim_013C6C, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &object_in_Anim_000CB0, 1.0f, ANIMMODE_LOOP, 0.0f }, { &object_in_Anim_0003B4, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &object_in_Anim_001BE0, 1.0f, ANIMMODE_LOOP, 0.0f }, { &object_in_Anim_013D60, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &object_in_Anim_01431C, 1.0f, ANIMMODE_LOOP, 0.0f }, { &object_in_Anim_014CA8, 1.0f, ANIMMODE_LOOP, 0.0f },
};

static AnimationHeader* D_80A7B918[] = {
    &object_in_Anim_0151C8, &object_in_Anim_015DF0, &object_in_Anim_016B3C, &object_in_Anim_015814,
    &object_in_Anim_01646C, &object_in_Anim_0175D0, &object_in_Anim_017B58, &object_in_Anim_018C38,
};

static Gfx* sAdultEraDLs[] = {
    NULL,
    NULL,
    gIngoAdultEraLeftThighDL,
    gIngoAdultEraLeftLegDL,
    gIngoAdultEraLeftFootDL,
    gIngoAdultEraRightThighDL,
    gIngoAdultEraRightLegDL,
    gIngoAdultEraRightFootDL,
    gIngoAdultEraTorsoDL,
    gIngoAdultEraChestDL,
    gIngoAdultEraLeftShoulderDL,
    gIngoAdultEraLeftArmDL,
    gIngoAdultEraLeftHandDL,
    gIngoAdultEraRightShoulderDL,
    gIngoAdultEraRightArmDL,
    gIngoAdultEraRightHandDL,
    gIngoAdultEraHeadDL,
    gIngoAdultEraLeftEyebrowDL,
    gIngoAdultEraRightEyebrowDL,
    gIngoAdultEraMustacheDL,
};

static u16 EnIn_GetTextIdChild(PlayState* play) {
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot)) {
        if (GET_INFTABLE_OOT(INFTABLE_97_Oot)) {
            return TEXT_OOT(0x2046);
        } else {
            return TEXT_OOT(0x2045);
        }
    }
    if (GET_INFTABLE_OOT(INFTABLE_94_Oot)) {
        return TEXT_OOT(0x2040);
    } else {
        return TEXT_OOT(0x203F);
    }
}

static u16 EnIn_GetTextIdAdult(PlayState* play) {
    Player* player = GET_PLAYER(play);
    u16 temp_v0 = Text_GetFaceReactionOot(play, 25);

    if (temp_v0 != 0) {
        return temp_v0;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
        if (IS_DAY_OOT) {
            return TEXT_OOT(0x205F);
        } else {
            return TEXT_OOT(0x2057);
        }
    }
    if (IS_NIGHT_OOT) {
        return TEXT_OOT(0x204E);
    }
    switch (gSaveContext.eventInfOot[0] & 0xF) {
        case 1:
            if (!(player->stateFlags1 & PLAYER_STATE1_800000)) {
                return TEXT_OOT(0x2036);
            } else if (GET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot)) {
                if (GET_INFTABLE_OOT(INFTABLE_A2_Oot)) {
                    return TEXT_OOT(0x2036);
                } else {
                    return TEXT_OOT(0x2038);
                }
            } else {
                return TEXT_OOT(0x2037);
            }
        case 3:
            if ((gSaveContext.eventInfOot[0] & 0x40) || (gSaveContext.eventInfOot[0] & 0x20)) {
                return TEXT_OOT(0x203E);
            } else {
                return TEXT_OOT(0x203D);
            }
        case 4:
            return TEXT_OOT(0x203A);
        case 5:
        case 6:
            return TEXT_OOT(0x203C);
        case 7:
            return TEXT_OOT(0x205B);
        case 2:
        default:
            if (GET_INFTABLE_OOT(INFTABLE_SPOKE_TO_INGO_ONCE_AS_ADULT_Oot)) {
                return TEXT_OOT(0x2031);
            } else {
                return TEXT_OOT(0x2030);
            }
    }
}

static u16 EnIn_GetTextId(PlayState* play, Actor* thisx) {
    u16 temp_v0 = Text_GetFaceReactionOot(play, 25);

    if (temp_v0 != 0) {
        return temp_v0;
    }
    if (!LINK_IS_ADULT_OOT) {
        return EnIn_GetTextIdChild(play);
    } else {
        return EnIn_GetTextIdAdult(play);
    }
}

static s16 EnIn_UpdateTalkStateOnClosing(PlayState* play, Actor* thisx) {
    s32 ret = NPC_TALK_STATE_IDLE;

    switch (thisx->textId) {
        case TEXT_OOT(0x2045):
            SET_INFTABLE_OOT(INFTABLE_97_Oot);
            break;
        case TEXT_OOT(0x203E):
            ret = NPC_TALK_STATE_ACTION;
            break;
        case TEXT_OOT(0x203F):
            SET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_INGO_AT_RANCH_BEFORE_TALON_RETURNS_Oot);
            SET_INFTABLE_OOT(INFTABLE_94_Oot);
            break;
    }
    return ret;
}

static s16 EnIn_UpdateTalkStateOnChoice(PlayState* play, Actor* thisx) {
    EnIn* this = (EnIn*)thisx;
    s32 sp18 = NPC_TALK_STATE_TALKING;

    switch (this->actor.textId) {
        case TEXT_OOT(0x2030):
        case TEXT_OOT(0x2031):
            if (play->msgCtx.choiceIndex == 1) {
                this->actor.textId = TEXT_OOT(0x2032);
            } else if (gSaveContext.save.saveInfo.playerData.rupees < 10) {
                this->actor.textId = TEXT_OOT(0x2033);
            } else {
                this->actor.textId = TEXT_OOT(0x2034);
            }
            Message_ContinueTextbox(play, this->actor.textId);
            SET_INFTABLE_OOT(INFTABLE_SPOKE_TO_INGO_ONCE_AS_ADULT_Oot);
            break;
        case TEXT_OOT(0x2034):
            if (play->msgCtx.choiceIndex == 1) {
                Rupees_ChangeBy(-10);
                this->actor.textId = TEXT_OOT(0x205C);
            } else {
                this->actor.textId = TEXT_OOT(0x2035);
            }
            Message_ContinueTextbox(play, this->actor.textId);
            break;
        case TEXT_OOT(0x2036):
        case TEXT_OOT(0x2037):
            if (play->msgCtx.choiceIndex == 1) {
                sp18 = NPC_TALK_STATE_ACTION;
            } else {
                this->actor.textId = TEXT_OOT(0x201F);
                Message_ContinueTextbox(play, this->actor.textId);
            }
            break;
        case TEXT_OOT(0x2038):
            if (play->msgCtx.choiceIndex == 0 && gSaveContext.save.saveInfo.playerData.rupees >= 50) {
                sp18 = NPC_TALK_STATE_ACTION;
            } else {
                this->actor.textId = TEXT_OOT(0x2039);
                Message_ContinueTextbox(play, this->actor.textId);
                SET_INFTABLE_OOT(INFTABLE_A2_Oot);
            }
            break;
        case TEXT_OOT(0x205B):
            if (play->msgCtx.choiceIndex == 0 && gSaveContext.save.saveInfo.playerData.rupees >= 50) {
                sp18 = NPC_TALK_STATE_ACTION;
            } else {
                Message_ContinueTextbox(play, this->actor.textId = TEXT_OOT(0x2039));
                gSaveContext.eventInfOot[0] &= ~0xF;
                gSaveContext.eventInfOot[0] &= ~0x20;
                gSaveContext.eventInfOot[0] &= ~0x40;
                this->actionFunc = func_80A7A4C8;
            }
            break;
    }
    if (!gSaveContext.save.saveInfo.playerData.rupees) {}

    return sp18;
}

static s16 EnIn_UpdateTalkStateOnEvent(PlayState* play, Actor* thisx) {
    s32 phi_v1 = NPC_TALK_STATE_TALKING;

    if (thisx->textId == TEXT_OOT(0x2035)) {
        Rupees_ChangeBy(-10);
        thisx->textId = TEXT_OOT(0x205C);
        Message_ContinueTextbox(play, thisx->textId);
    } else {
        phi_v1 = NPC_TALK_STATE_ACTION;
    }
    return phi_v1;
}

static s16 EnIn_UpdateTalkState(PlayState* play, Actor* thisx) {
    s16 sp1E = NPC_TALK_STATE_TALKING;

    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
            break;
        case TEXT_STATE_CLOSING:
            sp1E = EnIn_UpdateTalkStateOnClosing(play, thisx);
            break;
        case TEXT_STATE_FADING:
            break;
        case TEXT_STATE_CHOICE:
            if (Message_ShouldAdvance(play)) {
                sp1E = EnIn_UpdateTalkStateOnChoice(play, thisx);
            }
            break;
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(play)) {
                sp1E = EnIn_UpdateTalkStateOnEvent(play, thisx);
            }
            break;
        case TEXT_STATE_DONE:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            break;
    }
    return sp1E;
}

static void func_80A795C8(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 npcTrackingMode;

    if (this->skelAnime.animation == &object_in_Anim_0003B4 || this->skelAnime.animation == &object_in_Anim_001BE0 ||
        this->skelAnime.animation == &object_in_Anim_013D60) {
        npcTrackingMode = NPC_TRACKING_NONE;
    } else {
        npcTrackingMode = NPC_TRACKING_PLAYER_AUTO_TURN;
    }
    if (this->actionFunc == func_80A7A568) {
        npcTrackingMode = NPC_TRACKING_FULL_BODY;
    }
    if (this->actionFunc == func_80A7B024) {
        this->interactInfo.trackPos = play->view.eye;
        this->interactInfo.yOffset = 60.0f;
    } else {
        this->interactInfo.trackPos = player->actor.world.pos;
        this->interactInfo.yOffset = 16.0f;
    }
    Npc_TrackPoint(&this->actor, &this->interactInfo, 1, npcTrackingMode);
}

static void func_80A79690(SkelAnime* skelAnime, EnIn* this, PlayState* play) {
    if (skelAnime->baseTransl.y < skelAnime->jointTable[0].y) {
        skelAnime->movementFlags |= 3;
        AnimTaskQueue_AddActorMovement(play, &this->actor, skelAnime, 1.0f);
    }
}

static void EnIn_ChangeAnim(EnIn* this, s32 index) {
    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f,
                     Animation_GetLastFrame(sAnimationInfo[index].animation), sAnimationInfo[index].mode,
                     sAnimationInfo[index].morphFrames);
}

static s32 func_80A7975C(EnIn* this, PlayState* play) {
    if (this->actor.params != 1 || this->actor.shape.rot.z != 1 || !LINK_IS_ADULT_OOT) {
        return 0;
    }
    this->animationIdx = 1;
    this->collider.base.ocFlags1 &= ~OC1_ON;
    Animation_Change(&this->skelAnime, D_80A7B918[this->animationIdx], 1.0f, 0.0f,
                     Animation_GetLastFrame(D_80A7B918[this->animationIdx]), 2, 0.0f);
    this->actionFunc = func_80A7A304;
    return 1;
}

static s32 EnIn_GetStartMode(EnIn* this, PlayState* play) {
    if (play->sceneId == SCENE_LON_LON_RANCH_Oot && LINK_IS_CHILD_OOT && IS_DAY_OOT && this->actor.shape.rot.z == 1 &&
        !GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot)) {
        return 1;
    }
    if (play->sceneId == SCENE_STABLE_Oot && LINK_IS_CHILD_OOT && IS_DAY_OOT && this->actor.shape.rot.z == 3 &&
        (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot))) {
        return 1;
    }
    if (play->sceneId == SCENE_STABLE_Oot && LINK_IS_CHILD_OOT && IS_NIGHT_OOT) {
        if ((this->actor.shape.rot.z == 2) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot)) {
            return 1;
        }
        if ((this->actor.shape.rot.z == 4) && (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot))) {
            return 1;
        }
    }
    if (play->sceneId == SCENE_LON_LON_RANCH_Oot && LINK_IS_ADULT_OOT && IS_DAY_OOT) {
        if ((this->actor.shape.rot.z == 5) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
            return 2;
        }
        if ((this->actor.shape.rot.z == 7) && (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot))) {
            return 4;
        }
    }
    if (play->sceneId == SCENE_LON_LON_BUILDINGS_Oot && LINK_IS_ADULT_OOT && IS_NIGHT_OOT) {
        if (this->actor.shape.rot.z == 6 && !GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
            return 3;
        }
        if (this->actor.shape.rot.z == 8 && (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot))) {
            return 3;
        }
    }
    return 0;
}

static void EnIn_UpdateEyes(EnIn* this) {
    if (this->eyeIndex != 3) {
        if (DECR(this->blinkTimer) == 0) {
            this->eyeIndex++;
            if (this->eyeIndex >= 3) {
                this->blinkTimer = Rand_S16Offset(30, 30);
                this->eyeIndex = 0;
            }
        }
    }
}

static void func_80A79AB4(EnIn* this, PlayState* play) {
    s32 i;
    u32 f = 0;

    if (this->skelAnime.animation != &object_in_Anim_014CA8) {
        f = play->gameplayFrames;
    }
    for (i = 0; i < ARRAY_COUNT(this->unk_330); i++) {
        this->unk_330[i].y = (2068 + 50 * i) * f;
        this->unk_330[i].z = (2368 + 50 * i) * f;
    }
}

static void func_80A79BAC(EnIn* this, PlayState* play, s32 index, u32 transitionType) {
    s16 entrances[] = { 0x558 /* ENTR_LON_LON_RANCH_8 */, 0x4CA /* ENTR_LON_LON_RANCH_6 */, 0x157 /* ENTR_LON_LON_RANCH_ENTRANCE */ };

    play->nextEntrance = Hyrule_ExitFromOot(entrances[index]);
    if (index == 2) {
        gSaveContext.nextCutsceneIndex = 0xFFF0;
    }
    play->transitionType = transitionType;
    play->transitionTrigger = TRANS_TRIGGER_START;
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
    Interface_SetHudVisibility(1);
    if (index == 0) {
        AREG(6) = 0;
    }
    gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_OFF;
}

static void func_80A79C78(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f sp48;
    Vec3f sp3C;
    Vec3s zeroVec = { 0, 0, 0 };

    this->camId = Play_CreateSubCamera(play);
    Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
    Play_ChangeCameraStatus(play, this->camId, CAM_STATUS_ACTIVE);
    sp48.x = this->actor.world.pos.x;
    sp48.y = this->actor.world.pos.y + 60.0f;
    sp48.z = this->actor.world.pos.z;
    sp3C.x = sp48.x;
    sp3C.y = sp48.y - 22.0f;
    sp3C.z = sp48.z + 40.0f;
    Play_SetCameraAtEye(play, this->camId, &sp48, &sp3C);
    this->actor.shape.rot.y = Math_Vec3f_Yaw(&this->actor.world.pos, &sp3C);
    this->interactInfo.headRot = zeroVec;
    this->interactInfo.torsoRot = zeroVec;
    Message_StartTextbox(play, TEXT_OOT(0x2025), NULL);
    this->interactInfo.talkState = NPC_TALK_STATE_TALKING;
    player->actor.world.pos = this->actor.world.pos;
    player->actor.world.pos.x += 100.0f * Math_SinS(this->actor.shape.rot.y);
    player->actor.world.pos.z += 100.0f * Math_CosS(this->actor.shape.rot.y);
    if (player->rideActor != NULL) {
        player->rideActor->world.pos = player->actor.world.pos;
        player->rideActor->freezeTimer = 10;
    }
    player->actor.freezeTimer = 10;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    ShrinkWindow_Letterbox_SetSizeTarget(0x20);
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE_ALT);
}

static s32 D_80A7B998 = 0;

static void EnIn_Init(Actor* thisx, PlayState* play) {
    EnIn* this = (EnIn*)thisx;
    RespawnData* respawn = &gSaveContext.respawn[RESPAWN_MODE_DOWN];
    Vec3f respawnPos;

    this->ingoObjBankIndex = Object_GetSlot(&play->objectCtx, OBJECT_IN_Oot);
    if (this->ingoObjBankIndex < 0 && this->actor.params > 0) {
        this->actionFunc = NULL;
        Actor_Kill(&this->actor);
        return;
    }
    respawnPos = respawn->pos;
    // hardcoded coords for lon lon entrance
    if (D_80A7B998 == 0 && respawnPos.x == 1107.0f && respawnPos.y == 0.0f && respawnPos.z == -3740.0f) {
        gSaveContext.eventInfOot[0] = 0;
        D_80A7B998 = 1;
    }
    this->actionFunc = EnIn_WaitForObject;
}

static void EnIn_Destroy(Actor* thisx, PlayState* play) {
    EnIn* this = (EnIn*)thisx;

    if (this->actionFunc != NULL && this->actionFunc != EnIn_WaitForObject) {
        Collider_DestroyCylinder(play, &this->collider);
    }
}

static void EnIn_WaitForObject(EnIn* this, PlayState* play) {
    s32 sp3C = 0;

    if (Object_IsLoaded(&play->objectCtx, this->ingoObjBankIndex) || this->actor.params <= 0) {
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 36.0f);
        SkelAnime_InitFlex(play, &this->skelAnime, &gIngoSkel, NULL, this->jointTable, this->morphTable, 20);
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
        if (func_80A7975C(this, play)) {
            gSaveContext.eventInfOot[0] &= ~0x8000;
            return;
        }
        Actor_SetScale(&this->actor, 0.01f);
        this->actor.attentionRangeType = 6;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        this->actionFunc = func_80A7A4BC;

        switch (EnIn_GetStartMode(this, play)) {
            case 1:
                EnIn_ChangeAnim(this, ENIN_ANIM_9);
                this->actionFunc = func_80A7A4BC;
                break;
            case 3:
                EnIn_ChangeAnim(this, ENIN_ANIM_7);
                this->actionFunc = func_80A7A4BC;
                if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
                    this->actor.params = 5;
                }
                break;
            case 4:
                EnIn_ChangeAnim(this, ENIN_ANIM_8);
                this->eyeIndex = 3;
                this->actionFunc = func_80A7A4BC;
                break;
            case 0:
                Actor_Kill(&this->actor);
                break;
            default:
                switch (gSaveContext.eventInfOot[0] & 0xF) {
                    case 0:
                    case 2:
                    case 3:
                    case 4:
                    case 7:
                        if (this->actor.params == 2) {
                            sp3C = 1;
                        }
                        break;
                    case 1:
                        if (this->actor.params == 3) {
                            sp3C = 1;
                        }
                        break;
                    case 5:
                    case 6:
                        if (this->actor.params == 4) {
                            sp3C = 1;
                        }
                        break;
                }
                if (sp3C != 1) {
                    Actor_Kill(&this->actor);
                    return;
                }
                switch (gSaveContext.eventInfOot[0] & 0xF) {
                    case 0:
                    case 2:
                        EnIn_ChangeAnim(this, ENIN_ANIM_2);
                        this->actionFunc = func_80A7A4C8;
                        gSaveContext.eventInfOot[0] = 0;
                        break;
                    case 1:
                        this->actor.attentionRangeType = 3;
                        EnIn_ChangeAnim(this, ENIN_ANIM_2);
                        this->actionFunc = func_80A7A568;
                        Interface_StartTimer(TIMER_ID_MINIGAME_1, 0x3C);
                        break;
                    case 3:
                        EnIn_ChangeAnim(this, ENIN_ANIM_4);
                        this->actionFunc = func_80A7A770;
                        break;
                    case 4:
                        EnIn_ChangeAnim(this, ENIN_ANIM_6);
                        this->unk_1EC = 8;
                        this->actionFunc = func_80A7A940;
                        break;
                    case 5:
                    case 6:
                        this->actor.attentionRangeType = 3;
                        EnIn_ChangeAnim(this, ENIN_ANIM_6);
                        this->unk_1EC = 8;
                        this->actionFunc = func_80A7AA40;
                        break;
                    case 7:
                        EnIn_ChangeAnim(this, ENIN_ANIM_2);
                        this->actionFunc = func_80A7A848;
                        break;
                }
        }
    }
}

static void func_80A7A304(EnIn* this, PlayState* play) {
    if (this->skelAnime.animation == &object_in_Anim_015814 || this->skelAnime.animation == &object_in_Anim_01646C) {
        if (this->skelAnime.curFrame == 8.0f) {
            Audio_PlaySfx_Randomized(&this->actor.projectedPos, NA_SE_VO_IN_LASH_0,
                                  NA_SE_VO_IN_LASH_1 - NA_SE_VO_IN_LASH_0 + 1);
        }
    }
    if (this->skelAnime.animation == &object_in_Anim_018C38 && this->skelAnime.curFrame == 20.0f) {
        Actor_PlaySfx(&this->actor, NA_SE_VO_IN_CRY_0);
    }
    if (SkelAnime_Update(&this->skelAnime) != 0) {
        this->animationIdx %= 8;
        this->unk_1E8 = this->animationIdx;
        if (this->animationIdx == 3 || this->animationIdx == 4) {
            Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_IT_LASH);
            if (Rand_ZeroOne() < 0.3f) {
                AudioSfx_PlaySfx(NA_SE_IT_INGO_HORSE_NEIGH, &this->actor.projectedPos, 4,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
        }
        Animation_Change(&this->skelAnime, D_80A7B918[this->animationIdx], 1.0f, 0.0f,
                         Animation_GetLastFrame(D_80A7B918[this->animationIdx]), 2, -10.0f);
    }
}

static void func_80A7A4BC(EnIn* this, PlayState* play) {
}

static void func_80A7A4C8(EnIn* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        func_80A79BAC(this, play, 1, TRANS_TYPE_FADE_BLACK_FAST);
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x000F) | 0x0001;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x8000) | 0x8000;
        CLEAR_INFTABLE_OOT(INFTABLE_A2_Oot);
        Environment_ForcePlaySequence(NA_BGM_HORSE);
        play->msgCtx.stateTimer = 0;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void func_80A7A568(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 phi_a2;
    s32 transitionType;

    if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot) && (player->stateFlags1 & PLAYER_STATE1_800000)) {
        SET_INFTABLE_OOT(INFTABLE_AB_Oot);
    }
    if (gSaveContext.timerStates[TIMER_ID_MINIGAME_1] == TIMER_STATE_STOP) {
        Audio_PlaySfx(NA_SE_SY_FOUND);
        func_80A79C78(this, play);
        this->actionFunc = func_80A7B024;
        gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_OFF;
    } else if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        if (play->msgCtx.choiceIndex == 0) {
            if (gSaveContext.save.saveInfo.playerData.rupees < 50) {
                play->msgCtx.stateTimer = 4;
                play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
                this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
                return;
            }
            gSaveContext.eventInfOot[0] =
                (gSaveContext.eventInfOot[0] & ~0x10) | (((EnHorse*)GET_PLAYER(play)->rideActor)->type << 4);
            gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0xF) | 2;
            phi_a2 = 2;
            transitionType = TRANS_TYPE_FADE_BLACK;
        } else {
            Audio_PlaySfx(NA_SE_SY_FOUND);
            if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot)) {
                if (GET_INFTABLE_OOT(INFTABLE_AB_Oot)) {
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot);
                    SET_INFTABLE_OOT(INFTABLE_AB_Oot);
                }
            }
            gSaveContext.eventInfOot[0] &= ~0xF;
            phi_a2 = 0;
            transitionType = TRANS_TYPE_FADE_BLACK_FAST;
        }
        if ((true)) {
            func_80A79BAC(this, play, phi_a2, transitionType);
        }
        play->msgCtx.stateTimer = 0;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x8000) | 0x8000;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void func_80A7A770(EnIn* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
        this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
    } else if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        Rupees_ChangeBy(-50);
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        EnIn_ChangeAnim(this, ENIN_ANIM_3);
        this->actionFunc = func_80A7A848;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x0F) | 7;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & 0xFFFF) | 0x20;
        if (!(gSaveContext.eventInfOot[0] & 0x40)) {
            play->msgCtx.stateTimer = 4;
            play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        }
    }
}

static void func_80A7A848(EnIn* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        if ((play->msgCtx.choiceIndex == 0 && gSaveContext.save.saveInfo.playerData.rupees < 50) || play->msgCtx.choiceIndex == 1) {
            gSaveContext.eventInfOot[0] &= ~0xF;
            this->actionFunc = func_80A7A4C8;
        } else {
            if ((true)) {
                func_80A79BAC(this, play, 2, TRANS_TYPE_FADE_BLACK_FAST);
            }
            gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0xF) | 2;
            gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x8000) | 0x8000;
            play->msgCtx.stateTimer = 0;
            play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        }
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        if ((true)) {
            gSaveContext.eventInfOot[0] &= ~0x20;
            gSaveContext.eventInfOot[0] &= ~0x40;
        }
    }
}

static void func_80A7A940(EnIn* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
        this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        return;
    }
    if (this->unk_1EC != 0) {
        this->unk_1EC--;
        if (this->unk_1EC == 0) {
            Actor_PlaySfx(&this->actor, NA_SE_VO_IN_LOST);
        }
    }
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        func_80A79BAC(this, play, 2, TRANS_TYPE_FADE_BLACK_FAST);
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x000F) | 0x0002;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x8000) | 0x8000;
        play->msgCtx.stateTimer = 0;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & 0xFFFF) | 0x40;
    }
}

static void func_80A7AA40(EnIn* this, PlayState* play) {
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    Vec3f sp30;
    Vec3f sp24;

    this->camId = Play_CreateSubCamera(play);
    this->activeCamId = play->activeCamId;
    Play_ChangeCameraStatus(play, this->activeCamId, CAM_STATUS_WAIT);
    Play_ChangeCameraStatus(play, this->camId, CAM_STATUS_ACTIVE);

    this->subCamAtOffset = 0.0f;
    this->unk_2F4 = 50.0f;
    this->unk_2F8 = 0.0f;
    this->subCamEyeOffset = 0.0f;
    this->unk_300 = 50.0f;
    this->unk_304 = 50.0f;

    sp30 = this->actor.world.pos;
    sp24 = this->actor.world.pos;

    sp30.x += this->subCamAtOffset;
    sp30.y += this->unk_2F4;
    sp30.z += this->unk_2F8;

    sp24.x += this->subCamEyeOffset;
    sp24.y += this->unk_300;
    sp24.z += this->unk_304;

    Play_SetCameraAtEye(play, this->camId, &sp30, &sp24);
    this->actor.textId = TEXT_OOT(0x203B);
    Message_StartTextbox(play, this->actor.textId, NULL);
    this->interactInfo.talkState = NPC_TALK_STATE_TALKING;
    this->unk_1FC = 0;
    gCsCtxOot.frames = 0;
    ShrinkWindow_Letterbox_SetSizeTarget(0x20);
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE_ALT);
    this->actionFunc = func_80A7ABD4;
}

static void func_80A7ABD4(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f sp48;
    Vec3f sp3C;

    if (player->rideActor != NULL) {
        player->rideActor->freezeTimer = 10;
    }
    player->actor.freezeTimer = 10;
    if (this->actor.textId == TEXT_OOT(0x203B)) {
        if (this->unk_1EC != 0) {
            this->unk_1EC--;
            if (this->unk_1EC == 0) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_IN_LOST);
            }
        }
    }
    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
            if (this->actor.textId == TEXT_OOT(0x203B)) {
                // PORT: 3Ship's race frees the ranch (docs/EPONA.md E3): Link already has Epona, so OoT's "she's yours,
                // but you'll never leave" (0x203C), the gate and the fence jump are left out.
                play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
                this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot);
                SET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_KAKARIKO_Oot);
                gSaveContext.eventInfOot[0] = 0;
                Play_ChangeCameraStatus(play, this->activeCamId, CAM_STATUS_ACTIVE);
                Play_ClearCamera(play, this->camId);
                ShrinkWindow_Letterbox_SetSizeTarget(0);
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
                Interface_SetHudVisibility(0x32);
                this->actionFunc = func_80A7B018;
            } else {
                play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
                this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
            }
        }
    } else {
        if (gCsCtxOot.frames++ >= 50) {
            this->actionFunc = func_80A7AE84;
            return;
        }
        if (gCsCtxOot.frames == 44) {
            Actor_PlaySfx(&this->actor, NA_SE_EV_RONRON_DOOR_CLOSE);
        }
        Math_SmoothStepToF(&this->subCamAtOffset, 0.0f, 0.06f, 10000.0f, 0.0f);
        Math_SmoothStepToF(&this->unk_2F4, 50.0f, 0.06f, 10000.0f, 0.0f);
        Math_SmoothStepToF(&this->unk_2F8, 0.0f, 0.06f, 10000.0f, 0.0f);
        Math_SmoothStepToF(&this->subCamEyeOffset, 0.0f, 0.06f, 10000.0f, 0.0f);
        Math_SmoothStepToF(&this->unk_300, 150.0f, 0.06f, 10000.0f, 0.0f);
        Math_SmoothStepToF(&this->unk_304, 300.0f, 0.06f, 10000.0f, 0.0f);

        sp48 = this->actor.world.pos;
        sp3C = this->actor.world.pos;

        sp48.x += this->subCamAtOffset;
        sp48.y += this->unk_2F4;
        sp48.z += this->unk_2F8;
        sp3C.x += this->subCamEyeOffset;
        sp3C.y += this->unk_300;
        sp3C.z += this->unk_304;
        Play_SetCameraAtEye(play, this->camId, &sp48, &sp3C);
    }
}

static void func_80A7AE84(EnIn* this, PlayState* play) {
    Play_ChangeCameraStatus(play, this->activeCamId, CAM_STATUS_ACTIVE);
    Play_ClearCamera(play, this->camId);
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
    Interface_SetHudVisibility(0x32);
    this->actionFunc = func_80A7AEF0;
}

static void func_80A7AEF0(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 yaw;
    Vec3f pos = this->actor.world.pos;

    pos.x += 90.0f * Math_SinS(this->actor.shape.rot.y);
    pos.z += 90.0f * Math_CosS(this->actor.shape.rot.y);
    yaw = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
    if (ABS(yaw) > 0x4000) {
        play->nextEntrance = Hyrule_ExitFromOot(0x476); // OoT's 0x476 /* ENTR_HYRULE_FIELD_15 */
        play->transitionTrigger = TRANS_TRIGGER_START;
        play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
        this->actionFunc = func_80A7B018;
    } else if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        play->msgCtx.stateTimer = 4;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void func_80A7B018(EnIn* this, PlayState* play) {
}

static void func_80A7B024(EnIn* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (player->rideActor != NULL) {
        player->rideActor->freezeTimer = 10;
    }
    player->actor.freezeTimer = 10;
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot) && (GET_INFTABLE_OOT(INFTABLE_AB_Oot))) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_RENTED_HORSE_FROM_INGO_Oot);
            SET_INFTABLE_OOT(INFTABLE_AB_Oot);
        }
        func_80A79BAC(this, play, 0, TRANS_TYPE_FADE_BLACK_FAST);
        gSaveContext.eventInfOot[0] = gSaveContext.eventInfOot[0] & ~0xF;
        gSaveContext.eventInfOot[0] = (gSaveContext.eventInfOot[0] & ~0x8000) | 0x8000;
        play->msgCtx.stateTimer = 4;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void EnIn_Update(Actor* thisx, PlayState* play) {
    ColliderCylinder* collider;
    EnIn* this = (EnIn*)thisx;

    if (this->actionFunc == EnIn_WaitForObject) {
        this->actionFunc(this, play);
        return;
    }
    collider = &this->collider;
    Collider_UpdateCylinder(&this->actor, collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &collider->base);
    if (this->actionFunc != func_80A7A304) {
        SkelAnime_Update(&this->skelAnime);
        if (this->skelAnime.animation == &object_in_Anim_001BE0 && ((gSaveContext.eventInfOot[0] & 0xF) != 6)) {
            func_80A79690(&this->skelAnime, this, play);
        }
        Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    }
    EnIn_UpdateEyes(this);
    this->actionFunc(this, play);
    if (this->actionFunc != func_80A7A304) {
        func_80A79AB4(this, play);
        if (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_2] / 100)) < 6 && gSaveContext.timerStates[TIMER_ID_MINIGAME_2] != TIMER_STATE_OFF &&
            this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
            if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {}
        } else {
            Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState,
                              ((this->actor.attentionRangeType == 6) ? 80.0f : 320.0f) + this->collider.dim.radius,
                              EnIn_GetTextId, EnIn_UpdateTalkState);
            if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
                this->unk_1FA = this->unk_1F8;
                this->unk_1F8 = Message_GetState(&play->msgCtx);
            }
        }
        func_80A795C8(this, play);
    }
}

static s32 EnIn_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnIn* this = (EnIn*)thisx;
    Vec3s sp2C;

    if (this->actor.params > 0 && limbIndex != INGO_HEAD_LIMB) {
        if (sAdultEraDLs[limbIndex] != NULL) {
            *dList = sAdultEraDLs[limbIndex];
        }
    }
    if (limbIndex == INGO_HEAD_LIMB) {
        Matrix_Translate(1500.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        sp2C = this->interactInfo.headRot;
        Matrix_RotateZF(BINANG_TO_RAD(sp2C.x), MTXMODE_APPLY);
        Matrix_RotateXFApply(BINANG_TO_RAD(sp2C.y));
        Matrix_Translate(-1500.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == INGO_CHEST_LIMB) {
        sp2C = this->interactInfo.torsoRot;
        Matrix_RotateXFApply(BINANG_TO_RAD(sp2C.x));
        Matrix_RotateYF(BINANG_TO_RAD(sp2C.y), MTXMODE_APPLY);
    }
    if (limbIndex == INGO_CHEST_LIMB || limbIndex == INGO_LEFT_SHOULDER_LIMB || limbIndex == INGO_RIGHT_SHOULDER_LIMB) {
        rot->y += Math_SinS(this->unk_330[limbIndex].y) * 200.0f;
        rot->z += Math_CosS(this->unk_330[limbIndex].z) * 200.0f;
    }
    return 0;
}

static void EnIn_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnIn* this = (EnIn*)thisx;
    Vec3f D_80A7B9A8 = { 1600.0, 0.0f, 0.0f };

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == INGO_HEAD_LIMB) {
        Matrix_MultVec3f(&D_80A7B9A8, &this->actor.focus.pos);
        this->actor.focus.rot = this->actor.world.rot;
    }
    if (limbIndex == INGO_LEFT_HAND_LIMB && this->skelAnime.animation == &object_in_Anim_014CA8) {
        gSPDisplayList(POLY_OPA_DISP++, gIngoChildEraBasketDL);
    }
    if (limbIndex == INGO_RIGHT_HAND_LIMB && this->skelAnime.animation == &object_in_Anim_014CA8) {
        gSPDisplayList(POLY_OPA_DISP++, gIngoChildEraPitchForkDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnIn_Draw(Actor* thisx, PlayState* play) {
    static void* eyeTextures[] = { gIngoEyeOpenTex, gIngoEyeHalfTex, gIngoEyeClosedTex, gIngoEyeClosed2Tex };

    EnIn* this = (EnIn*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    if (this->actionFunc != EnIn_WaitForObject) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTextures[this->eyeIndex]));
        gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gIngoHeadGradient2Tex));
        SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnIn_OverrideLimbDraw, EnIn_PostLimbDraw, &this->actor);
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnIn_Reset(void) {
    D_80A7B998 = 0;
}

// 3Ship [Hyrule] MM's horse sets OoT's Ingo's race animation (OoT's horse wrote these fields itself).
void EnIn_SetRaceAnimationOot(Actor* thisx, s16 animationIdx, f32 curFrame) {
    EnIn* this = (EnIn*)thisx;

    this->animationIdx = animationIdx;
    this->unk_1E0 = curFrame;
}
