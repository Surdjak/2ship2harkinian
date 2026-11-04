/*
 * File: z_en_zl4.c
 * Overlay: ovl_En_Zl4
 * Description: Child Princess Zelda
 */

#include "z_en_zl4.h"
#include "HyruleCamera.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64rumble.h"
#include "HyruleScenes.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_zl4/object_zl4.h"
#include "assets/oot/scenes/indoors/nakaniwa/nakaniwa_scene.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

typedef enum {
    /* 0 */ ZL4_CS_WAIT,
    /* 1 */ ZL4_CS_START,
    /* 2 */ ZL4_CS_MEET,
    /* 3 */ ZL4_CS_STONE,
    /* 4 */ ZL4_CS_NAMES,
    /* 5 */ ZL4_CS_LEGEND,
    /* 6 */ ZL4_CS_WINDOW,
    /* 7 */ ZL4_CS_GANON,
    /* 8 */ ZL4_CS_PLAN
} EnZl4CutsceneState;

typedef enum {
    /* 0 */ ZL4_EYES_NEUTRAL,
    /* 1 */ ZL4_EYES_SHUT,
    /* 2 */ ZL4_EYES_LOOK_LEFT,
    /* 3 */ ZL4_EYES_LOOK_RIGHT,
    /* 4 */ ZL4_EYES_WIDE,
    /* 5 */ ZL4_EYES_SQUINT,
    /* 6 */ ZL4_EYES_OPEN
} EnZl4EyeExpression;

typedef enum {
    /* 0 */ ZL4_MOUTH_NEUTRAL,
    /* 1 */ ZL4_MOUTH_HAPPY,
    /* 2 */ ZL4_MOUTH_WORRIED,
    /* 3 */ ZL4_MOUTH_SURPRISED
} EnZl4MouthExpression;

typedef enum {
    /* 0 */ ZL4_EYE_OPEN,
    /* 1 */ ZL4_EYE_BLINK,
    /* 2 */ ZL4_EYE_SHUT,
    /* 3 */ ZL4_EYE_WIDE,
    /* 4 */ ZL4_EYE_SQUINT,
    /* 5 */ ZL4_EYE_LOOK_OUT,
    /* 6 */ ZL4_EYE_LOOK_IN
} EnZl4EyeState;

static void EnZl4_Init(Actor* thisx, PlayState* play);
static void EnZl4_Destroy(Actor* thisx, PlayState* play);
static void EnZl4_Update(Actor* thisx, PlayState* play);
static void EnZl4_Draw(Actor* thisx, PlayState* play);

static void EnZl4_Cutscene(EnZl4* this, PlayState* play);
static void EnZl4_Idle(EnZl4* this, PlayState* play);
static void EnZl4_TheEnd(EnZl4* this, PlayState* play);

ActorProfile En_Zl4_Oot_Profile = {
    /**/ ACTOR_EN_ZL4_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_ZL4_Oot,
    /**/ sizeof(EnZl4),
    /**/ EnZl4_Init,
    /**/ EnZl4_Destroy,
    /**/ EnZl4_Update,
    /**/ EnZl4_Draw,
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
    { 10, 44, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

typedef enum {
    /*  0 */ ZL4_ANIM_0,
    /*  1 */ ZL4_ANIM_1,
    /*  2 */ ZL4_ANIM_2,
    /*  3 */ ZL4_ANIM_3,
    /*  4 */ ZL4_ANIM_4,
    /*  5 */ ZL4_ANIM_5,
    /*  6 */ ZL4_ANIM_6,
    /*  7 */ ZL4_ANIM_7,
    /*  8 */ ZL4_ANIM_8,
    /*  9 */ ZL4_ANIM_9,
    /* 10 */ ZL4_ANIM_10,
    /* 11 */ ZL4_ANIM_11,
    /* 12 */ ZL4_ANIM_12,
    /* 13 */ ZL4_ANIM_13,
    /* 14 */ ZL4_ANIM_14,
    /* 15 */ ZL4_ANIM_15,
    /* 16 */ ZL4_ANIM_16,
    /* 17 */ ZL4_ANIM_17,
    /* 18 */ ZL4_ANIM_18,
    /* 19 */ ZL4_ANIM_19,
    /* 20 */ ZL4_ANIM_20,
    /* 21 */ ZL4_ANIM_21,
    /* 22 */ ZL4_ANIM_22,
    /* 23 */ ZL4_ANIM_23,
    /* 24 */ ZL4_ANIM_24,
    /* 25 */ ZL4_ANIM_25,
    /* 26 */ ZL4_ANIM_26,
    /* 27 */ ZL4_ANIM_27,
    /* 28 */ ZL4_ANIM_28,
    /* 29 */ ZL4_ANIM_29,
    /* 30 */ ZL4_ANIM_30,
    /* 31 */ ZL4_ANIM_31,
    /* 32 */ ZL4_ANIM_32,
    /* 33 */ ZL4_ANIM_33
} EnZl4Animation;

static AnimationInfo sAnimationInfo[] = {
    /*  0 */ /* standing idle */ { &gChildZeldaAnim_000654, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /*  1 */ /* moves to introduce herself */ { &gChildZeldaAnim_00E5C8, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /*  2 */ /* introducing herself */ { &gChildZeldaAnim_00EBC4, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /*  3 */ /* turns away from window surprised */
    { &gChildZeldaAnim_010DF8, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /*  4 */ /* standing with hand in front of mouth */
    { &gChildZeldaAnim_011248, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /*  5 */ /* surprise, moves hand to mouth */ { &gChildZeldaAnim_011698, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /*  6 */ /* uncrosses arms, leans toward link with hands together */
    { &gChildZeldaAnim_011B34, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -8.0f },
    /*  7 */ /* turns to write letter */ { &gChildZeldaAnim_0125E4, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    /*  8 */ /* writing letter */ { &gChildZeldaAnim_012E58, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /*  9 */ /* pulls back, looks askew */ { &gChildZeldaAnim_013280, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /* 10 */ /* looks askew at Link */ { &gChildZeldaAnim_013628, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 11 */ /* crosses arms, looks to the side */ { &gChildZeldaAnim_013A50, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /* 12 */ /* arms crossed, looking away */ { &gChildZeldaAnim_013EA0, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 13 */ /* turns away, hands behind back, looks up */
    { &gChildZeldaAnim_015F14, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 14 */ /* turns back to link, hands on top of each other */
    { &gChildZeldaAnim_0169B4, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 15 */ /* hands behind back looking up */ { &gChildZeldaAnim_016D08, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 16 */ /* leans toward link, looks askew */ { &gChildZeldaAnim_01726C, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /* 17 */ /* leaning toward link, looking askew */
    { &gChildZeldaAnim_017818, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -8.0f },
    /* 18 */ /* neutral, looking at Link */ { &gChildZeldaAnim_01805C, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 19 */ /* moves towards link, hands clasped */
    { &gChildZeldaAnim_018898, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /* 20 */ /* facing link, hands clasped */ { &gChildZeldaAnim_01910C, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 21 */ /* look in window */ { &gChildZeldaAnim_019600, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 22 */ /* leans forward, hands together */ { &gChildZeldaAnim_01991C, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    /* 23 */ /* turns to link, hands on top of each other */
    { &gChildZeldaAnim_01A2FC, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 24 */ /* stands, hands on top of each other */
    { &gChildZeldaAnim_01AAE0, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 25 */ /* leaning forward, hands together */ { &gChildZeldaAnim_01AE88, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 26 */ /* walks aside, points to window */ { &gChildZeldaAnim_01B874, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    /* 27 */ /* stands pointing at window */ { &gChildZeldaAnim_01BCF0, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 28 */ /* laughs, hands together */ { &gChildZeldaAnim_01C494, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 29 */ /* happy, hands together */ { &gChildZeldaAnim_01C7B0, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    /* 30 */ /* standing hands behind back looking down*/
    { &gChildZeldaAnim_01CE08, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 31 */ /* cocks head, hands clasped */ { &gChildZeldaAnim_00F0A4, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    /* 32 */ /* happy, hands clasped */ { &gChildZeldaAnim_00F894, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    /* 33 */ /* transition to standing */ { &gChildZeldaAnim_000654, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -8.0f },
};

#include "z_en_zl4_cutscene_data.c"

static void EnZl4_SetCsCameraAngle(PlayState* play, s16 index) {
    Hyrule_SetCsCameraAngleOot(play, &sCsCameraAngle[index]);
}

// PORT: OoT's CAM_SET_CS_0 spline (Camera_Demo1) through HyruleCamera.h.
static void EnZl4_SetCsCameraMove(PlayState* play, s16 index) {
    Hyrule_SetCsCameraMoveOot(play, &sCsCameraMove[index]);
}

static u16 EnZl4_GetText(PlayState* play, Actor* thisx) {
    u16 faceReaction = Text_GetFaceReactionOot(play, 22);
    u16 stoneCount;
    s16 ret;

    if (faceReaction != 0) {
        return faceReaction;
    }

    stoneCount = 0;
    if (CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot)) {
        stoneCount = 1;
    }
    if (CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot)) {
        stoneCount++;
    }
    if (CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot)) {
        stoneCount++;
    }

    if (stoneCount > 1) {
        ret = TEXT_OOT(0x703D);
    } else {
        ret = TEXT_OOT(0x703C);
    }
    return ret;
}

static s16 func_80B5B9B0(PlayState* play, Actor* thisx) {
    EnZl4* this = (EnZl4*)thisx;

    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
        return NPC_TALK_STATE_IDLE;
    }

    return NPC_TALK_STATE_TALKING;
}

static void EnZl4_UpdateFace(EnZl4* this) {
    if (this->blinkTimer > 0) {
        this->blinkTimer--;
    } else {
        this->blinkTimer = 0;
    }
    if (this->blinkTimer <= 2) {
        this->leftEyeState = this->rightEyeState = this->blinkTimer;
    }
    switch (this->eyeExpression) {
        case ZL4_EYES_NEUTRAL:
            if (this->blinkTimer == 0) {
                this->blinkTimer = Rand_S16Offset(30, 30);
            }
            break;
        case ZL4_EYES_SHUT:
            if (this->blinkTimer == 0) {
                this->leftEyeState = this->rightEyeState = ZL4_EYE_SHUT;
            }
            break;
        case ZL4_EYES_LOOK_LEFT:
            if (this->blinkTimer == 0) {
                this->leftEyeState = ZL4_EYE_LOOK_OUT;
                this->rightEyeState = ZL4_EYE_LOOK_IN;
            }
            break;
        case ZL4_EYES_LOOK_RIGHT:
            if (this->blinkTimer == 0) {
                this->leftEyeState = ZL4_EYE_LOOK_IN;
                this->rightEyeState = ZL4_EYE_LOOK_OUT;
            }
            break;
        case ZL4_EYES_WIDE:
            if (this->blinkTimer == 0) {
                this->leftEyeState = this->rightEyeState = ZL4_EYE_WIDE;
            }
            break;
        case ZL4_EYES_SQUINT:
            if (this->blinkTimer == 0) {
                this->leftEyeState = this->rightEyeState = ZL4_EYE_SQUINT;
            }
            break;
        case ZL4_EYES_OPEN:
            if (this->blinkTimer >= 3) {
                this->blinkTimer = ZL4_EYE_OPEN;
            }
            break;
    }
    switch (this->mouthExpression) {
        case ZL4_MOUTH_HAPPY:
            this->mouthState = 1;
            break;
        case ZL4_MOUTH_WORRIED:
            this->mouthState = 2;
            break;
        case ZL4_MOUTH_SURPRISED:
            this->mouthState = 3;
            break;
        default:
            this->mouthState = 0;
            break;
    }
}

static void EnZl4_SetMove(EnZl4* this, PlayState* play) {
    this->skelAnime.movementFlags |= 1;
    AnimTaskQueue_AddActorMovement(play, &this->actor, &this->skelAnime, 1.0f);
}

static void func_80B5BB78(EnZl4* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->interactInfo.trackPos = player->actor.world.pos;
    Npc_TrackPoint(&this->actor, &this->interactInfo, 2, NPC_TRACKING_HEAD_AND_TORSO);
}

static void EnZl4_GetActionStartPos(CsCmdActorCueOot* action, Vec3f* vec) {
    vec->x = action->startPos.x;
    vec->y = action->startPos.y;
    vec->z = action->startPos.z;
}

static s32 EnZl4_SetupFromLegendCs(EnZl4* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Actor* playerx = &GET_PLAYER(play)->actor;
    s16 rotY;

    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
    playerx->world.pos = this->actor.world.pos;
    rotY = this->actor.shape.rot.y;
    playerx->world.pos.x += 56.0f * Math_SinS(rotY);
    playerx->world.pos.z += 56.0f * Math_CosS(rotY);

    player->speedXZ = playerx->speed = 0.0f;

    EnZl4_SetCsCameraMove(play, 5);
    ShrinkWindow_Letterbox_SetSizeTarget(0x20);
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE_ALT);
    this->talkTimer2 = 0;
    return true;
}

static s32 EnZl4_InMovingAnim(EnZl4* this) {
    if ((this->skelAnime.animation == &gChildZeldaAnim_01B874) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01BCF0) ||
        (this->skelAnime.animation == &gChildZeldaAnim_0125E4) ||
        (this->skelAnime.animation == &gChildZeldaAnim_012E58) ||
        (this->skelAnime.animation == &gChildZeldaAnim_015F14) ||
        (this->skelAnime.animation == &gChildZeldaAnim_0169B4) ||
        (this->skelAnime.animation == &gChildZeldaAnim_016D08) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01805C) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01A2FC) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01AAE0) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01CE08) ||
        (this->skelAnime.animation == &gChildZeldaAnim_018898) ||
        (this->skelAnime.animation == &gChildZeldaAnim_01910C) ||
        (this->skelAnime.animation == &gChildZeldaAnim_00F0A4) ||
        (this->skelAnime.animation == &gChildZeldaAnim_00F894)) {
        return true;
    }
    return false;
}

static void EnZl4_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnZl4* this = (EnZl4*)thisx;

    SkelAnime_InitFlex(play, &this->skelAnime, &gChildZeldaSkel, NULL, this->jointTable, this->morphTable, 18);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);
    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_21);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.attentionRangeType = 6;
    this->actor.textId = -1;
    this->eyeExpression = this->mouthExpression = ZL4_MOUTH_NEUTRAL;

    if (gSaveContext.sceneLayer >= 4) {
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
        this->actionFunc = EnZl4_TheEnd;
    } else if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
        this->actionFunc = EnZl4_Idle;
    } else {
        if (gSaveContext.save.entrance != Hyrule_EntranceFromOot(0x5F0 /* ENTR_CASTLE_COURTYARD_ZELDA_1 */)) {
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_21);
            this->csState = ZL4_CS_WAIT;
            this->talkState = 0;
        } else {
            EnZl4_SetupFromLegendCs(this, play);
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
            this->csState = ZL4_CS_LEGEND;
            this->talkState = 0;
        }

        this->actionFunc = EnZl4_Cutscene;
    }
}

static void EnZl4_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    EnZl4* this = (EnZl4*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 EnZl4_SetNextAnim(EnZl4* this, s32 nextAnim) {
    if (!Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
        return false;
    }
    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, nextAnim);
    return true;
}

static void EnZl4_ReverseAnimation(EnZl4* this) {
    f32 tempFrame = this->skelAnime.startFrame;

    this->skelAnime.startFrame = this->skelAnime.endFrame;
    this->skelAnime.curFrame = this->skelAnime.endFrame;
    this->skelAnime.endFrame = tempFrame;
    this->skelAnime.playSpeed = -1.0f;
}

static s32 EnZl4_CsWaitForPlayer(EnZl4* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Actor* playerx = &GET_PLAYER(play)->actor;
    s16 rotY;
    s16 yawDiff;
    s16 absYawDiff;

    if (!Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        yawDiff = (f32)this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
        absYawDiff = ABS(yawDiff);
        if ((playerx->world.pos.y != this->actor.world.pos.y) || (absYawDiff >= 0x3FFC)) {
            return false;
        } else {
            Actor_OfferTalk(&this->actor, play, this->collider.dim.radius + 60.0f);
            return false;
        }
    }

    playerx->world.pos = this->actor.world.pos;
    rotY = this->actor.shape.rot.y;
    playerx->world.pos.x += 56.0f * Math_SinS(rotY);
    playerx->world.pos.z += 56.0f * Math_CosS(rotY);
    playerx->speed = 0.0f;
    player->speedXZ = 0.0f;
    return true;
}

static s32 EnZl4_CsMeetPlayer(EnZl4* this, PlayState* play) {
    switch (this->talkState) {
        case 0:
            if (this->skelAnime.curFrame == 50.0f) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_MEET);
            }
            if (!EnZl4_SetNextAnim(this, ZL4_ANIM_4)) {
                break;
            } else {
                Message_StartTextbox(play, TEXT_OOT(0x702E), NULL);
                this->talkState++;
            }
            break;
        case 1:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 1);
                Message_StartTextbox(play, TEXT_OOT(0x702F), NULL);
                this->talkTimer2 = 0;
                this->talkState++;
            }
            break;
        case 2:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                gCsCtxOot.segment = Lib_SegmentedToVirtual(gZeldasCourtyardMeetCs);
                Hyrule_TriggerCutsceneOot(1);
                EnZl4_SetCsCameraMove(play, 0);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkTimer2 = 0;
                this->talkState++;
            }
            break;
        case 3:
            this->talkTimer2++;
            if (this->talkTimer2 >= 45) {
                Message_StartTextbox(play, TEXT_OOT(0x70F9), NULL);
                this->talkState++;
            }
            break;
        case 4:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraMove(play, 1);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkTimer2 = 0;
                this->talkState++;
            }
            break;
        case 5:
            this->talkTimer2++;
            if (this->talkTimer2 >= 10) {
                Message_StartTextbox(play, TEXT_OOT(0x70FA), NULL);
                this->talkState++;
            }
            break;
        case 6:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 2);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_22);
                this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                this->talkTimer2 = 0;
                this->talkState++;
                Message_StartTextbox(play, TEXT_OOT(0x70FB), NULL);
            }
            break;
    }
    return (this->talkState == 7) ? 1 : 0;
}

static s32 EnZl4_CsAskStone(EnZl4* this, PlayState* play) {
    switch (this->talkState) {
        case 0:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_25)) {
                this->talkState++;
            }
        case 1:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 3);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkTimer1 = 40;
                this->talkState = 2;
            }
            break;
        case 2:
            if (DECR(this->talkTimer1) == 0) {
                Message_StartTextbox(play, TEXT_OOT(0x7030), NULL);
                this->talkState++;
            }
            break;
        case 3:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play))) {
                break;
            } else if (play->msgCtx.choiceIndex == 0) {
                EnZl4_SetCsCameraAngle(play, 4);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_28);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_SQUINT;
                this->mouthExpression = ZL4_MOUTH_HAPPY;
                Message_StartTextbox(play, TEXT_OOT(0x7032), NULL);
                this->talkState = 7;
            } else {
                EnZl4_SetCsCameraAngle(play, 2);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_9);
                this->mouthExpression = ZL4_MOUTH_WORRIED;
                Message_StartTextbox(play, TEXT_OOT(0x7031), NULL);
                this->talkState++;
            }
            break;
        case 4:
            if (this->skelAnime.curFrame == 16.0f) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_QUESTION);
            }
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_10)) {
                this->talkState++;
            }
        case 5:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_9);
                this->mouthExpression = ZL4_MOUTH_WORRIED;
                EnZl4_ReverseAnimation(this);
                this->talkState = 6;
            }
            break;
        case 6:
            this->mouthExpression = ZL4_MOUTH_NEUTRAL;
            EnZl4_SetCsCameraAngle(play, 3);
            Message_StartTextbox(play, TEXT_OOT(0x7030), NULL);
            this->talkState = 12;
            break;
        case 12:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_25)) {
                this->talkState = 13;
            }
        case 13:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play))) {
                break;
            } else if (play->msgCtx.choiceIndex == 0) {
                EnZl4_SetCsCameraAngle(play, 4);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_28);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_SQUINT;
                this->mouthExpression = ZL4_MOUTH_HAPPY;
                Message_StartTextbox(play, TEXT_OOT(0x7032), NULL);
                this->talkState = 7;
            } else {
                EnZl4_SetCsCameraAngle(play, 2);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_9);
                this->mouthExpression = ZL4_MOUTH_WORRIED;
                Message_StartTextbox(play, TEXT_OOT(0x7031), NULL);
                this->talkState = 4;
            }
            break;
        case 7:
            if (this->skelAnime.curFrame == 17.0f) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_SMILE_0);
            }
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_29)) {
                this->talkState++;
            }
        case 8:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraMove(play, 2);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                Message_StartTextbox(play, TEXT_OOT(0x70FC), NULL);
                this->talkState = 9;
            }
            break;
        case 9:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 5);
                Message_StartTextbox(play, TEXT_OOT(0x70FD), NULL);
                this->talkState++;
            }
            break;
        case 10:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_5);
                this->eyeExpression = ZL4_EYES_OPEN;
                this->mouthExpression = ZL4_MOUTH_SURPRISED;
                Message_StartTextbox(play, TEXT_OOT(0x70FE), NULL);
                this->talkState++;
            }
            break;
    }
    return (this->talkState == 11) ? 1 : 0;
}

static s32 EnZl4_CsAskName(EnZl4* this, PlayState* play) {
    switch (this->talkState) {
        case 0:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_4)) {
                this->talkState++;
            }
            break;
        case 1:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 6);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_1);
                this->blinkTimer = 11;
                this->eyeExpression = ZL4_EYES_SQUINT;
                this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                Message_StartTextbox(play, TEXT_OOT(0x70FF), NULL);
                this->talkState++;
            }
            break;
        case 2:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_2)) {
                this->talkState++;
            }
        case 3:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_16);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState = 4;
            }
            break;
        case 4:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_17)) {
                Message_StartTextbox(play, TEXT_OOT(0x2073), NULL);
                this->talkState++;
            }
            break;
        case 5:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraMove(play, 3);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkTimer2 = 0;
                this->talkState = 6;
            }
            break;
        case 6:
            this->talkTimer2++;
            if (this->talkTimer2 >= 15) {
                Message_StartTextbox(play, TEXT_OOT(0x2074), NULL);
                this->talkState++;
            }
            break;
        case 7:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_6);
                this->mouthExpression = ZL4_MOUTH_HAPPY;
                Message_StartTextbox(play, TEXT_OOT(0x2075), NULL);
                this->talkState++;
            }
            break;
        case 8:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_25)) {
                this->talkState++;
            }
        case 9:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x7033), NULL);
                this->talkState = 10;
            }
            break;
        case 10:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
                if (play->msgCtx.choiceIndex == 0) {
                    EnZl4_SetCsCameraMove(play, 4);
                    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_33);
                    this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                    play->msgCtx.msgMode = MSGMODE_PAUSED;
                    this->talkTimer2 = 0;
                    this->talkState = 15;
                } else {
                    EnZl4_SetCsCameraAngle(play, 6);
                    play->msgCtx.msgMode = MSGMODE_PAUSED;
                    this->talkTimer1 = 20;
                    this->talkState++;
                    this->skelAnime.playSpeed = 0.0f;
                }
            }
            break;
        case 11:
            if (DECR(this->talkTimer1) == 0) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_11);
                this->blinkTimer = 11;
                this->eyeExpression = ZL4_EYES_LOOK_RIGHT;
                this->mouthExpression = ZL4_MOUTH_WORRIED;
                Message_StartTextbox(play, TEXT_OOT(0x7034), NULL);
                this->talkState++;
            }
            break;
        case 12:
            if (this->skelAnime.curFrame == 5.0f) {
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_SIGH_0);
            }
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_12)) {
                this->talkState++;
            }
        case 13:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_6);
                this->blinkTimer = 3;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_HAPPY;
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState = 14;
            }
            break;
        case 14:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_25)) {
                Message_StartTextbox(play, TEXT_OOT(0x7033), NULL);
                this->talkState = 10;
            }
            break;
        case 15:
            this->talkTimer2++;
            if (this->talkTimer2 >= 30) {
                Message_StartTextbox(play, TEXT_OOT(0x7035), NULL);
                this->talkState++;
            }
            break;
        case 16:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
        case 17:
            this->talkTimer2++;
            if (this->talkTimer2 == 130) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                gSaveContext.nextCutsceneIndex = 0xFFF7;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
            }
            break;
    }
    if ((this->talkTimer2 == 17) && (this->talkTimer2 > 130)) {
        return true;
    }
    return false;
}

static s32 EnZl4_CsTellLegend(EnZl4* this, PlayState* play) {
    Camera* activeCam = GET_ACTIVE_CAM(play);

    switch (this->talkState) {
        case 0:
            this->talkTimer2++;
            if (this->talkTimer2 >= 60) {
                Message_StartTextbox(play, TEXT_OOT(0x7037), NULL);
                this->talkState++;
            }
            break;
        case 1:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 7);
                Message_StartTextbox(play, TEXT_OOT(0x2076), NULL);
                this->talkState++;
            }
            break;
        case 2:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraMove(play, 6);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
            break;
        case 3:
            if (activeCam->animState == 2) {
                Message_StartTextbox(play, TEXT_OOT(0x2077), NULL);
                this->talkState++;
            }
            break;
        case 4:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play))) {
                break;
            } else if (play->msgCtx.choiceIndex == 0) {
                EnZl4_SetCsCameraAngle(play, 8);
                Message_StartTextbox(play, TEXT_OOT(0x7005), NULL);
                this->talkState = 9;
            } else {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_5);
                this->mouthExpression = ZL4_MOUTH_SURPRISED;
                Message_StartTextbox(play, TEXT_OOT(0x7038), NULL);
                this->talkState++;
                Actor_PlaySfx(&this->actor, NA_SE_VO_Z0_HURRY);
            }
            break;
        case 5:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_4)) {
                this->talkState++;
            }
        case 6:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_33);
                this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                Message_StartTextbox(play, TEXT_OOT(0x7037), NULL);
                this->talkState++;
            }
            break;
        case 7:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x2076), NULL);
                this->talkState++;
            }
            break;
        case 8:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x2077), NULL);
                this->talkState = 4;
            }
            break;
        case 9:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_26);
                Message_StartTextbox(play, TEXT_OOT(0x2078), NULL);
                this->talkState++;
            }
            break;
        case 10:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_27)) {
                this->talkState++;
            }
        case 11:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play))) {
                break;
            } else if (play->msgCtx.choiceIndex == 0) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState = 13;
            } else {
                Message_StartTextbox(play, TEXT_OOT(0x700B), NULL);
                this->talkState = 12;
            }
            break;
        case 12:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState = 13;
            }
            break;
    }
    return (this->talkState == 13) ? 1 : 0;
}

static s32 EnZl4_CsLookWindow(EnZl4* this, PlayState* play) {
    switch (this->talkState) {
        case 0:
            EnZl4_SetCsCameraMove(play, 7);
            gCsCtxOot.segment = Lib_SegmentedToVirtual(gZeldasCourtyardWindowCs);
            Hyrule_TriggerCutsceneOot(1);
            this->talkState++;
            break;
        case 1:
            if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
                if (gCsCtxOot.frames == 90) {
                    gCsCtxOot.state = CS_STATE_UNSKIPPABLE_INIT_Oot;
                }
            } else {
                gCsCtxOot.segment = Lib_SegmentedToVirtual(gZeldasCourtyardGanonCs);
                Hyrule_TriggerCutsceneOot(1);
                this->talkState++;
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
            }
            break;
        case 2:
            if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
                if (gCsCtxOot.frames == 209) {
                    gCsCtxOot.state = CS_STATE_UNSKIPPABLE_INIT_Oot;
                }
            } else {
                Rumble_Request(0.0f, 0xA0, 0xA, 0x28);
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_30);
                EnZl4_SetCsCameraAngle(play, 11);
                Message_StartTextbox(play, TEXT_OOT(0x7039), NULL);
                this->talkState++;
            }
            break;
        case 3:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
            break;
    }
    return (this->talkState == 4) ? 1 : 0;
}

static s32 EnZl4_CsWarnAboutGanon(EnZl4* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 rotY;

    switch (this->talkState) {
        case 0:
            player->actor.world.pos = this->actor.world.pos;
            rotY = this->actor.shape.rot.y - 0x3FFC;
            player->actor.world.pos.x += 34.0f * Math_SinS(rotY);
            player->actor.world.pos.z += 34.0f * Math_CosS(rotY);
            EnZl4_SetCsCameraMove(play, 8);
            this->blinkTimer = 0;
            this->eyeExpression = ZL4_EYES_WIDE;
            this->mouthExpression = ZL4_MOUTH_WORRIED;
            this->talkTimer2 = 0;
            this->talkState++;
            Message_StartTextbox(play, TEXT_OOT(0x2079), NULL);
        case 1:
            this->talkTimer2++;
            if (this->talkTimer2 >= 20) {
                this->talkState++;
            }
            break;
        case 2:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraMove(play, 9);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkTimer2 = 0;
                this->talkState++;
            }
            break;
        case 3:
            this->talkTimer2++;
            if (this->talkTimer2 >= 20) {
                Message_StartTextbox(play, TEXT_OOT(0x207A), NULL);
                this->talkState++;
            }
            break;
        case 4:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 12);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_23);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_SURPRISED;
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
            break;
        case 5:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_24)) {
                Message_StartTextbox(play, TEXT_OOT(0x207B), NULL);
                this->talkState++;
            }
            break;
        case 6:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x703A), NULL);
                this->talkState++;
            }
            break;
        case 7:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play))) {
                break;
            } else if (play->msgCtx.choiceIndex == 0) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_31);
                this->blinkTimer = 11;
                this->eyeExpression = ZL4_EYES_SQUINT;
                this->mouthExpression = ZL4_MOUTH_HAPPY;
                Message_StartTextbox(play, TEXT_OOT(0x703B), NULL);
                this->talkState = 11;
            } else {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_13);
                this->blinkTimer = 11;
                this->eyeExpression = ZL4_EYES_LOOK_LEFT;
                this->mouthExpression = ZL4_MOUTH_WORRIED;
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
            break;
        case 8:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_15)) {
                this->blinkTimer = 3;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_SURPRISED;
                Message_StartTextbox(play, TEXT_OOT(0x7073), NULL);
                this->talkState++;
            }
            break;
        case 9:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_14);
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState++;
            }
            break;
        case 10:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_24)) {
                Message_StartTextbox(play, TEXT_OOT(0x703A), NULL);
                this->talkState = 7;
            }
            break;
        case 11:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_32)) {
                this->talkState++;
            }
        case 12:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                play->msgCtx.msgMode = MSGMODE_PAUSED;
                this->talkState = 13;
            }
            break;
    }
    return (this->talkState == 13) ? 1 : 0;
}

static s32 EnZl4_CsMakePlan(EnZl4* this, PlayState* play) {
    switch (this->talkState) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_18);
            this->blinkTimer = 0;
            this->eyeExpression = ZL4_EYES_NEUTRAL;
            this->mouthExpression = ZL4_MOUTH_WORRIED;
            EnZl4_SetCsCameraMove(play, 10);
            this->talkTimer2 = 0;
            this->talkState++;
        case 1:
            this->talkTimer2++;
            if (this->talkTimer2 >= 10) {
                Message_StartTextbox(play, TEXT_OOT(0x7123), NULL);
                this->talkState++;
            }
            break;
        case 2:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                EnZl4_SetCsCameraAngle(play, 13);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_19);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_SURPRISED;
                Message_StartTextbox(play, TEXT_OOT(0x207C), NULL);
                this->talkState++;
            }
            break;
        case 3:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_20)) {
                this->talkState++;
            }
        case 4:
            if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
                Message_StartTextbox(play, TEXT_OOT(0x207D), NULL);
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_7);
                this->blinkTimer = 0;
                this->eyeExpression = ZL4_EYES_NEUTRAL;
                this->mouthExpression = ZL4_MOUTH_NEUTRAL;
                this->talkState = 5;
                this->unk_20F = this->lastAction = 0;
            }
            break;
        case 5:
            if (EnZl4_SetNextAnim(this, ZL4_ANIM_8)) {
                this->talkState++;
            }
        case 6:
            if (!((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play))) {
                break;
            } else {
                Camera_ChangeSetting(GET_ACTIVE_CAM(play), 1);
                this->talkState = 7;
                play->talkWithPlayer(play, &this->actor);
                if ((true)) {
                    Actor_OfferGetItem(&this->actor, play, GI_LETTER_ZELDA_Oot, fabsf(this->actor.xzDistToPlayer) + 1.0f,
                                       fabsf(this->actor.playerHeightRel) + 1.0f);
                }
                play->msgCtx.stateTimer = 4;
                play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
            }
            break;
        case 7:
            if (Actor_HasParent(&this->actor, play) || !(true)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_0);
                this->talkState++;
            } else {
                Actor_OfferGetItem(&this->actor, play, GI_LETTER_ZELDA_Oot, fabsf(this->actor.xzDistToPlayer) + 1.0f,
                                   fabsf(this->actor.playerHeightRel) + 1.0f);
            }
            // no break here is required for matching
    }
    return (this->talkState == 8) ? 1 : 0;
}

static void EnZl4_Cutscene(EnZl4* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    switch (this->csState) {
        case ZL4_CS_WAIT:
            if (EnZl4_CsWaitForPlayer(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_START:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_3);
            this->blinkTimer = 0;
            this->eyeExpression = ZL4_EYES_NEUTRAL;
            this->mouthExpression = ZL4_MOUTH_SURPRISED;
            Audio_PlayFanfare(SEQ_OOT(0x51)); // OoT's NA_BGM_APPEAR
            EnZl4_SetCsCameraAngle(play, 0);
            Interface_SetHudVisibility(HUD_VISIBILITY_NONE_ALT);
            ShrinkWindow_Letterbox_SetSizeTarget(0x20);
            this->talkState = 0;
            this->csState++;
            break;
        case ZL4_CS_MEET:
            if (EnZl4_CsMeetPlayer(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_STONE:
            if (EnZl4_CsAskStone(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_NAMES:
            if (EnZl4_CsAskName(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_LEGEND:
            if (EnZl4_CsTellLegend(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_WINDOW:
            if (EnZl4_CsLookWindow(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_GANON:
            if (EnZl4_CsWarnAboutGanon(this, play)) {
                this->talkState = 0;
                this->csState++;
            }
            break;
        case ZL4_CS_PLAN:
            if (EnZl4_CsMakePlan(this, play)) {
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
                gSaveContext.prevHudVisibility = 0x32;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot);
                this->actionFunc = EnZl4_Idle;
            }
            break;
    }
    this->interactInfo.trackPos = player->actor.world.pos;
    Npc_TrackPoint(&this->actor, &this->interactInfo, 2,
                   (this->csState == ZL4_CS_WINDOW) ? NPC_TRACKING_HEAD_AND_TORSO : NPC_TRACKING_NONE);
    if (EnZl4_InMovingAnim(this)) {
        EnZl4_SetMove(this, play);
    }
}

static void EnZl4_Idle(EnZl4* this, PlayState* play) {
    Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState, this->collider.dim.radius + 60.0f,
                      EnZl4_GetText, func_80B5B9B0);
    func_80B5BB78(this, play);
}

static void EnZl4_TheEnd(EnZl4* this, PlayState* play) {
    s32 animIndex[] = { ZL4_ANIM_0, ZL4_ANIM_0, ZL4_ANIM_0,  ZL4_ANIM_0,  ZL4_ANIM_0,
                        ZL4_ANIM_0, ZL4_ANIM_0, ZL4_ANIM_26, ZL4_ANIM_21, ZL4_ANIM_3 };
    CsCmdActorCueOot* npcAction;
    Vec3f pos;

    if (SkelAnime_Update(&this->skelAnime) && (this->skelAnime.animation == &gChildZeldaAnim_010DF8)) {
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ZL4_ANIM_4);
    }
    if (EnZl4_InMovingAnim(this)) {
        EnZl4_SetMove(this, play);
    }
    if (gCsCtxOot.frames == 100) {
        this->eyeExpression = ZL4_EYES_LOOK_LEFT;
    }
    if (gCsCtxOot.frames == 450) {
        this->blinkTimer = 3;
        this->eyeExpression = ZL4_EYES_NEUTRAL;
        this->mouthExpression = ZL4_MOUTH_SURPRISED;
    }
    npcAction = gCsCtxOot.npcActions[0];
    if (npcAction != NULL) {
        EnZl4_GetActionStartPos(npcAction, &pos);
        if (this->lastAction == 0) {
            this->actor.world.pos = this->actor.home.pos = pos;
        }
        if (this->lastAction != npcAction->action) {
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, animIndex[npcAction->action]);
            this->lastAction = npcAction->action;
        }
        this->actor.velocity.x = 0.0f;
        this->actor.velocity.y = 0.0f;
        this->actor.velocity.z = 0.0f;
    }
}

static void EnZl4_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnZl4* this = (EnZl4*)thisx;

    if (this->actionFunc != EnZl4_TheEnd) {
        SkelAnime_Update(&this->skelAnime);
    }
    EnZl4_UpdateFace(this);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    this->actionFunc(this, play);
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static s32 EnZl4_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnZl4* this = (EnZl4*)thisx;
    Vec3s sp1C;

    if (limbIndex == 17) {
        sp1C = this->interactInfo.headRot;
        Matrix_Translate(900.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        Matrix_RotateXFApply((sp1C.y / (f32)0x8000) * M_PI);
        Matrix_RotateZF((sp1C.x / (f32)0x8000) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-900.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == 10) {
        sp1C = this->interactInfo.torsoRot;
        Matrix_RotateYF((sp1C.y / (f32)0x8000) * M_PI, MTXMODE_APPLY);
        Matrix_RotateXFApply((sp1C.x / (f32)0x8000) * M_PI);
    }
    if ((limbIndex >= 3) && (limbIndex < 7)) {
        *dList = NULL;
    }
    return false;
}

static void EnZl4_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    EnZl4* this = (EnZl4*)thisx;

    if (limbIndex == 17) {
        Matrix_MultVec3f(&zeroVec, &this->actor.focus.pos);
    }
}

static void EnZl4_Draw(Actor* thisx, PlayState* play) {
    EnZl4* this = (EnZl4*)thisx;
    void* mouthTex[] = { gChildZeldaMouthNeutralTex, gChildZeldaMouthHappyTex, gChildZeldaMouthWorriedTex,
                         gChildZeldaMouthSurprisedTex };
    void* eyeTex[] = {
        gChildZeldaEyeOpenTex,   gChildZeldaEyeBlinkTex, gChildZeldaEyeShutTex, gChildZeldaEyeWideTex,
        gChildZeldaEyeSquintTex, gChildZeldaEyeOutTex,   gChildZeldaEyeInTex,
    };

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTex[this->rightEyeState]));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(eyeTex[this->leftEyeState]));
    gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(mouthTex[this->mouthState]));
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnZl4_OverrideLimbDraw, EnZl4_PostLimbDraw, this);
    CLOSE_DISPS(play->state.gfxCtx);
}
