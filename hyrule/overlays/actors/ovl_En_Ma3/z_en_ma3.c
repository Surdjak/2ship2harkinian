/*
 * File: z_en_ma3.c
 * Overlay: En_Ma3
 * Description: Adult Malon (Ranch)
 */

#include "z_en_ma3.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "assets/oot/objects/object_ma2/object_ma2.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void EnMa3_Init(Actor* thisx, PlayState* play);
static void EnMa3_Destroy(Actor* thisx, PlayState* play);
static void EnMa3_Update(Actor* thisx, PlayState* play);
static void EnMa3_Draw(Actor* thisx, PlayState* play);

static u16 EnMa3_GetTextId(PlayState* play, Actor* this);
static s16 EnMa3_UpdateTalkState(PlayState* play, Actor* this);

static void func_80AA2E54(EnMa3* this, PlayState* play);
static s32 func_80AA2EC8(EnMa3* this, PlayState* play);
static s32 func_80AA2F28(EnMa3* this);
static void EnMa3_UpdateEyes(EnMa3* this);
static void func_80AA3200(EnMa3* this, PlayState* play);

ActorProfile En_Ma3_Oot_Profile = {
    /**/ ACTOR_EN_MA3_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_MA2_Oot,
    /**/ sizeof(EnMa3),
    /**/ EnMa3_Init,
    /**/ EnMa3_Destroy,
    /**/ EnMa3_Update,
    /**/ EnMa3_Draw,
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
    /* 0 */ ENMA3_ANIM_0,
    /* 1 */ ENMA3_ANIM_1,
    /* 2 */ ENMA3_ANIM_2,
    /* 3 */ ENMA3_ANIM_3,
    /* 4 */ ENMA3_ANIM_4
} EnMa3Animation;

static AnimationFrameCountInfo sAnimationInfo[] = {
    { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },       { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gMalonAdultStandStillAnim, 1.0f, ANIMMODE_LOOP, 0.0f }, { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
};

static u16 EnMa3_GetTextId(PlayState* play, Actor* thisx) {
    Player* player = GET_PLAYER(play);

    if (!GET_INFTABLE_OOT(INFTABLE_B8_Oot)) {
        return TEXT_OOT(0x2000);
    }
    if (gSaveContext.eventInfOot[0] & 0x400) {
        gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] = SECONDS_TO_TIMER(((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)));
        thisx->flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        if (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)) >= 0xD3) {
            return TEXT_OOT(0x208E);
        }
        if ((HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) == 0) || (HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) >= 0xB4)) {
            HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) = 0xB4;
            gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] = SECONDS_TO_TIMER(((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)));
        }
        if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_WON_COW_IN_MALONS_RACE_Oot) && (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)) < 0x32)) {
            return TEXT_OOT(0x208F);
        } else if (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)) < HIGH_SCORE_OOT(HS_HORSE_RACE_Oot)) {
            return TEXT_OOT(0x2012);
        } else {
            return TEXT_OOT(0x2004);
        }
    }
    if ((!(player->stateFlags1 & PLAYER_STATE1_800000)) &&
        (Actor_FindNearby(play, thisx, ACTOR_EN_HORSE, 1, 1200.0f) == NULL)) {
        return TEXT_OOT(0x2001);
    }
    if (!GET_INFTABLE_OOT(INFTABLE_B9_Oot)) {
        return TEXT_OOT(0x2002);
    } else {
        return TEXT_OOT(0x2003);
    }
}

static s16 EnMa3_UpdateTalkState(PlayState* play, Actor* thisx) {
    s16 ret = NPC_TALK_STATE_TALKING;

    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(play)) {
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's 0x157 /* ENTR_LON_LON_RANCH_ENTRANCE */
                gSaveContext.nextCutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.eventInfOot[0] |= 0x400;
                gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
            }
            break;
        case TEXT_STATE_CHOICE:
            if (Message_ShouldAdvance(play)) {
                SET_INFTABLE_OOT(INFTABLE_B9_Oot);
                if (play->msgCtx.choiceIndex == 0) {
                    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_WON_COW_IN_MALONS_RACE_Oot)) {
                        Message_ContinueTextbox(play, TEXT_OOT(0x2091));
                    } else if (HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) == 0) {
                        Message_ContinueTextbox(play, TEXT_OOT(0x2092));
                    } else {
                        Message_ContinueTextbox(play, TEXT_OOT(0x2090));
                    }
                }
            }
            break;
        case TEXT_STATE_CLOSING:
            switch (thisx->textId) {
                case TEXT_OOT(0x2000):
                    SET_INFTABLE_OOT(INFTABLE_B8_Oot);
                    ret = NPC_TALK_STATE_IDLE;
                    break;
                case TEXT_OOT(0x208F):
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_WON_COW_IN_MALONS_RACE_Oot);
                case TEXT_OOT(0x2004):
                case TEXT_OOT(0x2012):
                    if (HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) > ((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100))) {
                        HIGH_SCORE_OOT(HS_HORSE_RACE_Oot) = ((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100));
                    }
                case TEXT_OOT(0x208E):
                    gSaveContext.eventInfOot[0] &= ~0x400;
                    thisx->flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
                    ret = NPC_TALK_STATE_IDLE;
                    gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
                    break;
                case TEXT_OOT(0x2002):
                    SET_INFTABLE_OOT(INFTABLE_B9_Oot);
                case TEXT_OOT(0x2003):
                    if (!(gSaveContext.eventInfOot[0] & 0x400)) {
                        ret = NPC_TALK_STATE_IDLE;
                    }
                    break;
                default:
                    ret = NPC_TALK_STATE_IDLE;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
        case TEXT_STATE_FADING:
        case TEXT_STATE_DONE:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            break;
    }
    return ret;
}

static void func_80AA2E54(EnMa3* this, PlayState* play) {
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

static s32 func_80AA2EC8(EnMa3* this, PlayState* play) {
    if (LINK_IS_CHILD_OOT) {
        return 2;
    }
    if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
        return 2;
    }
    if (gSaveContext.eventInfOot[0] & 0x400) {
        return 1;
    }
    return 0;
}

static s32 func_80AA2F28(EnMa3* this) {
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

static void EnMa3_UpdateEyes(EnMa3* this) {
    if ((!func_80AA2F28(this)) && (DECR(this->blinkTimer) == 0)) {
        this->eyeIndex += 1;
        if (this->eyeIndex >= 3) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIndex = 0;
        }
    }
}

static void EnMa3_ChangeAnim(EnMa3* this, s32 index) {
    f32 frameCount = Animation_GetLastFrame(sAnimationInfo[index].animation);

    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f, frameCount,
                     sAnimationInfo[index].mode, sAnimationInfo[index].morphFrames);
}

static void EnMa3_Init(Actor* thisx, PlayState* play) {
    EnMa3* this = (EnMa3*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gMalonAdultSkel, NULL, NULL, NULL, 0);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, DamageTable_Get_Oot(22), &sColChkInfoInit);

    switch (func_80AA2EC8(this, play)) {
        case 0:
            EnMa3_ChangeAnim(this, ENMA3_ANIM_0);
            this->actionFunc = func_80AA3200;
            break;
        case 1:
            EnMa3_ChangeAnim(this, ENMA3_ANIM_0);
            this->actionFunc = func_80AA3200;
            break;
        case 2:
            Actor_Kill(&this->actor);
            return;
    }

    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    Actor_SetScale(&this->actor, 0.01f);
    this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
}

static void EnMa3_Destroy(Actor* thisx, PlayState* play) {
    EnMa3* this = (EnMa3*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
    Collider_DestroyCylinder(play, &this->collider);
}

static void func_80AA3200(EnMa3* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    }
}

static void EnMa3_Update(Actor* thisx, PlayState* play) {
    EnMa3* this = (EnMa3*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);
    EnMa3_UpdateEyes(this);
    this->actionFunc(this, play);
    func_80AA2E54(this, play);
    Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState, (f32)this->collider.dim.radius + 150.0f,
                      EnMa3_GetTextId, EnMa3_UpdateTalkState);
    if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
        if (this->isNotSinging != 0) {
            // PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported.
            this->isNotSinging = 0;
        }
    } else if (this->isNotSinging == 0) {
        // PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported.
        this->isNotSinging = 1;
    }
}

static s32 EnMa3_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnMa3* this = (EnMa3*)thisx;
    Vec3s vec;

    if ((limbIndex == MALON_ADULT_LIMB_LEFT_THIGH) || (limbIndex == MALON_ADULT_LIMB_RIGHT_THIGH)) {
        *dList = NULL;
    }
    if (limbIndex == MALON_ADULT_LIMB_HEAD) {
        Matrix_Translate(1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        vec = this->interactInfo.headRot;
        Matrix_RotateXFApply((vec.y / 32768.0f) * M_PI);
        Matrix_RotateZF((vec.x / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == MALON_ADULT_LIMB_CHEST_AND_NECK) {
        vec = this->interactInfo.torsoRot;
        Matrix_RotateYF((-vec.y / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_RotateXFApply((-vec.x / 32768.0f) * M_PI);
    }
    if ((limbIndex == MALON_ADULT_LIMB_CHEST_AND_NECK) || (limbIndex == MALON_ADULT_LIMB_LEFT_SHOULDER) ||
        (limbIndex == MALON_ADULT_LIMB_RIGHT_SHOULDER)) {
        rot->y += Math_SinS(this->unk_212[limbIndex].y) * 200.0f;
        rot->z += Math_CosS(this->unk_212[limbIndex].z) * 200.0f;
    }
    return false;
}

static void EnMa3_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnMa3* this = (EnMa3*)thisx;
    Vec3f vec = { 900.0f, 0.0f, 0.0f };

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == MALON_ADULT_LIMB_HEAD) {
        Matrix_MultVec3f(&vec, &this->actor.focus.pos);
    }

    if ((limbIndex == MALON_ADULT_LIMB_LEFT_HAND) && (this->skelAnime.animation == &gMalonAdultStandStillAnim)) {
        gSPDisplayList(POLY_OPA_DISP++, gMalonAdultBasketDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnMa3_Draw(Actor* thisx, PlayState* play) {
    static void* sMouthTextures[] = { gMalonAdultMouthNeutralTex, gMalonAdultMouthSadTex, gMalonAdultMouthHappyTex };
    static void* sEyeTextures[] = { gMalonAdultEyeOpenTex, gMalonAdultEyeHalfTex, gMalonAdultEyeClosedTex };
    EnMa3* this = (EnMa3*)thisx;
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

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnMa3_OverrideLimbDraw, EnMa3_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
