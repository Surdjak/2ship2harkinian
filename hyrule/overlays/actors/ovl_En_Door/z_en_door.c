/*
 * File: z_en_door.c
 * Overlay: ovl_En_Door
 * Description: Doors with handles
 */

#include "z_en_door.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleInventory.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/gameplay_field_keep/gameplay_field_keep.h"
#include "assets/oot/objects/object_hidan_objects/object_hidan_objects.h"
#include "assets/oot/objects/object_mizu_objects/object_mizu_objects.h"
#include "assets/oot/objects/object_haka_door/object_haka_door.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

#define DOOR_AJAR_SLAM_RANGE 120.0f
#define DOOR_AJAR_OPEN_RANGE (2 * DOOR_AJAR_SLAM_RANGE)

#define DOOR_CHECK_RANGE 40.0f

// 3Ship [Hyrule] OoT's small keys, gSaveContext.inventory.dungeonKeys[gSaveContext.mapIndex]. Locked doors are only in
// dungeons.
#define DUNGEON_KEYS_OOT(play) (gSaveContext.save.saveInfo.inventoryOot.dungeonKeys[Inventory_GetDungeonIndexOot(play)])

static void EnDoor_Init(Actor* thisx, PlayState* play);
static void EnDoor_Destroy(Actor* thisx, PlayState* play);
static void EnDoor_Update(Actor* thisx, PlayState* play);
static void EnDoor_Draw(Actor* thisx, PlayState* play);

static void EnDoor_SetupType(EnDoor* this, PlayState* play);
static void EnDoor_Idle(EnDoor* this, PlayState* play);
static void EnDoor_WaitForCheck(EnDoor* this, PlayState* play);
static void EnDoor_Check(EnDoor* this, PlayState* play);
static void EnDoor_AjarWait(EnDoor* this, PlayState* play);
static void EnDoor_AjarOpen(EnDoor* this, PlayState* play);
static void EnDoor_AjarClose(EnDoor* this, PlayState* play);
static void EnDoor_Open(EnDoor* this, PlayState* play);

ActorProfile En_Door_Oot_Profile = {
    /**/ ACTOR_EN_DOOR_Oot,
    /**/ ACTORCAT_DOOR,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnDoor),
    /**/ EnDoor_Init,
    /**/ EnDoor_Destroy,
    /**/ EnDoor_Update,
    /**/ EnDoor_Draw,
};

/**
 * Controls which object and display lists to use in a given scene
 */
static EnDoorInfo sDoorInfo[] = {
    { SCENE_FIRE_TEMPLE_Oot, 1, OBJECT_HIDAN_OBJECTS_Oot },
    { SCENE_WATER_TEMPLE_Oot, 2, OBJECT_MIZU_OBJECTS_Oot },
    { SCENE_SHADOW_TEMPLE_Oot, 3, OBJECT_HAKA_DOOR_Oot },
    { SCENE_BOTTOM_OF_THE_WELL_Oot, 3, OBJECT_HAKA_DOOR_Oot },
    { SCENE_FOREST_TEMPLE_Oot, 0, GAMEPLAY_KEEP }, // Hacky fix, but behavior same as console.
    // KEEP objects should remain last and in this order
    { -1, 0, GAMEPLAY_KEEP },
    { -1, 4, OBJECT_GAMEPLAY_FIELD_KEEP_Oot },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 0, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 4000, ICHAIN_STOP),
};

static AnimationHeader* D_809FCECC[] = { &gDoorAdultOpeningLeftAnim, &gDoorChildOpeningLeftAnim,
                                         &gDoorAdultOpeningRightAnim, &gDoorChildOpeningRightAnim };

// 3Ship [Hyrule] OoT's Player sets animStyle (adult left, child left, adult right, child right); MM's sets the knob
// door's animIndex, the transformation for a left door and PLAYER_FORM_MAX more for a right one.
static s32 EnDoor_GetAnimStyle(EnDoor* this) {
    return ((this->knobDoor.animIndex >= PLAYER_FORM_MAX) ? 2 : 0) + (LINK_IS_ADULT_OOT ? 0 : 1);
}

static u8 sDoorAnimOpenFrames[] = { 25, 25, 25, 25 };

static u8 sDoorAnimCloseFrames[] = { 60, 70, 60, 70 };

static Gfx* D_809FCEE4[5][2] = {
    { gDoorLeftDL, gDoorRightDL },
    { gFireTempleDoorWithHandleLeftDL, gFireTempleDoorWithHandleRightDL },
    { gWaterTempleDoorLeftDL, gWaterTempleDoorRightDL },
    { gShadowDoorLeftDL, gShadowDoorRightDL },
    { gFieldDoorLeftDL, gFieldDoorRightDL },
};

static void EnDoor_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnDoor* this = (EnDoor*)thisx;
    EnDoorInfo* objectInfo;
    s32 i;
    s32 objectSlot;
    f32 xOffset;
    f32 zOffset;

    objectInfo = &sDoorInfo[0];
    Actor_ProcessInitChain(&this->knobDoor.dyna.actor, sInitChain);
    SkelAnime_Init(play, &this->knobDoor.skelAnime, &gDoorSkel, &gDoorAdultOpeningLeftAnim, this->jointTable, this->morphTable,
                   5);
    for (i = 0; i < ARRAY_COUNT(sDoorInfo) - 2; i++, objectInfo++) {
        if (play->sceneId == objectInfo->sceneNum) {
            break;
        }
    }

    // Due to Object_GetIndex always returning 0, doors always use the OBJECT_GAMEPLAY_FIELD_KEEP_Oot door.
    if (i >= ARRAY_COUNT(sDoorInfo) - 2 && Object_GetSlot(&play->objectCtx, OBJECT_GAMEPLAY_FIELD_KEEP_Oot) >= 0) {
        objectInfo++;
    }

    this->knobDoor.dlIndex = objectInfo->dListIndex;
    objectSlot = Object_GetSlot(&play->objectCtx, objectInfo->objectId);
    if (objectSlot < 0) {
        Actor_Kill(&this->knobDoor.dyna.actor);
        return;
    }

    this->knobDoor.objectSlot = objectSlot;
    this->knobDoor.dlIndex = objectInfo->dListIndex;
    if (this->knobDoor.dyna.actor.objectSlot == this->knobDoor.objectSlot) {
        EnDoor_SetupType(this, play);
    } else {
        this->actionFunc = EnDoor_SetupType;
    }

    // Double doors
    if (this->knobDoor.dyna.actor.params & 0x40) {
        EnDoor* other;

        xOffset = Math_CosS(this->knobDoor.dyna.actor.shape.rot.y) * 30.0f;
        zOffset = Math_SinS(this->knobDoor.dyna.actor.shape.rot.y) * 30.0f;
        other = (EnDoor*)Actor_SpawnAsChild(&play->actorCtx, &this->knobDoor.dyna.actor, play, ACTOR_EN_DOOR_Oot,
                                            this->knobDoor.dyna.actor.world.pos.x + xOffset, this->knobDoor.dyna.actor.world.pos.y,
                                            this->knobDoor.dyna.actor.world.pos.z - zOffset, 0, this->knobDoor.dyna.actor.shape.rot.y + 0x8000, 0,
                                            this->knobDoor.dyna.actor.params & ~0x40);
        if (other != NULL) {
            other->unk_192 = 1;
        }
        this->knobDoor.dyna.actor.world.pos.x -= xOffset;
        this->knobDoor.dyna.actor.world.pos.z += zOffset;
    }
    Actor_SetFocus(&this->knobDoor.dyna.actor, 70.0f);
}

static void EnDoor_Destroy(Actor* thisx, PlayState* play) {
    TransitionActorEntry* transitionEntry;
    EnDoor* this = (EnDoor*)thisx;

    transitionEntry = &play->transitionActors.list[(u16)this->knobDoor.dyna.actor.params >> 0xA];
    if (transitionEntry->id < 0) {
        transitionEntry->id = -transitionEntry->id;
    }
}

static void EnDoor_SetupType(EnDoor* this, PlayState* play) {
    s32 doorType;

    if (Object_IsLoaded(&play->objectCtx, this->knobDoor.objectSlot)) {
        doorType = this->knobDoor.dyna.actor.params >> 7 & 7;
        this->knobDoor.dyna.actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->knobDoor.dyna.actor.objectSlot = this->knobDoor.objectSlot;
        this->actionFunc = EnDoor_Idle;
        if (doorType == DOOR_EVENING) {
            doorType =
                (gSaveContext.save.time > 0xC000 && gSaveContext.save.time < 0xE000) ? DOOR_SCENEEXIT : DOOR_CHECKABLE;
        }
        this->knobDoor.dyna.actor.world.rot.y = 0x0000;
        if (doorType == DOOR_LOCKED) {
            if ((!Flags_GetSwitch(play, this->knobDoor.dyna.actor.params & 0x3F))) {
                this->lockTimer = 10;
            }
        } else if (doorType == DOOR_AJAR) {
            if (Actor_WorldDistXZToActor(&this->knobDoor.dyna.actor, &GET_PLAYER(play)->actor) > DOOR_AJAR_SLAM_RANGE) {
                this->actionFunc = EnDoor_AjarWait;
                this->knobDoor.dyna.actor.world.rot.y = -0x1800;
            }
        } else if (doorType == DOOR_CHECKABLE) {
            this->knobDoor.dyna.actor.textId = TEXT_OOT((this->knobDoor.dyna.actor.params & 0x3F) + 0x0200);
            if (this->knobDoor.dyna.actor.textId == TEXT_OOT(0x0229) && !GET_EVENTCHKINF_OOT(EVENTCHKINF_TALON_RETURNED_FROM_CASTLE_Oot)) {
                // Talon's house door. If Talon has not been woken up at Hyrule Castle
                // this door should be openable at any time of day. Note that there is no
                // check for time of day as the scene setup for Lon Lon merely initializes
                // the door with a different text id between day and night setups
                doorType = DOOR_SCENEEXIT;
            } else {
                this->actionFunc = EnDoor_WaitForCheck;
                this->knobDoor.dyna.actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_LOCK_ON_DISABLED;
            }
        }
        // 3Ship [Hyrule] MM's Player takes type 7 as MM's ENDOOR_TYPE_FRAMED, which loads no room; OoT's DOOR_ROOMLOAD7
        // works as DOOR_ROOMLOAD.
        if (doorType == DOOR_ROOMLOAD7) {
            doorType = DOOR_ROOMLOAD;
        }
        // Replace the door type it was loaded with by the new type
        this->knobDoor.dyna.actor.params = (this->knobDoor.dyna.actor.params & ~0x380) | (doorType << 7);
    }
}

static void EnDoor_Idle(EnDoor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 doorType;
    Vec3f playerPosRelToDoor;
    s16 phi_v0;

    doorType = this->knobDoor.dyna.actor.params >> 7 & 7;
    Actor_WorldToActorCoords(&this->knobDoor.dyna.actor, &playerPosRelToDoor, &player->actor.world.pos);
    if (this->knobDoor.requestOpen != 0) {
        this->actionFunc = EnDoor_Open;
        Animation_PlayOnceSetSpeed(&this->knobDoor.skelAnime, D_809FCECC[EnDoor_GetAnimStyle(this)],
                                   (player->stateFlags1 & PLAYER_STATE1_8000000) ? 0.75f : 1.5f);
        if (this->lockTimer != 0) {
            {
                DUNGEON_KEYS_OOT(play)--;
                Flags_SetSwitch(play, this->knobDoor.dyna.actor.params & 0x3F);
            }
            Actor_PlaySfx(&this->knobDoor.dyna.actor, NA_SE_EV_CHAIN_KEY_UNLOCK);
        }
    } else if (!Player_InCsMode(play)) {
        if (((fabsf(playerPosRelToDoor.y) < 20.0f && fabsf(playerPosRelToDoor.x) < 20.0f &&
                                   fabsf(playerPosRelToDoor.z) < 50.0f))) {
            phi_v0 = player->actor.shape.rot.y - this->knobDoor.dyna.actor.shape.rot.y;
            if (playerPosRelToDoor.z > 0.0f) {
                phi_v0 = 0x8000 - phi_v0;
            }
            if (ABS(phi_v0) < 0x3000) {
                if (this->lockTimer != 0) {
                    if (DUNGEON_KEYS_OOT(play) <= 0) {
                        // PORT: OoT's Navi says the door is locked (naviTextId -0x203). MM's tatlTextId, an s16, cannot
                        // hold OoT's text id (TEXT_OOT_BASE + 0x203) negated.
                        return;
                    } else {
                        player->doorTimer = 10;
                    }
                }
                player->doorType = (doorType == DOOR_AJAR) ? PLAYER_DOORTYPE_TALKING : PLAYER_DOORTYPE_HANDLE;
                player->doorDirection = (playerPosRelToDoor.z >= 0.0f) ? 1.0f : -1.0f;
                player->doorActor = &this->knobDoor.dyna.actor;
                // 3Ship [Hyrule] MM's Player opens a door only once the door has queued the door cutscene, as MM's doors
                // do here.
                func_80122F28(player);
            }
        } else if (doorType == DOOR_AJAR && this->knobDoor.dyna.actor.xzDistToPlayer > DOOR_AJAR_OPEN_RANGE) {
            this->actionFunc = EnDoor_AjarOpen;
        }
    }
}

static void EnDoor_WaitForCheck(EnDoor* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->knobDoor.dyna.actor, &play->state)) {
        this->actionFunc = EnDoor_Check;
    } else {
        Actor_OfferTalk(&this->knobDoor.dyna.actor, play, DOOR_CHECK_RANGE);
    }
}

static void EnDoor_Check(EnDoor* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->knobDoor.dyna.actor, play)) {
        this->actionFunc = EnDoor_WaitForCheck;
    }
}

static void EnDoor_AjarWait(EnDoor* this, PlayState* play) {
    if (this->knobDoor.dyna.actor.xzDistToPlayer < DOOR_AJAR_SLAM_RANGE) {
        this->actionFunc = EnDoor_AjarClose;
    }
}

static void EnDoor_AjarOpen(EnDoor* this, PlayState* play) {
    if (this->knobDoor.dyna.actor.xzDistToPlayer < DOOR_AJAR_SLAM_RANGE) {
        this->actionFunc = EnDoor_AjarClose;
    } else if (Math_ScaledStepToS(&this->knobDoor.dyna.actor.world.rot.y, -0x1800, 0x100)) {
        this->actionFunc = EnDoor_AjarWait;
    }
}

static void EnDoor_AjarClose(EnDoor* this, PlayState* play) {
    if (Math_ScaledStepToS(&this->knobDoor.dyna.actor.world.rot.y, 0, 0x700)) {
        this->actionFunc = EnDoor_Idle;
    }
}

static void EnDoor_Open(EnDoor* this, PlayState* play) {
    s32 i;
    s32 numEffects;

    if (DECR(this->lockTimer) == 0) {
        if (SkelAnime_Update(&this->knobDoor.skelAnime)) {
            this->actionFunc = EnDoor_Idle;
            this->knobDoor.requestOpen = 0;
        } else if (Animation_OnFrame(&this->knobDoor.skelAnime, sDoorAnimOpenFrames[EnDoor_GetAnimStyle(this)])) {
            Actor_PlaySfx(&this->knobDoor.dyna.actor,
                                  (play->sceneId == SCENE_SHADOW_TEMPLE_Oot ||
                                   play->sceneId == SCENE_BOTTOM_OF_THE_WELL_Oot || play->sceneId == SCENE_FIRE_TEMPLE_Oot)
                                      ? NA_SE_EV_IRON_DOOR_OPEN
                                      : NA_SE_OC_DOOR_OPEN);
            if (this->knobDoor.skelAnime.playSpeed < 1.5f) {
                numEffects = (s32)(Rand_ZeroOne() * 30.0f) + 50;
                for (i = 0; i < numEffects; i++) {
                    EffectSsBubble_Spawn(play, &this->knobDoor.dyna.actor.world.pos, 60.0f, 100.0f, 50.0f, 0.15f);
                }
            }
        } else if (Animation_OnFrame(&this->knobDoor.skelAnime, sDoorAnimCloseFrames[EnDoor_GetAnimStyle(this)])) {
            Actor_PlaySfx(&this->knobDoor.dyna.actor,
                                  (play->sceneId == SCENE_SHADOW_TEMPLE_Oot ||
                                   play->sceneId == SCENE_BOTTOM_OF_THE_WELL_Oot || play->sceneId == SCENE_FIRE_TEMPLE_Oot)
                                      ? NA_SE_EV_IRON_DOOR_CLOSE
                                      : NA_SE_EV_DOOR_CLOSE);
        }
    }
}

static void EnDoor_Update(Actor* thisx, PlayState* play) {
    EnDoor* this = (EnDoor*)thisx;

    this->actionFunc(this, play);
}

static s32 EnDoor_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    s32 pad;
    TransitionActorEntry* transitionEntry;
    Gfx** temp_a2;
    s32 pad2;
    s16 phi_v0_2;
    s32 phi_v0;
    EnDoor* this = (EnDoor*)thisx;

    if (limbIndex == 4) {
        temp_a2 = D_809FCEE4[this->knobDoor.dlIndex];
        transitionEntry = &play->transitionActors.list[(u16)this->knobDoor.dyna.actor.params >> 0xA];
        rot->z += this->knobDoor.dyna.actor.world.rot.y;
        if ((play->roomCtx.prevRoom.num >= 0) || (transitionEntry->sides[0].room == transitionEntry->sides[1].room)) {
            phi_v0_2 = ((this->knobDoor.dyna.actor.shape.rot.y + this->knobDoor.skelAnime.jointTable[3].z) + rot->z) -
                       Math_Vec3f_Yaw(&play->view.eye, &this->knobDoor.dyna.actor.world.pos);
            *dList = (ABS(phi_v0_2) < 0x4000) ? temp_a2[0] : temp_a2[1];
        } else {
            phi_v0 = this->unk_192;
            if (transitionEntry->sides[0].room != this->knobDoor.dyna.actor.room) {
                phi_v0 ^= 1;
            }
            *dList = temp_a2[phi_v0];
        }
    }
    return false;
}

static void EnDoor_Draw(Actor* thisx, PlayState* play) {
    EnDoor* this = (EnDoor*)thisx;

    if (this->knobDoor.dyna.actor.objectSlot == this->knobDoor.objectSlot) {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        SkelAnime_DrawOpa(play, this->knobDoor.skelAnime.skeleton, this->knobDoor.skelAnime.jointTable, EnDoor_OverrideLimbDraw, NULL, &this->knobDoor.dyna.actor);
        if (this->knobDoor.dyna.actor.world.rot.y != 0) {
            if (this->knobDoor.dyna.actor.world.rot.y > 0) {
                gSPDisplayList(POLY_OPA_DISP++, gDoorRightDL);
            } else {
                gSPDisplayList(POLY_OPA_DISP++, gDoorLeftDL);
            }
        }
        if (this->lockTimer != 0) {
            Actor_DrawDoorLock(play, this->lockTimer, DOORLOCK_NORMAL);
        }

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
