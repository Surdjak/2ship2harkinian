/*
 * File: z_en_kz.c
 * Overlay: ovl_En_Kz
 * Description: King Zora
 */

#include "z_en_kz.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "assets/oot/objects/object_kz/object_kz.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnKz_Init(Actor* thisx, PlayState* play);
static void EnKz_Destroy(Actor* thisx, PlayState* play);
static void EnKz_Update(Actor* thisx, PlayState* play);
static void EnKz_Draw(Actor* thisx, PlayState* play);

static void EnKz_PreMweepWait(EnKz* this, PlayState* play);
static void EnKz_SetupMweep(EnKz* this, PlayState* play);
static void EnKz_Mweep(EnKz* this, PlayState* play);
static void EnKz_StopMweep(EnKz* this, PlayState* play);
static void EnKz_Wait(EnKz* this, PlayState* play);
static void EnKz_SetupGetItem(EnKz* this, PlayState* play);
static void EnKz_StartTimer(EnKz* this, PlayState* play);

ActorProfile En_Kz_Oot_Profile = {
    /**/ ACTOR_EN_KZ_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_KZ_Oot,
    /**/ sizeof(EnKz),
    /**/ EnKz_Init,
    /**/ EnKz_Destroy,
    /**/ EnKz_Update,
    /**/ EnKz_Draw,
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
    { 80, 120, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

typedef enum {
    /* 0 */ ENKZ_ANIM_0,
    /* 1 */ ENKZ_ANIM_1,
    /* 2 */ ENKZ_ANIM_2
} EnKzAnimation;

static AnimationInfo sAnimationInfo[] = {
    { &gKzIdleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 0.0f },
    { &gKzIdleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -10.0f },
    { &gKzMweepAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -10.0f },
};

static u16 EnKz_GetTextIdChild(PlayState* play, EnKz* this) {
    Player* player = GET_PLAYER(play);

    if (((CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot)))) {
        return TEXT_OOT(0x402B);
    } else if (GET_EVENTCHKINF_OOT(EVENTCHKINF_KING_ZORA_MOVED_Oot)) {
        return TEXT_OOT(0x401C);
    } else {
        player->exchangeItemAction = Hyrule_ExchangeItemToActionOot(EXCH_ITEM_LETTER_RUTO);
        return TEXT_OOT(0x401A);
    }
}

static u16 EnKz_GetTextIdAdult(PlayState* play, EnKz* this) {
    Player* player = GET_PLAYER(play);

    // this works because both ITEM_NONE and later trade items are > ITEM_FROG_Oot
    if (INV_CONTENT_OOT(ITEM_TRADE_ADULT_Oot) >= ITEM_FROG_Oot) {
        if (!GET_INFTABLE_OOT(INFTABLE_139_Oot)) {
            if ((CHECK_OWNED_EQUIP_OOT(2, EQUIP_OWNED_ZORA_HOVER))) {
                return TEXT_OOT(0x401F);
            } else {
                return TEXT_OOT(0x4012);
            }
        } else {
            return CHECK_QUEST_ITEM_OOT(QUEST_SONG_SERENADE_Oot) ? TEXT_OOT(0x4045) : TEXT_OOT(0x401A);
        }
    } else {
        player->exchangeItemAction = Hyrule_ExchangeItemToActionOot(EXCH_ITEM_PRESCRIPTION);
        return TEXT_OOT(0x4012);
    }
}

static u16 EnKz_GetTextId(PlayState* play, Actor* thisx) {
    EnKz* this = (EnKz*)thisx;
    u16 textId = Text_GetFaceReactionOot(play, 0x1E);

    if (textId != 0) {
        return textId;
    }

    if (LINK_IS_ADULT_OOT) {
        return EnKz_GetTextIdAdult(play, this);
    } else {
        return EnKz_GetTextIdChild(play, this);
    }
}

static s16 EnKz_UpdateTalkState(PlayState* play, Actor* thisx) {
    EnKz* this = (EnKz*)thisx;
    s16 talkState = NPC_TALK_STATE_TALKING;

    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_DONE:
            if ((false)) {
                if (Message_ShouldAdvance(play)) {
                    talkState = NPC_TALK_STATE_ITEM_GIVEN;
                }
            } else {
                talkState = NPC_TALK_STATE_IDLE;
                switch (this->actor.textId) {
                    case TEXT_OOT(0x4012):
                        SET_INFTABLE_OOT(INFTABLE_139_Oot);
                        talkState = NPC_TALK_STATE_ACTION;
                        break;
                    case TEXT_OOT(0x401B):
                        talkState = !Message_ShouldAdvance(play) ? NPC_TALK_STATE_TALKING : NPC_TALK_STATE_ACTION;
                        break;
                    case TEXT_OOT(0x401F):
                        SET_INFTABLE_OOT(INFTABLE_139_Oot);
                        break;
                }
            }
            break;
        case TEXT_STATE_CLOSING:
            if ((false)) {
                talkState = NPC_TALK_STATE_IDLE;
                switch (this->actor.textId) {
                    case TEXT_OOT(0x4012):
                        SET_INFTABLE_OOT(INFTABLE_139_Oot);
                        FALLTHROUGH;
                    case TEXT_OOT(0x401B):
                        talkState = NPC_TALK_STATE_ACTION;
                        break;
                    case TEXT_OOT(0x401F):
                        SET_INFTABLE_OOT(INFTABLE_139_Oot);
                        break;
                }
            }
            break;
        case TEXT_STATE_FADING:
            if (this->actor.textId != TEXT_OOT(0x4014)) {
                if (this->actor.textId == TEXT_OOT(0x401B) && !this->sfxPlayed) {
                    Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                    this->sfxPlayed = true;
                }
            } else if (!this->sfxPlayed) {
                Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
                this->sfxPlayed = true;
            }
            break;
        case TEXT_STATE_CHOICE:
            if (!Message_ShouldAdvance(play)) {
                break;
            }
            if (this->actor.textId == TEXT_OOT(0x4014)) {
                if (play->msgCtx.choiceIndex == 0) {
                    if (!(false)) {
                        EnKz_SetupGetItem(this, play);
                    }
                    talkState = NPC_TALK_STATE_ACTION;
                } else {
                    this->actor.textId = TEXT_OOT(0x4016);
                    Message_ContinueTextbox(play, this->actor.textId);
                }
            }
            break;
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(play)) {
                talkState = NPC_TALK_STATE_ACTION;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_NEXT:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            break;
    }
    return talkState;
}

static void EnKz_UpdateEyes(EnKz* this) {
    if (DECR(this->blinkTimer) == 0) {
        this->eyeIdx += 1;
        if (this->eyeIdx >= 3) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIdx = 0;
        }
    }
}

/**
 * Custom version of Npc_UpdateTalking.
 *
 * @see Npc_UpdateTalking
 */
static s32 EnKz_UpdateTalking(PlayState* play, EnKz* this, s16* talkState, f32 interactRange, NpcGetTextIdFunc getTextId,
                       NpcUpdateTalkStateFunc updateTalkState) {
    Player* player = GET_PLAYER(play);
    s16 x;
    s16 y;
    f32 xzDistToPlayer;
    f32 yaw;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        *talkState = NPC_TALK_STATE_TALKING;
        return true;
    }

    if (*talkState != NPC_TALK_STATE_IDLE) {
        *talkState = updateTalkState(play, &this->actor);
        return false;
    }

    yaw = Math_Vec3f_Yaw(&this->actor.home.pos, &player->actor.world.pos);
    yaw -= this->actor.shape.rot.y;
    if ((fabsf(yaw) > 1638.0f) || (this->actor.xzDistToPlayer < 265.0f)) {
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        return false;
    }

    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;

    Actor_GetScreenPos(play, &this->actor, &x, &y);
    if (!((x >= -30) && (x < 361) && (y >= -10) && (y < 241))) {
        return false;
    }

    xzDistToPlayer = this->actor.xzDistToPlayer;
    this->actor.xzDistToPlayer = Math_Vec3f_DistXZ(&this->actor.home.pos, &player->actor.world.pos);
    if (!Actor_OfferTalk(&this->actor, play, interactRange)) {
        this->actor.xzDistToPlayer = xzDistToPlayer;
        return false;
    }
    this->actor.xzDistToPlayer = xzDistToPlayer;
    this->actor.textId = getTextId(play, &this->actor);

    return false;
}

static void func_80A9CB18(EnKz* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (EnKz_UpdateTalking(play, this, &this->interactInfo.talkState, 340.0f, EnKz_GetTextId, EnKz_UpdateTalkState)) {
        if (((this->actor.textId == TEXT_OOT(0x401A))) &&
            !GET_EVENTCHKINF_OOT(EVENTCHKINF_KING_ZORA_MOVED_Oot)) {
            if (Actor_GetPlayerExchangeItemIdOot(play) == EXCH_ITEM_LETTER_RUTO) {
                this->actor.textId = TEXT_OOT(0x401B);
                this->sfxPlayed = false;
            } else {
                this->actor.textId = TEXT_OOT(0x401A);
            }
            player->actor.textId = this->actor.textId;
            return;
        }

        if (LINK_IS_ADULT_OOT) {
            if ((INV_CONTENT_OOT(ITEM_TRADE_ADULT_Oot) == ITEM_PRESCRIPTION_Oot) &&
                (Actor_GetPlayerExchangeItemIdOot(play) == EXCH_ITEM_PRESCRIPTION)) {
                this->actor.textId = TEXT_OOT(0x4014);
                this->sfxPlayed = false;
                player->actor.textId = this->actor.textId;
                if (!(false)) {
                    this->isTrading = true;
                }
                return;
            }
            if (!(false)) {
                this->isTrading = false;
            }
            if (GET_INFTABLE_OOT(INFTABLE_139_Oot)) {
                this->actor.textId = CHECK_QUEST_ITEM_OOT(QUEST_SONG_SERENADE_Oot) ? TEXT_OOT(0x4045) : TEXT_OOT(0x401A);
                player->actor.textId = this->actor.textId;
            } else {
                this->actor.textId =
                    (CHECK_OWNED_EQUIP_OOT(2, EQUIP_OWNED_ZORA_HOVER))
                        ? TEXT_OOT(0x401F)
                        : TEXT_OOT(0x4012);
                player->actor.textId = this->actor.textId;
            }
        }
    }
}

static s32 EnKz_FollowPath(EnKz* this, PlayState* play) {
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
    Math_SmoothStepToS(&this->actor.world.rot.y, (Math_FAtan2F(pathDiffX, pathDiffZ) * (0x8000 / M_PI)), 0xA, 0x3E8, 1);

    if ((SQ(pathDiffX) + SQ(pathDiffZ)) < 10.0f) {
        this->waypoint++;
        if (this->waypoint >= path->count) {
            this->waypoint = 0;
        }
        return 1;
    }
    return 0;
}

static s32 EnKz_SetMovedPos(EnKz* this, PlayState* play) {
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

static void EnKz_Init(Actor* thisx, PlayState* play) {
    EnKz* this = (EnKz*)thisx;
    s32 pad;

    SkelAnime_InitFlex(play, &this->skelanime, &gKzSkel, NULL, this->jointTable, this->morphTable, 12);
    ActorShape_Init(&this->actor.shape, 0.0, NULL, 0.0);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &sColChkInfoInit);
    Actor_SetScale(&this->actor, 0.01);
    this->actor.attentionRangeType = 3;
    this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
    Actor_ChangeAnimationByInfo(&this->skelanime, sAnimationInfo, ENKZ_ANIM_0);

    if (((GET_EVENTCHKINF_OOT(EVENTCHKINF_KING_ZORA_MOVED_Oot)))) {
        EnKz_SetMovedPos(this, play);
    }

    if (LINK_IS_ADULT_OOT) {
        if (!GET_INFTABLE_OOT(INFTABLE_138_Oot)) {
            Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BG_ICE_SHELTER_Oot, this->actor.world.pos.x,
                               this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 0x04FF);
        }
        this->actionFunc = EnKz_Wait;
    } else {
        this->actionFunc = EnKz_PreMweepWait;
    }
}

static void EnKz_Destroy(Actor* thisx, PlayState* play) {
    EnKz* this = (EnKz*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnKz_PreMweepWait(EnKz* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        Actor_ChangeAnimationByInfo(&this->skelanime, sAnimationInfo, ENKZ_ANIM_2);
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        this->actionFunc = EnKz_SetupMweep;
    } else {
        Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 12);
    }
}

static void EnKz_SetupMweep(EnKz* this, PlayState* play) {
    Vec3f unused = { 0.0f, 0.0f, 0.0f };
    Vec3f pos;
    Vec3f initPos;

    this->cutsceneCamera = Play_CreateSubCamera(play);
    this->gameplayCamera = play->activeCamId;
    Play_ChangeCameraStatus(play, this->gameplayCamera, CAM_STATUS_WAIT);
    Play_ChangeCameraStatus(play, this->cutsceneCamera, CAM_STATUS_ACTIVE);
    pos = this->actor.world.pos;
    initPos = this->actor.home.pos;
    pos.y += 60.0f;
    initPos.y += -100.0f;
    initPos.z += 260.0f;
    Play_SetCameraAtEye(play, this->cutsceneCamera, &pos, &initPos);
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
    this->actor.speed = 0.1f;
    this->actionFunc = EnKz_Mweep;
}

static void EnKz_Mweep(EnKz* this, PlayState* play) {
    Vec3f unused = { 0.0f, 0.0f, 0.0f };
    Vec3f pos;
    Vec3f initPos;

    pos = this->actor.world.pos;
    initPos = this->actor.home.pos;
    pos.y += 60.0f;
    initPos.y += -100.0f;
    initPos.z += 260.0f;
    Play_SetCameraAtEye(play, this->cutsceneCamera, &pos, &initPos);
    if ((EnKz_FollowPath(this, play) == 1) && (this->waypoint == 0)) {
        Actor_ChangeAnimationByInfo(&this->skelanime, sAnimationInfo, ENKZ_ANIM_1);
        Inventory_ReplaceItem(play, ITEM_LETTER_RUTO_Oot, ITEM_BOTTLE_Oot);
        EnKz_SetMovedPos(this, play);
        SET_EVENTCHKINF_OOT(EVENTCHKINF_KING_ZORA_MOVED_Oot);
        this->actor.speed = 0.0;
        this->actionFunc = EnKz_StopMweep;
    }
    if (this->skelanime.curFrame == 13.0f) {
        Actor_PlaySfx(&this->actor, NA_SE_VO_KZ_MOVE);
    }
}

static void EnKz_StopMweep(EnKz* this, PlayState* play) {
    Play_ChangeCameraStatus(play, this->gameplayCamera, CAM_STATUS_ACTIVE);
    Play_ClearCamera(play, this->cutsceneCamera);
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
    this->actionFunc = EnKz_Wait;
}

static void EnKz_Wait(EnKz* this, PlayState* play) {
    if (this->interactInfo.talkState == NPC_TALK_STATE_ACTION) {
        if ((false)) {
            this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        }
        this->actionFunc = EnKz_SetupGetItem;
        EnKz_SetupGetItem(this, play);
    } else {
        Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 12);
    }
}

static void EnKz_SetupGetItem(EnKz* this, PlayState* play) {
    s32 getItemId;
    f32 xzRange;
    f32 yRange;

    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actor.parent = NULL;
        this->interactInfo.talkState = NPC_TALK_STATE_TALKING;
        this->actionFunc = EnKz_StartTimer;
    } else {
        if ((false)) {
            getItemId = Actor_GetPlayerExchangeItemIdOot(play) == EXCH_ITEM_PRESCRIPTION ? GI_FROG_Oot : GI_TUNIC_ZORA_Oot;
        } else {
            getItemId = this->isTrading ? GI_FROG_Oot : GI_TUNIC_ZORA_Oot;
        }
        yRange = fabsf(this->actor.playerHeightRel) + 1.0f;
        xzRange = this->actor.xzDistToPlayer + 1.0f;
        Actor_OfferGetItem(&this->actor, play, getItemId, xzRange, yRange);
    }
}

static void EnKz_StartTimer(EnKz* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        if ((INV_CONTENT_OOT(ITEM_TRADE_ADULT_Oot) == ITEM_FROG_Oot)) {
            Interface_StartTimer(TIMER_ID_MINIGAME_2, 180); // start timer2 with 3 minutes
            gSaveContext.eventInfOot[1] &= ~1;
        }
        this->interactInfo.talkState = NPC_TALK_STATE_IDLE;
        this->actionFunc = EnKz_Wait;
    }
}

static void EnKz_Update(Actor* thisx, PlayState* play) {
    EnKz* this = (EnKz*)thisx;
    s32 pad;

    if (LINK_IS_ADULT_OOT && !GET_INFTABLE_OOT(INFTABLE_138_Oot)) {
        SET_INFTABLE_OOT(INFTABLE_138_Oot);
    }
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelanime);
    EnKz_UpdateEyes(this);
    Actor_MoveWithGravity(&this->actor);
    if (this->actionFunc != EnKz_StartTimer) {
        func_80A9CB18(this, play);
    }
    this->actionFunc(this, play);
}

static s32 EnKz_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnKz* this = (EnKz*)thisx;

    if (limbIndex == 8 || limbIndex == 9 || limbIndex == 10) {
        rot->y += Math_SinS(this->fidgetTableY[limbIndex]) * 200.0f;
        rot->z += Math_CosS(this->fidgetTableZ[limbIndex]) * 200.0f;
    }
    if (limbIndex) {}
    return false;
}

static void EnKz_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnKz* this = (EnKz*)thisx;
    Vec3f mult = { 2600.0f, 0.0f, 0.0f };

    if (limbIndex == 11) {
        Matrix_MultVec3f(&mult, &this->actor.focus.pos);
    }
}

static void EnKz_Draw(Actor* thisx, PlayState* play) {
    static void* sEyeSegments[] = {
        gKzEyeOpenTex,
        gKzEyeHalfTex,
        gKzEyeClosedTex,
    };
    EnKz* this = (EnKz*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeSegments[this->eyeIdx]));
    Gfx_SetupDL37_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelanime, EnKz_OverrideLimbDraw, EnKz_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
