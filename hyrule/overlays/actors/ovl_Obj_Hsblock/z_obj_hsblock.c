/*
 * File: z_obj_hsblock.c
 * Overlay: ovl_Obj_Hsblock
 * Description: Stone Hookshot Target
 */

#include "z_obj_hsblock.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_d_hsblock/object_d_hsblock.h"

#define FLAGS 0

static void ObjHsblock_Init(Actor* thisx, PlayState* play);
static void ObjHsblock_Destroy(Actor* thisx, PlayState* play);
static void ObjHsblock_Update(Actor* thisx, PlayState* play);
static void ObjHsblock_Draw(Actor* thisx, PlayState* play);

static void func_80B93DF4(ObjHsblock* this, PlayState* play);
static void func_80B93E5C(ObjHsblock* this, PlayState* play);

static void func_80B93D90(ObjHsblock* this);
static void func_80B93DB0(ObjHsblock* this);
static void func_80B93E38(ObjHsblock* this);

ActorProfile Obj_Hsblock_Oot_Profile = {
    /**/ ACTOR_OBJ_HSBLOCK_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_D_HSBLOCK_Oot,
    /**/ sizeof(ObjHsblock),
    /**/ ObjHsblock_Init,
    /**/ ObjHsblock_Destroy,
    /**/ ObjHsblock_Update,
    /**/ ObjHsblock_Draw,
};

static f32 D_80B940C0[] = { 85.0f, 85.0f, 0.0f };

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 2000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 400, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 2000, ICHAIN_STOP),
};

static CollisionHeader* sCollisionHeaders[] = { &gHookshotPostCol, &gHookshotPostCol, &gHookshotTargetCol };

static Color_RGB8 sFireTempleColor = { 165, 125, 55 };

static Gfx* sDLists[] = { gHookshotPostDL, gHookshotPostDL, gHookshotTargetDL };

static void ObjHsblock_SetupAction(ObjHsblock* this, ObjHsblockActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void func_80B93B68(ObjHsblock* this, PlayState* play, CollisionHeader* collision, s32 movementFlags) {
    s32 pad;
    CollisionHeader* colHeader = NULL;
    s32 pad2[2];

    DynaPolyActor_Init(&this->dyna, movementFlags);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void func_80B93BF0(ObjHsblock* this, PlayState* play) {
    if ((this->dyna.actor.params >> 5) & 1) {
        Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_OBJ_ICE_POLY_Oot, this->dyna.actor.world.pos.x,
                           this->dyna.actor.world.pos.y, this->dyna.actor.world.pos.z, this->dyna.actor.world.rot.x,
                           this->dyna.actor.world.rot.y, this->dyna.actor.world.rot.z, 1);
    }
}

static void ObjHsblock_Init(Actor* thisx, PlayState* play) {
    ObjHsblock* this = (ObjHsblock*)thisx;

    func_80B93B68(this, play, sCollisionHeaders[thisx->params & 3], 0);
    Actor_ProcessInitChain(thisx, sInitChain);
    func_80B93BF0(this, play);

    switch (thisx->params & 3) {
        case 0:
        case 2:
            func_80B93D90(this);
            break;
        case 1:
            if (Flags_GetSwitch(play, (thisx->params >> 8) & 0x3F)) {
                func_80B93D90(this);
            } else {
                func_80B93DB0(this);
            }
    }

    mREG(13) = 255;
    mREG(14) = 255;
    mREG(15) = 255;
}

static void ObjHsblock_Destroy(Actor* thisx, PlayState* play) {
    ObjHsblock* this = (ObjHsblock*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_80B93D90(ObjHsblock* this) {
    ObjHsblock_SetupAction(this, NULL);
}

static void func_80B93DB0(ObjHsblock* this) {
    this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - 105.0f;
    ObjHsblock_SetupAction(this, func_80B93DF4);
}

static void func_80B93DF4(ObjHsblock* this, PlayState* play) {
    if (Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
        func_80B93E38(this);
    }
}

static void func_80B93E38(ObjHsblock* this) {
    ObjHsblock_SetupAction(this, func_80B93E5C);
}

static void func_80B93E5C(ObjHsblock* this, PlayState* play) {
    Math_SmoothStepToF(&this->dyna.actor.velocity.y, 16.0f, 0.1f, 0.8f, 0.0f);
    if (fabsf(Math_SmoothStepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, 0.3f,
                                 this->dyna.actor.velocity.y, 0.3f)) < 0.001f) {
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y;
        func_80B93D90(this);
        this->dyna.actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    }
}

static void ObjHsblock_Update(Actor* thisx, PlayState* play) {
    ObjHsblock* this = (ObjHsblock*)thisx;

    if (this->actionFunc != NULL) {
        this->actionFunc(this, play);
    }
    Actor_SetFocus(thisx, D_80B940C0[thisx->params & 3]);
}

static void ObjHsblock_Draw(Actor* thisx, PlayState* play) {
    Color_RGB8* color;
    Color_RGB8 defaultColor;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    if (play->sceneId == SCENE_FIRE_TEMPLE_Oot) {
        color = &sFireTempleColor;
    } else {
        defaultColor.r = mREG(13);
        defaultColor.g = mREG(14);
        defaultColor.b = mREG(15);
        color = &defaultColor;
    }

    gDPSetEnvColor(POLY_OPA_DISP++, color->r, color->g, color->b, 255);
    gSPDisplayList(POLY_OPA_DISP++, sDLists[thisx->params & 3]);

    CLOSE_DISPS(play->state.gfxCtx);
}
