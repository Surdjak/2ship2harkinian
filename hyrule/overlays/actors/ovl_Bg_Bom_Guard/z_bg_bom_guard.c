/*
 * File: z_bg_bom_guard.c
 * Overlay: Bg_Bom_Guard
 * Description: Bombchu Bowling Alley Walls
 */

#include "z_bg_bom_guard.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "../ovl_En_Bom_Bowl_Man/z_en_bom_bowl_man.h"
#include "assets/oot/objects/object_bowl/object_bowl.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void BgBomGuard_Init(Actor* thisx, PlayState* play);
static void BgBomGuard_Destroy(Actor* thisx, PlayState* play);
static void BgBomGuard_Update(Actor* thisx, PlayState* play);

static void func_8086E638(BgBomGuard* this, PlayState* play);

ActorProfile Bg_Bom_Guard_Oot_Profile = {
    /**/ ACTOR_BG_BOM_GUARD_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_BOWL_Oot,
    /**/ sizeof(BgBomGuard),
    /**/ BgBomGuard_Init,
    /**/ BgBomGuard_Destroy,
    /**/ BgBomGuard_Update,
    /**/ NULL,
};

static void BgBomGuard_SetupAction(BgBomGuard* this, BgBomGuardActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void BgBomGuard_Init(Actor* thisx, PlayState* play) {
    BgBomGuard* this = (BgBomGuard*)thisx;
    s32 pad[2];
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gBowlingDefaultCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);


    thisx->scale.x = 1.0f;
    thisx->scale.y = 1.0f;
    thisx->scale.z = 1.0f;
    this->unk_16C = thisx->world.pos;
    BgBomGuard_SetupAction(this, func_8086E638);
}

static void BgBomGuard_Destroy(Actor* thisx, PlayState* play) {
    BgBomGuard* this = (BgBomGuard*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void func_8086E638(BgBomGuard* this, PlayState* play) {
    Actor* it = play->actorCtx.actorLists[ACTORCAT_NPC].first;
    Actor* thisx = &this->dyna.actor;

    this->unk_168 = 0;

    while (it != 0) {
        if (it->id == ACTOR_EN_BOM_BOWL_MAN_Oot) {
            if ((((EnBomBowlMan*)it)->minigamePlayStatus != 0) && (fabsf(play->view.eye.x) > -20.0f) &&
                (fabsf(play->view.eye.y) > 110.0f)) {
                this->unk_168 = 1;
            }
            break;
        }
        it = it->next;
    }

    if (this->unk_168 == 0) {
        thisx->world.pos.y = sREG(64) + -200.0f;
    } else {
        thisx->world.pos.y = 0.0f;
    }
}

static void BgBomGuard_Update(Actor* thisx, PlayState* play) {
    BgBomGuard* this = (BgBomGuard*)thisx;

    this->actionFunc(this, play);
}
