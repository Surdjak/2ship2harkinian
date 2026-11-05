#include "z_en_ma2.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "assets/oot/objects/object_ma2/object_ma2.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

static void EnMa2_Init(Actor* thisx, PlayState* play);
static void EnMa2_Destroy(Actor* thisx, PlayState* play);
static void EnMa2_Update(Actor* thisx, PlayState* play);
static void EnMa2_Draw(Actor* thisx, PlayState* play);

static u16 EnMa2_GetTextId(PlayState* play, Actor* this);
static s16 EnMa2_UpdateTalkState(PlayState* play, Actor* this);

static void EnMa2_UpdateTracking(EnMa2* this, PlayState* play);
static s32 EnMa2_IsSinging(EnMa2* this);
static void EnMa2_UpdateEyes(EnMa2* this);
static void EnMa2_UpdateSinging(EnMa2* this, PlayState* play);
static void EnMa2_WaitToEndTalk(EnMa2* this, PlayState* play);
static void EnMa2_WaitForOcarina(EnMa2* this, PlayState* play);
static void EnMa2_WaitForEponasSong(EnMa2* this, PlayState* play);
static void EnMa2_ForceTalkAfterSong(EnMa2* this, PlayState* play);

ActorProfile En_Ma2_Oot_Profile = {
    /**/ ACTOR_EN_MA2_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_MA2_Oot,
    /**/ sizeof(EnMa2),
    /**/ EnMa2_Init,
    /**/ EnMa2_Destroy,
    /**/ EnMa2_Update,
    /**/ EnMa2_Draw,
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
    /* 0 */ ENMA2_ANIM_0,
    /* 1 */ ENMA2_ANIM_1,
    /* 2 */ ENMA2_ANIM_2,
    /* 3 */ ENMA2_ANIM_3,
    /* 4 */ ENMA2_ANIM_4
} EnMa2Animation;

static AnimationFrameCountInfo sAnimationInfo[] = {
    { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },       { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gMalonAdultStandStillAnim, 1.0f, ANIMMODE_LOOP, 0.0f }, { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
};

static u16 EnMa2_GetTextId(PlayState* play, Actor* thisx) {
    u16 faceReaction = Text_GetFaceReactionOot(play, 23);

    if (faceReaction != 0) {
        return faceReaction;
    }
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
        return TEXT_OOT(0x2056);
    }
    if (IS_NIGHT_OOT) {
        if (GET_INFTABLE_OOT(INFTABLE_8C_Oot)) {
            return TEXT_OOT(0x2052);
        } else if (GET_INFTABLE_OOT(INFTABLE_8E_Oot)) {
            return TEXT_OOT(0x2051);
        } else {
            return TEXT_OOT(0x2050);
        }
    }
    return TEXT_OOT(0x204C);
}

static s16 EnMa2_UpdateTalkState(PlayState* play, Actor* thisx) {
    s16 ret = NPC_TALK_STATE_TALKING;

    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_CLOSING:
            switch (thisx->textId) {
                case TEXT_OOT(0x2051):
                    SET_INFTABLE_OOT(INFTABLE_8C_Oot);
                    ret = NPC_TALK_STATE_ACTION;
                    break;
                case TEXT_OOT(0x2053):
                    SET_INFTABLE_OOT(INFTABLE_8D_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                default:
                    ret = NPC_TALK_STATE_IDLE;
                    break;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
        case TEXT_STATE_FADING:
        case TEXT_STATE_CHOICE:
        case TEXT_STATE_EVENT:
        case TEXT_STATE_DONE:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_9:
            break;
    }
    return ret;
}

static void EnMa2_UpdateTracking(EnMa2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 trackingMode;

    if ((this->interactInfo.talkState == NPC_TALK_STATE_IDLE) && (this->skelAnime.animation == &gMalonAdultSingAnim)) {
        trackingMode = NPC_TRACKING_NONE;
    } else {
        trackingMode = NPC_TRACKING_PLAYER_AUTO_TURN;
    }

    this->interactInfo.trackPos = player->actor.world.pos;
    this->interactInfo.yOffset = 0.0f;

    Npc_TrackPoint(&this->actor, &this->interactInfo, 0, trackingMode);
}

static u16 EnMa2_GetSpawnIndex(EnMa2* this, PlayState* play) {
    if (LINK_IS_CHILD_OOT) {
        return 0;
    }
    if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot) && (play->sceneId == SCENE_STABLE_Oot) && IS_DAY_OOT &&
        (this->actor.shape.rot.z == 5)) {
        return 1;
    }
    if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot) && (play->sceneId == SCENE_LON_LON_RANCH_Oot) && IS_NIGHT_OOT &&
        (this->actor.shape.rot.z == 6)) {
        return 2;
    }
    if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot) || (play->sceneId != SCENE_LON_LON_RANCH_Oot)) {
        return 0;
    }
    if ((this->actor.shape.rot.z == 7) && IS_DAY_OOT) {
        return 3;
    }
    if ((this->actor.shape.rot.z == 8) && IS_NIGHT_OOT) {
        return 3;
    }
    return 0;
}

static s32 EnMa2_IsSinging(EnMa2* this) {
    if (this->skelAnime.animation != &gMalonAdultSingAnim) {
        return 0;
    }
    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        return 0;
    }
    this->blinkTimer = 0;
    if (this->eyeIndex != 2) {
        return 0;
    }
    this->mouthIndex = 2;
    return 1;
}

static void EnMa2_UpdateEyes(EnMa2* this) {
    if ((!EnMa2_IsSinging(this)) && (DECR(this->blinkTimer) == 0)) {
        this->eyeIndex += 1;
        if (this->eyeIndex >= 3) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIndex = 0;
        }
    }
}

static void EnMa2_ChangeAnim(EnMa2* this, s32 index) {
    f32 frameCount = Animation_GetLastFrame(sAnimationInfo[index].animation);

    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f, frameCount,
                     sAnimationInfo[index].mode, sAnimationInfo[index].morphFrames);
}

static void EnMa2_UpdateSinging(EnMa2* this, PlayState* play) {
    if (this->skelAnime.animation == &gMalonAdultSingAnim) {
        if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
            if (this->singingDisabled != 0) {
                // PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported.
                this->singingDisabled = 0;
            }
        } else {
            if (this->singingDisabled == 0) {
                // PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported.
                this->singingDisabled = 1;
            }
        }
    }
}

static void EnMa2_Init(Actor* thisx, PlayState* play) {
    EnMa2* this = (EnMa2*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gMalonAdultSkel, NULL, NULL, NULL, 0);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, DamageTable_Get_Oot(22), &sColChkInfoInit);

    switch (EnMa2_GetSpawnIndex(this, play)) {
        case 1:
            EnMa2_ChangeAnim(this, ENMA2_ANIM_2);
            this->actionFunc = EnMa2_WaitToEndTalk;
            break;
        case 2:
            EnMa2_ChangeAnim(this, ENMA2_ANIM_3);
            this->actionFunc = EnMa2_WaitForOcarina;
            break;
        case 3:
            if (GET_INFTABLE_OOT(INFTABLE_8D_Oot)) {
                EnMa2_ChangeAnim(this, ENMA2_ANIM_0);
            } else {
                EnMa2_ChangeAnim(this, ENMA2_ANIM_3);
            }
            this->actionFunc = EnMa2_WaitToEndTalk;
            break;
        case 0:
            Actor_Kill(&this->actor);
            return;
    }

    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.attentionRangeType = 6;
    this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
}

static void EnMa2_Destroy(Actor* thisx, PlayState* play) {
    EnMa2* this = (EnMa2*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
    Collider_DestroyCylinder(play, &this->collider);
}

static void EnMa2_WaitToEndTalk(EnMa2* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void EnMa2_WaitForOcarina(EnMa2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Hyrule_OcarinaAttemptedOot(play, (Actor*)this)) {
        player->autoLockOnActor = &this->actor;
        player->stateFlags2 |= PLAYER_STATE2_2000000;
        Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_EPONA_Oot);
        this->actionFunc = EnMa2_WaitForEponasSong;
    } else if (this->actor.xzDistToPlayer < 30.0f + (f32)this->collider.dim.radius) {
        Hyrule_OfferOcarinaOot(play, (Actor*)this);
    }
}

static void EnMa2_WaitForEponasSong(EnMa2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Hyrule_GetOcarinaModeOot(play) >= OCARINA_MODE_04_Oot) {
        this->actionFunc = EnMa2_WaitForOcarina;
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        this->timer = 0x1E;
        SET_INFTABLE_OOT(INFTABLE_8E_Oot);
        this->actionFunc = EnMa2_ForceTalkAfterSong;
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
    } else {
        Hyrule_OfferOcarinaOot(play, (Actor*)this);
    }
}

static void EnMa2_ForceTalkAfterSong(EnMa2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (DECR(this->timer)) {
        Hyrule_OfferOcarinaOot(play, (Actor*)this);
    } else {
        if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
            this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
            Message_CloseTextbox(play);
        } else {
            this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
            this->actionFunc = EnMa2_WaitToEndTalk;
        }
    }
}

static void EnMa2_Update(Actor* thisx, PlayState* play) {
    EnMa2* this = (EnMa2*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);
    EnMa2_UpdateEyes(this);
    this->actionFunc(this, play);
    EnMa2_UpdateSinging(this, play);
    EnMa2_UpdateTracking(this, play);
    if (this->actionFunc != EnMa2_WaitForEponasSong) {
        Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState, (f32)this->collider.dim.radius + 30.0f,
                          EnMa2_GetTextId, EnMa2_UpdateTalkState);
    }
}

static s32 EnMa2_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnMa2* this = (EnMa2*)thisx;
    Vec3s vec;

    if ((limbIndex == MALON_ADULT_LEFT_THIGH_LIMB) || (limbIndex == MALON_ADULT_RIGHT_THIGH_LIMB)) {
        *dList = NULL;
    }
    if (limbIndex == MALON_ADULT_HEAD_LIMB) {
        Matrix_Translate(1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        vec = this->interactInfo.headRot;
        Matrix_RotateXFApply((vec.y / 32768.0f) * M_PI);
        Matrix_RotateZF((vec.x / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == MALON_ADULT_CHEST_AND_NECK_LIMB) {
        vec = this->interactInfo.torsoRot;
        Matrix_RotateYF((-vec.y / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_RotateXFApply((-vec.x / 32768.0f) * M_PI);
    }
    if ((limbIndex == MALON_ADULT_CHEST_AND_NECK_LIMB) || (limbIndex == MALON_ADULT_LEFT_SHOULDER_LIMB) ||
        (limbIndex == MALON_ADULT_RIGHT_SHOULDER_LIMB)) {
        rot->y += Math_SinS(this->upperBodyRot[limbIndex].y) * 200.0f;
        rot->z += Math_CosS(this->upperBodyRot[limbIndex].z) * 200.0f;
    }
    return false;
}

static void EnMa2_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnMa2* this = (EnMa2*)thisx;
    Vec3f vec = { 900.0f, 0.0f, 0.0f };

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == MALON_ADULT_HEAD_LIMB) {
        Matrix_MultVec3f(&vec, &this->actor.focus.pos);
    }
    if ((limbIndex == MALON_ADULT_LEFT_HAND_LIMB) && (this->skelAnime.animation == &gMalonAdultStandStillAnim)) {
        gSPDisplayList(POLY_OPA_DISP++, gMalonAdultBasketDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnMa2_Draw(Actor* thisx, PlayState* play) {
    static void* sMouthTextures[] = { gMalonAdultMouthNeutralTex, gMalonAdultMouthSadTex, gMalonAdultMouthHappyTex };
    static void* sEyeTextures[] = { gMalonAdultEyeOpenTex, gMalonAdultEyeHalfTex, gMalonAdultEyeClosedTex };

    EnMa2* this = (EnMa2*)thisx;
    Camera* camera;
    f32 someFloat;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    camera = GET_ACTIVE_CAM(play);
    someFloat = Math_Vec3f_DistXZ(&this->actor.world.pos, &camera->eye);
    // PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported.
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(sMouthTextures[this->mouthIndex]));
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIndex]));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnMa2_OverrideLimbDraw, EnMa2_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
