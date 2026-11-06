// A new file's start in OoT's opening, and saving in Hyrule. See docs/TERMINA.md and HyruleSave.h.
//
// A file's last permanent save remembers its scene (savedSceneId). When that scene is OoT's, the file reloads as
// OoT's do (Sram_OpenSave in OoT), at the time it was saved; MM's own saves forget it and reload in Termina.

#include "global.h"
#include "HyruleSave.h"
#include "HyruleInventory.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"

// OoT's entrances.
#define ENTR_LINKS_HOUSE_CHILD_SPAWN_Oot 0x0BB
#define ENTR_TEMPLE_OF_TIME_WARP_PAD_Oot 0x5F4
#define ENTR_GANONS_TOWER_0_Oot 0x41B

// OoT's scenes, by OoT's ids.
#define SCENE_INSIDE_GANONS_CASTLE_Oot_ID 0x0D
#define SCENE_GANONS_TOWER_COLLAPSE_INTERIOR_Oot_ID 0x0E
#define SCENE_INSIDE_GANONS_CASTLE_COLLAPSE_Oot_ID 0x0F
#define SCENE_DEKU_TREE_BOSS_Oot_ID 0x11
#define SCENE_SHADOW_TEMPLE_BOSS_Oot_ID 0x18
#define SCENE_GANONDORF_BOSS_Oot_ID 0x19
#define SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR_Oot_ID 0x1A
#define SCENE_LINKS_HOUSE_Oot_ID 0x34
#define SCENE_GANON_BOSS_Oot_ID 0x4F

// OoT's sDungeonEntrances: a dungeon's entrance, by OoT's scene id, up to Inside Ganon's Castle.
static const u16 sDungeonEntrancesOot[] = {
    0x000, // SCENE_DEKU_TREE
    0x004, // SCENE_DODONGOS_CAVERN
    0x028, // SCENE_JABU_JABU
    0x169, // SCENE_FOREST_TEMPLE
    0x165, // SCENE_FIRE_TEMPLE
    0x010, // SCENE_WATER_TEMPLE
    0x082, // SCENE_SPIRIT_TEMPLE
    0x037, // SCENE_SHADOW_TEMPLE
    0x098, // SCENE_BOTTOM_OF_THE_WELL
    0x088, // SCENE_ICE_CAVERN
    0x41B, // SCENE_GANONS_TOWER
    0x008, // SCENE_GERUDO_TRAINING_GROUND
    0x486, // SCENE_THIEVES_HIDEOUT
    0x467, // SCENE_INSIDE_GANONS_CASTLE
};

// The OoT entrance a file saved in an OoT scene reloads at (OoT's Sram_OpenSave).
static u16 Hyrule_GetLoadEntranceOot(s32 ootSceneId) {
    if (ootSceneId <= SCENE_INSIDE_GANONS_CASTLE_Oot_ID) {
        return sDungeonEntrancesOot[ootSceneId];
    }
    if ((ootSceneId >= SCENE_DEKU_TREE_BOSS_Oot_ID) && (ootSceneId <= SCENE_SHADOW_TEMPLE_BOSS_Oot_ID)) {
        // A boss's room reloads at its dungeon's entrance, in the order of the dungeons.
        return sDungeonEntrancesOot[ootSceneId - SCENE_DEKU_TREE_BOSS_Oot_ID];
    }
    switch (ootSceneId) {
        case SCENE_GANONS_TOWER_COLLAPSE_INTERIOR_Oot_ID:
        case SCENE_INSIDE_GANONS_CASTLE_COLLAPSE_Oot_ID:
        case SCENE_GANONDORF_BOSS_Oot_ID:
        case SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR_Oot_ID:
        case SCENE_GANON_BOSS_Oot_ID:
            return ENTR_GANONS_TOWER_0_Oot;
        case SCENE_LINKS_HOUSE_Oot_ID:
            return ENTR_LINKS_HOUSE_CHILD_SPAWN_Oot;
        default:
            return LINK_IS_ADULT_OOT ? ENTR_TEMPLE_OF_TIME_WARP_PAD_Oot : ENTR_LINKS_HOUSE_CHILD_SPAWN_Oot;
    }
}

void Hyrule_InitNewFileOot(void) {
    // OoT's Link starts with nothing: MM's new file gives the Kokiri Sword and the Hero's Shield, which Link earns
    // in Hyrule (the Hero's Shield last, from the Happy Mask Salesman: DESIGN.md's Termina gate).
    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_NONE;
    SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_NONE);
    SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_NONE);
    Hyrule_InitInventoryOot();

    // OoT's Sram_InitSave: Link's house, with the opening (the Deku Tree sends Navi), at OoT's first time of day.
    gSaveContext.save.saveInfo.playerData.savedSceneId = SCENE_LINKS_HOUSE_Oot;
    gSaveContext.save.cutsceneIndex = 0xFFF1;
    gSaveContext.save.time = 0x6AAB;
}

s32 Hyrule_OpenSaveOot(void) {
    s32 ootSceneId = Hyrule_GetOotSceneId(gSaveContext.save.saveInfo.playerData.savedSceneId);
    u32 entrance;

    if (ootSceneId < 0) {
        return false;
    }
    entrance = Hyrule_EntranceFromOot(Hyrule_GetLoadEntranceOot(ootSceneId));
    if (entrance == HYRULE_ENTRANCE_NONE) {
        entrance = Hyrule_EntranceFromOot(ENTR_LINKS_HOUSE_CHILD_SPAWN_Oot);
    }
    gSaveContext.save.entrance = entrance;
    // The file's own cutscene index plays: OoT's opening (0xFFF1) on a new file, none on a saved one. MM's new file
    // leaves a pending index of 0 (Sram_InitNewSave), which Play_Init would put in its place.
    gSaveContext.nextCutsceneIndex = 0xFFEF;
    // OoT gives back three hearts to a file saved with fewer (OoT's Sram_OpenSave).
    if (gSaveContext.save.saveInfo.playerData.health < 0x30) {
        gSaveContext.save.saveInfo.playerData.health = 0x30;
    }
    return true;
}

s32 Hyrule_CanPauseSaveOot(PlayState* play) {
    return Hyrule_GetOotSceneId(play->sceneId) >= 0;
}

s32 Hyrule_IsPermanentPauseSaveOot(PlayState* play) {
    return Hyrule_GetOotSceneId(play->sceneId) >= 0;
}

void Hyrule_ForgetSavedSceneOot(void) {
    if (Hyrule_GetOotSceneId(gSaveContext.save.saveInfo.playerData.savedSceneId) >= 0) {
        gSaveContext.save.saveInfo.playerData.savedSceneId = SCENE_SPOT00;
    }
}
