/*
 * File: z_en_skjneedle.c
 * Overlay: ovl_En_Skjneedle
 * Description: Skullkid Needle Attack
 */

#include "z_en_skjneedle.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_skj/object_skj.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR)

static void EnSkjneedle_Init(Actor* thisx, PlayState* play);
static void EnSkjneedle_Destroy(Actor* thisx, PlayState* play);
static void EnSkjneedle_Update(Actor* thisx, PlayState* play);
static void EnSkjneedle_Draw(Actor* thisx, PlayState* play);

static s32 EnSkjNeedle_CollisionCheck(EnSkjneedle* this);

ActorProfile En_Skjneedle_Oot_Profile = {
    /**/ ACTOR_EN_SKJNEEDLE_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_SKJ_Oot,
    /**/ sizeof(EnSkjneedle),
    /**/ EnSkjneedle_Init,
    /**/ EnSkjneedle_Destroy,
    /**/ EnSkjneedle_Update,
    /**/ EnSkjneedle_Draw,
};

static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_HIT1,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 10, 4, -2, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 2, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 30, ICHAIN_STOP),
};

static void EnSkjneedle_Init(Actor* thisx, PlayState* play) {
    EnSkjneedle* this = (EnSkjneedle*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);
    ActorShape_Init(&this->actor.shape, 0, ActorShadow_DrawCircle, 20.0f);
    thisx->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    Actor_SetScale(&this->actor, 0.01f);
}

static void EnSkjneedle_Destroy(Actor* thisx, PlayState* play) {
    EnSkjneedle* this = (EnSkjneedle*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 EnSkjNeedle_CollisionCheck(EnSkjneedle* this) {
    if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        return 1;
    }
    return 0;
}

static void EnSkjneedle_Update(Actor* thisx, PlayState* play2) {
    EnSkjneedle* this = (EnSkjneedle*)thisx;
    PlayState* play = play2;

    this->unusedTimer1++;
    if (this->killTimer != 0) {
        this->killTimer--;
    }
    if (EnSkjNeedle_CollisionCheck(this) || this->killTimer == 0) {
        Actor_Kill(&this->actor);
    } else {
        Actor_SetScale(&this->actor, 0.01f);
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 20.0f, 20.0f, 20.0f, 0x407);
    }
}

static void EnSkjneedle_Draw(Actor* thisx, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gSkullKidNeedleDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
