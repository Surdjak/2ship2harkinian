/*
 * File: z_bg_jya_1flift.c
 * Overlay: ovl_Bg_Jya_1flift
 * Description: Shortcut Elevator used in the vanilla version of the Spirit Temple
 */

#include "z_bg_jya_1flift.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_jya_obj/object_jya_obj.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgJya1flift_Init(Actor* thisx, PlayState* play);
static void BgJya1flift_Destroy(Actor* thisx, PlayState* play);
static void BgJya1flift_Update(Actor* thisx, PlayState* play);
static void BgJya1flift_Draw(Actor* thisx, PlayState* play);

static void BgJya1flift_SetupWaitForSwitch(BgJya1flift* this);
static void BgJya1flift_WaitForSwitch(BgJya1flift* this, PlayState* play);
static void BgJya1flift_DoNothing(BgJya1flift* this, PlayState* play);
static void BgJya1flift_ChangeDirection(BgJya1flift* this);
static void BgJya1flift_Move(BgJya1flift* this, PlayState* play);
static void BgJya1flift_SetupDoNothing(BgJya1flift* this);
static void BgJya1flift_ResetMoveDelay(BgJya1flift* this);
static void BgJya1flift_DelayMove(BgJya1flift* this, PlayState* play);

static u8 sKankyoIsSpawned = false;

ActorProfile Bg_Jya_1flift_Oot_Profile = {
    /**/ ACTOR_BG_JYA_1FLIFT_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_JYA_OBJ_Oot,
    /**/ sizeof(BgJya1flift),
    /**/ BgJya1flift_Init,
    /**/ BgJya1flift_Destroy,
    /**/ BgJya1flift_Update,
    /**/ BgJya1flift_Draw,
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
    { 70, 80, -82, { 0, 0, 0 } },
};

static f32 sFinalPositions[] = { 443.0f, -50.0f };

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 400, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1200, ICHAIN_STOP),
};

static void BgJya1flift_InitDynapoly(BgJya1flift* this, PlayState* play, CollisionHeader* collision, s32 moveFlag) {
    s32 pad;
    CollisionHeader* colHeader = NULL;
    s32 pad2;

    DynaPolyActor_Init(&this->dyna, moveFlag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgJya1flift_InitCollision(Actor* thisx, PlayState* play) {
    BgJya1flift* this = (BgJya1flift*)thisx;

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->dyna.actor, &sCylinderInit);
    this->dyna.actor.colChkInfo.mass = MASS_IMMOVABLE;
}

static void BgJya1flift_Init(Actor* thisx, PlayState* play) {
    BgJya1flift* this = (BgJya1flift*)thisx;
    this->hasInitialized = false;
    if (sKankyoIsSpawned) {
        Actor_Kill(thisx);
        return;
    }
    BgJya1flift_InitDynapoly(this, play, &g1fliftCol, 0);
    Actor_ProcessInitChain(thisx, sInitChain);
    BgJya1flift_InitCollision(thisx, play);
    if (Flags_GetSwitch(play, (thisx->params & 0x3F))) {
        LINK_AGE_IN_YEARS_OOT == YEARS_ADULT ? BgJya1flift_ChangeDirection(this) : BgJya1flift_SetupDoNothing(this);
    } else {
        BgJya1flift_SetupWaitForSwitch(this);
    }
    thisx->room = -1;
    sKankyoIsSpawned = true;
    this->hasInitialized = true;
}

static void BgJya1flift_Destroy(Actor* thisx, PlayState* play) {
    BgJya1flift* this = (BgJya1flift*)thisx;

    if (this->hasInitialized) {
        sKankyoIsSpawned = false;
        Collider_DestroyCylinder(play, &this->collider);
        DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    }
}

static void BgJya1flift_SetupWaitForSwitch(BgJya1flift* this) {
    this->actionFunc = BgJya1flift_WaitForSwitch;
    this->dyna.actor.world.pos.y = sFinalPositions[0];
}

static void BgJya1flift_WaitForSwitch(BgJya1flift* this, PlayState* play) {
    if (Flags_GetSwitch(play, (this->dyna.actor.params & 0x3F))) {
        BgJya1flift_ChangeDirection(this);
    }
}

static void BgJya1flift_SetupDoNothing(BgJya1flift* this) {
    this->actionFunc = BgJya1flift_DoNothing;
    this->dyna.actor.world.pos.y = sFinalPositions[0];
}

static void BgJya1flift_DoNothing(BgJya1flift* this, PlayState* play) {
}

static void BgJya1flift_ChangeDirection(BgJya1flift* this) {
    this->actionFunc = BgJya1flift_Move;
    this->isMovingDown ^= true;
    this->dyna.actor.velocity.y = 0.0f;
}

static void BgJya1flift_Move(BgJya1flift* this, PlayState* play) {
    f32 tempVelocity;

    Math_StepToF(&this->dyna.actor.velocity.y, 6.0f, 0.4f);
    if (this->dyna.actor.velocity.y < 1.0f) {
        tempVelocity = 1.0f;
    } else {
        tempVelocity = this->dyna.actor.velocity.y;
    }
    if (fabsf(Math_SmoothStepToF(&this->dyna.actor.world.pos.y, (sFinalPositions[this->isMovingDown]), 0.5f,
                                 tempVelocity, 1.0f)) < 0.001f) {
        this->dyna.actor.world.pos.y = sFinalPositions[this->isMovingDown];
        BgJya1flift_ResetMoveDelay(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BLOCK_BOUND);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_ELEVATOR_MOVE3 - SFX_FLAG);
    }
}

static void BgJya1flift_ResetMoveDelay(BgJya1flift* this) {
    this->actionFunc = BgJya1flift_DelayMove;
    this->moveDelay = 0;
}

static void BgJya1flift_DelayMove(BgJya1flift* this, PlayState* play) {
    this->moveDelay++;
    if (this->moveDelay >= 21) {
        BgJya1flift_ChangeDirection(this);
    }
}

static void BgJya1flift_Update(Actor* thisx, PlayState* play2) {
    BgJya1flift* this = (BgJya1flift*)thisx;
    PlayState* play = play2;
    s32 tempIsRiding;

    // Room 0 is the first room and 6 is the room that the lift starts on
    if (play->roomCtx.curRoom.num == 6 || play->roomCtx.curRoom.num == 0) {
        this->actionFunc(this, play);
        tempIsRiding = DynaPolyActor_IsPlayerOnTop(&this->dyna) ? true : false;
        if ((this->actionFunc == BgJya1flift_Move) || (this->actionFunc == BgJya1flift_DelayMove)) {
            if (tempIsRiding) {
                Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON0);
            } else if (!tempIsRiding && this->isLinkRiding) {
                Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON0);
            }
        }
        this->isLinkRiding = tempIsRiding;
        Collider_UpdateCylinder(thisx, &this->collider);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    } else {
        Actor_Kill(thisx);
    }
}

static void BgJya1flift_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, g1fliftDL);
}
