// The way from Hyrule to Termina. See HyruleTermina.h and docs/TERMINA.md.

#include "global.h"
#include "HyruleTermina.h"
#include "HyruleFlags.h"
#include "HyruleInventory.h"
#include "HyruleIds.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"

s32 Hyrule_IsTerminaUnlocked(void) {
    return GET_EVENTCHKINF_OOT(EVENTCHKINF_TERMINA_UNLOCKED_3SHIP);
}

void Hyrule_UnlockTermina(void) {
    SET_EVENTCHKINF_OOT(EVENTCHKINF_TERMINA_UNLOCKED_3SHIP);
}

s32 Hyrule_OwnsTerminaGateItems(void) {
    return (INV_CONTENT(ITEM_OCARINA_OF_TIME) == ITEM_OCARINA_OF_TIME) &&
           CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI) && CHECK_QUEST_ITEM(QUEST_SONG_EPONA);
}

void Hyrule_GiveMaskShopGift(PlayState* play) {
    Item_Give(play, ITEM_MASK_GORON_Oot);
    Item_Give(play, ITEM_MASK_ZORA_Oot);
    Item_Give(play, ITEM_MASK_GERUDO_Oot);
}

s32 Hyrule_HasEpona(void) {
    return CHECK_QUEST_ITEM(QUEST_SONG_EPONA) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_STOLEN_3SHIP);
}

void Hyrule_RecoverEpona(void) {
    CLEAR_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_STOLEN_3SHIP);
}

// The Song of Healing has given Link the Deku Mask.
static s32 Hyrule_IsDekuCurseLifted(void) {
    return INV_CONTENT(ITEM_MASK_DEKU) == ITEM_MASK_DEKU;
}

// MM's intro: the Skull Kid takes Epona and the Ocarina of Time, which Link gets back in the Clock Tower.
static void Hyrule_StealForMmIntro(void) {
    s32 form;
    s32 button;

    SET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_STOLEN_3SHIP);
    INV_CONTENT(ITEM_OCARINA_OF_TIME) = ITEM_NONE;
    for (form = 0; form < ARRAY_COUNT(gSaveContext.save.saveInfo.equips.buttonItems); form++) {
        for (button = EQUIP_SLOT_C_LEFT; button <= EQUIP_SLOT_C_RIGHT; button++) {
            if (BUTTON_ITEM_EQUIP(form, button) == ITEM_OCARINA_OF_TIME) {
                BUTTON_ITEM_EQUIP(form, button) = ITEM_NONE;
                C_SLOT_EQUIP(form, button) = SLOT_NONE;
            }
        }
    }
}

// An exit of the passage between the worlds.
static void Hyrule_TakeWayBackExit(PlayState* play, u32 entrance) {
    play->nextEntrance = entrance;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
}

s32 Hyrule_TakeTerminaPassage(PlayState* play, s32 exitIndex) {
    Player* player = GET_PLAYER(play);

    // MM's hollow log on later trips: without the curse's cutscene, which carries Link from the dark ledge he falls
    // onto down to the opening dungeon's landing spot, he arrives on the landing spot itself (spawn 2).
    if ((play->sceneId == SCENE_LOST_WOODS) && gSaveContext.save.isFirstCycle &&
        (play->setupExitList[exitIndex] == ENTRANCE(OPENING_DUNGEON, 0))) {
        Hyrule_TakeWayBackExit(play, ENTRANCE(OPENING_DUNGEON, 2));
        return true;
    }

    // The first clearing's straight-ahead exit is on its -z side; the way back to Kokiri Forest, on its +z side, also
    // leads there.
    if ((play->sceneId != SCENE_LOST_WOODS_Oot) || (play->roomCtx.curRoom.num != 0) ||
        (player->actor.world.pos.z > 0.0f) ||
        (ENTRANCE_GET_SCENE(play->setupExitList[exitIndex]) != ENTR_SCENE_KOKIRI_FOREST_Oot)) {
        return false;
    }
    if (!Hyrule_IsTerminaUnlocked()) {
        return false;
    }
    if (!gSaveContext.save.isFirstCycle) {
        // The first passage, child Link only (docs/TERMINA.md T2): MM's intro from its Lost Woods ride (the cutscene
        // layer), without the opening narration of the Hero of Time.
        if (LINK_IS_ADULT_OOT) {
            return false;
        }
        Hyrule_StealForMmIntro();
        gSaveContext.nextCutsceneIndex = 0xFFF0;
    } else if (!Hyrule_IsDekuCurseLifted()) {
        // Link has been to Clock Town but the way stays shut until the Deku curse is lifted.
        return false;
    }
    // Later passages walk MM's intro areas without their cutscenes, at either age (T6).
    play->nextEntrance = ENTRANCE(LOST_WOODS, 0);
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
    return true;
}

s32 Hyrule_SkipsIntroCutscene(PlayState* play) {
    if (!gSaveContext.save.isFirstCycle) {
        return false;
    }
    switch (play->sceneId) {
        case SCENE_LOST_WOODS:
        case SCENE_OPENINGDAN:
            return true;
        case SCENE_INSIDETOWER:
            // Its spawn 0 is Link's arrival from the corridor, only used by the intro in MM.
            return ((gSaveContext.save.entrance >> 4) & 0x1F) == 0;
        default:
            return false;
    }
}

// The opening dungeon's landing spot, where Link lands from MM's Lost Woods (its spawn 0 falls onto it). It carries
// him back up once he has walked away from it and returns, so that landing on it does not.
static Vec3f sOpeningDungeonLandingSpot = { 0.0f, 0.0f, 309.0f };
#define OPENING_DUNGEON_LANDING_RADIUS 50.0f
#define OPENING_DUNGEON_LANDING_ARM_RADIUS 150.0f
static s32 sLandingSpotArmed = false;

// MM's Lost Woods: the clearing's west edge, behind the spot where Link wakes, is the path he came by.
#define LOST_WOODS_PATH_END_X -1000.0f

// OoT's ENTR_LOST_WOODS_3: OoT's Lost Woods' first clearing, arriving through its straight-ahead opening.
#define ENTR_LOST_WOODS_FIRST_CLEARING_AHEAD_Oot 0x1B1

void Hyrule_UpdateTerminaPassage(PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (play->sceneId != SCENE_OPENINGDAN) {
        sLandingSpotArmed = false;
    }
    if (!gSaveContext.save.isFirstCycle || !Hyrule_IsDekuCurseLifted() || (play->transitionTrigger != TRANS_TRIGGER_OFF) ||
        (play->transitionMode != TRANS_MODE_OFF) || Play_InCsMode(play) || (player->stateFlags1 & PLAYER_STATE1_DEAD)) {
        return;
    }
    switch (play->sceneId) {
        case SCENE_OPENINGDAN: {
            f32 dist = Math_Vec3f_DistXZ(&player->actor.world.pos, &sOpeningDungeonLandingSpot);

            if (dist > OPENING_DUNGEON_LANDING_ARM_RADIUS) {
                sLandingSpotArmed = true;
            } else if (sLandingSpotArmed && (dist < OPENING_DUNGEON_LANDING_RADIUS) &&
                       (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                Hyrule_TakeWayBackExit(play, ENTRANCE(LOST_WOODS, 1));
            }
            break;
        }
        case SCENE_LOST_WOODS:
            if (player->actor.world.pos.x < LOST_WOODS_PATH_END_X) {
                Hyrule_TakeWayBackExit(play, Hyrule_EntranceFromOot(ENTR_LOST_WOODS_FIRST_CLEARING_AHEAD_Oot));
            }
            break;
        default:
            break;
    }
}
