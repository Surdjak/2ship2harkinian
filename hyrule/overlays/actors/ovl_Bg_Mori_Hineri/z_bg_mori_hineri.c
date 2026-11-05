/*
 * File: z_bg_mori_hineri.c
 * Overlay: ovl_Bg_Mori_Hineri
 * Description: Twisting hallway in Forest Temple
 */

#include "z_bg_mori_hineri.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/object_box/object_box.h"
#include "assets/oot/objects/object_mori_hineri1/object_mori_hineri1.h"
#include "assets/oot/objects/object_mori_hineri1a/object_mori_hineri1a.h"
#include "assets/oot/objects/object_mori_hineri2/object_mori_hineri2.h"
#include "assets/oot/objects/object_mori_hineri2a/object_mori_hineri2a.h"
#include "assets/oot/objects/object_mori_tex/object_mori_tex.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

static void BgMoriHineri_Init(Actor* thisx, PlayState* play);
static void BgMoriHineri_Destroy(Actor* thisx, PlayState* play);
static void BgMoriHineri_Update(Actor* thisx, PlayState* play);
static void BgMoriHineri_DrawHallAndRoom(Actor* thisx, PlayState* play);
static void BgMoriHineri_Reset(void);

static void func_808A39FC(BgMoriHineri* this, PlayState* play);
static void func_808A3E54(BgMoriHineri* this, PlayState* play);
static void func_808A3C8C(BgMoriHineri* this, PlayState* play);
static void BgMoriHineri_SpawnBossKeyChest(BgMoriHineri* this, PlayState* play);
static void BgMoriHineri_DoNothing(BgMoriHineri* this, PlayState* play);
static void func_808A3D58(BgMoriHineri* this, PlayState* play);


static s16 sSubCamId = CAM_ID_NONE;

ActorProfile Bg_Mori_Hineri_Oot_Profile = {
    /**/ ACTOR_BG_MORI_HINERI_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(BgMoriHineri),
    /**/ BgMoriHineri_Init,
    /**/ BgMoriHineri_Destroy,
    /**/ BgMoriHineri_Update,
    /**/ NULL,
    /**/ BgMoriHineri_Reset,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static Gfx* sDLists[] = {
    object_mori_hineri1_DL_0024E0,
    object_mori_hineri1a_DL_001980,
    object_mori_hineri2_DL_0020F0,
    object_mori_hineri2a_DL_002B70,
};

static void BgMoriHineri_Init(Actor* thisx, PlayState* play) {
    BgMoriHineri* this = (BgMoriHineri*)thisx;
    s8 moriHineriObjIdx;
    u32 switchFlagParam;
    s32 t6;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);

    switchFlagParam = this->dyna.actor.params & 0x3F;
    t6 = this->dyna.actor.params & 0x4000;

    if (t6 != 0) {
        this->switchFlag = switchFlagParam;
    } else {
        this->switchFlag = ((this->dyna.actor.params >> 8) & 0x3F);
        this->switchFlag = (Flags_GetSwitch(play, this->switchFlag)) ? 1 : 0;
    }
    this->dyna.actor.params = ((this->dyna.actor.params & 0x8000) >> 0xE);
    if (Flags_GetSwitch(play, switchFlagParam)) {
        if (this->dyna.actor.params == 0) {
            this->dyna.actor.params = 1;
        } else if (this->dyna.actor.params == 2) {
            this->dyna.actor.params = 3;
        }
    }
    this->boxObjIdx = -1;
    if (this->dyna.actor.params == 0) {
        this->moriHineriObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI1_Oot);
        if (t6 == 0) {
            this->boxObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_BOX_Oot);
        }
    } else {
        if (this->dyna.actor.params == 1) {
            moriHineriObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI1A_Oot);
        } else {
            moriHineriObjIdx = (this->dyna.actor.params == 2) ? Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI2_Oot)
                                                              : Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI2A_Oot);
        }
        this->moriHineriObjIdx = moriHineriObjIdx;
    }
    this->moriTexObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_MORI_TEX_Oot);
    if (t6 != 0) {
        this->dyna.actor.params += 4;
    }
    if ((this->moriHineriObjIdx < 0) || (this->moriTexObjIdx < 0)) {
        Actor_Kill(&this->dyna.actor);
    } else {
        this->actionFunc = func_808A39FC;
    }
}

static void BgMoriHineri_Destroy(Actor* thisx, PlayState* play) {
    BgMoriHineri* this = (BgMoriHineri*)thisx;
    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_808A39FC(BgMoriHineri* this, PlayState* play) {
    CollisionHeader* colHeader;

    if (Object_IsLoaded(&play->objectCtx, this->moriHineriObjIdx) &&
        Object_IsLoaded(&play->objectCtx, this->moriTexObjIdx) &&
        ((this->boxObjIdx < 0) || Object_IsLoaded(&play->objectCtx, this->boxObjIdx))) {
        this->dyna.actor.objectSlot = this->moriHineriObjIdx;
        if (this->dyna.actor.params >= 4) {
            this->dyna.actor.params -= 4;
            if (this->dyna.actor.params == 0) {
                this->moriHineriObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI1A_Oot);
            } else if (this->dyna.actor.params == 1) {
                this->moriHineriObjIdx = Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI1_Oot);
            } else {
                this->moriHineriObjIdx = (this->dyna.actor.params == 2)
                                             ? Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI2A_Oot)
                                             : Object_GetSlot(&play->objectCtx, OBJECT_MORI_HINERI2_Oot);
            }
            if (this->moriHineriObjIdx < 0) {
                Actor_Kill(&this->dyna.actor);
            } else {
                this->actionFunc = func_808A3D58;
            }
        } else {
            Actor_SetObjectDependency(play, &this->dyna.actor);
            colHeader = NULL;
            this->dyna.actor.draw = BgMoriHineri_DrawHallAndRoom;
            if (this->dyna.actor.params == 0) {
                // In vanilla this actionFunc is set to func_808A3C8C, and the boss key chest is rendered from scratch
                // in BgMoriHineri_DrawHallAndRoom In SOH we instead spawn the en_box actor, which allows the rendering
                // to be handled consistently with the rest of the chests in the game
                this->actionFunc = BgMoriHineri_SpawnBossKeyChest;
                CollisionHeader_GetVirtual(&object_mori_hineri1_Col_0054B8, &colHeader);
            } else if (this->dyna.actor.params == 1) {
                this->actionFunc = BgMoriHineri_SpawnBossKeyChest;
                CollisionHeader_GetVirtual(&object_mori_hineri1a_Col_003490, &colHeader);
            } else if (this->dyna.actor.params == 2) {
                this->actionFunc = BgMoriHineri_DoNothing;
                CollisionHeader_GetVirtual(&object_mori_hineri2_Col_0043D0, &colHeader);
            } else {
                this->actionFunc = func_808A3C8C;
                CollisionHeader_GetVirtual(&object_mori_hineri2a_Col_006078, &colHeader);
            }
            this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
        }
    }
}

static void BgMoriHineri_DoNothing(BgMoriHineri* this, PlayState* play) {
}

static void BgMoriHineri_SpawnBossKeyChest(BgMoriHineri* this, PlayState* play) {
    if (this->dyna.actor.params == 0) {
        Object_SpawnPersistent(&play->objectCtx, OBJECT_BOX); // PORT: OoT's chests use MM's object
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOX_Oot, -1515.0f, 1440.0f, -3475.0f, -0x4000, 0x4000, 0, 0x27EE);
        this->actionFunc = func_808A3C8C;
    } else {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOX_Oot, this->dyna.actor.world.pos.x + 147.0f,
                    this->dyna.actor.world.pos.y + -245.0f, this->dyna.actor.world.pos.z + -453.0f, 0, 0x4000, 0,
                    0x27EE);
        this->actionFunc = BgMoriHineri_DoNothing;
    }
}

static void func_808A3C8C(BgMoriHineri* this, PlayState* play) {
    f32 f0;
    Player* player = GET_PLAYER(play);

    f0 = 1100.0f - (player->actor.world.pos.z - this->dyna.actor.world.pos.z);
    this->dyna.actor.shape.rot.z = CLAMP(f0, 0.0f, 1000.0f) * 16.384f;
    Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_DUNGEON1);
    if (this->dyna.actor.params != 0) {
        this->dyna.actor.shape.rot.z = -this->dyna.actor.shape.rot.z;
    }
}

static void func_808A3D58(BgMoriHineri* this, PlayState* play) {
    s16 mainCamChildIdx;

    if ((Flags_GetSwitch(play, this->switchFlag) && (this->dyna.actor.params == 0 || this->dyna.actor.params == 2)) ||
        (!Flags_GetSwitch(play, this->switchFlag) && (this->dyna.actor.params == 1 || this->dyna.actor.params == 3))) {
        this->dyna.actor.draw = BgMoriHineri_DrawHallAndRoom;
        this->actionFunc = func_808A3E54;

        mainCamChildIdx = play->cameraPtrs[CAM_ID_MAIN]->childCamId;
        if ((mainCamChildIdx != SUB_CAM_ID_DONE) &&
            (play->cameraPtrs[mainCamChildIdx]->setting == CAM_SET_DEMO0)) {
            OnePointCutscene_EndCutscene(play, mainCamChildIdx);
        }
        OnePointCutscene_Init(play, 3260, 40, &this->dyna.actor, CAM_ID_MAIN);
        sSubCamId = OnePointCutscene_Init(play, 3261, 40, &this->dyna.actor, CAM_ID_MAIN);
    }
}

static void func_808A3E54(BgMoriHineri* this, PlayState* play) {
    s8 objectSlot;

    if (play->activeCamId == sSubCamId) {
        if (sSubCamId != CAM_ID_MAIN) {
            objectSlot = this->dyna.actor.objectSlot;
            this->dyna.actor.objectSlot = this->moriHineriObjIdx;
            this->moriHineriObjIdx = objectSlot;
            this->dyna.actor.params ^= 1;
            sSubCamId = CAM_ID_MAIN;
            Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
        } else {
            this->dyna.actor.draw = NULL;
            this->actionFunc = func_808A3D58;
            sSubCamId = CAM_ID_NONE;
        }
    }
    if ((sSubCamId >= CAM_ID_SUB_FIRST) && ((GET_ACTIVE_CAM(play)->eye.z - this->dyna.actor.world.pos.z) < 1100.0f)) {
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_FLOOR_ROLLING - SFX_FLAG);
    }
}

static void BgMoriHineri_Update(Actor* thisx, PlayState* play) {
    BgMoriHineri* this = (BgMoriHineri*)thisx;

    this->actionFunc(this, play);
}

static void BgMoriHineri_DrawHallAndRoom(Actor* thisx, PlayState* play) {
    BgMoriHineri* this = (BgMoriHineri*)thisx;
    s8 objIndex;
    MtxF mtx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, play->objectCtx.slots[this->moriTexObjIdx].segment);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDLists[this->dyna.actor.params]);
    if (this->boxObjIdx > 0) {
        Matrix_Get(&mtx);
    }
    if ((this->actionFunc == func_808A3C8C) && (this->dyna.actor.shape.rot.z != 0)) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        if (this->dyna.actor.params == 0) {
            Matrix_Translate(-1761.0f, 1278.0f, -1821.0f, MTXMODE_NEW);
        } else {
            Matrix_Translate(1999.0f, 1278.0f, -1821.0f, MTXMODE_NEW);
        }
        Matrix_RotateZYX(0, -0x8000, this->dyna.actor.shape.rot.z, MTXMODE_APPLY);
        Matrix_Translate(0.0f, -50.0f, 0.0f, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gDungeonDoorDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BgMoriHineri_Reset() {
    sSubCamId = CAM_ID_NONE;
}
