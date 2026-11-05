/*
 * File: z_en_fu.c
 * Overlay: ovl_En_Fu
 * Description: Windmill Man
 */

#include "z_en_fu.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_fu/object_fu.h"
#include "assets/oot/scenes/indoors/hakasitarelay/hakasitarelay_scene.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_UPDATE_DURING_OCARINA)

#define FU_RESET_LOOK_ANGLE (1 << 0)
#define FU_WAIT (1 << 1)

static void EnFu_Init(Actor* thisx, PlayState* play);
static void EnFu_Destroy(Actor* thisx, PlayState* play);
static void EnFu_Update(Actor* thisx, PlayState* play);
static void EnFu_Draw(Actor* thisx, PlayState* play);

static void EnFu_WaitChild(EnFu* this, PlayState* play);
static void func_80A1DA04(EnFu* this, PlayState* play);

static void EnFu_WaitAdult(EnFu* this, PlayState* play);
static void EnFu_TeachSong(EnFu* this, PlayState* play);
static void EnFu_WaitForPlayback(EnFu* this, PlayState* play);
static void func_80A1DBA0(EnFu* this, PlayState* play);
static void func_80A1DBD4(EnFu* this, PlayState* play);
static void func_80A1DB60(EnFu* this, PlayState* play);

ActorProfile En_Fu_Oot_Profile = {
    /**/ ACTOR_EN_FU_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_FU_Oot,
    /**/ sizeof(EnFu),
    /**/ EnFu_Init,
    /**/ EnFu_Destroy,
    /**/ EnFu_Update,
    /**/ EnFu_Draw,
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
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 30, 40, 0, { 0, 0, 0 } },
};

static Vec3f sMtxSrc = {
    700.0f,
    700.0f,
    0.0f,
};

typedef enum {
    /* 0x00 */ FU_FACE_CALM,
    /* 0x01 */ FU_FACE_MAD
} EnFuFace;

static void EnFu_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnFu* this = (EnFu*)thisx;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 36.0f);
    SkelAnime_InitFlex(play, &this->skelanime, &gWindmillManSkel, &gWindmillManPlayStillAnim, this->jointTable,
                       this->morphTable, FU_LIMB_MAX);
    Animation_PlayLoop(&this->skelanime, &gWindmillManPlayStillAnim);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    Actor_SetScale(&this->actor, 0.01f);
    if (!LINK_IS_ADULT_OOT) {
        this->actionFunc = EnFu_WaitChild;
        this->facialExpression = FU_FACE_CALM;
    } else {
        this->actionFunc = EnFu_WaitAdult;
        this->facialExpression = FU_FACE_MAD;
        this->skelanime.playSpeed = 2.0f;
    }
    this->behaviorFlags = 0;
    this->actor.attentionRangeType = 6;
}

static void EnFu_Destroy(Actor* thisx, PlayState* play) {
    EnFu* this = (EnFu*)thisx;
    Collider_DestroyCylinder(play, &this->collider);
}

static s32 func_80A1D94C(EnFu* this, PlayState* play, u16 textID, EnFuActionFunc actionFunc) {
    s16 yawDiff;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = actionFunc;
        return true;
    }
    this->actor.textId = textID;
    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if ((ABS(yawDiff) < 0x2301) && (this->actor.xzDistToPlayer < 100.0f)) {
        Actor_OfferTalk(&this->actor, play, 100.0f);
    } else {
        this->behaviorFlags |= FU_RESET_LOOK_ANGLE;
    }
    return false;
}

static void func_80A1DA04(EnFu* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->behaviorFlags &= ~FU_WAIT;
        this->actionFunc = EnFu_WaitChild;

        if (this->skelanime.animation == &gWindmillManPlayAndMoveHeadAnim) {
            Animation_Change(&this->skelanime, &gWindmillManPlayStillAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gWindmillManPlayStillAnim), ANIMMODE_ONCE, -4.0f);
        }
    }
}

static void EnFu_WaitChild(EnFu* this, PlayState* play) {
    u16 textID = Text_GetFaceReactionOot(play, 0xB);

    if (textID == 0) {
        textID = (GET_EVENTCHKINF_OOT(EVENTCHKINF_DRAINED_WELL_IN_KAKARIKO_Oot)) ? TEXT_OOT(0x5033) : TEXT_OOT(0x5032);
    }

    // if ACTOR_FLAG_PLAYER_TALKED_TO is set and textID is 0x5033, change animation
    // if func_80A1D94C returns 1, actionFunc is set to func_80A1DA04
    if (func_80A1D94C(this, play, textID, func_80A1DA04)) {
        if (textID == 0x5033) {
            Animation_Change(&this->skelanime, &gWindmillManPlayAndMoveHeadAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gWindmillManPlayAndMoveHeadAnim), ANIMMODE_ONCE, -4.0f);
        }
    }
}

static void func_80A1DB60(EnFu* this, PlayState* play) {
    if (gCsCtxOot.state == CS_STATE_IDLE_Oot) {
        this->actionFunc = EnFu_WaitAdult;
        SET_EVENTCHKINF_OOT(EVENTCHKINF_LEARNED_SONG_OF_STORMS_Oot);
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
    }
}

static void func_80A1DBA0(EnFu* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = EnFu_WaitAdult;
    }
}

static void func_80A1DBD4(EnFu* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Hyrule_GetOcarinaModeOot(play) >= OCARINA_MODE_04_Oot) {
        this->actionFunc = EnFu_WaitAdult;
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        this->actionFunc = func_80A1DB60;
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        if ((true)) {
            gCsCtxOot.segment = Lib_SegmentedToVirtual(gSongOfStormsCs);
            Hyrule_TriggerCutsceneOot(1);
        }
        if ((true)) {
            Item_Give(play, ITEM_SONG_STORMS_Oot);
        }
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_00_Oot);
        SET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SONG_OF_STORMS_IN_WINDMILL_Oot);
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_02_Oot) {
        // PORT: MM takes an accepted ocarina offer back by itself (OoT's ATTEMPT_PLAY_FOR_ACTOR).
        this->actionFunc = EnFu_WaitAdult;
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_01_Oot) {
        Hyrule_OfferOcarinaOot(play, (Actor*)this);
    }
}

static void EnFu_WaitForPlayback(EnFu* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Hyrule_OfferOcarinaOot(play, (Actor*)this);
    // if dialog state is 7, player has played back the song
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_SONG_DEMO_DONE) {
        Message_StartOcarinaOot(play, OCARINA_ACTION_PLAYBACK_STORMS_Oot);
        this->actionFunc = func_80A1DBD4;
    }
}

static void EnFu_TeachSong(EnFu* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Hyrule_OfferOcarinaOot(play, (Actor*)this);
    // if dialog state is 2, start song demonstration
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
        this->behaviorFlags &= ~FU_WAIT;
        AudioOcarina_SetInstrument(4); // seems to be related to setting instrument type
        Message_StartOcarinaOot(play, OCARINA_ACTION_TEACH_STORMS_Oot);
        this->actionFunc = EnFu_WaitForPlayback;
    }
}

static void EnFu_WaitAdult(EnFu* this, PlayState* play) {
    static s16 yawDiff;
    Player* player = GET_PLAYER(play);

    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    if ((GET_EVENTCHKINF_OOT(EVENTCHKINF_LEARNED_SONG_OF_STORMS_Oot))) {
        func_80A1D94C(this, play, 0x508E, func_80A1DBA0);
    } else if (Hyrule_OcarinaAttemptedOot(play, (Actor*)this)) {
        this->actor.textId = TEXT_OOT(0x5035);
        Message_StartTextbox(play, this->actor.textId, NULL);
        this->actionFunc = EnFu_TeachSong;
        this->behaviorFlags |= FU_WAIT;
    } else if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = func_80A1DBA0;
    } else if (ABS(yawDiff) < 0x2301) {
        if (this->actor.xzDistToPlayer < 100.0f) {
            this->actor.textId = TEXT_OOT(0x5034);
            Actor_OfferTalk(&this->actor, play, 100.0f);
            Hyrule_OfferOcarinaOot(play, (Actor*)this);
        }
    }
}

static void EnFu_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnFu* this = (EnFu*)thisx;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    if ((!(this->behaviorFlags & FU_WAIT)) && (SkelAnime_Update(&this->skelanime) != 0)) {
        Animation_Change(&this->skelanime, this->skelanime.animation, 1.0f, 0.0f,
                         Animation_GetLastFrame(this->skelanime.animation), ANIMMODE_ONCE, 0.0f);
    }
    this->actionFunc(this, play);
    if ((this->behaviorFlags & FU_RESET_LOOK_ANGLE)) {
        Math_SmoothStepToS(&this->lookAngleOffset.x, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->lookAngleOffset.y, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->unk_2A2.x, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->unk_2A2.y, 0, 6, 6200, 100);
        this->behaviorFlags &= ~FU_RESET_LOOK_ANGLE;
    } else {
        Actor_TrackPlayer(play, &this->actor, &this->lookAngleOffset, &this->unk_2A2, this->actor.focus.pos);
    }
}

static s32 EnFu_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnFu* this = (EnFu*)thisx;
    s32 pad;

    if (limbIndex == FU_LIMB_UNK) {
        return false;
    }
    switch (limbIndex) {
        case FU_LIMB_HEAD:
            rot->x += this->lookAngleOffset.y;
            rot->z += this->lookAngleOffset.x;
            break;
        case FU_LIMB_CHEST_MUSIC_BOX:
            break;
    }

    if (!(this->behaviorFlags & FU_WAIT)) {
        return false;
    }

    if (limbIndex == FU_LIMB_CHEST_MUSIC_BOX) {
        rot->y += (Math_SinS((play->state.frames * (limbIndex * 50 + 0x814))) * 200.0f);
        rot->z += (Math_CosS((play->state.frames * (limbIndex * 50 + 0x940))) * 200.0f);
    }
    return false;
}

static void EnFu_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnFu* this = (EnFu*)thisx;

    if (limbIndex == FU_LIMB_HEAD) {
        Matrix_MultVec3f(&sMtxSrc, &this->actor.focus.pos);
    }
}

static void EnFu_Draw(Actor* thisx, PlayState* play) {
    static void* sEyesSegments[] = { gWindmillManEyeClosedTex, gWindmillManEyeAngryTex };
    static void* sMouthSegments[] = { gWindmillManMouthOpenTex, gWindmillManMouthAngryTex };
    s32 pad;
    EnFu* this = (EnFu*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL37_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyesSegments[this->facialExpression]));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(sMouthSegments[this->facialExpression]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelanime, EnFu_OverrideLimbDraw, EnFu_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
