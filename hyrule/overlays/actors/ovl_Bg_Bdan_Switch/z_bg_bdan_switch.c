/*
 * File: z_bg_bdan_switch.c
 * Overlay: ovl_Bg_Bdan_Switch
 * Description: Switches Inside Lord Jabu-Jabu
 */

#include "z_bg_bdan_switch.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64rumble.h"
#include "assets/oot/objects/object_bdan_objects/object_bdan_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgBdanSwitch_Init(Actor* thisx, PlayState* play);
static void BgBdanSwitch_Destroy(Actor* thisx, PlayState* play);
static void BgBdanSwitch_Update(Actor* thisx, PlayState* play);
static void BgBdanSwitch_Draw(Actor* thisx, PlayState* play);

static void func_8086D5C4(BgBdanSwitch* this);
static void func_8086D5E0(BgBdanSwitch* this, PlayState* play);
static void func_8086D67C(BgBdanSwitch* this);
static void func_8086D694(BgBdanSwitch* this, PlayState* play);
static void func_8086D730(BgBdanSwitch* this);
static void func_8086D754(BgBdanSwitch* this, PlayState* play);
static void func_8086D7FC(BgBdanSwitch* this);
static void func_8086D80C(BgBdanSwitch* this, PlayState* play);
static void func_8086D86C(BgBdanSwitch* this);
static void func_8086D888(BgBdanSwitch* this, PlayState* play);
static void func_8086D8BC(BgBdanSwitch* this);
static void func_8086D8CC(BgBdanSwitch* this, PlayState* play);
static void func_8086D95C(BgBdanSwitch* this, PlayState* play);
static void func_8086D9F8(BgBdanSwitch* this);
static void func_8086DA1C(BgBdanSwitch* this, PlayState* play);
static void func_8086DAB4(BgBdanSwitch* this);
static void func_8086DAC4(BgBdanSwitch* this, PlayState* play);
static void func_8086DB24(BgBdanSwitch* this);
static void func_8086DB40(BgBdanSwitch* this, PlayState* play);
static void func_8086DB4C(BgBdanSwitch* this);
static void func_8086DB68(BgBdanSwitch* this, PlayState* play);
static void func_8086DC30(BgBdanSwitch* this);
static void func_8086DC48(BgBdanSwitch* this, PlayState* play);
static void func_8086DCCC(BgBdanSwitch* this);
static void func_8086DCE8(BgBdanSwitch* this, PlayState* play);
static void func_8086DDA8(BgBdanSwitch* this);
static void func_8086DDC0(BgBdanSwitch* this, PlayState* play);

ActorProfile Bg_Bdan_Switch_Oot_Profile = {
    /**/ ACTOR_BG_BDAN_SWITCH_Oot,
    /**/ ACTORCAT_SWITCH,
    /**/ FLAGS,
    /**/ OBJECT_BDAN_OBJECTS_Oot,
    /**/ sizeof(BgBdanSwitch),
    /**/ BgBdanSwitch_Init,
    /**/ BgBdanSwitch_Destroy,
    /**/ BgBdanSwitch_Update,
    /**/ BgBdanSwitch_Draw,
};

static ColliderJntSphElementInit sJntSphElementsInit[] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0xEFC1FFFE), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON,
            OCELEM_ON,
        },
        { 0, { { 0, 120, 0 }, 370 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeDistance, 1400, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1200, ICHAIN_STOP),
};

static Vec3f D_8086E0E0 = { 0.0f, 140.0f, 0.0f };

static void BgBdanSwitch_InitDynaPoly(BgBdanSwitch* this, PlayState* play, CollisionHeader* collision, s32 flag) {
    s16 pad1;
    CollisionHeader* colHeader = NULL;
    s16 pad2;

    DynaPolyActor_Init(&this->dyna, flag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgBdanSwitch_InitCollision(BgBdanSwitch* this, PlayState* play) {
    Actor* actor = &this->dyna.actor;
    Collider_InitJntSph(play, &this->collider);
    Collider_SetJntSph(play, &this->collider, actor, &sJntSphInit, this->colliderItems);
}

static void func_8086D0EC(BgBdanSwitch* this) {
    if (this->unk_1CC > 0) {
        this->unk_1CC += 0x5DC;
    } else {
        this->unk_1CC += 0xFA0;
    }

    switch (this->dyna.actor.params & 0xFF) {
        case BLUE:
        case YELLOW_HEAVY:
        case YELLOW:
            this->unk_1D4 = ((Math_CosS(this->unk_1CC) * 0.5f) + (53.000004f / 6.0f)) * 0.012f;
            this->unk_1D0 = ((Math_CosS(this->unk_1CC) * 0.5f) + 20.5f) * (this->unk_1C8 * 0.0050000004f);
            this->dyna.actor.scale.y = this->unk_1C8 * 0.1f;
            break;
        case YELLOW_TALL_1:
        case YELLOW_TALL_2:
            this->unk_1D4 = ((Math_CosS(this->unk_1CC) * 0.5f) + (43.0f / 6.0f)) * 0.0075000003f;
            this->unk_1D0 = ((Math_CosS(this->unk_1CC) * 0.5f) + 20.5f) * (this->unk_1C8 * 0.0050000004f);
            this->dyna.actor.scale.y = this->unk_1C8 * 0.1f;
    }
    this->dyna.actor.shape.yOffset = 1.2f / this->unk_1D0;
}

static void BgBdanSwitch_Init(Actor* thisx, PlayState* play) {
    BgBdanSwitch* this = (BgBdanSwitch*)thisx;
    s32 pad;
    s16 type;
    s32 flag;

    type = this->dyna.actor.params & 0xFF;
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    if (type == YELLOW_TALL_1 || type == YELLOW_TALL_2) {
        this->dyna.actor.scale.z = 0.05f;
        this->dyna.actor.scale.x = 0.05f;
    } else {
        this->dyna.actor.scale.z = 0.1f;
        this->dyna.actor.scale.x = 0.1f;
    }
    this->dyna.actor.scale.y = 0.0f;
    Actor_SetFocus(&this->dyna.actor, 10.0f);

    switch (type) {
        case BLUE:
        case YELLOW_HEAVY:
        case YELLOW:
            BgBdanSwitch_InitDynaPoly(this, play, &gJabuFloorSwitchCol, DYNA_TRANSFORM_POS);
            break;
        case YELLOW_TALL_1:
        case YELLOW_TALL_2:
            BgBdanSwitch_InitCollision(this, play);
            this->dyna.actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
            this->dyna.actor.attentionRangeType = 4;
            break;
    }

    flag = Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F);

    switch (type) {
        case BLUE:
        case YELLOW:
            if (flag) {
                func_8086D730(this);
            } else {
                func_8086D5C4(this);
            }
            break;
        case YELLOW_HEAVY:
            if (flag) {
                func_8086DB24(this);
            } else {
                func_8086D86C(this);
            }
            break;
        case YELLOW_TALL_1:
        case YELLOW_TALL_2:
            if (flag) {
                func_8086DCCC(this);
            } else {
                func_8086DB4C(this);
            }
            break;
        default:
            Actor_Kill(&this->dyna.actor);
            return;
    }
}

static void BgBdanSwitch_Destroy(Actor* thisx, PlayState* play) {
    BgBdanSwitch* this = (BgBdanSwitch*)thisx;

    switch (this->dyna.actor.params & 0xFF) {
        case BLUE:
        case YELLOW_HEAVY:
        case YELLOW:
            DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
            break;
        case YELLOW_TALL_1:
        case YELLOW_TALL_2:
            Collider_DestroyJntSph(play, &this->collider);
            break;
    }
}

static void func_8086D4B4(BgBdanSwitch* this, PlayState* play) {
    s32 pad;
    s32 type;

    if (!Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
        type = this->dyna.actor.params & 0xFF;
        Flags_SetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F);
        if ((true)) {
            if (type == BLUE || type == YELLOW_TALL_2) {
                OnePointCutscene_AttentionSetSfxOot(play, &this->dyna.actor, NA_SE_SY_TRE_BOX_APPEAR);
            } else {
                OnePointCutscene_AttentionSetSfxOot(play, &this->dyna.actor, NA_SE_SY_CORRECT_CHIME);
            }
        }
    }
}

static void func_8086D548(BgBdanSwitch* this, PlayState* play) {
    if (Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
        Flags_UnsetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F);
        if ((this->dyna.actor.params & 0xFF) == YELLOW_TALL_2) {
            if ((true)) {
                OnePointCutscene_AttentionSetSfxOot(play, &this->dyna.actor, NA_SE_SY_TRE_BOX_APPEAR);
            }
        }
    }
}

static void func_8086D5C4(BgBdanSwitch* this) {
    this->actionFunc = func_8086D5E0;
    this->unk_1C8 = 1.0f;
}

static void func_8086D5E0(BgBdanSwitch* this, PlayState* play) {
    switch (this->dyna.actor.params & 0xFF) {
        case BLUE:
            if (DynaPolyActor_IsSwitchPressed(&this->dyna)) {
                func_8086D67C(this);
                func_8086D4B4(this, play);
            }
            break;
        case YELLOW:
            if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
                func_8086D67C(this);
                func_8086D4B4(this, play);
            }
            break;
    }
}

static void func_8086D67C(BgBdanSwitch* this) {
    this->actionFunc = func_8086D694;
    this->unk_1DA = 0x64;
}

static void func_8086D694(BgBdanSwitch* this, PlayState* play) {
    if ((Hyrule_GetAttentionCategoryOot() == this->dyna.actor.category) || (this->unk_1DA <= 0)) {
        this->unk_1C8 -= 0.2f;
        if (this->unk_1C8 <= 0.1f) {
            func_8086D730(this);
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
            Rumble_Request(this->dyna.actor.xyzDistToPlayerSq, 0x78, 0x14, 0xA);
        }
    }
}

static void func_8086D730(BgBdanSwitch* this) {
    this->unk_1C8 = 0.1f;
    this->actionFunc = func_8086D754;
    this->unk_1D8 = 6;
}

static void func_8086D754(BgBdanSwitch* this, PlayState* play) {
    switch (this->dyna.actor.params & 0xFF) {
        case BLUE:
            if (!DynaPolyActor_IsSwitchPressed(&this->dyna)) {
                if (this->unk_1D8 <= 0) {
                    func_8086D7FC(this);
                    func_8086D548(this, play);
                }
            } else {
                this->unk_1D8 = 6;
            }
            break;
        case YELLOW:
            if (!Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
                func_8086D7FC(this);
            }
            break;
    }
}

static void func_8086D7FC(BgBdanSwitch* this) {
    this->actionFunc = func_8086D80C;
}

static void func_8086D80C(BgBdanSwitch* this, PlayState* play) {
    this->unk_1C8 += 0.2f;
    if (this->unk_1C8 >= 1.0f) {
        func_8086D5C4(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
    }
}

static void func_8086D86C(BgBdanSwitch* this) {
    this->actionFunc = func_8086D888;
    this->unk_1C8 = 1.0f;
}

static void func_8086D888(BgBdanSwitch* this, PlayState* play) {
    if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        func_8086D8BC(this);
    }
}

static void func_8086D8BC(BgBdanSwitch* this) {
    this->actionFunc = func_8086D8CC;
}

static void func_8086D8CC(BgBdanSwitch* this, PlayState* play) {
    this->unk_1C8 -= 0.2f;
    if (this->unk_1C8 <= 0.6f) {
        func_8086D9F8(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
        Rumble_Request(this->dyna.actor.xyzDistToPlayerSq, 0x78, 0x14, 0xA);
    }
}

static void func_8086D944(BgBdanSwitch* this) {
    this->actionFunc = func_8086D95C;
    this->unk_1DA = 0x64;
}

static void func_8086D95C(BgBdanSwitch* this, PlayState* play) {
    if ((Hyrule_GetAttentionCategoryOot() == this->dyna.actor.category) || (this->unk_1DA <= 0)) {
        this->unk_1C8 -= 0.2f;
        if (this->unk_1C8 <= 0.1f) {
            func_8086DB24(this);
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
            Rumble_Request(this->dyna.actor.xyzDistToPlayerSq, 0x78, 0x14, 0xA);
        }
    }
}

static void func_8086D9F8(BgBdanSwitch* this) {
    this->unk_1C8 = 0.6f;
    this->actionFunc = func_8086DA1C;
    this->unk_1D8 = 6;
}

static void func_8086DA1C(BgBdanSwitch* this, PlayState* play) {
    Actor* heldActor = GET_PLAYER(play)->heldActor;

    if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        if (heldActor != NULL && heldActor->id == ACTOR_EN_RU1_Oot) {
            if (this->unk_1D8 <= 0) {
                func_8086D944(this);
                func_8086D4B4(this, play);
            }
        } else {
            this->unk_1D8 = 6;
        }
    } else {
        if (this->unk_1D8 <= 0) {
            func_8086DAB4(this);
        }
    }
}

static void func_8086DAB4(BgBdanSwitch* this) {
    this->actionFunc = func_8086DAC4;
}

static void func_8086DAC4(BgBdanSwitch* this, PlayState* play) {
    this->unk_1C8 += 0.2f;
    if (this->unk_1C8 >= 1.0f) {
        func_8086D86C(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
    }
}

static void func_8086DB24(BgBdanSwitch* this) {
    this->unk_1C8 = 0.1f;
    this->actionFunc = func_8086DB40;
}

static void func_8086DB40(BgBdanSwitch* this, PlayState* play) {
}

static void func_8086DB4C(BgBdanSwitch* this) {
    this->actionFunc = func_8086DB68;
    this->unk_1C8 = 2.0f;
}

static void func_8086DB68(BgBdanSwitch* this, PlayState* play) {
    switch (this->dyna.actor.params & 0xFF) {
        default:
            return;
        case YELLOW_TALL_1:
            if (((this->collider.base.acFlags & AC_HIT) != 0) && this->unk_1D8 <= 0) {
                this->unk_1D8 = 0xA;
                func_8086DC30(this);
                func_8086D4B4(this, play);
            }
            break;
        case YELLOW_TALL_2:
            if (((this->collider.base.acFlags & AC_HIT) != 0) && ((this->unk_1DC & 2) == 0) && this->unk_1D8 <= 0) {
                this->unk_1D8 = 0xA;
                func_8086DC30(this);
                func_8086D4B4(this, play);
            }
            break;
    }
}

static void func_8086DC30(BgBdanSwitch* this) {
    this->actionFunc = func_8086DC48;
    this->unk_1DA = 0x64;
}

static void func_8086DC48(BgBdanSwitch* this, PlayState* play) {
    if ((Hyrule_GetAttentionCategoryOot() == this->dyna.actor.category) || (this->unk_1DA <= 0)) {
        this->unk_1C8 -= 0.3f;
        if (this->unk_1C8 <= 1.0f) {
            func_8086DCCC(this);
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
        }
    }
}

static void func_8086DCCC(BgBdanSwitch* this) {
    this->actionFunc = func_8086DCE8;
    this->unk_1C8 = 1.0f;
}

static void func_8086DCE8(BgBdanSwitch* this, PlayState* play) {
    switch (this->dyna.actor.params & 0xFF) {
        case YELLOW_TALL_1:
            if (!Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
                func_8086DDA8(this);
            }
            break;
        case YELLOW_TALL_2:
            if (((this->collider.base.acFlags & AC_HIT) != 0) && ((this->unk_1DC & 2) == 0) && (this->unk_1D8 <= 0)) {
                this->unk_1D8 = 0xA;
                func_8086DDA8(this);
                func_8086D548(this, play);
            }
            break;
    }
}

static void func_8086DDA8(BgBdanSwitch* this) {
    this->actionFunc = func_8086DDC0;
    this->unk_1DA = 0x64;
}

static void func_8086DDC0(BgBdanSwitch* this, PlayState* play) {
    if ((((this->dyna.actor.params & 0xFF) != YELLOW_TALL_2) || (Hyrule_GetAttentionCategoryOot() == this->dyna.actor.category)) ||
        (this->unk_1DA <= 0)) {
        this->unk_1C8 += 0.3f;
        if (this->unk_1C8 >= 2.0f) {
            func_8086DB4C(this);
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_FOOT_SWITCH);
        }
    }
}

static void BgBdanSwitch_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgBdanSwitch* this = (BgBdanSwitch*)thisx;
    s32 type;

    if (this->unk_1DA > 0) {
        this->unk_1DA--;
    }
    this->actionFunc(this, play);
    func_8086D0EC(this);
    type = this->dyna.actor.params & 0xFF;
    if (type != 3 && type != 4) {
        this->unk_1D8--;
    } else {
        if (!Player_InCsMode(play) && this->unk_1D8 > 0) {
            this->unk_1D8--;
        }
        this->unk_1DC = this->collider.base.acFlags;
        this->collider.base.acFlags &= ~AC_HIT;
        this->collider.elements[0].dim.modelSphere.radius = this->unk_1D4 * 370.0f;
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void func_8086DF58(BgBdanSwitch* this, PlayState* play, Gfx* dlist) {
    Matrix_SetTranslateRotateYXZ(this->dyna.actor.world.pos.x,
                                 this->dyna.actor.world.pos.y + (this->dyna.actor.shape.yOffset * this->unk_1D0),
                                 this->dyna.actor.world.pos.z, &this->dyna.actor.shape.rot);
    Matrix_Scale(this->unk_1D4, this->unk_1D0, this->unk_1D4, MTXMODE_APPLY);
    Gfx_DrawDListOpa(play, dlist);
}

static void BgBdanSwitch_Draw(Actor* thisx, PlayState* play) {
    BgBdanSwitch* this = (BgBdanSwitch*)thisx;

    switch (this->dyna.actor.params & 0xFF) {
        case YELLOW_HEAVY:
        case YELLOW:
            func_8086DF58(this, play, gJabuYellowFloorSwitchDL);
            break;
        case YELLOW_TALL_1:
        case YELLOW_TALL_2:
            func_8086DF58(this, play, gJabuYellowFloorSwitchDL);
            Collider_UpdateSpheres(0, &this->collider);
            Matrix_MultVec3f(&D_8086E0E0, &this->dyna.actor.focus.pos);
            break;
        case BLUE:
            func_8086DF58(this, play, gJabuBlueFloorSwitchDL);
            break;
    }
}
