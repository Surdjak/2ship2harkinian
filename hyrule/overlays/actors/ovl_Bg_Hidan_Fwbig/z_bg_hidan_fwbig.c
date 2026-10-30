/*
 * File: z_bg_hidan_fwbig.c
 * Overlay: ovl_Bg_Hidan_Fwbig
 * Description: Large fire walls at Fire Temple (flame wall before bombable door and the one that chases the player in
 * the lava room)
 */

#include "z_bg_hidan_fwbig.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/object_hidan_objects/object_hidan_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

typedef enum {
    /* 0 */ FWBIG_MOVE,
    /* 1 */ FWBIG_RESET,
    /* 2 */ FWBIG_KILL
} HidanFwbigMoveState;

static void BgHidanFwbig_Init(Actor* thisx, PlayState* play);
static void BgHidanFwbig_Destroy(Actor* thisx, PlayState* play);
static void BgHidanFwbig_Update(Actor* thisx, PlayState* play);
static void BgHidanFwbig_Draw(Actor* thisx, PlayState* play);

static void BgHidanFwbig_UpdatePosition(BgHidanFwbig* this);

static void BgHidanFwbig_WaitForSwitch(BgHidanFwbig* this, PlayState* play);
static void BgHidanFwbig_WaitForCs(BgHidanFwbig* this, PlayState* play);
static void BgHidanFwbig_Lower(BgHidanFwbig* this, PlayState* play);
static void BgHidanFwbig_WaitForTimer(BgHidanFwbig* this, PlayState* play);
static void BgHidanFwbig_WaitForPlayer(BgHidanFwbig* this, PlayState* play);
static void BgHidanFwbig_Move(BgHidanFwbig* this, PlayState* play);

ActorProfile Bg_Hidan_Fwbig_Oot_Profile = {
    /**/ ACTOR_BG_HIDAN_FWBIG_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_HIDAN_OBJECTS_Oot,
    /**/ sizeof(BgHidanFwbig),
    /**/ BgHidanFwbig_Init,
    /**/ BgHidanFwbig_Destroy,
    /**/ BgHidanFwbig_Update,
    /**/ BgHidanFwbig_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0x20000000), 0x01, 0x04 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NONE,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 30, 130, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(cullingVolumeScale, 1000, ICHAIN_STOP),
};

static void BgHidanFwbig_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BgHidanFwbig* this = (BgHidanFwbig*)thisx;
    Player* player = GET_PLAYER(play);

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->direction = (u16)(thisx->params >> 8);
    thisx->params &= 0xFF;
    if (this->direction != 0) {
        this->actor.home.pos.x = 1560.0f;
        this->actor.home.pos.z = 0.0f;
        if (player->actor.world.pos.z > 300.0f) {
            this->direction = -1;
            this->actor.home.rot.y = this->actor.shape.rot.y = -0x4E38;
        } else if (player->actor.world.pos.z < -300.0f) {
            this->direction = 1;
            this->actor.home.rot.y = this->actor.shape.rot.y = -0x31C8;
        } else {
            Actor_Kill(&this->actor);
            return;
        }
        BgHidanFwbig_UpdatePosition(this);
        Actor_SetScale(&this->actor, 0.15f);
        this->collider.dim.height = 230;
        this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->moveState = FWBIG_MOVE;
        this->actionFunc = BgHidanFwbig_WaitForPlayer;
        this->actor.world.pos.y = this->actor.home.pos.y - (2400.0f * this->actor.scale.y);
    } else {
        Actor_SetScale(&this->actor, 0.1f);
        this->actionFunc = BgHidanFwbig_WaitForSwitch;
    }
}

static void BgHidanFwbig_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BgHidanFwbig* this = (BgHidanFwbig*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void BgHidanFwbig_UpdatePosition(BgHidanFwbig* this) {
    s16 startAngle = this->actor.shape.rot.y + this->direction * -0x4000;

    this->actor.world.pos.x = (Math_SinS(startAngle) * 885.4f) + this->actor.home.pos.x;
    this->actor.world.pos.z = (Math_CosS(startAngle) * 885.4f) + this->actor.home.pos.z;
}

static void BgHidanFwbig_WaitForSwitch(BgHidanFwbig* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->actor.params)) {
        this->actionFunc = BgHidanFwbig_WaitForCs;
        // PORT: OoT's camera cutscene on the fire (OnePointCutscene_Init 3340) is not ported.
        this->timer = 35;
    }
}

static void BgHidanFwbig_WaitForCs(BgHidanFwbig* this, PlayState* play) {
    if (this->timer-- == 0) {
        this->actionFunc = BgHidanFwbig_Lower;
    }
}

static void BgHidanFwbig_Rise(BgHidanFwbig* this, PlayState* play) {
    if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y, 10.0f)) {
        if (this->direction == 0) {
            Flags_UnsetSwitch(play, this->actor.params);
            this->actionFunc = BgHidanFwbig_WaitForSwitch;
        } else {
            this->actionFunc = BgHidanFwbig_Move;
        }
    }
}

static void BgHidanFwbig_Lower(BgHidanFwbig* this, PlayState* play) {
    if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y - (2400.0f * this->actor.scale.y), 10.0f)) {
        if (this->direction == 0) {
            this->actionFunc = BgHidanFwbig_WaitForTimer;
            this->timer = 150;
        } else if (this->moveState == FWBIG_KILL) {
            Actor_Kill(&this->actor);
        } else {
            if (this->moveState == FWBIG_MOVE) {
                this->actor.shape.rot.y -= (this->direction * 0x1800);
            } else {
                this->moveState = FWBIG_MOVE;
                this->actor.shape.rot.y = this->actor.home.rot.y;
            }
            BgHidanFwbig_UpdatePosition(this);
            this->actionFunc = BgHidanFwbig_Rise;
        }
    }
}

static void BgHidanFwbig_WaitForTimer(BgHidanFwbig* this, PlayState* play) {
    if ((this->timer != 0)) {
        this->timer--;
    }

    if (this->timer == 0) {
        this->actionFunc = BgHidanFwbig_Rise;
    }
    Actor_PlaySfx_FlaggedTimer(&this->actor, this->timer);
}

static void BgHidanFwbig_WaitForPlayer(BgHidanFwbig* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (player->actor.world.pos.x < 1150.0f) {
        this->actionFunc = BgHidanFwbig_Rise;
        // PORT: OoT's camera cutscene on the fire (OnePointCutscene_Init 3290) is not ported.
    }
}

static void BgHidanFwbig_Move(BgHidanFwbig* this, PlayState* play) {
    if (!Player_InCsMode(play)) {
        if (Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.home.rot.y + (this->direction * 0x6390), 0x20)) {
            this->moveState = FWBIG_RESET;
            this->actionFunc = BgHidanFwbig_Lower;
        } else {
            BgHidanFwbig_UpdatePosition(this);
        }
    }
}

static void BgHidanFwbig_MoveCollider(BgHidanFwbig* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f projPos;
    f32 cs;
    f32 sn;

    Actor_WorldToActorCoords(&this->actor, &projPos, &player->actor.world.pos);
    projPos.z = ((projPos.z >= 0.0f) ? 1.0f : -1.0f) * 25.0f * -1.0f;
    if (this->direction == 0) {
        projPos.x = CLAMP(projPos.x, -360.0f, 360.0f);
    } else {
        projPos.x = CLAMP(projPos.x, -500.0f, 500.0f);
    }

    sn = Math_SinS(this->actor.shape.rot.y);
    cs = Math_CosS(this->actor.shape.rot.y);
    this->collider.dim.pos.x = this->actor.world.pos.x + (projPos.x * cs) + (projPos.z * sn);
    this->collider.dim.pos.z = this->actor.world.pos.z - (projPos.x * sn) + (projPos.z * cs);
    this->collider.dim.pos.y = this->actor.world.pos.y;

    this->actor.world.rot.y = (projPos.z < 0.0f) ? this->actor.shape.rot.y : this->actor.shape.rot.y + 0x8000;
}

static void BgHidanFwbig_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BgHidanFwbig* this = (BgHidanFwbig*)thisx;

    if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.atFlags &= ~AT_HIT;
        func_800B8D98(play, &this->actor, 5.0f, this->actor.world.rot.y, 1.0f);
        if (this->direction != 0) {
            this->actionFunc = BgHidanFwbig_Lower;
        }
    }
    if ((this->direction != 0) && (play->roomCtx.prevRoom.num == this->actor.room)) {
        this->moveState = FWBIG_KILL;
        this->actionFunc = BgHidanFwbig_Lower;
    }

    this->actionFunc(this, play);

    if ((this->actor.home.pos.y - 200.0f) < this->actor.world.pos.y) {
        if (gSaveContext.sceneLayer < 4) {
            Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_BURNING - SFX_FLAG);
        } else if ((s16)this->actor.world.pos.x == -513) {
            Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_FLAME_OF_FIRE - SFX_FLAG);
        }
        BgHidanFwbig_MoveCollider(this, play);
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void BgHidanFwbig_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    f32 height;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x09, Lib_SegmentedToVirtual(gEffUnknown4Tex));

    gSPSegment(POLY_XLU_DISP++, 0x0A, Lib_SegmentedToVirtual(gEffUnknown5Tex));

    height = thisx->scale.y * 2400.0f;
    gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 220, 0,
                    ((height - (thisx->home.pos.y - thisx->world.pos.y)) * 255.0f) / height);

    gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, 0);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, play->gameplayFrames % 0x80, 0, 0x20, 0x40, 1, 0,
                                  (u8)(play->gameplayFrames * -15), 0x20, 0x40, 1, 0, 0, -15));

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    gSPDisplayList(POLY_XLU_DISP++, gFireTempleBigFireWallDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
