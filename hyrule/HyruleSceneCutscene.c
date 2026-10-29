// OoT's scene cutscenes (docs/CUTSCENES.md step 2), played by OoT's interpreter (HyruleCutscene.c): a scene's
// cutscene-layer script, and OoT's entrance cutscenes (the first-visit introductions).

#include "global.h"
#include "BenPort.h"
#include "HyruleCutscene.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"
#include <string.h>

#define CS_PATH_OOT(scene, name) "oot/scenes/shared/" scene "/" name
#define CS_PATH_NONMQ_OOT(scene, name) "oot/scenes/nonmq/" scene "/" name

// OoT's sEntranceCutsceneTable (z_demo.c), by OoT entrance index.
static const EntranceCutsceneOot sEntranceCutsceneTableOot[] = {
    { 0x185, 2, EVENTCHKINF_ENTERED_HYRULE_FIELD_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldIntroCs") },
    { 0x13D, 2, EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_TRAIL_Oot, CS_PATH_OOT("spot16_scene", "gDMTIntroCs") },
    { 0x0DB, 2, EVENTCHKINF_ENTERED_KAKARIKO_VILLAGE_Oot, CS_PATH_OOT("spot01_scene", "gKakarikoVillageIntroCs") },
    { 0x108, 2, EVENTCHKINF_ENTERED_ZORAS_DOMAIN_Oot, CS_PATH_OOT("spot07_scene", "gZorasDomainIntroCs") },
    { 0x138, 1, EVENTCHKINF_ENTERED_HYRULE_CASTLE_Oot, CS_PATH_OOT("spot15_scene", "gHyruleCastleIntroCs") },
    { 0x14D, 2, EVENTCHKINF_ENTERED_GORON_CITY_Oot, CS_PATH_OOT("spot18_scene", "gGoronCityIntroCs") },
    { 0x053, 2, EVENTCHKINF_ENTERED_TEMPLE_OF_TIME_Oot, CS_PATH_OOT("tokinoma_scene", "gTempleOfTimeIntroCs") },
    { 0x000, 2, EVENTCHKINF_ENTERED_DEKU_TREE_Oot, CS_PATH_NONMQ_OOT("ydan_scene", "gDekuTreeIntroCs") },
    { 0x28A, 0, EVENTCHKINF_EPONA_OBTAINED_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldSouthEponaJumpCs") },
    { 0x292, 0, EVENTCHKINF_EPONA_OBTAINED_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldEastEponaJumpCs") },
    { 0x28E, 0, EVENTCHKINF_EPONA_OBTAINED_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldWestEponaJumpCs") },
    { 0x476, 0, EVENTCHKINF_EPONA_OBTAINED_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldGateEponaJumpCs") },
    { 0x50F, 1, EVENTCHKINF_LEARNED_SONG_OF_TIME_Oot, CS_PATH_OOT("spot00_scene", "gHyruleFieldGetOoTCs") },
    { 0x102, 2, EVENTCHKINF_ENTERED_LAKE_HYLIA_Oot, CS_PATH_OOT("spot06_scene", "gLakeHyliaIntroCs") },
    { 0x117, 2, EVENTCHKINF_ENTERED_GERUDO_VALLEY_Oot, CS_PATH_OOT("spot09_scene", "gGerudoValleyIntroCs") },
    { 0x129, 2, EVENTCHKINF_ENTERED_GERUDOS_FORTRESS_Oot, CS_PATH_OOT("spot12_scene", "gGerudoFortressIntroCs") },
    { 0x157, 2, EVENTCHKINF_ENTERED_LON_LON_RANCH_Oot, CS_PATH_OOT("spot20_scene", "gLonLonRanchIntroCs") },
    { 0x028, 2, EVENTCHKINF_ENTERED_JABU_JABUS_BELLY_Oot, CS_PATH_NONMQ_OOT("bdan_scene", "gJabuJabuIntroCs") },
    { 0x0E4, 2, EVENTCHKINF_ENTERED_GRAVEYARD_Oot, CS_PATH_OOT("spot02_scene", "gGraveyardIntroCs") },
    { 0x225, 2, EVENTCHKINF_ENTERED_ZORAS_FOUNTAIN_Oot, CS_PATH_OOT("spot08_scene", "gZorasFountainIntroCs") },
    { 0x123, 2, EVENTCHKINF_ENTERED_DESERT_COLOSSUS_Oot, CS_PATH_OOT("spot11_scene", "gDesertColossusIntroCs") },
    { 0x147, 2, EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_CRATER_Oot,
      CS_PATH_OOT("spot17_scene", "gDeathMountainCraterIntroCs") },
    { 0x138, 0, EVENTCHKINF_ENTERED_GANONS_CASTLE_EXTERIOR_Oot, CS_PATH_OOT("ganon_tou_scene", "gGanonsCastleIntroCs") },
    { 0x574, 2, EVENTCHKINF_LEARNED_SUNS_SONG_Oot,
      CS_PATH_OOT("hakaana_ouke_scene", "gSunSongGraveSunSongTeachPart2Cs") },
    { 0x538, 2, EVENTCHKINF_COMPLETED_FOREST_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gForestBarrierCs") },
    { 0x53C, 2, EVENTCHKINF_COMPLETED_WATER_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gWaterBarrierCs") },
    { 0x540, 2, EVENTCHKINF_COMPLETED_SHADOW_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gShadowBarrierCs") },
    { 0x544, 2, EVENTCHKINF_COMPLETED_FIRE_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gFireBarrierCs") },
    { 0x548, 2, EVENTCHKINF_COMPLETED_LIGHT_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gLightBarrierCs") },
    { 0x54C, 2, EVENTCHKINF_COMPLETED_SPIRIT_TRIAL_Oot, CS_PATH_NONMQ_OOT("ganontika_scene", "gSpiritBarrierCs") },
    { 0x08D, 0, EVENTCHKINF_NABOORU_ORDERED_TO_FIGHT_BY_TWINROVA_Oot,
      CS_PATH_OOT("jyasinboss_scene", "gSpiritBossNabooruKnuckleIntroCs") },
    { 0x3B4, 0, EVENTCHKINF_WATCHED_GANONS_CASTLE_COLLAPSE_CAUGHT_BY_GERUDO_Oot,
      CS_PATH_OOT("spot12_scene", "gGerudoFortressFirstCaptureCs") },
    { 0x246, 2, EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_CRATER_Oot,
      CS_PATH_OOT("spot17_scene", "gDeathMountainCraterIntroCs") },
    { 0x5E8, 2, EVENTCHKINF_SPOKE_TO_DEKU_TREE_SPROUT_Oot, CS_PATH_OOT("spot04_scene", "gKokiriForestDekuSproutCs") },
};

// The first-visit introductions' flags, which the cycle reset keeps (C2).
static const u16 sIntroFlagsOot[] = {
    EVENTCHKINF_ENTERED_HYRULE_FIELD_Oot,       EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_TRAIL_Oot,
    EVENTCHKINF_ENTERED_KAKARIKO_VILLAGE_Oot,   EVENTCHKINF_ENTERED_ZORAS_DOMAIN_Oot,
    EVENTCHKINF_ENTERED_HYRULE_CASTLE_Oot,      EVENTCHKINF_ENTERED_GORON_CITY_Oot,
    EVENTCHKINF_ENTERED_TEMPLE_OF_TIME_Oot,     EVENTCHKINF_ENTERED_DEKU_TREE_Oot,
    EVENTCHKINF_ENTERED_LAKE_HYLIA_Oot,         EVENTCHKINF_ENTERED_GERUDO_VALLEY_Oot,
    EVENTCHKINF_ENTERED_GERUDOS_FORTRESS_Oot,   EVENTCHKINF_ENTERED_LON_LON_RANCH_Oot,
    EVENTCHKINF_ENTERED_JABU_JABUS_BELLY_Oot,   EVENTCHKINF_ENTERED_GRAVEYARD_Oot,
    EVENTCHKINF_ENTERED_ZORAS_FOUNTAIN_Oot,     EVENTCHKINF_ENTERED_DESERT_COLOSSUS_Oot,
    EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_CRATER_Oot, EVENTCHKINF_ENTERED_GANONS_CASTLE_EXTERIOR_Oot,
};

// OoT's Cutscene_HandleEntranceTriggers.
static void Hyrule_HandleEntranceTriggersOot(PlayState* play) {
    const EntranceCutsceneOot* entranceCutscene;
    u8 requiredAge;
    s16 i;

    for (i = 0; i < ARRAY_COUNT(sEntranceCutsceneTableOot); i++) {
        entranceCutscene = &sEntranceCutsceneTableOot[i];
        requiredAge = entranceCutscene->ageRestriction;
        if (requiredAge == 2) {
            requiredAge = LINK_AGE_OOT;
        }

        if ((gSaveContext.save.entrance == Hyrule_EntranceFromOot(entranceCutscene->entrance)) &&
            (!GET_EVENTCHKINF_OOT(entranceCutscene->flag) ||
             (entranceCutscene->flag == EVENTCHKINF_EPONA_OBTAINED_Oot)) &&
            (LINK_AGE_OOT == requiredAge) && (gSaveContext.respawnFlag <= 0)) {
            SET_EVENTCHKINF_OOT(entranceCutscene->flag);
            gCsCtxOot.segment = ResourceMgr_LoadCSByName(entranceCutscene->script);
            Hyrule_TriggerCutsceneOot(2);
            gSaveContext.showTitleCard = false;
            break;
        }
    }
}

void Hyrule_HandleSceneCutscenesOot(PlayState* play) {
    if (Hyrule_GetOotSceneId(play->sceneId) < 0) {
        return;
    }
    // A cutscene layer (OoT's cutscene index of 0xFFF0 and up) plays its script, which OTRExporter puts first in MM's
    // script list; OoT's entrance cutscenes wait for a layer without one.
    if (gSaveContext.sceneLayer >= SCENE_LAYER_CUTSCENE_FIRST_OOT) {
        if (play->csCtx.scriptListCount != 0) {
            Hyrule_StartSceneCutsceneOot(play, play->csCtx.scriptList[0].script);
        }
    } else {
        Hyrule_HandleEntranceTriggersOot(play);
    }
}

void Hyrule_ResetEventChkInfOotForCycle(void) {
    u16 kept[ARRAY_COUNT(sIntroFlagsOot)];
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sIntroFlagsOot); i++) {
        kept[i] = GET_EVENTCHKINF_OOT(sIntroFlagsOot[i]);
    }
    memset(gSaveContext.save.saveInfo.eventChkInfOot, 0, sizeof(gSaveContext.save.saveInfo.eventChkInfOot));
    for (i = 0; i < ARRAY_COUNT(sIntroFlagsOot); i++) {
        if (kept[i]) {
            SET_EVENTCHKINF_OOT(sIntroFlagsOot[i]);
        }
    }
}
