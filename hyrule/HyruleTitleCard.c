// OoT's place-name title cards (OoT's TitleCard_InitPlaceName): the name of the area Link enters, drawn from
// OoT's g_pn_* textures. MM names its areas with scene title text instead (Message_DisplaySceneTitleCard).

#include "global.h"
#include "HyruleTitleCard.h"
#include "HyruleFlags.h"
#include "HyruleScenes.h"
#include "assets/oot/textures/place_title_cards/g_pn_01.h"
#include "assets/oot/textures/place_title_cards/g_pn_02.h"
#include "assets/oot/textures/place_title_cards/g_pn_03.h"
#include "assets/oot/textures/place_title_cards/g_pn_04.h"
#include "assets/oot/textures/place_title_cards/g_pn_05.h"
#include "assets/oot/textures/place_title_cards/g_pn_06.h"
#include "assets/oot/textures/place_title_cards/g_pn_07.h"
#include "assets/oot/textures/place_title_cards/g_pn_08.h"
#include "assets/oot/textures/place_title_cards/g_pn_09.h"
#include "assets/oot/textures/place_title_cards/g_pn_10.h"
#include "assets/oot/textures/place_title_cards/g_pn_11.h"
#include "assets/oot/textures/place_title_cards/g_pn_12.h"
#include "assets/oot/textures/place_title_cards/g_pn_13.h"
#include "assets/oot/textures/place_title_cards/g_pn_14.h"
#include "assets/oot/textures/place_title_cards/g_pn_15.h"
#include "assets/oot/textures/place_title_cards/g_pn_16.h"
#include "assets/oot/textures/place_title_cards/g_pn_17.h"
#include "assets/oot/textures/place_title_cards/g_pn_18.h"
#include "assets/oot/textures/place_title_cards/g_pn_19.h"
#include "assets/oot/textures/place_title_cards/g_pn_20.h"
#include "assets/oot/textures/place_title_cards/g_pn_21.h"
#include "assets/oot/textures/place_title_cards/g_pn_22.h"
#include "assets/oot/textures/place_title_cards/g_pn_23.h"
#include "assets/oot/textures/place_title_cards/g_pn_24.h"
#include "assets/oot/textures/place_title_cards/g_pn_25.h"
#include "assets/oot/textures/place_title_cards/g_pn_26.h"
#include "assets/oot/textures/place_title_cards/g_pn_27.h"
#include "assets/oot/textures/place_title_cards/g_pn_28.h"
#include "assets/oot/textures/place_title_cards/g_pn_29.h"
#include "assets/oot/textures/place_title_cards/g_pn_30.h"
#include "assets/oot/textures/place_title_cards/g_pn_31.h"
#include "assets/oot/textures/place_title_cards/g_pn_32.h"
#include "assets/oot/textures/place_title_cards/g_pn_33.h"
#include "assets/oot/textures/place_title_cards/g_pn_34.h"
#include "assets/oot/textures/place_title_cards/g_pn_35.h"
#include "assets/oot/textures/place_title_cards/g_pn_36.h"
#include "assets/oot/textures/place_title_cards/g_pn_37.h"
#include "assets/oot/textures/place_title_cards/g_pn_38.h"
#include "assets/oot/textures/place_title_cards/g_pn_39.h"
#include "assets/oot/textures/place_title_cards/g_pn_40.h"
#include "assets/oot/textures/place_title_cards/g_pn_41.h"
#include "assets/oot/textures/place_title_cards/g_pn_42.h"
#include "assets/oot/textures/place_title_cards/g_pn_43.h"
#include "assets/oot/textures/place_title_cards/g_pn_44.h"
#include "assets/oot/textures/place_title_cards/g_pn_45.h"
#include "assets/oot/textures/place_title_cards/g_pn_46.h"
#include "assets/oot/textures/place_title_cards/g_pn_47.h"
#include "assets/oot/textures/place_title_cards/g_pn_48.h"
#include "assets/oot/textures/place_title_cards/g_pn_49.h"
#include "assets/oot/textures/place_title_cards/g_pn_50.h"
#include "assets/oot/textures/place_title_cards/g_pn_51.h"
#include "assets/oot/textures/place_title_cards/g_pn_52.h"
#include "assets/oot/textures/place_title_cards/g_pn_53.h"
#include "assets/oot/textures/place_title_cards/g_pn_54.h"
#include "assets/oot/textures/place_title_cards/g_pn_55.h"
#include "assets/oot/textures/place_title_cards/g_pn_56.h"
#include "assets/oot/textures/place_title_cards/g_pn_57.h"

typedef struct {
    s16 sceneId;
    const char* texture;
} PlaceNameOot;

// OoT's TitleCard_InitPlaceName, in SoH's order. Scenes left out have no card.
static const PlaceNameOot sPlaceNamesOot[] = {
    { SCENE_DEKU_TREE_Oot, gDekuTreeTitleCardENGTex },
    { SCENE_DODONGOS_CAVERN_Oot, gDodongosCavernTitleCardENGTex },
    { SCENE_JABU_JABU_Oot, gJabuJabuTitleCardENGTex },
    { SCENE_FOREST_TEMPLE_Oot, gForestTempleTitleCardENGTex },
    { SCENE_FIRE_TEMPLE_Oot, gFireTempleTitleCardENGTex },
    { SCENE_WATER_TEMPLE_Oot, gWaterTempleTitleCardENGTex },
    { SCENE_SPIRIT_TEMPLE_Oot, gSpiritTempleTitleCardENGTex },
    { SCENE_SHADOW_TEMPLE_Oot, gShadowTempleTitleCardENGTex },
    { SCENE_BOTTOM_OF_THE_WELL_Oot, gBottomOfTheWellTitleCardENGTex },
    { SCENE_ICE_CAVERN_Oot, gIceCavernTitleCardENGTex },
    { SCENE_GERUDO_TRAINING_GROUND_Oot, gGERudoTrainingGroundTitleCardENGTex },
    { SCENE_THIEVES_HIDEOUT_Oot, gThievesHideoutTitleCardENGTex },
    { SCENE_OUTSIDE_GANONS_CASTLE_Oot, gGanonsCastleTitleCardENGTex },
    { SCENE_INSIDE_GANONS_CASTLE_Oot, gInsideGanonsCastleTitleCardENGTex },
    { SCENE_TREASURE_BOX_SHOP_Oot, gTreasureBoxShopTitleCardENGTex },
    { SCENE_BACK_ALLEY_DAY_Oot, gBackAlleyTitleCardENGTex },
    { SCENE_BACK_ALLEY_NIGHT_Oot, gBackAlleyTitleCardENGTex },
    { SCENE_MARKET_DAY_Oot, gMarketTitleCardENGTex },
    { SCENE_MARKET_NIGHT_Oot, gMarketTitleCardENGTex },
    { SCENE_MARKET_RUINS_Oot, gMarketTitleCardENGTex },
    { SCENE_BAZAAR_Oot, gBazaarTitleCardENGTex },
    { SCENE_KOKIRI_SHOP_Oot, gKokiriShopTitleCardENGTex },
    { SCENE_GORON_SHOP_Oot, gGoronShopTitleCardENGTex },
    { SCENE_ZORA_SHOP_Oot, gZoraShopTitleCardENGTex },
    { SCENE_BOMBCHU_SHOP_Oot, gBombchuShopTitleCardENGTex },
    { SCENE_POTION_SHOP_KAKARIKO_Oot, gPotionShopTitleCardENGTex },
    { SCENE_POTION_SHOP_GRANNY_Oot, gPotionShopTitleCardENGTex },
    { SCENE_POTION_SHOP_MARKET_Oot, gPotionShopTitleCardENGTex },
    { SCENE_HAPPY_MASK_SHOP_Oot, gHappyMaskShopTitleCardENGTex },
    { SCENE_STABLE_Oot, gStableTitleCardENGTex },
    { SCENE_LAKESIDE_LABORATORY_Oot, gLakesideLaboratoryTitleCardENGTex },
    { SCENE_GRAVEKEEPERS_HUT_Oot, gGravekeepersHutTitleCardENGTex },
    { SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot, gGreatFairysFountainTitleCardENGTex },
    { SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS_Oot, gGreatFairysFountainTitleCardENGTex },
    { SCENE_FAIRYS_FOUNTAIN_Oot, gFairysFountainTitleCardENGTex },
    { SCENE_ROYAL_FAMILYS_TOMB_Oot, gRoyalFamilysTombTitleCardENGTex },
    { SCENE_SHOOTING_GALLERY_Oot, gShootingGalleryTitleCardENGTex },
    { SCENE_TEMPLE_OF_TIME_Oot, gTempleOfTimeTitleCardENGTex },
    { SCENE_CHAMBER_OF_THE_SAGES_Oot, gChamberOfTheSagesTitleCardENGTex },
    { SCENE_CASTLE_COURTYARD_GUARDS_DAY_Oot, gCastleCourtyardTitleCardENGTex },
    { SCENE_CASTLE_COURTYARD_GUARDS_NIGHT_Oot, gCastleCourtyardTitleCardENGTex },
    { SCENE_CASTLE_COURTYARD_ZELDA_Oot, gCastleCourtyardTitleCardENGTex },
    { SCENE_WINDMILL_AND_DAMPES_GRAVE_Oot, gQuestionMarkTitleCardENGTex },
    { SCENE_FISHING_POND_Oot, gFishingPondTitleCardENGTex },
    { SCENE_BOMBCHU_BOWLING_ALLEY_Oot, gBombchuBowlingAlleyCardENGTex },
    { SCENE_HOUSE_OF_SKULLTULA_Oot, gHouseOfSkulltulaTitleCardENGTex },
    { SCENE_HYRULE_FIELD_Oot, gHyruleFieldTitleCardENGTex },
    { SCENE_KAKARIKO_VILLAGE_Oot, gKakarikoVillageTitleCardENGTex },
    { SCENE_GRAVEYARD_Oot, gGraveyardTitleCardENGTex },
    { SCENE_ZORAS_RIVER_Oot, gZorasRiverTitleCardENGTex },
    { SCENE_KOKIRI_FOREST_Oot, gKokiriForestTitleCardENGTex },
    { SCENE_SACRED_FOREST_MEADOW_Oot, gSacredForestMeadowTitleCardENGTex },
    { SCENE_LAKE_HYLIA_Oot, gLakeHyliaTitleCardENGTex },
    { SCENE_ZORAS_DOMAIN_Oot, gZorasDomainTitleCardENGTex },
    { SCENE_ZORAS_FOUNTAIN_Oot, gZorasFountainTitleCardENGTex },
    { SCENE_GERUDO_VALLEY_Oot, gGERudoValleyTitleCardENGTex },
    { SCENE_LOST_WOODS_Oot, gLostWoodsTitleCardENGTex },
    { SCENE_DESERT_COLOSSUS_Oot, gDesertColossusTitleCardENGTex },
    { SCENE_GERUDOS_FORTRESS_Oot, gGERudosFortressTitleCardENGTex },
    { SCENE_HAUNTED_WASTELAND_Oot, gHauntedWastelandTitleCardENGTex },
    { SCENE_HYRULE_CASTLE_Oot, gHyruleCastleTitleCardENGTex },
    { SCENE_DEATH_MOUNTAIN_TRAIL_Oot, gDeathMountainTrailTitleCardENGTex },
    { SCENE_DEATH_MOUNTAIN_CRATER_Oot, gDeathMountainCraterTitleCardENGTex },
    { SCENE_GORON_CITY_Oot, gGoronCityTitleCardENGTex },
    { SCENE_LON_LON_RANCH_Oot, gLonLonRanchTitleCardENGTex },
};

void Hyrule_InitPlaceNameOot(PlayState* play) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sPlaceNamesOot); i++) {
        if (sPlaceNamesOot[i].sceneId == play->sceneId) {
            TitleCard_InitPlaceName(&play->state, &play->actorCtx.titleCtx, (TexturePtr)sPlaceNamesOot[i].texture, 160,
                                    120, 144, 24, 20);
            return;
        }
    }
}

s32 Hyrule_ShowsPlaceNameOot(PlayState* play) {
    // OoT's Player_Init: Dodongo's Cavern is named only once Link has been in it, and the Bombchu Shop only once it
    // has opened.
    if ((play->sceneId == SCENE_DODONGOS_CAVERN_Oot) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_ENTERED_DODONGOS_CAVERN_Oot)) {
        return false;
    }
    if ((play->sceneId == SCENE_BOMBCHU_SHOP_Oot) &&
        !GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_DODONGOS_CAVERN_BLUE_WARP_Oot)) {
        return false;
    }
    return true;
}
