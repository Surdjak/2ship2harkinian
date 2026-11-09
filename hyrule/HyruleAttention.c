// OoT's attention cutscenes (z_onepointdemo.c's OnePointCutscene_Attention, cutscene 5010): when an actor does
// something worth seeing (a door unbars, a chest appears, a switch is pressed), the camera turns to it for a moment
// before handing back to Link. Ported actors wait for that turn (Hyrule_GetAttentionCategoryOot) before they act.
//
// Each attention is a sub camera with MM's CAM_SET_FREE0, which leaves its at and eye to this file. OoT's queue of
// sub cameras (their parentCamIdx and childCamIdx, which MM's Camera no longer has) is kept here by camera id. The
// shot is OoT's Camera_Demo5, which picks keyframes by where the target is, then OoT's Camera_Unique9, which plays
// them (OoT's CAM_SET_CS_C). Only the keyframe actions the attention keyframes use are ported.

#include "global.h"
#include "HyruleCamera.h"
#include "HyrulePlayer.h"
#include "HyrulePort.h"

// MM's z_camera.c, not in its headers.
void Camera_UpdateInterface(s32 interfaceFlags);
s32 Camera_BgCheck(Camera* camera, Vec3f* from, Vec3f* to);

#define ONEPOINT_CS_ATTENTION_OOT 5010

// OoT's OnePointCsFull: one keyframe of a Camera_Unique9 shot.
typedef struct {
    /* 0x00 */ u8 actionFlags;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ s16 initFlags;
    /* 0x04 */ s16 timerInit;
    /* 0x06 */ s16 rollTargetInit;
    /* 0x08 */ f32 fovTargetInit;
    /* 0x0C */ f32 lerpStepScale;
    /* 0x10 */ Vec3f atTargetInit;
    /* 0x1C */ Vec3f eyeTargetInit;
} OnePointCsFullOot;

// OoT's Unique9Anim.
typedef struct {
    OnePointCsFullOot* curKeyFrame;
    Vec3f atTarget;
    Vec3f eyeTarget;
    Vec3f playerPos;
    f32 fovTarget;
    VecGeo atEyeOffsetTarget;
    s16 rollTarget;
    s16 curKeyFrameIdx;
    s16 unk_38;
    s16 isNewKeyFrame;
    s16 keyFrameTimer;
} Unique9AnimOot;

typedef enum {
    /* 0 */ ATTENTION_SETTING_ATTENTION, // OoT's CAM_SET_CS_ATTENTION: Camera_Demo5 has not run yet
    /* 1 */ ATTENTION_SETTING_CS_C       // OoT's CAM_SET_CS_C: Camera_Unique9 plays the keyframes
} AttentionSettingOot;

// What OoT keeps in a one-point sub camera.
typedef struct {
    u8 inUse;
    u8 setting;
    s16 parentCamId; // OoT's parentCamIdx
    s16 childCamId;  // OoT's childCamIdx, CAM_ID_MAIN for none (OoT's SUBCAM_FREE)
    s16 timer;
    s16 animState;
    Actor* target;
    s32 sfxId; // OoT's data1
    OnePointCsFullOot* keyFrames;
    s32 keyFrameCnt;
    Unique9AnimOot anim;
} AttentionCamOot;

// By MM camera id; the main camera's entry only holds its child.
static AttentionCamOot sCamsOot[NUM_CAMS];

// OoT's D_8011D3AC: the category of the actor the last attention shot turned to. OoT never resets it.
static s32 sAttentionCategoryOot = -1;
static s32 sDemo5PrevSfxFrameOot = -200;
static s32 sDemo5PrevAction12FrameOot = -16;
// OoT's sCameraInterfaceFlags, set by the shot and applied after it.
static s16 sInterfaceFlagsOot;

/*******************************************************
 * OoT's keyframes for Camera_Demo5 (z_camera_data.inc). Camera_Demo5 changes some of them in place, as OoT does.
 ********************************************************/

// The target is Link, far from the eye.
static OnePointCsFullOot D_8011D6ACOot[] = {
    { 0x8F, 0xFF, 0x0002, 0x0001, 0x0000, 60.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { 0x81, 0xFF, 0x0303, 0x0013, 0x0000, 45.0f, 1.0f, { 0.0f, 20.0f, 0.0f }, { 0.0f, 20.0f, 0.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The target is Link, close to the eye.
static OnePointCsFullOot D_8011D724Oot[] = {
    { 0x8F, 0xFF, 0x2424, 0x0001, 0x0000, 60.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 10.0f, -20.0f } },
    { 0x81, 0xFF, 0x2121, 0x0013, 0x0000, 50.0f, 1.0f, { 0.0f, -10.0f, 0.0f }, { 0.0f, 0.0f, 60.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The target is close to Link.
static OnePointCsFullOot D_8011D79COot[] = {
    { 0xCF, 0xFF, 0x0002, 0x0001, 0x0000, 60.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { 0xC1, 0xFF, 0x0303, 0x0013, 0x0000, 45.0f, 1.0f, { 0.0f, -20.0f, 0.0f }, { 0.0f, -10.0f, 5.0f } },
    { 0xC1, 0xFF, 0x0303, 0x0009, 0x0000, 60.0f, 1.0f, { 0.0f, 10.0f, 0.0f }, { 0.0f, 10.0f, 0.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The target is within 300 units of the eye, and Link within 30.
static OnePointCsFullOot D_8011D83COot[] = {
    { 0x83, 0xFF, 0x2141, 0x0014, 0x0000, 45.0f, 0.2f, { 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 10.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The target is within 700 units of the eye, on screen, in front of Link.
static OnePointCsFullOot D_8011D88COot[] = {
    { 0x81, 0xFF, 0x0303, 0x0014, 0x0000, 45.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The same, with the target off screen.
static OnePointCsFullOot D_8011D8DCOot[] = {
    { 0x8F, 0xFF, 0x0404, 0x0014, 0x0001, 50.0f, 1.0f, { 0.0f, 5.0f, 10.0f }, { 0.0f, 10.0f, -80.0f } },
    { 0x82, 0xFF, 0x2121, 0x0005, 0x0000, 60.0f, 1.0f, { 0.0f, 5.0f, 0.0f }, { 5.0f, 5.0f, -200.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// The target is a door.
static OnePointCsFullOot D_8011D954Oot[] = {
    { 0x0F, 0xFF, 0xC1C1, 0x0014, 0x0000, 60.0f, 1.0f, { 0.0f, 0.0f, 50.0f }, { 0.0f, 0.0f, 250.0f } },
    { 0x83, 0xFF, 0x05B1, 0x0005, 0x0000, 60.0f, 0.1f, { 0.0f, 10.0f, 50.0f }, { 0.0f, 10.0f, 100.0f } },
    { 0x82, 0xFF, 0x2121, 0x0005, 0x0002, 60.0f, 1.0f, { 0.0f, 10.0f, 0.0f }, { 0.0f, 20.0f, -150.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

// Otherwise.
static OnePointCsFullOot D_8011D9F4Oot[] = {
    { 0x8F, 0xFF, 0x0504, 0x0014, 0x0002, 60.0f, 1.0f, { 0.0f, 5.0f, 50.0f }, { 0.0f, 20.0f, 300.0f } },
    { 0x82, 0xFF, 0x2121, 0x0005, 0x0002, 60.0f, 1.0f, { 0.0f, 10.0f, 0.0f }, { 0.0f, 20.0f, -150.0f } },
    { 0x12, 0xFF, 0x0000, 0x0001, 0x0000, 60.0f, 1.0f, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } },
};

/*******************************************************
 * OoT's camera helpers
 ********************************************************/

static f32 Camera_LERPCeilFOot(f32 target, f32 cur, f32 stepScale, f32 minDiff) {
    f32 diff = target - cur;

    return (fabsf(diff) >= minDiff) ? cur + diff * stepScale : target;
}

static f32 Camera_LERPFloorFOot(f32 target, f32 cur, f32 stepScale, f32 minDiff) {
    f32 diff = target - cur;

    return (fabsf(diff) >= minDiff) ? cur + diff * stepScale : cur;
}

static s16 Camera_LERPCeilSOot(s16 target, s16 cur, f32 stepScale, s16 minDiff) {
    s16 diff = target - cur;
    s16 step;

    if (ABS(diff) >= minDiff) {
        step = diff * stepScale + 0.5f;
        return cur + step;
    }
    return target;
}

static s16 Camera_LERPFloorSOot(s16 target, s16 cur, f32 stepScale, s16 minDiff) {
    s16 diff = target - cur;
    s16 step;

    if (ABS(diff) >= minDiff) {
        step = diff * stepScale + 0.5f;
        return cur + step;
    }
    return cur;
}

// OoT's keyframe angles in degrees (OoT's DEGF_TO_BINANG).
#define DEGF_TO_BINANG_OOT(degreesf) CAM_DEG_TO_BINANG(degreesf)

// OoT's actors are freed when they die; a target is only read while it is still in the actor lists.
static s32 Hyrule_IsActorLoadedOot(PlayState* play, Actor* actor) {
    s32 category;
    Actor* it;

    if (actor == NULL) {
        return false;
    }
    for (category = 0; category < ACTORCAT_MAX; category++) {
        for (it = play->actorCtx.actorLists[category].first; it != NULL; it = it->next) {
            if (it == actor) {
                return actor->update != NULL;
            }
        }
    }
    return false;
}

// OoT's func_800C0D34: the yaw a door faces from its front room, false for a door between two sides of one room.
static s32 Hyrule_GetDoorFrontYawOot(PlayState* play, Actor* actor, s16* yaw) {
    TransitionActorEntry* transitionActor;
    s32 frontRoom;

    if (actor->category != ACTORCAT_DOOR) {
        return false;
    }
    transitionActor = &play->transitionActors.list[(u16)actor->params >> 10];
    frontRoom = transitionActor->sides[0].room;
    if (frontRoom == transitionActor->sides[1].room) {
        return false;
    }
    *yaw = (frontRoom == actor->room) ? actor->shape.rot.y : (s16)(actor->shape.rot.y + 0x8000);
    return true;
}

/*******************************************************
 * The queue (OoT's z_onepointdemo.c)
 ********************************************************/

// OoT's OnePointCutscene_SetAsChild.
static s16 Hyrule_SetAsChildOot(s16 newCamId, s16 parentCamId) {
    s16 prevCamId = sCamsOot[parentCamId].childCamId;

    sCamsOot[newCamId].parentCamId = parentCamId;
    sCamsOot[parentCamId].childCamId = newCamId;
    return prevCamId;
}

// OoT's OnePointCutscene_Init and OnePointCutscene_SetInfo for cutscene 5010. All attention shots have the same
// priority, so none removes another.
static s16 Hyrule_InitAttentionCameraOot(PlayState* play, s16 timer, Actor* actor, s16 parentCamId) {
    Camera* mainCam = play->cameraPtrs[CAM_ID_MAIN];
    Camera* subCam;
    AttentionCamOot* att;
    s16 subCamId = Play_CreateSubCamera(play);
    s16 childCamId;
    s16 status;

    if (subCamId == CAM_ID_NONE) {
        return CAM_ID_NONE;
    }
    att = &sCamsOot[subCamId];
    memset(att, 0, sizeof(*att));
    att->inUse = true;

    // In front of the parent's camera in the queue.
    childCamId = sCamsOot[parentCamId].childCamId;
    status = CAM_STATUS_ACTIVE;
    if (childCamId >= CAM_ID_SUB_FIRST) {
        Hyrule_SetAsChildOot(childCamId, subCamId);
        status = CAM_STATUS_WAIT;
    } else {
        // OoT's Interface_ChangeHudVisibilityMode(HUD_VISIBILITY_NOTHING_ALT), through MM's camera so that it puts the
        // HUD back when the main camera resumes.
        Camera_UpdateInterface(CAM_LETTERBOX_IGNORE | CAM_HUD_VISIBILITY_NONE_ALT);
    }
    Hyrule_SetAsChildOot(subCamId, parentCamId);

    att->timer = timer;
    att->target = actor;
    att->setting = ATTENTION_SETTING_ATTENTION;
    att->animState = 0;
    att->sfxId = NA_SE_SY_CORRECT_CHIME;

    subCam = play->cameraPtrs[subCamId];
    Play_CopyCamera(play, subCamId, CAM_ID_MAIN);
    Play_ChangeCameraSetting(play, subCamId, CAM_SET_FREE0);
    // OoT's SetInfo for 5010 starts from the main camera's at and eye.
    subCam->at = mainCam->at;
    subCam->eye = subCam->eyeNext = mainCam->eye;
    subCam->fov = play->view.fovy;
    subCam->roll = 0;

    Play_ChangeCameraStatus(play, parentCamId, (parentCamId == CAM_ID_MAIN) ? CAM_STATUS_UNK3 : CAM_STATUS_WAIT);
    Play_ChangeCameraStatus(play, subCamId, status);
    return subCamId;
}

s32 OnePointCutscene_Attention(PlayState* play, Actor* actor) {
    Camera* mainCam = play->cameraPtrs[CAM_ID_MAIN];
    s16 camId = CAM_ID_MAIN;
    s16 parentCamId;
    s32 lastHigherCat = -1;
    s32 targetCat;
    s16 timer;

    if (mainCam->mode == CAM_MODE_FOLLOWBOOMERANG) {
        Camera_ChangeMode(mainCam, CAM_MODE_NORMAL);
    }

    // The first attention shot whose actor has a lower category, or the first other shot after one attention shot.
    while (sCamsOot[camId].childCamId != CAM_ID_MAIN) {
        camId = sCamsOot[camId].childCamId;
        if (!sCamsOot[camId].inUse || (play->cameraPtrs[camId] == NULL)) {
            break;
        } else if (sCamsOot[camId].setting != ATTENTION_SETTING_ATTENTION) {
            if (lastHigherCat == -1) {
                continue;
            } else {
                break;
            }
        } else {
            targetCat = sCamsOot[camId].target->category;
            if (actor->category > targetCat) {
                break;
            }
            lastHigherCat = targetCat;
        }
    }
    parentCamId = (lastHigherCat == -1) ? CAM_ID_MAIN : camId;

    switch (actor->category) {
        case ACTORCAT_NPC:
        case ACTORCAT_ITEMACTION:
        case ACTORCAT_CHEST:
            timer = 100;
            break;
        default:
            timer = 30;
            break;
    }

    // An actor of the same category as the shot before it is skipped.
    if (actor->category == lastHigherCat) {
        return CAM_ID_NONE;
    }
    return Hyrule_InitAttentionCameraOot(play, timer, actor, parentCamId);
}

s32 OnePointCutscene_AttentionSetSfxOot(PlayState* play, Actor* actor, s32 sfxId) {
    s32 subCamId = OnePointCutscene_Attention(play, actor);

    if (subCamId != CAM_ID_NONE) {
        sCamsOot[subCamId].sfxId = sfxId;
    }
    return subCamId;
}

s32 OnePointCutscene_CheckForCategory(PlayState* play, s32 actorCategory) {
    s16 camId = CAM_ID_MAIN;

    while (sCamsOot[camId].childCamId != CAM_ID_MAIN) {
        camId = sCamsOot[camId].childCamId;
        if (!sCamsOot[camId].inUse || (sCamsOot[camId].setting != ATTENTION_SETTING_ATTENTION)) {
            break;
        } else if (actorCategory == sCamsOot[camId].target->category) {
            return true;
        }
    }
    return false;
}

s32 Hyrule_GetAttentionCategoryOot(void) {
    return sAttentionCategoryOot;
}

/*******************************************************
 * The shot (OoT's z_camera.c)
 ********************************************************/

// OoT's Camera_Unique9, for the keyframe actions the attention keyframes use (1, 2, 3, 15 and 18).
static void Camera_Unique9Oot(PlayState* play, Camera* camera, AttentionCamOot* att) {
    Unique9AnimOot* anim = &att->anim;
    Player* player = GET_PLAYER(play);
    Vec3f* eyeNext = &camera->eyeNext;
    Vec3f* at = &camera->at;
    Vec3f* eye = &camera->eye;
    Vec3f atTarget;
    Vec3f eyeTarget;
    Vec3f eyeLookAtPos;
    VecGeo eyeNextAtOffset;
    VecGeo scratchGeo;
    VecGeo playerTargetOffset;
    PosRot targethead;
    PosRot playerhead;
    PosRot playerPosRot;
    PosRot focusPosRot;
    Actor* focusActor;
    f32 invKeyFrameTimer;
    s16 atInitFlags;
    s16 eyeInitFlags;
    s16 action;

    // CAM_SET_CS_C's interface: the large letterbox, the HUD as it was.
    sInterfaceFlagsOot = 0x3F00;
    playerPosRot = Actor_GetWorld(&player->actor);

    if (att->animState == 0) {
        att->animState++;
        anim->curKeyFrameIdx = -1;
        anim->keyFrameTimer = 1;
        anim->unk_38 = 0;
        anim->playerPos = playerPosRot.pos;
    }

    if ((anim->unk_38 == 0) && (anim->keyFrameTimer > 0)) {
        anim->keyFrameTimer--;
    }

    if (anim->keyFrameTimer == 0) {
        anim->isNewKeyFrame = true;
        anim->curKeyFrameIdx++;
        if (anim->curKeyFrameIdx < att->keyFrameCnt) {
            anim->curKeyFrame = &att->keyFrames[anim->curKeyFrameIdx];
            anim->keyFrameTimer = anim->curKeyFrame->timerInit;

            if (anim->curKeyFrame->unk_01 != 0xFF) {
                if ((anim->curKeyFrame->unk_01 & 0xF0) == 0x80) {
                    sAttentionCategoryOot = anim->curKeyFrame->unk_01 & 0xF;
                } else if ((anim->curKeyFrame->unk_01 & 0xF0) == 0xC0) {
                    sInterfaceFlagsOot = 0xF000 | ((anim->curKeyFrame->unk_01 & 0xF) << 8);
                } else if ((player->stateFlags1 & PLAYER_STATE1_8000000) &&
                           !PLAYER_BOOTS_WALKS_UNDERWATER(player->currentBoots)) {
                    Player_SetCsActionWithHaltedActorsOot(play, att->target, 8);
                } else {
                    Player_SetCsActionWithHaltedActorsOot(play, att->target, anim->curKeyFrame->unk_01);
                }
            }
        } else {
            // All the keyframes have played.
            att->timer = 0;
            return;
        }
    } else {
        anim->isNewKeyFrame = false;
    }

    if ((att->target != NULL) && !Hyrule_IsActorLoadedOot(play, att->target)) {
        att->target = NULL;
    }

    atInitFlags = anim->curKeyFrame->initFlags & 0xFF;
    if (atInitFlags == 1) {
        anim->atTarget = anim->curKeyFrame->atTargetInit;
    } else if (atInitFlags == 2) {
        if (anim->isNewKeyFrame) {
            anim->atTarget.x = play->view.at.x + anim->curKeyFrame->atTargetInit.x;
            anim->atTarget.y = play->view.at.y + anim->curKeyFrame->atTargetInit.y;
            anim->atTarget.z = play->view.at.z + anim->curKeyFrame->atTargetInit.z;
        }
    } else if (atInitFlags == 3) {
        if (anim->isNewKeyFrame) {
            anim->atTarget.x = camera->at.x + anim->curKeyFrame->atTargetInit.x;
            anim->atTarget.y = camera->at.y + anim->curKeyFrame->atTargetInit.y;
            anim->atTarget.z = camera->at.z + anim->curKeyFrame->atTargetInit.z;
        }
    } else if ((atInitFlags == 4) || (atInitFlags == 0x84)) {
        if (att->target != NULL) {
            targethead = Actor_GetFocus(att->target);
            playerhead = Actor_GetFocus(&player->actor);
            playerhead.pos.x = playerPosRot.pos.x;
            playerhead.pos.z = playerPosRot.pos.z;
            playerTargetOffset = OLib_Vec3fDiffToVecGeo(&targethead.pos, &playerhead.pos);
            if (atInitFlags & (s16)0x8080) {
                scratchGeo.pitch = DEGF_TO_BINANG_OOT(anim->curKeyFrame->atTargetInit.x);
                scratchGeo.yaw = DEGF_TO_BINANG_OOT(anim->curKeyFrame->atTargetInit.y);
                scratchGeo.r = anim->curKeyFrame->atTargetInit.z;
            } else {
                scratchGeo = OLib_Vec3fToVecGeo(&anim->curKeyFrame->atTargetInit);
            }
            scratchGeo.yaw += playerTargetOffset.yaw;
            scratchGeo.pitch += playerTargetOffset.pitch;
            anim->atTarget = OLib_AddVecGeoToVec3f(&targethead.pos, &scratchGeo);
        } else {
            anim->atTarget = camera->at;
        }
    } else if (atInitFlags & 0x6060) {
        if (!(atInitFlags & 4) || anim->isNewKeyFrame) {
            if (atInitFlags & 0x2020) {
                focusActor = &player->actor;
            } else {
                focusActor = att->target;
            }

            if (focusActor != NULL) {
                if ((atInitFlags & 0xF) == 1) {
                    focusPosRot = Actor_GetFocus(focusActor);
                } else if ((atInitFlags & 0xF) == 2) {
                    focusPosRot = Actor_GetWorld(focusActor);
                } else {
                    focusPosRot = Actor_GetWorldPosShapeRot(focusActor);
                }

                if (atInitFlags & (s16)0x8080) {
                    scratchGeo.pitch = DEGF_TO_BINANG_OOT(anim->curKeyFrame->atTargetInit.x);
                    scratchGeo.yaw = DEGF_TO_BINANG_OOT(anim->curKeyFrame->atTargetInit.y);
                    scratchGeo.r = anim->curKeyFrame->atTargetInit.z;
                } else {
                    scratchGeo = OLib_Vec3fToVecGeo(&anim->curKeyFrame->atTargetInit);
                }

                scratchGeo.yaw += focusPosRot.rot.y;
                scratchGeo.pitch -= focusPosRot.rot.x;
                anim->atTarget = OLib_AddVecGeoToVec3f(&focusPosRot.pos, &scratchGeo);
            } else {
                anim->atTarget = *at;
            }
        }
    } else {
        anim->atTarget = *at;
    }

    eyeInitFlags = anim->curKeyFrame->initFlags & 0xFF00;
    if (eyeInitFlags == 0x100) {
        anim->eyeTarget = anim->curKeyFrame->eyeTargetInit;
    } else if (eyeInitFlags == 0x200) {
        if (anim->isNewKeyFrame) {
            anim->eyeTarget.x = play->view.eye.x + anim->curKeyFrame->eyeTargetInit.x;
            anim->eyeTarget.y = play->view.eye.y + anim->curKeyFrame->eyeTargetInit.y;
            anim->eyeTarget.z = play->view.eye.z + anim->curKeyFrame->eyeTargetInit.z;
        }
    } else if (eyeInitFlags == 0x300) {
        if (anim->isNewKeyFrame) {
            anim->eyeTarget.x = camera->eyeNext.x + anim->curKeyFrame->eyeTargetInit.x;
            anim->eyeTarget.y = camera->eyeNext.y + anim->curKeyFrame->eyeTargetInit.y;
            anim->eyeTarget.z = camera->eyeNext.z + anim->curKeyFrame->eyeTargetInit.z;
        }
    } else if ((eyeInitFlags == 0x400) || (eyeInitFlags == (s16)0x8400) || (eyeInitFlags == 0x500) ||
               (eyeInitFlags == (s16)0x8500)) {
        if (att->target != NULL) {
            targethead = Actor_GetFocus(att->target);
            playerhead = Actor_GetFocus(&player->actor);
            playerhead.pos.x = playerPosRot.pos.x;
            playerhead.pos.z = playerPosRot.pos.z;
            playerTargetOffset = OLib_Vec3fDiffToVecGeo(&targethead.pos, &playerhead.pos);
            if ((eyeInitFlags == 0x400) || (eyeInitFlags == (s16)0x8400)) {
                eyeLookAtPos = targethead.pos;
            } else {
                eyeLookAtPos = anim->atTarget;
            }

            if (eyeInitFlags & (s16)0x8080) {
                scratchGeo.pitch = DEGF_TO_BINANG_OOT(anim->curKeyFrame->eyeTargetInit.x);
                scratchGeo.yaw = DEGF_TO_BINANG_OOT(anim->curKeyFrame->eyeTargetInit.y);
                scratchGeo.r = anim->curKeyFrame->eyeTargetInit.z;
            } else {
                scratchGeo = OLib_Vec3fToVecGeo(&anim->curKeyFrame->eyeTargetInit);
            }

            scratchGeo.yaw += playerTargetOffset.yaw;
            scratchGeo.pitch += playerTargetOffset.pitch;
            anim->eyeTarget = OLib_AddVecGeoToVec3f(&eyeLookAtPos, &scratchGeo);
        } else {
            anim->eyeTarget = *eyeNext;
        }
    } else if (eyeInitFlags & 0x6060) {
        if (!(eyeInitFlags & 0x400) || anim->isNewKeyFrame) {
            if (eyeInitFlags & 0x2020) {
                focusActor = &player->actor;
            } else {
                focusActor = att->target;
            }

            if (focusActor != NULL) {
                if ((eyeInitFlags & 0xF00) == 0x100) {
                    focusPosRot = Actor_GetFocus(focusActor);
                } else if ((eyeInitFlags & 0xF00) == 0x200) {
                    focusPosRot = Actor_GetWorld(focusActor);
                } else {
                    focusPosRot = Actor_GetWorldPosShapeRot(focusActor);
                }

                if (eyeInitFlags & (s16)0x8080) {
                    scratchGeo.pitch = DEGF_TO_BINANG_OOT(anim->curKeyFrame->eyeTargetInit.x);
                    scratchGeo.yaw = DEGF_TO_BINANG_OOT(anim->curKeyFrame->eyeTargetInit.y);
                    scratchGeo.r = anim->curKeyFrame->eyeTargetInit.z;
                } else {
                    scratchGeo = OLib_Vec3fToVecGeo(&anim->curKeyFrame->eyeTargetInit);
                }

                scratchGeo.yaw += focusPosRot.rot.y;
                scratchGeo.pitch -= focusPosRot.rot.x;
                anim->eyeTarget = OLib_AddVecGeoToVec3f(&focusPosRot.pos, &scratchGeo);
            } else {
                anim->eyeTarget = *eyeNext;
            }
        }
    } else {
        anim->eyeTarget = *eyeNext;
    }

    if (anim->curKeyFrame->initFlags == 2) {
        anim->fovTarget = play->view.fovy;
        anim->rollTarget = 0;
    } else if (anim->curKeyFrame->initFlags == 0) {
        anim->fovTarget = camera->fov;
        anim->rollTarget = camera->roll;
    } else {
        anim->fovTarget = anim->curKeyFrame->fovTargetInit;
        anim->rollTarget = DEGF_TO_BINANG_OOT(anim->curKeyFrame->rollTargetInit);
    }

    action = anim->curKeyFrame->actionFlags & 0x1F;
    switch (action) {
        case 15:
            // Jump to the targets.
            *at = anim->atTarget;
            *eyeNext = anim->eyeTarget;
            camera->fov = anim->fovTarget;
            camera->roll = anim->rollTarget;
            break;

        case 1:
            // Interpolate the eye around at, in spherical coordinates.
            eyeNextAtOffset = OLib_Vec3fDiffToVecGeo(at, eyeNext);
            anim->atEyeOffsetTarget = OLib_Vec3fDiffToVecGeo(&anim->atTarget, &anim->eyeTarget);
            invKeyFrameTimer = 1.0f / anim->keyFrameTimer;
            scratchGeo.r = F32_LERPIMP(eyeNextAtOffset.r, anim->atEyeOffsetTarget.r, invKeyFrameTimer);
            scratchGeo.pitch = eyeNextAtOffset.pitch +
                               (BINANG_SUB(anim->atEyeOffsetTarget.pitch, eyeNextAtOffset.pitch) * invKeyFrameTimer);
            scratchGeo.yaw =
                eyeNextAtOffset.yaw + (BINANG_SUB(anim->atEyeOffsetTarget.yaw, eyeNextAtOffset.yaw) * invKeyFrameTimer);
            eyeTarget = OLib_AddVecGeoToVec3f(at, &scratchGeo);
            goto setEyeNext;

        case 2:
            // Interpolate the eye in a line.
            invKeyFrameTimer = 1.0f / anim->keyFrameTimer;
            eyeTarget.x = F32_LERPIMP(camera->eyeNext.x, anim->eyeTarget.x, invKeyFrameTimer);
            eyeTarget.y = F32_LERPIMP(camera->eyeNext.y, anim->eyeTarget.y, invKeyFrameTimer);
            eyeTarget.z = F32_LERPIMP(camera->eyeNext.z, anim->eyeTarget.z, invKeyFrameTimer);

        setEyeNext:
            camera->eyeNext.x =
                Camera_LERPFloorFOot(eyeTarget.x, camera->eyeNext.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->eyeNext.y =
                Camera_LERPFloorFOot(eyeTarget.y, camera->eyeNext.y, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->eyeNext.z =
                Camera_LERPFloorFOot(eyeTarget.z, camera->eyeNext.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            // fallthrough
        case 9:
        case 10:
            // Interpolate at, fov and roll in a line.
            invKeyFrameTimer = 1.0f / anim->keyFrameTimer;
            atTarget.x = F32_LERPIMP(camera->at.x, anim->atTarget.x, invKeyFrameTimer);
            atTarget.y = F32_LERPIMP(camera->at.y, anim->atTarget.y, invKeyFrameTimer);
            atTarget.z = F32_LERPIMP(camera->at.z, anim->atTarget.z, invKeyFrameTimer);
            camera->at.x = Camera_LERPFloorFOot(atTarget.x, camera->at.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.y = Camera_LERPFloorFOot(atTarget.y, camera->at.y, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.z = Camera_LERPFloorFOot(atTarget.z, camera->at.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->fov = Camera_LERPFloorFOot(F32_LERPIMP(camera->fov, anim->fovTarget, invKeyFrameTimer),
                                               camera->fov, anim->curKeyFrame->lerpStepScale, 0.01f);
            camera->roll = Camera_LERPFloorSOot(BINANG_LERPIMPINV(camera->roll, anim->rollTarget, anim->keyFrameTimer),
                                                camera->roll, anim->curKeyFrame->lerpStepScale, 0xA);
            break;

        case 3:
            // Step the eye towards its target.
            camera->eyeNext.x =
                Camera_LERPCeilFOot(anim->eyeTarget.x, camera->eyeNext.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->eyeNext.y =
                Camera_LERPCeilFOot(anim->eyeTarget.y, camera->eyeNext.y, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->eyeNext.z =
                Camera_LERPCeilFOot(anim->eyeTarget.z, camera->eyeNext.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            // fallthrough
        case 11:
        case 12:
            // Step at, fov and roll towards their targets.
            camera->at.x = Camera_LERPCeilFOot(anim->atTarget.x, camera->at.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.y = Camera_LERPCeilFOot(anim->atTarget.y, camera->at.y, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.z = Camera_LERPCeilFOot(anim->atTarget.z, camera->at.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->fov = Camera_LERPCeilFOot(anim->fovTarget, camera->fov, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->roll = Camera_LERPCeilSOot(anim->rollTarget, camera->roll, anim->curKeyFrame->lerpStepScale, 1);
            break;

        case 18: {
            // Hand this framing to the parent camera (OoT's Camera_Copy), which goes on from it.
            s16 parentCamId = (att->parentCamId <= CAM_ID_NONE) ? CAM_ID_MAIN : att->parentCamId;

            *eye = *eyeNext;
            Camera_Copy(play->cameraPtrs[parentCamId], camera);
        }
            // fallthrough
        default:
            att->timer = 0;
            break;
    }

    *eye = *eyeNext;

    if (anim->curKeyFrame->actionFlags & 0x80) {
        Camera_BgCheck(camera, at, eye);
    }

    if (anim->curKeyFrame->actionFlags & 0x40) {
        // Link holds his place.
        player->actor.world.pos.x = anim->playerPos.x;
        player->actor.world.pos.z = anim->playerPos.z;
        if ((player->stateFlags1 & PLAYER_STATE1_8000000) && !PLAYER_BOOTS_WALKS_UNDERWATER(player->currentBoots)) {
            player->actor.world.pos.y = anim->playerPos.y;
        }
    } else {
        anim->playerPos = playerPosRot.pos;
    }

    if ((anim->unk_38 == 0) && (att->timer > 0)) {
        att->timer--;
    }

    camera->dist = OLib_Vec3fDist(at, eye);
}

// OoT's Camera_Demo5: picks the keyframes for where the target is, then plays them with Camera_Unique9.
static void Camera_Demo5Oot(PlayState* play, Camera* camera, AttentionCamOot* att) {
    Player* player = GET_PLAYER(play);
    Actor* target = att->target;
    PosRot playerhead = Actor_GetFocus(&player->actor);
    PosRot playerPosRot = Actor_GetWorldPosShapeRot(&player->actor);
    PosRot targetPosRot;
    PosRot targethead;
    VecGeo playerTargetGeo;
    VecGeo eyePlayerGeo;
    f32 eyeTargetDist;
    f32 sp90;
    s16 screenX;
    s16 screenY;
    s16 sp4A;
    s32 temp;
    s32 frameDiff;

    sInterfaceFlagsOot = 0x3200;
    if (!Hyrule_IsActorLoadedOot(play, target)) {
        // OoT stops here and keeps the camera waiting for a target that is gone; the shot ends instead.
        att->target = NULL;
        att->timer = 0;
        return;
    }
    targetPosRot = Actor_GetFocus(target);
    playerTargetGeo = OLib_Vec3fDiffToVecGeo(&targetPosRot.pos, &playerPosRot.pos);
    sAttentionCategoryOot = target->category;
    Actor_GetScreenPos(play, target, &screenX, &screenY);
    eyeTargetDist = OLib_Vec3fDist(&targetPosRot.pos, &camera->eye);
    eyePlayerGeo = OLib_Vec3fDiffToVecGeo(&playerhead.pos, &camera->eyeNext);
    sp4A = eyePlayerGeo.yaw - playerTargetGeo.yaw;

    if (target->category == ACTORCAT_PLAYER) {
        if (eyePlayerGeo.r > 30.0f) {
            D_8011D6ACOot[1].timerInit = att->timer - 1;
            D_8011D6ACOot[1].atTargetInit.z = Rand_ZeroOne() * 10.0f;
            D_8011D6ACOot[1].eyeTargetInit.x = Rand_ZeroOne() * 10.0f;
            att->keyFrames = D_8011D6ACOot;
            att->keyFrameCnt = ARRAY_COUNT(D_8011D6ACOot);
            if (att->parentCamId != CAM_ID_MAIN) {
                att->keyFrameCnt--;
            } else {
                att->timer += D_8011D6ACOot[2].timerInit;
            }
        } else {
            D_8011D724Oot[1].eyeTargetInit.x = Rand_ZeroOne() * 10.0f;
            D_8011D724Oot[1].timerInit = att->timer - 1;
            att->keyFrames = D_8011D724Oot;
            att->keyFrameCnt = ARRAY_COUNT(D_8011D724Oot);
            if (att->parentCamId != CAM_ID_MAIN) {
                att->keyFrameCnt--;
            } else {
                att->timer += D_8011D724Oot[2].timerInit;
            }
        }
    } else if (playerTargetGeo.r < 30.0f) {
        att->keyFrames = D_8011D79COot;
        att->keyFrameCnt = ARRAY_COUNT(D_8011D79COot);
        if ((screenX < 0x15) || (screenX >= 0x12C) || (screenY < 0x29) || (screenY >= 0xC8)) {
            D_8011D79COot[0].actionFlags = 0x41;
            D_8011D79COot[0].atTargetInit.y = -30.0f;
            D_8011D79COot[0].atTargetInit.x = 0.0f;
            D_8011D79COot[0].atTargetInit.z = 0.0f;
            D_8011D79COot[0].eyeTargetInit.y = 0.0f;
            D_8011D79COot[0].eyeTargetInit.x = 10.0f;
            D_8011D79COot[0].eyeTargetInit.z = -50.0f;
        }

        D_8011D79COot[1].timerInit = att->timer - 1;

        if (att->parentCamId != CAM_ID_MAIN) {
            att->keyFrameCnt -= 2;
        } else {
            att->timer += D_8011D79COot[2].timerInit + D_8011D79COot[3].timerInit;
        }
    } else if ((eyeTargetDist < 300.0f) && (eyePlayerGeo.r < 30.0f)) {
        D_8011D83COot[0].timerInit = att->timer;
        att->keyFrames = D_8011D83COot;
        att->keyFrameCnt = ARRAY_COUNT(D_8011D83COot);
        if (att->parentCamId != CAM_ID_MAIN) {
            att->keyFrameCnt--;
        } else {
            att->timer += D_8011D83COot[1].timerInit;
        }
    } else if ((eyeTargetDist < 700.0f) && (ABS(sp4A) < 0x36B0)) {
        if ((screenX >= 0x15) && (screenX < 0x12C) && (screenY >= 0x29) && (screenY < 0xC8) &&
            (eyePlayerGeo.r > 30.0f)) {
            D_8011D88COot[0].timerInit = att->timer;
            att->keyFrames = D_8011D88COot;
            att->keyFrameCnt = ARRAY_COUNT(D_8011D88COot);
            if (att->parentCamId != CAM_ID_MAIN) {
                att->keyFrameCnt--;
            } else {
                att->timer += D_8011D88COot[1].timerInit;
            }
        } else {
            D_8011D8DCOot[0].atTargetInit.z = eyeTargetDist * 0.6f;
            D_8011D8DCOot[0].eyeTargetInit.z = eyeTargetDist + 50.0f;
            D_8011D8DCOot[0].eyeTargetInit.x = Rand_ZeroOne() * 10.0f;
            if (BINANG_SUB(eyePlayerGeo.yaw, playerTargetGeo.yaw) > 0) {
                D_8011D8DCOot[0].atTargetInit.x = -D_8011D8DCOot[0].atTargetInit.x;
                D_8011D8DCOot[0].eyeTargetInit.x = -D_8011D8DCOot[0].eyeTargetInit.x;
                D_8011D8DCOot[0].rollTargetInit = -D_8011D8DCOot[0].rollTargetInit;
            }
            D_8011D8DCOot[0].timerInit = att->timer;
            D_8011D8DCOot[1].timerInit = (s16)(eyeTargetDist * 0.005f) + 8;
            att->keyFrames = D_8011D8DCOot;
            att->keyFrameCnt = ARRAY_COUNT(D_8011D8DCOot);
            if (att->parentCamId != CAM_ID_MAIN) {
                att->keyFrameCnt -= 2;
            } else {
                att->timer += D_8011D8DCOot[1].timerInit + D_8011D8DCOot[2].timerInit;
            }
        }
    } else if (target->category == ACTORCAT_DOOR) {
        D_8011D954Oot[0].timerInit = att->timer - 5;
        sp4A = 0;
        if (!Hyrule_GetDoorFrontYawOot(play, target, &sp4A)) {
            // A door between two sides of one room.
            if (ABS(playerTargetGeo.yaw - target->shape.rot.y) >= 0x4000) {
                sp4A = target->shape.rot.y;
            } else {
                sp4A = BINANG_ROT180(target->shape.rot.y);
            }
        }

        D_8011D954Oot[0].atTargetInit.y = D_8011D954Oot[0].eyeTargetInit.y = D_8011D954Oot[1].atTargetInit.y =
            (target->shape.rot.y == sp4A) ? 180.0f : 0.0f;
        sp90 = ((BINANG_SUB(playerTargetGeo.yaw, sp4A) < 0) ? 20.0f : -20.0f) * Rand_ZeroOne();
        D_8011D954Oot[0].eyeTargetInit.y = D_8011D954Oot[0].eyeTargetInit.y + sp90;
        temp = Rand_ZeroOne() * (sp90 * -0.2f);
        D_8011D954Oot[1].rollTargetInit = temp;
        D_8011D954Oot[0].rollTargetInit = temp;
        targethead = Actor_GetFocus(target);
        targethead.pos.x += 50.0f * Math_SinS(BINANG_ROT180(sp4A));
        targethead.pos.z += 50.0f * Math_CosS(BINANG_ROT180(sp4A));
        if (Camera_BgCheck(camera, &playerhead.pos, &targethead.pos)) {
            D_8011D954Oot[1].actionFlags = 0xC1;
            D_8011D954Oot[2].actionFlags = 0x8F;
        } else {
            D_8011D954Oot[2].timerInit = (s16)(eyeTargetDist * 0.004f) + 6;
        }
        att->keyFrames = D_8011D954Oot;
        att->keyFrameCnt = ARRAY_COUNT(D_8011D954Oot);
        if (att->parentCamId != CAM_ID_MAIN) {
            att->keyFrameCnt -= 2;
        } else {
            att->timer += D_8011D954Oot[2].timerInit + D_8011D954Oot[3].timerInit;
        }
    } else {
        if (playerTargetGeo.r < 200.0f) {
            D_8011D9F4Oot[0].eyeTargetInit.z = playerTargetGeo.r;
            D_8011D9F4Oot[0].atTargetInit.z = playerTargetGeo.r * 0.25f;
        }
        if (playerTargetGeo.r < 400.0f) {
            D_8011D9F4Oot[0].eyeTargetInit.x = Rand_ZeroOne() * 25.0f;
        }
        D_8011D9F4Oot[0].timerInit = att->timer;
        targethead = Actor_GetFocus(target);
        if (Camera_BgCheck(camera, &playerhead.pos, &targethead.pos)) {
            D_8011D9F4Oot[1].timerInit = 4;
            D_8011D9F4Oot[1].actionFlags = 0x8F;
        } else {
            D_8011D9F4Oot[1].timerInit = (s16)(eyeTargetDist * 0.005f) + 8;
        }
        att->keyFrames = D_8011D9F4Oot;
        att->keyFrameCnt = ARRAY_COUNT(D_8011D9F4Oot);
        if (att->parentCamId != CAM_ID_MAIN) {
            if (play->state.frames & 1) {
                D_8011D9F4Oot[0].rollTargetInit = -D_8011D9F4Oot[0].rollTargetInit;
                D_8011D9F4Oot[1].rollTargetInit = -D_8011D9F4Oot[1].rollTargetInit;
            }
            att->keyFrameCnt -= 2;
        } else {
            att->timer += D_8011D9F4Oot[1].timerInit + D_8011D9F4Oot[2].timerInit;
            D_8011D9F4Oot[0].rollTargetInit = D_8011D9F4Oot[1].rollTargetInit = 0;
        }
    }

    frameDiff = sDemo5PrevSfxFrameOot - (s32)play->state.frames;
    if ((frameDiff >= 0x33) || (frameDiff < -0x32)) {
        Audio_PlaySfx(att->sfxId);
    }
    sDemo5PrevSfxFrameOot = play->state.frames;

    if ((player->stateFlags1 & PLAYER_STATE1_8000000) && !PLAYER_BOOTS_WALKS_UNDERWATER(player->currentBoots)) {
        // Swimming: Link and the other actors stop.
        player->stateFlags1 |= PLAYER_STATE1_20000000;
        player->actor.freezeTimer = att->timer;
    } else if (target->category == ACTORCAT_PLAYER) {
        frameDiff = (s32)play->state.frames - sDemo5PrevAction12FrameOot;
        if (player->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR) {
            Player_SetCsActionWithHaltedActorsOot(play, target, 8);
        } else if (ABS(frameDiff) > 3000) {
            Player_SetCsActionWithHaltedActorsOot(play, target, 12);
        } else {
            Player_SetCsActionWithHaltedActorsOot(play, target, 69);
        }
    } else {
        Player_SetCsActionWithHaltedActorsOot(play, target, 1);
    }
    sDemo5PrevAction12FrameOot = play->state.frames;

    // OoT's Camera_ChangeSettingFlags(camera, CAM_SET_CS_C, 4 | 1): Camera_Unique9 from now on, from its start.
    att->setting = ATTENTION_SETTING_CS_C;
    att->animState = 0;
    Camera_Unique9Oot(play, camera, att);
}

// OoT's Camera_Finish: the shot is over, its parent camera takes over.
static void Hyrule_FinishAttentionCameraOot(PlayState* play, s16 camId) {
    AttentionCamOot* att = &sCamsOot[camId];
    Player* player = GET_PLAYER(play);
    s16 parentCamId = att->parentCamId;
    s16 childCamId = att->childCamId;

    Play_ChangeCameraStatus(play, parentCamId, CAM_STATUS_ACTIVE);

    if (parentCamId == CAM_ID_MAIN) {
        player->actor.freezeTimer = 0;
        player->stateFlags1 &= ~PLAYER_STATE1_20000000;
        if (player->csAction != PLAYER_CSACTION_NONE) {
            Player_SetCsActionWithHaltedActorsOot(play, &player->actor, 7);
        }
        Camera_SetFinishedFlag(play->cameraPtrs[CAM_ID_MAIN]);
    }

    if ((childCamId != CAM_ID_MAIN) && (sCamsOot[childCamId].parentCamId == camId)) {
        sCamsOot[childCamId].parentCamId = parentCamId;
    }
    if (sCamsOot[parentCamId].childCamId == camId) {
        sCamsOot[parentCamId].childCamId = childCamId;
    }
    if (parentCamId == CAM_ID_MAIN) {
        play->cameraPtrs[CAM_ID_MAIN]->animState = 0;
    }

    memset(att, 0, sizeof(*att));
    play->envCtx.fillScreen = false;
    Play_ClearCamera(play, camId);
}

void Hyrule_UpdateAttentionCameraOot(PlayState* play) {
    s16 camId = play->activeCamId;

    if ((camId == CAM_ID_MAIN) || !sCamsOot[camId].inUse || (play->cameraPtrs[camId] == NULL)) {
        return;
    }
    // OoT finishes a shot after the frame its timer ran out on (Camera_Finish, at the end of Play_Draw).
    if (sCamsOot[camId].timer == 0) {
        Hyrule_FinishAttentionCameraOot(play, camId);
        camId = play->activeCamId;
        if ((camId == CAM_ID_MAIN) || !sCamsOot[camId].inUse || (play->cameraPtrs[camId] == NULL)) {
            return;
        }
    }

    if (sCamsOot[camId].setting == ATTENTION_SETTING_ATTENTION) {
        Camera_Demo5Oot(play, play->cameraPtrs[camId], &sCamsOot[camId]);
    } else {
        Camera_Unique9Oot(play, play->cameraPtrs[camId], &sCamsOot[camId]);
    }

    // OoT's Camera_UpdateInterface(sCameraInterfaceFlags): OoT's and MM's flags share their layout.
    Camera_UpdateInterface(sInterfaceFlagsOot);
}

void Hyrule_ResetAttentionCameraOot(void) {
    memset(sCamsOot, 0, sizeof(sCamsOot));
}
