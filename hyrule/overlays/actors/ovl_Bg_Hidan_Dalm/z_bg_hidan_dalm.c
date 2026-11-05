/*
 * File: z_bg_hidan_dalm.c
 * Overlay: ovl_Bg_Hidan_Dalm
 * Description: Hammerable Totem Pieces (Fire Temple)
 */

#include "z_bg_hidan_dalm.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_hidan_objects/object_hidan_objects.h"

#define FLAGS 0

static void BgHidanDalm_Init(Actor* thisx, PlayState* play);
static void BgHidanDalm_Destroy(Actor* thisx, PlayState* play);
static void BgHidanDalm_Update(Actor* thisx, PlayState* play);
static void BgHidanDalm_Draw(Actor* thisx, PlayState* play);

static void BgHidanDalm_Wait(BgHidanDalm* this, PlayState* play);
static void BgHidanDalm_Shrink(BgHidanDalm* this, PlayState* play);

ActorProfile Bg_Hidan_Dalm_Oot_Profile = {
    /**/ ACTOR_BG_HIDAN_DALM_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_HIDAN_OBJECTS_Oot,
    /**/ sizeof(BgHidanDalm),
    /**/ BgHidanDalm_Init,
    /**/ BgHidanDalm_Destroy,
    /**/ BgHidanDalm_Update,
    /**/ BgHidanDalm_Draw,
};

static ColliderTrisElementInit sTrisElementInit[4] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000040), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON | ACELEM_NO_AT_INFO | ACELEM_NO_DAMAGE | ACELEM_NO_SWORD_SFX | ACELEM_NO_HITMARK,
            OCELEM_NONE,
        },
        { { { 305.0f, 0.0f, -300.0f }, { 305.0f, 600.0f, -300.0f }, { 305.0f, 600.0f, 300.0f } } },
    },
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000040), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON | ACELEM_NO_AT_INFO | ACELEM_NO_DAMAGE | ACELEM_NO_SWORD_SFX | ACELEM_NO_HITMARK,
            OCELEM_NONE,
        },
        { { { 305.0f, 0.0f, -300.0f }, { 305.0f, 600.0f, 300.0f }, { 305.0f, 0.0f, 300.0f } } },
    },
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000040), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON | ACELEM_NO_AT_INFO | ACELEM_NO_DAMAGE | ACELEM_NO_SWORD_SFX | ACELEM_NO_HITMARK,
            OCELEM_NONE,
        },
        { { { -305.0f, 0.0f, -300.0f }, { -305.0f, 600.0f, 300.0f }, { -305.0f, 600.0f, -300.0f } } },
    },
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000040), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON | ACELEM_NO_AT_INFO | ACELEM_NO_DAMAGE | ACELEM_NO_SWORD_SFX | ACELEM_NO_HITMARK,
            OCELEM_NONE,
        },
        { { { -305.0f, 0.0f, -300.0f }, { -305.0f, 0.0f, 300.0f }, { -305.0f, 600.0f, 300.0f } } },
    },
};

static ColliderTrisInit sTrisInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_TYPE_2,
        COLSHAPE_TRIS,
    },
    4,
    sTrisElementInit,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, -200, ICHAIN_STOP),
};

static void BgHidanDalm_Init(Actor* thisx, PlayState* play) {
    BgHidanDalm* this = (BgHidanDalm*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(thisx, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gFireTempleHammerableTotemCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    Collider_InitTris(play, &this->collider);
    Collider_SetTris(play, &this->collider, thisx, &sTrisInit, this->colliderItems);

    this->switchFlag = (thisx->params >> 8) & 0xFF;
    thisx->params &= 0xFF;
    if (Flags_GetSwitch(play, this->switchFlag)) {
        Actor_Kill(thisx);
    } else {
        this->actionFunc = BgHidanDalm_Wait;
    }
}

static void BgHidanDalm_Destroy(Actor* thisx, PlayState* play) {
    BgHidanDalm* this = (BgHidanDalm*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    Collider_DestroyTris(play, &this->collider);
}

static void BgHidanDalm_Wait(BgHidanDalm* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (((this->collider.base.acFlags & AC_HIT) && !Player_InCsMode(play) &&
                                  (player->meleeWeaponAnimation == 22 || player->meleeWeaponAnimation == 23))) {
        this->collider.base.acFlags &= ~AC_HIT;
        if ((this->collider.elements[0].base.acElemFlags & ACELEM_HIT) ||
            (this->collider.elements[1].base.acElemFlags & ACELEM_HIT)) {
            this->dyna.actor.world.rot.y -= 0x4000;
        } else {
            this->dyna.actor.world.rot.y += 0x4000;
        }
        this->dyna.actor.world.pos.x += 32.5f * Math_SinS(this->dyna.actor.world.rot.y);
        this->dyna.actor.world.pos.z += 32.5f * Math_CosS(this->dyna.actor.world.rot.y);

        Player_SetCsActionWithHaltedActorsOot(play, &this->dyna.actor, 8);
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->actionFunc = BgHidanDalm_Shrink;
        this->dyna.actor.bgCheckFlags &= ~2;
        this->dyna.actor.bgCheckFlags &= ~8;
        this->dyna.actor.speed = 10.0f;
        Flags_SetSwitch(play, this->switchFlag);
        Player_PlaySfx(GET_PLAYER(play), NA_SE_IT_HAMMER_HIT);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_DARUMA_VANISH);
    } else {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void BgHidanDalm_Shrink(BgHidanDalm* this, PlayState* play) {
    static Vec3f accel = { 0.0f, 0.0f, 0.0f };
    s32 i;
    Vec3f velocity;
    Vec3f pos;

    if (Math_StepToF(&this->dyna.actor.scale.x, 0.0f, 0.004f)) {
        Player_SetCsActionWithHaltedActorsOot(play, &this->dyna.actor, 7);
        Actor_Kill(&this->dyna.actor);
    }

    this->dyna.actor.scale.y = this->dyna.actor.scale.z = this->dyna.actor.scale.x;

    pos.x = this->dyna.actor.world.pos.x;
    pos.y = this->dyna.actor.world.pos.y + this->dyna.actor.scale.x * 160.0f;
    pos.z = this->dyna.actor.world.pos.z;

    for (i = 0; i < 4; i++) {
        velocity.x = 5.0f * Math_SinS(this->dyna.actor.world.rot.y + 0x8000) + (Rand_ZeroOne() - 0.5f) * 5.0f;
        velocity.z = 5.0f * Math_CosS(this->dyna.actor.world.rot.y + 0x8000) + (Rand_ZeroOne() - 0.5f) * 5.0f;
        velocity.y = (Rand_ZeroOne() - 0.5f) * 1.5f;
        EffectSsKirakira_SpawnSmallYellow(play, &pos, &velocity, &accel);
    }
}

static void BgHidanDalm_Update(Actor* thisx, PlayState* play) {
    BgHidanDalm* this = (BgHidanDalm*)thisx;

    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->dyna.actor);
    Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 10.0f, 15.0f, 32.0f, 0x405);
}

/**
 * Update vertices of collider tris based on the current matrix
 */
static void BgHidanDalm_UpdateCollider(BgHidanDalm* this) {
    Vec3f pos2;
    Vec3f pos1;
    Vec3f pos0;

    Matrix_MultVec3f(&sTrisElementInit[0].dim.vtx[0], &pos0);
    Matrix_MultVec3f(&sTrisElementInit[0].dim.vtx[1], &pos1);
    Matrix_MultVec3f(&sTrisElementInit[0].dim.vtx[2], &pos2);
    Collider_SetTrisVertices(&this->collider, 0, &pos0, &pos1, &pos2);
    Matrix_MultVec3f(&sTrisElementInit[1].dim.vtx[2], &pos1);
    Collider_SetTrisVertices(&this->collider, 1, &pos0, &pos2, &pos1);

    Matrix_MultVec3f(&sTrisElementInit[2].dim.vtx[0], &pos0);
    Matrix_MultVec3f(&sTrisElementInit[2].dim.vtx[1], &pos1);
    Matrix_MultVec3f(&sTrisElementInit[2].dim.vtx[2], &pos2);
    Collider_SetTrisVertices(&this->collider, 2, &pos0, &pos1, &pos2);
    Matrix_MultVec3f(&sTrisElementInit[3].dim.vtx[1], &pos2);
    Collider_SetTrisVertices(&this->collider, 3, &pos0, &pos2, &pos1);
}

static void BgHidanDalm_Draw(Actor* thisx, PlayState* play) {
    BgHidanDalm* this = (BgHidanDalm*)thisx;

    if (this->dyna.actor.params == 0) {
        Gfx_DrawDListOpa(play, gFireTempleHammerableTotemBodyDL);
    } else {
        Gfx_DrawDListOpa(play, gFireTempleHammerableTotemHeadDL);
    }

    if (this->actionFunc == BgHidanDalm_Wait) {
        BgHidanDalm_UpdateCollider(this);
    }
}
