/*
 * File: z_obj_makekinsuta.c
 * Overlay: ovl_Obj_Makekinsuta
 * Description: Skulltula Sprouting from Bean Spot
 */

#include "z_obj_makekinsuta.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
bool func_800B7200(Player* player); // MM's name for OoT's func_8002DEEC (z_actor.c), which no header declares


#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void ObjMakekinsuta_Init(Actor* thisx, PlayState* play);
static void ObjMakekinsuta_Update(Actor* thisx, PlayState* play);

static void func_80B98320(ObjMakekinsuta* this, PlayState* play);
static void ObjMakekinsuta_DoNothing(ObjMakekinsuta* this, PlayState* play);

ActorProfile Obj_Makekinsuta_Oot_Profile = {
    /**/ ACTOR_OBJ_MAKEKINSUTA_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ObjMakekinsuta),
    /**/ ObjMakekinsuta_Init,
    /**/ Actor_Noop,
    /**/ ObjMakekinsuta_Update,
    /**/ NULL,
};

static void ObjMakekinsuta_Init(Actor* thisx, PlayState* play) {
    ObjMakekinsuta* this = (ObjMakekinsuta*)thisx;

    if ((this->actor.params & 0x6000) == 0x4000) {
    } else {
    }
    this->actionFunc = func_80B98320;
}

static void func_80B98320(ObjMakekinsuta* this, PlayState* play) {
    if (this->unk_152 != 0) {
        if ((this->timer >= 60) && !func_800B7200(GET_PLAYER(play))) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_SW_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                        this->actor.world.pos.z, 0, this->actor.shape.rot.y, 0, (this->actor.params | 0x8000));
            this->actionFunc = ObjMakekinsuta_DoNothing;
        } else {
            this->timer++;
        }
    } else {
        this->timer = 0;
    }
}

static void ObjMakekinsuta_DoNothing(ObjMakekinsuta* this, PlayState* play) {
}

static void ObjMakekinsuta_Update(Actor* thisx, PlayState* play) {
    ObjMakekinsuta* this = (ObjMakekinsuta*)thisx;

    this->actionFunc(this, play);
}
