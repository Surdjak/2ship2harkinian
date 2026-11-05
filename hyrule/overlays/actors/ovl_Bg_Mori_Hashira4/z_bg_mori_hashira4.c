/*
 * File: z_bg_mori_hashira4
 * Overlay: ovl_Bg_Mori_Hashira4
 * Description: Forest Temple gates and rotating pillars
 */

#include "z_bg_mori_hashira4.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_mori_objects/object_mori_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgMoriHashira4_Init(Actor* thisx, PlayState* play);
static void BgMoriHashira4_Destroy(Actor* thisx, PlayState* play);
static void BgMoriHashira4_Update(Actor* thisx, PlayState* play);
static void BgMoriHashira4_Draw(Actor* thisx, PlayState* play);

static void BgMoriHashira4_SetupWaitForMoriTex(BgMoriHashira4* this);
static void BgMoriHashira4_WaitForMoriTex(BgMoriHashira4* this, PlayState* play);
static void BgMoriHashira4_SetupPillarsRotate(BgMoriHashira4* this);
static void BgMoriHashira4_PillarsRotate(BgMoriHashira4* this, PlayState* play);
static void BgMoriHashira4_GateWait(BgMoriHashira4* this, PlayState* play);
static void BgMoriHashira4_GateOpen(BgMoriHashira4* this, PlayState* play);

ActorProfile Bg_Mori_Hashira4_Oot_Profile = {
    /**/ ACTOR_BG_MORI_HASHIRA4_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_MORI_OBJECTS_Oot,
    /**/ sizeof(BgMoriHashira4),
    /**/ BgMoriHashira4_Init,
    /**/ BgMoriHashira4_Destroy,
    /**/ BgMoriHashira4_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeDistance, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 700, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1000, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static Gfx* sDisplayLists[] = { gMoriHashiraPlatformsDL, gMoriHashiraGateDL };

static s16 sUnkTimer; // seems to be unused

static void BgMoriHashira4_SetupAction(BgMoriHashira4* this, BgMoriHashira4ActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgMoriHashira4_InitDynaPoly(BgMoriHashira4* this, PlayState* play, CollisionHeader* collision, s32 moveFlag) {
    s32 pad;
    CollisionHeader* colHeader;
    s32 pad2;

    colHeader = NULL;
    DynaPolyActor_Init(&this->dyna, moveFlag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgMoriHashira4_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriHashira4* this = (BgMoriHashira4*)thisx;

    this->switchFlag = (this->dyna.actor.params >> 8) & 0x3F;
    this->dyna.actor.params &= 0xFF;

    if (this->dyna.actor.params == 0) {
        BgMoriHashira4_InitDynaPoly(this, play, &gMoriHashira1Col, DYNA_TRANSFORM_POS | DYNA_TRANSFORM_ROT_Y);
    } else {
        BgMoriHashira4_InitDynaPoly(this, play, &gMoriHashira2Col, 0);
    }
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    this->moriTexObjIndex = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (this->moriTexObjIndex < 0) {
        Actor_Kill(&this->dyna.actor);
        return;
    }
    if ((this->dyna.actor.params != 0) && Flags_GetSwitch(play, this->switchFlag)) {
        Actor_Kill(&this->dyna.actor);
        return;
    }
    Actor_SetFocus(&this->dyna.actor, 50.0f);
    BgMoriHashira4_SetupWaitForMoriTex(this);
    sUnkTimer = 0;
}

static void BgMoriHashira4_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriHashira4* this = (BgMoriHashira4*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgMoriHashira4_SetupWaitForMoriTex(BgMoriHashira4* this) {
    BgMoriHashira4_SetupAction(this, BgMoriHashira4_WaitForMoriTex);
}

static void BgMoriHashira4_WaitForMoriTex(BgMoriHashira4* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->moriTexObjIndex)) {
        this->gateTimer = 0;
        if (this->dyna.actor.params == 0) {
            BgMoriHashira4_SetupPillarsRotate(this);
        } else {
            BgMoriHashira4_SetupAction(this, BgMoriHashira4_GateWait);
        }
        this->dyna.actor.draw = BgMoriHashira4_Draw;
    }
}

static void BgMoriHashira4_SetupPillarsRotate(BgMoriHashira4* this) {
    BgMoriHashira4_SetupAction(this, BgMoriHashira4_PillarsRotate);
}

static void BgMoriHashira4_PillarsRotate(BgMoriHashira4* this, PlayState* play) {
    this->dyna.actor.shape.rot.y = this->dyna.actor.world.rot.y += 0x96;
    Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_ROLL_STAND_2 - SFX_FLAG);
}

static void BgMoriHashira4_GateWait(BgMoriHashira4* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->switchFlag) || (this->gateTimer != 0)) {
        this->gateTimer++;
        if (this->gateTimer > 30) {
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_METALDOOR_OPEN);
            BgMoriHashira4_SetupAction(this, BgMoriHashira4_GateOpen);
            OnePointCutscene_Init(play, 6010, 20, &this->dyna.actor, CAM_ID_MAIN);
            sUnkTimer++;
        }
    }
}

static void BgMoriHashira4_GateOpen(BgMoriHashira4* this, PlayState* play) {
    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 120.0f, 10.0f)) {
        Actor_Kill(&this->dyna.actor);
    }
}

static void BgMoriHashira4_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriHashira4* this = (BgMoriHashira4*)thisx;

    if (this->actionFunc != NULL) {
        this->actionFunc(this, play);
    }
}

static void BgMoriHashira4_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriHashira4* this = (BgMoriHashira4*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIndex].segment);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_OPA_DISP++, sDisplayLists[this->dyna.actor.params]);
    CLOSE_DISPS(play->state.gfxCtx);
}
