/*
 * File: z_bg_spot09_obj.c
 * Overlay: ovl_Bg_Spot09_Obj
 * Description:
 */

#include "z_bg_spot09_obj.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_spot09_obj/object_spot09_obj.h"

#define FLAGS 0

static void BgSpot09Obj_Init(Actor* thisx, PlayState* play);
static void BgSpot09Obj_Destroy(Actor* thisx, PlayState* play);
static void BgSpot09Obj_Update(Actor* thisx, PlayState* play);
static void BgSpot09Obj_Draw(Actor* thisx, PlayState* play);

static s32 func_808B1AE0(BgSpot09Obj* this, PlayState* play);
static s32 func_808B1BA0(BgSpot09Obj* this, PlayState* play);
static s32 func_808B1BEC(BgSpot09Obj* this, PlayState* play);

ActorProfile Bg_Spot09_Obj_Oot_Profile = {
    /**/ ACTOR_BG_SPOT09_OBJ_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SPOT09_OBJ_Oot,
    /**/ sizeof(BgSpot09Obj),
    /**/ BgSpot09Obj_Init,
    /**/ BgSpot09Obj_Destroy,
    /**/ BgSpot09Obj_Update,
    /**/ BgSpot09Obj_Draw,
};

static CollisionHeader* D_808B1F90[] = {
    NULL, &gValleyObjects1Col, &gValleyObjects2Col, &gValleyObjects3Col, &gValleyObjects4Col,
};

static s32 (*D_808B1FA4[])(BgSpot09Obj* this, PlayState* play) = {
    func_808B1BEC,
    func_808B1AE0,
    func_808B1BA0,
};

static InitChainEntry sInitChain1[] = {
    ICHAIN_F32(cullingVolumeDistance, 7200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 7200, ICHAIN_STOP),
};

static InitChainEntry sInitChain2[] = {
    ICHAIN_F32(cullingVolumeDistance, 7200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 800, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1500, ICHAIN_STOP),
};

static Gfx* sDLists[] = {
    gValleyBridgeSidesDL, gValleyBrokenBridgeDL, gValleyBridgeChildDL, gCarpentersTentDL, gValleyRepairedBridgeDL,
};

static s32 func_808B1AE0(BgSpot09Obj* this, PlayState* play) {
    s32 carpentersRescued;

    if (gSaveContext.sceneLayer >= 4) {
        return this->dyna.actor.params == 0;
    }

    carpentersRescued = GET_EVENTCHKINF_CARPENTERS_FREE_ALL_OOT();

    if (LINK_AGE_IN_YEARS_OOT == YEARS_ADULT) {
        switch (this->dyna.actor.params) {
            case 0:
                return 0;
            case 1:
                return !carpentersRescued;
            case 4:
                return carpentersRescued;
            case 3:
                return 1;
        }
    } else {
        return this->dyna.actor.params == 2;
    }

    return 0;
}

static s32 func_808B1BA0(BgSpot09Obj* this, PlayState* play) {
    if (this->dyna.actor.params == 3) {
        Actor_SetScale(&this->dyna.actor, 0.1f);
    } else {
        Actor_SetScale(&this->dyna.actor, 1.0f);
    }
    return 1;
}

static s32 func_808B1BEC(BgSpot09Obj* this, PlayState* play) {
    s32 pad;
    CollisionHeader* colHeader = NULL;
    s32 pad2[2];

    if (D_808B1F90[this->dyna.actor.params] != NULL) {
        DynaPolyActor_Init(&this->dyna, 0);
        CollisionHeader_GetVirtual(D_808B1F90[this->dyna.actor.params], &colHeader);
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    }
    return true;
}

static s32 func_808B1C70(BgSpot09Obj* this, PlayState* play) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(D_808B1FA4); i++) {
        if (!D_808B1FA4[i](this, play)) {
            return false;
        }
    }
    return true;
}

static s32 func_808B1CEC(BgSpot09Obj* this, PlayState* play) {
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain1);
    return true;
}

static s32 func_808B1D18(BgSpot09Obj* this, PlayState* play) {
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain2);
    return true;
}

static s32 func_808B1D44(BgSpot09Obj* this, PlayState* play) {
    if (this->dyna.actor.params == 3) {
        return func_808B1D18(this, play);
    } else {
        return func_808B1CEC(this, play);
    }
}

static void BgSpot09Obj_Init(Actor* thisx, PlayState* play) {
    BgSpot09Obj* this = (BgSpot09Obj*)thisx;

    this->dyna.actor.params &= 0xFF;
    if ((this->dyna.actor.params < 0) || (this->dyna.actor.params >= 5)) {
    }

    if (!func_808B1C70(this, play)) {
        Actor_Kill(&this->dyna.actor);
    } else if (!func_808B1D44(this, play)) {
        Actor_Kill(&this->dyna.actor);
    }
}

static void BgSpot09Obj_Destroy(Actor* thisx, PlayState* play) {
    DynaCollisionContext* dynaColCtx = &play->colCtx.dyna;
    BgSpot09Obj* this = (BgSpot09Obj*)thisx;

    if (this->dyna.actor.params != 0) {
        DynaPoly_DeleteBgActor(play, dynaColCtx, this->dyna.bgId);
    }
}

static void BgSpot09Obj_Update(Actor* thisx, PlayState* play) {
}

static void BgSpot09Obj_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, sDLists[thisx->params]);

    if (thisx->params == 3) {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Xlu(play->state.gfxCtx);

        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gCarpentersTentEntranceDL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
