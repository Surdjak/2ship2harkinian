#include "z_en_ossan.h"

// PORT: OoT's OBJECT_ID_MAX, "no object" in sShopkeeperObjectIds.
#define OBJECT_ID_NONE_Oot -1
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleScenes.h"
#include "HyruleTermina.h"
#include "HyruleLines.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/object_ossan/object_ossan.h"
#include "assets/oot/objects/object_oF1d_map/object_oF1d_map.h"
#include "assets/oot/objects/object_os/object_os.h"
#include "assets/oot/objects/object_zo/object_zo.h"
#include "assets/oot/objects/object_rs/object_rs.h"
#include "assets/oot/objects/object_ds2/object_ds2.h"
#include "../ovl_En_Elf/z_en_elf.h"
#include "assets/oot/objects/object_masterkokiri/object_masterkokiri.h"
#include "assets/oot/objects/object_km1/object_km1.h"
#include "assets/oot/objects/object_mastergolon/object_mastergolon.h"
#include "assets/oot/objects/object_masterzoora/object_masterzoora.h"
#include "assets/oot/objects/object_masterkokirihead/object_masterkokirihead.h"
#include <assert.h>

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnOssan_Init(Actor* thisx, PlayState* play);
static void EnOssan_Destroy(Actor* thisx, PlayState* play);
static void EnOssan_Update(Actor* thisx, PlayState* play);
static void EnOssan_DrawKokiriShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawPotionShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawBombchuShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawBazaarShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawZoraShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawGoronShopkeeper(Actor* thisx, PlayState* play);
static void EnOssan_DrawHappyMaskShopkeeper(Actor* thisx, PlayState* play);

static void EnOssan_InitActionFunc(EnOssan* this, PlayState* play);
static void EnOssan_MainActionFunc(EnOssan* this, PlayState* play);

static void EnOssan_TalkDefaultShopkeeper(PlayState* play);
static void EnOssan_TalkKokiriShopkeeper(PlayState* play);
static void EnOssan_TalkKakarikoPotionShopkeeper(PlayState* play);
static void EnOssan_TalkBombchuShopkeeper(PlayState* play);
static void EnOssan_TalkMarketPotionShopkeeper(PlayState* play);
static void EnOssan_TalkBazaarShopkeeper(PlayState* play);
static void EnOssan_TalkZoraShopkeeper(PlayState* play);
static void EnOssan_TalkGoronShopkeeper(PlayState* play);
static void EnOssan_TalkHappyMaskShopkeeper(PlayState* play);

static s16 ShopItemDisp_Default(s16 v);
static s16 ShopItemDisp_SpookyMask(s16 v);
static s16 ShopItemDisp_SkullMask(s16 v);
static s16 ShopItemDisp_BunnyHood(s16 v);
static s16 ShopItemDisp_ZoraMask(s16 v);
static s16 ShopItemDisp_GoronMask(s16 v);
static s16 ShopItemDisp_GerudoMask(s16 v);

static void EnOssan_InitKokiriShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitPotionShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitBombchuShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitBazaarShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitZoraShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitGoronShopkeeper(EnOssan* this, PlayState* play);
static void EnOssan_InitHappyMaskShopkeeper(EnOssan* this, PlayState* play);

static void EnOssan_State_Idle(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_StartConversation(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_FacingShopkeeper(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_TalkingToShopkeeper(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_LookToLeftShelf(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_LookToRightShelf(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_BrowseLeftShelf(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_BrowseRightShelf(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_LookFromShelfToShopkeeper(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_ItemSelected(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_SelectMilkBottle(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_SelectWeirdEgg(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_SelectUnimplementedItem(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_SelectBombs(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_CantGetItem(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_GiveItemWithFanfare(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_ItemPurchased(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_ContinueShoppingPrompt(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_GiveLonLonMilk(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_DisplayOnlyBombDialog(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_WaitForDisplayOnlyBombDialog(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_21(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_22(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_QuickBuyDialog(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_SelectMaskItem(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_LendMaskOfTruth(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_GiveDiscountDialog(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_OfferHeroShield(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_GiveHeroShield(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_HeroShieldGiven(EnOssan* this, PlayState* play, Player* player);
static void EnOssan_State_LeaveForTermina(EnOssan* this, PlayState* play, Player* player);

static void EnOssan_Obj3ToSeg6(EnOssan* this, PlayState* play);

static void EnOssan_StartShopping(PlayState* play, EnOssan* this);

static void EnOssan_WaitForBlink(EnOssan* this);
static void EnOssan_Blink(EnOssan* this);

static u16 EnOssan_SetupHelloDialog(EnOssan* this);

static s32 EnOssan_TakeItemOffShelf(EnOssan* this);
static s32 EnOssan_ReturnItemToShelf(EnOssan* this);
static void EnOssan_ResetItemPosition(EnOssan* this);
static void EnOssan_SetStateGiveDiscountDialog(PlayState* play, EnOssan* this);

#define CURSOR_INVALID 0xFF

ActorProfile En_Ossan_Oot_Profile = {
    /**/ ACTOR_EN_OSSAN_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnOssan),
    /**/ EnOssan_Init,
    /**/ EnOssan_Destroy,
    /**/ EnOssan_Update,
    /**/ NULL,
};

// Unused collider
static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_NONE | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 30, 80, 0, { 0, 0, 0 } },
};

// Rupees to pay back to Happy Mask Shop
static s16 sMaskPaymentPrice[] = { 10, 30, 20, 50 };

// item yaw offsets
static s16 sItemShelfRot[] = { 0xEAAC, 0xEAAC, 0xEAAC, 0xEAAC, 0x1554, 0x1554, 0x1554, 0x1554 };

// unused values?
static s16 D_80AC8904[] = { 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025 };

static char* sShopkeeperPrintName[] = {
    "コキリの店  ", // "Kokiri Shop"
    "薬屋        ", // "Potion Shop"
    "夜の店      ", // "Night Shop"
    "路地裏の店  ", // "Back Alley Shop"
    "盾の店      ", // "Shield Shop"
    "大人の店    ", // "Adult Shop"
    "タロンの店  ", // "Talon Shop"
    "ゾーラの店  ", // "Zora Shop"
    "ゴロン夜の店", // "Goron Night Shop"
    "インゴーの店", // "Ingo Store"
    "お面屋      ", // "Mask Shop"
};

typedef struct {
    /* 0x00 */ s16 objId;
    /* 0x02 */ s16 unk_02;
    /* 0x04 */ s16 unk_04;
} ShopkeeperObjInfo;

static s16 sShopkeeperObjectIds[][3] = {
    { OBJECT_KM1_Oot, OBJECT_MASTERKOKIRIHEAD_Oot, OBJECT_MASTERKOKIRI_Oot },
    { OBJECT_DS2_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_RS_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_DS2_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_OSSAN_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_OSSAN_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_OSSAN_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_ZO_Oot, OBJECT_ID_NONE_Oot, OBJECT_MASTERZOORA_Oot },
    { OBJECT_OF1D_MAP_Oot, OBJECT_ID_NONE_Oot, OBJECT_MASTERGOLON_Oot },
    { OBJECT_OSSAN_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
    { OBJECT_OS_Oot, OBJECT_ID_NONE_Oot, OBJECT_ID_NONE_Oot },
};

static EnOssanTalkOwnerFunc sShopkeeperTalkOwner[] = {
    EnOssan_TalkKokiriShopkeeper,       EnOssan_TalkKakarikoPotionShopkeeper, EnOssan_TalkBombchuShopkeeper,
    EnOssan_TalkMarketPotionShopkeeper, EnOssan_TalkBazaarShopkeeper,         EnOssan_TalkDefaultShopkeeper,
    EnOssan_TalkDefaultShopkeeper,      EnOssan_TalkZoraShopkeeper,           EnOssan_TalkGoronShopkeeper,
    EnOssan_TalkDefaultShopkeeper,      EnOssan_TalkHappyMaskShopkeeper,
};

static f32 sShopkeeperScale[] = {
    0.01f, 0.011f, 0.0105f, 0.011f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f,
};

typedef struct {
    /* 0x00 */ s16 shopItemIndex;
    /* 0x02 */ s16 xOffset;
    /* 0x04 */ s16 yOffset;
    /* 0x06 */ s16 zOffset;
} ShopItem; // size 0x08

ShopItem sShopkeeperStores[][8] = {
    { { SI_DEKU_SHIELD, 50, 52, -20 },
      { SI_DEKU_NUTS_5, 50, 76, -20 },
      { SI_DEKU_NUTS_10, 80, 52, -3 },
      { SI_DEKU_STICK, 80, 76, -3 },
      { SI_DEKU_SEEDS_30, -50, 52, -20 },
      { SI_ARROWS_10, -50, 76, -20 },
      { SI_ARROWS_30, -80, 52, -3 },
      { SI_RECOVERY_HEART, -80, 76, -3 } },

    { { SI_GREEN_POTION, 50, 52, -20 },
      { SI_BLUE_FIRE, 50, 76, -20 },
      { SI_RED_POTION_R30, 80, 52, -3 },
      { SI_FAIRY, 80, 76, -3 },
      { SI_DEKU_NUTS_5, -50, 52, -20 },
      { SI_BUGS, -50, 76, -20 },
      { SI_POE, -80, 52, -3 },
      { SI_FISH, -80, 76, -3 } },

    { { SI_BOMBCHU_10_2, 50, 52, -20 },
      { SI_BOMBCHU_10_4, 50, 76, -20 },
      { SI_BOMBCHU_10_3, 80, 52, -3 },
      { SI_BOMBCHU_10_1, 80, 76, -3 },
      { SI_BOMBCHU_20_3, -50, 52, -20 },
      { SI_BOMBCHU_20_1, -50, 76, -20 },
      { SI_BOMBCHU_20_4, -80, 52, -3 },
      { SI_BOMBCHU_20_2, -80, 76, -3 } },

    { { SI_GREEN_POTION, 50, 52, -20 },
      { SI_BLUE_FIRE, 50, 76, -20 },
      { SI_RED_POTION_R30, 80, 52, -3 },
      { SI_FAIRY, 80, 76, -3 },
      { SI_DEKU_NUTS_5, -50, 52, -20 },
      { SI_BUGS, -50, 76, -20 },
      { SI_POE, -80, 52, -3 },
      { SI_FISH, -80, 76, -3 } },

    { { SI_HYLIAN_SHIELD, 50, 52, -20 },
      { SI_BOMBS_5_R35, 50, 76, -20 },
      { SI_DEKU_NUTS_5, 80, 52, -3 },
      { SI_RECOVERY_HEART, 80, 76, -3 },
      { SI_ARROWS_10, -50, 52, -20 },
      { SI_ARROWS_50, -50, 76, -20 },
      { SI_DEKU_STICK, -80, 52, -3 },
      { SI_ARROWS_30, -80, 76, -3 } },

    { { SI_HYLIAN_SHIELD, 50, 52, -20 },
      { SI_BOMBS_5_R25, 50, 76, -20 },
      { SI_DEKU_NUTS_5, 80, 52, -3 },
      { SI_RECOVERY_HEART, 80, 76, -3 },
      { SI_ARROWS_10, -50, 52, -20 },
      { SI_ARROWS_50, -50, 76, -20 },
      { SI_DEKU_STICK, -80, 52, -3 },
      { SI_ARROWS_30, -80, 76, -3 } },

    { { SI_MILK_BOTTLE, 50, 52, -20 },
      { SI_DEKU_NUTS_5, 50, 76, -20 },
      { SI_DEKU_NUTS_10, 80, 52, -3 },
      { SI_RECOVERY_HEART, 80, 76, -3 },
      { SI_WEIRD_EGG, -50, 52, -20 },
      { SI_DEKU_STICK, -50, 76, -20 },
      { SI_RECOVERY_HEART, -80, 52, -3 },
      { SI_RECOVERY_HEART, -80, 76, -3 } },

    { { SI_ZORA_TUNIC, 50, 52, -20 },
      { SI_ARROWS_10, 50, 76, -20 },
      { SI_RECOVERY_HEART, 80, 52, -3 },
      { SI_ARROWS_30, 80, 76, -3 },
      { SI_DEKU_NUTS_5, -50, 52, -20 },
      { SI_ARROWS_50, -50, 76, -20 },
      { SI_FISH, -80, 52, -3 },
      { SI_RED_POTION_R50, -80, 76, -3 } },

    { { SI_BOMBS_5_R25, 50, 52, -20 },
      { SI_BOMBS_10, 50, 76, -20 },
      { SI_BOMBS_20, 80, 52, -3 },
      { SI_BOMBS_30, 80, 76, -3 },
      { SI_GORON_TUNIC, -50, 52, -20 },
      { SI_RECOVERY_HEART, -50, 76, -20 },
      { SI_RED_POTION_R40, -80, 52, -3 },
      { SI_RECOVERY_HEART, -80, 76, -3 } },

    { { SI_19, 50, 52, -20 },
      { SI_19, 50, 76, -20 },
      { SI_19, 80, 52, -3 },
      { SI_19, 80, 76, -3 },
      { SI_20, -50, 52, -20 },
      { SI_20, -50, 76, -20 },
      { SI_20, -80, 52, -3 },
      { SI_20, -80, 76, -3 } },

    { { SI_GERUDO_MASK, 50, 52, -20 },
      { SI_ZORA_MASK, 50, 76, -20 },
      { SI_MASK_OF_TRUTH, 80, 52, -3 },
      { SI_GORON_MASK, 80, 76, -3 },
      { SI_SKULL_MASK, -50, 52, -20 },
      { SI_KEATON_MASK, -50, 76, -20 },
      { SI_BUNNY_HOOD, -80, 52, -3 },
      { SI_SPOOKY_MASK, -80, 76, -3 } },
};
static EnOssanGetGirlAParamsFunc sShopItemReplaceFunc[] = {
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_SpookyMask,
    ShopItemDisp_SkullMask, ShopItemDisp_BunnyHood,  ShopItemDisp_Default, ShopItemDisp_ZoraMask,
    ShopItemDisp_GoronMask, ShopItemDisp_GerudoMask, ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,    ShopItemDisp_Default, ShopItemDisp_Default,
    ShopItemDisp_Default,   ShopItemDisp_Default,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 2, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 500, ICHAIN_STOP),
};

// When selecting an item to buy, this is the position the item moves to
static Vec3f sSelectedItemPosition[] = { { 17.0f, 58.0f, 30.0f }, { -17.0f, 58.0f, 30.0f } };

static EnOssanInitFunc sInitFuncs[] = {
    EnOssan_InitKokiriShopkeeper, EnOssan_InitPotionShopkeeper,    EnOssan_InitBombchuShopkeeper,
    EnOssan_InitPotionShopkeeper, EnOssan_InitBazaarShopkeeper,    EnOssan_InitBazaarShopkeeper,
    EnOssan_InitBazaarShopkeeper, EnOssan_InitZoraShopkeeper,      EnOssan_InitGoronShopkeeper,
    EnOssan_InitBazaarShopkeeper, EnOssan_InitHappyMaskShopkeeper,
};

static Vec3f sShopkeeperPositionOffsets[] = {
    { 0.0f, 0.0f, 33.0f }, { 0.0f, 0.0f, 31.0f }, { 0.0f, 0.0f, 31.0f }, { 0.0f, 0.0f, 31.0f },
    { 0.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 36.0f },
    { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 26.0f },
};

static EnOssanStateFunc sStateFunc[] = {
    EnOssan_State_Idle,
    EnOssan_State_StartConversation,
    EnOssan_State_FacingShopkeeper,
    EnOssan_State_TalkingToShopkeeper,
    EnOssan_State_LookToLeftShelf,
    EnOssan_State_LookToRightShelf,
    EnOssan_State_BrowseLeftShelf,
    EnOssan_State_BrowseRightShelf,
    EnOssan_State_LookFromShelfToShopkeeper,
    EnOssan_State_ItemSelected,
    EnOssan_State_SelectMilkBottle,
    EnOssan_State_SelectWeirdEgg,
    EnOssan_State_SelectUnimplementedItem,
    EnOssan_State_SelectBombs,
    EnOssan_State_CantGetItem,
    EnOssan_State_GiveItemWithFanfare,
    EnOssan_State_ItemPurchased,
    EnOssan_State_ContinueShoppingPrompt,
    EnOssan_State_GiveLonLonMilk,
    EnOssan_State_DisplayOnlyBombDialog,
    EnOssan_State_WaitForDisplayOnlyBombDialog,
    EnOssan_State_21,
    EnOssan_State_22,
    EnOssan_State_QuickBuyDialog,
    EnOssan_State_SelectMaskItem,
    EnOssan_State_LendMaskOfTruth,
    EnOssan_State_GiveDiscountDialog,
    EnOssan_State_OfferHeroShield,
    EnOssan_State_GiveHeroShield,
    EnOssan_State_HeroShieldGiven,
    EnOssan_State_LeaveForTermina,
};

static void EnOssan_SetupAction(EnOssan* this, EnOssanActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static s16 ShopItemDisp_Default(s16 v) {
    return v;
}

static s16 ShopItemDisp_SpookyMask(s16 v) {
    // Sold Skull Mask
    if (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot)) {
        return v;
    }
    return -1;
}

static s16 ShopItemDisp_SkullMask(s16 v) {
    // Sold Keaton Mask
    if (GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) {
        return v;
    }
    return -1;
}

static s16 ShopItemDisp_BunnyHood(s16 v) {
    // Sold Spooky Mask
    if (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot)) {
        return v;
    }
    return -1;
}

static s16 ShopItemDisp_ZoraMask(s16 v) {
    // Obtained Mask of Truth
    if (GET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot)) {
        return v;
    }
    return -1;
}

static s16 ShopItemDisp_GoronMask(s16 v) {
    // Obtained Mask of Truth
    if (GET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot)) {
        return v;
    }
    return -1;
}

static s16 ShopItemDisp_GerudoMask(s16 v) {
    // Obtained Mask of Truth
    if (GET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot)) {
        return v;
    }
    return -1;
}

static void EnOssan_SpawnItemsOnShelves(EnOssan* this, PlayState* play, ShopItem* shopItems) {
    EnTana* shelves;
    s16 itemParams;
    s32 i;

    for (i = 0; i < 8; i++, shopItems++) {
        if (shopItems->shopItemIndex < 0) {
            this->shelfSlots[i] = NULL;
        } else {
            itemParams = sShopItemReplaceFunc[shopItems->shopItemIndex](shopItems->shopItemIndex);

            if (itemParams < 0) {
                this->shelfSlots[i] = NULL;
            } else {
                shelves = this->shelves;
                this->shelfSlots[i] = (EnGirlA*)Actor_Spawn(
                    &play->actorCtx, play, ACTOR_EN_GIRLA_Oot, shelves->actor.world.pos.x + shopItems->xOffset,
                    shelves->actor.world.pos.y + shopItems->yOffset, shelves->actor.world.pos.z + shopItems->zOffset,
                    shelves->actor.shape.rot.x, shelves->actor.shape.rot.y + sItemShelfRot[i],
                    shelves->actor.shape.rot.z, itemParams);
            }
        }
    }
}

static void EnOssan_UpdateShopOfferings(EnOssan* this, PlayState* play) {
    s32 i;
    ShopItem* storeItems;
    ShopItem* shopItem;

    if (this->actor.params == OSSAN_TYPE_MASK) {
        storeItems = sShopkeeperStores[this->actor.params];
        for (i = 0; i < 8; i++) {
            shopItem = &storeItems[i];
            if (shopItem->shopItemIndex >= 0 && this->shelfSlots[i] == NULL) {
                s16 params = sShopItemReplaceFunc[shopItem->shopItemIndex](shopItem->shopItemIndex);

                if (params >= 0) {
                    this->shelfSlots[i] = (EnGirlA*)Actor_Spawn(
                        &play->actorCtx, play, ACTOR_EN_GIRLA_Oot, this->shelves->actor.world.pos.x + shopItem->xOffset,
                        this->shelves->actor.world.pos.y + shopItem->yOffset,
                        this->shelves->actor.world.pos.z + shopItem->zOffset, this->shelves->actor.shape.rot.x,
                        this->shelves->actor.shape.rot.y + sItemShelfRot[i], this->shelves->actor.shape.rot.z, params);
                }
            }
        }
    }
}

static void EnOssan_TalkDefaultShopkeeper(PlayState* play) {
    Message_ContinueTextbox(play, TEXT_OOT(0x9E));
}

static void EnOssan_TalkKakarikoPotionShopkeeper(PlayState* play) {
    if (play->curSpawn == 0) {
        Message_ContinueTextbox(play, TEXT_OOT(0x5046));
    } else {
        Message_ContinueTextbox(play, TEXT_OOT(0x504E));
    }
}

static void EnOssan_TalkMarketPotionShopkeeper(PlayState* play) {
    Message_ContinueTextbox(play, TEXT_OOT(0x504E));
}

static void EnOssan_TalkKokiriShopkeeper(PlayState* play) {
    Message_ContinueTextbox(play, TEXT_OOT(0x10BA));
}

static void EnOssan_TalkBazaarShopkeeper(PlayState* play) {
    if (play->curSpawn == 0) {
        Message_ContinueTextbox(play, TEXT_OOT(0x9D));
    } else {
        Message_ContinueTextbox(play, TEXT_OOT(0x9C));
    }
}

static void EnOssan_TalkBombchuShopkeeper(PlayState* play) {
    Message_ContinueTextbox(play, TEXT_OOT(0x7076));
}

static void EnOssan_TalkZoraShopkeeper(PlayState* play) {
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        Message_ContinueTextbox(play, TEXT_OOT(0x403A));
    } else {
        Message_ContinueTextbox(play, TEXT_OOT(0x403B));
    }
}

// Goron City, Goron
static void EnOssan_TalkGoronShopkeeper(PlayState* play) {
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
            Message_ContinueTextbox(play, TEXT_OOT(0x3028));
        } else if (CUR_UPG_VALUE(UPG_STRENGTH) != 0) {
            Message_ContinueTextbox(play, TEXT_OOT(0x302D));
        } else {
            Message_ContinueTextbox(play, TEXT_OOT(0x300F));
        }
    } else if (!CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_FIRE_Oot)) {
        Message_ContinueTextbox(play, TEXT_OOT(0x3057));
    } else {
        Message_ContinueTextbox(play, TEXT_OOT(0x305B));
    }
}

// Happy Mask Shop
static void EnOssan_TalkHappyMaskShopkeeper(PlayState* play) {
    if ((GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot))       // Sold Keaton Mask
        && (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot))    // Sold Skull Mask
        && (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot))    // Sold Spooky Mask
        && (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot))) { // Sold Bunny Hood
        Message_ContinueTextbox(play, TEXT_OOT(0x70AE));
    } else {
        switch (play->msgCtx.choiceIndex) {
            case 1:
                Message_ContinueTextbox(play, TEXT_OOT(0x70A4));
                break;
            case 0:
                Message_ContinueTextbox(play, TEXT_OOT(0x70A3));
                break;
        }
    }
}

static void EnOssan_UpdateCameraDirection(EnOssan* this, PlayState* play, f32 cameraFaceAngle) {
    this->cameraFaceAngle = cameraFaceAngle;
    Camera_SetCameraData(GET_ACTIVE_CAM(play), 0xC, NULL, NULL, cameraFaceAngle, 0);
}

static s32 EnOssan_TryGetObjBankIndexes(EnOssan* this, PlayState* play, s16* objectIds) {
    if (objectIds[1] != OBJECT_ID_NONE_Oot) {
        this->objBankIndex2 = Object_GetSlot(&play->objectCtx, objectIds[1]);
        if (this->objBankIndex2 < 0) {
            return false;
        }
    } else {
        this->objBankIndex2 = -1;
    }
    if (objectIds[2] != OBJECT_ID_NONE_Oot) {
        this->objBankIndex3 = Object_GetSlot(&play->objectCtx, objectIds[2]);
        if (this->objBankIndex3 < 0) {
            return false;
        }
    } else {
        this->objBankIndex3 = -1;
    }
    return true;
}

static void EnOssan_Init(Actor* thisx, PlayState* play) {
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;
    s16* objectIds;

    if (this->actor.params == OSSAN_TYPE_TALON && (LINK_AGE_IN_YEARS_OOT != YEARS_CHILD)) {
        this->actor.params = OSSAN_TYPE_INGO;
    }

    //! @bug This check will always evaluate to false, it should be || not &&
    if (this->actor.params > OSSAN_TYPE_MASK && this->actor.params < OSSAN_TYPE_KOKIRI) {
        Actor_Kill(&this->actor);
        assert(this->actor.params > OSSAN_TYPE_MASK && this->actor.params < OSSAN_TYPE_KOKIRI);
        return;
    }

    // If you haven't given Zelda's Letter to the Kakariko Guard
    if (this->actor.params == OSSAN_TYPE_MASK && !GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) {
        Actor_Kill(&this->actor);
        return;
    }

    // 3Ship [Hyrule] Once he has left for Termina, his shop stands empty for good (docs/TERMINA.md).
    if ((this->actor.params == OSSAN_TYPE_MASK) && Hyrule_IsTerminaUnlocked()) {
        Actor_Kill(&this->actor);
        return;
    }

    if (this->actor.params == OSSAN_TYPE_KAKARIKO_POTION && (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD)) {
        Actor_Kill(&this->actor);
        return;
    }

    // Don't kill bombchu shop actor in rando, making it so the shop is immediately open
    // GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot) - Completed Dodongo's Cavern
    if (this->actor.params == OSSAN_TYPE_BOMBCHUS &&
        !GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
        Actor_Kill(&this->actor);
        return;
    }

    objectIds = sShopkeeperObjectIds[this->actor.params];
    this->objBankIndex1 = Object_GetSlot(&play->objectCtx, objectIds[0]);

    if (this->objBankIndex1 < 0) {
        Actor_Kill(&this->actor);
        assert(this->objBankIndex1 < 0);
        return;
    }

    if (EnOssan_TryGetObjBankIndexes(this, play, objectIds) == 0) {
        Actor_Kill(&this->actor);
        assert(EnOssan_TryGetObjBankIndexes(this, play, objectIds) == 0);
        return;
    }

    Actor_ProcessInitChain(&this->actor, sInitChain);
    EnOssan_SetupAction(this, EnOssan_InitActionFunc);
}

static void EnOssan_Destroy(Actor* thisx, PlayState* play) {
    EnOssan* this = (EnOssan*)thisx;
    SkelAnime_Free(&this->skelAnime, play);
    Collider_DestroyCylinder(play, &this->collider);
}

static void EnOssan_UpdateCursorPos(PlayState* play, EnOssan* this) {
    s16 x;
    s16 y;

    Actor_GetScreenPos(play, &this->shelfSlots[this->cursorIndex]->actor, &x, &y);
    this->cursorX = x;
    this->cursorY = y;

    if (0) {
        this->cursorX = SCREEN_WIDTH - x;
    }
}

static void EnOssan_EndInteraction(PlayState* play, EnOssan* this) {
    Player* player = GET_PLAYER(play);

    YREG(31) = 0;
    Actor_TalkOfferAccepted(&this->actor, &play->state);
    play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
    play->msgCtx.stateTimer = 4;
    player->stateFlags2 &= ~PLAYER_STATE2_20000000;
    Hyrule_SetViewpoint(play, 1);
    Interface_SetHudVisibility(50);
    this->drawCursor = 0;
    this->stickLeftPrompt.isEnabled = false;
    this->stickRightPrompt.isEnabled = false;
    EnOssan_UpdateCameraDirection(this, play, 0.0f);
    this->actor.textId = EnOssan_SetupHelloDialog(this);
    this->stateFlag = OSSAN_STATE_IDLE;
}

static s32 EnOssan_TestEndInteraction(EnOssan* this, PlayState* play, Input* input) {
    if ((CHECK_BTN_ALL(input->press.button, BTN_B))) {
        EnOssan_EndInteraction(play, this);
        return true;
    } else {
        return false;
    }
}

static s32 EnOssan_TestCancelOption(EnOssan* this, PlayState* play, Input* input) {
    if ((CHECK_BTN_ALL(input->press.button, BTN_B))) {
        this->stateFlag = this->tempStateFlag;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
        return true;
    } else {
        return false;
    }
}

static void EnOssan_SetStateStartShopping(PlayState* play, EnOssan* this, u8 skipHelloState) {
    YREG(31) = 1;
    this->headRot = this->headTargetRot = 0;
    Interface_SetAButtonDoAction(play, DO_ACTION_NEXT);
    EnOssan_UpdateCameraDirection(this, play, 0);

    if (!skipHelloState) {
        this->stateFlag = OSSAN_STATE_START_CONVERSATION;
    } else {
        EnOssan_StartShopping(play, this);
    }
}

static void EnOssan_StartShopping(PlayState* play, EnOssan* this) {
    this->stateFlag = OSSAN_STATE_FACING_SHOPKEEPER;

    if (this->actor.params == OSSAN_TYPE_MASK) {
        // if all masks have been sold, give the option to ask about the mask of truth
        if ((GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) && (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot)) &&
            (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot)) && (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot))) {
            Message_ContinueTextbox(play, TEXT_OOT(0x70AD));
        } else {
            Message_ContinueTextbox(play, TEXT_OOT(0x70A2));
        }
    } else {
        Message_ContinueTextbox(play, TEXT_OOT(0x83));
    }

    Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
    this->stickRightPrompt.isEnabled = true;
    this->stickLeftPrompt.isEnabled = true;
    EnOssan_UpdateCameraDirection(this, play, 0.0f);
}

static void EnOssan_ChooseTalkToOwner(PlayState* play, EnOssan* this) {
    this->stateFlag = OSSAN_STATE_TALKING_TO_SHOPKEEPER;
    sShopkeeperTalkOwner[this->actor.params](play);
    Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
    this->stickLeftPrompt.isEnabled = false;
    this->stickRightPrompt.isEnabled = false;
}

static void EnOssan_SetLookToShopkeeperFromShelf(PlayState* play, EnOssan* this) {
    Audio_PlaySfx(NA_SE_SY_CURSOR);
    this->drawCursor = 0;
    this->stateFlag = OSSAN_STATE_LOOK_SHOPKEEPER;
}

static void EnOssan_State_Idle(EnOssan* this, PlayState* play, Player* player) {
    this->headTargetRot = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        player->stateFlags2 |= PLAYER_STATE2_20000000;
        Hyrule_SetShopBrowsingViewpoint(play);
        EnOssan_SetStateStartShopping(play, this, false);
    } else if (this->actor.xzDistToPlayer < 100.0f) {
        Actor_OfferTalk(&this->actor, play, 100);
    }
}

static void EnOssan_UpdateJoystickInputState(PlayState* play, EnOssan* this) {
    Input* input = &play->state.input[0];
    s8 stickX = input->rel.stick_x;
    s8 stickY = input->rel.stick_y;

    if (0) {
        stickX = -input->rel.stick_x;
    }

    this->moveHorizontal = this->moveVertical = false;

    if (this->stickAccumX == 0) {
        if (stickX > 30 || stickX < -30) {
            this->stickAccumX = stickX;
            this->moveHorizontal = true;
        }
    } else if (stickX <= 30 && stickX >= -30) {
        this->stickAccumX = 0;
    } else if (this->stickAccumX * stickX < 0) { // Stick has swapped directions
        this->stickAccumX = stickX;
        this->moveHorizontal = true;
    } else {
        this->stickAccumX += stickX;

        if (this->stickAccumX > 2000) {
            this->stickAccumX = 2000;
        } else if (this->stickAccumX < -2000) {
            this->stickAccumX = -2000;
        }
    }

    if (this->stickAccumY == 0) {
        if (stickY > 30 || stickY < -30) {
            this->stickAccumY = stickY;
            this->moveVertical = true;
        }
    } else if (stickY <= 30 && stickY >= -30) {
        this->stickAccumY = 0;
    } else if (this->stickAccumY * stickY < 0) { // Stick has swapped directions
        this->stickAccumY = stickY;
        this->moveVertical = true;
    } else {
        this->stickAccumY += stickY;

        if (this->stickAccumY > 2000) {
            this->stickAccumY = 2000;
        } else if (this->stickAccumY < -2000) {
            this->stickAccumY = -2000;
        }
    }
}

static u8 EnOssan_SetCursorIndexFromNeutral(EnOssan* this, u8 shelfOffset) {
    u8 i;

    // if cursor is on the top shelf
    if (this->cursorIndex & 1) {
        // scan top shelf for non-null item
        for (i = shelfOffset + 1; i < shelfOffset + 4; i += 2) {
            if (this->shelfSlots[i] != NULL) {
                return i;
            }
        }
        // scan bottom shelf for non-null item
        for (i = shelfOffset; i < shelfOffset + 4; i += 2) {
            if (this->shelfSlots[i] != NULL) {
                return i;
            }
        }
    } else {
        // scan bottom shelf for non-null item
        for (i = shelfOffset; i < shelfOffset + 4; i += 2) {
            if (this->shelfSlots[i] != NULL) {
                return i;
            }
        }
        // scan top shelf for non-null item
        for (i = shelfOffset + 1; i < shelfOffset + 4; i += 2) {
            if (this->shelfSlots[i] != NULL) {
                return i;
            }
        }
    }
    return CURSOR_INVALID;
}

static u8 EnOssan_CursorRight(EnOssan* this, u8 cursorIndex, u8 shelfSlotMin) {
    u8 c = shelfSlotMin + 4;

    while (cursorIndex >= shelfSlotMin && cursorIndex < c) {
        cursorIndex -= 2;
        if (cursorIndex >= shelfSlotMin && cursorIndex < c) {
            if (this->shelfSlots[cursorIndex] != NULL) {
                return cursorIndex;
            }
        }
    }
    return CURSOR_INVALID;
}

static u8 EnOssan_CursorLeft(EnOssan* this, u8 cursorIndex, u8 shelfSlotMax) {

    while (cursorIndex < shelfSlotMax) {
        cursorIndex += 2;
        if ((cursorIndex < shelfSlotMax) && this->shelfSlots[cursorIndex] != NULL) {
            return cursorIndex;
        }
    }
    return CURSOR_INVALID;
}

// pay salesman back
static void EnOssan_TryPaybackMask(EnOssan* this, PlayState* play) {
    s16 price = sMaskPaymentPrice[this->happyMaskShopState];

    if (gSaveContext.save.saveInfo.playerData.rupees < price) {
        Message_ContinueTextbox(play, TEXT_OOT(0x70A8));
        this->happyMaskShopkeeperEyeIdx = 1;
        this->happyMaskShopState = OSSAN_HAPPY_STATE_ANGRY;
    } else {
        Rupees_ChangeBy(-price);

        if (this->happyMaskShopState == OSSAN_HAPPY_STATE_REQUEST_PAYMENT_BUNNY_HOOD) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_BUNNY_HOOD_FEE_Oot);
            Message_ContinueTextbox(play, TEXT_OOT(0x70A9));
            this->happyMaskShopState = OSSAN_HAPPY_STATE_ALL_MASKS_SOLD;
            return;
        }

        if (this->happyMaskShopState == OSSAN_HAPPY_STATE_REQUEST_PAYMENT_KEATON_MASK) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_KEATON_MASK_FEE_Oot);
        } else if (this->happyMaskShopState == OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SPOOKY_MASK) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_SPOOKY_MASK_FEE_Oot);
        } else if (this->happyMaskShopState == OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SKULL_MASK) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_SKULL_MASK_FEE_Oot);
        }

        Message_ContinueTextbox(play, TEXT_OOT(0x70A7));
        this->happyMaskShopState = OSSAN_HAPPY_STATE_NONE;
    }
    this->stateFlag = OSSAN_STATE_START_CONVERSATION;
}

static void EnOssan_State_StartConversation(EnOssan* this, PlayState* play, Player* player) {
    u8 dialogState = Message_GetState(&play->msgCtx);

    if (this->actor.params == OSSAN_TYPE_MASK && dialogState == TEXT_STATE_CHOICE) {
        if (!EnOssan_TestEndInteraction(this, play, &play->state.input[0]) && Message_ShouldAdvance(play)) {
            switch (play->msgCtx.choiceIndex) {
                case 0:
                    EnOssan_StartShopping(play, this);
                    break;
                case 1:
                    EnOssan_EndInteraction(play, this);
                    break;
            }
        }
    } else if (dialogState == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        Audio_PlaySfx(NA_SE_SY_MESSAGE_PASS);

        switch (this->happyMaskShopState) {
            case OSSAN_HAPPY_STATE_ALL_MASKS_SOLD:
                Message_ContinueTextbox(play, TEXT_OOT(0x70AA));
                this->stateFlag = OSSAN_STATE_LEND_MASK_OF_TRUTH;
                return;
            case OSSAN_HAPPY_STATE_BORROWED_FIRST_MASK:
                EnOssan_EndInteraction(play, this);
                return;
            case OSSAN_HAPPY_STATE_OFFER_HERO_SHIELD_3SHIP:
                // 3Ship [Hyrule] Link owns the Termina gate's items: the Hero's Shield (docs/TERMINA.md).
                Hyrule_ContinueTextbox(play, HYRULE_LINE_HERO_SHIELD_OFFER);
                this->stateFlag = OSSAN_STATE_OFFER_HERO_SHIELD_3SHIP;
                return;
            case OSSAN_HAPPY_STATE_REQUEST_PAYMENT_KEATON_MASK:
            case OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SPOOKY_MASK:
            case OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SKULL_MASK:
            case OSSAN_HAPPY_STATE_REQUEST_PAYMENT_BUNNY_HOOD:
                EnOssan_TryPaybackMask(this, play);
                return;
            case OSSAN_HAPPY_STATE_ANGRY:
                // In ER, handle happy mask throwing link out with not enough rupees
                {
                    play->nextEntrance = Hyrule_ExitFromOot(0x1D1); // OoT's ENTR_MARKET_DAY_OUTSIDE_HAPPY_MASK_SHOP
                }
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
                return;
        }

        if (!EnOssan_TestEndInteraction(this, play, &play->state.input[0])) {
            EnOssan_StartShopping(play, this);
        }
    }
}

static s32 EnOssan_FacingShopkeeperDialogResult(EnOssan* this, PlayState* play) {
    switch (play->msgCtx.choiceIndex) {
        case 0:
            EnOssan_ChooseTalkToOwner(play, this);
            return true;
        case 1:
            EnOssan_EndInteraction(play, this);
            return true;
        default:
            return false;
    }
}

static void EnOssan_State_FacingShopkeeper(EnOssan* this, PlayState* play, Player* player) {
    Input* input = &play->state.input[0];
    u8 nextIndex;
    bool dpad = 0;

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) &&
        !EnOssan_TestEndInteraction(this, play, &play->state.input[0])) {
        if (Message_ShouldAdvance(play) && EnOssan_FacingShopkeeperDialogResult(this, play)) {
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            return;
        }

        u16 dLeft = BTN_DLEFT;
        u16 dRight = BTN_DRIGHT;
        if (0) {
            dLeft = BTN_DRIGHT;
            dRight = BTN_DLEFT;
        }

        // Stick Left
        if ((this->stickAccumX < 0) || (dpad && CHECK_BTN_ALL(input->press.button, dLeft))) {
            nextIndex = EnOssan_SetCursorIndexFromNeutral(this, 4);
            if (nextIndex != CURSOR_INVALID) {
                this->cursorIndex = nextIndex;
                this->stateFlag = OSSAN_STATE_LOOK_SHELF_LEFT;
                Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                this->stickLeftPrompt.isEnabled = false;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        } else if ((this->stickAccumX > 0) || (dpad && CHECK_BTN_ALL(input->press.button, dRight))) {
            nextIndex = EnOssan_SetCursorIndexFromNeutral(this, 0);
            if (nextIndex != CURSOR_INVALID) {
                this->cursorIndex = nextIndex;
                this->stateFlag = OSSAN_STATE_LOOK_SHELF_RIGHT;
                Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                this->stickRightPrompt.isEnabled = false;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        }
    }
}

static void EnOssan_State_TalkingToShopkeeper(EnOssan* this, PlayState* play, Player* player) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        EnOssan_StartShopping(play, this);
    }
}

static void EnOssan_State_LookToLeftShelf(EnOssan* this, PlayState* play, Player* player) {
    Math_ApproachF(&this->cameraFaceAngle, 30.0f, 0.5f, 10.0f);

    if (this->cameraFaceAngle > 29.5f) {
        EnOssan_UpdateCameraDirection(this, play, 30.0f);
    }

    EnOssan_UpdateCameraDirection(this, play, this->cameraFaceAngle);

    if (this->cameraFaceAngle >= 30.0f) {
        EnOssan_UpdateCameraDirection(this, play, 30.0f);
        EnOssan_UpdateCursorPos(play, this);
        this->stateFlag = OSSAN_STATE_BROWSE_LEFT_SHELF;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
    } else {
        this->stickAccumX = 0;
    }
}

static void EnOssan_State_LookToRightShelf(EnOssan* this, PlayState* play, Player* player) {
    Math_ApproachF(&this->cameraFaceAngle, -30.0f, 0.5f, 10.0f);

    if (this->cameraFaceAngle < -29.5f) {
        EnOssan_UpdateCameraDirection(this, play, -30.0f);
    }

    EnOssan_UpdateCameraDirection(this, play, this->cameraFaceAngle);

    if (this->cameraFaceAngle <= -30.0f) {
        EnOssan_UpdateCameraDirection(this, play, -30.0f);
        EnOssan_UpdateCursorPos(play, this);
        this->stateFlag = OSSAN_STATE_BROWSE_RIGHT_SHELF;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
    } else {
        this->stickAccumX = 0;
    }
}

static void EnOssan_CursorUpDown(EnOssan* this, PlayState* play) {
    Input* input = &play->state.input[0];
    u8 curTemp = this->cursorIndex;
    u8 curScanTemp;
    bool dpad = 0;

    if ((this->stickAccumY < 0) || (dpad && CHECK_BTN_ALL(input->press.button, BTN_DDOWN))) {
        curTemp &= 0xFE;
        if (this->shelfSlots[curTemp] != NULL) {
            this->cursorIndex = curTemp;
            return;
        }
        // cursorIndex on right shelf
        if (curTemp < 4) {
            curScanTemp = curTemp + 2;
            if (curScanTemp >= 4) {
                curScanTemp = 0;
            }
            while (curScanTemp != curTemp) {
                if (this->shelfSlots[curScanTemp] != NULL) {
                    this->cursorIndex = curScanTemp;
                    return;
                }
                curScanTemp += 2;
                if (curScanTemp >= 4) {
                    curScanTemp = 0;
                }
            }
        } else {
            // cursorIndex on left shelf
            curScanTemp = curTemp + 2;
            if (curScanTemp >= 8) {
                curScanTemp = 4;
            }
            while (curScanTemp != curTemp) {
                if (this->shelfSlots[curScanTemp] != NULL) {
                    this->cursorIndex = curScanTemp;
                    return;
                }
                curScanTemp += 2;
                if (curScanTemp >= 8) {
                    curScanTemp = 4;
                }
            }
        }
    } else if ((this->stickAccumY > 0) || (dpad && CHECK_BTN_ALL(input->press.button, BTN_DUP))) {
        curTemp |= 1;
        if (this->shelfSlots[curTemp] != NULL) {
            this->cursorIndex = curTemp;
            return;
        }
        // cursorIndex on right shelf
        if (curTemp < 4) {
            curScanTemp = curTemp + 2;
            if (curScanTemp >= 4) {
                curScanTemp = 1;
            }
            while (curScanTemp != curTemp) {
                if (this->shelfSlots[curScanTemp] != NULL) {
                    this->cursorIndex = curScanTemp;
                    return;
                }
                curScanTemp += 2;
                if (curScanTemp >= 4) {
                    curScanTemp = 1;
                }
            }
        } else {
            // cursorIndex on left shelf
            curScanTemp = curTemp + 2;
            if (curScanTemp >= 8) {
                curScanTemp = 5;
            }
            while (curScanTemp != curTemp) {
                if (this->shelfSlots[curScanTemp] != NULL) {
                    this->cursorIndex = curScanTemp;
                    return;
                }
                curScanTemp += 2;
                if (curScanTemp >= 8) {
                    curScanTemp = 5;
                }
            }
        }
    }
}

static s32 EnOssan_HasPlayerSelectedItem(PlayState* play, EnOssan* this, Input* input) {
    EnGirlA* selectedItem = this->shelfSlots[this->cursorIndex];

    if (EnOssan_TestEndInteraction(this, play, input)) {
        return true;
    }
    if (Message_ShouldAdvance(play)) {
        if (selectedItem->actor.params != SI_SOLD_OUT && selectedItem->isInvisible == 0) {
            this->tempStateFlag = this->stateFlag;
            Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->itemBuyPromptTextId);
            this->stickLeftPrompt.isEnabled = false;
            this->stickRightPrompt.isEnabled = false;
            switch (selectedItem->actor.params) {
                case SI_KEATON_MASK:
                case SI_SPOOKY_MASK:
                case SI_SKULL_MASK:
                case SI_BUNNY_HOOD:
                case SI_MASK_OF_TRUTH:
                case SI_ZORA_MASK:
                case SI_GORON_MASK:
                case SI_GERUDO_MASK:
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM_MASK;
                    return true;
                case SI_MILK_BOTTLE:
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM_MILK_BOTTLE;
                    return true;
                case SI_WEIRD_EGG:
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM_WEIRD_EGG;
                    return true;
                case SI_19:
                case SI_20:
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM_UNIMPLEMENTED;
                    return true;
                case SI_BOMBS_5_R25:
                case SI_BOMBS_10:
                case SI_BOMBS_20:
                case SI_BOMBS_30:
                case SI_BOMBS_5_R35:
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM_BOMBS;
                    return true;
                default:
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                    this->drawCursor = 0;
                    this->stateFlag = OSSAN_STATE_SELECT_ITEM;
                    return true;
            }
        }
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return true;
    }
    return false;
}

static void EnOssan_State_BrowseLeftShelf(EnOssan* this, PlayState* play, Player* player) {
    Input* input = &play->state.input[0];
    s32 a;
    s32 b;
    u8 prevIndex = this->cursorIndex;
    s32 c;
    s32 d;
    bool dpad = 0;

    if (!EnOssan_ReturnItemToShelf(this)) {
        this->delayTimer = 3;
        return;
    }
    if (this->delayTimer != 0) {
        this->delayTimer--;
        return;
    }
    this->drawCursor = 0xFF;
    this->stickRightPrompt.isEnabled = true;
    EnOssan_UpdateCursorPos(play, this);
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) &&
        !EnOssan_HasPlayerSelectedItem(play, this, &play->state.input[0])) {

        u16 dLeft = BTN_DLEFT;
        u16 dRight = BTN_DRIGHT;
        if (0) {
            dLeft = BTN_DRIGHT;
            dRight = BTN_DLEFT;
        }

        if (this->moveHorizontal) {
            if ((this->stickAccumX > 0) || (dpad && CHECK_BTN_ALL(input->press.button, dRight))) {
                a = EnOssan_CursorRight(this, this->cursorIndex, 4);
                if (a != CURSOR_INVALID) {
                    this->cursorIndex = a;
                } else {
                    EnOssan_SetLookToShopkeeperFromShelf(play, this);
                    return;
                }
            } else if ((this->stickAccumX < 0) || (dpad && CHECK_BTN_ALL(input->press.button, dLeft))) {
                b = EnOssan_CursorLeft(this, this->cursorIndex, 8);
                if (b != CURSOR_INVALID) {
                    this->cursorIndex = b;
                }
            }
        } else {
            if ((this->stickAccumX > 0 && this->stickAccumX > 500) ||
                (dpad && CHECK_BTN_ALL(input->press.button, dRight))) {
                c = EnOssan_CursorRight(this, this->cursorIndex, 4);
                if (c != CURSOR_INVALID) {
                    this->cursorIndex = c;
                } else {
                    EnOssan_SetLookToShopkeeperFromShelf(play, this);
                    return;
                }
            } else if ((this->stickAccumX < 0 && this->stickAccumX < -500) ||
                       (dpad && CHECK_BTN_ALL(input->press.button, dLeft))) {
                d = EnOssan_CursorLeft(this, this->cursorIndex, 8);
                if (d != CURSOR_INVALID) {
                    this->cursorIndex = d;
                }
            }
        }
        EnOssan_CursorUpDown(this, play);
        if (this->cursorIndex != prevIndex) {
            Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
            Audio_PlaySfx(NA_SE_SY_CURSOR);
        }
    }
}

static void EnOssan_State_BrowseRightShelf(EnOssan* this, PlayState* play, Player* player) {
    Input* input = &play->state.input[0];
    s32 pad[2];
    u8 prevIndex;
    u8 nextIndex;
    bool dpad = 0;

    prevIndex = this->cursorIndex;
    if (!EnOssan_ReturnItemToShelf(this)) {
        this->delayTimer = 3;
        return;
    }
    if (this->delayTimer != 0) {
        this->delayTimer--;
        return;
    }
    this->drawCursor = 0xFF;
    this->stickLeftPrompt.isEnabled = true;
    EnOssan_UpdateCursorPos(play, this);
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) &&
        !EnOssan_HasPlayerSelectedItem(play, this, &play->state.input[0])) {

        u16 dLeft = BTN_DLEFT;
        u16 dRight = BTN_DRIGHT;
        if (0) {
            dLeft = BTN_DRIGHT;
            dRight = BTN_DLEFT;
        }

        if (this->moveHorizontal) {
            if ((this->stickAccumX < 0) || (dpad && CHECK_BTN_ALL(input->press.button, dLeft))) {
                nextIndex = EnOssan_CursorRight(this, this->cursorIndex, 0);
                if (nextIndex != CURSOR_INVALID) {
                    this->cursorIndex = nextIndex;
                } else {
                    EnOssan_SetLookToShopkeeperFromShelf(play, this);
                    return;
                }
            } else if ((this->stickAccumX > 0) || (dpad && CHECK_BTN_ALL(input->press.button, dRight))) {
                nextIndex = EnOssan_CursorLeft(this, this->cursorIndex, 4);
                if (nextIndex != CURSOR_INVALID) {
                    this->cursorIndex = nextIndex;
                }
            }
        } else {
            if ((this->stickAccumX < 0 && this->stickAccumX < -500) ||
                (dpad && CHECK_BTN_ALL(input->press.button, dLeft))) {
                nextIndex = EnOssan_CursorRight(this, this->cursorIndex, 0);
                if (nextIndex != CURSOR_INVALID) {
                    this->cursorIndex = nextIndex;
                } else {
                    EnOssan_SetLookToShopkeeperFromShelf(play, this);
                    return;
                }
            } else if ((this->stickAccumX > 0 && this->stickAccumX > 500) ||
                       (dpad && CHECK_BTN_ALL(input->press.button, dRight))) {
                nextIndex = EnOssan_CursorLeft(this, this->cursorIndex, 4);
                if (nextIndex != CURSOR_INVALID) {
                    this->cursorIndex = nextIndex;
                }
            }
        }
        EnOssan_CursorUpDown(this, play);
        if (this->cursorIndex != prevIndex) {
            Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
            Audio_PlaySfx(NA_SE_SY_CURSOR);
        }
    }
}

static void EnOssan_State_LookFromShelfToShopkeeper(EnOssan* this, PlayState* play, Player* player) {
    Math_ApproachF(&this->cameraFaceAngle, 0.0f, 0.5f, 10.0f);
    if ((this->cameraFaceAngle < 0.5f) && (this->cameraFaceAngle > -0.5f)) {
        EnOssan_UpdateCameraDirection(this, play, 0.0f);
    }
    EnOssan_UpdateCameraDirection(this, play, this->cameraFaceAngle);
    if (this->cameraFaceAngle == 0.0f) {
        EnOssan_StartShopping(play, this);
    }
}

static void EnOssan_State_DisplayOnlyBombDialog(EnOssan* this, PlayState* play, Player* player) {
    if (!EnOssan_ReturnItemToShelf(this)) {
        return;
    }
    Math_ApproachF(&this->cameraFaceAngle, 0.0f, 0.5f, 10.0f);
    if (this->cameraFaceAngle < 0.5f && this->cameraFaceAngle > -0.5f) {
        EnOssan_UpdateCameraDirection(this, play, 0.0f);
    }
    EnOssan_UpdateCameraDirection(this, play, this->cameraFaceAngle);
    if (this->cameraFaceAngle == 0.0f) {
        Message_ContinueTextbox(play, TEXT_OOT(0x3010));
        this->stateFlag = OSSAN_STATE_WAIT_FOR_DISPLAY_ONLY_BOMB_DIALOG;
    }
}

static void EnOssan_GiveItemWithFanfare(PlayState* play, EnOssan* this) {
    Player* player = GET_PLAYER(play);

    Actor_OfferGetItem(&this->actor, play, this->shelfSlots[this->cursorIndex]->getItemId, 120.0f, 120.0f);
    play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
    play->msgCtx.stateTimer = 4;
    player->stateFlags2 &= ~PLAYER_STATE2_20000000;
    Hyrule_SetViewpoint(play, 1);
    Interface_SetHudVisibility(50);
    this->drawCursor = 0;
    EnOssan_UpdateCameraDirection(this, play, 0.0f);
    this->stateFlag = OSSAN_STATE_GIVE_ITEM_FANFARE;
}

static void EnOssan_SetStateCantGetItem(PlayState* play, EnOssan* this, u16 textId) {
    Message_ContinueTextbox(play, textId);
    this->stateFlag = OSSAN_STATE_CANT_GET_ITEM;
}

static void EnOssan_SetStateQuickBuyDialog(PlayState* play, EnOssan* this, u16 textId) {
    Message_ContinueTextbox(play, textId);
    this->stateFlag = OSSAN_STATE_QUICK_BUY;
}

static void EnOssan_HandleCanBuyItem(PlayState* play, EnOssan* this) {
    EnGirlA* selectedItem = this->shelfSlots[this->cursorIndex];

    switch (selectedItem->canBuyFunc(play, selectedItem)) {
        case CANBUY_RESULT_SUCCESS_FANFARE:
            if (selectedItem->actor.params == SI_HYLIAN_SHIELD &&
                GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) {
                EnOssan_SetStateGiveDiscountDialog(play, this);
            } else {
                EnOssan_GiveItemWithFanfare(play, this);
                this->drawCursor = 0;
                this->shopItemSelectedTween = 0.0f;
                selectedItem->setOutOfStockFunc(play, selectedItem);
            }
            break;
        case CANBUY_RESULT_SUCCESS:
            selectedItem->itemGiveFunc(play, selectedItem);
            EnOssan_SetStateQuickBuyDialog(play, this, TEXT_OOT(0x84));
            this->drawCursor = 0;
            this->shopItemSelectedTween = 0.0f;
            selectedItem->setOutOfStockFunc(play, selectedItem);
            break;
        case CANBUY_RESULT_CANT_GET_NOW:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x86));
            break;
        case CANBUY_RESULT_NEED_BOTTLE:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x96));
            break;
        case CANBUY_RESULT_NEED_RUPEES:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x85));
            break;
        case CANBUY_RESULT_CANT_GET_NOW_5:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x86));
            break;
    }
}

static void EnOssan_HandleCanBuyLonLonMilk(PlayState* play, EnOssan* this) {
    EnGirlA* item = this->shelfSlots[this->cursorIndex];

    switch (item->canBuyFunc(play, item)) {
        case CANBUY_RESULT_SUCCESS_FANFARE:
            Message_ContinueTextbox(play, TEXT_OOT(0x9C));
            this->stateFlag = OSSAN_STATE_GIVE_LON_LON_MILK;
            this->drawCursor = 0;
            break;
        case CANBUY_RESULT_SUCCESS:
            item->itemGiveFunc(play, item);
            EnOssan_SetStateQuickBuyDialog(play, this, TEXT_OOT(0x98));
            this->drawCursor = 0;
            this->shopItemSelectedTween = 0.0f;
            item->setOutOfStockFunc(play, item);
            break;
        case CANBUY_RESULT_NEED_BOTTLE:
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x96));
            break;
        case CANBUY_RESULT_NEED_RUPEES:
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x85));
            break;
    }
}

static void EnOssan_HandleCanBuyWeirdEgg(PlayState* play, EnOssan* this) {
    EnGirlA* item = this->shelfSlots[this->cursorIndex];

    switch (item->canBuyFunc(play, item)) {
        case CANBUY_RESULT_SUCCESS_FANFARE:
            EnOssan_GiveItemWithFanfare(play, this);
            this->drawCursor = 0;
            this->shopItemSelectedTween = 0.0f;
            item->setOutOfStockFunc(play, item);
            break;
        case CANBUY_RESULT_SUCCESS:
            item->itemGiveFunc(play, item);
            EnOssan_SetStateQuickBuyDialog(play, this, TEXT_OOT(0x9A));
            this->drawCursor = 0;
            this->shopItemSelectedTween = 0.0f;
            item->setOutOfStockFunc(play, item);
            break;
        case CANBUY_RESULT_CANT_GET_NOW:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x9D));
            break;
        case CANBUY_RESULT_NEED_RUPEES:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x85));
            break;
    }
}

static void EnOssan_HandleCanBuyBombs(PlayState* play, EnOssan* this) {
    EnGirlA* item = this->shelfSlots[this->cursorIndex];

    switch (item->canBuyFunc(play, item)) {
        case CANBUY_RESULT_SUCCESS_FANFARE:
        case CANBUY_RESULT_SUCCESS:
            item->itemGiveFunc(play, item);
            EnOssan_SetStateQuickBuyDialog(play, this, TEXT_OOT(0x84));
            this->drawCursor = 0;
            this->shopItemSelectedTween = 0.0f;
            item->setOutOfStockFunc(play, item);
            break;
        case CANBUY_RESULT_CANT_GET_NOW:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x86));
            break;
        case CANBUY_RESULT_NEED_RUPEES:
            Audio_PlaySfx(NA_SE_SY_ERROR);
            EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x85));
            break;
    }
}

static void EnOssan_BuyGoronCityBombs(PlayState* play, EnOssan* this) {
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        // Let players buy the right side of the goron shop in rando regardless of DC completion
        // Players will still need a bomb bag to buy bombs (handled by vanilla behaviour)
        // GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot) - Completed Dodongo's Cavern
        if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
            if (GET_INFTABLE_OOT(INFTABLE_FC_Oot)) {
                EnOssan_SetStateCantGetItem(play, this, TEXT_OOT(0x302E));
            } else {
                this->stickLeftPrompt.isEnabled = false;
                this->stickRightPrompt.isEnabled = false;
                this->drawCursor = 0;
                this->stateFlag = OSSAN_STATE_DISPLAY_ONLY_BOMB_DIALOG;
            }
        } else {
            EnOssan_HandleCanBuyBombs(play, this);
        }
    } else {
        EnOssan_HandleCanBuyBombs(play, this);
    }
}

static void EnOssan_State_ItemSelected(EnOssan* this, PlayState* play2, Player* player) {
    PlayState* play = play2; // Necessary for OKs

    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE &&
        !EnOssan_TestCancelOption(this, play, &play->state.input[0]) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0:
                EnOssan_HandleCanBuyItem(play, this);
                break;
            case 1:
                this->stateFlag = this->tempStateFlag;
                Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
                break;
        }
    }
}

static void EnOssan_State_SelectMilkBottle(EnOssan* this, PlayState* play2, Player* player) {
    PlayState* play = play2; // Need for OK

    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE &&
        !EnOssan_TestCancelOption(this, play, &play->state.input[0]) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0:
                EnOssan_HandleCanBuyLonLonMilk(play, this);
                break;
            case 1:
                this->stateFlag = this->tempStateFlag;
                Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
                break;
        }
    }
}

static void EnOssan_State_SelectWeirdEgg(EnOssan* this, PlayState* play2, Player* player) {
    PlayState* play = play2; // Needed for OK

    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE &&
        !EnOssan_TestCancelOption(this, play, &play->state.input[0]) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0:
                EnOssan_HandleCanBuyWeirdEgg(play, this);
                break;
            case 1:
                this->stateFlag = this->tempStateFlag;
                Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
                break;
        }
    }
}

static void EnOssan_State_SelectUnimplementedItem(EnOssan* this, PlayState* play, Player* player) {
    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        this->stateFlag = this->tempStateFlag;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
    }
}

static void EnOssan_State_SelectBombs(EnOssan* this, PlayState* play, Player* player) {
    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (this->actor.params != OSSAN_TYPE_GORON) {
        EnOssan_State_ItemSelected(this, play, player);
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE &&
        !EnOssan_TestCancelOption(this, play, &play->state.input[0]) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0:
                EnOssan_BuyGoronCityBombs(play, this);
                break;
            case 1:
                this->stateFlag = this->tempStateFlag;
                Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
                break;
        }
    }
}

static void EnOssan_State_SelectMaskItem(EnOssan* this, PlayState* play, Player* player) {
    u8 talkState = Message_GetState(&play->msgCtx);
    EnGirlA* item = this->shelfSlots[this->cursorIndex];

    if (!EnOssan_TakeItemOffShelf(this)) {
        return;
    }
    if (talkState == TEXT_STATE_EVENT) {
        if (Message_ShouldAdvance(play)) {
            this->stateFlag = this->tempStateFlag;
            Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
        }
    } else if (talkState == TEXT_STATE_CHOICE && !EnOssan_TestCancelOption(this, play, &play->state.input[0]) &&
               Message_ShouldAdvance(play)) {
        switch (play->msgCtx.choiceIndex) {
            case 0:
                switch (item->actor.params) {
                    case SI_KEATON_MASK:
                        SET_ITEMGETINF_OOT(ITEMGETINF_23_Oot);
                        break;
                    case SI_SPOOKY_MASK:
                        SET_ITEMGETINF_OOT(ITEMGETINF_25_Oot);
                        break;
                    case SI_SKULL_MASK:
                        SET_ITEMGETINF_OOT(ITEMGETINF_24_Oot);
                        break;
                    case SI_BUNNY_HOOD:
                        SET_ITEMGETINF_OOT(ITEMGETINF_26_Oot);
                        break;
                    case SI_MASK_OF_TRUTH:
                    case SI_ZORA_MASK:
                    case SI_GORON_MASK:
                    case SI_GERUDO_MASK:
                        break;
                }
                EnOssan_GiveItemWithFanfare(play, this);
                this->drawCursor = 0;
                this->shopItemSelectedTween = 0.0f;
                item->setOutOfStockFunc(play, item);
                break;
            case 1:
                this->stateFlag = this->tempStateFlag;
                Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
                break;
        }
    }
}

static void EnOssan_State_CantGetItem(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        this->stateFlag = this->tempStateFlag;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
    }
}

static void EnOssan_State_QuickBuyDialog(EnOssan* this, PlayState* play, Player* player) {
    EnGirlA* item;

    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        this->shopItemSelectedTween = 0.0f;
        EnOssan_ResetItemPosition(this);
        item = this->shelfSlots[this->cursorIndex];
        item->updateStockedItemFunc(play, item);
        this->stateFlag = this->tempStateFlag;
        Message_ContinueTextbox(play, this->shelfSlots[this->cursorIndex]->actor.textId);
    }
}

static void EnOssan_State_GiveItemWithFanfare(EnOssan* this, PlayState* play, Player* player) {
    // The player sets itself as the parent actor to signal that it has obtained the give item request
    if (Actor_HasParent(&this->actor, play)) {
        this->actor.parent = NULL;
        this->stateFlag = OSSAN_STATE_ITEM_PURCHASED;
        return;
    }
    Actor_OfferGetItem(&this->actor, play, this->shelfSlots[this->cursorIndex]->getItemId, 120.0f, 120.0f);
}

static void EnOssan_State_ItemPurchased(EnOssan* this, PlayState* play, Player* player) {
    EnGirlA* item;
    EnGirlA* itemTemp;

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        if (this->actor.params == OSSAN_TYPE_MASK) {
            itemTemp = this->shelfSlots[this->cursorIndex];
            EnOssan_ResetItemPosition(this);
            item = this->shelfSlots[this->cursorIndex];
            item->updateStockedItemFunc(play, item);
            if (itemTemp->actor.params == SI_MASK_OF_TRUTH && !GET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot)) {
                SET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot);
                // 3Ship [Hyrule] After the fourth sale he gives the other masks too, and the Hero's Shield once Link
                // owns the Termina gate's other items (DESIGN.md: Mask Shop). OoT's line was 0x70AB.
                Hyrule_GiveMaskShopGift(play);
                if (Hyrule_OwnsTerminaGateItems()) {
                    Hyrule_ContinueTextbox(play, HYRULE_LINE_MASK_GIFT);
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_OFFER_HERO_SHIELD_3SHIP;
                } else {
                    Hyrule_ContinueTextbox(play, HYRULE_LINE_MASK_GIFT_WITH_HINT);
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_BORROWED_FIRST_MASK;
                }
                EnOssan_UpdateShopOfferings(this, play);
                this->stateFlag = OSSAN_STATE_START_CONVERSATION;
                return;
            } else {
                EnOssan_EndInteraction(play, this);
                return;
            }
        }
        item = this->shelfSlots[this->cursorIndex];
        item->buyEventFunc(play, item);
        this->stateFlag = OSSAN_STATE_CONTINUE_SHOPPING_PROMPT;
        Message_ContinueTextbox(play, TEXT_OOT(0x6B));
    }
}

static void EnOssan_State_ContinueShoppingPrompt(EnOssan* this, PlayState* play, Player* player) {
    EnGirlA* selectedItem;
    u8 talkState = Message_GetState(&play->msgCtx);

    if (talkState == TEXT_STATE_CHOICE) {
        if (Message_ShouldAdvance(play)) {
            EnOssan_ResetItemPosition(this);
            selectedItem = this->shelfSlots[this->cursorIndex];
            selectedItem->updateStockedItemFunc(play, selectedItem);
            if (!EnOssan_TestEndInteraction(this, play, &play->state.input[0])) {
                switch (play->msgCtx.choiceIndex) {
                    case 0:
                        player->actor.shape.rot.y += 0x8000;
                        player->stateFlags2 |= PLAYER_STATE2_20000000;
                        Hyrule_SetViewpoint(play, 2);
                        Message_StartTextbox(play, this->actor.textId, &this->actor);
                        EnOssan_SetStateStartShopping(play, this, true);
                        Actor_OfferTalkExchangeEquiCylinder(&this->actor, play, 100.0f, Hyrule_ExchangeItemToActionOot(-1));
                        break;
                    case 1:
                    default:
                        EnOssan_EndInteraction(play, this);
                        break;
                }
            }
        }
    } else if (talkState == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        EnOssan_ResetItemPosition(this);
        selectedItem = this->shelfSlots[this->cursorIndex];
        selectedItem->updateStockedItemFunc(play, selectedItem);
        player->actor.shape.rot.y += 0x8000;
        player->stateFlags2 |= PLAYER_STATE2_20000000;
        Hyrule_SetViewpoint(play, 2);
        Message_StartTextbox(play, this->actor.textId, &this->actor);
        EnOssan_SetStateStartShopping(play, this, true);
        Actor_OfferTalkExchangeEquiCylinder(&this->actor, play, 100.0f, Hyrule_ExchangeItemToActionOot(-1));
    }
}

static void EnOssan_State_WaitForDisplayOnlyBombDialog(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        SET_INFTABLE_OOT(INFTABLE_FC_Oot);
        EnOssan_StartShopping(play, this);
    }
}

// Unreachable
static void EnOssan_State_21(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_NEXT && Message_ShouldAdvance(play)) {
        this->stateFlag = OSSAN_STATE_22;
        Message_ContinueTextbox(play, TEXT_OOT(0x3012));
        SET_INFTABLE_OOT(INFTABLE_FC_Oot);
    }
}

// Unreachable
static void EnOssan_State_22(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        EnOssan_StartShopping(play, this);
    }
}

static void EnOssan_State_GiveLonLonMilk(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        EnOssan_GiveItemWithFanfare(play, this);
    }
}

// For giving Mask of Truth when you first sell all masks
static void EnOssan_State_LendMaskOfTruth(EnOssan* this, PlayState* play, Player* player) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT && Message_ShouldAdvance(play)) {
        SET_ITEMGETINF_OOT(ITEMGETINF_2A_Oot);
        this->cursorIndex = 2;
        EnOssan_GiveItemWithFanfare(play, this);
    }
}

// 3Ship [Hyrule] The Hero's Shield and the salesman's departure for Termina (docs/TERMINA.md). His offer is an event
// line; Link then holds up the shield, the salesman says farewell, and Link is back in the market.
static void EnOssan_State_OfferHeroShield(EnOssan* this, PlayState* play, Player* player) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Actor_OfferGetItem(&this->actor, play, GI_SHIELD_HERO, 120.0f, 120.0f);
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        play->msgCtx.stateTimer = 4;
        player->stateFlags2 &= ~PLAYER_STATE2_20000000;
        Hyrule_SetViewpoint(play, 1);
        Interface_SetHudVisibility(50);
        this->drawCursor = 0;
        EnOssan_UpdateCameraDirection(this, play, 0.0f);
        this->stateFlag = OSSAN_STATE_GIVE_HERO_SHIELD_3SHIP;
    }
}

static void EnOssan_State_GiveHeroShield(EnOssan* this, PlayState* play, Player* player) {
    if (Actor_HasParent(&this->actor, play)) {
        this->actor.parent = NULL;
        this->stateFlag = OSSAN_STATE_HERO_SHIELD_GIVEN_3SHIP;
        return;
    }
    Actor_OfferGetItem(&this->actor, play, GI_SHIELD_HERO, 120.0f, 120.0f);
}

static void EnOssan_State_HeroShieldGiven(EnOssan* this, PlayState* play, Player* player) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        Hyrule_ContinueTextbox(play, HYRULE_LINE_SALESMAN_FAREWELL);
        this->stateFlag = OSSAN_STATE_LEAVE_FOR_TERMINA_3SHIP;
    }
}

static void EnOssan_State_LeaveForTermina(EnOssan* this, PlayState* play, Player* player) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Hyrule_UnlockTermina();
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;
        play->msgCtx.stateTimer = 4;
        play->nextEntrance = Hyrule_ExitFromOot(0x1D1); // OoT's ENTR_MARKET_DAY_OUTSIDE_HAPPY_MASK_SHOP
        play->transitionTrigger = TRANS_TRIGGER_START;
        play->transitionType = TRANS_TYPE_FADE_WHITE;
        this->stateFlag = OSSAN_STATE_IDLE;
    }
}

// Hylian Shield discount dialog
static void EnOssan_SetStateGiveDiscountDialog(PlayState* play, EnOssan* this) {
    Message_ContinueTextbox(play, TEXT_OOT(0x71B2));
    this->stateFlag = OSSAN_STATE_DISCOUNT_DIALOG;
}

static void EnOssan_State_GiveDiscountDialog(EnOssan* this, PlayState* play, Player* player) {
    EnGirlA* selectedItem;

    if (Message_GetState(&play->msgCtx) == TEXT_STATE_DONE && Message_ShouldAdvance(play)) {
        selectedItem = this->shelfSlots[this->cursorIndex];
        EnOssan_GiveItemWithFanfare(play, this);
        this->drawCursor = 0;
        this->shopItemSelectedTween = 0.0f;
        selectedItem->setOutOfStockFunc(play, selectedItem);
    }
}

static void EnOssan_PositionSelectedItem(EnOssan* this) {
    EnGirlA* item;
    u8 i;
    u8 i2;
    ShopItem* shopItem;
    f32 tx;
    f32 ty;
    f32 tz;

    i = this->cursorIndex;
    shopItem = &sShopkeeperStores[this->actor.params][i];
    item = this->shelfSlots[i];

    i2 = i >> 2;
    tx = (sSelectedItemPosition[i2].x - shopItem->xOffset) * this->shopItemSelectedTween + shopItem->xOffset;
    ty = (sSelectedItemPosition[i2].y - shopItem->yOffset) * this->shopItemSelectedTween + shopItem->yOffset;
    tz = (sSelectedItemPosition[i2].z - shopItem->zOffset) * this->shopItemSelectedTween + shopItem->zOffset;

    item->actor.world.pos.x = this->shelves->actor.world.pos.x + tx;
    item->actor.world.pos.y = this->shelves->actor.world.pos.y + ty;
    item->actor.world.pos.z = this->shelves->actor.world.pos.z + tz;
}

static void EnOssan_ResetItemPosition(EnOssan* this) {
    this->shopItemSelectedTween = 0.0f;
    EnOssan_PositionSelectedItem(this);
}

// returns true if animation has completed
static s32 EnOssan_TakeItemOffShelf(EnOssan* this) {
    Math_ApproachF(&this->shopItemSelectedTween, 1.0f, 1.0f, 0.15f);
    if (this->shopItemSelectedTween >= 0.85f) {
        this->shopItemSelectedTween = 1.0f;
    }
    EnOssan_PositionSelectedItem(this);
    if (this->shopItemSelectedTween == 1.0f) {
        return true;
    } else {
        return false;
    }
}

// returns true if animation has completed
static s32 EnOssan_ReturnItemToShelf(EnOssan* this) {
    Math_ApproachF(&this->shopItemSelectedTween, 0.0f, 1.0f, 0.15f);
    if (this->shopItemSelectedTween <= 0.15f) {
        this->shopItemSelectedTween = 0.0f;
    }
    EnOssan_PositionSelectedItem(this);
    if (this->shopItemSelectedTween == 0.0f) {
        return true;
    } else {
        return false;
    }
}

static void EnOssan_UpdateItemSelectedProperty(EnOssan* this) {
    EnGirlA** temp_a1 = this->shelfSlots;
    s32 i;

    for (i = 0; i < 8; i++) {
        if (temp_a1[0] != NULL) {
            if (this->stateFlag != OSSAN_STATE_SELECT_ITEM && this->stateFlag != OSSAN_STATE_SELECT_ITEM_MILK_BOTTLE &&
                this->stateFlag != OSSAN_STATE_SELECT_ITEM_WEIRD_EGG &&
                this->stateFlag != OSSAN_STATE_SELECT_ITEM_UNIMPLEMENTED &&
                this->stateFlag != OSSAN_STATE_SELECT_ITEM_BOMBS && this->stateFlag != OSSAN_STATE_SELECT_ITEM_MASK &&
                this->stateFlag != OSSAN_STATE_CANT_GET_ITEM && this->drawCursor == 0) {
                temp_a1[0]->isSelected = false;
            } else {
                if (this->cursorIndex == i) {
                    temp_a1[0]->isSelected = true;
                } else {
                    temp_a1[0]->isSelected = false;
                }
            }
        }
        temp_a1++;
    }
}

static void EnOssan_UpdateCursorAnim(EnOssan* this) {
    Color_RGB8 aButtonColor = { 0, 80, 255 };
    if (0) {
        aButtonColor = aButtonColor;
    } else if (false) {
        aButtonColor = (Color_RGB8){ 0, 255, 80 };
    }
    f32 t;

    t = this->cursorAnimTween;
    if (this->cursorAnimState == 0) {
        t += 0.05f;
        if (t >= 1.0f) {
            t = 1.0f;
            this->cursorAnimState = 1;
        }
    } else {
        t -= 0.05f;
        if (t <= 0.0f) {
            t = 0.0f;
            this->cursorAnimState = 0;
        }
    }
    this->cursorColorR = ColChanMix(aButtonColor.r, 0.0f, t);
    this->cursorColorG = ColChanMix(aButtonColor.g, 80.0f, t);
    this->cursorColorB = ColChanMix(aButtonColor.b, 0.0f, t);
    this->cursorColorA = ColChanMix(255, 0.0f, t);
    this->cursorAnimTween = t;
}

static void EnOssan_UpdateStickDirectionPromptAnim(EnOssan* this) {
    f32 arrowAnimTween;
    f32 new_var3;       // likely fake temp
    s32 new_var2 = 255; // likely fake temp
    f32 stickAnimTween;

    arrowAnimTween = this->arrowAnimTween;
    stickAnimTween = this->stickAnimTween;
    if (this->arrowAnimState == 0) {
        arrowAnimTween += 0.05f;
        if (arrowAnimTween > 1.0f) {
            arrowAnimTween = 1.0f;
            this->arrowAnimState = 1;
        }

    } else {
        arrowAnimTween -= 0.05f;
        if (arrowAnimTween < 0.0f) {
            arrowAnimTween = 0.0f;
            this->arrowAnimState = 0;
        }
    }

    this->arrowAnimTween = arrowAnimTween;
    if (this->stickAnimState == 0) {
        stickAnimTween += 0.1f;
        if (stickAnimTween > 1.0f) {
            stickAnimTween = 1.0f;
            this->stickAnimState = 1;
        }

    } else {
        stickAnimTween = 0.0f;
        this->stickAnimState = 0;
    }

    this->stickAnimTween = stickAnimTween;
    this->stickLeftPrompt.arrowColorR = (u8)(255 - ((s32)(155.0f * arrowAnimTween)));
    this->stickLeftPrompt.arrowColorG = (u8)(new_var2 - (s32)(155.0f * arrowAnimTween));
    new_var3 = (155.0f * arrowAnimTween);
    this->stickLeftPrompt.arrowColorB = (u8)(0 - ((s32)((-100.0f) * arrowAnimTween)));
    this->stickLeftPrompt.arrowColorA = (u8)(200 - ((s32)(50.0f * arrowAnimTween)));
    this->stickRightPrompt.arrowColorR = (u8)(new_var2 - (s32)new_var3);
    this->stickRightPrompt.arrowColorG = (u8)(255 - (s32)new_var3);
    this->stickRightPrompt.arrowColorB = (u8)(0 - ((s32)((-100.0f) * arrowAnimTween)));
    this->stickRightPrompt.arrowColorA = (u8)(200 - ((s32)(50.0f * arrowAnimTween)));
    this->stickRightPrompt.arrowTexX = 290.0f;
    this->stickLeftPrompt.arrowTexX = 33.0f;
    this->stickRightPrompt.stickTexX = 274.0f;
    this->stickLeftPrompt.stickTexX = 49.0f;
    this->stickRightPrompt.stickTexX += (8.0f * stickAnimTween);
    this->stickLeftPrompt.stickTexX -= (8.0f * stickAnimTween);
    this->stickLeftPrompt.arrowTexY = this->stickRightPrompt.arrowTexY = 91.0f;
    this->stickLeftPrompt.stickTexY = this->stickRightPrompt.stickTexY = 95.0f;
}

static void EnOssan_WaitForBlink(EnOssan* this) {
    s16 decr = this->blinkTimer - 1;

    if (decr != 0) {
        this->blinkTimer = decr;
    } else {
        this->blinkFunc = EnOssan_Blink;
    }
}

static void EnOssan_Blink(EnOssan* this) {
    s16 decr;
    s16 eyeTextureIdxTemp;

    decr = this->blinkTimer - 1;
    if (decr != 0) {
        this->blinkTimer = decr;
        return;
    }
    eyeTextureIdxTemp = this->eyeTextureIdx + 1;
    if (eyeTextureIdxTemp > 2) {
        this->eyeTextureIdx = 0;
        this->blinkTimer = (s32)(Rand_ZeroOne() * 60.0f) + 20;
        this->blinkFunc = EnOssan_WaitForBlink;
    } else {
        this->eyeTextureIdx = eyeTextureIdxTemp;
        this->blinkTimer = 1;
    }
}

static s32 EnOssan_AreShopkeeperObjectsLoaded(EnOssan* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->objBankIndex1)) {
        if (this->objBankIndex2 >= 0 && !Object_IsLoaded(&play->objectCtx, this->objBankIndex2)) {
            return false;
        }
        if (this->objBankIndex3 >= 0 && !Object_IsLoaded(&play->objectCtx, this->objBankIndex3)) {
            return false;
        }
        return true;
    }
    return false;
}

static void EnOssan_InitBazaarShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gObjectOssanSkel, &gObjectOssanAnim_000338, NULL, NULL, 0);
    this->actor.draw = EnOssan_DrawBazaarShopkeeper;
    this->obj3ToSeg6Func = NULL;
}

static void EnOssan_InitKokiriShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gKm1Skel, NULL, NULL, NULL, 0);
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objBankIndex3].segment);
    Animation_Change(&this->skelAnime, &object_masterkokiri_Anim_0004A8, 1.0f, 0.0f,
                     Animation_GetLastFrame(&object_masterkokiri_Anim_0004A8), 0, 0.0f);
    this->actor.draw = EnOssan_DrawKokiriShopkeeper;
    this->obj3ToSeg6Func = EnOssan_Obj3ToSeg6;
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_ELF_Oot, this->actor.world.pos.x,
                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, FAIRY_KOKIRI);
}

static void EnOssan_InitGoronShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gGoronSkel, NULL, NULL, NULL, 0);
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objBankIndex3].segment);
    Animation_Change(&this->skelAnime, &gGoronShopkeeperAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gGoronShopkeeperAnim),
                     0, 0.0f);
    this->actor.draw = EnOssan_DrawGoronShopkeeper;
    this->obj3ToSeg6Func = EnOssan_Obj3ToSeg6;
}

static void EnOssan_InitZoraShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gZoraSkel, NULL, NULL, NULL, 0);
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objBankIndex3].segment);
    Animation_Change(&this->skelAnime, &gZoraShopkeeperAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gZoraShopkeeperAnim),
                     0, 0.0f);
    this->actor.draw = EnOssan_DrawZoraShopkeeper;
    this->obj3ToSeg6Func = EnOssan_Obj3ToSeg6;
}

static void EnOssan_InitPotionShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &object_ds2_Skel_004258, &object_ds2_Anim_0002E4, 0, 0, 0);
    this->actor.draw = EnOssan_DrawPotionShopkeeper;
    this->obj3ToSeg6Func = NULL;
}

static void EnOssan_InitHappyMaskShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gHappyMaskSalesmanSkel, &gHappyMaskSalesmanIdleAnim, NULL, NULL, 0);
    this->actor.draw = EnOssan_DrawHappyMaskShopkeeper;
    this->obj3ToSeg6Func = NULL;
}

static void EnOssan_InitBombchuShopkeeper(EnOssan* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gBombchuShopkeeperSkel, &gBombchuShopkeeperIdleAnim, 0, 0, 0);
    this->actor.draw = EnOssan_DrawBombchuShopkeeper;
    this->obj3ToSeg6Func = NULL;
}

static u16 EnOssan_SetupHelloDialog(EnOssan* this) {
    this->happyMaskShopState = OSSAN_HAPPY_STATE_NONE;
    // 3Ship [Hyrule] Link comes back with the Termina gate's items after the masks' gift: after his welcome, the
    // salesman gives the Hero's Shield (docs/TERMINA.md).
    if ((this->actor.params == OSSAN_TYPE_MASK) && GET_ITEMGETINF_OOT(ITEMGETINF_3F_Oot) &&
        !Hyrule_IsTerminaUnlocked() && Hyrule_OwnsTerminaGateItems()) {
        this->happyMaskShopState = OSSAN_HAPPY_STATE_OFFER_HERO_SHIELD_3SHIP;
        return TEXT_OOT(0x70AC);
    }
    // mask shop messages
    if (this->actor.params == OSSAN_TYPE_MASK) {
        if ((INV_CONTENT_OOT(ITEM_TRADE_CHILD_Oot) == ITEM_SOLD_OUT_Oot)) {
            if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_BUNNY_HOOD_FEE_Oot)) {
                    // Pay back Bunny Hood
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_REQUEST_PAYMENT_BUNNY_HOOD;
                    return TEXT_OOT(0x70C6);
                } else {
                    return TEXT_OOT(0x70AC);
                }
            }
            if (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot)) {
                if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_SPOOKY_MASK_FEE_Oot)) {
                    // Pay back Spooky Mask
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SPOOKY_MASK;
                    return TEXT_OOT(0x70C5);
                } else {
                    return TEXT_OOT(0x70AC);
                }
            }
            if (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot)) {
                if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_SKULL_MASK_FEE_Oot)) {
                    // Pay back Skull Mask
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_REQUEST_PAYMENT_SKULL_MASK;
                    return TEXT_OOT(0x70C4);
                } else {
                    return TEXT_OOT(0x70AC);
                }
            }
            if (GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) {
                if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_PAID_BACK_KEATON_MASK_FEE_Oot)) {
                    // Pay back Keaton Mask
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_REQUEST_PAYMENT_KEATON_MASK;
                    return TEXT_OOT(0x70A5);
                } else {
                    return TEXT_OOT(0x70AC);
                }
            }
        } else {
            if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                return TEXT_OOT(0x70AC);
            } else if (!GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot) && !GET_ITEMGETINF_OOT(ITEMGETINF_24_Oot) &&
                       !GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) {
                // Haven't borrowed the Keaton Mask
                if (!GET_ITEMGETINF_OOT(ITEMGETINF_23_Oot)) {
                    return TEXT_OOT(0x70A1);
                } else {
                    // Haven't sold the Keaton Mask
                    this->happyMaskShopState = OSSAN_HAPPY_STATE_BORROWED_FIRST_MASK;
                    return TEXT_OOT(0x70A6);
                }
            } else {
                return TEXT_OOT(0x70C7);
            }
        }
    }

    return TEXT_OOT(0x9E);
}

static void EnOssan_InitActionFunc(EnOssan* this, PlayState* play) {
    ShopItem* items;

    if (EnOssan_AreShopkeeperObjectsLoaded(this, play)) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->actor.objectSlot = this->objBankIndex1;
        Actor_SetObjectDependency(play, &this->actor);

        this->shelves = (EnTana*)Actor_FindOot(&play->actorCtx, ACTOR_EN_TANA_Oot, ACTORCAT_PROP);

        if (this->shelves == NULL) {
            return;
        }


        this->actor.world.pos.x += sShopkeeperPositionOffsets[this->actor.params].x;
        this->actor.world.pos.y += sShopkeeperPositionOffsets[this->actor.params].y;
        this->actor.world.pos.z += sShopkeeperPositionOffsets[this->actor.params].z;

        items = sShopkeeperStores[this->actor.params];

        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
        sInitFuncs[this->actor.params](this, play);
        this->actor.textId = EnOssan_SetupHelloDialog(this);
        this->cursorY = this->cursorX = 100.0f;
        this->actor.colChkInfo.mass = MASS_IMMOVABLE;
        this->actor.colChkInfo.cylRadius = 50;
        this->stateFlag = OSSAN_STATE_IDLE;
        this->stickAccumX = this->stickAccumY = 0;

        this->cursorIndex = 0;
        this->cursorZ = 1.5f;
        this->cursorColorR = 0;
        this->cursorColorG = 255;
        this->cursorColorB = 80;
        this->cursorColorA = 255;
        this->cursorAnimTween = 0;

        this->cursorAnimState = 0;
        this->drawCursor = 0;
        this->happyMaskShopkeeperEyeIdx = 0;

        this->stickLeftPrompt.stickColorR = 200;
        this->stickLeftPrompt.stickColorG = 200;
        this->stickLeftPrompt.stickColorB = 200;
        this->stickLeftPrompt.stickColorA = 180;
        this->stickLeftPrompt.stickTexX = 49;
        this->stickLeftPrompt.stickTexY = 95;
        this->stickLeftPrompt.arrowColorR = 255;
        this->stickLeftPrompt.arrowColorG = 255;
        this->stickLeftPrompt.arrowColorB = 0;
        this->stickLeftPrompt.arrowColorA = 200;
        this->stickLeftPrompt.arrowTexX = 33;
        this->stickLeftPrompt.arrowTexY = 91;
        this->stickLeftPrompt.z = 1;
        this->stickLeftPrompt.isEnabled = false;

        this->stickRightPrompt.stickColorR = 200;
        this->stickRightPrompt.stickColorG = 200;
        this->stickRightPrompt.stickColorB = 200;
        this->stickRightPrompt.stickColorA = 180;
        this->stickRightPrompt.stickTexX = 274;
        this->stickRightPrompt.stickTexY = 95;
        this->stickRightPrompt.arrowColorR = 255;
        this->stickRightPrompt.arrowColorG = 255;
        this->stickRightPrompt.arrowColorB = 0;
        this->stickRightPrompt.arrowColorA = 200;
        this->stickRightPrompt.arrowTexX = 290;
        this->stickRightPrompt.arrowTexY = 91;
        this->stickRightPrompt.z = 1;
        this->stickRightPrompt.isEnabled = false;

        this->arrowAnimState = 0;
        this->stickAnimState = 0;
        this->arrowAnimTween = 0;
        this->stickAnimTween = 0;
        this->shopItemSelectedTween = 0;
        Actor_SetScale(&this->actor, sShopkeeperScale[this->actor.params]);
        EnOssan_SpawnItemsOnShelves(this, play, items);
        this->headRot = this->headTargetRot = 0;
        this->blinkTimer = 20;
        this->eyeTextureIdx = 0;
        this->blinkFunc = EnOssan_WaitForBlink;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        EnOssan_SetupAction(this, EnOssan_MainActionFunc);
    }
}

static void EnOssan_Obj3ToSeg6(EnOssan* this, PlayState* play) {
    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objBankIndex3].segment);
}

static void EnOssan_MainActionFunc(EnOssan* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->blinkFunc(this);
    EnOssan_UpdateJoystickInputState(play, this);
    EnOssan_UpdateItemSelectedProperty(this);
    EnOssan_UpdateStickDirectionPromptAnim(this);
    EnOssan_UpdateCursorAnim(this);
    Math_StepToS(&this->headRot, this->headTargetRot, 0x190);

    if (player != NULL) {
        sStateFunc[this->stateFlag](this, play, player);
    }

    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 26.0f, 10.0f, 0.0f, 0x405);
    Actor_SetFocus(&this->actor, 90.0f);
    Actor_SetScale(&this->actor, sShopkeeperScale[this->actor.params]);

    // use animation object if needed
    if (this->obj3ToSeg6Func != NULL) {
        this->obj3ToSeg6Func(this, play);
    }

    SkelAnime_Update(&this->skelAnime);
}

static void EnOssan_Update(Actor* thisx, PlayState* play) {
    EnOssan* this = (EnOssan*)thisx;

    this->timer++;
    this->actionFunc(this, play);
}

static s32 EnOssan_OverrideLimbDrawDefaultShopkeeper(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                              void* thisx) {
    EnOssan* this = (EnOssan*)thisx;

    if (limbIndex == 8) {
        rot->x += this->headRot;
    }
    return 0;
}

static void EnOssan_DrawCursor(PlayState* play, EnOssan* this, f32 x, f32 y, f32 z, u8 drawCursor) {
    s32 ulx, uly, lrx, lry;
    f32 w;
    s32 dsdx;

    OPEN_DISPS(play->state.gfxCtx);
    if (drawCursor != 0) {
        Gfx_SetupDL39_Overlay(play->state.gfxCtx);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, this->cursorColorR, this->cursorColorG, this->cursorColorB,
                        this->cursorColorA);
        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gSelectionCursorTex, G_IM_FMT_IA, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
                               G_TX_MIRROR | G_TX_WRAP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);
        w = 16.0f * z;
        ulx = (x - w) * 4.0f;
        uly = (y - w) * 4.0f;
        lrx = (x + w) * 4.0f;
        lry = (y + w) * 4.0f;
        dsdx = (1.0f / z) * 1024.0f;
        gSPTextureRectangle(OVERLAY_DISP++, ulx, uly, lrx, lry, G_TX_RENDERTILE, 0, 0, dsdx, dsdx);
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawTextRec(PlayState* play, s32 r, s32 g, s32 b, s32 a, f32 x, f32 y, f32 z, s32 s, s32 t, f32 dx,
                         f32 dy) {
    f32 unk;
    s32 ulx, uly, lrx, lry;
    f32 w, h;
    s32 dsdx, dtdy;

    OPEN_DISPS(play->state.gfxCtx);
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, r, g, b, a);

    w = 8.0f * z;
    h = 12.0f * z;
    unk = (1.0f / z) * 1024;
    dsdx = unk * dx;
    dtdy = dy * unk;

    ulx = (x - w) * 4.0f;
    uly = (y - h) * 4.0f;
    lrx = (x + w) * 4.0f;
    lry = (y + h) * 4.0f;
    gSPTextureRectangle(OVERLAY_DISP++, ulx, uly, lrx, lry, G_TX_RENDERTILE, s, t, dsdx, dtdy);
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawStickDirectionPrompts(PlayState* play, EnOssan* this) {
    s32 drawStickLeftPrompt = this->stickLeftPrompt.isEnabled;
    s32 drawStickRightPrompt = this->stickRightPrompt.isEnabled;

    // Invert which stick prompt is active when only one is active
    if (0 && (drawStickLeftPrompt != drawStickRightPrompt)) {
        drawStickLeftPrompt = !drawStickLeftPrompt;
        drawStickRightPrompt = !drawStickRightPrompt;
    }

    OPEN_DISPS(play->state.gfxCtx);
    if (drawStickLeftPrompt || drawStickRightPrompt) {
        Gfx_SetupDL39_Overlay(play->state.gfxCtx);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPLoadTextureBlock(OVERLAY_DISP++, gArrowCursorTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 24, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 4, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        if (drawStickLeftPrompt) {
            EnOssan_DrawTextRec(play, this->stickLeftPrompt.arrowColorR, this->stickLeftPrompt.arrowColorG,
                                this->stickLeftPrompt.arrowColorB, this->stickLeftPrompt.arrowColorA,
                                this->stickLeftPrompt.arrowTexX, this->stickLeftPrompt.arrowTexY,
                                this->stickLeftPrompt.z, 0, 0, -1.0f, 1.0f);
        }
        if (drawStickRightPrompt) {
            EnOssan_DrawTextRec(play, this->stickRightPrompt.arrowColorR, this->stickRightPrompt.arrowColorG,
                                this->stickRightPrompt.arrowColorB, this->stickRightPrompt.arrowColorA,
                                this->stickRightPrompt.arrowTexX, this->stickRightPrompt.arrowTexY,
                                this->stickRightPrompt.z, 0, 0, 1.0f, 1.0f);
        }
        gDPLoadTextureBlock(OVERLAY_DISP++, gControlStickTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 4, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        if (drawStickLeftPrompt) {
            EnOssan_DrawTextRec(play, this->stickLeftPrompt.stickColorR, this->stickLeftPrompt.stickColorG,
                                this->stickLeftPrompt.stickColorB, this->stickLeftPrompt.stickColorA,
                                this->stickLeftPrompt.stickTexX, this->stickLeftPrompt.stickTexY,
                                this->stickLeftPrompt.z, 0, 0, -1.0f, 1.0f);
        }
        if (drawStickRightPrompt) {
            EnOssan_DrawTextRec(play, this->stickRightPrompt.stickColorR, this->stickRightPrompt.stickColorG,
                                this->stickRightPrompt.stickColorB, this->stickRightPrompt.stickColorA,
                                this->stickRightPrompt.stickTexX, this->stickRightPrompt.stickTexY,
                                this->stickRightPrompt.z, 0, 0, 1.0f, 1.0f);
        }
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawBazaarShopkeeper(Actor* thisx, PlayState* play) {
    static void* sBazaarShopkeeperEyeTextures[] = { gOssanEyeOpenTex, gOssanEyeHalfTex, gOssanEyeClosedTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sBazaarShopkeeperEyeTextures[this->eyeTextureIdx]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnOssan_OverrideLimbDrawDefaultShopkeeper, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static s32 EnOssan_OverrideLimbDrawKokiriShopkeeper(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                             void* thisx) {
    static void* sKokiriShopkeeperEyeTextures[] = {
        gKokiriShopkeeperEyeDefaultTex,
        gKokiriShopkeeperEyeHalfTex,
        gKokiriShopkeeperEyeOpenTex,
    };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == 15) {
        gSPSegment(POLY_OPA_DISP++, 0x06, play->objectCtx.slots[this->objBankIndex2].segment);
        gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->objBankIndex2].segment);
        *dList = gKokiriShopkeeperHeadDL;
        gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(sKokiriShopkeeperEyeTextures[this->eyeTextureIdx]));
    }

    CLOSE_DISPS(play->state.gfxCtx);

    return 0;
}

Gfx* EnOssan_EndDList(GraphicsContext* gfxCtx) {
    Gfx* disp = GRAPH_ALLOC(gfxCtx, sizeof(Gfx));

    gSPEndDisplayList(disp);
    return disp;
}

Gfx* EnOssan_SetEnvColor(GraphicsContext* gfxCtx, u8 r, u8 g, u8 b, u8 a) {
    Gfx* disp = GRAPH_ALLOC(gfxCtx, sizeof(Gfx) * 2);

    gDPSetEnvColor(disp, r, g, b, a);
    gSPEndDisplayList(disp + 1);
    return disp;
}

static void EnOssan_DrawKokiriShopkeeper(Actor* thisx, PlayState* play) {
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSPSegment(POLY_OPA_DISP++, 0x08, EnOssan_SetEnvColor(play->state.gfxCtx, 0, 130, 70, 255));
    gSPSegment(POLY_OPA_DISP++, 0x09, EnOssan_SetEnvColor(play->state.gfxCtx, 110, 170, 20, 255));
    gSPSegment(POLY_OPA_DISP++, 0x0C, EnOssan_EndDList(play->state.gfxCtx));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnOssan_OverrideLimbDrawKokiriShopkeeper, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static s32 EnGo2_OverrideLimbDrawGoronShopkeeper(PlayState* play, s32 limb, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnOssan* this = (EnOssan*)thisx;

    if (limb == 17) {
        Matrix_Translate(0.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    return 0;
}

static void EnOssan_DrawGoronShopkeeper(Actor* thisx, PlayState* play) {
    static void* sGoronShopkeeperEyeTextures[] = { gGoronCsEyeOpenTex, gGoronCsEyeHalfTex, gGoronCsEyeClosedTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sGoronShopkeeperEyeTextures[this->eyeTextureIdx]));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(gGoronCsMouthNeutralTex));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnGo2_OverrideLimbDrawGoronShopkeeper, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static s32 EnOssan_OverrideLimbDrawZoraShopkeeper(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                           void* thisx) {
    EnOssan* this = (EnOssan*)thisx;

    if (limbIndex == 15) {
        rot->x += this->headRot;
    }
    return 0;
}

static void EnOssan_DrawZoraShopkeeper(Actor* thisx, PlayState* play) {
    static void* sZoraShopkeeperEyeTextures[] = { gZoraEyeOpenTex, gZoraEyeHalfTex, gZoraEyeClosedTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSPSegment(POLY_OPA_DISP++, 0x0C, EnOssan_EndDList(play->state.gfxCtx));
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sZoraShopkeeperEyeTextures[this->eyeTextureIdx]));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnOssan_OverrideLimbDrawZoraShopkeeper, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawPotionShopkeeper(Actor* thisx, PlayState* play) {
    static void* sPotionShopkeeperEyeTextures[] = { gPotionShopkeeperEyeOpenTex, gPotionShopkeeperEyeHalfTex,
                                                    gPotionShopkeeperEyeClosedTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sPotionShopkeeperEyeTextures[this->eyeTextureIdx]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawHappyMaskShopkeeper(Actor* thisx, PlayState* play) {
    static void* sHappyMaskShopkeeperEyeTextures[] = { gHappyMaskSalesmanEyeClosedTex, gHappyMaskSalesmanEyeOpenTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08,
               Lib_SegmentedToVirtual(sHappyMaskShopkeeperEyeTextures[this->happyMaskShopkeeperEyeIdx]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void EnOssan_DrawBombchuShopkeeper(Actor* thisx, PlayState* play) {
    static void* sBombchuShopkeeperEyeTextures[] = { gBombchuShopkeeperEyeOpenTex, gBombchuShopkeeperEyeHalfTex,
                                                     gBombchuShopkeeperEyeClosedTex };
    EnOssan* this = (EnOssan*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sBombchuShopkeeperEyeTextures[this->eyeTextureIdx]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, this);
    EnOssan_DrawCursor(play, this, this->cursorX, this->cursorY, this->cursorZ, this->drawCursor);
    EnOssan_DrawStickDirectionPrompts(play, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
