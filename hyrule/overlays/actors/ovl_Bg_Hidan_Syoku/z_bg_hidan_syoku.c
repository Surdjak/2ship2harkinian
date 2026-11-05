/*
 * File: z_bg_hidan_syoku.c
 * Overlay: ovl_Bg_Hidan_Syoku
 * Description: Stone Elevator in the Fire Temple
 */

#include "z_bg_hidan_syoku.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_hidan_objects/object_hidan_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgHidanSyoku_Init(Actor* thisx, PlayState* play);
static void BgHidanSyoku_Destroy(Actor* thisx, PlayState* play);
static void BgHidanSyoku_Update(Actor* thisx, PlayState* play);
static void BgHidanSyoku_Draw(Actor* thisx, PlayState* play);

static void func_8088F4B8(BgHidanSyoku* this, PlayState* play);
static void func_8088F514(BgHidanSyoku* this, PlayState* play);
static void func_8088F62C(BgHidanSyoku* this, PlayState* play);

ActorProfile Bg_Hidan_Syoku_Oot_Profile = {
    /**/ ACTOR_BG_HIDAN_SYOKU_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_HIDAN_OBJECTS_Oot,
    /**/ sizeof(BgHidanSyoku),
    /**/ BgHidanSyoku_Init,
    /**/ BgHidanSyoku_Destroy,
    /**/ BgHidanSyoku_Update,
    /**/ BgHidanSyoku_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static void BgHidanSyoku_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgHidanSyoku* this = (BgHidanSyoku*)thisx;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    CollisionHeader_GetVirtual(&gFireTempleFlareDancerPlatformCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    this->actionFunc = func_8088F4B8;
    this->dyna.actor.home.pos.y += 540.0f;
}

static void BgHidanSyoku_Destroy(Actor* thisx, PlayState* play) {
    BgHidanSyoku* this = (BgHidanSyoku*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_8088F47C(BgHidanSyoku* this) {
    this->timer = 60;
    Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BLOCK_BOUND);
    this->actionFunc = func_8088F62C;
}

static void func_8088F4B8(BgHidanSyoku* this, PlayState* play) {
    if (Flags_GetClear(play, this->dyna.actor.room) && DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        this->timer = 140;
        this->actionFunc = func_8088F514;
    }
}

static void func_8088F514(BgHidanSyoku* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    this->dyna.actor.world.pos.y = (cosf(this->timer * (M_PI / 140)) * 540.0f) + this->dyna.actor.home.pos.y;
    if (this->timer == 0) {
        func_8088F47C(this);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_ELEVATOR_MOVE3 - SFX_FLAG);
    }
}

static void func_8088F5A0(BgHidanSyoku* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - (cosf(this->timer * (M_PI / 140)) * 540.0f);
    if (this->timer == 0) {
        func_8088F47C(this);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_ELEVATOR_MOVE3 - SFX_FLAG);
    }
}

static void func_8088F62C(BgHidanSyoku* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    if (this->timer == 0) {
        this->timer = 140;
        if (this->dyna.actor.world.pos.y < this->dyna.actor.home.pos.y) {
            this->actionFunc = func_8088F514;
        } else {
            this->actionFunc = func_8088F5A0;
        }
    }
}

static void BgHidanSyoku_Update(Actor* thisx, PlayState* play) {
    BgHidanSyoku* this = (BgHidanSyoku*)thisx;

    this->actionFunc(this, play);
    if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        if (this->unk_168 == 0) {
            this->unk_168 = 3;
        }
        Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON0);
    } else if (!DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        if (this->unk_168 != 0) {
            Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON0);
        }
        this->unk_168 = 0;
    }
}

static void BgHidanSyoku_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, gFireTempleFlareDancerPlatformDL);
}
