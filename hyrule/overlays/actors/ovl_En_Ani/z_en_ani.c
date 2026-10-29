/*
 * File: z_en_ani.c
 * Overlay: ovl_En_Ani
 * Description: Kakariko Roof Guy
 */

#include "z_en_ani.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleCutscene.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_ani/object_ani.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnAni_Init(Actor* thisx, PlayState* play);
static void EnAni_Destroy(Actor* thisx, PlayState* play);
static void EnAni_Update(Actor* thisx, PlayState* play);
static void EnAni_Draw(Actor* thisx, PlayState* play);

static s32 EnAni_SetText(EnAni* this, PlayState* play, u16 textId);
static void func_809B04F0(EnAni* this, PlayState* play);
static void func_809B0524(EnAni* this, PlayState* play);
static void func_809B0558(EnAni* this, PlayState* play);
static void func_809B05F0(EnAni* this, PlayState* play);
static void func_809B064C(EnAni* this, PlayState* play);
static void func_809B07F8(EnAni* this, PlayState* play);
static void func_809B0988(EnAni* this, PlayState* play);
static void func_809B0994(EnAni* this, PlayState* play);
static void func_809B0A28(EnAni* this, PlayState* play);
static void func_809B0A6C(EnAni* this, PlayState* play);

ActorProfile En_Ani_Oot_Profile = {
    /**/ ACTOR_EN_ANI_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_ANI_Oot,
    /**/ sizeof(EnAni),
    /**/ EnAni_Init,
    /**/ EnAni_Destroy,
    /**/ EnAni_Update,
    /**/ EnAni_Draw,
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
    { 30, 40, 0, { 0 } },
};

static void EnAni_SetupAction(EnAni* this, EnAniActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 10, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 850, ICHAIN_STOP),
};

static void EnAni_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnAni* this = (EnAni*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, -2800.0f, ActorShadow_DrawCircle, 36.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gRoofManSkel, &gRoofManIdleAnim, this->jointTable, this->morphTable,
                       0x10);
    Animation_PlayOnce(&this->skelAnime, &gRoofManIdleAnim);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    if (!LINK_IS_ADULT_OOT) {
        EnAni_SetupAction(this, func_809B064C);
    } else {
        EnAni_SetupAction(this, func_809B07F8);
    }
    this->unk_2AA = 0;
    this->unk_2A8 = 0;
    this->actor.terminalVelocity = -1.0f;
    this->actor.velocity.y = -1.0f;
}

static void EnAni_Destroy(Actor* thisx, PlayState* play) {
    EnAni* this = (EnAni*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 EnAni_SetText(EnAni* this, PlayState* play, u16 textId) {
    this->actor.textId = textId;
    this->unk_2A8 |= 1;
    Actor_OfferTalk(&this->actor, play, 100.0f);
    return 0;
}

static void func_809B04F0(EnAni* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        EnAni_SetupAction(this, func_809B064C);
    }
}

static void func_809B0524(EnAni* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        EnAni_SetupAction(this, func_809B07F8);
    }
}

static void func_809B0558(EnAni* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actor.parent = NULL;
        if (!LINK_IS_ADULT_OOT) {
            EnAni_SetupAction(this, func_809B04F0);
        } else {
            EnAni_SetupAction(this, func_809B0524);
        }
        SET_ITEMGETINF_OOT(ITEMGETINF_15_Oot);
    } else {
        if ((true)) {
            Actor_OfferGetItem(&this->actor, play, GI_HEART_PIECE_Oot, 10000.0f, 200.0f);
        }
    }
}

static void func_809B05F0(EnAni* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        EnAni_SetupAction(this, func_809B0558);
    }

    if ((true)) {
        Actor_OfferGetItem(&this->actor, play, GI_HEART_PIECE_Oot, 10000.0f, 200.0f);
    }
}

static void func_809B064C(EnAni* this, PlayState* play) {
    u16 textId;
    s16 yawDiff;
    u16 textId2;

    textId2 = Text_GetFaceReactionOot(play, 0xA);
    textId = textId2 & 0xFFFF;

    if (!textId) {}

    if (textId2 == 0) {
        textId = !IS_DAY_OOT ? TEXT_OOT(0x5051) : TEXT_OOT(0x5050);
    }

    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if (this->actor.textId == TEXT_OOT(0x5056)) {
            EnAni_SetupAction(this, func_809B04F0);
        } else if (this->actor.textId == TEXT_OOT(0x5055)) {
            EnAni_SetupAction(this, func_809B05F0);
        } else {
            EnAni_SetupAction(this, func_809B04F0);
        }
    } else if (yawDiff >= -0x36AF && yawDiff < 0 && this->actor.xzDistToPlayer < 150.0f &&
               -80.0f < this->actor.playerHeightRel) {
        if (GET_ITEMGETINF_OOT(ITEMGETINF_15_Oot)) {
            EnAni_SetText(this, play, TEXT_OOT(0x5056));
        } else {
            EnAni_SetText(this, play, TEXT_OOT(0x5055));
        }
    } else if (yawDiff >= -0x3E7 && yawDiff < 0x36B0 && this->actor.xzDistToPlayer < 350.0f) {
        EnAni_SetText(this, play, textId);
    }
}

static void func_809B07F8(EnAni* this, PlayState* play) {
    s16 pad;
    s16 yawDiff;
    u16 textId;

    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if (this->actor.textId == TEXT_OOT(0x5056)) {
            EnAni_SetupAction(this, func_809B0524);
        } else if (this->actor.textId == TEXT_OOT(0x5055)) {
            EnAni_SetupAction(this, func_809B05F0);
        } else {
            EnAni_SetupAction(this, func_809B0524);
        }
    } else if (yawDiff > -0x36B0 && yawDiff < 0 && this->actor.xzDistToPlayer < 150.0f &&
               -80.0f < this->actor.playerHeightRel) {
        if (GET_ITEMGETINF_OOT(ITEMGETINF_15_Oot)) {
            EnAni_SetText(this, play, TEXT_OOT(0x5056));
        } else {
            EnAni_SetText(this, play, TEXT_OOT(0x5055));
        }
    } else if (yawDiff > -0x3E8 && yawDiff < 0x36B0 && this->actor.xzDistToPlayer < 350.0f) {
        if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_DEATH_MOUNTAIN_ERUPTED_Oot)) {
            textId = TEXT_OOT(0x5052);
        } else {
            textId = (GET_ITEMGETINF_OOT(ITEMGETINF_15_Oot)) ? TEXT_OOT(0x5054) : TEXT_OOT(0x5053);
        }
        EnAni_SetText(this, play, textId);
    }
}

static void func_809B0988(EnAni* this, PlayState* play) {
}

static void func_809B0994(EnAni* this, PlayState* play) {
    if (gCsCtxOot.npcActions[0]->action == 4) {
        Animation_Change(&this->skelAnime, &gRoofManGettingUpAfterKnockbackAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gRoofManGettingUpAfterKnockbackAnim), ANIMMODE_ONCE, -4.0f);
        this->unk_2AA++;
        this->actor.shape.shadowDraw = ActorShadow_DrawCircle;
    }
}

static void func_809B0A28(EnAni* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        this->unk_2AA++;
    }
}

static void func_809B0A6C(EnAni* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        this->skelAnime.curFrame = 0.0f;
    }
    if (gCsCtxOot.npcActions[0]->action == 2) {
        Animation_Change(&this->skelAnime, &gRoofManKnockbackAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gRoofManKnockbackAnim), ANIMMODE_ONCE, 0.0f);
        this->actor.shape.shadowDraw = NULL;
        this->unk_2AA++;
    }
}

static void EnAni_Update(Actor* thisx, PlayState* play) {
    EnAni* this = (EnAni*)thisx;
    s32 pad[2];

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    if ((gCsCtxOot.state != CS_STATE_IDLE_Oot) && (gCsCtxOot.npcActions[0] != NULL)) {
        switch (this->unk_2AA) {
            case 0:
                func_809B0A6C(this, play);
                break;
            case 1:
                func_809B0A28(this, play);
                break;
            case 2:
                func_809B0994(this, play);
                break;
            case 3:
                func_809B0A28(this, play);
                break;
            case 4:
                func_809B0988(this, play);
                break;
        }

        if (gCsCtxOot.frames == 100) {
            Audio_PlaySfx_2(NA_SE_IT_EARTHQUAKE);
        }
    } else {
        if (SkelAnime_Update(&this->skelAnime) != 0) {
            this->skelAnime.curFrame = 0.0f;
        }
        this->actionFunc(this, play);
    }

    if (this->unk_2A8 & 1) {
        Actor_TrackPlayer(play, &this->actor, &this->unk_29C, &this->unk_2A2, this->actor.focus.pos);
        this->unk_2A2.z = 0;
        this->unk_2A2.y = this->unk_2A2.z;
        this->unk_2A2.x = this->unk_2A2.z;
    } else {
        Math_SmoothStepToS(&this->unk_29C.x, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->unk_29C.y, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->unk_2A2.x, 0, 6, 6200, 100);
        Math_SmoothStepToS(&this->unk_2A2.y, 0, 6, 6200, 100);
    }

    if (DECR(this->blinkTimer) == 0) {
        this->blinkTimer = Rand_S16Offset(60, 60);
    }
    this->eyeIndex = this->blinkTimer;
    if (this->eyeIndex >= 3) {
        this->eyeIndex = 0;
    }
}

static s32 EnAni_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnAni* this = (EnAni*)thisx;

    if (limbIndex == 15) {
        rot->x += this->unk_29C.y;
        rot->z += this->unk_29C.x;
    }
    return false;
}

static void EnAni_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f sMultVec = { 800.0f, 500.0f, 0.0f };
    EnAni* this = (EnAni*)thisx;

    if (limbIndex == 15) {
        Matrix_MultVec3f(&sMultVec, &this->actor.focus.pos);
    }
}

static void EnAni_Draw(Actor* thisx, PlayState* play) {
    static void* eyeTextures[] = {
        gRoofManEyeOpenTex,
        gRoofManEyeHalfTex,
        gRoofManEyeClosedTex,
    };
    EnAni* this = (EnAni*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL37_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTextures[this->eyeIndex]));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnAni_OverrideLimbDraw, EnAni_PostLimbDraw, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
