/*
 * File: z_bg_mori_bigst.c
 * Overlay: ovl_Bg_Mori_Bigst
 * Description: Forest Temple falling platform and Stalfos fight
 */

#include "z_bg_mori_bigst.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64quake.h"
#include "assets/oot/objects/object_mori_objects/object_mori_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgMoriBigst_Init(Actor* thisx, PlayState* play);
static void BgMoriBigst_Destroy(Actor* thisx, PlayState* play);
static void BgMoriBigst_Update(Actor* thisx, PlayState* play);
static void BgMoriBigst_Draw(Actor* thisx, PlayState* play);

static void BgMoriBigst_SetupWaitForMoriTex(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_WaitForMoriTex(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupNoop(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupStalfosFight(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_StalfosFight(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupFall(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_Fall(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupLanding(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_Landing(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupStalfosPairFight(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_StalfosPairFight(BgMoriBigst* this, PlayState* play);
static void BgMoriBigst_SetupDone(BgMoriBigst* this, PlayState* play);

ActorProfile Bg_Mori_Bigst_Oot_Profile = {
    /**/ ACTOR_BG_MORI_BIGST_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_MORI_OBJECTS_Oot,
    /**/ sizeof(BgMoriBigst),
    /**/ BgMoriBigst_Init,
    /**/ BgMoriBigst_Destroy,
    /**/ BgMoriBigst_Update,
    /**/ NULL,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeDistance, 3000, ICHAIN_CONTINUE),      ICHAIN_F32(cullingVolumeScale, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 3000, ICHAIN_CONTINUE),     ICHAIN_F32_DIV1000(gravity, -500, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(terminalVelocity, -12000, ICHAIN_CONTINUE), ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static void BgMoriBigst_SetupAction(BgMoriBigst* this, BgMoriBigstActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgMoriBigst_InitDynapoly(BgMoriBigst* this, PlayState* play, CollisionHeader* collision, s32 moveFlag) {
    s32 pad;
    CollisionHeader* colHeader = NULL;
    s32 pad2;

    DynaPolyActor_Init(&this->dyna, moveFlag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    if (this->dyna.bgId == BG_ACTOR_MAX) {
    }
}

static void BgMoriBigst_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriBigst* this = (BgMoriBigst*)thisx;

    BgMoriBigst_InitDynapoly(this, play, &gMoriBigstCol, 0);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    this->moriTexObjIndex = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (this->moriTexObjIndex < 0) {
        Actor_Kill(&this->dyna.actor);
        return;
    }
    if (Flags_GetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F)) {
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y;
    } else {
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y + 270.0f;
    }
    Actor_SetFocus(&this->dyna.actor, 50.0f);
    BgMoriBigst_SetupWaitForMoriTex(this, play);
}

static void BgMoriBigst_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriBigst* this = (BgMoriBigst*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void BgMoriBigst_SetupWaitForMoriTex(BgMoriBigst* this, PlayState* play) {
    BgMoriBigst_SetupAction(this, BgMoriBigst_WaitForMoriTex);
}

static void BgMoriBigst_WaitForMoriTex(BgMoriBigst* this, PlayState* play) {
    Actor* thisx = &this->dyna.actor;

    if (Object_IsLoaded(&play->objectCtx, this->moriTexObjIndex)) {
        thisx->draw = BgMoriBigst_Draw;
        if (Flags_GetClear(play, thisx->room) && (GET_PLAYER(play)->actor.world.pos.y > 700.0f)) {
            if (Flags_GetSwitch(play, (thisx->params >> 8) & 0x3F)) {
                BgMoriBigst_SetupDone(this, play);
            } else {
                BgMoriBigst_SetupStalfosFight(this, play);
            }
        } else {
            BgMoriBigst_SetupNoop(this, play);
        }
    }
}

static void BgMoriBigst_SetupNoop(BgMoriBigst* this, PlayState* play) {
    BgMoriBigst_SetupAction(this, NULL);
}

static void BgMoriBigst_SetupStalfosFight(BgMoriBigst* this, PlayState* play) {
    Actor* stalfos;

    BgMoriBigst_SetupAction(this, BgMoriBigst_StalfosFight);
    Flags_UnsetClear(play, this->dyna.actor.room);
    stalfos = Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_EN_TEST_Oot, 209.0f, 827.0f, -3320.0f, 0,
                                 0, 0, 1);
    if (stalfos != NULL) {
        this->dyna.actor.child = NULL;
        this->dyna.actor.home.rot.z++;
    } else {
    }
    Flags_SetClear(play, this->dyna.actor.room);
}

static void BgMoriBigst_StalfosFight(BgMoriBigst* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if ((this->dyna.actor.home.rot.z == 0) &&
        ((this->dyna.actor.home.pos.y - 5.0f) <= GET_PLAYER(play)->actor.world.pos.y)) {
        BgMoriBigst_SetupFall(this, play);
        if ((true)) {
            OnePointCutscene_Init(play, 3220, 72, &this->dyna.actor, CAM_ID_MAIN);
        }
    }
}

static void BgMoriBigst_SetupFall(BgMoriBigst* this, PlayState* play) {
    BgMoriBigst_SetupAction(this, BgMoriBigst_Fall);
}

static void BgMoriBigst_Fall(BgMoriBigst* this, PlayState* play) {
    Actor_MoveWithGravity(&this->dyna.actor);
    if (this->dyna.actor.world.pos.y <= this->dyna.actor.home.pos.y) {
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y;
        BgMoriBigst_SetupLanding(this, play);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BIGWALL_BOUND);
        if ((true)) {
            OnePointCutscene_Init(play, 1020, 8, &this->dyna.actor, CAM_ID_MAIN);
            Player_SetCsAction(play, NULL, 0x3C);
        }
    }
}

static void BgMoriBigst_SetupLanding(BgMoriBigst* this, PlayState* play) {
    s32 pad;
    s32 quake;

    BgMoriBigst_SetupAction(this, BgMoriBigst_Landing);
    this->waitTimer = 18;
    quake = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);
    Quake_SetSpeed(quake, 25000);
    Quake_SetPerturbations(quake, 5, 0, 0, 0);
    Quake_SetDuration(quake, 16);
}

static void BgMoriBigst_Landing(BgMoriBigst* this, PlayState* play) {
    if (this->waitTimer <= 0) {
        BgMoriBigst_SetupStalfosPairFight(this, play);
    }
}

static void BgMoriBigst_SetupStalfosPairFight(BgMoriBigst* this, PlayState* play) {
    Actor* stalfos1;
    Actor* stalfos2;

    BgMoriBigst_SetupAction(this, BgMoriBigst_StalfosPairFight);
    Flags_UnsetClear(play, this->dyna.actor.room);
    if ((true)) {
        stalfos1 = Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_EN_TEST_Oot, 70.0f, 827.0f, -3383.0f,
                                      0, 0, 0, 5);
        if (stalfos1 != NULL) {
            this->dyna.actor.child = NULL;
            this->dyna.actor.home.rot.z++;
        } else {
        }
        stalfos2 = Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_EN_TEST_Oot, 170.0f, 827.0f, -3260.0f,
                                      0, 0, 0, 5);
        if (stalfos2 != NULL) {
            this->dyna.actor.child = NULL;
            this->dyna.actor.home.rot.z++;
        } else {
        }
    }
    Flags_SetClear(play, this->dyna.actor.room);
}

static void BgMoriBigst_StalfosPairFight(BgMoriBigst* this, PlayState* play) {
    if (
        // Check if all enemies are defeated instead of the regular stalfos when crowd control is on. TODO: move to the
        // way that enemy randomizer does this
        (0 ? Flags_GetClearTemp(play, this->dyna.actor.room)
                                                                 : this->dyna.actor.home.rot.z == 0) &&
        !Player_InCsMode(play)) {
        Flags_SetSwitch(play, (this->dyna.actor.params >> 8) & 0x3F);
        BgMoriBigst_SetupDone(this, play);
    }
}

static void BgMoriBigst_SetupDone(BgMoriBigst* this, PlayState* play) {
    BgMoriBigst_SetupAction(this, NULL);
}

static void BgMoriBigst_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriBigst* this = (BgMoriBigst*)thisx;

    Actor_SetFocus(&this->dyna.actor, 50.0f);
    if (this->waitTimer > 0) {
        this->waitTimer--;
    }
    if (DynaPolyActor_IsPlayerAbove(&this->dyna)) {
        Environment_ChangeLightSetting(play, 6);
    }
    if (this->actionFunc != NULL) {
        this->actionFunc(this, play);
    }
}

static void BgMoriBigst_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BgMoriBigst* this = (BgMoriBigst*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIndex].segment);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_OPA_DISP++, gMoriBigstDL);
    CLOSE_DISPS(play->state.gfxCtx);
}
