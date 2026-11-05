/*
 * File: z_en_girla.c
 * Overlay: En_GirlA
 * Description: Shop Items
 */

#include "z_en_girla.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#include <assert.h>

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnGirlA_Init(Actor* thisx, PlayState* play);
static void EnGirlA_Destroy(Actor* thisx, PlayState* play);
static void EnGirlA_Update(Actor* thisx, PlayState* play);

static void EnGirlA_SetItemOutOfStock(PlayState* play, EnGirlA* this);
static void EnGirlA_UpdateStockedItem(PlayState* play, EnGirlA* this);
static void EnGirlA_WaitForObject(EnGirlA* this, PlayState* play);
static void EnGirlA_Update2(EnGirlA* this, PlayState* play);
static void func_80A3C498(Actor* thisx, PlayState* play, s32 flags);
static void EnGirlA_Draw(Actor* thisx, PlayState* play);

static s32 EnGirlA_CanBuy_Arrows(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Bombs(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_DekuNuts(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_DekuSticks(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Fish(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_RedPotion(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_GreenPotion(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_BluePotion(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Longsword(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_HylianShield(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_DekuShield(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_GoronTunic(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_ZoraTunic(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Health(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_MilkBottle(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_WeirdEgg(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Unk19(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Unk20(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Bombchus(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_DekuSeeds(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_SoldOut(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_BlueFire(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Bugs(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Poe(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Fairy(PlayState* play, EnGirlA* this);
static s32 EnGirlA_CanBuy_Randomizer(PlayState* play, EnGirlA* this);

static void EnGirlA_ItemGive_DekuNuts(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Arrows(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Bombs(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_DekuSticks(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_BottledItem(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Longsword(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_HylianShield(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_DekuShield(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_GoronTunic(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_ZoraTunic(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Health(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_MilkBottle(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_WeirdEgg(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Unk19(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Unk20(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_DekuSeeds(PlayState* play, EnGirlA* this);
static void EnGirlA_ItemGive_Randomizer(PlayState* play, EnGirlA* this);
static void EnGirlA_BuyEvent_ShieldDiscount(PlayState* play, EnGirlA* this);
static void EnGirlA_BuyEvent_ObtainBombchuPack(PlayState* play, EnGirlA* this);
static void EnGirlA_BuyEvent_GoronTunic(PlayState* play, EnGirlA* this);
static void EnGirlA_BuyEvent_ZoraTunic(PlayState* play, EnGirlA* this);


ActorProfile En_GirlA_Oot_Profile = {
    /**/ ACTOR_EN_GIRLA_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnGirlA),
    /**/ EnGirlA_Init,
    /**/ EnGirlA_Destroy,
    /**/ EnGirlA_Update,
    /**/ NULL,
};

static char* sShopItemDescriptions[] = {
    "デクの実×5   ",  // "Deku nut x5"
    "矢×30        ",  // "Arrow x30"
    "矢×50        ",  // "Arrow x50"
    "爆弾×5       ",  // "bomb"
    "デクの実×10  ",  // "Deku nut x10"
    "デクの棒      ", // "Deku stick"
    "爆弾×10      ",  // "Bomb x10"
    "さかな        ", // "Fish"
    "赤クスリ      ", // "Red medicine"
    "緑クスリ      ", // "Green medicine"
    "青クスリ      ", // "Blue medicine"
    "巨人のナイフ  ", // "Giant knife"
    "ハイリアの盾  ", // "Hyria Shield"
    "デクの盾      ", // "Deku Shield"
    "ゴロンの服    ", // "Goron's clothes"
    "ゾ─ラの服    ",  // "Zora's clothes"
    "回復のハート  ", // "Heart of recovery"
    "ロンロン牛乳  ", // "Ron Ron milk"
    "鶏の卵        ", // "Chicken egg"
    "インゴー牛乳  ", // "Ingo milk"
    "インゴー卵    ", // "Ingo egg"
    "もだえ石      ", // "Modae stone"
    "大人の財布    ", // "Adult wallet"
    "ハートの欠片  ", // "Heart fragment"
    "ボムチュウ    ", // "Bombchu"
    "ボムチュウ    ", // "Bombchu"
    "ボムチュウ    ", // "Bombchu"
    "ボムチュウ    ", // "Bombchu"
    "ボムチュウ    ", // "Bombchu"
    "デクのタネ    ", // "Deku seeds"
    "キータンのお面", // "Ketan's mask"
    "こわそなお面  ", // "Scary face"
    "ドクロのお面  ", // "Skull mask"
    "ウサギずきん  ", // "Rabbit hood"
    "まことの仮面  ", // "True mask"
    "ゾーラのお面  ", // "Zora's mask"
    "ゴロンのお面  ", // "Goron's mask"
    "ゲルドのお面  ", // "Gerd's mask"
    "ＳＯＬＤＯＵＴ",
    "炎            ", // "Flame"
    "虫            ", // "Bugs"
    "チョウチョ    ", // "Butterfly"
    "ポウ          ", // "Poe"
    "妖精の魂      ", // "Fairy soul"
    "矢×10        ",  // "Arrow"
    "爆弾×20      ",  // "Bomb x20"
    "爆弾×30      ",  // "Bomb x30"
    "爆弾×5       ",  // "Bomb x5"
    "赤クスリ      ", // "Red medicine"
    "赤クスリ      ", // "Red medicine"
    "Random Item  "   // "Random Item"
};

static s16 sMaskShopItems[8] = {
    ITEM_MASK_KEATON_Oot, ITEM_MASK_SPOOKY_Oot, ITEM_MASK_SKULL_Oot, ITEM_MASK_BUNNY_Oot,
    ITEM_MASK_TRUTH_Oot,  ITEM_MASK_ZORA_Oot,   ITEM_MASK_GORON_Oot, ITEM_MASK_GERUDO_Oot,
};

static u16 sMaskShopFreeToBorrowTextIds[5] = { TEXT_OOT(0x70B6), TEXT_OOT(0x70B5), TEXT_OOT(0x70B4), TEXT_OOT(0x70B7), TEXT_OOT(0x70BB) };

typedef struct {
    /* 0x00 */ s16 objID;
    /* 0x02 */ s16 giDrawId;
    /* 0x04 */ void (*hiliteFunc)(Actor*, PlayState*, s32);
    /* 0x08 */ s16 price;
    /* 0x0A */ s16 count;
    /* 0x0C */ u16 itemDescTextId;
    /* 0x0C */ u16 itemBuyPromptTextId;
    /* 0x10 */ s32 getItemId;
    /* 0x14 */ s32 (*canBuyFunc)(PlayState*, EnGirlA*);
    /* 0x18 */ void (*itemGiveFunc)(PlayState*, EnGirlA*);
    /* 0x1C */ void (*buyEventFunc)(PlayState*, EnGirlA*);
} ShopItemEntry; // size = 0x20

static ShopItemEntry shopItemEntries[] = {
    // SI_DEKU_NUTS_5
    { OBJECT_GI_NUTS_Oot, GID_NUTS_Oot, func_800B8118, 15, 5, TEXT_OOT(0x00B2), TEXT_OOT(0x007F), GI_NUTS_5_2_Oot, EnGirlA_CanBuy_DekuNuts,
      EnGirlA_ItemGive_DekuNuts, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_ARROWS_30 */
    { OBJECT_GI_ARROW_Oot, GID_ARROWS_MEDIUM_Oot, func_800B8050, 60, 30, TEXT_OOT(0x00C1), TEXT_OOT(0x009B), GI_ARROWS_MEDIUM_Oot,
      EnGirlA_CanBuy_Arrows, EnGirlA_ItemGive_Arrows, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_ARROWS_50 */
    { OBJECT_GI_ARROW_Oot, GID_ARROWS_LARGE_Oot, func_800B8050, 90, 50, TEXT_OOT(0x00B0), TEXT_OOT(0x007D), GI_ARROWS_LARGE_Oot, EnGirlA_CanBuy_Arrows,
      EnGirlA_ItemGive_Arrows, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBS_5_R25 */
    { OBJECT_GI_BOMB_1_Oot, GID_BOMB_Oot, func_800B8050, 25, 5, TEXT_OOT(0x00A3), TEXT_OOT(0x008B), GI_BOMBS_5_Oot, EnGirlA_CanBuy_Bombs,
      EnGirlA_ItemGive_Bombs, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_DEKU_NUTS_10 */
    { OBJECT_GI_NUTS_Oot, GID_NUTS_Oot, func_800B8118, 30, 10, TEXT_OOT(0x00A2), TEXT_OOT(0x0087), GI_NUTS_10_Oot, EnGirlA_CanBuy_DekuNuts,
      EnGirlA_ItemGive_DekuNuts, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_DEKU_STICK */
    { OBJECT_GI_STICK_Oot, GID_STICK_Oot, NULL, 10, 1, TEXT_OOT(0x00A1), TEXT_OOT(0x0088), GI_STICKS_1_Oot, EnGirlA_CanBuy_DekuSticks,
      EnGirlA_ItemGive_DekuSticks, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBS_10 */
    { OBJECT_GI_BOMB_1_Oot, GID_BOMB_Oot, func_800B8050, 50, 10, TEXT_OOT(0x00B1), TEXT_OOT(0x007C), GI_BOMBS_10_Oot, EnGirlA_CanBuy_Bombs,
      EnGirlA_ItemGive_Bombs, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_FISH */
    { OBJECT_GI_FISH_Oot, GID_FISH_Oot, func_800B8118, 200, 1, TEXT_OOT(0x00B3), TEXT_OOT(0x007E), GI_FISH_Oot, EnGirlA_CanBuy_Fish, NULL,
      EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_RED_POTION_R30 */
    { OBJECT_GI_LIQUID_Oot, GID_POTION_RED_Oot, func_800B8050, 30, 1, TEXT_OOT(0x00A5), TEXT_OOT(0x008E), GI_POTION_RED_Oot, EnGirlA_CanBuy_RedPotion,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_GREEN_POTION */
    { OBJECT_GI_LIQUID_Oot, GID_POTION_GREEN_Oot, func_800B8050, 30, 1, TEXT_OOT(0x00A6), TEXT_OOT(0x008F), GI_POTION_GREEN_Oot,
      EnGirlA_CanBuy_GreenPotion, EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BLUE_POTION */
    { OBJECT_GI_LIQUID_Oot, GID_POTION_BLUE_Oot, func_800B8050, 60, 1, TEXT_OOT(0x00A7), TEXT_OOT(0x0090), GI_POTION_BLUE_Oot,
      EnGirlA_CanBuy_BluePotion, EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_LONGSWORD */
    { OBJECT_GI_LONGSWORD_Oot, GID_SWORD_BGS_Oot, func_800B8050, 1000, 1, TEXT_OOT(0x00A8), TEXT_OOT(0x0091), GI_SWORD_KNIFE_Oot,
      EnGirlA_CanBuy_Longsword, EnGirlA_ItemGive_Longsword, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_HYLIAN_SHIELD */
    { OBJECT_GI_SHIELD_2_Oot, GID_SHIELD_HYLIAN_Oot, func_800B8050, 80, 1, TEXT_OOT(0x00A9), TEXT_OOT(0x0092), GI_SHIELD_HYLIAN_Oot,
      EnGirlA_CanBuy_HylianShield, EnGirlA_ItemGive_HylianShield, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_DEKU_SHIELD */
    { OBJECT_GI_SHIELD_1_Oot, GID_SHIELD_DEKU_Oot, func_800B8050, 40, 1, TEXT_OOT(0x009F), TEXT_OOT(0x0089), GI_SHIELD_DEKU_Oot,
      EnGirlA_CanBuy_DekuShield, EnGirlA_ItemGive_DekuShield, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_GORON_TUNIC */
    { OBJECT_GI_CLOTHES_Oot, GID_TUNIC_GORON_Oot, NULL, 200, 1, TEXT_OOT(0x00AA), TEXT_OOT(0x0093), GI_TUNIC_GORON_Oot, EnGirlA_CanBuy_GoronTunic,
      EnGirlA_ItemGive_GoronTunic, EnGirlA_BuyEvent_GoronTunic },
    /* SI_ZORA_TUNIC */
    { OBJECT_GI_CLOTHES_Oot, GID_TUNIC_ZORA_Oot, NULL, 300, 1, TEXT_OOT(0x00AB), TEXT_OOT(0x0094), GI_TUNIC_ZORA_Oot, EnGirlA_CanBuy_ZoraTunic,
      EnGirlA_ItemGive_ZoraTunic, EnGirlA_BuyEvent_ZoraTunic },
    /* SI_RECOVERY_HEART */
    { OBJECT_GI_HEART_Oot, GID_HEART_Oot, NULL, 10, 16, TEXT_OOT(0x00AC), TEXT_OOT(0x0095), GI_HEART_Oot, EnGirlA_CanBuy_Health,
      EnGirlA_ItemGive_Health, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_MILK_BOTTLE */
    { OBJECT_GI_MILK_Oot, GID_MILK_Oot, func_80A3C498, 100, 1, TEXT_OOT(0x00AD), TEXT_OOT(0x0097), GI_MILK_BOTTLE_Oot, EnGirlA_CanBuy_MilkBottle,
      EnGirlA_ItemGive_MilkBottle, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_WEIRD_EGG */
    { OBJECT_GI_EGG_Oot, GID_EGG_Oot, func_800B8050, 100, 1, TEXT_OOT(0x00AE), TEXT_OOT(0x0099), GI_WEIRD_EGG_Oot, EnGirlA_CanBuy_WeirdEgg,
      EnGirlA_ItemGive_WeirdEgg, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_19 */
    { OBJECT_GI_MILK_Oot, GID_MILK_Oot, func_80A3C498, 10000, 1, TEXT_OOT(0x00B4), TEXT_OOT(0x0085), GI_NONE, EnGirlA_CanBuy_Unk19,
      EnGirlA_ItemGive_Unk19, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_20 */
    { OBJECT_GI_EGG_Oot, GID_EGG_Oot, func_800B8050, 10000, 1, TEXT_OOT(0x00B5), TEXT_OOT(0x0085), GI_NONE, EnGirlA_CanBuy_Unk20,
      EnGirlA_ItemGive_Unk20, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBCHU_10_1 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 100, 10, TEXT_OOT(0x00BC), TEXT_OOT(0x008C), GI_BOMBCHUS_10_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_20_1 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 180, 20, TEXT_OOT(0x0061), TEXT_OOT(0x002A), GI_BOMBCHUS_20_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_20_2 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 180, 20, TEXT_OOT(0x0061), TEXT_OOT(0x002A), GI_BOMBCHUS_20_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_10_2 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 100, 10, TEXT_OOT(0x00BC), TEXT_OOT(0x008C), GI_BOMBCHUS_10_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_10_3 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 100, 10, TEXT_OOT(0x00BC), TEXT_OOT(0x008C), GI_BOMBCHUS_10_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_20_3 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 180, 20, TEXT_OOT(0x0061), TEXT_OOT(0x002A), GI_BOMBCHUS_20_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_20_4 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 180, 20, TEXT_OOT(0x0061), TEXT_OOT(0x002A), GI_BOMBCHUS_20_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_BOMBCHU_10_4 */
    { OBJECT_GI_BOMB_2_Oot, GID_BOMBCHU_Oot, func_800B8050, 100, 10, TEXT_OOT(0x00BC), TEXT_OOT(0x008C), GI_BOMBCHUS_10_Oot, EnGirlA_CanBuy_Bombchus,
      NULL, EnGirlA_BuyEvent_ObtainBombchuPack },
    /* SI_DEKU_SEEDS_30 */
    { OBJECT_GI_SEED_Oot, GID_SEEDS_Oot, func_800B8050, 30, 30, TEXT_OOT(0x00DF), TEXT_OOT(0x00DE), GI_SEEDS_30_Oot, EnGirlA_CanBuy_DekuSeeds,
      EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_KEATON_MASK */
    { OBJECT_GI_KI_TAN_MASK_Oot, GID_MASK_KEATON_Oot, func_800B8050, 0, 1, TEXT_OOT(0x70B2), TEXT_OOT(0x70BE), GI_MASK_KEATON_Oot,
      EnGirlA_CanBuy_DekuSeeds, EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_SPOOKY_MASK */
    { OBJECT_GI_REDEAD_MASK_Oot, GID_MASK_SPOOKY_Oot, func_800B8050, 0, 1, TEXT_OOT(0x70B1), TEXT_OOT(0x70BD), GI_MASK_SPOOKY_Oot,
      EnGirlA_CanBuy_DekuSeeds, EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_SKULL_MASK */
    { OBJECT_GI_SKJ_MASK_Oot, GID_MASK_SKULL_Oot, func_800B8050, 0, 1, TEXT_OOT(0x70B0), TEXT_OOT(0x70BC), GI_MASK_SKULL_Oot, EnGirlA_CanBuy_DekuSeeds,
      EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BUNNY_HOOD */
    { OBJECT_GI_RABIT_MASK_Oot, GID_MASK_BUNNY_Oot, func_800B8050, 0, 1, TEXT_OOT(0x70B3), TEXT_OOT(0x70BF), GI_MASK_BUNNY_Oot,
      EnGirlA_CanBuy_DekuSeeds, EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_MASK_OF_TRUTH */
    { OBJECT_GI_TRUTH_MASK_Oot, GID_MASK_TRUTH_Oot, func_80A3C498, 0, 1, TEXT_OOT(0x70AF), TEXT_OOT(0x70C3), GI_MASK_TRUTH_Oot,
      EnGirlA_CanBuy_DekuSeeds, EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_ZORA_MASK */
    { OBJECT_GI_ZORAMASK_Oot, GID_MASK_ZORA_Oot, NULL, 0, 1, TEXT_OOT(0x70B9), TEXT_OOT(0x70C1), GI_MASK_ZORA_Oot, EnGirlA_CanBuy_DekuSeeds,
      EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_GORON_MASK */
    { OBJECT_GI_GOLONMASK_Oot, GID_MASK_GORON_Oot, NULL, 0, 1, TEXT_OOT(0x70B8), TEXT_OOT(0x70C0), GI_MASK_GORON_Oot, EnGirlA_CanBuy_DekuSeeds,
      EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_GERUDO_MASK */
    { OBJECT_GI_GERUDOMASK_Oot, GID_MASK_GERUDO_Oot, NULL, 0, 1, TEXT_OOT(0x70BA), TEXT_OOT(0x70C2), GI_MASK_GERUDO_Oot, EnGirlA_CanBuy_DekuSeeds,
      EnGirlA_ItemGive_DekuSeeds, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_SOLD_OUT */
    { OBJECT_GI_SOLDOUT_Oot, GID_SOLDOUT_Oot, func_800B8050, 0, 0, TEXT_OOT(0x00BD), TEXT_OOT(0x70C2), GI_MASK_GERUDO_Oot, EnGirlA_CanBuy_SoldOut, NULL,
      NULL },
    /* SI_BLUE_FIRE */
    { OBJECT_GI_FIRE_Oot, GID_BLUE_FIRE_Oot, func_800B8050, 300, 1, TEXT_OOT(0x00B9), TEXT_OOT(0x00B8), GI_BLUE_FIRE_Oot, EnGirlA_CanBuy_BlueFire,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BUGS */
    { OBJECT_GI_INSECT_Oot, GID_BUG_Oot, func_80A3C498, 50, 1, TEXT_OOT(0x00BB), TEXT_OOT(0x00BA), GI_BUGS_Oot, EnGirlA_CanBuy_Bugs,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BIG_POE */
    { OBJECT_GI_GHOST_Oot, GID_BIG_POE_Oot, func_80A3C498, 50, 1, TEXT_OOT(0x506F), TEXT_OOT(0x5070), GI_BIG_POE_Oot, EnGirlA_CanBuy_Poe,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_POE */
    { OBJECT_GI_GHOST_Oot, GID_POE_Oot, func_80A3C498, 30, 1, TEXT_OOT(0x506D), TEXT_OOT(0x506E), GI_POE_Oot, EnGirlA_CanBuy_Poe,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_FAIRY */
    { OBJECT_GI_SOUL_Oot, GID_FAIRY_Oot, func_80A3C498, 50, 1, TEXT_OOT(0x00B7), TEXT_OOT(0x00B6), GI_FAIRY_Oot, EnGirlA_CanBuy_Fairy,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_ARROWS_10 */
    { OBJECT_GI_ARROW_Oot, GID_ARROWS_SMALL_Oot, func_800B8050, 20, 10, TEXT_OOT(0x00A0), TEXT_OOT(0x008A), GI_ARROWS_SMALL_Oot, EnGirlA_CanBuy_Arrows,
      EnGirlA_ItemGive_Arrows, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBS_20 */
    { OBJECT_GI_BOMB_1_Oot, GID_BOMB_Oot, func_800B8050, 80, 20, TEXT_OOT(0x001C), TEXT_OOT(0x0006), GI_BOMBS_20_Oot, EnGirlA_CanBuy_Bombs,
      EnGirlA_ItemGive_Bombs, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBS_30 */
    { OBJECT_GI_BOMB_1_Oot, GID_BOMB_Oot, func_800B8050, 120, 30, TEXT_OOT(0x001D), TEXT_OOT(0x001E), GI_BOMBS_30_Oot, EnGirlA_CanBuy_Bombs,
      EnGirlA_ItemGive_Bombs, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_BOMBS_5_R35 */
    { OBJECT_GI_BOMB_1_Oot, GID_BOMB_Oot, func_800B8050, 35, 5, TEXT_OOT(0x00CB), TEXT_OOT(0x00CA), GI_BOMBS_5_Oot, EnGirlA_CanBuy_Bombs,
      EnGirlA_ItemGive_Bombs, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_RED_POTION_R40 */
    { OBJECT_GI_LIQUID_Oot, GID_POTION_RED_Oot, func_800B8050, 40, 1, TEXT_OOT(0x0064), TEXT_OOT(0x0062), GI_POTION_RED_Oot, EnGirlA_CanBuy_RedPotion,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_RED_POTION_R50 */
    { OBJECT_GI_LIQUID_Oot, GID_POTION_RED_Oot, func_800B8050, 50, 1, TEXT_OOT(0x0065), TEXT_OOT(0x0063), GI_POTION_RED_Oot, EnGirlA_CanBuy_RedPotion,
      EnGirlA_ItemGive_BottledItem, EnGirlA_BuyEvent_ShieldDiscount },
    /* SI_RANDOMIZED_ITEM */
    { OBJECT_INVALID_Oot, GID_NONE, NULL, 40, 1, TEXT_OOT(0x9100), TEXT_OOT(0x9100), GI_NONE, EnGirlA_CanBuy_Randomizer,
      EnGirlA_ItemGive_Randomizer, NULL }
};

// Defines the Hylian Shield discount amount
static s16 sShieldDiscounts[] = { 5, 10, 15, 20, 25, 30, 35, 40 };

static void EnGirlA_SetupAction(EnGirlA* this, EnGirlAActionFunc func) {
    this->actionFunc = func;
}

static s32 EnGirlA_TryChangeShopItem(EnGirlA* this) {
    switch (this->actor.params) {
        case SI_MILK_BOTTLE:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_TALON_BOTTLE_Oot)) {
                this->actor.params = SI_RECOVERY_HEART;
                return true;
            }
            break;
        case SI_BOMBCHU_10_2:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_06_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_10_3:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_07_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_20_3:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_08_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_20_4:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_09_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_10_4:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_0A_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_10_1:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_03_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_20_1:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_04_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
        case SI_BOMBCHU_20_2:
            if (GET_ITEMGETINF_OOT(ITEMGETINF_05_Oot)) {
                this->actor.params = SI_SOLD_OUT;
                return true;
            }
            break;
    }
    return false;
}

static void EnGirlA_InitItem(EnGirlA* this, PlayState* play) {
    s16 params = this->actor.params;


    if ((params >= SI_MAX) && (params < 0)) {
        Actor_Kill(&this->actor);
        assert((params >= SI_MAX) && (params < 0));
        // ASSERT(0, "0", "../z_en_girlA.c", 1421); zret assert, currently errors due to missing macro
        return;
    }

    {
        this->requiredObjectSlot = Object_GetSlot(&play->objectCtx, shopItemEntries[params].objID);
    }

    if (this->requiredObjectSlot < 0) {
        Actor_Kill(&this->actor);
        // ASSERT(0, "0", "../z_en_girlA.c", 1434); zret assert, currently errors due to missing macro
        return;
    }

    this->actor.params = params;
    this->actionFunc2 = EnGirlA_WaitForObject;
}

static void EnGirlA_Init(Actor* thisx, PlayState* play) {
    {
        EnGirlA* this = (EnGirlA*)thisx;

        EnGirlA_TryChangeShopItem(this);
        EnGirlA_InitItem(this, play);
    }
}

static void EnGirlA_Destroy(Actor* thisx, PlayState* play) {
}

static s32 EnGirlA_CanBuy_Arrows(PlayState* play, EnGirlA* this) {
    if (Item_CheckObtainability(ITEM_BOW_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_CANT_GET_NOW_5;
    }
    if (AMMO(ITEM_BOW_Oot) >= CUR_CAPACITY(UPG_QUIVER)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Bombs(PlayState* play, EnGirlA* this) {
    if (!CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (AMMO(ITEM_BOMB_Oot) >= CUR_CAPACITY(UPG_BOMB_BAG)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_DekuNuts(PlayState* play, EnGirlA* this) {

    if ((CUR_CAPACITY(UPG_DEKU_NUTS) != 0) && (AMMO(ITEM_NUT_Oot) >= CUR_CAPACITY(UPG_DEKU_NUTS))) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if ((Item_CheckObtainability(ITEM_NUT_Oot) == ITEM_NONE) && !0) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_DekuSticks(PlayState* play, EnGirlA* this) {

    if ((CUR_CAPACITY(UPG_DEKU_STICKS) != 0) && (AMMO(ITEM_STICK_Oot) >= CUR_CAPACITY(UPG_DEKU_STICKS))) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if ((Item_CheckObtainability(ITEM_STICK_Oot) == ITEM_NONE) && !0) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Fish(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_FISH_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_RedPotion(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_POTION_RED_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_GreenPotion(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_POTION_GREEN_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_BluePotion(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_POTION_BLUE_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Longsword(PlayState* play, EnGirlA* this) {
    if (CHECK_OWNED_EQUIP_OOT(0, EQUIP_OWNED_SWORD_BIGGORON) &&
        !CHECK_OWNED_EQUIP_OOT(0, EQUIP_OWNED_SWORD_KNIFE_BROKEN)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_SWORD_BGS_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_HylianShield(PlayState* play, EnGirlA* this) {
    s32 canBuy;
    if ((false)) {
        return canBuy;
    }
    if (CHECK_OWNED_EQUIP_OOT(1, EQUIP_OWNED_SHIELD_HYLIAN)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_SHIELD_HYLIAN_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_DekuShield(PlayState* play, EnGirlA* this) {
    s32 canBuy;
    if ((false)) {
        return canBuy;
    }
    if (CHECK_OWNED_EQUIP_OOT(1, EQUIP_OWNED_SHIELD_DEKU)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_SHIELD_DEKU_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_GoronTunic(PlayState* play, EnGirlA* this) {
    s32 canBuy;
    if ((false)) {
        return canBuy;
    }
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (CHECK_OWNED_EQUIP_OOT(2, EQUIP_OWNED_GORON_IRON)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_TUNIC_GORON_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_ZoraTunic(PlayState* play, EnGirlA* this) {
    s32 canBuy;
    if ((false)) {
        return canBuy;
    }
    if (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (CHECK_OWNED_EQUIP_OOT(2, EQUIP_OWNED_ZORA_HOVER)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_TUNIC_ZORA_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Health(PlayState* play, EnGirlA* this) {
    if (gSaveContext.save.saveInfo.playerData.healthCapacity == gSaveContext.save.saveInfo.playerData.health) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_MilkBottle(PlayState* play, EnGirlA* this) {
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_MILK_BOTTLE_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_WeirdEgg(PlayState* play, EnGirlA* this) {
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_LETTER_ZELDA_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Unk19(PlayState* play, EnGirlA* this) {
    return CANBUY_RESULT_NEED_RUPEES;
}

static s32 EnGirlA_CanBuy_Unk20(PlayState* play, EnGirlA* this) {
    return CANBUY_RESULT_NEED_RUPEES;
}

static s32 EnGirlA_CanBuy_Bombchus(PlayState* play, EnGirlA* this) {
    s32 canBuy;
    if ((false)) {
        return canBuy;
    }

    if (AMMO(ITEM_BOMBCHU_Oot) >= 50) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_BOMBCHU_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_DekuSeeds(PlayState* play, EnGirlA* this) {
    if (AMMO(ITEM_SLINGSHOT_Oot) >= CUR_CAPACITY(UPG_BULLET_BAG)) {
        return CANBUY_RESULT_CANT_GET_NOW;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if ((Item_CheckObtainability(ITEM_SEEDS_Oot) == ITEM_NONE) && !0) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_SoldOut(PlayState* play, EnGirlA* this) {
    return CANBUY_RESULT_CANT_GET_NOW_5;
}

static s32 EnGirlA_CanBuy_BlueFire(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_BLUE_FIRE_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Bugs(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_BUG_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Poe(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_POE_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Fairy(PlayState* play, EnGirlA* this) {
    if (!Inventory_HasEmptyBottle()) {
        return CANBUY_RESULT_NEED_BOTTLE;
    }
    if (gSaveContext.save.saveInfo.playerData.rupees < this->basePrice) {
        return CANBUY_RESULT_NEED_RUPEES;
    }
    if (Item_CheckObtainability(ITEM_FAIRY_Oot) == ITEM_NONE) {
        return CANBUY_RESULT_SUCCESS_FANFARE;
    }
    return CANBUY_RESULT_SUCCESS;
}

static s32 EnGirlA_CanBuy_Randomizer(PlayState* play, EnGirlA* this) {
    return CANBUY_RESULT_CANT_GET_NOW;
}

static void EnGirlA_ItemGive_Arrows(PlayState* play, EnGirlA* this) {
    Inventory_ChangeAmmo(ITEM_BOW_Oot, this->itemCount);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_Bombs(PlayState* play, EnGirlA* this) {
    switch (this->itemCount) {
        case 5:
            Item_Give(play, ITEM_BOMBS_5_Oot);
            break;
        case 10:
            Item_Give(play, ITEM_BOMBS_10_Oot);
            break;
        case 20:
            Item_Give(play, ITEM_BOMBS_20_Oot);
            break;
        case 30:
            Item_Give(play, ITEM_BOMBS_30_Oot);
            break;
    }
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_DekuNuts(PlayState* play, EnGirlA* this) {
    switch (this->itemCount) {
        case 5:
            Item_Give(play, ITEM_NUTS_5_Oot);
            break;
        case 10:
            Item_Give(play, ITEM_NUTS_10_Oot);
            break;
    }
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_DekuSticks(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_STICK_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_Longsword(PlayState* play, EnGirlA* this) {
    Hyrule_GiveGiantsKnife();
    gSaveContext.save.saveInfo.inventoryOot.knifeHealth = 8;
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_HylianShield(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_SHIELD_HYLIAN_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_DekuShield(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_SHIELD_DEKU_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_GoronTunic(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_TUNIC_GORON_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_ZoraTunic(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_TUNIC_ZORA_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_Health(PlayState* play, EnGirlA* this) {
    Health_ChangeBy(play, this->itemCount);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_MilkBottle(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_MILK_BOTTLE_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_WeirdEgg(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_WEIRD_EGG_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_Unk19(PlayState* play, EnGirlA* this) {
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_Unk20(PlayState* play, EnGirlA* this) {
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_DekuSeeds(PlayState* play, EnGirlA* this) {
    Item_Give(play, ITEM_SEEDS_30_Oot);
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_ItemGive_BottledItem(PlayState* play, EnGirlA* this) {
    switch (this->actor.params) {
        case SI_FISH:
            Item_Give(play, ITEM_FISH_Oot);
            break;
        case SI_RED_POTION_R30:
            Item_Give(play, ITEM_POTION_RED_Oot);
            break;
        case SI_GREEN_POTION:
            Item_Give(play, ITEM_POTION_GREEN_Oot);
            break;
        case SI_BLUE_POTION:
            Item_Give(play, ITEM_POTION_BLUE_Oot);
            break;
        case SI_BLUE_FIRE:
            Item_Give(play, ITEM_BLUE_FIRE_Oot);
            break;
        case SI_BUGS:
            Item_Give(play, ITEM_BUG_Oot);
            break;
        case SI_BIG_POE:
            Item_Give(play, ITEM_BIG_POE_Oot);
            break;
        case SI_POE:
            Item_Give(play, ITEM_POE_Oot);
            break;
        case SI_FAIRY:
            Item_Give(play, ITEM_FAIRY_Oot);
            break;
    }
    Rupees_ChangeBy(-this->basePrice);
}

// This is called when EnGirlA_CanBuy_Randomizer returns CANBUY_RESULT_SUCCESS
// The giving of the item is handled here, and no fanfare is played
static void EnGirlA_ItemGive_Randomizer(PlayState* play, EnGirlA* this) {
}

static void EnGirlA_BuyEvent_ShieldDiscount(PlayState* play, EnGirlA* this) {
    if (this->actor.params == SI_HYLIAN_SHIELD) {
        if (GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) {
            Rupees_ChangeBy(-(this->basePrice - sShieldDiscounts[(s32)Rand_ZeroFloat(7.9f)]));
            return;
        }
    }
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_BuyEvent_GoronTunic(PlayState* play, EnGirlA* this) {
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_BuyEvent_ZoraTunic(PlayState* play, EnGirlA* this) {
    Rupees_ChangeBy(-this->basePrice);
}

static void EnGirlA_BuyEvent_ObtainBombchuPack(PlayState* play, EnGirlA* this) {
    Rupees_ChangeBy(-this->basePrice);

    // Normally, buying a bombchu pack sets a flag indicating the pack is now sold out
    // If they're in logic for rando, skip setting that flag so they can be purchased repeatedly

    switch (this->actor.params) {
        case SI_BOMBCHU_10_2:
            SET_ITEMGETINF_OOT(ITEMGETINF_06_Oot);
            break;
        case SI_BOMBCHU_10_3:
            SET_ITEMGETINF_OOT(ITEMGETINF_07_Oot);
            break;
        case SI_BOMBCHU_20_3:
            SET_ITEMGETINF_OOT(ITEMGETINF_08_Oot);
            break;
        case SI_BOMBCHU_20_4:
            SET_ITEMGETINF_OOT(ITEMGETINF_09_Oot);
            break;
        case SI_BOMBCHU_10_4:
            SET_ITEMGETINF_OOT(ITEMGETINF_0A_Oot);
            break;
        case SI_BOMBCHU_10_1:
            SET_ITEMGETINF_OOT(ITEMGETINF_03_Oot);
            break;
        case SI_BOMBCHU_20_1:
            SET_ITEMGETINF_OOT(ITEMGETINF_04_Oot);
            break;
        case SI_BOMBCHU_20_2:
            SET_ITEMGETINF_OOT(ITEMGETINF_05_Oot);
            break;
    }
}

static void EnGirlA_Noop(EnGirlA* this, PlayState* play) {
}

static void EnGirlA_SetItemDescription(PlayState* play, EnGirlA* this) {
    ShopItemEntry* tmp = &shopItemEntries[this->actor.params];
    s32 params = this->actor.params;
    s32 maskId;
    s32 isMaskFreeToBorrow;

    if ((this->actor.params >= SI_KEATON_MASK) && (this->actor.params <= SI_MASK_OF_TRUTH)) {
        maskId = this->actor.params - SI_KEATON_MASK;
        isMaskFreeToBorrow = false;
        switch (this->actor.params) {
            case SI_KEATON_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) {
                    isMaskFreeToBorrow = true;
                }
                break;
            case SI_SPOOKY_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot)) {
                    isMaskFreeToBorrow = true;
                }
                break;
            case SI_SKULL_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot)) {
                    isMaskFreeToBorrow = true;
                }
                break;
            case SI_BUNNY_HOOD:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                    isMaskFreeToBorrow = true;
                }
                break;
            case SI_MASK_OF_TRUTH:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                    isMaskFreeToBorrow = true;
                }
                break;
        }
        if (isMaskFreeToBorrow) {
            this->actor.textId = sMaskShopFreeToBorrowTextIds[maskId];
        } else {
            this->actor.textId = tmp->itemDescTextId;
        }
    } else {
        this->actor.textId = tmp->itemDescTextId;
    }


    this->isInvisible = false;
    this->actor.draw = EnGirlA_Draw;
}

static void EnGirlA_SetItemOutOfStock(PlayState* play, EnGirlA* this) {
    this->isInvisible = true;
    this->actor.draw = NULL;
    if (((this->actor.params >= SI_KEATON_MASK) && (this->actor.params <= SI_GERUDO_MASK))) {
        this->actor.textId = TEXT_OOT(0xBD);
    }
}

static void EnGirlA_UpdateStockedItem(PlayState* play, EnGirlA* this) {
    {
        ShopItemEntry* itemEntry;
        if (EnGirlA_TryChangeShopItem(this)) {
            EnGirlA_InitItem(this, play);
            itemEntry = &shopItemEntries[this->actor.params];
            this->actor.textId = itemEntry->itemDescTextId;
        } else {
            this->isInvisible = false;
            this->actor.draw = EnGirlA_Draw;
        }
    }
}

static s32 EnGirlA_TrySetMaskItemDescription(EnGirlA* this, PlayState* play) {
    s32 params;

    if ((this->actor.params >= SI_KEATON_MASK) && (this->actor.params <= SI_GERUDO_MASK)) {
        params = this->actor.params - SI_KEATON_MASK;
        if (INV_CONTENT_OOT(ITEM_TRADE_CHILD_Oot) == sMaskShopItems[params]) {
            EnGirlA_SetItemOutOfStock(play, this);
        } else {
            EnGirlA_SetItemDescription(play, this);
        }
        return true;
    }
    return false;
}

static void EnGirlA_WaitForObject(EnGirlA* this, PlayState* play) {
    s16 params = this->actor.params;
    ShopItemEntry* itemEntry = &shopItemEntries[params];

    if (Object_IsLoaded(&play->objectCtx, this->requiredObjectSlot)) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->actor.objectSlot = this->requiredObjectSlot;
        switch (this->actor.params) {
            case SI_KEATON_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_38_Oot)) {
                    this->actor.textId = TEXT_OOT(0x70B6);
                } else {
                    this->actor.textId = itemEntry->itemDescTextId;
                }
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_SPOOKY_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3A_Oot)) {
                    this->actor.textId = TEXT_OOT(0x70B5);
                } else {
                    this->actor.textId = itemEntry->itemDescTextId;
                }
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_SKULL_MASK:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_39_Oot)) {
                    this->actor.textId = TEXT_OOT(0x70B4);
                } else {
                    this->actor.textId = itemEntry->itemDescTextId;
                }
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_BUNNY_HOOD:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                    this->actor.textId = TEXT_OOT(0x70B7);
                } else {
                    this->actor.textId = itemEntry->itemDescTextId;
                }
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_MASK_OF_TRUTH:
                if (GET_ITEMGETINF_OOT(ITEMGETINF_3B_Oot)) {
                    this->actor.textId = TEXT_OOT(0x70BB);
                    this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                } else {
                    this->actor.textId = itemEntry->itemDescTextId;
                    this->itemBuyPromptTextId = TEXT_OOT(0xEB);
                }
                break;
            case SI_ZORA_MASK:
                this->actor.textId = itemEntry->itemDescTextId;
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_GORON_MASK:
                this->actor.textId = itemEntry->itemDescTextId;
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            case SI_GERUDO_MASK:
                this->actor.textId = itemEntry->itemDescTextId;
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
            default:
                this->actor.textId = itemEntry->itemDescTextId;
                this->itemBuyPromptTextId = itemEntry->itemBuyPromptTextId;
                break;
        }
        if (!EnGirlA_TrySetMaskItemDescription(this, play)) {
            EnGirlA_SetItemDescription(play, this);
        }
        {
            this->setOutOfStockFunc = EnGirlA_SetItemOutOfStock;
            this->updateStockedItemFunc = EnGirlA_UpdateStockedItem;
            this->getItemId = itemEntry->getItemId;
            this->canBuyFunc = itemEntry->canBuyFunc;
            this->itemGiveFunc = itemEntry->itemGiveFunc;
            this->buyEventFunc = itemEntry->buyEventFunc;
            this->basePrice = itemEntry->price;
            this->itemCount = itemEntry->count;
            this->hiliteFunc = itemEntry->hiliteFunc;
            this->giDrawId = itemEntry->giDrawId;
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            Actor_SetScale(&this->actor, 0.25f);
            this->actor.shape.yOffset = 24.0f;
            this->actor.shape.shadowScale = 4.0f;
            this->actor.floorHeight = this->actor.home.pos.y;
            this->actor.gravity = 0.0f;
            EnGirlA_SetupAction(this, EnGirlA_Noop);
            this->isInitialized = true;
            this->actionFunc2 = EnGirlA_Update2;
            this->isSelected = false;
            this->yRotation = 0;
            this->yRotationInit = this->actor.shape.rot.y;
        }
    }
}

static void EnGirlA_Update2(EnGirlA* this, PlayState* play) {
    Actor_SetScale(&this->actor, 0.25f);
    this->actor.shape.yOffset = 24.0f;
    this->actor.shape.shadowScale = 4.0f;
    EnGirlA_TrySetMaskItemDescription(this, play);
    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, 5.0f);
    this->actor.shape.rot.x = 0.0f;
    if (this->actor.params != SI_SOLD_OUT) {
        if (this->isSelected) {
            this->yRotation += 0x1F4;
        } else {
            Math_SmoothStepToS(&this->yRotation, 0, 10, 0x7D0, 0);
        }
    }
}

static void EnGirlA_Update(Actor* thisx, PlayState* play) {
    EnGirlA* this = (EnGirlA*)thisx;

    this->actionFunc2(this, play);
}

static void func_80A3C498(Actor* thisx, PlayState* play, s32 flags) {
    func_800B8050(thisx, play, 0);
    func_800B8118(thisx, play, 0);
}

static void EnGirlA_Draw(Actor* thisx, PlayState* play) {
    EnGirlA* this = (EnGirlA*)thisx;

    Matrix_RotateYF(((this->yRotation * 360.0f) / 65536.0f) * (M_PI / 180.0f), MTXMODE_APPLY);
    if (this->hiliteFunc != NULL) {
        this->hiliteFunc(thisx, play, 0);
    }


    GetItem_Draw(play, this->giDrawId);
}
