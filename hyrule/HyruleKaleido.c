// OoT's pages on MM's pause menu ring (docs/PAUSE.md): the equipment page (z_kaleido_equipment.c in OoT)
// here, the quest status page in HyruleKaleidoQuest.c.
// The grid is OoT's 4x4: an upgrade column, then three cells each for swords, shields, tunics and boots.
// A cell can hold several pieces (MM's swords, the Master and Great Fairy's Swords, the Deku, Hero's and MM
// Mirror shields); it shows the one worn, or else the first owned, and A wears it or the next one.

#include "global.h"
#include "HyruleKaleido.h"
#include "HyruleKaleidoQuest.h"
#include "HyruleInventory.h"
#include "HyrulePort.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/oot/textures/icon_item_static/icon_item_static.h"
#include "assets/oot/textures/icon_item_nes_static/icon_item_nes_static.h"
#include "assets/oot/textures/item_name_static/item_name_static.h"
#include "assets/oot/textures/icon_item_24_static/icon_item_24_static.h"
#include "interface/parameter_static/parameter_static.h"

static TexturePtr sEquipPageBgTextures[] = {
    // Column 0
    (TexturePtr)gPauseEquipment00Tex,
    (TexturePtr)gPauseEquipment01Tex,
    (TexturePtr)gPauseEquipment02Tex,
    (TexturePtr)gPauseEquipment03Tex,
    (TexturePtr)gPauseEquipment04Tex,
    // Column 1
    (TexturePtr)gPauseEquipment10ENGTex,
    (TexturePtr)gPauseEquipment11Tex,
    (TexturePtr)gPauseEquipment12Tex,
    (TexturePtr)gPauseEquipment13Tex,
    (TexturePtr)gPauseEquipment14Tex,
    // Column 2
    (TexturePtr)gPauseEquipment20Tex,
    (TexturePtr)gPauseEquipment21Tex,
    (TexturePtr)gPauseEquipment22Tex,
    (TexturePtr)gPauseEquipment23Tex,
    (TexturePtr)gPauseEquipment24Tex,
};


#define EQUIP_ROWS 4
#define EQUIP_COLS 4
#define EQUIP_CELL_PIECES_MAX 3

// The bodies a piece fits. Deku, Goron, Zora and Fierce Deity Link wear none.
#define BODY_CHILD (1 << 0)
#define BODY_ADULT (1 << 1)

typedef struct EquipPiece {
    /* 0x0 */ u8 owned;   // EquipOwned* bit within the row's type
    /* 0x1 */ u8 value;   // EQUIP_VALUE_* worn
    /* 0x2 */ u16 item;   // for the icon and the name: MM's id for MM's pieces, else OoT's
    /* 0x4 */ u8 bodies;  // BODY_*
} EquipPiece;              // size = 0x6

typedef struct EquipCell {
    /* 0x0 */ u8 count;
    /* 0x2 */ EquipPiece pieces[EQUIP_CELL_PIECES_MAX]; // in display priority
} EquipCell;

// Rows are EQUIP_TYPE_SWORD, EQUIP_TYPE_SHIELD, EQUIP_TYPE_TUNIC and EQUIP_TYPE_BOOTS_Oot; columns 1 to 3.
static const EquipCell sEquipCells[EQUIP_ROWS][EQUIP_COLS - 1] = {
    {
        // MM's swords: only one exists at a time.
        { 3,
          { { EQUIP_OWNED_SWORD_GILDED, EQUIP_VALUE_SWORD_GILDED, ITEM_SWORD_GILDED, BODY_CHILD },
            { EQUIP_OWNED_SWORD_RAZOR, EQUIP_VALUE_SWORD_RAZOR, ITEM_SWORD_RAZOR, BODY_CHILD },
            { EQUIP_OWNED_SWORD_KOKIRI, EQUIP_VALUE_SWORD_KOKIRI, ITEM_SWORD_KOKIRI, BODY_CHILD } } },
        { 2,
          { { EQUIP_OWNED_SWORD_MASTER, EQUIP_VALUE_SWORD_MASTER_Oot, ITEM_SWORD_MASTER_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_GREAT_FAIRY, EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot, ITEM_SWORD_GREAT_FAIRY,
              BODY_CHILD | BODY_ADULT } } },
        { 3,
          { { EQUIP_OWNED_SWORD_BIGGORON, EQUIP_VALUE_SWORD_BIGGORON_Oot, ITEM_SWORD_BGS_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_KNIFE, EQUIP_VALUE_SWORD_KNIFE_Oot, ITEM_SWORD_KNIFE_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_KNIFE_BROKEN, EQUIP_VALUE_SWORD_KNIFE_Oot, ITEM_SWORD_BROKEN_Oot, BODY_ADULT } } },
    },
    {
        { 3,
          { { EQUIP_OWNED_SHIELD_MIRROR_MM, EQUIP_VALUE_SHIELD_MIRROR, ITEM_SHIELD_MIRROR, BODY_CHILD },
            { EQUIP_OWNED_SHIELD_HERO, EQUIP_VALUE_SHIELD_HERO, ITEM_SHIELD_HERO, BODY_CHILD | BODY_ADULT },
            { EQUIP_OWNED_SHIELD_DEKU, EQUIP_VALUE_SHIELD_DEKU_Oot, ITEM_SHIELD_DEKU_Oot, BODY_CHILD } } },
        { 1, { { EQUIP_OWNED_SHIELD_HYLIAN, EQUIP_VALUE_SHIELD_HYLIAN_Oot, ITEM_SHIELD_HYLIAN_Oot,
                 BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_SHIELD_MIRROR_OOT, EQUIP_VALUE_SHIELD_MIRROR_Oot, ITEM_SHIELD_MIRROR_Oot, BODY_ADULT } } },
    },
    {
        { 1, { { EQUIP_OWNED_KOKIRI, EQUIP_VALUE_TUNIC_KOKIRI, ITEM_TUNIC_KOKIRI_Oot, BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_GORON_IRON, EQUIP_VALUE_TUNIC_GORON, ITEM_TUNIC_GORON_Oot, BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_ZORA_HOVER, EQUIP_VALUE_TUNIC_ZORA, ITEM_TUNIC_ZORA_Oot, BODY_ADULT } } },
    },
    {
        { 1, { { EQUIP_OWNED_KOKIRI, EQUIP_VALUE_BOOTS_KOKIRI_Oot, ITEM_BOOTS_KOKIRI_Oot, BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_GORON_IRON, EQUIP_VALUE_BOOTS_IRON_Oot, ITEM_BOOTS_IRON_Oot, BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_ZORA_HOVER, EQUIP_VALUE_BOOTS_HOVER_Oot, ITEM_BOOTS_HOVER_Oot, BODY_ADULT } } },
    },
};

static const u8 sEquipRowTypes[EQUIP_ROWS] = { EQUIP_TYPE_SWORD, EQUIP_TYPE_SHIELD, EQUIP_TYPE_TUNIC,
                                               EQUIP_TYPE_BOOTS_Oot };

// The upgrade column (U4): Bullet Bag, Strength, Scale, Wallet; each shows its level's item.
static const u8 sUpgradeRowTypes[EQUIP_ROWS] = { UPG_BULLET_BAG, UPG_STRENGTH, UPG_SCALE, UPG_WALLET };
static const u16 sUpgradeItems[EQUIP_ROWS][3] = {
    { ITEM_BULLET_BAG_30_Oot, ITEM_BULLET_BAG_40_Oot, ITEM_BULLET_BAG_50_Oot },
    { ITEM_BRACELET_Oot, ITEM_GAUNTLETS_SILVER_Oot, ITEM_GAUNTLETS_GOLD_Oot },
    { ITEM_SCALE_SILVER_Oot, ITEM_SCALE_GOLDEN_Oot, ITEM_NONE },
    { ITEM_WALLET_ADULT, ITEM_WALLET_GIANT, ITEM_NONE }, // MM's wallets
};

// OoT's cell corners on the page (KaleidoScope_SetVertices in OoT).
static const s16 sEquipColumnX[EQUIP_COLS] = { -114, 12, 44, 76 };
#define EQUIP_ROW_Y(row) (58 - 32 * (row))

// The cursor is drawn in its own view (KaleidoScope_DrawCursor); these map a page point to it, as MM's cursor
// tables do for its item grid.
#define PAGE_TO_CURSOR_X(x) ((x) * 0.8125f + 3.0f)
#define PAGE_TO_CURSOR_Y(y) ((y) * 0.8125f - 3.125f)

static u8 Hyrule_GetBody(void) {
    if (GET_PLAYER_FORM != PLAYER_FORM_HUMAN) {
        return 0;
    }
    return LINK_IS_ADULT_OOT ? BODY_ADULT : BODY_CHILD;
}

static s32 Hyrule_IsPieceWearable(u8 type, const EquipPiece* piece) {
    return CHECK_OWNED_EQUIP_OOT(type, piece->owned) && (piece->bodies & Hyrule_GetBody());
}

// The piece a cell shows: the one worn, else the first wearable one owned. NULL if none.
static const EquipPiece* Hyrule_GetCellPiece(s32 row, s32 col) {
    const EquipCell* cell = &sEquipCells[row][col - 1];
    u8 type = sEquipRowTypes[row];
    const EquipPiece* first = NULL;
    s32 i;

    for (i = 0; i < cell->count; i++) {
        const EquipPiece* piece = &cell->pieces[i];

        if (!Hyrule_IsPieceWearable(type, piece)) {
            continue;
        }
        if (GET_CUR_EQUIP_VALUE(type) == piece->value) {
            return piece;
        }
        if (first == NULL) {
            first = piece;
        }
    }
    return first;
}

static u16 Hyrule_GetUpgradeItem(s32 row) {
    s32 level = GET_CUR_UPG_VALUE(sUpgradeRowTypes[row]);

    return (level == 0) ? ITEM_NONE : sUpgradeItems[row][level - 1];
}

// What a cell shows, as an item id; ITEM_NONE if empty.
static u16 Hyrule_GetCellItem(s32 row, s32 col) {
    const EquipPiece* piece;

    if (col == 0) {
        return Hyrule_GetUpgradeItem(row);
    }
    piece = Hyrule_GetCellPiece(row, col);
    return (piece != NULL) ? piece->item : ITEM_NONE;
}

// OoT's item icons and names, by OoT item id (gItemIcons in OoT's z_inventory.c, its pause menu's English
// names); NULL where OoT has none.
static const TexturePtr sOotItemIcons[] = {
    (TexturePtr)gItemIconDekuStickTex, // ITEM_STICK_Oot
    (TexturePtr)gItemIconDekuNutTex, // ITEM_NUT_Oot
    (TexturePtr)gItemIconBombTex, // ITEM_BOMB_Oot
    (TexturePtr)gItemIconBowTex, // ITEM_BOW_Oot
    (TexturePtr)gItemIconArrowFireTex, // ITEM_ARROW_FIRE_Oot
    (TexturePtr)gItemIconDinsFireTex, // ITEM_DINS_FIRE_Oot
    (TexturePtr)gItemIconSlingshotTex, // ITEM_SLINGSHOT_Oot
    (TexturePtr)gItemIconOcarinaFairyTex, // ITEM_OCARINA_FAIRY_Oot
    (TexturePtr)gItemIconOcarinaOfTimeTex, // ITEM_OCARINA_TIME_Oot
    (TexturePtr)gItemIconBombchuTex, // ITEM_BOMBCHU_Oot
    (TexturePtr)gItemIconHookshotTex, // ITEM_HOOKSHOT_Oot
    (TexturePtr)gItemIconLongshotTex, // ITEM_LONGSHOT_Oot
    (TexturePtr)gItemIconArrowIceTex, // ITEM_ARROW_ICE_Oot
    (TexturePtr)gItemIconFaroresWindTex, // ITEM_FARORES_WIND_Oot
    (TexturePtr)gItemIconBoomerangTex, // ITEM_BOOMERANG_Oot
    (TexturePtr)gItemIconLensOfTruthTex, // ITEM_LENS_Oot
    (TexturePtr)gItemIconMagicBeanTex, // ITEM_BEAN_Oot
    (TexturePtr)gItemIconHammerTex, // ITEM_HAMMER_Oot
    (TexturePtr)gItemIconArrowLightTex, // ITEM_ARROW_LIGHT_Oot
    (TexturePtr)gItemIconNayrusLoveTex, // ITEM_NAYRUS_LOVE_Oot
    (TexturePtr)gItemIconBottleEmptyTex, // ITEM_BOTTLE_Oot
    (TexturePtr)gItemIconBottlePotionRedTex, // ITEM_POTION_RED_Oot
    (TexturePtr)gItemIconBottlePotionGreenTex, // ITEM_POTION_GREEN_Oot
    (TexturePtr)gItemIconBottlePotionBlueTex, // ITEM_POTION_BLUE_Oot
    (TexturePtr)gItemIconBottleFairyTex, // ITEM_FAIRY_Oot
    (TexturePtr)gItemIconBottleFishTex, // ITEM_FISH_Oot
    (TexturePtr)gItemIconBottleMilkFullTex, // ITEM_MILK_BOTTLE_Oot
    (TexturePtr)gItemIconBottleRutosLetterTex, // ITEM_LETTER_RUTO_Oot
    (TexturePtr)gItemIconBottleBlueFireTex, // ITEM_BLUE_FIRE_Oot
    (TexturePtr)gItemIconBottleBugTex, // ITEM_BUG_Oot
    (TexturePtr)gItemIconBottleBigPoeTex, // ITEM_BIG_POE_Oot
    (TexturePtr)gItemIconBottleMilkHalfTex, // ITEM_MILK_HALF_Oot
    (TexturePtr)gItemIconBottlePoeTex, // ITEM_POE_Oot
    (TexturePtr)gItemIconWeirdEggTex, // ITEM_WEIRD_EGG_Oot
    (TexturePtr)gItemIconChickenTex, // ITEM_CHICKEN_Oot
    (TexturePtr)gItemIconZeldasLetterTex, // ITEM_LETTER_ZELDA_Oot
    (TexturePtr)gItemIconMaskKeatonTex, // ITEM_MASK_KEATON_Oot
    (TexturePtr)gItemIconMaskSkullTex, // ITEM_MASK_SKULL_Oot
    (TexturePtr)gItemIconMaskSpookyTex, // ITEM_MASK_SPOOKY_Oot
    (TexturePtr)gItemIconMaskBunnyHoodTex, // ITEM_MASK_BUNNY_Oot
    (TexturePtr)gItemIconMaskGoronTex, // ITEM_MASK_GORON_Oot
    (TexturePtr)gItemIconMaskZoraTex, // ITEM_MASK_ZORA_Oot
    (TexturePtr)gItemIconMaskGerudoTex, // ITEM_MASK_GERUDO_Oot
    (TexturePtr)gItemIconMaskTruthTex, // ITEM_MASK_TRUTH_Oot
    (TexturePtr)gItemIconSoldOutTex, // ITEM_SOLD_OUT_Oot
    (TexturePtr)gItemIconPocketEggTex, // ITEM_POCKET_EGG_Oot
    (TexturePtr)gItemIconPocketCuccoTex, // ITEM_POCKET_CUCCO_Oot
    (TexturePtr)gItemIconCojiroTex, // ITEM_COJIRO_Oot
    (TexturePtr)gItemIconOddMushroomTex, // ITEM_ODD_MUSHROOM_Oot
    (TexturePtr)gItemIconOddPotionTex, // ITEM_ODD_POTION_Oot
    (TexturePtr)gItemIconPoachersSawTex, // ITEM_SAW_Oot
    (TexturePtr)gItemIconBrokenGoronsSwordTex, // ITEM_SWORD_BROKEN_Oot
    (TexturePtr)gItemIconPrescriptionTex, // ITEM_PRESCRIPTION_Oot
    (TexturePtr)gItemIconEyeballFrogTex, // ITEM_FROG_Oot
    (TexturePtr)gItemIconEyeDropsTex, // ITEM_EYEDROPS_Oot
    (TexturePtr)gItemIconClaimCheckTex, // ITEM_CLAIM_CHECK_Oot
    (TexturePtr)gItemIconBowFireTex, // ITEM_BOW_ARROW_FIRE_Oot
    (TexturePtr)gItemIconBowIceTex, // ITEM_BOW_ARROW_ICE_Oot
    (TexturePtr)gItemIconBowLightTex, // ITEM_BOW_ARROW_LIGHT_Oot
    (TexturePtr)gItemIconSwordKokiriTex, // ITEM_SWORD_KOKIRI_Oot
    (TexturePtr)gItemIconSwordMasterTex, // ITEM_SWORD_MASTER_Oot
    (TexturePtr)gItemIconSwordBiggoronTex, // ITEM_SWORD_BGS_Oot
    (TexturePtr)gItemIconShieldDekuTex, // ITEM_SHIELD_DEKU_Oot
    (TexturePtr)gItemIconShieldHylianTex, // ITEM_SHIELD_HYLIAN_Oot
    (TexturePtr)gItemIconShieldMirrorTex, // ITEM_SHIELD_MIRROR_Oot
    (TexturePtr)gItemIconTunicKokiriTex, // ITEM_TUNIC_KOKIRI_Oot
    (TexturePtr)gItemIconTunicGoronTex, // ITEM_TUNIC_GORON_Oot
    (TexturePtr)gItemIconTunicZoraTex, // ITEM_TUNIC_ZORA_Oot
    (TexturePtr)gItemIconBootsKokiriTex, // ITEM_BOOTS_KOKIRI_Oot
    (TexturePtr)gItemIconBootsIronTex, // ITEM_BOOTS_IRON_Oot
    (TexturePtr)gItemIconBootsHoverTex, // ITEM_BOOTS_HOVER_Oot
    (TexturePtr)gItemIconBulletBag30Tex, // ITEM_BULLET_BAG_30_Oot
    (TexturePtr)gItemIconBulletBag40Tex, // ITEM_BULLET_BAG_40_Oot
    (TexturePtr)gItemIconBulletBag50Tex, // ITEM_BULLET_BAG_50_Oot
    (TexturePtr)gItemIconQuiver30Tex, // ITEM_QUIVER_30_Oot
    (TexturePtr)gItemIconQuiver40Tex, // ITEM_QUIVER_40_Oot
    (TexturePtr)gItemIconQuiver50Tex, // ITEM_QUIVER_50_Oot
    (TexturePtr)gItemIconBombBag20Tex, // ITEM_BOMB_BAG_20_Oot
    (TexturePtr)gItemIconBombBag30Tex, // ITEM_BOMB_BAG_30_Oot
    (TexturePtr)gItemIconBombBag40Tex, // ITEM_BOMB_BAG_40_Oot
    (TexturePtr)gItemIconGoronsBraceletTex, // ITEM_BRACELET_Oot
    (TexturePtr)gItemIconSilverGauntletsTex, // ITEM_GAUNTLETS_SILVER_Oot
    (TexturePtr)gItemIconGoldenGauntletsTex, // ITEM_GAUNTLETS_GOLD_Oot
    (TexturePtr)gItemIconScaleSilverTex, // ITEM_SCALE_SILVER_Oot
    (TexturePtr)gItemIconScaleGoldenTex, // ITEM_SCALE_GOLDEN_Oot
    (TexturePtr)gItemIconBrokenGiantsKnifeTex, // ITEM_SWORD_KNIFE_Oot
    (TexturePtr)gItemIconAdultsWalletTex, // ITEM_WALLET_ADULT_Oot
    (TexturePtr)gItemIconGiantsWalletTex, // ITEM_WALLET_GIANT_Oot
    (TexturePtr)gItemIconDekuSeedsTex, // ITEM_SEEDS_Oot
    (TexturePtr)gItemIconFishingPoleTex, // ITEM_FISHING_POLE_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_MINUET_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_BOLERO_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_SERENADE_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_REQUIEM_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_NOCTURNE_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_PRELUDE_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_LULLABY_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_EPONA_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_SARIA_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_SUN_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_TIME_Oot
    (TexturePtr)gSongNoteTex, // ITEM_SONG_STORMS_Oot
    (TexturePtr)gQuestIconMedallionForestTex, // ITEM_MEDALLION_FOREST_Oot
    (TexturePtr)gQuestIconMedallionFireTex, // ITEM_MEDALLION_FIRE_Oot
    (TexturePtr)gQuestIconMedallionWaterTex, // ITEM_MEDALLION_WATER_Oot
    (TexturePtr)gQuestIconMedallionSpiritTex, // ITEM_MEDALLION_SPIRIT_Oot
    (TexturePtr)gQuestIconMedallionShadowTex, // ITEM_MEDALLION_SHADOW_Oot
    (TexturePtr)gQuestIconMedallionLightTex, // ITEM_MEDALLION_LIGHT_Oot
    (TexturePtr)gQuestIconKokiriEmeraldTex, // ITEM_KOKIRI_EMERALD_Oot
    (TexturePtr)gQuestIconGoronRubyTex, // ITEM_GORON_RUBY_Oot
    (TexturePtr)gQuestIconZoraSapphireTex, // ITEM_ZORA_SAPPHIRE_Oot
    (TexturePtr)gQuestIconStoneOfAgonyTex, // ITEM_STONE_OF_AGONY_Oot
    (TexturePtr)gQuestIconGerudosCardTex, // ITEM_GERUDO_CARD_Oot
    (TexturePtr)gQuestIconGoldSkulltulaTex, // ITEM_SKULL_TOKEN_Oot
    (TexturePtr)gQuestIconHeartContainerTex, // ITEM_HEART_CONTAINER_Oot
    (TexturePtr)gQuestIconHeartPieceTex, // ITEM_HEART_PIECE_Oot
    (TexturePtr)gQuestIconDungeonBossKeyTex, // ITEM_KEY_BOSS_Oot
    (TexturePtr)gQuestIconDungeonCompassTex, // ITEM_COMPASS_Oot
    (TexturePtr)gQuestIconDungeonMapTex, // ITEM_DUNGEON_MAP_Oot
    (TexturePtr)gQuestIconSmallKeyTex, // ITEM_KEY_SMALL_Oot
    (TexturePtr)gQuestIconMagicJarSmallTex, // ITEM_MAGIC_SMALL_Oot
    (TexturePtr)gQuestIconMagicJarBigTex, // ITEM_MAGIC_LARGE_Oot
    (TexturePtr)gHeartPieceIcon1Tex, // ITEM_HEART_PIECE_2_Oot
    (TexturePtr)gHeartPieceIcon2Tex, // ITEM_SINGLE_MAGIC_Oot
    (TexturePtr)gHeartPieceIcon3Tex, // ITEM_DOUBLE_MAGIC_Oot
    NULL, // ITEM_DOUBLE_DEFENSE_Oot
    NULL, // ITEM_INVALID_4_Oot
    NULL, // ITEM_INVALID_5_Oot
    NULL, // ITEM_INVALID_6_Oot
    NULL, // ITEM_INVALID_7_Oot
    NULL, // ITEM_MILK_Oot
};

static const TexturePtr sOotItemNames[] = {
    (TexturePtr)gDekuStickItemNameENGTex, // ITEM_STICK_Oot
    (TexturePtr)gDekuNutItemNameENGTex, // ITEM_NUT_Oot
    (TexturePtr)gBombItemNameENGTex, // ITEM_BOMB_Oot
    (TexturePtr)gFairyBowItemNameENGTex, // ITEM_BOW_Oot
    (TexturePtr)gFireArrowItemNameENGTex, // ITEM_ARROW_FIRE_Oot
    (TexturePtr)gDinsFireItemNameENGTex, // ITEM_DINS_FIRE_Oot
    (TexturePtr)gFairySlingshotItemNameENGTex, // ITEM_SLINGSHOT_Oot
    (TexturePtr)gFairyOcarinaItemNameENGTex, // ITEM_OCARINA_FAIRY_Oot
    (TexturePtr)gOcarinaOfTimeItemNameENGTex, // ITEM_OCARINA_TIME_Oot
    (TexturePtr)gBombchuItemNameENGTex, // ITEM_BOMBCHU_Oot
    (TexturePtr)gHookshotItemNameENGTex, // ITEM_HOOKSHOT_Oot
    (TexturePtr)gLongshotItemNameENGTex, // ITEM_LONGSHOT_Oot
    (TexturePtr)gIceArrowItemNameENGTex, // ITEM_ARROW_ICE_Oot
    (TexturePtr)gFaroresWindItemNameENGTex, // ITEM_FARORES_WIND_Oot
    (TexturePtr)gBoomerangItemNameENGTex, // ITEM_BOOMERANG_Oot
    (TexturePtr)gLensItemNameENGTex, // ITEM_LENS_Oot
    (TexturePtr)gMagicBeansItemNameENGTex, // ITEM_BEAN_Oot
    (TexturePtr)gMegatonHammerItemNameENGTex, // ITEM_HAMMER_Oot
    (TexturePtr)gLightArrowItemNameENGTex, // ITEM_ARROW_LIGHT_Oot
    (TexturePtr)gNayrusLoveItemNameENGTex, // ITEM_NAYRUS_LOVE_Oot
    (TexturePtr)gEmptyBottleItemNameENGTex, // ITEM_BOTTLE_Oot
    (TexturePtr)gRedPotionItemNameENGTex, // ITEM_POTION_RED_Oot
    (TexturePtr)gGreenPotionItemNameENGTex, // ITEM_POTION_GREEN_Oot
    (TexturePtr)gBluePotionItemNameENGTex, // ITEM_POTION_BLUE_Oot
    (TexturePtr)gBottledFairyItemNameENGTex, // ITEM_FAIRY_Oot
    (TexturePtr)gFishItemNameENGTex, // ITEM_FISH_Oot
    (TexturePtr)gFullMilkItemNameENGTex, // ITEM_MILK_BOTTLE_Oot
    (TexturePtr)gRutosLetterItemNameENGTex, // ITEM_LETTER_RUTO_Oot
    (TexturePtr)gBlueFireItemNameENGTex, // ITEM_BLUE_FIRE_Oot
    (TexturePtr)gBugItemNameENGTex, // ITEM_BUG_Oot
    (TexturePtr)gBigPoeItemNameENGTex, // ITEM_BIG_POE_Oot
    (TexturePtr)gHalfMilkItemNameENGTex, // ITEM_MILK_HALF_Oot
    (TexturePtr)gPoeItemNameENGTex, // ITEM_POE_Oot
    (TexturePtr)gWeirdEggItemNameENGTex, // ITEM_WEIRD_EGG_Oot
    (TexturePtr)gCuccoItemNameENGTex, // ITEM_CHICKEN_Oot
    (TexturePtr)gZeldasLetterItemNameENGTex, // ITEM_LETTER_ZELDA_Oot
    (TexturePtr)gKeatonMaskItemNameENGTex, // ITEM_MASK_KEATON_Oot
    (TexturePtr)gSkullMaskItemNameENGTex, // ITEM_MASK_SKULL_Oot
    (TexturePtr)gSpookyMaskItemNameENGTex, // ITEM_MASK_SPOOKY_Oot
    (TexturePtr)gBunnyHoodItemNameENGTex, // ITEM_MASK_BUNNY_Oot
    (TexturePtr)gGoronMaskItemNameENGTex, // ITEM_MASK_GORON_Oot
    (TexturePtr)gZoraMaskItemNameENGTex, // ITEM_MASK_ZORA_Oot
    (TexturePtr)gGerudoMaskItemNameENGTex, // ITEM_MASK_GERUDO_Oot
    (TexturePtr)gMaskofTruthItemNameENGTex, // ITEM_MASK_TRUTH_Oot
    (TexturePtr)gSOLDOUTItemNameENGTex, // ITEM_SOLD_OUT_Oot
    (TexturePtr)gPocketEggItemNameENGTex, // ITEM_POCKET_EGG_Oot
    (TexturePtr)gPocketCuccoItemNameENGTex, // ITEM_POCKET_CUCCO_Oot
    (TexturePtr)gCojiroItemNameENGTex, // ITEM_COJIRO_Oot
    (TexturePtr)gOddMushroomItemNameENGTex, // ITEM_ODD_MUSHROOM_Oot
    (TexturePtr)gOddPotionItemNameENGTex, // ITEM_ODD_POTION_Oot
    (TexturePtr)gPoachersSawItemNameENGTex, // ITEM_SAW_Oot
    (TexturePtr)gBrokenGoronsSwordItemNameENGTex, // ITEM_SWORD_BROKEN_Oot
    (TexturePtr)gPrescriptionItemNameENGTex, // ITEM_PRESCRIPTION_Oot
    (TexturePtr)gEyeBallFrogItemNameENGTex, // ITEM_FROG_Oot
    (TexturePtr)gEyeDropsItemNameENGTex, // ITEM_EYEDROPS_Oot
    (TexturePtr)gClaimCheckItemNameENGTex, // ITEM_CLAIM_CHECK_Oot
    (TexturePtr)gUnusedWindMedallionItemName1JPNTex, // ITEM_BOW_ARROW_FIRE_Oot
    (TexturePtr)gUnusedFireMedallionItemName1JPNTex, // ITEM_BOW_ARROW_ICE_Oot
    (TexturePtr)gUnusedIceMedallionItemName1JPNTex, // ITEM_BOW_ARROW_LIGHT_Oot
    (TexturePtr)gKokiriSwordItemNameENGTex, // ITEM_SWORD_KOKIRI_Oot
    (TexturePtr)gMasterSwordItemNameENGTex, // ITEM_SWORD_MASTER_Oot
    (TexturePtr)gGiantsKnifeItemNameENGTex, // ITEM_SWORD_BGS_Oot
    (TexturePtr)gDekuShieldItemNameENGTex, // ITEM_SHIELD_DEKU_Oot
    (TexturePtr)gHylianShieldItemNameENGTex, // ITEM_SHIELD_HYLIAN_Oot
    (TexturePtr)gMirrorShieldItemNameENGTex, // ITEM_SHIELD_MIRROR_Oot
    (TexturePtr)gKokiriTunicItemNameENGTex, // ITEM_TUNIC_KOKIRI_Oot
    (TexturePtr)gGoronTunicItemNameENGTex, // ITEM_TUNIC_GORON_Oot
    (TexturePtr)gZoraTunicItemNameENGTex, // ITEM_TUNIC_ZORA_Oot
    (TexturePtr)gKokiriBootsItemNameENGTex, // ITEM_BOOTS_KOKIRI_Oot
    (TexturePtr)gIronBootsItemNameENGTex, // ITEM_BOOTS_IRON_Oot
    (TexturePtr)gHoverBootsItemNameENGTex, // ITEM_BOOTS_HOVER_Oot
    (TexturePtr)gBulletBag30ItemNameENGTex, // ITEM_BULLET_BAG_30_Oot
    (TexturePtr)gBulletBag40ItemNameENGTex, // ITEM_BULLET_BAG_40_Oot
    (TexturePtr)gBulletBag50ItemNameENGTex, // ITEM_BULLET_BAG_50_Oot
    (TexturePtr)gQuiver30ItemNameENGTex, // ITEM_QUIVER_30_Oot
    (TexturePtr)gQuiver40ItemNameENGTex, // ITEM_QUIVER_40_Oot
    (TexturePtr)gQuiver50ItemNameENGTex, // ITEM_QUIVER_50_Oot
    (TexturePtr)gBombBag20ItemNameENGTex, // ITEM_BOMB_BAG_20_Oot
    (TexturePtr)gBombBag30ItemNameENGTex, // ITEM_BOMB_BAG_30_Oot
    (TexturePtr)gBombBag40ItemNameENGTex, // ITEM_BOMB_BAG_40_Oot
    (TexturePtr)gGoronsBraceletItemNameENGTex, // ITEM_BRACELET_Oot
    (TexturePtr)gSilverGauntletsItemNameENGTex, // ITEM_GAUNTLETS_SILVER_Oot
    (TexturePtr)gGoldenGauntletsItemNameENGTex, // ITEM_GAUNTLETS_GOLD_Oot
    (TexturePtr)gSilverScaleItemNameENGTex, // ITEM_SCALE_SILVER_Oot
    (TexturePtr)gGoldenScaleItemNameENGTex, // ITEM_SCALE_GOLDEN_Oot
    (TexturePtr)gBrokenGiantsKnifeItemNameENGTex, // ITEM_SWORD_KNIFE_Oot
    (TexturePtr)gUnusedBossKeyItemName1JPNTex, // ITEM_WALLET_ADULT_Oot
    (TexturePtr)gUnusedBossKeyItemName2JPNTex, // ITEM_WALLET_GIANT_Oot
    (TexturePtr)gUnusedBossKeyItemName3JPNTex, // ITEM_SEEDS_Oot
    (TexturePtr)gUnusedBossKeyItemName4JPNTex, // ITEM_FISHING_POLE_Oot
    (TexturePtr)gMinuetOfForestItemNameENGTex, // ITEM_SONG_MINUET_Oot
    (TexturePtr)gBoleroOfFireItemNameENGTex, // ITEM_SONG_BOLERO_Oot
    (TexturePtr)gSerenadeOfWaterItemNameENGTex, // ITEM_SONG_SERENADE_Oot
    (TexturePtr)gRequiemOfSpiritItemNameENGTex, // ITEM_SONG_REQUIEM_Oot
    (TexturePtr)gNocturneOfShadowItemNameENGTex, // ITEM_SONG_NOCTURNE_Oot
    (TexturePtr)gPreludeOfLightItemNameENGTex, // ITEM_SONG_PRELUDE_Oot
    (TexturePtr)gZeldasLullabyItemNameENGTex, // ITEM_SONG_LULLABY_Oot
    (TexturePtr)gEponasSongItemNameENGTex, // ITEM_SONG_EPONA_Oot
    (TexturePtr)gSariasSongItemNameENGTex, // ITEM_SONG_SARIA_Oot
    (TexturePtr)gSunsSongItemNameENGTex, // ITEM_SONG_SUN_Oot
    (TexturePtr)gSongOfTimeItemNameENGTex, // ITEM_SONG_TIME_Oot
    (TexturePtr)gSongOfStormsItemNameENGTex, // ITEM_SONG_STORMS_Oot
    (TexturePtr)gForestMedallionItemNameENGTex, // ITEM_MEDALLION_FOREST_Oot
    (TexturePtr)gFireMedallionItemNameENGTex, // ITEM_MEDALLION_FIRE_Oot
    (TexturePtr)gWaterMedallionItemNameENGTex, // ITEM_MEDALLION_WATER_Oot
    (TexturePtr)gSpiritMedallionItemNameENGTex, // ITEM_MEDALLION_SPIRIT_Oot
    (TexturePtr)gShadowMedallionItemNameENGTex, // ITEM_MEDALLION_SHADOW_Oot
    (TexturePtr)gLightMedallionItemNameENGTex, // ITEM_MEDALLION_LIGHT_Oot
    (TexturePtr)gKokiriEmeraldItemNameENGTex, // ITEM_KOKIRI_EMERALD_Oot
    (TexturePtr)gGoronsRubyItemNameENGTex, // ITEM_GORON_RUBY_Oot
    (TexturePtr)gZorasSapphireItemNameENGTex, // ITEM_ZORA_SAPPHIRE_Oot
    (TexturePtr)gStoneofAgonyItemNameENGTex, // ITEM_STONE_OF_AGONY_Oot
    (TexturePtr)gGerudosCardItemNameENGTex, // ITEM_GERUDO_CARD_Oot
    (TexturePtr)gGoldSkulltulaItemNameENGTex, // ITEM_SKULL_TOKEN_Oot
    (TexturePtr)gPieceOfHeartItemNameENGTex, // ITEM_HEART_CONTAINER_Oot
    (TexturePtr)gUnusedPieceOfHeartItemName1JPNTex, // ITEM_HEART_PIECE_Oot
    (TexturePtr)gUnusedBigKeyItemNameENGTex, // ITEM_KEY_BOSS_Oot
    (TexturePtr)gCompassItemNameENGTex, // ITEM_COMPASS_Oot
    (TexturePtr)gDungeonMapItemNameENGTex, // ITEM_DUNGEON_MAP_Oot
    (TexturePtr)gUnusedBossKeyItemName5JPNTex, // ITEM_KEY_SMALL_Oot
    (TexturePtr)gUnusedBossKeyItemName6JPNTex, // ITEM_MAGIC_SMALL_Oot
    (TexturePtr)gUnusedBossKeyItemName7JPNTex, // ITEM_MAGIC_LARGE_Oot
    (TexturePtr)gBiggoronsSwordItemNameENGTex, // ITEM_HEART_PIECE_2_Oot
};

TexturePtr Hyrule_GetItemIcon(u16 item) {
    if (item >= ITEM_OOT_BASE) {
        item -= ITEM_OOT_BASE;
        return (item < ARRAY_COUNT(sOotItemIcons)) ? sOotItemIcons[item] : NULL;
    }
    return (item < ARRAY_COUNT(gItemIcons)) ? gItemIcons[item] : NULL;
}

TexturePtr Hyrule_GetItemNameTexture(u16 item) {
    if ((item < ITEM_OOT_BASE) || (item - ITEM_OOT_BASE >= ARRAY_COUNT(sOotItemNames))) {
        return NULL;
    }
    return sOotItemNames[item - ITEM_OOT_BASE];
}

static void Hyrule_SetQuad(Vtx* vtx, s16 x, s16 y, s16 size, u8 alpha) {
    s32 i;

    for (i = 0; i < 4; i++) {
        vtx[i].v.ob[0] = x + ((i & 1) ? size : 0);
        vtx[i].v.ob[1] = y - ((i & 2) ? size : 0);
        vtx[i].v.ob[2] = 0;
        vtx[i].v.flag = 0;
        vtx[i].v.tc[0] = (i & 1) ? 32 << 5 : 0;
        vtx[i].v.tc[1] = (i & 2) ? 32 << 5 : 0;
        vtx[i].v.cn[0] = vtx[i].v.cn[1] = vtx[i].v.cn[2] = 255;
        vtx[i].v.cn[3] = alpha;
    }
}

// equipVtx: a 28x28 icon quad per cell, then a 32x32 outline quad per row for the piece worn.
#define EQUIP_VTX_ICON(row, col) (((row) * EQUIP_COLS + (col)) * 4)
#define EQUIP_VTX_OUTLINE(row) ((EQUIP_ROWS * EQUIP_COLS + (row)) * 4)
#define EQUIP_VTX_COUNT ((EQUIP_ROWS * EQUIP_COLS + EQUIP_ROWS) * 4)

static void Hyrule_KaleidoSetEquipmentVertices(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 row;
    s32 col;

    pauseCtx->equipVtx = GRAPH_ALLOC(play->state.gfxCtx, EQUIP_VTX_COUNT * sizeof(Vtx));

    for (row = 0; row < EQUIP_ROWS; row++) {
        s16 y = EQUIP_ROW_Y(row) + pauseCtx->offsetY;

        for (col = 0; col < EQUIP_COLS; col++) {
            Hyrule_SetQuad(&pauseCtx->equipVtx[EQUIP_VTX_ICON(row, col)], sEquipColumnX[col] + 2, y - 2, 28, 255);
        }
        // Moved onto the worn piece's cell when drawn.
        Hyrule_SetQuad(&pauseCtx->equipVtx[EQUIP_VTX_OUTLINE(row)], 0, y, 32, 255);
    }
}

static void Hyrule_KaleidoDrawEquipment(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 row;
    s32 col;

    Hyrule_SyncEquipmentOwnership();

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    // Outline the piece worn of each type, as OoT does.
    for (row = 0; row < EQUIP_ROWS; row++) {
        for (col = 1; col < EQUIP_COLS; col++) {
            const EquipPiece* piece = Hyrule_GetCellPiece(row, col);

            if ((piece != NULL) && (GET_CUR_EQUIP_VALUE(sEquipRowTypes[row]) == piece->value)) {
                Vtx* outline = &pauseCtx->equipVtx[EQUIP_VTX_OUTLINE(row)];
                s32 i;

                for (i = 0; i < 4; i++) {
                    outline[i].v.ob[0] = sEquipColumnX[col] + ((i & 1) ? 32 : 0);
                }
                gSPVertex(POLY_OPA_DISP++, outline, 4, 0);
                POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gEquippedItemOutlineTex, 32, 32, 0);
                break;
            }
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    for (row = 0; row < EQUIP_ROWS; row++) {
        for (col = 0; col < EQUIP_COLS; col++) {
            TexturePtr icon;
            u16 item = Hyrule_GetCellItem(row, col);

            if ((item == ITEM_NONE) || ((icon = Hyrule_GetItemIcon(item)) == NULL)) {
                continue;
            }
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->equipVtx[EQUIP_VTX_ICON(row, col)], 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, icon, 32, 32, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void Hyrule_KaleidoSetEquipCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 col = pauseCtx->cursorXIndex[PAUSE_EQUIP];
    s16 row = pauseCtx->cursorYIndex[PAUSE_EQUIP];

    pauseCtx->cursorX = PAGE_TO_CURSOR_X(sEquipColumnX[col] + 16);
    pauseCtx->cursorY = PAGE_TO_CURSOR_Y(EQUIP_ROW_Y(row) - 16);
    pauseCtx->cursorWidth = 15.0f;
    pauseCtx->cursorHeight = 15.0f;
}

static void Hyrule_SetEquipCursorCell(PauseContext* pauseCtx, s16 row, s16 col) {
    pauseCtx->cursorXIndex[PAUSE_EQUIP] = col;
    pauseCtx->cursorYIndex[PAUSE_EQUIP] = row;
    pauseCtx->cursorPoint[PAUSE_EQUIP] = row * EQUIP_COLS + col;
}

// Enters the page from one of its sides: the first filled cell from that side, column by column.
static s32 Hyrule_EnterEquipPage(PauseContext* pauseCtx, s32 fromTheLeft) {
    s32 i;
    s32 row;

    for (i = 0; i < EQUIP_COLS; i++) {
        s32 col = fromTheLeft ? i : (EQUIP_COLS - 1 - i);

        for (row = 0; row < EQUIP_ROWS; row++) {
            if (Hyrule_GetCellItem(row, col) != ITEM_NONE) {
                Hyrule_SetEquipCursorCell(pauseCtx, row, col);
                return true;
            }
        }
    }
    return false;
}

// Moves the cursor one filled cell along a row or column; returns false at the edge.
static s32 Hyrule_MoveEquipCursor(PauseContext* pauseCtx, s32 dRow, s32 dCol) {
    s32 row = pauseCtx->cursorYIndex[PAUSE_EQUIP] + dRow;
    s32 col = pauseCtx->cursorXIndex[PAUSE_EQUIP] + dCol;

    while ((row >= 0) && (row < EQUIP_ROWS) && (col >= 0) && (col < EQUIP_COLS)) {
        if (Hyrule_GetCellItem(row, col) != ITEM_NONE) {
            Hyrule_SetEquipCursorCell(pauseCtx, row, col);
            return true;
        }
        row += dRow;
        col += dCol;
    }
    return false;
}

static void Hyrule_WearPiece(PlayState* play, u8 type, const EquipPiece* piece) {
    SET_EQUIP_VALUE(type, piece->value);

    if (type == EQUIP_TYPE_SWORD) {
        // MM keeps the sword on B (Item_Give). Only human Link reaches here.
        CUR_FORM_EQUIP(EQUIP_SLOT_B) = piece->item;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
    }
    Player_SetEquipmentData(play, GET_PLAYER(play));
}

// A: wears the piece shown, or, if it is already worn, the cell's next wearable piece (U5).
static void Hyrule_EquipCell(PlayState* play, s32 row, s32 col) {
    const EquipCell* cell = &sEquipCells[row][col - 1];
    const EquipPiece* shown = Hyrule_GetCellPiece(row, col);
    u8 type = sEquipRowTypes[row];
    const EquipPiece* next = shown;

    if (GET_CUR_EQUIP_VALUE(type) == shown->value) {
        s32 index = shown - cell->pieces;
        s32 i;

        for (i = 1; i < cell->count; i++) {
            const EquipPiece* piece = &cell->pieces[(index + i) % cell->count];

            if (Hyrule_IsPieceWearable(type, piece)) {
                next = piece;
                break;
            }
        }
    }

    Hyrule_WearPiece(play, type, next);
    Audio_PlaySfx(NA_SE_SY_DECIDE);
}

static void Hyrule_KaleidoUpdateEquipCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    Input* input = CONTROLLER1(&play->state);
    s16 oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
    s16 row;
    s16 col;

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;

    if ((pauseCtx->state != PAUSE_STATE_MAIN) || (pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE) ||
        (pauseCtx->pageIndex != PAUSE_EQUIP)) {
        return;
    }

    if (pauseCtx->cursorSpecialPos == 0) {
        // The cell's contents can change (Deku Link, a piece lost); leave for a filled one, or the side.
        if (Hyrule_GetCellItem(pauseCtx->cursorYIndex[PAUSE_EQUIP], pauseCtx->cursorXIndex[PAUSE_EQUIP]) ==
            ITEM_NONE) {
            if (!Hyrule_EnterEquipPage(pauseCtx, true)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            }
        } else if (pauseCtx->stickAdjX < -30) {
            if (!Hyrule_MoveEquipCursor(pauseCtx, 0, -1)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            }
        } else if (pauseCtx->stickAdjX > 30) {
            if (!Hyrule_MoveEquipCursor(pauseCtx, 0, 1)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            }
        } else if (pauseCtx->stickAdjY > 30) {
            Hyrule_MoveEquipCursor(pauseCtx, -1, 0);
        } else if (pauseCtx->stickAdjY < -30) {
            Hyrule_MoveEquipCursor(pauseCtx, 1, 0);
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) && (pauseCtx->stickAdjX > 30)) {
        if (Hyrule_EnterEquipPage(pauseCtx, true)) {
            KaleidoScope_MoveCursorFromSpecialPos(play); // plays the cursor sound
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT) && (pauseCtx->stickAdjX < -30)) {
        if (Hyrule_EnterEquipPage(pauseCtx, false)) {
            KaleidoScope_MoveCursorFromSpecialPos(play);
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
        }
    }

    if (pauseCtx->cursorSpecialPos != 0) {
        pauseCtx->cursorItem[PAUSE_EQUIP] = PAUSE_ITEM_NONE;
        return;
    }

    row = pauseCtx->cursorYIndex[PAUSE_EQUIP];
    col = pauseCtx->cursorXIndex[PAUSE_EQUIP];
    pauseCtx->cursorItem[PAUSE_EQUIP] = Hyrule_GetCellItem(row, col);
    pauseCtx->cursorSlot[PAUSE_EQUIP] = pauseCtx->cursorPoint[PAUSE_EQUIP];

    if (pauseCtx->cursorPoint[PAUSE_EQUIP] != oldCursorPoint) {
        pauseCtx->cursorShrinkRate = 4.0f;
        Audio_PlaySfx(NA_SE_SY_CURSOR);
    }

    if (col != 0) {
        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_YELLOW;
        Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);

        if (CHECK_BTN_ALL(input->press.button, BTN_A) && (pauseCtx->cursorItem[PAUSE_EQUIP] != ITEM_NONE)) {
            Hyrule_EquipCell(play, row, col);
        }
    } else {
        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
    }
}

TexturePtr* Hyrule_GetPageBgTextures(s16 page) {
    return (page == PAUSE_EQUIP) ? sEquipPageBgTextures : Hyrule_GetQuestPageBgTextures();
}

TexturePtr Hyrule_GetToPageTexture(s16 page) {
    return (page == PAUSE_EQUIP) ? (TexturePtr)gPauseToEquipmentENGTex : Hyrule_GetToQuestStatusTexture();
}

void Hyrule_KaleidoSetPageVertices(PlayState* play, s16 page) {
    if (page == PAUSE_EQUIP) {
        Hyrule_KaleidoSetEquipmentVertices(play);
    } else {
        Hyrule_KaleidoSetQuestVertices(play);
    }
}

void Hyrule_KaleidoDrawPage(PlayState* play, s16 page) {
    if (page == PAUSE_EQUIP) {
        Hyrule_KaleidoDrawEquipment(play);
    } else {
        Hyrule_KaleidoDrawQuest(play);
    }
}

void Hyrule_KaleidoSetCursor(PlayState* play, s16 page) {
    if (page == PAUSE_EQUIP) {
        Hyrule_KaleidoSetEquipCursor(play);
    } else {
        Hyrule_KaleidoSetQuestCursor(play);
    }
}

void Hyrule_KaleidoUpdateCursor(PlayState* play, s16 page) {
    if (page == PAUSE_EQUIP) {
        Hyrule_KaleidoUpdateEquipCursor(play);
    } else {
        Hyrule_KaleidoUpdateQuestCursor(play);
    }
}
