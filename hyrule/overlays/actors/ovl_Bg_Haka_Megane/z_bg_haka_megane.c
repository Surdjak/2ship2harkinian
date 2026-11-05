/*
 * File: z_bg_haka_megane.c
 * Overlay: ovl_Bg_Haka_Megane
 * Description: Shadow Temple Fake Walls
 */

#include "z_bg_haka_megane.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_hakach_objects/object_hakach_objects.h"
#include "assets/oot/objects/object_haka_objects/object_haka_objects.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_REACT_TO_LENS)

static void BgHakaMegane_Init(Actor* thisx, PlayState* play);
static void BgHakaMegane_Destroy(Actor* thisx, PlayState* play);
static void BgHakaMegane_Update(Actor* thisx, PlayState* play);
static void BgHakaMegane_Draw(Actor* thisx, PlayState* play);

static void BgHakaMegane_WaitForObject(BgHakaMegane* this, PlayState* play);
static void BgHakaMegane_UpdateState(BgHakaMegane* this, PlayState* play);
static void BgHakaMegane_DoNothing(BgHakaMegane* this, PlayState* play);

ActorProfile Bg_Haka_Megane_Oot_Profile = {
    /**/ ACTOR_BG_HAKA_MEGANE_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(BgHakaMegane),
    /**/ BgHakaMegane_Init,
    /**/ BgHakaMegane_Destroy,
    /**/ BgHakaMegane_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static CollisionHeader* sCollisionHeaders[] = {
    &gBotw1Col,
    &gBotw2Col,
    NULL,
    &object_haka_objects_Col_004330,
    &object_haka_objects_Col_0044D0,
    NULL,
    &object_haka_objects_Col_004780,
    &object_haka_objects_Col_004940,
    NULL,
    &object_haka_objects_Col_004B00,
    NULL,
    &object_haka_objects_Col_004CC0,
    NULL,
};

static Gfx* sDLists[] = {
    gBotwFakeWallsAndFloorsDL,     gBotwThreeFakeFloorsDL,        gBotwHoleTrap2DL,
    object_haka_objects_DL_0040F0, object_haka_objects_DL_0043B0, object_haka_objects_DL_001120,
    object_haka_objects_DL_0045A0, object_haka_objects_DL_0047F0, object_haka_objects_DL_0018F0,
    object_haka_objects_DL_0049B0, object_haka_objects_DL_003CF0, object_haka_objects_DL_004B70,
    object_haka_objects_DL_002ED0,
};

static void BgHakaMegane_Init(Actor* thisx, PlayState* play) {
    BgHakaMegane* this = (BgHakaMegane*)thisx;

    Actor_ProcessInitChain(thisx, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);

    if (thisx->params < 3) {
        this->objectSlot = Object_GetSlot(&play->objectCtx, OBJECT_HAKACH_OBJECTS_Oot);
    } else {
        this->objectSlot = Object_GetSlot(&play->objectCtx, OBJECT_HAKA_OBJECTS_Oot);
    }

    if (this->objectSlot < 0) {
        Actor_Kill(thisx);
    } else {
        this->actionFunc = BgHakaMegane_WaitForObject;
    }
}

static void BgHakaMegane_Destroy(Actor* thisx, PlayState* play) {
    BgHakaMegane* this = (BgHakaMegane*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgHakaMegane_WaitForObject(BgHakaMegane* this, PlayState* play) {
    CollisionHeader* colHeader;
    CollisionHeader* collision;

    if (Object_IsLoaded(&play->objectCtx, this->objectSlot)) {
        this->dyna.actor.objectSlot = this->objectSlot;
        this->dyna.actor.draw = BgHakaMegane_Draw;
        Actor_SetObjectDependency(play, &this->dyna.actor);
        if (play->roomCtx.curRoom.lensMode != LENS_MODE_HIDE_ACTORS) {
            this->actionFunc = BgHakaMegane_UpdateState;
            collision = sCollisionHeaders[this->dyna.actor.params];
            if (collision != NULL) {
                CollisionHeader_GetVirtual(collision, &colHeader);
                this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
            }
        } else {
            this->actionFunc = BgHakaMegane_DoNothing;
        }
    }
}

static void BgHakaMegane_UpdateState(BgHakaMegane* this, PlayState* play) {
    Actor* thisx = &this->dyna.actor;

    if (play->actorCtx.lensActive) {
        thisx->flags |= ACTOR_FLAG_REACT_TO_LENS;
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    } else {
        thisx->flags &= ~ACTOR_FLAG_REACT_TO_LENS;
        DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    }
}

static void BgHakaMegane_DoNothing(BgHakaMegane* this, PlayState* play) {
}

static void BgHakaMegane_Update(Actor* thisx, PlayState* play) {
    BgHakaMegane* this = (BgHakaMegane*)thisx;

    this->actionFunc(this, play);
}

static void BgHakaMegane_Draw(Actor* thisx, PlayState* play) {
    BgHakaMegane* this = (BgHakaMegane*)thisx;

    if (CHECK_FLAG_ALL(thisx->flags, ACTOR_FLAG_REACT_TO_LENS)) {
        Gfx_DrawDListXlu(play, sDLists[thisx->params]);
    } else {
        Gfx_DrawDListOpa(play, sDLists[thisx->params]);
    }

    if (thisx->params == 0) {
        Gfx_DrawDListXlu(play, gBotwBloodSplatterDL);
    }
}
