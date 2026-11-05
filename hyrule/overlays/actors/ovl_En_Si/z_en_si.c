/*
 * File: z_en_si.c
 * Overlay: En_Si
 * Description: Gold Skulltula token
 */

#include "z_en_si.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR)

static void EnSi_Init(Actor* thisx, PlayState* play);
static void EnSi_Destroy(Actor* thisx, PlayState* play);
static void EnSi_Update(Actor* thisx, PlayState* play);
static void EnSi_Draw(Actor* thisx, PlayState* play);

static s32 func_80AFB748(EnSi* this, PlayState* play);
static void func_80AFB768(EnSi* this, PlayState* play);
static void func_80AFB89C(EnSi* this, PlayState* play);
static void func_80AFB950(EnSi* this, PlayState* play);

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_NO_PUSH | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0x00000090), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON | ACELEM_HOOKABLE,
        OCELEM_ON,
    },
    { 20, 18, 2, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 D_80AFBADC = { 0, 0, 0, 0, MASS_IMMOVABLE };

ActorProfile En_Si_Oot_Profile = {
    /**/ ACTOR_EN_SI_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_ST_Oot,
    /**/ sizeof(EnSi),
    /**/ EnSi_Init,
    /**/ EnSi_Destroy,
    /**/ EnSi_Update,
    /**/ EnSi_Draw,
};

static void EnSi_Init(Actor* thisx, PlayState* play) {
    EnSi* this = (EnSi*)thisx;

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &D_80AFBADC);
    Actor_SetScale(&this->actor, 0.025f);
    this->unk_19C = 0;
    this->actionFunc = func_80AFB768;
    this->actor.shape.yOffset = 42.0f;
}

static void EnSi_Destroy(Actor* thisx, PlayState* play) {
    EnSi* this = (EnSi*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 func_80AFB748(EnSi* this, PlayState* play) {
    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
    }
    return 0;
}

static void func_80AFB768(EnSi* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_HOOKSHOT_ATTACHED)) {
        this->actionFunc = func_80AFB89C;
    } else {
        Math_SmoothStepToF(&this->actor.scale.x, 0.25f, 0.4f, 1.0f, 0.0f);
        Actor_SetScale(&this->actor, this->actor.scale.x);
        this->actor.shape.rot.y += 0x400;

        if (!Player_InCsMode(play)) {
            func_80AFB748(this, play);

            if (this->collider.base.ocFlags2 & OC2_HIT_PLAYER) {
                this->collider.base.ocFlags2 &= ~OC2_HIT_PLAYER;
                if ((true)) {
                    Item_Give(play, ITEM_SKULL_TOKEN_Oot);
                    if ((true)) {
                        player->actor.freezeTimer = 10;
                    }
                    Message_StartTextbox(play, TEXT_OOT(0xB4), NULL);
                    Audio_PlayFanfare(SEQ_OOT(0x39)); // OoT's SEQ_OOT(0x39)
                }
                this->actionFunc = func_80AFB950;
            } else {
                Collider_UpdateCylinder(&this->actor, &this->collider);
                CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
                CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            }
        }
    }
}

static void func_80AFB89C(EnSi* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.scale.x, 0.25f, 0.4f, 1.0f, 0.0f);
    Actor_SetScale(&this->actor, this->actor.scale.x);
    this->actor.shape.rot.y += 0x400;

    if (!CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_HOOKSHOT_ATTACHED)) {
        if ((true)) {
            Item_Give(play, ITEM_SKULL_TOKEN_Oot);
            if ((true)) {
                player->actor.freezeTimer = 10;
            }
            Message_StartTextbox(play, TEXT_OOT(0xB4), NULL);
            Audio_PlayFanfare(SEQ_OOT(0x39)); // OoT's SEQ_OOT(0x39)
        }
        this->actionFunc = func_80AFB950;
    }
}

static void func_80AFB950(EnSi* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Message_GetState(&play->msgCtx) != TEXT_STATE_CLOSING &&
        (true)) {
        player->actor.freezeTimer = 10;
    } else {
        SET_GS_FLAGS_OOT((this->actor.params & 0x1F00) >> 8, this->actor.params & 0xFF);
        Actor_Kill(&this->actor);
    }
}

static void EnSi_Update(Actor* thisx, PlayState* play) {
    EnSi* this = (EnSi*)thisx;

    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, 16.0f);
}

static void EnSi_Draw(Actor* thisx, PlayState* play) {
    EnSi* this = (EnSi*)thisx;

    if (this->actionFunc != func_80AFB950) {
        func_800B8118(&this->actor, play, 0);
        func_800B8050(&this->actor, play, 0);
        GetItem_Draw(play, GID_SKULL_TOKEN_2_Oot);
    }
}
