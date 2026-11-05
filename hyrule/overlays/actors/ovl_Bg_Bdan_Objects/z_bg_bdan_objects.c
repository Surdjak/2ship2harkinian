/*
 * File: z_bg_bdan_objects.c
 * Overlay: ovl_Bg_Bdan_Objects
 * Description: Lord Jabu-Jabu Objects
 */

#include "z_bg_bdan_objects.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "assets/oot/objects/object_bdan_objects/object_bdan_objects.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgBdanObjects_Init(Actor* thisx, PlayState* play);
static void BgBdanObjects_Destroy(Actor* thisx, PlayState* play);
static void BgBdanObjects_Update(Actor* thisx, PlayState* play);
static void BgBdanObjects_Draw(Actor* thisx, PlayState* play);

static void BgBdanObjects_OctoPlatform_WaitForRutoToStartCutscene(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_OctoPlatform_RaiseToUpperPosition(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_OctoPlatform_WaitForRutoToAdvanceCutscene(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_OctoPlatform_PauseBeforeDescending(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_OctoPlatform_WaitForBigOctoToStartBattle(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_OctoPlatform_BattleInProgress(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_SinkToFloorHeight(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_WaitForPlayerInRange(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_RaiseToUpperPosition(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_DoNothing(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_ElevatorOscillate(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_WaitForSwitch(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_ChangeWaterBoxLevel(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_WaitForTimerExpired(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_WaitForPlayerOnTop(BgBdanObjects* this, PlayState* play);
static void BgBdanObjects_FallToLowerPos(BgBdanObjects* this, PlayState* play);

ActorProfile Bg_Bdan_Objects_Oot_Profile = {
    /**/ ACTOR_BG_BDAN_OBJECTS_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_BDAN_OBJECTS_Oot,
    /**/ sizeof(BgBdanObjects),
    /**/ BgBdanObjects_Init,
    /**/ BgBdanObjects_Destroy,
    /**/ BgBdanObjects_Update,
    /**/ BgBdanObjects_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x04 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_HARD,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 0x00BB, 0x0050, 0x0000, { 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

static Gfx* sDLists[] = {
    gJabuObjectsLargeRotatingSpikePlatformDL,
    gJabuElevatorPlatformDL,
    gJabuWaterDL,
    gJabuFallingPlatformDL,
};

static s32 BgBdanObjects_GetContactRu1(BgBdanObjects* this, s32 arg1) {
    switch (arg1) {
        case 0:
            return this->cameraSetting == CAM_SET_NORMAL0;
        case 4:
            return GET_INFTABLE_OOT(INFTABLE_146_Oot);
        case 3:
            return this->cameraSetting == CAM_SET_DUNGEON1;
        default:
            return -1;
    }
}

static void BgBdanObjects_SetContactRu1(BgBdanObjects* this, s32 arg1) {
    switch (arg1) {
        case 1:
            this->cameraSetting = CAM_SET_NORMAL1;
            break;
        case 2:
            this->cameraSetting = CAM_SET_DUNGEON0;
            break;
        case 4:
            SET_INFTABLE_OOT(INFTABLE_146_Oot);
            break;
        default:
    }
}

static void BgBdanObjects_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BgBdanObjects* this = (BgBdanObjects*)thisx;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    this->switchFlag = (thisx->params >> 8) & 0x3F;
    thisx->params &= 0xFF;
    if (thisx->params == 2) {
        thisx->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
        play->colCtx.colHeader->waterBoxes[7].minPos.y = thisx->world.pos.y;
        this->actionFunc = BgBdanObjects_WaitForSwitch;
        return;
    }
    if (thisx->params == 0) {
        CollisionHeader_GetVirtual(&gJabuBigOctoPlatformCol, &colHeader);
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->dyna.actor, &sCylinderInit);
        thisx->world.pos.y += -79.0f;
        if (Flags_GetClear(play, thisx->room)) {
            Flags_SetSwitch(play, this->switchFlag);
            this->actionFunc = BgBdanObjects_SinkToFloorHeight;
        } else {
            if (BgBdanObjects_GetContactRu1(this, 4)) {
                if (Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_EN_BIGOKUTA_Oot, thisx->home.pos.x,
                                       thisx->home.pos.y, thisx->home.pos.z, 0, thisx->shape.rot.y + 0x8000, 0,
                                       3) != NULL) {
                    thisx->child->world.pos.z = thisx->child->home.pos.z + 263.0f;
                }
                thisx->world.rot.y = 0;
                this->actionFunc = BgBdanObjects_OctoPlatform_BattleInProgress;
                thisx->world.pos.y = thisx->home.pos.y + -70.0f;
            } else {
                Flags_SetSwitch(play, this->switchFlag);
                this->timer = 0;
                this->actionFunc = BgBdanObjects_OctoPlatform_WaitForRutoToStartCutscene;
            }
        }
    } else {
        if (thisx->params == 1) {
            CollisionHeader_GetVirtual(&gJabuElevatorCol, &colHeader);
            this->timer = 512;
            this->switchFlag = 0;
            this->actionFunc = BgBdanObjects_ElevatorOscillate;
        } else {
            CollisionHeader_GetVirtual(&gJabuLoweringPlatformCol, &colHeader);
            if (Flags_GetSwitch(play, this->switchFlag)) {
                this->actionFunc = BgBdanObjects_DoNothing;
                thisx->world.pos.y = thisx->home.pos.y - 400.0f;
            } else {
                this->actionFunc = BgBdanObjects_WaitForPlayerOnTop;
            }
        }
    }
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
}

static void BgBdanObjects_Destroy(Actor* thisx, PlayState* play) {
    BgBdanObjects* this = (BgBdanObjects*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    if (thisx->params == 0) {
        Collider_DestroyCylinder(play, &this->collider);
    }
}

static void BgBdanObjects_OctoPlatform_WaitForRutoToStartCutscene(BgBdanObjects* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (BgBdanObjects_GetContactRu1(this, 0)) {
        if (this->dyna.actor.xzDistToPlayer < 250.0f) {
            BgBdanObjects_SetContactRu1(this, 1);
            this->timer = 20;
            OnePointCutscene_Init(play, 3070, -99, &this->dyna.actor, CAM_ID_MAIN);
            player->actor.world.pos.x = -1130.0f;
            player->actor.world.pos.y = -1025.0f;
            player->actor.world.pos.z = -3300.0f;
            Rumble_Request(0.0f, 0xFF, 0x14, 0x96);
        }
    } else if (this->timer != 0) {
        if (this->timer != 0) {
            this->timer--;
        }
        if (this->timer == 0) {
            this->actionFunc = BgBdanObjects_OctoPlatform_RaiseToUpperPosition;
        }
    }

    if (!Play_InCsMode(play) && !BgBdanObjects_GetContactRu1(this, 0)) {
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y + -79.0f;
    } else {
        this->dyna.actor.world.pos.y = (this->dyna.actor.home.pos.y + -79.0f) - 5.0f;
    }
}

static void BgBdanObjects_OctoPlatform_RaiseToUpperPosition(BgBdanObjects* this, PlayState* play) {
    if (Math_SmoothStepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 500.0f, 0.5f, 7.5f, 1.0f) <
        0.1f) {
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_DOOR_BELL);
        this->actionFunc = BgBdanObjects_OctoPlatform_WaitForRutoToAdvanceCutscene;
        this->timer = 30;
        BgBdanObjects_SetContactRu1(this, 2);
        Rumble_Request(0.0f, 0xFF, 0x14, 0x96);
    } else {
        if (this->timer != 0) {
            this->timer--;
        }
        if (this->timer == 0) {
            Rumble_Request(0.0f, 0x78, 0x14, 0xA);
            this->timer = 11;
        }
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BUYOSTAND_RISING - SFX_FLAG);
    }
}

static void BgBdanObjects_OctoPlatform_WaitForRutoToAdvanceCutscene(BgBdanObjects* this, PlayState* play) {
    s32 temp;

    if (this->timer != 0) {
        this->timer--;
        if (this->timer == 0) {
            temp = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_1);
            Quake_SetSpeed(temp, 0x3A98);
            Quake_SetPerturbations(temp, 0, 1, 0xFA, 1);
            Quake_SetDuration(temp, 0xA);
        }
    }

    if (BgBdanObjects_GetContactRu1(this, 3)) {
        Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_EN_BIGOKUTA_Oot, this->dyna.actor.world.pos.x,
                           this->dyna.actor.world.pos.y + 140.0f, this->dyna.actor.world.pos.z, 0,
                           this->dyna.actor.shape.rot.y + 0x8000, 0, 0);
        BgBdanObjects_SetContactRu1(this, 4);
        this->timer = 10;
        this->actionFunc = BgBdanObjects_OctoPlatform_PauseBeforeDescending;
        Camera_SetFinishedFlag(GET_ACTIVE_CAM(play));
    }
}

static void BgBdanObjects_OctoPlatform_DescendWithBigOcto(BgBdanObjects* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->dyna.actor.velocity.y += 0.5f;
    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + -70.0f,
                     this->dyna.actor.velocity.y)) {
        this->dyna.actor.world.rot.y = 0;
        this->timer = 60;
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BOUND_ON_MAGMA);
        this->dyna.actor.child->world.pos.y = this->dyna.actor.world.pos.y + 140.0f;
        this->actionFunc = BgBdanObjects_OctoPlatform_WaitForBigOctoToStartBattle;
        OnePointCutscene_Init(play, 3080, -99, this->dyna.actor.child, CAM_ID_MAIN);
        player->actor.world.pos.x = -1130.0f;
        player->actor.world.pos.y = -1025.0f;
        player->actor.world.pos.z = -3500.0f;
        player->actor.shape.rot.y = 0x7530;
        player->actor.world.rot.y = player->actor.shape.rot.y;
        Rumble_Request(0.0f, 0xFF, 0x1E, 0x96);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BUYOSTAND_FALL - SFX_FLAG);
        if (this->timer != 0) {
            this->timer--;
        }
        if (this->timer == 0) {
            Rumble_Request(0.0f, 0x78, 0x14, 0xA);
            this->timer = 11;
        }
        if (this->dyna.actor.child != NULL) {
            this->dyna.actor.child->world.pos.y = this->dyna.actor.world.pos.y + 140.0f;
        }
    }
}

static void BgBdanObjects_OctoPlatform_PauseBeforeDescending(BgBdanObjects* this, PlayState* play) {
    this->timer--;

    if (this->timer == 0) {
        Flags_UnsetSwitch(play, this->switchFlag);
    } else if (this->timer == -40) {
        this->timer = 0;
        this->actionFunc = BgBdanObjects_OctoPlatform_DescendWithBigOcto;
    }
}

static void BgBdanObjects_OctoPlatform_WaitForBigOctoToStartBattle(BgBdanObjects* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    if ((this->timer == 0) && (this->dyna.actor.child != NULL)) {
        if (this->dyna.actor.child->params == 2) {
            this->actionFunc = BgBdanObjects_OctoPlatform_BattleInProgress;
        } else if (this->dyna.actor.child->params == 0) {
            this->dyna.actor.child->params = 1;
        }
    }
}

static void BgBdanObjects_OctoPlatform_BattleInProgress(BgBdanObjects* this, PlayState* play) {
    Collider_UpdateCylinder(&this->dyna.actor, &this->collider);
    CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    if (Flags_GetClear(play, this->dyna.actor.room)) {
        Flags_SetSwitch(play, this->switchFlag);
        this->dyna.actor.home.rot.y = (s16)(this->dyna.actor.shape.rot.y + 0x2000) & 0xC000;
        this->actionFunc = BgBdanObjects_SinkToFloorHeight;
    } else {
        this->dyna.actor.shape.rot.y += this->dyna.actor.world.rot.y;
        Audio_PlaySfx_AtPosWithFreq(&this->dyna.actor.projectedPos, 0x2063, ABS(this->dyna.actor.world.rot.y) / 512.0f);
    }
}

static void BgBdanObjects_SinkToFloorHeight(BgBdanObjects* this, PlayState* play) {
    s32 cond = Math_ScaledStepToS(&this->dyna.actor.shape.rot.y, this->dyna.actor.home.rot.y, 0x200);

    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + -125.0f, 3.0f)) {
        if (cond) {
            this->actionFunc = BgBdanObjects_WaitForPlayerInRange;
        }
    }
}

static void BgBdanObjects_WaitForPlayerInRange(BgBdanObjects* this, PlayState* play) {
    if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        if (this->dyna.actor.xzDistToPlayer < 120.0f) {
            this->actionFunc = BgBdanObjects_RaiseToUpperPosition;
            OnePointCutscene_Init(play, 3090, -99, &this->dyna.actor, CAM_ID_MAIN);
        }
    }
}

static void BgBdanObjects_RaiseToUpperPosition(BgBdanObjects* this, PlayState* play) {
    if (Math_SmoothStepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 965.0f, 0.5f, 15.0f, 0.2f) <
        0.01f) {
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_DOOR_BELL);
        this->actionFunc = BgBdanObjects_DoNothing;
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BUYOSTAND_RISING - SFX_FLAG);
    }
}

static void BgBdanObjects_DoNothing(BgBdanObjects* this, PlayState* play) {
}

static void BgBdanObjects_ElevatorOscillate(BgBdanObjects* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    if (this->switchFlag == 0) {
        if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
            this->cameraSetting = play->cameraPtrs[CAM_ID_MAIN]->setting;
            Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_NORMAL2);
            Camera_UnsetStateFlag(play->cameraPtrs[CAM_ID_MAIN], 4);
            this->switchFlag = 10;
        }
    } else {
        Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], CAM_SET_NORMAL2);
        if (!DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
            if (this->switchFlag != 0) {
                this->switchFlag--;
            }
        }
        if (this->switchFlag == 0) {
            Camera_ChangeSetting(play->cameraPtrs[CAM_ID_MAIN], this->cameraSetting);
            Camera_SetStateFlag(play->cameraPtrs[CAM_ID_MAIN], 4);
        }
    }
    this->dyna.actor.world.pos.y =
        this->dyna.actor.home.pos.y - (sinf(this->timer * (M_PI / 256.0f)) * 471.24f); // pi * 150
    if (this->timer == 0) {
        this->timer = 512;
    }
}

static void BgBdanObjects_WaitForSwitch(BgBdanObjects* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->switchFlag)) {
        this->timer = 100;
        this->actionFunc = BgBdanObjects_ChangeWaterBoxLevel;
    }
}

static void BgBdanObjects_ChangeWaterBoxLevel(BgBdanObjects* this, PlayState* play) {
    if (this->timer == 0) {
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, 0.5f)) {
            Flags_UnsetSwitch(play, this->switchFlag);
            this->actionFunc = BgBdanObjects_WaitForSwitch;
        }
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
    } else {
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 75.0f, 0.5f)) {
            this->actionFunc = BgBdanObjects_WaitForTimerExpired;
        }
        Actor_PlaySfx_FlaggedCentered2(&this->dyna.actor, NA_SE_EV_WATER_LEVEL_DOWN - SFX_FLAG);
    }
    play->colCtx.colHeader->waterBoxes[7].minPos.y = this->dyna.actor.world.pos.y;
}

static void BgBdanObjects_WaitForTimerExpired(BgBdanObjects* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }
    Actor_PlaySfx_FlaggedTimer(&this->dyna.actor, this->timer);
    if (this->timer == 0) {
        this->actionFunc = BgBdanObjects_ChangeWaterBoxLevel;
    }
}

static void BgBdanObjects_WaitForPlayerOnTop(BgBdanObjects* this, PlayState* play) {
    if (DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
        Flags_SetSwitch(play, this->switchFlag);
        this->timer = 50;
        this->actionFunc = BgBdanObjects_FallToLowerPos;
        this->dyna.actor.home.pos.y -= 200.0f;
        OnePointCutscene_Init(play, 3100, 51, &this->dyna.actor, CAM_ID_MAIN);
    }
}

static void BgBdanObjects_FallToLowerPos(BgBdanObjects* this, PlayState* play) {
    if (this->timer != 0) {
        this->timer--;
    }

    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - (cosf(this->timer * (M_PI / 50.0f)) * 200.0f);

    if (this->timer == 0) {
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_BOUND_ON_MAGMA);
        this->actionFunc = BgBdanObjects_DoNothing;
        Play_CopyCamera(play, CAM_ID_MAIN, CAM_ID_NONE);
    } else {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_BUYOSTAND_FALL - SFX_FLAG);
    }
}

static void BgBdanObjects_Update(Actor* thisx, PlayState* play) {
    BgBdanObjects* this = (BgBdanObjects*)thisx;

    Actor_SetFocus(thisx, 50.0f);
    this->actionFunc(this, play);
}

static void BgBdanObjects_Draw(Actor* thisx, PlayState* play) {
    BgBdanObjects* this = (BgBdanObjects*)thisx;

    if (thisx->params == 0) {
        if (this->actionFunc == BgBdanObjects_OctoPlatform_WaitForRutoToStartCutscene) {
            if (((thisx->home.pos.y + -79.0f) - 5.0f) < thisx->world.pos.y) {
                Matrix_Translate(0.0f, -50.0f, 0.0f, MTXMODE_APPLY);
            }
        }
    }

    if (thisx->params == 2) {
        Gfx_DrawDListXlu(play, gJabuWaterDL);
    } else {
        Gfx_DrawDListOpa(play, sDLists[thisx->params]);
    }
}
