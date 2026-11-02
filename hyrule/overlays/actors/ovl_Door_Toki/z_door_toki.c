/*
 * File: z_door_toki.c
 * Overlay: ovl_Door_Toki
 * Description: Door of Time Collision
 */

#include "z_door_toki.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_toki_objects/object_toki_objects.h"

#define FLAGS 0

static void DoorToki_Init(Actor* thisx, PlayState* play);
static void DoorToki_Destroy(Actor* thisx, PlayState* play);
static void DoorToki_Update(Actor* thisx, PlayState* play);

ActorProfile Door_Toki_Oot_Profile = {
    /**/ ACTOR_DOOR_TOKI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_TOKI_OBJECTS_Oot,
    /**/ sizeof(DoorToki),
    /**/ DoorToki_Init,
    /**/ DoorToki_Destroy,
    /**/ DoorToki_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static void DoorToki_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    DoorToki* this = (DoorToki*)thisx;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gDoorTokiCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
}

static void DoorToki_Destroy(Actor* thisx, PlayState* play) {
    DoorToki* this = (DoorToki*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void DoorToki_Update(Actor* thisx, PlayState* play) {
    DoorToki* this = (DoorToki*)thisx;

    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OPENED_THE_DOOR_OF_TIME_Oot)) {
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    } else {
        DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    }
}
