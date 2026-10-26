// OoT's get-item models (docs/OOT_ITEMS.md): OoT's z_draw.c, ported from SoH by
// tools/porting/port_oot_get_item_draw.py. Do not edit by hand.

#include "global.h"
#include "HyruleIds.h"
#include "assets/oot/objects/object_gi_key/object_gi_key.h"
#include "assets/oot/objects/object_gi_jewel/object_gi_jewel.h"
#include "assets/oot/objects/object_gi_melody/object_gi_melody.h"
#include "assets/oot/objects/object_gi_heart/object_gi_heart.h"
#include "assets/oot/objects/object_gi_compass/object_gi_compass.h"
#include "assets/oot/objects/object_gi_bosskey/object_gi_bosskey.h"
#include "assets/oot/objects/object_gi_medal/object_gi_medal.h"
#include "assets/oot/objects/object_gi_nuts/object_gi_nuts.h"
#include "assets/oot/objects/object_gi_hearts/object_gi_hearts.h"
#include "assets/oot/objects/object_gi_arrowcase/object_gi_arrowcase.h"
#include "assets/oot/objects/object_gi_bombpouch/object_gi_bombpouch.h"
#include "assets/oot/objects/object_gi_bottle/object_gi_bottle.h"
#include "assets/oot/objects/object_gi_stick/object_gi_stick.h"
#include "assets/oot/objects/object_gi_map/object_gi_map.h"
#include "assets/oot/objects/object_gi_shield_1/object_gi_shield_1.h"
#include "assets/oot/objects/object_gi_magicpot/object_gi_magicpot.h"
#include "assets/oot/objects/object_gi_bomb_1/object_gi_bomb_1.h"
#include "assets/oot/objects/object_gi_purse/object_gi_purse.h"
#include "assets/oot/objects/object_gi_gerudo/object_gi_gerudo.h"
#include "assets/oot/objects/object_gi_arrow/object_gi_arrow.h"
#include "assets/oot/objects/object_gi_bomb_2/object_gi_bomb_2.h"
#include "assets/oot/objects/object_gi_egg/object_gi_egg.h"
#include "assets/oot/objects/object_gi_scale/object_gi_scale.h"
#include "assets/oot/objects/object_gi_shield_2/object_gi_shield_2.h"
#include "assets/oot/objects/object_gi_hookshot/object_gi_hookshot.h"
#include "assets/oot/objects/object_gi_ocarina/object_gi_ocarina.h"
#include "assets/oot/objects/object_gi_milk/object_gi_milk.h"
#include "assets/oot/objects/object_gi_pachinko/object_gi_pachinko.h"
#include "assets/oot/objects/object_gi_boomerang/object_gi_boomerang.h"
#include "assets/oot/objects/object_gi_bow/object_gi_bow.h"
#include "assets/oot/objects/object_gi_glasses/object_gi_glasses.h"
#include "assets/oot/objects/object_gi_liquid/object_gi_liquid.h"
#include "assets/oot/objects/object_gi_shield_3/object_gi_shield_3.h"
#include "assets/oot/objects/object_gi_letter/object_gi_letter.h"
#include "assets/oot/objects/object_gi_clothes/object_gi_clothes.h"
#include "assets/oot/objects/object_gi_bean/object_gi_bean.h"
#include "assets/oot/objects/object_gi_fish/object_gi_fish.h"
#include "assets/oot/objects/object_gi_saw/object_gi_saw.h"
#include "assets/oot/objects/object_gi_hammer/object_gi_hammer.h"
#include "assets/oot/objects/object_gi_grass/object_gi_grass.h"
#include "assets/oot/objects/object_gi_longsword/object_gi_longsword.h"
#include "assets/oot/objects/object_gi_niwatori/object_gi_niwatori.h"
#include "assets/oot/objects/object_gi_bottle_letter/object_gi_bottle_letter.h"
#include "assets/oot/objects/object_gi_ocarina_0/object_gi_ocarina_0.h"
#include "assets/oot/objects/object_gi_boots_2/object_gi_boots_2.h"
#include "assets/oot/objects/object_gi_seed/object_gi_seed.h"
#include "assets/oot/objects/object_gi_gloves/object_gi_gloves.h"
#include "assets/oot/objects/object_gi_coin/object_gi_coin.h"
#include "assets/oot/objects/object_gi_ki_tan_mask/object_gi_ki_tan_mask.h"
#include "assets/oot/objects/object_gi_redead_mask/object_gi_redead_mask.h"
#include "assets/oot/objects/object_gi_skj_mask/object_gi_skj_mask.h"
#include "assets/oot/objects/object_gi_rabit_mask/object_gi_rabit_mask.h"
#include "assets/oot/objects/object_gi_truth_mask/object_gi_truth_mask.h"
#include "assets/oot/objects/object_gi_eye_lotion/object_gi_eye_lotion.h"
#include "assets/oot/objects/object_gi_powder/object_gi_powder.h"
#include "assets/oot/objects/object_gi_mushroom/object_gi_mushroom.h"
#include "assets/oot/objects/object_gi_ticketstone/object_gi_ticketstone.h"
#include "assets/oot/objects/object_gi_brokensword/object_gi_brokensword.h"
#include "assets/oot/objects/object_gi_prescription/object_gi_prescription.h"
#include "assets/oot/objects/object_gi_bracelet/object_gi_bracelet.h"
#include "assets/oot/objects/object_gi_soldout/object_gi_soldout.h"
#include "assets/oot/objects/object_gi_frog/object_gi_frog.h"
#include "assets/oot/objects/object_gi_golonmask/object_gi_golonmask.h"
#include "assets/oot/objects/object_gi_zoramask/object_gi_zoramask.h"
#include "assets/oot/objects/object_gi_gerudomask/object_gi_gerudomask.h"
#include "assets/oot/objects/object_gi_hoverboots/object_gi_hoverboots.h"
#include "assets/oot/objects/object_gi_m_arrow/object_gi_m_arrow.h"
#include "assets/oot/objects/object_gi_sutaru/object_gi_sutaru.h"
#include "assets/oot/objects/object_gi_goddess/object_gi_goddess.h"
#include "assets/oot/objects/object_gi_fire/object_gi_fire.h"
#include "assets/oot/objects/object_gi_insect/object_gi_insect.h"
#include "assets/oot/objects/object_gi_butterfly/object_gi_butterfly.h"
#include "assets/oot/objects/object_gi_ghost/object_gi_ghost.h"
#include "assets/oot/objects/object_gi_soul/object_gi_soul.h"
#include "assets/oot/objects/object_gi_dekupouch/object_gi_dekupouch.h"
#include "assets/oot/objects/object_gi_rupy/object_gi_rupy.h"
#include "assets/oot/objects/object_gi_sword_1/object_gi_sword_1.h"
#include "assets/oot/objects/object_fish/object_fish.h"
#include "assets/oot/objects/object_st/object_st.h"


// "Get Item" Model Draw Functions
static void GetItemOot_DrawNothing(PlayState* play, s16 drawId);
static void GetItemOot_DrawMaskOrBombchu(PlayState* play, s16 drawId);
static void GetItemOot_DrawSoldOut(PlayState* play, s16 drawId);
static void GetItemOot_DrawBlueFire(PlayState* play, s16 drawId);
static void GetItemOot_DrawPoes(PlayState* play, s16 drawId);
static void GetItemOot_DrawFairy(PlayState* play, s16 drawId);
static void GetItemOot_DrawMirrorShield(PlayState* play, s16 drawId);
static void GetItemOot_DrawSkullToken(PlayState* play, s16 drawId);
static void GetItemOot_DrawEggOrMedallion(PlayState* play, s16 drawId);
static void GetItemOot_DrawCompass(PlayState* play, s16 drawId);
static void GetItemOot_DrawPotion(PlayState* play, s16 drawId);
static void GetItemOot_DrawGoronSword(PlayState* play, s16 drawId);
static void GetItemOot_DrawDekuNuts(PlayState* play, s16 drawId);
static void GetItemOot_DrawRecoveryHeart(PlayState* play, s16 drawId);
static void GetItemOot_DrawFish(PlayState* play, s16 drawId);
static void GetItemOot_DrawOpa0(PlayState* play, s16 drawId);
static void GetItemOot_DrawOpa0Xlu1(PlayState* play, s16 drawId);
static void GetItemOot_DrawXlu01(PlayState* play, s16 drawId);
static void GetItemOot_DrawOpa10Xlu2(PlayState* play, s16 drawId);
static void GetItemOot_DrawMagicArrow(PlayState* play, s16 drawId);
static void GetItemOot_DrawMagicSpell(PlayState* play, s16 drawId);
static void GetItemOot_DrawOpa1023(PlayState* play, s16 drawId);
static void GetItemOot_DrawOpa10Xlu32(PlayState* play, s16 drawId);
static void GetItemOot_DrawSmallRupee(PlayState* play, s16 drawId);
static void GetItemOot_DrawScale(PlayState* play, s16 drawId);
static void GetItemOot_DrawBulletBag(PlayState* play, s16 drawId);
static void GetItemOot_DrawWallet(PlayState* play, s16 drawId);
static void GetItemOot_DrawJewel(PlayState* play, s16 drawId);
static void GetItemOot_DrawJewelKokiri(PlayState* play, s16 drawId);
static void GetItemOot_DrawJewelGoron(PlayState* play, s16 drawId);
static void GetItemOot_DrawJewelZora(PlayState* play, s16 drawId);
static void GetItemOot_DrawGenericMusicNote(PlayState* play, s16 drawId);

typedef struct {
    /* 0x00 */ void (*drawFunc)(PlayState*, s16);
    /* 0x04 */ Gfx* dlists[8];
} DrawItemTableEntryOot; // size = 0x24

static DrawItemTableEntryOot sDrawItemTableOot[] = {
    // bottle, OBJECT_GI_BOTTLE
    { GetItemOot_DrawOpa0Xlu1, { gGiBottleStopperDL, gGiBottleDL } },
    // small key, OBJECT_GI_KEY
    { GetItemOot_DrawOpa0, { gGiSmallKeyDL } },
    // minuet of forest, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiMinuetColorDL, gGiSongNoteDL } },
    // bolero of fire, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiBoleroColorDL, gGiSongNoteDL } },
    // serenade of water, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiSerenadeColorDL, gGiSongNoteDL } },
    // requiem of spirit, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiRequiemColorDL, gGiSongNoteDL } },
    // nocturne of shadow, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiNocturneColorDL, gGiSongNoteDL } },
    // prelude of light, OBJECT_GI_MELODY
    { GetItemOot_DrawXlu01, { gGiPreludeColorDL, gGiSongNoteDL } },
    // recovery heart, OBJECT_GI_HEART
    { GetItemOot_DrawRecoveryHeart, { gGiRecoveryHeartDL } },
    // boss key, OBJECT_GI_BOSSKEY
    { GetItemOot_DrawOpa0Xlu1, { gGiBossKeyDL, gGiBossKeyGemDL } },
    // compass, OBJECT_GI_COMPASS
    { GetItemOot_DrawCompass, { gGiCompassDL, gGiCompassGlassDL } },
    // forest medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiForestMedallionFaceDL, gGiMedallionDL } },
    // fire medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiFireMedallionFaceDL, gGiMedallionDL } },
    // water medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiWaterMedallionFaceDL, gGiMedallionDL } },
    // spirit medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiSpiritMedallionFaceDL, gGiMedallionDL } },
    // shadow medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiShadowMedallionFaceDL, gGiMedallionDL } },
    // light medallion, OBJECT_GI_MEDAL
    { GetItemOot_DrawEggOrMedallion, { gGiLightMedallionFaceDL, gGiMedallionDL } },
    // deku nuts, OBJECT_GI_NUTS
    { GetItemOot_DrawDekuNuts, { gGiNutDL } },
    // heart container, OBJECT_GI_HEARTS
    { GetItemOot_DrawXlu01, { gGiHeartBorderDL, gGiHeartContainerDL } },
    // heart piece, OBJECT_GI_HEARTS
    { GetItemOot_DrawXlu01, { gGiHeartBorderDL, gGiHeartPieceDL } },
    // quiver 30, OBJECT_GI_ARROWCASE
    { GetItemOot_DrawOpa1023, { gGiQuiverInnerDL, gGiQuiver30InnerColorDL, gGiQuiver30OuterColorDL, gGiQuiverOuterDL } },
    // quiver 40, OBJECT_GI_ARROWCASE
    { GetItemOot_DrawOpa1023, { gGiQuiverInnerDL, gGiQuiver40InnerColorDL, gGiQuiver40OuterColorDL, gGiQuiverOuterDL } },
    // quiver 50, OBJECT_GI_ARROWCASE
    { GetItemOot_DrawOpa1023, { gGiQuiverInnerDL, gGiQuiver50InnerColorDL, gGiQuiver50OuterColorDL, gGiQuiverOuterDL } },
    // bomb bag 20, OBJECT_GI_BOMBPOUCH
    { GetItemOot_DrawOpa1023, { gGiBombBagDL, gGiBombBag20BagColorDL, gGiBombBag20RingColorDL, gGiBombBagRingDL } },
    // bomb bag 30, OBJECT_GI_BOMBPOUCH
    { GetItemOot_DrawOpa1023, { gGiBombBagDL, gGiBombBag30BagColorDL, gGiBombBag30RingColorDL, gGiBombBagRingDL } },
    // bomb bag 40, OBJECT_GI_BOMBPOUCH
    { GetItemOot_DrawOpa1023, { gGiBombBagDL, gGiBombBag40BagColorDL, gGiBombBag40RingColorDL, gGiBombBagRingDL } },
    // stick, OBJECT_GI_STICK
    { GetItemOot_DrawOpa0, { gGiStickDL } },
    // dungeon map, OBJECT_GI_MAP
    { GetItemOot_DrawOpa0, { gGiDungeonMapDL } },
    // deku shield, OBJECT_GI_SHIELD_1
    { GetItemOot_DrawOpa0, { gGiDekuShieldDL } },
    // small magic jar, OBJECT_GI_MAGICPOT
    { GetItemOot_DrawOpa0, { gGiMagicJarSmallDL } },
    // large magic jar, OBJECT_GI_MAGICPOT
    { GetItemOot_DrawOpa0, { gGiMagicJarLargeDL } },
    // bombs, OBJECT_GI_BOMB_1
    { GetItemOot_DrawOpa0, { gGiBombDL } },
    // stone of agony, OBJECT_GI_MAP
    { GetItemOot_DrawOpa0, { gGiStoneOfAgonyDL } },
    // adult's wallet, OBJECT_GI_PURSE
    { GetItemOot_DrawWallet,
      { gGiWalletDL, gGiAdultWalletColorDL, gGiAdultWalletRupeeOuterColorDL, gGiWalletRupeeOuterDL,
        gGiAdultWalletStringColorDL, gGiWalletStringDL, gGiAdultWalletRupeeInnerColorDL, gGiWalletRupeeInnerDL } },
    // giant's wallet, OBJECT_GI_PURSE
    { GetItemOot_DrawWallet,
      { gGiWalletDL, gGiGiantsWalletColorDL, gGiGiantsWalletRupeeOuterColorDL, gGiWalletRupeeOuterDL,
        gGiGiantsWalletStringColorDL, gGiWalletStringDL, gGiGiantsWalletRupeeInnerColorDL, gGiWalletRupeeInnerDL } },
    // gerudo card, OBJECT_GI_GERUDO
    { GetItemOot_DrawOpa0, { gGiGerudoCardDL } },
    // arrows (small), OBJECT_GI_ARROW
    { GetItemOot_DrawOpa0, { gGiArrowSmallDL } },
    // arrows (medium), OBJECT_GI_ARROW
    { GetItemOot_DrawOpa0, { gGiArrowMediumDL } },
    // arrows (large), OBJECT_GI_ARROW
    { GetItemOot_DrawOpa0, { gGiArrowLargeDL } },
    // bombchus, OBJECT_GI_BOMB_2
    { GetItemOot_DrawMaskOrBombchu, { gGiBombchuDL } },
    // egg, OBJECT_GI_EGG
    { GetItemOot_DrawEggOrMedallion, { gGiEggMaterialDL, gGiEggDL } },
    // silver scale, OBJECT_GI_SCALE
    { GetItemOot_DrawScale, { gGiScaleWaterDL, gGiSilverScaleWaterColorDL, gGiSilverScaleColorDL, gGiScaleDL } },
    // gold scale, OBJECT_GI_SCALE
    { GetItemOot_DrawScale, { gGiScaleWaterDL, gGiGoldenScaleWaterColorDL, gGiGoldenScaleColorDL, gGiScaleDL } },
    // hylian shield, OBJECT_GI_SHIELD_2
    { GetItemOot_DrawOpa0, { gGiHylianShieldDL } },
    // hookshot, OBJECT_GI_HOOKSHOT
    { GetItemOot_DrawOpa0, { gGiHookshotDL } },
    // longshot, OBJECT_GI_HOOKSHOT
    { GetItemOot_DrawOpa0, { gGiLongshotDL } },
    // ocarina of time, OBJECT_GI_OCARINA
    { GetItemOot_DrawOpa0Xlu1, { gGiOcarinaTimeDL, gGiOcarinaTimeHolesDL } },
    // milk, OBJECT_GI_MILK
    { GetItemOot_DrawOpa0Xlu1, { gGiMilkBottleContentsDL, gGiMilkBottleDL } },
    // keaton mask, OBJECT_GI_KI_TAN_MASK
    { GetItemOot_DrawOpa0Xlu1, { gGiKeatonMaskDL, gGiKeatonMaskEyesDL } },
    // spooky mask, OBJECT_GI_REDEAD_MASK
    { GetItemOot_DrawOpa0, { gGiSpookyMaskDL } },
    // slingshot, OBJECT_GI_PACHINKO
    { GetItemOot_DrawOpa0, { gGiSlingshotDL } },
    // boomerang, OBJECT_GI_BOOMERANG
    { GetItemOot_DrawOpa0, { gGiBoomerangDL } },
    // bow, OBJECT_GI_BOW
    { GetItemOot_DrawOpa0, { gGiBowDL } },
    // lens, OBJECT_GI_GLASSES
    { GetItemOot_DrawOpa0Xlu1, { gGiLensDL, gGiLensGlassDL } },
    // green potion, OBJECT_GI_LIQUID
    { GetItemOot_DrawPotion,
      { gGiPotionPotDL, gGiGreenPotColorDL, gGiGreenLiquidColorDL, gGiPotionLiquidDL, gGiGreenPatternColorDL,
        gGiPotionPatternDL } },
    // red potion, OBJECT_GI_LIQUID
    { GetItemOot_DrawPotion,
      { gGiPotionPotDL, gGiRedPotColorDL, gGiRedLiquidColorDL, gGiPotionLiquidDL, gGiRedPatternColorDL,
        gGiPotionPatternDL } },
    // blue potion, OBJECT_GI_LIQUID
    { GetItemOot_DrawPotion,
      { gGiPotionPotDL, gGiBluePotColorDL, gGiBlueLiquidColorDL, gGiPotionLiquidDL, gGiBluePatternColorDL,
        gGiPotionPatternDL } },
    // mirror shield, OBJECT_GI_SHIELD_3
    { GetItemOot_DrawMirrorShield, { gGiMirrorShieldDL, gGiMirrorShieldSymbolDL } },
    // zelda's letter, OBJECT_GI_LETTER
    { GetItemOot_DrawOpa0Xlu1, { gGiLetterDL, gGiLetterWritingDL } },
    // goron tunic, OBJECT_GI_CLOTHES
    { GetItemOot_DrawOpa1023, { gGiTunicCollarDL, gGiGoronCollarColorDL, gGiGoronTunicColorDL, gGiTunicDL } },
    // zora tunic, OBJECT_GI_CLOTHES
    { GetItemOot_DrawOpa1023, { gGiTunicCollarDL, gGiZoraCollarColorDL, gGiZoraTunicColorDL, gGiTunicDL } },
    // beans, OBJECT_GI_BEAN
    { GetItemOot_DrawOpa0, { gGiBeanDL } },
    // fish, OBJECT_GI_FISH
    { GetItemOot_DrawFish, { gGiFishDL } },
    // saw, OBJECT_GI_SAW
    { GetItemOot_DrawOpa0, { gGiSawDL } },
    // hammer, OBJECT_GI_HAMMER
    { GetItemOot_DrawOpa0, { gGiHammerDL } },
    // grass, OBJECT_GI_GRASS
    { GetItemOot_DrawOpa0, { gGiGrassDL } },
    // biggorons sword, OBJECT_GI_LONGSWORD
    { GetItemOot_DrawGoronSword, { gGiBiggoronSwordDL } },
    // chicken, OBJECT_GI_NIWATORI
    { GetItemOot_DrawOpa10Xlu2, { gGiChickenDL, gGiChickenColorDL, gGiChickenEyesDL } },
    // ruto's letter, OBJECT_GI_BOTTLE_LETTER
    { GetItemOot_DrawOpa0Xlu1, { gGiLetterBottleContentsDL, gGiLetterBottleDL } },
    // fairy ocarina, OBJECT_GI_OCARINA_0
    { GetItemOot_DrawOpa0Xlu1, { gGiOcarinaFairyDL, gGiOcarinaFairyHolesDL } },
    // iron boots, OBJECT_GI_BOOTS_2
    { GetItemOot_DrawOpa0Xlu1, { gGiIronBootsDL, gGiIronBootsRivetsDL } },
    // seeds, OBJECT_GI_SEED
    { GetItemOot_DrawOpa0, { gGiSeedDL } },
    // silver gauntlets, OBJECT_GI_GLOVES
    { GetItemOot_DrawOpa10Xlu32,
      { gGiGauntletsDL, gGiSilverGauntletsColorDL, gGiGauntletsPlateDL, gGiSilverGauntletsPlateColorDL } },
    // golden gauntlets, OBJECT_GI_GLOVES
    { GetItemOot_DrawOpa10Xlu32,
      { gGiGauntletsDL, gGiGoldenGauntletsColorDL, gGiGauntletsPlateDL, gGiGoldenGauntletsPlateColorDL } },
    // yellow n coin, OBJECT_GI_COIN
    { GetItemOot_DrawOpa10Xlu2, { gGiCoinDL, gGiYellowCoinColorDL, gGiNDL } },
    // red n coin, OBJECT_GI_COIN
    { GetItemOot_DrawOpa10Xlu2, { gGiCoinDL, gGiRedCoinColorDL, gGiNDL } },
    // green n coin, OBJECT_GI_COIN
    { GetItemOot_DrawOpa10Xlu2, { gGiCoinDL, gGiGreenCoinColorDL, gGiNDL } },
    // blue n coin, OBJECT_GI_COIN
    { GetItemOot_DrawOpa10Xlu2, { gGiCoinDL, gGiBlueCoinColorDL, gGiNDL } },
    // skull mask, OBJECT_GI_SKJ_MASK
    { GetItemOot_DrawOpa0, { gGiSkullMaskDL } },
    // bunny hood OBJECT_GI_RABIT_MASK
    { GetItemOot_DrawOpa0Xlu1, { gGiBunnyHoodDL, gGiBunnyHoodEyesDL } },
    // mask of truth, OBJECT_GI_TRUTH_MASK
    { GetItemOot_DrawOpa0Xlu1, { gGiMaskOfTruthDL, gGiMaskOfTruthAccentsDL } },
    // eyedrops, OBJECT_GI_EYE_LOTION
    { GetItemOot_DrawOpa0Xlu1, { gGiEyeDropsCapDL, gGiEyeDropsBottleDL } },
    // odd potion, OBJECT_GI_POWDER
    { GetItemOot_DrawOpa0, { gGiOddPotionDL } },
    // odd mushroom, OBJECT_GI_MUSHROOM
    { GetItemOot_DrawOpa0, { gGiOddMushroomDL } },
    // claim check, OBJECT_GI_TICKETSTONE
    { GetItemOot_DrawOpa0Xlu1, { gGiClaimCheckDL, gGiClaimCheckWritingDL } },
    // broken goron's sword, OBJECT_GI_BROKENSWORD
    { GetItemOot_DrawGoronSword, { gGiBrokenGoronSwordDL } },
    // prescription, OBJECT_GI_PRESCRIPTION
    { GetItemOot_DrawOpa0Xlu1, { gGiPrescriptionDL, gGiPrescriptionWritingDL } },
    // goron bracelet, OBJECT_GI_BRACELET
    { GetItemOot_DrawOpa0, { gGiGoronBraceletDL } },
    // sold out, OBJECT_GI_SOLDOUT
    { GetItemOot_DrawSoldOut, { gGiSoldOutDL } },
    // frog, OBJECT_GI_FROG
    { GetItemOot_DrawOpa0Xlu1, { gGiFrogDL, gGiFrogEyesDL } },
    // goron mask, OBJECT_GI_GOLONMASK
    { GetItemOot_DrawMaskOrBombchu, { gGiGoronMaskDL } },
    // zora mask, OBJECT_GI_ZORAMASK
    { GetItemOot_DrawMaskOrBombchu, { gGiZoraMaskDL } },
    // gerudo mask, OBJECT_GI_GERUDOMASK
    { GetItemOot_DrawMaskOrBombchu, { gGiGerudoMaskDL } },
    // cojiro, OBJECT_GI_NIWATORI
    { GetItemOot_DrawOpa10Xlu2, { gGiChickenDL, gGiCojiroColorDL, gGiChickenEyesDL } },
    // hover boots, OBJECT_GI_HOVERBOOTS
    { GetItemOot_DrawOpa0, { gGiHoverBootsDL } },
    // fire arrows, OBJECT_GI_M_ARROW
    { GetItemOot_DrawMagicArrow, { gGiMagicArrowDL, gGiFireArrowColorDL, gGiArrowMagicDL } },
    // ice arrows, OBJECT_GI_M_ARROW
    { GetItemOot_DrawMagicArrow, { gGiMagicArrowDL, gGiIceArrowColorDL, gGiArrowMagicDL } },
    // light arrows, OBJECT_GI_M_ARROW
    { GetItemOot_DrawMagicArrow, { gGiMagicArrowDL, gGiLightArrowColorDL, gGiArrowMagicDL } },
    // skulltula token, OBJECT_GI_SUTARU
    { GetItemOot_DrawSkullToken, { gGiSkulltulaTokenDL, gGiSkulltulaTokenFlameDL } },
    // din's fire, OBJECT_GI_GODDESS
    { GetItemOot_DrawMagicSpell, { gGiMagicSpellDiamondDL, gGiDinsFireColorDL, gGiMagicSpellOrbDL } },
    // farore's wind, OBJECT_GI_GODDESS
    { GetItemOot_DrawMagicSpell, { gGiMagicSpellDiamondDL, gGiFaroresWindColorDL, gGiMagicSpellOrbDL } },
    // nayru's Love, OBJECT_GI_GODDESS
    { GetItemOot_DrawMagicSpell, { gGiMagicSpellDiamondDL, gGiNayrusLoveColorDL, gGiMagicSpellOrbDL } },
    // blue fire, OBJECT_GI_FIRE
    { GetItemOot_DrawBlueFire, { gGiBlueFireChamberstickDL, gGiBlueFireFlameDL } },
    // bugs, OBJECT_GI_INSECT
    { GetItemOot_DrawOpa0Xlu1, { gGiBugsContainerDL, gGiBugsGlassDL } },
    // butterfly, OBJECT_GI_BUTTERFLY
    { GetItemOot_DrawOpa0Xlu1, { gGiButterflyContainerDL, gGiButterflyGlassDL } },
    // poe, OBJECT_GI_GHOST
    { GetItemOot_DrawPoes,
      { gGiGhostContainerLidDL, gGiGhostContainerGlassDL, gGiGhostContainerContentsDL, gGiPoeColorDL } },
    // fairy, OBJECT_GI_SOUL
    { GetItemOot_DrawFairy, { gGiFairyContainerBaseCapDL, gGiFairyContainerGlassDL, gGiFairyContainerContentsDL } },
    // bullet bag 40, OBJECT_GI_DEKUPOUCH
    { GetItemOot_DrawBulletBag,
      { gGiBulletBagDL, gGiBulletBagColorDL, gGiBulletBagStringDL, gGiBulletBagStringColorDL, gGiBulletBagWritingDL } },
    // green rupee, OBJECT_GI_RUPY
    { GetItemOot_DrawSmallRupee,
      { gGiRupeeInnerDL, gGiGreenRupeeInnerColorDL, gGiRupeeOuterDL, gGiGreenRupeeOuterColorDL } },
    // blue rupee, OBJECT_GI_RUPY
    { GetItemOot_DrawSmallRupee,
      { gGiRupeeInnerDL, gGiBlueRupeeInnerColorDL, gGiRupeeOuterDL, gGiBlueRupeeOuterColorDL } },
    // red rupee, OBJECT_GI_RUPY
    { GetItemOot_DrawSmallRupee, { gGiRupeeInnerDL, gGiRedRupeeInnerColorDL, gGiRupeeOuterDL, gGiRedRupeeOuterColorDL } },
    // big poe, OBJECT_GI_GHOST
    { GetItemOot_DrawPoes,
      { gGiGhostContainerLidDL, gGiGhostContainerGlassDL, gGiGhostContainerContentsDL, gGiBigPoeColorDL } },
    // purple rupee, OBJECT_GI_RUPY
    { GetItemOot_DrawOpa10Xlu32,
      { gGiRupeeInnerDL, gGiPurpleRupeeInnerColorDL, gGiRupeeOuterDL, gGiPurpleRupeeOuterColorDL } },
    // gold rupee, OBJECT_GI_RUPY
    { GetItemOot_DrawOpa10Xlu32,
      { gGiRupeeInnerDL, gGiGoldRupeeInnerColorDL, gGiRupeeOuterDL, gGiGoldRupeeOuterColorDL } },
    // bullet bag 50, OBJECT_GI_DEKUPOUCH
    { GetItemOot_DrawBulletBag,
      { gGiBulletBagDL, gGiBulletBag50ColorDL, gGiBulletBagStringDL, gGiBulletBag50StringColorDL,
        gGiBulletBagWritingDL } },
    // kokiri sword, OBJECT_GI_SWORD_1
    { GetItemOot_DrawOpa0, { gGiKokiriSwordDL } },
    // gold skulltula token, OBJECT_ST
    { GetItemOot_DrawSkullToken, { gSkulltulaTokenDL, gSkulltulaTokenFlameDL } },

    { GetItemOot_DrawJewelKokiri, { gGiKokiriEmeraldGemDL, gGiKokiriEmeraldSettingDL } },
    { GetItemOot_DrawJewelGoron, { gGiGoronRubyGemDL, gGiGoronRubySettingDL } },
    { GetItemOot_DrawJewelZora, { gGiZoraSapphireGemDL, gGiZoraSapphireSettingDL } },

    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } }, // Generic

    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Zelda's  Lullaby
    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Epona's song
    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Saria's song
    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Sun's song
    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Song of time
    { GetItemOot_DrawGenericMusicNote, { gGiSongNoteDL } },     // Song of storms
    { GetItemOot_DrawNothing, { NULL } },    // Triforce Piece
    { GetItemOot_DrawNothing, { NULL } }, // Triforce
    { GetItemOot_DrawNothing, { NULL } },       // Fishing Pole
};

// OoT's GetItem_Draw, for draw ids from GID_OOT_BASE.
void Hyrule_GetItemDrawOot(PlayState* play, s16 drawId) {
    drawId -= GID_OOT_BASE;
    sDrawItemTableOot[drawId].drawFunc(play, drawId);
}


static void GetItemOot_DrawNothing(PlayState* play, s16 drawId) {
}

// All remaining functions in this file are draw functions referenced in the table and called by the function above

static /* 0x0178 */ u8 primXluColor[3];
static /* 0x017B */ u8 envXluColor[3];
static /* 0x017E */ u8 primOpaColor[3];
static /* 0x0181 */ u8 envOpaColor[3];

static void GetItemOot_DrawJewelKokiri(PlayState* play, s16 drawId) {
    primXluColor[2] = 160;
    primXluColor[0] = 255;
    primXluColor[1] = 255;
    envXluColor[0] = 0;
    envXluColor[1] = 255;
    envXluColor[2] = 0;
    primOpaColor[2] = 170;
    primOpaColor[0] = 255;
    primOpaColor[1] = 255;
    envOpaColor[1] = 120;
    envOpaColor[0] = 150;
    envOpaColor[2] = 0;

    GetItemOot_DrawJewel(play, drawId);
}

static void GetItemOot_DrawJewelGoron(PlayState* play, s16 drawId) {
    primXluColor[1] = 170;
    primXluColor[0] = 255;
    primXluColor[2] = 255;
    envXluColor[2] = 100;
    envXluColor[0] = 255;
    envXluColor[1] = 0;
    primOpaColor[2] = 170;
    primOpaColor[0] = 255;
    primOpaColor[1] = 255;
    envOpaColor[1] = 120;
    envOpaColor[0] = 150;
    envOpaColor[2] = 0;

    GetItemOot_DrawJewel(play, drawId);
}

static void GetItemOot_DrawJewelZora(PlayState* play, s16 drawId) {
    primXluColor[0] = 50;
    primXluColor[1] = 255;
    primXluColor[2] = 255;
    envXluColor[2] = 150;
    envXluColor[0] = 50;
    envXluColor[1] = 0;
    primOpaColor[2] = 170;
    primOpaColor[0] = 255;
    primOpaColor[1] = 255;
    envOpaColor[1] = 120;
    envOpaColor[0] = 150;
    envOpaColor[2] = 0;

    GetItemOot_DrawJewel(play, drawId);
}

static void GetItemOot_DrawJewel(PlayState* play, s16 drawId) {
    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 9,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 % 256, (256 - (0 % 256)) - 1, 64, 64, 1, 0 % 256,
                                  (256 - (0 % 256)) - 1, 16, 16, 0, 0, 0, 0));

    gSPSegment(POLY_OPA_DISP++, 8, Gfx_TexScrollEx(play->state.gfxCtx, (u8)0, (u8)0, 16, 16, 0, 0));

    Matrix_Push();
    Matrix_RotateZYX(0, -0x4000, 0x4000, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    // func_8002ED80(&this->actor, play, 0);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 128, primXluColor[0], primXluColor[1], primXluColor[2], 255);
    gDPSetEnvColor(POLY_XLU_DISP++, envXluColor[0], envXluColor[1], envXluColor[2], 255);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    // func_8002EBCC(&this->actor, play, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 128, primOpaColor[0], primOpaColor[1], primOpaColor[2], 255);
    gDPSetEnvColor(POLY_OPA_DISP++, envOpaColor[0], envOpaColor[1], envOpaColor[2], 255);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawMaskOrBombchu(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL26_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawSoldOut(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, 5);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawBlueFire(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0), 0 * (play->state.frames * 0), 16,
                                  32, 1, 1 * (play->state.frames * 1), 1 * -(play->state.frames * 8), 16, 32, 0, 0, 1,
                                  -8));
    Matrix_Push();
    Matrix_Translate(-8.0f, -2.0f, 0.0f, MTXMODE_APPLY);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawPoes(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0), 0 * (play->state.frames * 0), 16,
                                  32, 1, 1 * (play->state.frames * 1), 1 * -(play->state.frames * 6), 16, 32, 0, 0, 1,
                                  -6));
    Matrix_Push();
    Matrix_ReplaceRotation(&play->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawFairy(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0), 0 * (play->state.frames * 0), 32,
                                  32, 1, 1 * (play->state.frames * 1), 1 * -(play->state.frames * 6), 32, 32, 0, 0, 1,
                                  -6));
    Matrix_Push();
    Matrix_ReplaceRotation(&play->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawMirrorShield(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0) % 256,
                                  1 * (play->state.frames * 2) % 256, 64, 64, 1, 0 * (play->state.frames * 0) % 128,
                                  1 * (play->state.frames * 1) % 128, 32, 32, 0, 2, 0, 1));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawSkullToken(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0), 1 * -(play->state.frames * 5),
                                  32, 32, 1, 0 * (play->state.frames * 0), 0 * (play->state.frames * 0), 32, 64, 0, -5,
                                  0, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawEggOrMedallion(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL26_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawCompass(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, 5);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawPotion(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, -1 * (play->state.frames * 1), 1 * (play->state.frames * 1),
                                  32, 32, 1, -1 * (play->state.frames * 1), 1 * (play->state.frames * 1), 32, 32, -1, 1,
                                  -1, 1));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[3]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[4]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[5]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawGoronSword(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 1 * (play->state.frames * 1), 0 * (play->state.frames * 1), 32,
                                  32, 1, 0 * (play->state.frames * 1), 0 * (play->state.frames * 1), 32, 32, 1, 0, 0,
                                  0));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawDekuNuts(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 1 * (play->state.frames * 6), 1 * (play->state.frames * 6), 32,
                                  32, 1, 1 * (play->state.frames * 6), 1 * (play->state.frames * 6), 32, 32, 6, 6, 6,
                                  6));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawRecoveryHeart(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 1), 1 * -(play->state.frames * 3),
                                  32, 32, 1, 0 * (play->state.frames * 1), 1 * -(play->state.frames * 2), 32, 32, 0, -3,
                                  0, -2));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawFish(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0 * (play->state.frames * 0), 1 * (play->state.frames * 1), 32,
                                  32, 1, 0 * (play->state.frames * 0), 1 * (play->state.frames * 1), 32, 32, 0, 1, 0,
                                  1));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawOpa0(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawOpa0Xlu1(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawGenericMusicNote(PlayState* play, s16 drawId) {
    s32 pad;
    s16 color_slot = drawId - 120; // 0 = generic
    s16* colors[7][3] = {
        { 255, 255, 255 }, // Generic Song (full white)
        { 109, 73, 143 },  // Lullaby
        { 217, 110, 48 },  // Epona
        { 62, 109, 23 },   // Saria
        { 237, 231, 62 },  // Sun
        { 98, 177, 211 },  // Time
        { 146, 146, 146 }  // Storms
    };

    OPEN_DISPS(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, colors[color_slot][0], colors[color_slot][1], colors[color_slot][2], 255);
    gSPGrayscale(POLY_XLU_DISP++, true);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPGrayscale(POLY_XLU_DISP++, false);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawXlu01(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawOpa10Xlu2(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawMagicArrow(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawMagicSpell(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 1 * (play->state.frames * 2), 1 * -(play->state.frames * 6),
                                  32, 32, 1, 1 * (play->state.frames * 1), -1 * (play->state.frames * 2), 32, 32, 2, -6,
                                  1, -2));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawOpa1023(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[3]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawOpa10Xlu32(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawSmallRupee(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Scale(0.7f, 0.7f, 0.7f, MTXMODE_APPLY);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawScale(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 1 * (play->state.frames * 2), -1 * (play->state.frames * 2),
                                  64, 64, 1, 1 * (play->state.frames * 4), 1 * -(play->state.frames * 4), 32, 32, 2, -2,
                                  4, -4));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawBulletBag(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_XLU_DISP++, sDrawItemTableOot[drawId].dlists[4]);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void GetItemOot_DrawWallet(PlayState* play, s16 drawId) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[1]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[0]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[2]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[3]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[4]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[5]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[6]);
    gSPDisplayList(POLY_OPA_DISP++, sDrawItemTableOot[drawId].dlists[7]);

    CLOSE_DISPS(play->state.gfxCtx);
}



