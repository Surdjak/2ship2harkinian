/*
 * File: z_item_b_heart.c
 * Overlay: ovl_Item_B_Heart
 * Description: Heart Container
 */

#include "z_item_b_heart.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_gi_hearts/object_gi_hearts.h"

#define FLAGS 0

static void ItemBHeart_Init(Actor* thisx, PlayState* play);
static void ItemBHeart_Destroy(Actor* thisx, PlayState* play);
static void ItemBHeart_Update(Actor* thisx, PlayState* play);
static void ItemBHeart_Draw(Actor* thisx, PlayState* play);

static void func_80B85264(ItemBHeart* this, PlayState* play);

ActorProfile Item_B_Heart_Oot_Profile = {
    /**/ ACTOR_ITEM_B_HEART_Oot,
    /**/ ACTORCAT_MISC,
    /**/ FLAGS,
    /**/ OBJECT_GI_HEARTS_Oot,
    /**/ sizeof(ItemBHeart),
    /**/ ItemBHeart_Init,
    /**/ ItemBHeart_Destroy,
    /**/ ItemBHeart_Update,
    /**/ ItemBHeart_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 0, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 4000, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 800, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 800, ICHAIN_STOP),
};

static void ItemBHeart_Init(Actor* thisx, PlayState* play) {
    ItemBHeart* this = (ItemBHeart*)thisx;

    if ((Flags_GetCollectible(play, 0x1F))) {
        Actor_Kill(&this->actor);
    } else {
        Actor_ProcessInitChain(&this->actor, sInitChain);
        ActorShape_Init(&this->actor.shape, 0.0f, NULL, 0.8f);
    }
}

static void ItemBHeart_Destroy(Actor* thisx, PlayState* play) {
}

static void ItemBHeart_Update(Actor* thisx, PlayState* play) {
    ItemBHeart* this = (ItemBHeart*)thisx;

    func_80B85264(this, play);
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 0x404);
    if (Actor_HasParent(&this->actor, play)) {
        Flags_SetCollectible(play, 0x1F);
        Actor_Kill(&this->actor);
    } else {
        Actor_OfferGetItem(&this->actor, play, GI_HEART_CONTAINER_2_Oot, 30.0f, 40.0f);
    }
}

static void func_80B85264(ItemBHeart* this, PlayState* play) {
    f32 yOffset;

    this->unk_164++;
    yOffset = (Math_SinS(this->unk_164 * 0x60C) * 5.0f) + 20.0f;
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y + yOffset, 0.1f, this->unk_158);
    Math_ApproachF(&this->unk_158, 2.0f, 1.0f, 0.1f);
    this->actor.shape.rot.y += 0x400;

    Math_ApproachF(&this->actor.scale.x, 0.4f, 0.1f, 0.01f);
    this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;
}

static void ItemBHeart_Draw(Actor* thisx, PlayState* play) {
    ItemBHeart* this = (ItemBHeart*)thisx;
    Actor* actorIt;
    u8 flag = false;

    OPEN_DISPS(play->state.gfxCtx);

    actorIt = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;

    while (actorIt != NULL) {
        if ((actorIt->id == ACTOR_DOOR_WARP1_Oot) && (actorIt->projectedPos.z > this->actor.projectedPos.z)) {
            flag = true;
            break;
        }
        actorIt = actorIt->next;
    }

    if (flag) {
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gGiHeartBorderDL);
        gSPDisplayList(POLY_XLU_DISP++, gGiHeartContainerDL);
    } else {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gGiHeartBorderDL);
        gSPDisplayList(POLY_OPA_DISP++, gGiHeartContainerDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
