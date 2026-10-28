/*
 * File: z_elf_msg2.c
 * Overlay: ovl_Elf_Msg2
 * Description: Targetable Navi check spot
 */

#include "z_elf_msg2.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void ElfMsg2_Init(Actor* thisx, PlayState* play);
static void ElfMsg2_Destroy(Actor* thisx, PlayState* play);
static void ElfMsg2_Update(Actor* thisx, PlayState* play);
static void ElfMsg2_Draw(Actor* thisx, PlayState* play);

static s32 ElfMsg2_GetMessageId(ElfMsg2* this);
static void ElfMsg2_WaitUntilActivated(ElfMsg2* this, PlayState* play);
static void ElfMsg2_WaitForTextRead(ElfMsg2* this, PlayState* play);

ActorProfile Elf_Msg2_Oot_Profile = {
    /**/ ACTOR_ELF_MSG2_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ElfMsg2),
    /**/ ElfMsg2_Init,
    /**/ ElfMsg2_Destroy,
    /**/ ElfMsg2_Update,
    /**/ ElfMsg2_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 200, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1000, ICHAIN_STOP),
};

static void ElfMsg2_SetupAction(ElfMsg2* this, ElfMsg2ActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

/**
 * Checks a scene flag - if flag is set, the actor is killed and function returns 1. Otherwise returns 0.
 * Can also set a switch flag from params while killing.
 */
static s32 ElfMsg2_KillCheck(ElfMsg2* this, PlayState* play) {
    if ((this->actor.world.rot.y > 0) && (this->actor.world.rot.y < 0x41) &&
        Flags_GetSwitch(play, this->actor.world.rot.y - 1)) { // "Mutual destruction"
        if (((this->actor.params >> 8) & 0x3F) != 0x3F) {
            Flags_SetSwitch(play, ((this->actor.params >> 8) & 0x3F));
        }
        Actor_Kill(&this->actor);
        return 1;
    } else if ((this->actor.world.rot.y == -1) && Flags_GetClear(play, this->actor.room)) { // "Mutual destruction 2"
        if (((this->actor.params >> 8) & 0x3F) != 0x3F) {
            Flags_SetSwitch(play, ((this->actor.params >> 8) & 0x3F));
        }
        Actor_Kill(&this->actor);
        return 1;
    } else if (((this->actor.params >> 8) & 0x3F) == 0x3F) {
        return 0;
    } else if (Flags_GetSwitch(play, ((this->actor.params >> 8) & 0x3F))) { // "Mutual destruction"
        Actor_Kill(&this->actor);
        return 1;
    }
    return 0;
}

static void ElfMsg2_Init(Actor* thisx, PlayState* play) {
    ElfMsg2* this = (ElfMsg2*)thisx;

    if (!ElfMsg2_KillCheck(this, play)) {
        if ((this->actor.world.rot.x > 0) && (this->actor.world.rot.x < 8)) {
            this->actor.attentionRangeType = this->actor.world.rot.x - 1;
        }
        Actor_ProcessInitChain(thisx, sInitChain);
        if (this->actor.world.rot.y >= 0x41) {
            ElfMsg2_SetupAction(this, ElfMsg2_WaitUntilActivated);
        } else {
            ElfMsg2_SetupAction(this, ElfMsg2_WaitForTextRead);
            this->actor.flags |=
                ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_TALK_WITH_C_UP; // Make actor targetable and Navi-checkable
            this->actor.textId = ElfMsg2_GetMessageId(this);
        }
        this->actor.shape.rot.x = this->actor.shape.rot.y = this->actor.shape.rot.z = 0;
    }
}

static void ElfMsg2_Destroy(Actor* thisx, PlayState* play) {
}

static s32 ElfMsg2_GetMessageId(ElfMsg2* this) {
    return TEXT_OOT((this->actor.params & 0xFF) + 0x100);
}

/**
 * Runs while Navi text is up. Kills the actor upon closing the text box unless rot.z == 1, can also set a switch flag
 * from params.
 */
static void ElfMsg2_WaitForTextClose(ElfMsg2* this, PlayState* play) {
    s32 switchFlag;

    if (Actor_TextboxIsClosing(&this->actor, play)) {
        if (this->actor.world.rot.z != 1) {
            Actor_Kill(&this->actor);
            switchFlag = (this->actor.params >> 8) & 0x3F;
            if (switchFlag != 0x3F) {
                Flags_SetSwitch(play, switchFlag);
            }
        } else {
            ElfMsg2_SetupAction(this, ElfMsg2_WaitForTextRead);
        }
    }
}

/**
 * Runs while Navi text is not up.
 */
static void ElfMsg2_WaitForTextRead(ElfMsg2* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        ElfMsg2_SetupAction(this, ElfMsg2_WaitForTextClose);
    }
}

/**
 * Idles until a switch flag is set, at which point the actor becomes interactable
 */
static void ElfMsg2_WaitUntilActivated(ElfMsg2* this, PlayState* play) {
    // If (y >= 0x41) && (y <= 0x80), Idles until switch flag (actor.world.rot.y - 0x41) is set
    // If (y > 0x80), Idles forever (Bug?)
    if ((this->actor.world.rot.y >= 0x41) && (this->actor.world.rot.y <= 0x80) &&
        (Flags_GetSwitch(play, (this->actor.world.rot.y - 0x41)))) {
        ElfMsg2_SetupAction(this, ElfMsg2_WaitForTextRead);
        this->actor.flags |=
            ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_TALK_WITH_C_UP; // Make actor targetable and Navi-checkable
        this->actor.textId = ElfMsg2_GetMessageId(this);
    }
}

static void ElfMsg2_Update(Actor* thisx, PlayState* play) {
    ElfMsg2* this = (ElfMsg2*)thisx;

    if (!ElfMsg2_KillCheck(this, play)) {
        this->actionFunc(this, play);
    }
}

#if ZELDA_DEBUG
#include "assets/oot/overlays/ovl_Elf_Msg2/ovl_Elf_Msg2.h"
#endif

static void ElfMsg2_Draw(Actor* thisx, PlayState* play) {
#if ZELDA_DEBUG
    OPEN_DISPS(play->state.gfxCtx);

    if (R_NAVI_MSG_REGION_ALPHA == 0) {
        return;
    }

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 100, 100, 255, R_NAVI_MSG_REGION_ALPHA);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, D_809ADC38);
    gSPDisplayList(POLY_XLU_DISP++, sCubeDL);

    CLOSE_DISPS(play->state.gfxCtx);
#endif
}
