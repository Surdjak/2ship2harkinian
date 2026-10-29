/*
 * File: z_en_md.c
 * Overlay: ovl_En_Md
 * Description: Mido
 */

#include "z_en_md.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleOcarina.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_md/object_md.h"

#define FLAGS                                                                                  \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_UPDATE_DURING_OCARINA)

static void EnMd_Init(Actor* thisx, PlayState* play);
static void EnMd_Destroy(Actor* thisx, PlayState* play);
static void EnMd_Update(Actor* thisx, PlayState* play);
static void EnMd_Draw(Actor* thisx, PlayState* play);

static void EnMd_Idle(EnMd* this, PlayState* play);
static void EnMd_Watch(EnMd* this, PlayState* play);
static void EnMd_BlockPath(EnMd* this, PlayState* play);
static void EnMd_ListenToOcarina(EnMd* this, PlayState* play);
static void EnMd_Walk(EnMd* this, PlayState* play);

ActorProfile En_Md_Oot_Profile = {
    /**/ ACTOR_EN_MD_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_MD_Oot,
    /**/ sizeof(EnMd),
    /**/ EnMd_Init,
    /**/ EnMd_Destroy,
    /**/ EnMd_Update,
    /**/ EnMd_Draw,
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
    { 36, 46, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

typedef enum {
    /*  0 */ ENMD_ANIM_0,
    /*  1 */ ENMD_ANIM_1,
    /*  2 */ ENMD_ANIM_2,
    /*  3 */ ENMD_ANIM_3,
    /*  4 */ ENMD_ANIM_4,
    /*  5 */ ENMD_ANIM_5,
    /*  6 */ ENMD_ANIM_6,
    /*  7 */ ENMD_ANIM_7,
    /*  8 */ ENMD_ANIM_8,
    /*  9 */ ENMD_ANIM_9,
    /* 10 */ ENMD_ANIM_10,
    /* 11 */ ENMD_ANIM_11,
    /* 12 */ ENMD_ANIM_12,
    /* 13 */ ENMD_ANIM_13,
} EnMdAnimation;

static AnimationInfo sAnimationInfo[] = {
    { &gMidoHandsOnHipsIdleAnim, 0.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMidoHandsOnHipsIdleAnim, 0.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -10.0f },
    { &gMidoRaiseHand1Anim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gMidoHaltAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gMidoPutHandDownAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gMidoAnnoyedPointedHeadIdle1Anim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gMidoAnnoyedPointedHeadIdle2Anim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gMidoAnim_92B0, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gMidoWalkingAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gMidoHandsOnHipsTransitionAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gMidoHandsOnHipsIdleAnim, 0.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -8.0f },
    { &gMidoSlamAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gMidoRaiseHand2Anim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gMidoAngryHeadTurnAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
};

static void func_80AAA250(EnMd* this) {
    f32 startFrame;

    startFrame = this->skelAnime.startFrame;
    this->skelAnime.startFrame = this->skelAnime.endFrame;
    this->skelAnime.curFrame = this->skelAnime.endFrame;
    this->skelAnime.endFrame = startFrame;
    this->skelAnime.playSpeed = -1.0f;
}

static void func_80AAA274(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_2);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_3);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA308(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_4);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_5);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA39C(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_2);
            func_80AAA250(this);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_7);
                this->animSequenceEntry++;
            } else {
                break;
            }
        case 2:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_8);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA474(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_7);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_10);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA508(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_2);
            func_80AAA250(this);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_10);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA5A4(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_9);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_6);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA638(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_9);
            func_80AAA250(this);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_10);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA6D4(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_11);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_6);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA768(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_12);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_3);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA7FC(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_13);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_6);
                this->animSequenceEntry++;
            }
    }
}

static void func_80AAA890(EnMd* this) {
    switch (this->animSequenceEntry) {
        case 0:
            Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_7);
            func_80AAA250(this);
            this->animSequenceEntry++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_10);
                this->animSequenceEntry++;
            }
    }
}

static void EnMd_SetAnimSequence(EnMd* this, u8 arg1) {
    this->animSequence = arg1;
    this->animSequenceEntry = 0;
}

static void EnMd_UpdateAnimSequence(EnMd* this) {
    switch (this->animSequence) {
        case 1:
            func_80AAA274(this);
            break;
        case 2:
            func_80AAA308(this);
            break;
        case 3:
            func_80AAA39C(this);
            break;
        case 4:
            func_80AAA474(this);
            break;
        case 5:
            func_80AAA508(this);
            break;
        case 6:
            func_80AAA5A4(this);
            break;
        case 7:
            func_80AAA638(this);
            break;
        case 8:
            func_80AAA6D4(this);
            break;
        case 9:
            func_80AAA768(this);
            break;
        case 10:
            func_80AAA7FC(this);
            break;
        case 11:
            func_80AAA890(this);
    }
}

static void EnMd_UpdateAnimSequence_WithTalking(EnMd* this) {
    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        switch (this->actor.textId) {
            case TEXT_OOT(0x102F):
                if ((this->messageEntry == 0) && (this->animSequence != 1)) {
                    EnMd_SetAnimSequence(this, 1);
                }
                if ((this->messageEntry == 2) && (this->animSequence != 2)) {
                    EnMd_SetAnimSequence(this, 2);
                }
                if ((this->messageEntry == 5) && (this->animSequence != 8)) {
                    EnMd_SetAnimSequence(this, 8);
                }
                if ((this->messageEntry == 11) && (this->animSequence != 9)) {
                    EnMd_SetAnimSequence(this, 9);
                }
                break;
            case TEXT_OOT(0x1033):
                if ((this->messageEntry == 0) && (this->animSequence != 1)) {
                    EnMd_SetAnimSequence(this, 1);
                }
                if ((this->messageEntry == 1) && (this->animSequence != 2)) {
                    EnMd_SetAnimSequence(this, 2);
                }
                if ((this->messageEntry == 5) && (this->animSequence != 10)) {
                    EnMd_SetAnimSequence(this, 10);
                }
                if ((this->messageEntry == 7) && (this->animSequence != 9)) {
                    EnMd_SetAnimSequence(this, 9);
                }
                break;
            case TEXT_OOT(0x1030):
            case TEXT_OOT(0x1034):
            case TEXT_OOT(0x1045):
                if ((this->messageEntry == 0) && (this->animSequence != 1)) {
                    EnMd_SetAnimSequence(this, 1);
                }
                break;
            case TEXT_OOT(0x1046):
                if ((this->messageEntry == 0) && (this->animSequence != 6)) {
                    EnMd_SetAnimSequence(this, 6);
                }
                break;
        }
    } else if (this->skelAnime.animation != &gMidoHandsOnHipsIdleAnim) {
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_10);
        EnMd_SetAnimSequence(this, 0);
    }

    EnMd_UpdateAnimSequence(this);
}

static s16 func_80AAAC78(EnMd* this, PlayState* play) {
    s16 dialogState = Message_GetState(&play->msgCtx);

    if ((this->messageState == TEXT_STATE_AWAITING_NEXT) || (this->messageState == TEXT_STATE_EVENT) ||
        (this->messageState == TEXT_STATE_CLOSING) || (this->messageState == TEXT_STATE_NEXT)) {
        if (this->messageState != dialogState) {
            this->messageEntry++;
        }
    }

    this->messageState = dialogState;
    return dialogState;
}

static u16 EnMd_GetTextKokiriForest(PlayState* play, EnMd* this) {
    u16 reactionText = Text_GetFaceReactionOot(play, 0x11);

    if (reactionText != 0) {
        return reactionText;
    }

    this->messageEntry = 0;
    this->messageState = TEXT_STATE_NONE;

    if ((CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot))) {
        return TEXT_OOT(0x1045);
    }

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_SHOWED_MIDO_SWORD_SHIELD_Oot)) {
        return TEXT_OOT(0x1034);
    }

    // PORT: OoT checks that Link wears the Deku Shield (EQUIP_VALUE_SHIELD_DEKU_Oot). It can be owned now, but not
    // worn until the pause menu's equipment page exists, so any shield with the Kokiri Sword will do.
    if ((GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) != EQUIP_VALUE_SHIELD_NONE) &&
        (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_KOKIRI)) {
        return TEXT_OOT(0x1033);
    }

    if (GET_INFTABLE_OOT(INFTABLE_0C_Oot)) {
        return TEXT_OOT(0x1030);
    }

    return TEXT_OOT(0x102F);
}

static u16 EnMd_GetTextKokiriHome(PlayState* play, EnMd* this) {
    this->messageEntry = 0;
    this->messageState = TEXT_STATE_NONE;

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
        return TEXT_OOT(0x1028);
    }

    return TEXT_OOT(0x1046);
}

static u16 EnMd_GetTextLostWoods(PlayState* play, EnMd* this) {
    this->messageEntry = 0;
    this->messageState = TEXT_STATE_NONE;

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP_Oot)) {
        if (GET_INFTABLE_OOT(INFTABLE_19_Oot)) {
            return TEXT_OOT(0x1071);
        }
        return TEXT_OOT(0x1070);
    }

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SARIAS_SONG_FOR_MIDO_AS_ADULT_Oot)) {
        return TEXT_OOT(0x1068);
    }

    if (GET_INFTABLE_OOT(INFTABLE_15_Oot)) {
        return TEXT_OOT(0x1061);
    }

    return TEXT_OOT(0x1060);
}

static u16 EnMd_GetText(PlayState* play, Actor* thisx) {
    EnMd* this = (EnMd*)thisx;

    switch (play->sceneId) {
        case SCENE_KOKIRI_FOREST_Oot:
            return EnMd_GetTextKokiriForest(play, this);
        case SCENE_MIDOS_HOUSE_Oot:
            return EnMd_GetTextKokiriHome(play, this);
        case SCENE_LOST_WOODS_Oot:
            return EnMd_GetTextLostWoods(play, this);
        default:
            return 0;
    }
}

static s16 func_80AAAF04(PlayState* play, Actor* thisx) {
    EnMd* this = (EnMd*)thisx;
    switch (func_80AAAC78(this, play)) {
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
        case TEXT_STATE_FADING:
        case TEXT_STATE_CHOICE:
        case TEXT_STATE_DONE:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            return NPC_TALK_STATE_TALKING;
        case TEXT_STATE_CLOSING:
            switch (this->actor.textId) {
                case TEXT_OOT(0x1028):
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_ABOUT_SARIA_Oot);
                    break;
                case TEXT_OOT(0x102F):
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_FIRST_SPOKE_TO_MIDO_Oot);
                    SET_INFTABLE_OOT(INFTABLE_0C_Oot);
                    break;
                case TEXT_OOT(0x1060):
                    SET_INFTABLE_OOT(INFTABLE_15_Oot);
                    break;
                case TEXT_OOT(0x1070):
                    SET_INFTABLE_OOT(INFTABLE_19_Oot);
                    break;
                case TEXT_OOT(0x1033):
                case TEXT_OOT(0x1067):
                    return NPC_TALK_STATE_ACTION;
            }
            return NPC_TALK_STATE_IDLE;
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(play)) {
                return NPC_TALK_STATE_ACTION;
            }
        default:
            return NPC_TALK_STATE_TALKING;
    }
}

static u8 EnMd_ShouldSpawn(EnMd* this, PlayState* play) {
    if (play->sceneId == SCENE_KOKIRI_FOREST_Oot) {
        if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot) &&
            !GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
            return 1;
        }
    }

    if (play->sceneId == SCENE_MIDOS_HOUSE_Oot) {
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot) ||
            GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_ZELDAS_LETTER_Oot)) {
            if (!LINK_IS_ADULT_OOT) {
                return 1;
            }
        }
    }

    if (play->sceneId == SCENE_LOST_WOODS_Oot) {
        return 1;
    }

    return 0;
}

static void EnMd_UpdateEyes(EnMd* this) {
    if (DECR(this->blinkTimer) == 0) {
        this->eyeIdx++;
        if (this->eyeIdx > 2) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIdx = 0;
        }
    }
}

static void EnMd_UpdateTalking(EnMd* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 absYawDiff;
    s16 trackingMode;
    s16 canUpdateTalking;
    s16 yawDiff;

    if (this->actor.xzDistToPlayer < 170.0f) {
        yawDiff = (f32)this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
        absYawDiff = ABS(yawDiff);

        trackingMode =
            absYawDiff <= Npc_GetTrackingPresetMaxPlayerYaw(2) ? NPC_TRACKING_HEAD_AND_TORSO : NPC_TRACKING_NONE;
        canUpdateTalking = true;
    } else {
        trackingMode = NPC_TRACKING_NONE;
        canUpdateTalking = false;
    }

    if (this->interactInfo.talkState != NPC_TALK_STATE_IDLE) {
        trackingMode = NPC_TRACKING_FULL_BODY;
    }

    if (this->actionFunc == EnMd_Walk) {
        trackingMode = NPC_TRACKING_NONE;
        canUpdateTalking = false;
    }
    if (this->actionFunc == EnMd_Watch) {
        trackingMode = NPC_TRACKING_FULL_BODY;
        canUpdateTalking = true;
    }

    if ((play->csCtx.state != CS_STATE_IDLE) || gDbgCamEnabled) {
        this->interactInfo.trackPos = play->view.eye;
        this->interactInfo.yOffset = 40.0f;
        trackingMode = NPC_TRACKING_HEAD_AND_TORSO;
    } else {
        this->interactInfo.trackPos = player->actor.world.pos;
        this->interactInfo.yOffset = LINK_IS_CHILD_OOT ? 0.0f : -18.0f;
    }

    Npc_TrackPoint(&this->actor, &this->interactInfo, 2, trackingMode);
    if (this->actionFunc != EnMd_ListenToOcarina) {
        if (canUpdateTalking) {
            Npc_UpdateTalking(play, &this->actor, &this->interactInfo.talkState, this->collider.dim.radius + 30.0f,
                              EnMd_GetText, func_80AAAF04);
        }
    }
}

static u8 EnMd_FollowPath(EnMd* this, PlayState* play) {
    Path* path;
    Vec3s* pointPos;
    f32 pathDiffX;
    f32 pathDiffZ;

    if ((this->actor.params & 0xFF00) == 0xFF00) {
        return 0;
    }

    path = &play->setupPathList[(this->actor.params & 0xFF00) >> 8];
    pointPos = Lib_SegmentedToVirtual(path->points);
    pointPos += this->waypoint;

    pathDiffX = pointPos->x - this->actor.world.pos.x;
    pathDiffZ = pointPos->z - this->actor.world.pos.z;
    Math_SmoothStepToS(&this->actor.world.rot.y, Math_FAtan2F(pathDiffX, pathDiffZ) * (65536.0f / (2 * M_PI)), 4, 4000,
                       1);

    if ((SQ(pathDiffX) + SQ(pathDiffZ)) < 100.0f) {
        this->waypoint++;
        if (this->waypoint >= path->count) {
            this->waypoint = 0;
        }
        return 1;
    }
    return 0;
}

static u8 EnMd_SetMovedPos(EnMd* this, PlayState* play) {
    Path* path;
    Vec3s* lastPointPos;

    if ((this->actor.params & 0xFF00) == 0xFF00) {
        return 0;
    }

    path = &play->setupPathList[(this->actor.params & 0xFF00) >> 8];
    lastPointPos = Lib_SegmentedToVirtual(path->points);
    lastPointPos += path->count - 1;

    this->actor.world.pos.x = lastPointPos->x;
    this->actor.world.pos.y = lastPointPos->y;
    this->actor.world.pos.z = lastPointPos->z;

    return 1;
}

static void EnMd_UpdateAlphaByDistance(EnMd* this, PlayState* play) {
    f32 radius;

    if ((play->sceneId != SCENE_MIDOS_HOUSE_Oot)) {
        radius = ((CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot)) &&
                  !GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot) &&
                  (play->sceneId == SCENE_KOKIRI_FOREST_Oot))
                     ? 100.0f
                     : 400.0f;
        this->alpha = Actor_UpdateAlphaByDistance(&this->actor, play, this->alpha, radius);
        this->actor.shape.shadowAlpha = this->alpha;
    } else {
        this->alpha = 255;
        this->actor.shape.shadowAlpha = this->alpha;
    }
}

static void EnMd_Init(Actor* thisx, PlayState* play) {
    EnMd* this = (EnMd*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 24.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gMidoSkel, NULL, this->jointTable, this->morphTable, 17);

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
    if (!(EnMd_ShouldSpawn(this, play))) {
        Actor_Kill(&this->actor);
        return;
    }

    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, ENMD_ANIM_0);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.attentionRangeType = 6;
    this->alpha = 255;
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_ELF_Oot, this->actor.world.pos.x,
                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 3 /* OoT's FAIRY_KOKIRI */);

    if (((play->sceneId == SCENE_KOKIRI_FOREST_Oot) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_SHOWED_MIDO_SWORD_SHIELD_Oot)) ||
        ((play->sceneId == SCENE_KOKIRI_FOREST_Oot) && GET_EVENTCHKINF_OOT(EVENTCHKINF_SHOWED_MIDO_SWORD_SHIELD_Oot) &&
         (CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot))) ||
        ((play->sceneId == SCENE_LOST_WOODS_Oot) &&
         !GET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SARIAS_SONG_FOR_MIDO_AS_ADULT_Oot))) {
        this->actor.home.pos = this->actor.world.pos;
        this->actionFunc = EnMd_BlockPath;
        return;
    }

    if (play->sceneId != SCENE_MIDOS_HOUSE_Oot) {
        EnMd_SetMovedPos(this, play);
    }

    this->actionFunc = EnMd_Idle;
}

static void EnMd_Destroy(Actor* thisx, PlayState* play) {
    EnMd* this = (EnMd*)thisx;
    Collider_DestroyCylinder(play, &this->collider);
}

static void EnMd_Idle(EnMd* this, PlayState* play) {
    if (this->skelAnime.animation == &gMidoHandsOnHipsIdleAnim) {
        Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 17);
    } else if ((this->interactInfo.talkState == NPC_TALK_STATE_IDLE) && (this->animSequence != 7)) {
        EnMd_SetAnimSequence(this, 7);
    }

    EnMd_UpdateAnimSequence_WithTalking(this);
}

static void EnMd_Watch(EnMd* this, PlayState* play) {
    if (this->skelAnime.animation == &gMidoHandsOnHipsIdleAnim) {
        Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 17);
    }
    EnMd_UpdateAnimSequence(this);
}

static void EnMd_BlockPath(EnMd* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 temp;
    Actor* actorToBlock = &GET_PLAYER(play)->actor;
    s16 yaw;

    EnMd_UpdateAnimSequence_WithTalking(this);

    if (this->interactInfo.talkState == NPC_TALK_STATE_IDLE) {
        this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;

        yaw = Math_Vec3f_Yaw(&this->actor.home.pos, &actorToBlock->world.pos);

        this->actor.world.pos.x = this->actor.home.pos.x;
        this->actor.world.pos.x += 60.0f * Math_SinS(yaw);

        this->actor.world.pos.z = this->actor.home.pos.z;
        this->actor.world.pos.z += 60.0f * Math_CosS(yaw);

        temp = fabsf((f32)this->actor.yawTowardsPlayer - yaw) * 0.001f * 3.0f;
        this->skelAnime.playSpeed = CLAMP(temp, 1.0f, 3.0f);
    }

    if (((this->interactInfo.talkState == NPC_TALK_STATE_ACTION) &&
         play->sceneId == SCENE_KOKIRI_FOREST_Oot) ||
        this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        if ((CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot)) &&
            !GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot) &&
            (play->sceneId == SCENE_KOKIRI_FOREST_Oot)) {
            play->msgCtx.msgMode = MSGMODE_PAUSED;
        }

        if (play->sceneId == SCENE_KOKIRI_FOREST_Oot) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_SHOWED_MIDO_SWORD_SHIELD_Oot);
        }
        if (play->sceneId == SCENE_LOST_WOODS_Oot) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SARIAS_SONG_FOR_MIDO_AS_ADULT_Oot);
        }

        EnMd_SetAnimSequence(this, 3);
        EnMd_UpdateAnimSequence(this);
        this->waypoint = 1;
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        this->actionFunc = EnMd_Walk;
        this->actor.speed = 1.5f;
        return;
    }

    if (this->skelAnime.animation == &gMidoHandsOnHipsIdleAnim) {
        Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 17);
    }

    // In the Lost Woods, adult Link plays Saria's Song for Mido. OoT's Player plays for an actor that asks
    // (PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR); MM's offers the ocarina to an actor that offers itself.
    if ((this->interactInfo.talkState == NPC_TALK_STATE_IDLE) && (play->sceneId == SCENE_LOST_WOODS_Oot)) {
        if (Actor_OcarinaInteractionAccepted(&this->actor, &play->state)) {
            Message_StartOcarinaOot(play, OCARINA_ACTION_CHECK_SARIA_Oot);
            this->actionFunc = EnMd_ListenToOcarina;
            return;
        }
        Actor_OfferOcarinaInteraction(&this->actor, play, 30.0f + this->collider.dim.radius, 100.0f);
    }
}

static void EnMd_ListenToOcarina(EnMd* this, PlayState* play) {
    if (Hyrule_GetOcarinaModeOot(play) >= OCARINA_MODE_04_Oot) {
        this->actionFunc = EnMd_BlockPath;
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
    } else if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_03_Oot) {
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        this->actor.textId = TEXT_OOT(0x1067);
        Actor_OfferTalk(&this->actor, play, this->collider.dim.radius + 30.0f);

        this->actionFunc = EnMd_BlockPath;
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
    }
}

static void EnMd_Walk(EnMd* this, PlayState* play) {
    Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 17);
    EnMd_UpdateAnimSequence(this);

    if (!(EnMd_FollowPath(this, play)) || (this->waypoint != 0)) {
        this->actor.shape.rot = this->actor.world.rot;
        return;
    }

    if ((CHECK_QUEST_ITEM_OOT(QUEST_KOKIRI_EMERALD_Oot)) &&
        !GET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot) &&
        (play->sceneId == SCENE_KOKIRI_FOREST_Oot)) {
        Message_CloseTextbox(play);
        SET_EVENTCHKINF_OOT(EVENTCHKINF_SPOKE_TO_MIDO_AFTER_DEKU_TREES_DEATH_Oot);
        Actor_Kill(&this->actor);
        return;
    }

    EnMd_SetAnimSequence(this, 11);

    this->skelAnime.playSpeed = 0.0f;
    this->actor.speed = 0.0f;
    this->actor.home.pos = this->actor.world.pos;
    this->actionFunc = EnMd_Watch;
}

static void EnMd_Update(Actor* thisx, PlayState* play) {
    EnMd* this = (EnMd*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);
    EnMd_UpdateEyes(this);
    EnMd_UpdateAlphaByDistance(this, play);
    Actor_MoveWithGravity(&this->actor);
    EnMd_UpdateTalking(this, play);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    this->actionFunc(this, play);
}

static s32 EnMd_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx, Gfx** gfx) {
    EnMd* this = (EnMd*)thisx;
    Vec3s vec;

    if (limbIndex == 16) {
        Matrix_Translate(1200.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        vec = this->interactInfo.headRot;
        Matrix_RotateXFApply((vec.y / 32768.0f) * M_PI);
        Matrix_RotateZF((vec.x / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-1200.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == 9) {
        vec = this->interactInfo.torsoRot;
        Matrix_RotateXFApply((vec.x / 32768.0f) * M_PI);
        Matrix_RotateYF((vec.y / 32768.0f) * M_PI, MTXMODE_APPLY);
    }

    if (((limbIndex == 9) || (limbIndex == 10)) || (limbIndex == 13)) {
        rot->y += Math_SinS(this->fidgetTableY[limbIndex]) * 200.0f;
        rot->z += Math_CosS(this->fidgetTableZ[limbIndex]) * 200.0f;
    }

    return false;
}

static void EnMd_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx, Gfx** gfx) {
    EnMd* this = (EnMd*)thisx;
    Vec3f vec = { 400.0f, 0.0f, 0.0f };

    if (limbIndex == 16) {
        Matrix_MultVec3f(&vec, &this->actor.focus.pos);
    }
}

static void EnMd_Draw(Actor* thisx, PlayState* play) {
    static void* sEyeTextures[] = {
        gMidoEyeOpenTex,
        gMidoEyeHalfTex,
        gMidoEyeClosedTex,
    };
    EnMd* this = (EnMd*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->alpha == 255) {
        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIdx]));
        func_80034BA0(play, &this->skelAnime, EnMd_OverrideLimbDraw, EnMd_PostLimbDraw, &this->actor, this->alpha);
    } else if (this->alpha != 0) {
        gSPSegment(POLY_XLU_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIdx]));
        func_80034CC4(play, &this->skelAnime, EnMd_OverrideLimbDraw, EnMd_PostLimbDraw, &this->actor, this->alpha);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
