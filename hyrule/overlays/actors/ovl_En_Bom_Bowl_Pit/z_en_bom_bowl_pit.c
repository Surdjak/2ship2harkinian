#include "z_en_bom_bowl_pit.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "../ovl_En_Bom_Chu/z_en_bom_chu.h"
#include "../ovl_En_Ex_Item/z_en_ex_item.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnBomBowlPit_Init(Actor* thisx, PlayState* play);
static void EnBomBowlPit_Destroy(Actor* thisx, PlayState* play);
static void EnBomBowlPit_Update(Actor* thisx, PlayState* play);

static void EnBomBowlPit_SetupDetectHit(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_DetectHit(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_CameraDollyIn(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_SpawnPrize(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_SetupGivePrize(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_GivePrize(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_WaitTillPrizeGiven(EnBomBowlPit* this, PlayState* play);
static void EnBomBowlPit_Reset(EnBomBowlPit* this, PlayState* play);

static s32 sGetItemIds[] = { GI_BOMB_BAG_30_Oot, GI_HEART_PIECE_Oot, GI_BOMBCHUS_10_Oot, GI_BOMBS_1_Oot, GI_RUPEE_PURPLE_Oot };

ActorProfile En_Bom_Bowl_Pit_Oot_Profile = {
    /**/ ACTOR_EN_BOM_BOWL_PIT_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBomBowlPit),
    /**/ EnBomBowlPit_Init,
    /**/ EnBomBowlPit_Destroy,
    /**/ EnBomBowlPit_Update,
    /**/ NULL,
};

static void EnBomBowlPit_Init(Actor* thisx, PlayState* play) {
    EnBomBowlPit* this = (EnBomBowlPit*)thisx;

    this->actionFunc = EnBomBowlPit_SetupDetectHit;
}

static void EnBomBowlPit_Destroy(Actor* thisx, PlayState* play) {
}

static void EnBomBowlPit_SetupDetectHit(EnBomBowlPit* this, PlayState* play) {
    if (this->start != 0) {
        this->start = this->status = 0;
        this->actionFunc = EnBomBowlPit_DetectHit;
    }
}

static void EnBomBowlPit_DetectHit(EnBomBowlPit* this, PlayState* play) {
    EnBomChu* chu;
    Vec3f chuPosDiff;

    if (play->cameraPtrs[CAM_ID_MAIN]->setting == CAM_SET_FIXED1) {
        chu = (EnBomChu*)play->actorCtx.actorLists[ACTORCAT_EXPLOSIVES].first;

        while (chu != NULL) {
            if ((&chu->actor == &this->actor) || (chu->actor.id != ACTOR_EN_BOM_CHU_Oot)) {
                chu = (EnBomChu*)chu->actor.next;
                continue;
            }

            chuPosDiff.x = chu->actor.world.pos.x - this->actor.world.pos.x;
            chuPosDiff.y = chu->actor.world.pos.y - this->actor.world.pos.y;
            chuPosDiff.z = chu->actor.world.pos.z - this->actor.world.pos.z;

            if (((fabsf(chuPosDiff.x) < 40.0f) || (BREG(2))) && ((fabsf(chuPosDiff.y) < 40.0f) || (BREG(2))) &&
                ((fabsf(chuPosDiff.z) < 40.0f) || (BREG(2)))) {
                Player_SetCsActionWithHaltedActorsOot(play, NULL, 8);
                chu->timer = 1;

                this->camId = Play_CreateSubCamera(play);
                Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, this->camId, CAM_STATUS_ACTIVE);

                this->subCamAtMaxVelFrac.x = this->subCamAtMaxVelFrac.y = this->subCamAtMaxVelFrac.z = 0.1f;
                this->subCamEyeMaxVelFrac.x = this->subCamEyeMaxVelFrac.y = this->subCamEyeMaxVelFrac.z = 0.1f;

                this->subCamAt.x = this->viewAt.x = play->view.at.x;
                this->subCamAt.y = this->viewAt.y = play->view.at.y;
                this->subCamAt.z = this->viewAt.z = play->view.at.z;

                this->subCamEye.x = this->viewEye.x = play->view.eye.x;
                this->subCamEye.y = this->viewEye.y = play->view.eye.y;
                this->subCamEye.z = this->viewEye.z = play->view.eye.z;

                this->subCamAtNext.x = 20.0f;
                this->subCamAtNext.y = 100.0f;
                this->subCamAtNext.z = -800.0f;

                this->subCamEyeNext.x = 20.0f;
                this->subCamEyeNext.y = 50.0f;
                this->subCamEyeNext.z = -485.0f;

                this->subCamEyeVel.x = fabsf(this->subCamEye.x - this->subCamEyeNext.x) * 0.02f;
                this->subCamEyeVel.y = fabsf(this->subCamEye.y - this->subCamEyeNext.y) * 0.02f;
                this->subCamEyeVel.z = fabsf(this->subCamEye.z - this->subCamEyeNext.z) * 0.02f;

                this->subCamAtVel.x = fabsf(this->subCamAt.x - this->subCamAtNext.x) * 0.02f;
                this->subCamAtVel.y = fabsf(this->subCamAt.y - this->subCamAtNext.y) * 0.02f;
                this->subCamAtVel.z = fabsf(this->subCamAt.z - this->subCamAtNext.z) * 0.02f;

                Play_SetCameraAtEye(play, this->camId, &this->subCamAt, &this->subCamEye);
                this->actor.textId = TEXT_OOT(0xF);
                Message_StartTextbox(play, this->actor.textId, NULL);
                this->unk_154 = TEXT_STATE_EVENT;
                Audio_PlaySfx(NA_SE_EV_HIT_SOUND);
                Player_SetCsActionWithHaltedActorsOot(play, NULL, 8);
                this->status = 1;
                this->actionFunc = EnBomBowlPit_CameraDollyIn;
                break;
            } else {
                chu = (EnBomChu*)chu->actor.next;
            }
        }
    }
}

static void EnBomBowlPit_CameraDollyIn(EnBomBowlPit* this, PlayState* play) {
    if (this->camId != SUB_CAM_ID_DONE) {
        Math_ApproachF(&this->subCamAt.x, this->subCamAtNext.x, this->subCamAtMaxVelFrac.x, this->subCamAtVel.x);
        Math_ApproachF(&this->subCamAt.y, this->subCamAtNext.y, this->subCamAtMaxVelFrac.y, this->subCamAtVel.y);
        Math_ApproachF(&this->subCamAt.z, this->subCamAtNext.z, this->subCamAtMaxVelFrac.z, this->subCamAtVel.z);
        Math_ApproachF(&this->subCamEye.x, this->subCamEyeNext.x, this->subCamEyeMaxVelFrac.x, this->subCamEyeVel.x);
        Math_ApproachF(&this->subCamEye.y, this->subCamEyeNext.y, this->subCamEyeMaxVelFrac.y, this->subCamEyeVel.y);
        Math_ApproachF(&this->subCamEye.z, this->subCamEyeNext.z, this->subCamEyeMaxVelFrac.z, this->subCamEyeVel.z);
    }

    Play_SetCameraAtEye(play, this->camId, &this->subCamAt, &this->subCamEye);

    if ((this->unk_154 == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
    }

    if ((fabsf(this->subCamEye.x - this->subCamEyeNext.x) < 5.0f) &&
        (fabsf(this->subCamEye.y - this->subCamEyeNext.y) < 5.0f) &&
        (fabsf(this->subCamEye.z - this->subCamEyeNext.z) < 5.0f) &&
        (fabsf(this->subCamAt.x - this->subCamAtNext.x) < 5.0f) &&
        (fabsf(this->subCamAt.y - this->subCamAtNext.y) < 5.0f) &&
        (fabsf(this->subCamAt.z - this->subCamAtNext.z) < 5.0f)) {
        Message_CloseTextbox(play);
        this->timer = 30;
        this->actionFunc = EnBomBowlPit_SpawnPrize;
    }
}

static void EnBomBowlPit_SpawnPrize(EnBomBowlPit* this, PlayState* play) {
    if (this->timer == 0) {
        this->exItem = (EnExItem*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_EX_ITEM_Oot,
                                                     this->actor.world.pos.x, this->actor.world.pos.y,
                                                     this->actor.world.pos.z - 70.0f, 0, 0, 0, this->prizeIndex);
        if (this->exItem != NULL) {
            this->actionFunc = EnBomBowlPit_SetupGivePrize;
        }
    }
}

static void EnBomBowlPit_SetupGivePrize(EnBomBowlPit* this, PlayState* play) {
    if (this->exItemDone != 0) {
        switch (this->prizeIndex) {
            case EXITEM_BOMB_BAG_BOWLING:
                SET_ITEMGETINF_OOT(ITEMGETINF_11_Oot);
                break;
            case EXITEM_HEART_PIECE_BOWLING:
                SET_ITEMGETINF_OOT(ITEMGETINF_12_Oot);
                break;
        }

        Play_ClearCamera(play, this->camId);
        Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_ACTIVE);
        Player_SetCsActionWithHaltedActorsOot(play, NULL, 8);
        this->actionFunc = EnBomBowlPit_GivePrize;
    }
}

static void EnBomBowlPit_GivePrize(EnBomBowlPit* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Player_SetCsActionWithHaltedActorsOot(play, NULL, 7);
    this->getItemId = sGetItemIds[this->prizeIndex];

    if ((this->getItemId == GI_BOMB_BAG_30_Oot) && (CUR_CAPACITY(UPG_BOMB_BAG) == 30)) {
        this->getItemId = GI_BOMB_BAG_40_Oot;
    }

    player->stateFlags1 &= ~PLAYER_STATE1_20000000;
    this->actor.parent = NULL;
    if ((true)) {
        Actor_OfferGetItem(&this->actor, play, this->getItemId, 2000.0f, 1000.0f);
    }
    player->stateFlags1 |= PLAYER_STATE1_20000000;
    this->actionFunc = EnBomBowlPit_WaitTillPrizeGiven;
}

static void EnBomBowlPit_WaitTillPrizeGiven(EnBomBowlPit* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actionFunc = EnBomBowlPit_Reset;
    } else {
        Actor_OfferGetItem(&this->actor, play, this->getItemId, 2000.0f, 1000.0f);
    }
}

static void EnBomBowlPit_Reset(EnBomBowlPit* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        if (this->getItemId == GI_HEART_PIECE_Oot) {
            gSaveContext.healthAccumulator = (20 * 0x10);
            // "Ah recovery!" (?)
        }
        this->exItemDone = 0;
        this->status = 2;
        this->actionFunc = EnBomBowlPit_SetupDetectHit;
    }
}

static void EnBomBowlPit_Update(Actor* thisx, PlayState* play) {
    EnBomBowlPit* this = (EnBomBowlPit*)thisx;

    this->actionFunc(this, play);

    if (this->timer != 0) {
        this->timer--;
    }
}
