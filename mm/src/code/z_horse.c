#include "global.h"
#include "HyruleTermina.h"
#include "z64horse.h"
#include "overlays/actors/ovl_Bg_Umajump/z_bg_umajump.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"
#include "HyruleScenes.h"
#include "HyruleFlags.h"

s32 Horse_GetJumpingFencePathIndex(PlayState* play) {
    Actor* actor;

    for (actor = NULL;; actor = actor->next) {
        actor = SubS_FindActor(play, actor, ACTORCAT_PROP, ACTOR_BG_UMAJUMP);

        if (actor == NULL) {
            break;
        }

        if (actor->params == BG_UMAJUMP_TYPE_2) {
            return ((BgUmajump*)actor)->pathIndex;
        }
    }

    return -1;
}

// unused
s32 Horse_CopyPointFromPathList(PlayState* play, s32 pathIndex, s32 pointIndex, Vec3s* dst, s16* arg4) {
    Path* path = &play->setupPathList[pathIndex];
    Vec3s* points;
    s32 count = path->count;

    dst->x = 0;
    dst->y = 0;
    dst->z = 0;
    *arg4 = 0;

    if (count == 0) {
        return false;
    }
    if (pointIndex >= count) {
        return false;
    }

    points = Lib_SegmentedToVirtual(path->points);
    points += pointIndex;

    dst->x = points->x;
    dst->y = points->y;
    dst->z = points->z;
    *arg4 = 0;

    return true;
}

typedef struct HorseValidScene {
    /* 0x0 */ s16 sceneId;
    /* 0x2 */ s16 sceneLayerMinusOne;
} HorseValidScene; // size = 0x4

HorseValidScene sHorseValidScenes[] = {
    { SCENE_00KEIKOKU, 0 },      // Termina Field
    { SCENE_24KEMONOMITI, 0 },   // Road to southern swap
    { SCENE_F01, 0 },            // Romani ranch
    { SCENE_KOEPONARACE, 0 },    // Gorman track
    { SCENE_20SICHITAI, 0 },     // Southern swamp poisoned
    { SCENE_20SICHITAI2, 0 },    // Souther swamp clear
    { SCENE_30GYOSON, 0 },       // Great bay coast
    { SCENE_31MISAKI, 0 },       // Zora cape
    { SCENE_ROMANYMAE, 0 },      // Milk road
    { SCENE_IKANAMAE, 0 },       // Road to Ikana
    { SCENE_13HUBUKINOMITI, 0 }, // Path to Mountain Village
    // 3Ship [Hyrule] OoT's horse scenes, where Epona's Song calls her for both ages (docs/EPONA.md).
    { SCENE_HYRULE_FIELD_Oot, 0 },
    { SCENE_LAKE_HYLIA_Oot, 0 },
    { SCENE_GERUDO_VALLEY_Oot, 0 },
    { SCENE_GERUDOS_FORTRESS_Oot, 0 },
    { SCENE_LON_LON_RANCH_Oot, 0 },
};

s32 Horse_IsValidSpawn(s16 sceneId) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sHorseValidScenes); i++) {
        if (sceneId == sHorseValidScenes[i].sceneId) {
            return true;
        }
    }

    return false;
}

void Horse_ResetHorseData(PlayState* play) {
    gSaveContext.save.saveInfo.horseData.sceneId = SCENE_F01;
    gSaveContext.save.saveInfo.horseData.pos.x = -1420;
    gSaveContext.save.saveInfo.horseData.pos.y = 257;
    gSaveContext.save.saveInfo.horseData.pos.z = -1285;
    gSaveContext.save.saveInfo.horseData.yaw = 0x2AAA;
}

s32 gHorseIsMounted = false;
s32 D_801BDAA0 = false;
s32 gHorsePlayedEponasSong = false;

HorseValidScene sHorseValidSceneLayers[] = {
    { SCENE_00KEIKOKU, 5 - 1 },      // Termina Field - First Cycle
    { SCENE_30GYOSON, 1 - 1 },       // Great Bay Coast - Post-Gyorg
    { SCENE_31MISAKI, 1 - 1 },       // Zora Cape - Post-Gyorg
    { SCENE_13HUBUKINOMITI, 1 - 1 }, // Path to Mountain Village - Post-Goht
};

s32 Horse_IsValidSceneLayer(PlayState* play, Player* player) {
    s32 i;

    if (gSaveContext.sceneLayer == 0) {
        return true;
    }
    // 3Ship [Hyrule] OoT's layers 0 to 3 are its child and adult day and night (its cutscene layers follow).
    if ((Hyrule_GetOotSceneId(play->sceneId) >= 0) && (gSaveContext.sceneLayer < SCENE_LAYER_CUTSCENE_FIRST_OOT)) {
        return true;
    }

    for (i = 0; i < ARRAY_COUNT(sHorseValidSceneLayers); i++) {
        if ((sHorseValidSceneLayers[i].sceneId == play->sceneId) &&
            (gSaveContext.sceneLayer == sHorseValidSceneLayers[i].sceneLayerMinusOne + 1)) {
            return true;
        }
    }
    return false;
}

void Horse_SpawnOverworld(PlayState* play, Player* player) {
    if (!Horse_IsValidSceneLayer(play, player)) {
        return;
    }

    // 3Ship [Hyrule] Epona answers unless the Skull Kid has her (docs/TERMINA.md)
    if (gHorseIsMounted && Hyrule_HasEpona()) {
        s32 pad;
        Vec3f pos = player->actor.world.pos;
        f32 yIntersect;
        CollisionPoly* poly;
        s32 pad2[3];

        pos.y += 5.0f;
        yIntersect = BgCheck_EntityRaycastFloor1(&play->colCtx, &poly, &pos);
        if (yIntersect == BGCHECK_Y_MIN) {
            yIntersect = player->actor.world.pos.y;
        }

        player->rideActor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, player->actor.world.pos.x, yIntersect,
                                        player->actor.world.pos.z, player->actor.shape.rot.x, player->actor.shape.rot.y,
                                        player->actor.shape.rot.z, ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_11));
        Player_MountHorse(play, player, player->rideActor);
        Player_SetCameraHorseSetting(play, player);
    } else if ((play->sceneId == gSaveContext.save.saveInfo.horseData.sceneId) && Hyrule_HasEpona()) {
        if (Horse_IsValidSpawn(gSaveContext.save.saveInfo.horseData.sceneId)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, gSaveContext.save.saveInfo.horseData.pos.x,
                        gSaveContext.save.saveInfo.horseData.pos.y, gSaveContext.save.saveInfo.horseData.pos.z, 0,
                        gSaveContext.save.saveInfo.horseData.yaw, 0, ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_1));
        } else {
            Horse_ResetHorseData(play);
        }
    } else if ((play->sceneId == SCENE_LON_LON_RANCH_Oot) && !CHECK_QUEST_ITEM(QUEST_SONG_EPONA)) {
        // 3Ship [Hyrule] Epona waits at Lon Lon Ranch, where OoT has the foal, until Link knows her song (EPONA.md E2).
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, -4.0f, 0.0f, -572.0f, 0, 0x4000, 0,
                    ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_1));
    } else if ((play->sceneId == SCENE_F01) && !Hyrule_HasEpona()) { // 3Ship [Hyrule] and while stolen
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, -1420.0f, 257.0f, -1285.0f, 0, 0x2AAA, 0,
                    ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_1));
    } else if (Hyrule_HasEpona() && Horse_IsValidSpawn(play->sceneId)) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, player->actor.world.pos.x, player->actor.world.pos.y,
                    player->actor.world.pos.z, 0, player->actor.shape.rot.y, 0,
                    ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_2));
    }
}

void Horse_SpawnMinigame(PlayState* play, Player* player) {
    if ((play->sceneId == SCENE_KOEPONARACE) &&
        (GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_START)) {
        player->rideActor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, -1262.0f, -106.0f, 470.0f, 0, 0x7FFF, 0,
                                        ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_13));
        Player_MountHorse(play, player, player->rideActor);
        Player_SetCameraHorseSetting(play, player);
    } else if ((play->sceneId == SCENE_KOEPONARACE) &&
               ((GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_3) ||
                (GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_2))) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, -1741.0f, -106.0f, -641.0f, 0, -0x4FA4, 0,
                    ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_1));
    } else if ((gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0)) && (Cutscene_GetSceneLayer(play) != 0) &&
               (player->transformation == PLAYER_FORM_HUMAN)) {
        player->rideActor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, -1106.0f, 260.0f, -1185.0f, 0, 0x13, 0,
                                        ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_7));
        Player_MountHorse(play, player, player->rideActor);
        Player_SetCameraHorseSetting(play, player);
    }
}

// 3Ship [Hyrule] OoT's horseback archery (its Horse_SetupInCutscene): the Gerudo's game reloads Gerudo's Fortress in
// its cutscene layer, where Link starts the course on Epona (docs/EPONA.md E4: either age).
static s32 Horse_SpawnArcheryOot(PlayState* play, Player* player) {
    if ((play->sceneId != SCENE_GERUDOS_FORTRESS_Oot) || (gSaveContext.sceneLayer != SCENE_LAYER_CUTSCENE_FIRST_OOT) ||
        !(gSaveContext.eventInfOot[0] & 0x100)) {
        return false;
    }
    player->rideActor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, 3600.0f, 1413.0f, 360.0f, 0, 0x8001, 0,
                                    ENHORSE_PARAMS(ENHORSE_PARAM_4000, ENHORSE_10));
    if (player->rideActor == NULL) {
        return false;
    }
    player->rideActor->room = -1;
    player->actor.world.pos = player->rideActor->world.pos;
    player->actor.shape.rot.x = player->actor.shape.rot.z = 0;
    player->actor.shape.rot.y = 0x8001;
    Player_MountHorse(play, player, player->rideActor);
    Player_SetCameraHorseSetting(play, player);
    return true;
}

// 3Ship [Hyrule] OoT's race with Ingo at Lon Lon Ranch (its Horse_SetupInCutscene): its cutscene layer starts the race
// with Link mounted, Epona or the rented horse (eventInfOot bit 0x10, OoT's horse type), and the second race's win brings
// him back mounted (race state 6). docs/EPONA.md E3.
static s32 Horse_SpawnRaceOot(PlayState* play, Player* player) {
    Vec3f pos;
    s16 yaw;
    s32 params;

    if (play->sceneId != SCENE_LON_LON_RANCH_Oot) {
        return false;
    }
    if (gSaveContext.sceneLayer == SCENE_LAYER_CUTSCENE_FIRST_OOT) {
        pos.x = -250.0f;
        pos.y = 1.0f;
        pos.z = -1580.0f;
        yaw = 0x4000;
        // OoT's type 6, MM's ENHORSE_8 (it cannot jump).
        params = ENHORSE_PARAMS((gSaveContext.eventInfOot[0] & 0x10) ? ENHORSE_PARAM_HNI_OOT : ENHORSE_PARAM_4000,
                                ENHORSE_8);
    } else if (((gSaveContext.eventInfOot[0] & 0xF) == 6) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
        pos.x = 894.0f;
        pos.y = 0.0f;
        pos.z = -2084.0f;
        yaw = -0x7FFF;
        // OoT's type 5, a plain ridden horse (MM's ENHORSE_17).
        params = ENHORSE_PARAMS((gSaveContext.eventInfOot[0] & 0x10) ? ENHORSE_PARAM_HNI_OOT : ENHORSE_PARAM_4000,
                                ENHORSE_17);
    } else {
        return false;
    }
    player->rideActor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_HORSE, pos.x, pos.y, pos.z, 0, yaw, 0, params);
    if (player->rideActor == NULL) {
        return false;
    }
    player->actor.world.pos = pos;
    player->actor.shape.rot.x = player->actor.shape.rot.z = 0;
    player->actor.shape.rot.y = yaw;
    Player_MountHorse(play, player, player->rideActor);
    Player_SetCameraHorseSetting(play, player);
    gSaveContext.save.saveInfo.horseData.sceneId = play->sceneId;
    return true;
}

void Horse_Spawn(PlayState* play, Player* player) {
    if (Horse_SpawnRaceOot(play, player)) {
        D_801BDAA0 = false;
        return;
    }
    if (Horse_SpawnArcheryOot(play, player)) {
        D_801BDAA0 = false;
        return;
    }
    if (((play->sceneId == SCENE_KOEPONARACE) &&
         (GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_START)) ||
        ((play->sceneId == SCENE_F01) && (((gSaveContext.sceneLayer == 1)) || (gSaveContext.sceneLayer == 5)) &&
         (player->transformation == PLAYER_FORM_HUMAN)) ||
        ((play->sceneId == SCENE_KOEPONARACE) &&
         (((GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_3)) ||
          (GET_WEEKEVENTREG_HORSE_RACE_STATE == WEEKEVENTREG_HORSE_RACE_STATE_2)))) {
        // Gorman Track and horse state is either STATE_START, STATE_2 or STATE_3
        // or Romani Ranch, Player is Human and scene layer is either 1 or 5
        Horse_SpawnMinigame(play, player);
    } else {
        Horse_SpawnOverworld(play, player);
    }

    D_801BDAA0 = false;
}

void Horse_RotateToPoint(Actor* actor, Vec3f* pos, s16 turnYaw) {
    s16 yaw = Math_Vec3f_Yaw(&actor->world.pos, pos) - actor->world.rot.y;

    if (yaw > turnYaw) {
        actor->world.rot.y += turnYaw;
    } else if (yaw < -turnYaw) {
        actor->world.rot.y -= turnYaw;
    } else {
        actor->world.rot.y += yaw;
    }

    actor->shape.rot.y = actor->world.rot.y;
}

s32 Horse_IsActive(PlayState* play, ActorContext* actorCtx) {
    Actor* bgActor = actorCtx->actorLists[ACTORCAT_BG].first;

    if (bgActor != NULL) {
        while (true) {
            if ((bgActor->update != NULL) && (bgActor->init == NULL)) {
                if (Object_IsLoaded(&play->objectCtx, bgActor->objectSlot)) {
                    if ((bgActor->id == ACTOR_EN_HORSE) && (((EnHorse*)bgActor)->action != ENHORSE_ACTION_INACTIVE)) {
                        return true;
                    }
                }
            }
            bgActor = bgActor->next;
            if (bgActor == NULL) {
                break;
            }
        }
    }

    return false;
}
