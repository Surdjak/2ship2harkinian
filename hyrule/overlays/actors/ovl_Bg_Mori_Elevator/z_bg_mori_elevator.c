#include "z_bg_mori_elevator.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_mori_objects/object_mori_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgMoriElevator_Init(Actor* thisx, PlayState* play);
static void BgMoriElevator_Destroy(Actor* thisx, PlayState* play);
static void BgMoriElevator_Update(Actor* thisx, PlayState* play);

static void BgMoriElevator_SetupWaitAfterInit(BgMoriElevator* this);
static void BgMoriElevator_WaitAfterInit(BgMoriElevator* this, PlayState* play);
static void BgMoriElevator_SetupSetPosition(BgMoriElevator* this);
static void BgMoriElevator_SetPosition(BgMoriElevator* this, PlayState* play);
static void BgMoriElevator_Draw(Actor* thisx, PlayState* play);
static void BgMoriElevator_StopMovement(BgMoriElevator* this);
static void func_808A2008(BgMoriElevator* this, PlayState* play);
static void BgMoriElevator_MoveIntoGround(BgMoriElevator* this, PlayState* play);
static void BgMoriElevator_MoveAboveGround(BgMoriElevator* this, PlayState* play);

static s16 sKankyoIsSpawned = false;

ActorProfile Bg_Mori_Elevator_Oot_Profile = {
    /**/ ACTOR_BG_MORI_ELEVATOR_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_MORI_OBJECTS_Oot,
    /**/ sizeof(BgMoriElevator),
    /**/ BgMoriElevator_Init,
    /**/ BgMoriElevator_Destroy,
    /**/ BgMoriElevator_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeDistance, 2000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 3000, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static f32 func_808A1800(f32* pValue, f32 target, f32 scale, f32 maxStep, f32 minStep) {
    f32 var = (target - *pValue) * scale;

    if (*pValue < target) {
        if (maxStep < var) {
            var = maxStep;
        } else {
            if (var < minStep) {
                var = minStep;
            }
        }
        *pValue = (*pValue + var);

        if (target < *pValue) {
            *pValue = target;
        }
    } else {
        if (target < *pValue) {
            if (var < (-maxStep)) {
                var = (-maxStep);
            } else {
                if ((-minStep) < var) {
                    var = (-minStep);
                }
            }
            *pValue = (*pValue + var);
            if (*pValue < target) {
                *pValue = target;
            }
        } else {
            var = 0.0f;
        }
    }
    return var;
}

static void func_808A18FC(BgMoriElevator* this, f32 distTo) {
    f32 temp;

    temp = fabsf(distTo) * 0.09f;
    Audio_PlaySfx_AtPosWithFreq(&this->dyna.actor.projectedPos, NA_SE_EV_NAVY_FLY_REBIRTH - SFX_FLAG, CLAMP(temp, 0.0f, 1.0f));
}

static void BgMoriElevator_Init(Actor* thisx, PlayState* play) {
    BgMoriElevator* this = (BgMoriElevator*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    this->unk_172 = sKankyoIsSpawned;
    this->moriTexObjIndex = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (this->moriTexObjIndex < 0) {
        Actor_Kill(thisx);
    } else {
        switch (sKankyoIsSpawned) {
            case false:
                sKankyoIsSpawned = true;
                this->dyna.actor.room = -1;
                Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
                DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
                CollisionHeader_GetVirtual(&gMoriElevatorCol, &colHeader);
                this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
                BgMoriElevator_SetupWaitAfterInit(this);
                break;
            case true:
                Actor_Kill(thisx);
                break;
        }
    }
}

static void BgMoriElevator_Destroy(Actor* thisx, PlayState* play) {
    BgMoriElevator* this = (BgMoriElevator*)thisx;

    if (this->unk_172 == 0) {
        DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
        sKankyoIsSpawned = false;
    }
}

static s32 BgMoriElevator_IsPlayerRiding(BgMoriElevator* this, PlayState* play) {
    return ((this->dyna.interactFlags & DYNA_INTERACT_PLAYER_ON_TOP) &&
            !(this->unk_170 & DYNA_INTERACT_PLAYER_ON_TOP) &&
            ((GET_PLAYER(play)->actor.world.pos.y - this->dyna.actor.world.pos.y) < 80.0f));
}

static void BgMoriElevator_SetupWaitAfterInit(BgMoriElevator* this) {
    this->actionFunc = BgMoriElevator_WaitAfterInit;
}

static void BgMoriElevator_WaitAfterInit(BgMoriElevator* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->moriTexObjIndex)) {
        if (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
            if (play->roomCtx.curRoom.num == 2) {
                this->dyna.actor.world.pos.y = 73.0f;
                BgMoriElevator_SetupSetPosition(this);
            } else {
            }
        } else {
            BgMoriElevator_SetupSetPosition(this);
        }
        this->dyna.actor.draw = BgMoriElevator_Draw;
    }
}

static void func_808A1C30(BgMoriElevator* this) {
    this->actionFunc = BgMoriElevator_MoveIntoGround;
}

static void BgMoriElevator_MoveIntoGround(BgMoriElevator* this, PlayState* play) {
    f32 distToTarget;

    func_808A1800(&this->dyna.actor.velocity.y, 2.0f, 0.05f, 1.0f, 0.0f);
    distToTarget = func_808A1800(&this->dyna.actor.world.pos.y, 73.0f, 0.08f, this->dyna.actor.velocity.y, 1.5f);
    if (fabsf(distToTarget) < 0.001f) {
        BgMoriElevator_SetupSetPosition(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_CHAINLIFT_STOP);
    } else {
        func_808A18FC(this, distToTarget);
    }
}

static void func_808A1CF4(BgMoriElevator* this, PlayState* play) {
    this->actionFunc = BgMoriElevator_MoveAboveGround;
    OnePointCutscene_Init(play, 3230, 70, &this->dyna.actor, CAM_ID_MAIN);
    OnePointCutscene_Init(play, 1020, 15, &this->dyna.actor, CAM_ID_MAIN);
}

static void BgMoriElevator_MoveAboveGround(BgMoriElevator* this, PlayState* play) {
    f32 distToTarget;

    func_808A1800(&this->dyna.actor.velocity.y, 2.0f, 0.05f, 1.0f, 0.0f);
    distToTarget = func_808A1800(&this->dyna.actor.world.pos.y, 233.0f, 0.08f, this->dyna.actor.velocity.y, 1.5f);
    if (fabsf(distToTarget) < 0.001f) {
        BgMoriElevator_SetupSetPosition(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_CHAINLIFT_STOP);
    } else {
        func_808A18FC(this, distToTarget);
    }
}

static void BgMoriElevator_SetupSetPosition(BgMoriElevator* this) {
    this->actionFunc = BgMoriElevator_SetPosition;
}

static void BgMoriElevator_SetPosition(BgMoriElevator* this, PlayState* play) {
    s32 pad;

    if (BgMoriElevator_IsPlayerRiding(this, play)) {
        if (play->roomCtx.curRoom.num == 2) {
            this->targetY = -779.0f;
            BgMoriElevator_StopMovement(this);
        } else if (play->roomCtx.curRoom.num == 17) {
            this->targetY = 233.0f;
            BgMoriElevator_StopMovement(this);
        } else {
        }
    } else if ((play->roomCtx.curRoom.num == 2) && (this->dyna.actor.world.pos.y < -275.0f)) {
        this->targetY = 233.0f;
        BgMoriElevator_StopMovement(this);
    } else if ((play->roomCtx.curRoom.num == 17) && (-275.0f < this->dyna.actor.world.pos.y)) {
        this->targetY = -779.0f;
        BgMoriElevator_StopMovement(this);
    } else if ((play->roomCtx.curRoom.num == 2) && Flags_GetSwitch(play, this->dyna.actor.params & 0x3F) &&
               (this->unk_16C == 0)) {
        this->targetY = 73.0f;
        func_808A1C30(this);
    } else if ((play->roomCtx.curRoom.num == 2) && !Flags_GetSwitch(play, this->dyna.actor.params & 0x3F) &&
               (this->unk_16C != 0)) {
        this->targetY = 233.0f;
        func_808A1CF4(this, play);
    }
}

static void BgMoriElevator_StopMovement(BgMoriElevator* this) {
    this->actionFunc = func_808A2008;
    this->dyna.actor.velocity.y = 0.0f;
}

static void func_808A2008(BgMoriElevator* this, PlayState* play) {
    f32 distTo;

    func_808A1800(&this->dyna.actor.velocity.y, 12.0f, 0.1f, 1.0f, 0.0f);
    distTo = func_808A1800(&this->dyna.actor.world.pos.y, this->targetY, 0.1f, this->dyna.actor.velocity.y, 0.3f);
    if (fabsf(distTo) < 0.001f) {
        BgMoriElevator_SetupSetPosition(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_CHAINLIFT_STOP);

    } else {
        func_808A18FC(this, distTo);
    }
}

static void BgMoriElevator_Update(Actor* thisx, PlayState* play) {
    BgMoriElevator* this = (BgMoriElevator*)thisx;

    this->actionFunc(this, play);
    this->unk_170 = this->dyna.interactFlags;
    this->unk_16C = Flags_GetSwitch(play, (thisx->params & 0x3F));
}

static void BgMoriElevator_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriElevator* this = (BgMoriElevator*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIndex].segment);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gMoriElevatorDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
