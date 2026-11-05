/*
 * File: z_bg_jya_amishutter.c
 * Overlay: Bg_Jya_Amishutter
 * Description: Circular metal grate. Lifts up when you get close to it.
 */

#include "z_bg_jya_amishutter.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_jya_obj/object_jya_obj.h"

#define FLAGS 0

static void BgJyaAmishutter_Init(Actor* thisx, PlayState* play);
static void BgJyaAmishutter_Destroy(Actor* thisx, PlayState* play);
static void BgJyaAmishutter_Update(Actor* thisx, PlayState* play);
static void BgJyaAmishutter_Draw(Actor* thisx, PlayState* play);

static void BgJyaAmishutter_SetupWaitForPlayer(BgJyaAmishutter* this);
static void BgJyaAmishutter_WaitForPlayer(BgJyaAmishutter* this);
static void func_80893428(BgJyaAmishutter* this);
static void func_80893438(BgJyaAmishutter* this);
static void func_808934B0(BgJyaAmishutter* this);
static void func_808934C0(BgJyaAmishutter* this);
static void func_808934FC(BgJyaAmishutter* this);
static void func_8089350C(BgJyaAmishutter* this);

ActorProfile Bg_Jya_Amishutter_Oot_Profile = {
    /**/ ACTOR_BG_JYA_AMISHUTTER_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_JYA_OBJ_Oot,
    /**/ sizeof(BgJyaAmishutter),
    /**/ BgJyaAmishutter_Init,
    /**/ BgJyaAmishutter_Destroy,
    /**/ BgJyaAmishutter_Update,
    /**/ BgJyaAmishutter_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1000, ICHAIN_STOP),
};

static void BgJyaAmishutter_InitDynaPoly(BgJyaAmishutter* this, PlayState* play, CollisionHeader* collision, s32 flag) {
    s32 pad1;
    CollisionHeader* colHeader = NULL;
    s32 pad2;

    DynaPolyActor_Init(&this->dyna, flag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgJyaAmishutter_Init(Actor* thisx, PlayState* play) {
    BgJyaAmishutter* this = (BgJyaAmishutter*)thisx;

    BgJyaAmishutter_InitDynaPoly(this, play, &gAmishutterCol, 0);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    BgJyaAmishutter_SetupWaitForPlayer(this);
}

static void BgJyaAmishutter_Destroy(Actor* thisx, PlayState* play) {
    BgJyaAmishutter* this = (BgJyaAmishutter*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgJyaAmishutter_SetupWaitForPlayer(BgJyaAmishutter* this) {
    this->actionFunc = BgJyaAmishutter_WaitForPlayer;
}

static void BgJyaAmishutter_WaitForPlayer(BgJyaAmishutter* this) {
    if ((this->dyna.actor.xzDistToPlayer < 60.0f) && (fabsf(this->dyna.actor.playerHeightRel) < 30.0f)) {
        func_80893428(this);
    }
}

static void func_80893428(BgJyaAmishutter* this) {
    this->actionFunc = func_80893438;
}

static void func_80893438(BgJyaAmishutter* this) {
    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 100.0f, 3.0f)) {
        func_808934B0(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_METALDOOR_STOP);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
    }
}

static void func_808934B0(BgJyaAmishutter* this) {
    this->actionFunc = func_808934C0;
}

static void func_808934C0(BgJyaAmishutter* this) {
    if (this->dyna.actor.xzDistToPlayer > 300.0f) {
        func_808934FC(this);
    }
}

static void func_808934FC(BgJyaAmishutter* this) {
    this->actionFunc = func_8089350C;
}

static void func_8089350C(BgJyaAmishutter* this) {
    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, 3.0f)) {
        BgJyaAmishutter_SetupWaitForPlayer(this);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_METALDOOR_STOP);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
    }
}

static void BgJyaAmishutter_Update(Actor* thisx, PlayState* play) {
    BgJyaAmishutter* this = (BgJyaAmishutter*)thisx;

    this->actionFunc(this);
}

static void BgJyaAmishutter_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, gAmishutterDL);
}
