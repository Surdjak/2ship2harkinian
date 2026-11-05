/*
 * File: z_obj_roomtimer.c
 * Overlay: ovl_Obj_Roomtimer
 * Description: Starts Timer 1 with a value specified in params
 */

#include "z_obj_roomtimer.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void ObjRoomtimer_Init(Actor* thisx, PlayState* play);
static void ObjRoomtimer_Destroy(Actor* thisx, PlayState* play);
static void ObjRoomtimer_Update(Actor* thisx, PlayState* play);

static void func_80B9D054(ObjRoomtimer* this, PlayState* play);
static void func_80B9D0B0(ObjRoomtimer* this, PlayState* play);

ActorProfile Obj_Roomtimer_Oot_Profile = {
    /**/ ACTOR_OBJ_ROOMTIMER_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ObjRoomtimer),
    /**/ ObjRoomtimer_Init,
    /**/ ObjRoomtimer_Destroy,
    /**/ ObjRoomtimer_Update,
    /**/ NULL,
};

static void ObjRoomtimer_Init(Actor* thisx, PlayState* play) {
    ObjRoomtimer* this = (ObjRoomtimer*)thisx;
    s16 params = this->actor.params;

    this->switchFlag = (params >> 10) & 0x3F;
    this->actor.params = params & 0x3FF;
    params = this->actor.params;

    if (params != 0x3FF) {
        if (params > 600) {
            this->actor.params = 600;
        } else {
            this->actor.params = params;
        }
    }

    this->actionFunc = func_80B9D054;
}

static void ObjRoomtimer_Destroy(Actor* thisx, PlayState* play) {
    ObjRoomtimer* this = (ObjRoomtimer*)thisx;

    if ((this->actor.params != 0x3FF) && (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)) > 0)) {
        gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
    }
}

static void func_80B9D054(ObjRoomtimer* this, PlayState* play) {
    if (this->actor.params != 0x3FF) {
        Interface_StartTimer(TIMER_ID_MINIGAME_1, this->actor.params);
    }

    Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_PROP);
    this->actionFunc = func_80B9D0B0;
}

static void func_80B9D0B0(ObjRoomtimer* this, PlayState* play) {
    if (Flags_GetClearTemp(play, this->actor.room)) {
        if (this->actor.params != 0x3FF) {
            gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
        }
        Flags_SetClear(play, this->actor.room);
        Flags_SetSwitch(play, this->switchFlag);
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        Actor_Kill(&this->actor);
    } else {
        if ((this->actor.params != 0x3FF) && (((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100)) == 0)) {
            Audio_PlaySfx(NA_SE_OC_ABYSS);
            func_80169FDC(play);
            Actor_Kill(&this->actor);
        }
    }
}

static void ObjRoomtimer_Update(Actor* thisx, PlayState* play) {
    ObjRoomtimer* this = (ObjRoomtimer*)thisx;

    this->actionFunc(this, play);
}
