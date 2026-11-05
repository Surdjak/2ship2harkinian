/*
 * File: z_en_gm.c
 * Overlay: ovl_En_Gm
 * Description: Medi-Goron
 */

#include "z_en_gm.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_oF1d_map/object_oF1d_map.h"
#include "assets/oot/objects/object_gm/object_gm.h"
#include <assert.h>

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnGm_Init(Actor* thisx, PlayState* play);
static void EnGm_Destroy(Actor* thisx, PlayState* play);
static void EnGm_Update(Actor* thisx, PlayState* play);
static void EnGm_Draw(Actor* thisx, PlayState* play);

static void func_80A3D838(EnGm* this, PlayState* play);
static void func_80A3DFBC(EnGm* this, PlayState* play);
static void func_80A3DB04(EnGm* this, PlayState* play);
static void func_80A3DC44(EnGm* this, PlayState* play);
static void func_80A3DBF4(EnGm* this, PlayState* play);
static void func_80A3DD7C(EnGm* this, PlayState* play);
static void EnGm_ProcessChoiceIndex(EnGm* this, PlayState* play);
static void func_80A3DF00(EnGm* this, PlayState* play);
static void func_80A3DF60(EnGm* this, PlayState* play);

ActorProfile En_Gm_Oot_Profile = {
    /**/ ACTOR_EN_GM_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_OF1D_MAP_Oot,
    /**/ sizeof(EnGm),
    /**/ EnGm_Init,
    /**/ EnGm_Destroy,
    /**/ EnGm_Update,
    /**/ NULL,
};

static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
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
    { 100, 120, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 5, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 30, ICHAIN_STOP),
};

static void EnGm_Init(Actor* thisx, PlayState* play) {
    EnGm* this = (EnGm*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);


    this->objGmBankIndex = Object_GetSlot(&play->objectCtx, OBJECT_GM_Oot);

    if (this->objGmBankIndex < 0) {
        assert(this->objGmBankIndex < 0);
    }

    this->updateFunc = func_80A3D838;
}

static void EnGm_Destroy(Actor* thisx, PlayState* play) {
    EnGm* this = (EnGm*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static s32 func_80A3D7C8(void) {
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        return 0;
    } else if (((!CHECK_OWNED_EQUIP_OOT(0, EQUIP_OWNED_SWORD_BIGGORON) // Don't have giant's knife
                    ))) {
        return 1;
    } else if (CHECK_OWNED_EQUIP_OOT(0, EQUIP_OWNED_SWORD_KNIFE_BROKEN)) { // Have broken giant's knife
        return 2;
    } else {
        return 3;
    }
}

static void func_80A3D838(EnGm* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->objGmBankIndex)) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        SkelAnime_InitFlex(play, &this->skelAnime, &gGoronSkel, NULL, this->jointTable, this->morphTable, 18);
        gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objGmBankIndex].segment);
        Animation_Change(&this->skelAnime, &object_gm_Anim_0002B8, 1.0f, 0.0f,
                         Animation_GetLastFrame(&object_gm_Anim_0002B8), ANIMMODE_LOOP, 0.0f);
        this->actor.draw = EnGm_Draw;
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 35.0f);
        Actor_SetScale(&this->actor, 0.05f);
        this->actor.colChkInfo.mass = MASS_IMMOVABLE;
        this->eyeTexIndex = 0;
        this->blinkTimer = 20;
        this->actor.textId = TEXT_OOT(0x3049);
        this->updateFunc = func_80A3DFBC;
        this->actionFunc = func_80A3DB04;
        this->actor.speed = 0.0f;
        this->actor.gravity = -1.0f;
        this->actor.velocity.y = 0.0f;
    }
}

static void EnGm_UpdateEye(EnGm* this) {
    if (this->blinkTimer != 0) {
        this->blinkTimer--;
    } else {
        this->eyeTexIndex++;

        if (this->eyeTexIndex >= 3) {
            this->eyeTexIndex = 0;
            this->blinkTimer = Rand_ZeroFloat(60.0f) + 20.0f;
        }
    }
}

static void EnGm_SetTextID(EnGm* this) {
    switch (func_80A3D7C8()) {
        case 0:
            if (GET_INFTABLE_OOT(INFTABLE_B0_Oot)) {
                this->actor.textId = TEXT_OOT(0x304B);
            } else {
                this->actor.textId = TEXT_OOT(0x304A);
            }
            break;
        case 1:
            if (GET_INFTABLE_OOT(INFTABLE_B1_Oot)) {
                this->actor.textId = TEXT_OOT(0x304F);
            } else {
                this->actor.textId = TEXT_OOT(0x304C);
            }
            break;
        case 2:
            this->actor.textId = TEXT_OOT(0x304E);
            break;
        case 3:
            this->actor.textId = TEXT_OOT(0x304D);
            break;
    }
}

static void func_80A3DB04(EnGm* this, PlayState* play) {
    f32 dx;
    f32 dz;
    Player* player = GET_PLAYER(play);

    dx = this->talkPos.x - player->actor.world.pos.x;
    dz = this->talkPos.z - player->actor.world.pos.z;

    if (Flags_GetSwitch(play, this->actor.params)) {
        EnGm_SetTextID(this);
        this->actionFunc = func_80A3DC44;
    } else if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = func_80A3DBF4;
    } else if ((this->collider.base.ocFlags1 & OC1_HIT) || (SQ(dx) + SQ(dz)) < SQ(100.0f)) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_OfferTalk(&this->actor, play, 415.0f);
    }
}

static void func_80A3DBF4(EnGm* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        this->actionFunc = func_80A3DB04;
    }
}

static void func_80A3DC44(EnGm* this, PlayState* play) {
    f32 dx;
    f32 dz;
    s32 pad;
    Player* player = GET_PLAYER(play);

    EnGm_SetTextID(this);

    dx = this->talkPos.x - player->actor.world.pos.x;
    dz = this->talkPos.z - player->actor.world.pos.z;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        switch (func_80A3D7C8()) {
            case 0:
                SET_INFTABLE_OOT(INFTABLE_B0_Oot);
            case 3:
                this->actionFunc = func_80A3DD7C;
                return;
            case 1:
                SET_INFTABLE_OOT(INFTABLE_B1_Oot);
            case 2:
                this->actionFunc = EnGm_ProcessChoiceIndex;
            default:
                return;
        }

        this->actionFunc = EnGm_ProcessChoiceIndex;
    }
    if ((this->collider.base.ocFlags1 & OC1_HIT) || (SQ(dx) + SQ(dz)) < SQ(100.0f)) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_OfferTalk(&this->actor, play, 415.0f);
    }
}

static void func_80A3DD7C(EnGm* this, PlayState* play) {
    u8 dialogState = Message_GetState(&play->msgCtx);

    if ((dialogState == TEXT_STATE_DONE || dialogState == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        this->actionFunc = func_80A3DC44;
        if (dialogState == TEXT_STATE_EVENT) {
            play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
            play->msgCtx.stateTimer = 4;
        }
    }
}

static void EnGm_ProcessChoiceIndex(EnGm* this, PlayState* play) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0: // yes
                if ((gSaveContext.save.saveInfo.playerData.rupees < 200)) {
                    Message_ContinueTextbox(play, TEXT_OOT(0xC8));
                    this->actionFunc = func_80A3DD7C;
                } else {
                    if ((true)) {
                        Actor_OfferGetItem(&this->actor, play, GI_SWORD_KNIFE_Oot, 415.0f, 10.0f);
                        this->actionFunc = func_80A3DF00;
                    }
                }
                break;
            case 1: // no
                Message_ContinueTextbox(play, TEXT_OOT(0x3050));
                this->actionFunc = func_80A3DD7C;
                break;
        }
    }
}

static void func_80A3DF00(EnGm* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play)) {
        this->actor.parent = NULL;
        this->actionFunc = func_80A3DF60;
    } else {
        Actor_OfferGetItem(&this->actor, play, GI_SWORD_KNIFE_Oot, 415.0f, 10.0f);
    }
}

static void func_80A3DF60(EnGm* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        Rupees_ChangeBy(-200);
        this->actionFunc = func_80A3DC44;
    }
}

static void func_80A3DFBC(EnGm* this, PlayState* play) {
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objGmBankIndex].segment);
    this->timer++;
    this->actionFunc(this, play);
    this->actor.focus.rot.x = this->actor.world.rot.x;
    this->actor.focus.rot.y = this->actor.world.rot.y;
    this->actor.focus.rot.z = this->actor.world.rot.z;
    EnGm_UpdateEye(this);
    SkelAnime_Update(&this->skelAnime);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static void EnGm_Update(Actor* thisx, PlayState* play) {
    EnGm* this = (EnGm*)thisx;

    this->updateFunc(this, play);
}

static void func_80A3E090(EnGm* this) {
    Vec3f vec1;
    Vec3f vec2;

    Matrix_Push();
    Matrix_Translate(0.0f, 0.0f, 2600.0f, MTXMODE_APPLY);
    Matrix_RotateZYX(this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, MTXMODE_APPLY);
    vec1.x = vec1.y = vec1.z = 0.0f;
    Matrix_MultVec3f(&vec1, &vec2);
    this->collider.dim.pos.x = vec2.x;
    this->collider.dim.pos.y = vec2.y;
    this->collider.dim.pos.z = vec2.z;
    Matrix_Pop();
    Matrix_Push();
    Matrix_Translate(0.0f, 0.0f, 4300.0f, MTXMODE_APPLY);
    Matrix_RotateZYX(this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, MTXMODE_APPLY);
    vec1.x = vec1.y = vec1.z = 0.0f;
    Matrix_MultVec3f(&vec1, &this->talkPos);
    Matrix_Pop();
    Matrix_Translate(0.0f, 0.0f, 3800.0f, MTXMODE_APPLY);
    Matrix_RotateZYX(this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, MTXMODE_APPLY);
    vec1.x = vec1.y = vec1.z = 0.0f;
    Matrix_MultVec3f(&vec1, &this->actor.focus.pos);
    this->actor.focus.pos.y += 100.0f;
}

static void EnGm_Draw(Actor* thisx, PlayState* play) {
    static void* eyeTextures[] = { gGoronCsEyeOpenTex, gGoronCsEyeHalfTex, gGoronCsEyeClosedTex };
    EnGm* this = (EnGm*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTextures[this->eyeTexIndex]));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gGoronCsMouthNeutralTex));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, &this->actor);

    CLOSE_DISPS(play->state.gfxCtx);

    func_80A3E090(this);
}
