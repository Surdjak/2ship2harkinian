/*
 * File: z_en_dns.c
 * Overlay: En_Dns
 * Description: Deku Salesman
 */

#include "z_en_dns.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_shopnuts/object_shopnuts.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnDns_Init(Actor* thisx, PlayState* play);
static void EnDns_Destroy(Actor* thisx, PlayState* play);
static void EnDns_Update(Actor* thisx, PlayState* play);
static void EnDns_Draw(Actor* thisx, PlayState* play);

static u32 EnDns_CanBuyPrice(EnDns* this);
static u32 EnDns_CanBuyDekuNuts(EnDns* this);
static u32 EnDns_CanBuyDekuSticks(EnDns* this);
static u32 EnDns_CanBuyDekuSeeds(EnDns* this);
static u32 EnDns_CanBuyDekuShield(EnDns* this);
static u32 EnDns_CanBuyBombs(EnDns* this);
static u32 EnDns_CanBuyArrows(EnDns* this);
static u32 EnDns_CanBuyBottle(EnDns* this);

static void EnDns_PayPrice(EnDns* this);
static void EnDns_PayForDekuNuts(EnDns* this);
static void EnDns_PayForHeartPiece(EnDns* this);
static void EnDns_PayForBombs(EnDns* this);
static void EnDns_PayForArrows(EnDns* this);
static void EnDns_PayForDekuStickUpgrade(EnDns* this);
static void EnDns_PayForDekuNutUpgrade(EnDns* this);

static void EnDns_SetupIdle(EnDns* this, PlayState* play);
static void EnDns_Idle(EnDns* this, PlayState* play);
static void EnDns_Talk(EnDns* this, PlayState* play);
static void EnDns_OfferSaleItem(EnDns* this, PlayState* play);
static void EnDns_SetupSale(EnDns* this, PlayState* play);
static void EnDns_Sale(EnDns* this, PlayState* play);
static void EnDns_SetupBurrow(EnDns* this, PlayState* play);
static void EnDns_SetupNoSaleBurrow(EnDns* this, PlayState* play);
static void EnDns_Burrow(EnDns* this, PlayState* play);
static void EnDns_PostBurrow(EnDns* this, PlayState* play);

ActorProfile En_Dns_Oot_Profile = {
    /**/ ACTOR_EN_DNS_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SHOPNUTS_Oot,
    /**/ sizeof(EnDns),
    /**/ EnDns_Init,
    /**/ EnDns_Destroy,
    /**/ EnDns_Update,
    /**/ EnDns_Draw,
};

static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 18, 32, 0, { 0, 0, 0 } },
};

static u16 sStartingTextIds[] = {
    TEXT_OOT(0x10A0), TEXT_OOT(0x10A1), TEXT_OOT(0x10A2), TEXT_OOT(0x10CA), TEXT_OOT(0x10CB), TEXT_OOT(0x10CC), TEXT_OOT(0x10CD), TEXT_OOT(0x10CE), TEXT_OOT(0x10CF), TEXT_OOT(0x10DC), TEXT_OOT(0x10DD),
};

// Debug text: "sells"  { "Deku Nuts",    "Deku Sticks",        "Piece of Heart",  "Deku Seeds",
//                        "Deku Shield",  "Bombs",              "Arrows",          "Red Potion",
//                        "Green Potion", "Deku Stick Upgrade", "Deku Nut Upgrade" }
static char* sItemDebugTxt[] = {
    "デクの実売り            ", "デクの棒売り            ", "ハートの欠片売り        ", "デクの種売り            ",
    "デクの盾売り            ", "バクダン売り            ", "矢売り                  ", "赤のくすり売り          ",
    "緑のくすり売り          ", "デクの棒持てる数を増やす", "デクの実持てる数を増やす",
};

static DnsItemEntry sItemNuts = { 20, 5, GI_NUTS_5_2_Oot, EnDns_CanBuyDekuNuts, EnDns_PayForDekuNuts };
static DnsItemEntry sItemSticks = { 15, 1, GI_STICKS_1_Oot, EnDns_CanBuyDekuSticks, EnDns_PayPrice };
static DnsItemEntry sItemHeartPiece = { 10, 1, GI_HEART_PIECE_Oot, EnDns_CanBuyPrice, EnDns_PayForHeartPiece };
static DnsItemEntry sItemSeeds = { 40, 30, GI_SEEDS_30_Oot, EnDns_CanBuyDekuSeeds, EnDns_PayPrice };
static DnsItemEntry sItemDekuShield = { 50, 1, GI_SHIELD_DEKU_Oot, EnDns_CanBuyDekuShield, EnDns_PayPrice };
static DnsItemEntry sItemBombs = { 40, 5, GI_BOMBS_5_Oot, EnDns_CanBuyBombs, EnDns_PayForBombs };
static DnsItemEntry sItemArrows = { 70, 20, GI_ARROWS_LARGE_Oot, EnDns_CanBuyArrows, EnDns_PayForArrows };
static DnsItemEntry sItemRedPotion = { 40, 1, GI_POTION_RED_Oot, EnDns_CanBuyBottle, EnDns_PayPrice };
static DnsItemEntry sItemGreenPotion = { 40, 1, GI_POTION_GREEN_Oot, EnDns_CanBuyBottle, EnDns_PayPrice };

static DnsItemEntry sItemStickUpgrade = { 40, 1, GI_STICK_UPGRADE_20_Oot, EnDns_CanBuyPrice, EnDns_PayForDekuStickUpgrade };
static DnsItemEntry sItemNutUpgrade = { 40, 1, GI_NUT_UPGRADE_30_Oot, EnDns_CanBuyPrice, EnDns_PayForDekuNutUpgrade };

static DnsItemEntry* sItemEntries[] = {
    &sItemNuts,   &sItemSticks,    &sItemHeartPiece,  &sItemSeeds,        &sItemDekuShield, &sItemBombs,
    &sItemArrows, &sItemRedPotion, &sItemGreenPotion, &sItemStickUpgrade, &sItemNutUpgrade,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x4E
    ICHAIN_U8(attentionRangeType, 2, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 30, ICHAIN_STOP),
};

static AnimationMinimalInfo sAnimationInfo[] = {
    { &gBusinessScrubNervousIdleAnim, ANIMMODE_LOOP, 0.0f },
    { &gBusinessScrubLeaveBurrowAnim, ANIMMODE_ONCE, 0.0f },
    { &gBusinessScrubNervousTransitionAnim, ANIMMODE_ONCE, 0.0f },
};

static void EnDns_Init(Actor* thisx, PlayState* play) {
    EnDns* this = (EnDns*)thisx;

    if (this->actor.params < 0) {
        Actor_Kill(&this->actor);
        return;
    }
    // Sell Seeds instead of Arrows if Link is child
    if ((this->actor.params == DNS_TYPE_ARROWS_30) && (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD)) {
        this->actor.params = DNS_TYPE_DEKU_SEEDS_30;
    }
    Actor_ProcessInitChain(&this->actor, sInitChain);
    SkelAnime_InitFlex(play, &this->skelAnime, &gBusinessScrubSkel, &gBusinessScrubNervousTransitionAnim,
                       this->jointTable, this->morphTable, 18);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 35.0f);
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->maintainCollider = 1;
    this->standOnGround = 1;
    this->dropCollectible = 0;
    this->actor.speed = 0.0f;
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = -1.0f;
    this->actor.textId = sStartingTextIds[this->actor.params];
    this->dnsItemEntry = sItemEntries[this->actor.params];
    this->actionFunc = EnDns_SetupIdle;
}

static void EnDns_Destroy(Actor* thisx, PlayState* play) {
    EnDns* this = (EnDns*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnDns_ChangeAnim(EnDns* this, u8 index) {
    s16 frameCount;

    frameCount = Animation_GetLastFrame(sAnimationInfo[index].animation);
    this->animIndex = index; // Not used anywhere else?
    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f, (f32)frameCount,
                     sAnimationInfo[index].mode, sAnimationInfo[index].morphFrames);
}

/* Item give checking functions */

static u32 EnDns_CanBuyDekuNuts(EnDns* this) {
    if ((CUR_CAPACITY(UPG_DEKU_NUTS) != 0) && (AMMO(ITEM_NUT_Oot) >= CUR_CAPACITY(UPG_DEKU_NUTS))) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_NUT_Oot) == ITEM_NONE) {
        return DNS_CANBUY_RESULT_SUCCESS_NEW_ITEM;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyDekuSticks(EnDns* this) {
    if ((CUR_CAPACITY(UPG_DEKU_STICKS) != 0) && (AMMO(ITEM_STICK_Oot) >= CUR_CAPACITY(UPG_DEKU_STICKS))) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_STICK_Oot) == ITEM_NONE) {
        return DNS_CANBUY_RESULT_SUCCESS_NEW_ITEM;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyPrice(EnDns* this) {
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyDekuSeeds(EnDns* this) {
    if (INV_CONTENT_OOT(ITEM_SLINGSHOT_Oot) == ITEM_NONE) {
        return DNS_CANBUY_RESULT_CANT_GET_NOW;
    }
    if (AMMO(ITEM_SLINGSHOT_Oot) >= CUR_CAPACITY(UPG_BULLET_BAG)) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_SEEDS_Oot) == ITEM_NONE) {
        return DNS_CANBUY_RESULT_SUCCESS_NEW_ITEM;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyDekuShield(EnDns* this) {
    if (CHECK_OWNED_EQUIP_OOT(1, EQUIP_OWNED_SHIELD_DEKU)) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyBombs(EnDns* this) {
    if (!CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot)) {
        return DNS_CANBUY_RESULT_CANT_GET_NOW;
    }
    if (AMMO(ITEM_BOMB_Oot) >= CUR_CAPACITY(UPG_BOMB_BAG)) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyArrows(EnDns* this) {
    if (Item_CheckObtainability(ITEM_BOW_Oot) == ITEM_NONE) {
        return DNS_CANBUY_RESULT_CANT_GET_NOW;
    }
    if (AMMO(ITEM_BOW_Oot) >= CUR_CAPACITY(UPG_QUIVER)) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

static u32 EnDns_CanBuyBottle(EnDns* this) {
    if (!Inventory_HasEmptyBottle()) {
        return DNS_CANBUY_RESULT_CAPACITY_FULL;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->dnsItemEntry->itemPrice) {
        return DNS_CANBUY_RESULT_NEED_RUPEES;
    }
    return DNS_CANBUY_RESULT_SUCCESS;
}

/* Paying and flagging functions */

static void EnDns_PayPrice(EnDns* this) {
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForDekuNuts(EnDns* this) {
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForHeartPiece(EnDns* this) {
    SET_ITEMGETINF_OOT(ITEMGETINF_DEKU_SCRUB_HEART_PIECE_Oot);
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForBombs(EnDns* this) {
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForArrows(EnDns* this) {
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForDekuStickUpgrade(EnDns* this) {
    SET_INFTABLE_OOT(INFTABLE_BOUGHT_STICK_UPGRADE_Oot);
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_PayForDekuNutUpgrade(EnDns* this) {
    SET_INFTABLE_OOT(INFTABLE_BOUGHT_NUT_UPGRADE_Oot);
    Rupees_ChangeBy(-this->dnsItemEntry->itemPrice);
}

static void EnDns_SetupIdle(EnDns* this, PlayState* play) {
    if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
        this->actionFunc = EnDns_Idle;
        EnDns_ChangeAnim(this, DNS_ANIM_IDLE);
    }
}

static void EnDns_Idle(EnDns* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 2000, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnDns_Talk;
    } else {
        if ((this->collider.base.ocFlags1 & OC1_HIT) || this->actor.isLockedOn) {
            this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        } else {
            this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        }
        if ((this->actor.xzDistToPlayer < 130.0f)) {
            Actor_OfferTalkNearColChkInfoCylinder(&this->actor, play);
        }
    }
}

static void EnDns_Talk(EnDns* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0: // OK
                switch (this->dnsItemEntry->purchaseableCheck(this)) {
                    case DNS_CANBUY_RESULT_NEED_RUPEES:
                        Message_ContinueTextbox(play, TEXT_OOT(0x10A5));
                        this->actionFunc = EnDns_SetupNoSaleBurrow;
                        break;
                    case DNS_CANBUY_RESULT_CAPACITY_FULL:
                        Message_ContinueTextbox(play, TEXT_OOT(0x10A6));
                        this->actionFunc = EnDns_SetupNoSaleBurrow;
                        break;
                    case DNS_CANBUY_RESULT_CANT_GET_NOW:
                        Message_ContinueTextbox(play, TEXT_OOT(0x10DE));
                        this->actionFunc = EnDns_SetupNoSaleBurrow;
                        break;
                    case DNS_CANBUY_RESULT_SUCCESS_NEW_ITEM:
                    case DNS_CANBUY_RESULT_SUCCESS:
                        Message_ContinueTextbox(play, TEXT_OOT(0x10A7));
                        this->actionFunc = EnDns_SetupSale;
                        break;
                }
                break;
            case 1: // No way
                Message_ContinueTextbox(play, TEXT_OOT(0x10A4));
                this->actionFunc = EnDns_SetupNoSaleBurrow;
        }
    }
}

static void EnDns_OfferSaleItem(EnDns* this, PlayState* play) {
    u16 pendingGetItemId;
    if (this->actor.params == DNS_TYPE_DEKU_STICK_UPGRADE) {
        if (CUR_UPG_VALUE(UPG_DEKU_STICKS) < 2) {
            pendingGetItemId = GI_STICK_UPGRADE_20_Oot;
        } else {
            pendingGetItemId = GI_STICK_UPGRADE_30_Oot;
        }
    } else if (this->actor.params == DNS_TYPE_DEKU_NUT_UPGRADE) {
        if (CUR_UPG_VALUE(UPG_DEKU_NUTS) < 2) {
            pendingGetItemId = GI_NUT_UPGRADE_30_Oot;
        } else {
            pendingGetItemId = GI_NUT_UPGRADE_40_Oot;
        }
    } else {
        pendingGetItemId = this->dnsItemEntry->getItemId;
    }
    Actor_OfferGetItem(&this->actor, play, pendingGetItemId, 130.0f, 100.0f);
}

static void EnDns_SetupSale(EnDns* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        if ((true)) {
            EnDns_OfferSaleItem(this, play);
        }
        this->actionFunc = EnDns_Sale;
    }
}

static void EnDns_Sale(EnDns* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play) || !(true)) {
        this->actor.parent = NULL;
        this->actionFunc = EnDns_SetupBurrow;
    } else {
        EnDns_OfferSaleItem(this, play);
    }
}

static void EnDns_SetupBurrow(EnDns* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (player->stateFlags1 & PLAYER_STATE1_400) {
        if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
            this->dnsItemEntry->setRupeesAndFlags(this);
            this->dropCollectible = 1;
            this->maintainCollider = 0;
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            EnDns_ChangeAnim(this, DNS_ANIM_BURROW);
            this->actionFunc = EnDns_Burrow;
        }
    } else {
        this->dnsItemEntry->setRupeesAndFlags(this);
        this->dropCollectible = 1;
        this->maintainCollider = 0;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        EnDns_ChangeAnim(this, DNS_ANIM_BURROW);
        this->actionFunc = EnDns_Burrow;
    }
}

static void EnDns_SetupNoSaleBurrow(EnDns* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        this->maintainCollider = 0;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        EnDns_ChangeAnim(this, DNS_ANIM_BURROW);
        this->actionFunc = EnDns_Burrow;
    }
}

static void EnDns_Burrow(EnDns* this, PlayState* play) {
    f32 frameCount = Animation_GetLastFrame(&gBusinessScrubLeaveBurrowAnim);

    if (this->skelAnime.curFrame == frameCount) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_AKINDONUTS_HIDE);
        this->actionFunc = EnDns_PostBurrow;
        this->standOnGround = 0;
        this->yInitPos = this->actor.world.pos.y;
    }
}

static void EnDns_PostBurrow(EnDns* this, PlayState* play) {
    f32 depth;
    Vec3f initPos;
    s32 i;

    depth = this->yInitPos - this->actor.world.pos.y;
    if ((this->dustTimer & 3) == 0) {
        initPos.x = this->actor.world.pos.x;
        initPos.y = this->yInitPos;
        initPos.z = this->actor.world.pos.z;
        func_800B14D4(play, 20.0f, &initPos);
    }
    this->actor.shape.rot.y += 0x2000;
    // Drops only if you bought its item
    if (depth > 400.0f) {
        if (this->dropCollectible) {
            initPos.x = this->actor.world.pos.x;
            initPos.y = this->yInitPos;
            initPos.z = this->actor.world.pos.z;
            for (i = 0; i < 3; i++) {
                Item_DropCollectible(play, &initPos, ITEM00_RECOVERY_HEART);
            }
        }
        Actor_Kill(&this->actor);
    }
}

static void EnDns_Update(Actor* thisx, PlayState* play) {
    EnDns* this = (EnDns*)thisx;
    s16 pad;

    this->dustTimer++;
    this->actor.textId = sStartingTextIds[this->actor.params];
    Actor_SetFocus(&this->actor, 60.0f);
    Actor_SetScale(&this->actor, 0.01f);
    SkelAnime_Update(&this->skelAnime);
    Actor_MoveWithGravity(&this->actor);
    this->actionFunc(this, play);
    if (this->standOnGround) {
        Actor_UpdateBgCheckInfo(play, &this->actor, 20.0f, 20.0f, 20.0f, 0x404);
    }
    if (this->maintainCollider) {
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void EnDns_Draw(Actor* thisx, PlayState* play) {
    EnDns* this = (EnDns*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, &this->actor);
}
