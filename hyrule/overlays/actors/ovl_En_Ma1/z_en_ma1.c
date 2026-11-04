/*
 * File: z_en_ma1.c
 * Overlay: En_Ma1
 * Description: Child Malon
 */

#include "z_en_ma1.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "HyruleOcarina.h"
#include "assets/oot/objects/object_ma1/object_ma1.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

static void EnMa1_Init(Actor* thisx, PlayState* play);
static void EnMa1_Destroy(Actor* thisx, PlayState* play);
static void EnMa1_Update(Actor* thisx, PlayState* play);
static void EnMa1_Draw(Actor* thisx, PlayState* play);

static u16 EnMa1_GetText(PlayState* play, Actor* this);
static s16 EnMa1_UpdateTalkState(PlayState* play, Actor* this);

static void EnMa1_Idle(EnMa1* this, PlayState* play);
static void EnMa1_GiveWeirdEgg(EnMa1* this, PlayState* play);
static void EnMa1_FinishGivingWeirdEgg(EnMa1* this, PlayState* play);
static void EnMa1_IdleTeachSong(EnMa1* this, PlayState* play);
static void EnMa1_StartTeachSong(EnMa1* this, PlayState* play);
static void EnMa1_TeachSong(EnMa1* this, PlayState* play);
static void EnMa1_WaitForPlayback(EnMa1* this, PlayState* play);
static void EnMa1_DoNothing(EnMa1* this, PlayState* play);

ActorProfile En_Ma1_Oot_Profile = {
    /**/ ACTOR_EN_MA1_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_MA1_Oot,
    /**/ sizeof(EnMa1),
    /**/ EnMa1_Init,
    /**/ EnMa1_Destroy,
    /**/ EnMa1_Update,
    /**/ EnMa1_Draw,
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

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

typedef enum {
    /* 0 */ ENMA1_ANIM_0,
    /* 1 */ ENMA1_ANIM_1,
    /* 2 */ ENMA1_ANIM_2,
    /* 3 */ ENMA1_ANIM_3
} EnMa1Animation;

static AnimationFrameCountInfo sAnimationInfo[] = {
    { &gMalonChildIdleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMalonChildIdleAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gMalonChildSingAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMalonChildSingAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
};

static Vec3f D_80AA16B8 = { 800.0f, 0.0f, 0.0f };

static void* sMouthTextures[] = {
    gMalonChildNeutralMouthTex,
    gMalonChildSmilingMouthTex,
    gMalonChildTalkingMouthTex,
};

static void* sEyeTextures[] = {
    gMalonChildEyeOpenTex,
    gMalonChildEyeHalfTex,
    gMalonChildEyeClosedTex,
};

static u16 EnMa1_GetText(PlayState* play, Actor* thisx) {
    bool malonReturnedFromCastle = (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot));
    bool malonTaughtEponasSong =
        (CHECK_QUEST_ITEM_OOT(QUEST_SONG_EPONA_Oot));
    u16 faceReaction = Text_GetFaceReactionOot(play, 0x17);

    if (faceReaction != 0) {
        return faceReaction;
    }
    if (malonTaughtEponasSong) {
        return TEXT_OOT(0x204A);
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_INVITED_TO_SING_WITH_CHILD_MALON_Oot)) {
        return TEXT_OOT(0x2049);
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_CHILD_MALON_AT_RANCH_Oot)) {
        if ((GET_INFTABLE_OOT(INFTABLE_CHILD_MALON_SAID_EPONA_WAS_AFRAID_OF_YOU_Oot))) {
            return TEXT_OOT(0x2049);
        } else {
            return TEXT_OOT(0x2048);
        }
    }
    if (malonReturnedFromCastle) {
        return TEXT_OOT(0x2047);
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_POCKET_EGG_Oot)) {
        return TEXT_OOT(0x2044);
    }
    if (GET_INFTABLE_OOT(INFTABLE_MET_CHILD_MALON_AT_CASTLE_OR_MARKET_Oot)) {
        if (GET_INFTABLE_OOT(INFTABLE_ENTERED_HYRULE_CASTLE_Oot)) {
            return TEXT_OOT(0x2043);
        } else {
            return TEXT_OOT(0x2042);
        }
    }
    return TEXT_OOT(0x2041);
}

static s16 EnMa1_UpdateTalkState(PlayState* play, Actor* thisx) {
    s16 ret = NPC_TALK_STATE_TALKING;

    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_CLOSING:
            switch (thisx->textId) {
                case TEXT_OOT(0x2041):
                    SET_INFTABLE_OOT(INFTABLE_MET_CHILD_MALON_AT_CASTLE_OR_MARKET_Oot);
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_CHILD_MALON_AT_CASTLE_OR_MARKET_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                case TEXT_OOT(0x2043):
                    ret = NPC_TALK_STATE_TALKING;
                    break;
                case TEXT_OOT(0x2047):
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_CHILD_MALON_AT_RANCH_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                case TEXT_OOT(0x2048):
                    SET_INFTABLE_OOT(INFTABLE_CHILD_MALON_SAID_EPONA_WAS_AFRAID_OF_YOU_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                case TEXT_OOT(0x2049):
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_INVITED_TO_SING_WITH_CHILD_MALON_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                case TEXT_OOT(0x2061):
                    ret = NPC_TALK_STATE_ACTION;
                    break;
                default:
                    ret = NPC_TALK_STATE_IDLE;
                    break;
            }
            break;
        case TEXT_STATE_CHOICE:
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(play)) {
                ret = NPC_TALK_STATE_ACTION;
            }
            break;
        case TEXT_STATE_DONE:
            if (Message_ShouldAdvance(play)) {
                ret = NPC_TALK_STATE_ITEM_GIVEN;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
        case TEXT_STATE_FADING:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            ret = NPC_TALK_STATE_TALKING;
            break;
    }
    return ret;
}

static s32 EnMa1_ShouldSpawn(EnMa1* this, PlayState* play) {
    bool malonReturnedFromCastle = (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot));

    if ((this->actor.shape.rot.z == 3) && (gSaveContext.sceneLayer == 5)) {
        return 1;
    }
    if (!LINK_IS_CHILD_OOT) {
        return 0;
    }
    // Causes Malon to appear in the market if you haven't met her yet.
    if (((play->sceneId == SCENE_MARKET_NIGHT_Oot) || (play->sceneId == SCENE_MARKET_DAY_Oot)) && !malonReturnedFromCastle &&
        !GET_INFTABLE_OOT(INFTABLE_ENTERED_HYRULE_CASTLE_Oot)) {
        return 1;
    }
    if ((play->sceneId == SCENE_HYRULE_CASTLE_Oot) && !malonReturnedFromCastle) {
        if (GET_INFTABLE_OOT(INFTABLE_ENTERED_HYRULE_CASTLE_Oot)) {
            return 1;
        } else {
            SET_INFTABLE_OOT(INFTABLE_ENTERED_HYRULE_CASTLE_Oot);
            return 0;
        }
    }
    if ((play->sceneId == SCENE_LON_LON_BUILDINGS_Oot) && IS_NIGHT_OOT && malonReturnedFromCastle) {
        return 1;
    }
    if (play->sceneId != SCENE_LON_LON_RANCH_Oot) {
        return 0;
    }
    if ((this->actor.shape.rot.z == 3) && IS_DAY_OOT && malonReturnedFromCastle) {
        return 1;
    }
    return 0;
}

static void EnMa1_UpdateEyes(EnMa1* this) {
    if (DECR(this->blinkTimer) == 0) {
        this->eyeIndex += 1;
        if (this->eyeIndex >= 3) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIndex = 0;
        }
    }
}

static void EnMa1_ChangeAnim(EnMa1* this, s32 index) {
    f32 frameCount = Animation_GetLastFrame(sAnimationInfo[index].animation);

    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f, frameCount,
                     sAnimationInfo[index].mode, sAnimationInfo[index].morphFrames);
}

static void EnMa1_UpdateTracking(EnMa1* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 trackingMode;

    if ((this->interactInfo.talkState == NPC_TALK_STATE_IDLE) && (this->skelAnime.animation == &gMalonChildSingAnim)) {
        trackingMode = NPC_TRACKING_NONE;
    } else {
        trackingMode = NPC_TRACKING_PLAYER_AUTO_TURN;
    }

    this->interactInfo.trackPos = player->actor.world.pos;
    this->interactInfo.trackPos.y -= -10.0f;

    Npc_TrackPoint(&this->actor, &this->interactInfo, 0, trackingMode);
}

static void EnMa1_UpdateSinging(EnMa1* this) {
    if (this->skelAnime.animation == &gMalonChildSingAnim) {
        if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
            if (this->singingDisabled != 0) {
                this->singingDisabled = 0;
                // PORT: OoT's Audio_ToggleMalonSinging (her voice in Lon Lon Ranch's music) is not ported.
            }
        } else {
            if (this->singingDisabled == 0) {
                this->singingDisabled = 1;
            }
        }
    }
}

static void EnMa1_Init(Actor* thisx, PlayState* play) {
    EnMa1* this = (EnMa1*)thisx;
    bool malonReturnedFromCastle = (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot));
    bool malonTaughtEponasSong =
        (CHECK_QUEST_ITEM_OOT(QUEST_SONG_EPONA_Oot));
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gMalonChildSkel, NULL, NULL, NULL, 0);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, DamageTable_Get_Oot(22), &sColChkInfoInit);

    if (!EnMa1_ShouldSpawn(this, play)) {
        Actor_Kill(&this->actor);
        return;
    }

    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.attentionRangeType = 6;
    this->interactInfo.talkState = NPC_TALK_STATE_IDLE;

    if (!malonReturnedFromCastle || malonTaughtEponasSong) {
        this->actionFunc = EnMa1_Idle;
        EnMa1_ChangeAnim(this, ENMA1_ANIM_2);
    } else {
        this->actionFunc = EnMa1_IdleTeachSong;
        EnMa1_ChangeAnim(this, ENMA1_ANIM_2);
    }
}

static void EnMa1_Destroy(Actor* thisx, PlayState* play) {
    EnMa1* this = (EnMa1*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
    Collider_DestroyCylinder(play, &this->collider);
}

static void EnMa1_Idle(EnMa1* this, PlayState* play) {
    bool malonReturnedFromCastle = (GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot));
    bool malonTaughtEponasSong =
        (CHECK_QUEST_ITEM_OOT(QUEST_SONG_EPONA_Oot));

    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        if (this->skelAnime.animation != &gMalonChildIdleAnim) {
            EnMa1_ChangeAnim(this, ENMA1_ANIM_1);
        }
    } else {
        if (this->skelAnime.animation != &gMalonChildSingAnim) {
            EnMa1_ChangeAnim(this, ENMA1_ANIM_3);
        }
    }

    if ((play->sceneId == SCENE_HYRULE_CASTLE_Oot) && malonReturnedFromCastle) {
        if ((true)) {
            Actor_Kill(&this->actor);
        }
    } else if (!malonReturnedFromCastle || malonTaughtEponasSong) {
        if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
            this->actionFunc = EnMa1_GiveWeirdEgg;
            play->msgCtx.stateTimer = 4;
            play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        }
    }
}

static void EnMa1_GiveWeirdEgg(EnMa1* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actor.parent = NULL;
        this->actionFunc = EnMa1_FinishGivingWeirdEgg;
    } else {
        Actor_OfferGetItem(&this->actor, play, GI_WEIRD_EGG_Oot, 120.0f, 10.0f);
    }
}

static void EnMa1_FinishGivingWeirdEgg(EnMa1* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ITEM_GIVEN ||
        !(true)) {
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        this->actionFunc = EnMa1_Idle;
        SET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_POCKET_EGG_Oot);
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
    }
}

static void EnMa1_IdleTeachSong(EnMa1* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        if (this->skelAnime.animation != &gMalonChildIdleAnim) {
            EnMa1_ChangeAnim(this, ENMA1_ANIM_1);
        }
    } else {
        if (this->skelAnime.animation != &gMalonChildSingAnim) {
            EnMa1_ChangeAnim(this, ENMA1_ANIM_3);
        }
    }

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_INVITED_TO_SING_WITH_CHILD_MALON_Oot)) {
        // PORT: MM's Player offers the ocarina to an actor that offers itself, where OoT's plays for an actor that
        // asks (PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR).
        if (Actor_OcarinaInteractionAccepted(&this->actor, &play->state)) {
            this->actor.textId = TEXT_OOT(0x2061);
            Message_StartTextbox(play, this->actor.textId, NULL);
            this->interactInfo.talkState = NPC_TALK_STATE_TALKING;
            this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
            this->actionFunc = EnMa1_StartTeachSong;
        } else {
            Actor_OfferOcarinaInteraction(&this->actor, play, 30.0f + (f32)this->collider.dim.radius, 100.0f);
        }
    }
}

static void EnMa1_StartTeachSong(EnMa1* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        AudioOcarina_SetInstrument(2);
        Message_StartOcarinaOot(play, OCARINA_ACTION_TEACH_EPONA_Oot);
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->actionFunc = EnMa1_TeachSong;
    }
}

static void EnMa1_TeachSong(EnMa1* this, PlayState* play) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_SONG_DEMO_DONE) {
        Message_StartOcarinaOot(play, OCARINA_ACTION_PLAYBACK_EPONA_Oot);
        this->actionFunc = EnMa1_WaitForPlayback;
    }
}

static void EnMa1_WaitForPlayback(EnMa1* this, PlayState* play) {

    if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
        play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
        gSaveContext.nextCutsceneIndex = 0xFFF1;
        play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
        play->transitionTrigger = TRANS_TRIGGER_START;
        this->actionFunc = EnMa1_DoNothing;
    }
}

static void EnMa1_DoNothing(EnMa1* this, PlayState* play) {
}

static void EnMa1_Update(Actor* thisx, PlayState* play) {
    EnMa1* this = (EnMa1*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);
    EnMa1_UpdateEyes(this);
    this->actionFunc(this, play);
    if (this->actionFunc != EnMa1_DoNothing) {
        Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState, (f32)this->collider.dim.radius + 30.0f,
                          EnMa1_GetText, EnMa1_UpdateTalkState);
    }
    EnMa1_UpdateSinging(this);
    EnMa1_UpdateTracking(this, play);
}

static s32 EnMa1_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnMa1* this = (EnMa1*)thisx;
    Vec3s vec;

    if ((limbIndex == 2) || (limbIndex == 5)) {
        *dList = NULL;
    }
    if (limbIndex == 15) {
        Matrix_Translate(1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        vec = this->interactInfo.headRot;
        Matrix_RotateXFApply((vec.y / 32768.0f) * M_PI);
        Matrix_RotateZF((vec.x / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == 8) {
        vec = this->interactInfo.torsoRot;
        Matrix_RotateXFApply((-vec.y / 32768.0f) * M_PI);
        Matrix_RotateZF((-vec.x / 32768.0f) * M_PI, MTXMODE_APPLY);
    }
    return false;
}

static void EnMa1_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnMa1* this = (EnMa1*)thisx;
    Vec3f vec = D_80AA16B8;

    if (limbIndex == 15) {
        Matrix_MultVec3f(&vec, &this->actor.focus.pos);
    }
}

static void EnMa1_Draw(Actor* thisx, PlayState* play) {
    EnMa1* this = (EnMa1*)thisx;
    Camera* camera;
    f32 distFromCamera;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    camera = GET_ACTIVE_CAM(play);
    distFromCamera = Math_Vec3f_DistXZ(&this->actor.world.pos, &camera->eye);
    // PORT: OoT's Audio_UpdateMalonSinging (Malon's voice in Lon Lon Ranch's music, by distance) is not ported.
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(sMouthTextures[this->mouthIndex]));
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIndex]));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnMa1_OverrideLimbDraw, EnMa1_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
