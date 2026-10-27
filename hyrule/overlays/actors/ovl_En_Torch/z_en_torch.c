/*
 * File: z_en_torch.c
 * Overlay: ovl_En_Torch
 * Description: Spawns a chest with the appropriate contents then unloads. Used in grottos.
 */

#include "z_en_torch.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS 0

static void EnTorch_Init(Actor* thisx, PlayState* play);

ActorProfile En_Torch_Oot_Profile = {
    /**/ ACTOR_EN_TORCH_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnTorch),
    /**/ EnTorch_Init,
    /**/ NULL,
    /**/ NULL,
    /**/ NULL,
};

// MM's chest (En_Box) and get items: the chests share their parameter layout, and MM has all of these (OoT's
// GI_RUPEE_GOLD is MM's GI_RUPEE_HUGE).
static u8 sChestContents[] = {
    GI_RUPEE_BLUE, GI_RUPEE_RED, GI_RUPEE_HUGE, GI_BOMBS_20, GI_BOMBS_1, GI_BOMBS_1, GI_BOMBS_1, GI_BOMBS_1,
};

static void EnTorch_Init(Actor* thisx, PlayState* play) {
    EnTorch* this = (EnTorch*)thisx;
    // MM keeps OoT's grotto return point as RESPAWN_MODE_UNK_3.
    s8 returnData = gSaveContext.respawn[RESPAWN_MODE_UNK_3].data;

    /* Spawn chest with desired contents.
       Contents are passed to en_torch from grotto params via Save Context. */
    Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOX, this->actor.world.pos.x, this->actor.world.pos.y,
                this->actor.world.pos.z, 0, this->actor.shape.rot.y, 0,
                (sChestContents[(returnData >> 0x5) & 0x7] << 0x5) | 0x5000 | (returnData & 0x1F));

    Actor_Kill(&this->actor);
}
