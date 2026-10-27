/*
 * File: z_door_ana.c
 * Overlay: ovl_Door_Ana
 * Description: Grottos Entrances/Exits
 */

#include "z_door_ana.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "assets/oot/objects/gameplay_field_keep/gameplay_field_keep.h"

#define FLAGS ACTOR_FLAG_UPDATE_DURING_OCARINA

static void DoorAna_Init(Actor* thisx, PlayState* play);
static void DoorAna_Destroy(Actor* thisx, PlayState* play);
static void DoorAna_Update(Actor* thisx, PlayState* play);
static void DoorAna_Draw(Actor* thisx, PlayState* play);

static void DoorAna_WaitClosed(DoorAna* this, PlayState* play);
static void DoorAna_WaitOpen(DoorAna* this, PlayState* play);
static void DoorAna_GrabPlayer(DoorAna* this, PlayState* play);

ActorProfile Door_Ana_Oot_Profile = {
    /**/ ACTOR_DOOR_ANA_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_GAMEPLAY_FIELD_KEEP_Oot,
    /**/ sizeof(DoorAna),
    /**/ DoorAna_Init,
    /**/ DoorAna_Destroy,
    /**/ DoorAna_Update,
    /**/ DoorAna_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0x00000048), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_NONE,
    },
    { 50, 10, 0, { 0 } },
};

// array of entrance table entries to grotto destinations
// OoT entrance indexes (ENTR_FAIRYS_FOUNTAIN_0, then ENTR_GROTTOS_0 to _13), translated by Hyrule_EntranceFromOot.
static s16 sGrottoEntrances[] = {
    0x36D, 0x03F, 0x598, 0x59C, 0x5A0, 0x5A4, 0x5A8, 0x5AC,
    0x5B0, 0x5B4, 0x5B8, 0x5BC, 0x5C0, 0x5C4, 0x5FC,
};

static void DoorAna_SetupAction(DoorAna* this, DoorAnaActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void DoorAna_Init(Actor* thisx, PlayState* play) {
    DoorAna* this = (DoorAna*)thisx;

    this->actor.shape.rot.z = 0;
    this->actor.shape.rot.y = this->actor.shape.rot.z;
    // init block for grottos that are initially "hidden" (require explosives/hammer/song of storms to open)
    if ((this->actor.params & 0x300) != 0) {
        // only allocate collider for grottos that need bombing/hammering open
        if ((this->actor.params & 0x200) != 0) {
            Collider_InitCylinder(play, &this->collider);
            Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        } else {
            this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        }
        Actor_SetScale(&this->actor, 0);
        DoorAna_SetupAction(this, DoorAna_WaitClosed);
    } else {
        DoorAna_SetupAction(this, DoorAna_WaitOpen);
    }
    this->actor.attentionRangeType = 0;
}

static void DoorAna_Destroy(Actor* thisx, PlayState* play) {
    DoorAna* this = (DoorAna*)thisx;

    // free collider if it has one
    if ((this->actor.params & 0x200) != 0) {
        Collider_DestroyCylinder(play, &this->collider);
    }
}

// update routine for grottos that are currently "hidden"/unopened
static void DoorAna_WaitClosed(DoorAna* this, PlayState* play) {
    u32 openGrotto = false;

    if (!(this->actor.params & 0x200)) {
        // opening with song of storms
        // OoT's storm flag (Flags_GetEnv 5) is MM's ocarina mode after the Song of Storms is played.
        if (this->actor.xyzDistToPlayerSq < 40000.0f && (play->msgCtx.ocarinaMode == OCARINA_MODE_PLAYED_STORMS)) {
            openGrotto = true;
            this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        }
    } else {
        // bombing/hammering open a grotto
        if (this->collider.base.acFlags & AC_HIT) {
            openGrotto = true;
            Collider_DestroyCylinder(play, &this->collider);
        } else {
            Collider_UpdateCylinder(&this->actor, &this->collider);
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        }
    }
    // open the grotto
    if (openGrotto) {
        this->actor.params &= ~0x0300;
        DoorAna_SetupAction(this, DoorAna_WaitOpen);
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
    }
    Actor_SetClosestSecretDistance(&this->actor, play);
}

// update routine for grottos that are open
static void DoorAna_WaitOpen(DoorAna* this, PlayState* play) {
    Player* player;
    s32 destinationIdx;

    player = GET_PLAYER(play);
    if (Math_StepToF(&this->actor.scale.x, 0.01f, 0.001f)) {
        if ((this->actor.attentionRangeType != 0) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
            (player->stateFlags1 & PLAYER_STATE1_80000000) && (player->av1.actionVar1 == 0)) {
            destinationIdx = ((this->actor.params >> 0xC) & 7) - 1;
            // MM keeps OoT's grotto return point as RESPAWN_MODE_UNK_3, which its Player and En_Torch read.
            Play_SetupRespawnPoint(play, RESPAWN_MODE_UNK_3, PLAYER_PARAMS(0xFF, PLAYER_START_MODE_GROTTO));
            gSaveContext.respawn[RESPAWN_MODE_UNK_3].pos.y = this->actor.world.pos.y;
            gSaveContext.respawn[RESPAWN_MODE_UNK_3].yaw = this->actor.home.rot.y;
            gSaveContext.respawn[RESPAWN_MODE_UNK_3].data = this->actor.params & 0xFFFF;
            if (destinationIdx < 0) {
                destinationIdx = this->actor.home.rot.z + 1;
            }
            play->nextEntrance = Hyrule_EntranceFromOot(sGrottoEntrances[destinationIdx]);

            DoorAna_SetupAction(this, DoorAna_GrabPlayer);
        } else {
            if (!Player_InCsMode(play) && !(player->stateFlags1 & (PLAYER_STATE1_800000 | PLAYER_STATE1_8000000)) &&
                this->actor.xzDistToPlayer <= 15.0f && -50.0f <= this->actor.playerHeightRel &&
                this->actor.playerHeightRel <= 15.0f) {
                player->stateFlags1 |= PLAYER_STATE1_80000000;
                this->actor.attentionRangeType = 1;
            } else {
                this->actor.attentionRangeType = 0;
            }
        }
    }
    Actor_SetScale(&this->actor, this->actor.scale.x);
}

// update function for after the player has triggered the grotto
static void DoorAna_GrabPlayer(DoorAna* this, PlayState* play) {
    Player* player;

    if (this->actor.playerHeightRel <= 0.0f && 15.0f < this->actor.xzDistToPlayer) {
        player = GET_PLAYER(play);
        player->actor.world.pos.x = Math_SinS(this->actor.yawTowardsPlayer) * 15.0f + this->actor.world.pos.x;
        player->actor.world.pos.z = Math_CosS(this->actor.yawTowardsPlayer) * 15.0f + this->actor.world.pos.z;
    }
}

static void DoorAna_Update(Actor* thisx, PlayState* play) {
    DoorAna* this = (DoorAna*)thisx;

    this->actionFunc(this, play);
    this->actor.shape.rot.y = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
}

static void DoorAna_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gGrottoDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
