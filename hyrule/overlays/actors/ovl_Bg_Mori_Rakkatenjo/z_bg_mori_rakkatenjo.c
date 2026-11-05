/*
 * File: z_bg_mori_rakkatenjo.c
 * Overlay: ovl_Bg_Mori_Rakkatenjo
 * Description: Falling ceiling in Forest Temple
 */

#include "z_bg_mori_rakkatenjo.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "assets/oot/objects/object_mori_objects/object_mori_objects.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgMoriRakkatenjo_Init(Actor* thisx, PlayState* play);
static void BgMoriRakkatenjo_Destroy(Actor* thisx, PlayState* play);
static void BgMoriRakkatenjo_Update(Actor* thisx, PlayState* play);
static void BgMoriRakkatenjo_Draw(Actor* thisx, PlayState* play);

static void BgMoriRakkatenjo_SetupWaitForMoriTex(BgMoriRakkatenjo* this);
static void BgMoriRakkatenjo_WaitForMoriTex(BgMoriRakkatenjo* this, PlayState* play);
static void BgMoriRakkatenjo_SetupWait(BgMoriRakkatenjo* this);
static void BgMoriRakkatenjo_Wait(BgMoriRakkatenjo* this, PlayState* play);
static void BgMoriRakkatenjo_SetupFall(BgMoriRakkatenjo* this);
static void BgMoriRakkatenjo_Fall(BgMoriRakkatenjo* this, PlayState* play);
static void BgMoriRakkatenjo_SetupRest(BgMoriRakkatenjo* this);
static void BgMoriRakkatenjo_Rest(BgMoriRakkatenjo* this, PlayState* play);
static void BgMoriRakkatenjo_SetupRise(BgMoriRakkatenjo* this);
static void BgMoriRakkatenjo_Rise(BgMoriRakkatenjo* this, PlayState* play);

static s16 sCamSetting = 0;

ActorProfile Bg_Mori_Rakkatenjo_Oot_Profile = {
    /**/ ACTOR_BG_MORI_RAKKATENJO_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_MORI_OBJECTS_Oot,
    /**/ sizeof(BgMoriRakkatenjo),
    /**/ BgMoriRakkatenjo_Init,
    /**/ BgMoriRakkatenjo_Destroy,
    /**/ BgMoriRakkatenjo_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(gravity, -1, ICHAIN_CONTINUE),
    ICHAIN_F32(terminalVelocity, -11, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static void BgMoriRakkatenjo_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriRakkatenjo* this = (BgMoriRakkatenjo*)thisx;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    if ((fabsf(1991.0f - this->dyna.actor.home.pos.x) > 0.001f) ||
        (fabsf(683.0f - this->dyna.actor.home.pos.y) > 0.001f) ||
        (fabsf(-2520.0f - this->dyna.actor.home.pos.z) > 0.001f)) {
    }
    if (this->dyna.actor.home.rot.y != 0x8000) {
    }
    this->moriTexObjIndex = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (this->moriTexObjIndex < 0) {
        Actor_Kill(&this->dyna.actor);
        return;
    }
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    CollisionHeader_GetVirtual(&gMoriRakkatenjoCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    BgMoriRakkatenjo_SetupWaitForMoriTex(this);
    sCamSetting = 0;
}

static void BgMoriRakkatenjo_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriRakkatenjo* this = (BgMoriRakkatenjo*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static s32 BgMoriRakkatenjo_IsLinkUnder(BgMoriRakkatenjo* this, PlayState* play) {
    Vec3f* pos = &GET_PLAYER(play)->actor.world.pos;

    return (-3300.0f < pos->z) && (pos->z < -1840.0f) && (1791.0f < pos->x) && (pos->x < 2191.0f);
}

static s32 BgMoriRakkatenjo_IsLinkClose(BgMoriRakkatenjo* this, PlayState* play) {
    Vec3f* pos = &GET_PLAYER(play)->actor.world.pos;

    return (-3360.0f < pos->z) && (pos->z < -1840.0f) && (1791.0f < pos->x) && (pos->x < 2191.0f);
}

static void BgMoriRakkatenjo_SetupWaitForMoriTex(BgMoriRakkatenjo* this) {
    this->actionFunc = BgMoriRakkatenjo_WaitForMoriTex;
}

static void BgMoriRakkatenjo_WaitForMoriTex(BgMoriRakkatenjo* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->moriTexObjIndex)) {
        BgMoriRakkatenjo_SetupWait(this);
        this->dyna.actor.draw = BgMoriRakkatenjo_Draw;
    }
}

static void BgMoriRakkatenjo_SetupWait(BgMoriRakkatenjo* this) {
    this->timer = (this->fallCount > 0) ? 100 : 21;
    this->dyna.actor.world.pos.y = 683.0f;
    this->actionFunc = BgMoriRakkatenjo_Wait;
}

static void BgMoriRakkatenjo_Wait(BgMoriRakkatenjo* this, PlayState* play) {
    if (this->fallCount == 0) {
        if (BgMoriRakkatenjo_IsLinkClose(this, play) || (this->timer < 20)) {
            if (this->timer <= 0) {
                BgMoriRakkatenjo_SetupFall(this);
            }
        } else {
            this->timer = 21;
        }
    } else {
        if (BgMoriRakkatenjo_IsLinkUnder(this, play) || (this->timer < 20)) {
            if (this->timer <= 0) {
                BgMoriRakkatenjo_SetupFall(this);
            }
        } else {
            this->timer = 100;
        }
    }
    if (this->timer < 20) {
        Audio_PlaySfx_2(NA_SE_EV_BLOCKSINK - SFX_FLAG);
    }
}

static void BgMoriRakkatenjo_SetupFall(BgMoriRakkatenjo* this) {
    this->actionFunc = BgMoriRakkatenjo_Fall;
    this->bounceCount = 0;
    this->dyna.actor.velocity.y = 0.0f;
}

static void BgMoriRakkatenjo_Fall(BgMoriRakkatenjo* this, PlayState* play) {
    static f32 bounceVel[] = { 4.0f, 1.5f, 0.4f, 0.1f };
    s32 pad;
    Actor* thisx = &this->dyna.actor;
    s32 quake;

    Actor_MoveWithGravity(thisx);
    if ((thisx->velocity.y < 0.0f) && (thisx->world.pos.y <= 403.0f)) {
        if (this->bounceCount >= ARRAY_COUNT(bounceVel)) {
            BgMoriRakkatenjo_SetupRest(this);
        } else {
            if (this->bounceCount == 0) {
                this->fallCount++;
                Audio_PlaySfx_2(NA_SE_EV_BIGWALL_BOUND);
                Rumble_Request(SQ(thisx->playerHeightRel), 0xFF, 0x14, 0x96);
            }
            thisx->world.pos.y =
                403.0f - (thisx->world.pos.y - 403.0f) * bounceVel[this->bounceCount] / fabsf(thisx->velocity.y);
            thisx->velocity.y = bounceVel[this->bounceCount];
            this->bounceCount++;
            quake = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);
            Quake_SetSpeed(quake, 50000);
            Quake_SetPerturbations(quake, 5, 0, 0, 0);
            Quake_SetDuration(quake, 5);
        }
    }
}

static void BgMoriRakkatenjo_SetupRest(BgMoriRakkatenjo* this) {
    this->actionFunc = BgMoriRakkatenjo_Rest;
    this->dyna.actor.world.pos.y = 403.0f;
    this->timer = 20;
}

static void BgMoriRakkatenjo_Rest(BgMoriRakkatenjo* this, PlayState* play) {
    if (this->timer <= 0) {
        BgMoriRakkatenjo_SetupRise(this);
    }
}

static void BgMoriRakkatenjo_SetupRise(BgMoriRakkatenjo* this) {
    this->actionFunc = BgMoriRakkatenjo_Rise;
    this->dyna.actor.velocity.y = -0.1f;
}

static void BgMoriRakkatenjo_Rise(BgMoriRakkatenjo* this, PlayState* play) {
    Math_SmoothStepToF(&this->dyna.actor.velocity.y, 5.0f, 0.06f, 0.1f, 0.0f);
    this->dyna.actor.world.pos.y += this->dyna.actor.velocity.y;
    if (this->dyna.actor.world.pos.y >= 683.0f) {
        BgMoriRakkatenjo_SetupWait(this);
    }
}

static void BgMoriRakkatenjo_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriRakkatenjo* this = (BgMoriRakkatenjo*)thisx;

    if (this->timer > 0) {
        this->timer--;
    }
    this->actionFunc(this, play);
    if (BgMoriRakkatenjo_IsLinkUnder(this, play)) {
        if (sCamSetting == CAM_SET_NONE) {
            sCamSetting = play->cameraPtrs[CAM_ID_MAIN]->setting;
            Camera_SetCameraData(play->cameraPtrs[CAM_ID_MAIN], 1, &this->dyna.actor, NULL, 0, 0);
            Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_BIRDS_EYE_VIEW_0);
        }
    } else if (sCamSetting != CAM_SET_NONE) {
        Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON1);
        sCamSetting = 0;
    }
}

static void BgMoriRakkatenjo_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriRakkatenjo* this = (BgMoriRakkatenjo*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIndex].segment);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_OPA_DISP++, gMoriRakkatenjoDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
