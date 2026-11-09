// OoT's one-point camera cutscenes (z_onepointdemo.c): short camera shots that actors start to show what they do (a
// door unbarring, platforms rising, a boss falling), each a sub camera that takes over from the camera before it and
// hands back when its timer runs out. The attention shots (OnePointCutscene_Attention, cutscene 5010) turn to an
// actor that did something worth seeing; ported actors wait for that turn (Hyrule_GetAttentionCategoryOot).
//
// Each shot is a sub camera with MM's CAM_SET_FREE0, which leaves its at and eye to this file. OoT's queue of sub
// cameras (their parentCamIdx and childCamIdx, which MM's Camera no longer has) and the setting each shot plays are
// kept here by camera id. The shots are OoT's: OnePointCutscene_SetInfo (HyruleOnePointInfo.inc, generated from SoH's)
// picks each one's setting and keyframes, and OoT's camera functions for those settings play them: Camera_Unique9 (CS_C),
// Camera_Demo5 (CS_ATTENTION), Camera_Unique6 (FREE2), Camera_Demo9 (CS_3), Camera_Special0 (PIVOT_VERTICAL),
// Camera_Demo6 (FOREST_DEFEAT_POE) and Camera_Special4 (FIRE_STAIRCASE). OoT's Camera_Keep4 (TURN_AROUND, cutscene 9806,
// which no ported actor starts) is not ported.

#include "global.h"
#include "z64quake.h"
#include "HyruleCamera.h"
#include "HyruleCutscene.h"
#include "HyrulePlayer.h"
#include "HyrulePort.h"
#include "overlays/actors/ovl_En_Sw/z_en_sw.h" // OoT's, in hyrule/

// MM's z_camera.c, not in its headers.
void Camera_UpdateInterface(s32 interfaceFlags);
s32 Camera_BgCheck(Camera* camera, Vec3f* from, Vec3f* to);

#define ONEPOINT_CS_ATTENTION_OOT 5010

// OoT's angle macros.
#define DEGF_TO_BINANG_OOT(degreesf) CAM_DEG_TO_BINANG(degreesf)
#define BINANG_TO_DEGF(binang) ((f32)(binang) * (360.0001525f / 65535.0f))
#define RADF_TO_DEGF(radf) ((radf) * (180.0f / M_PI))
#define DEGF_TO_BINANG(degreesf) DEGF_TO_BINANG_OOT(degreesf)

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

// OoT's Demo9Anim.
typedef struct {
    s16 keyframe;
    s16 finishAction;
    s16 animTimer;
    s16 doLERPAt;
    f32 curFrame;
} Demo9AnimOot;

// The OoT camera settings one-point shots play.
typedef enum {
    /* 0 */ ONEPOINT_SET_NONE,
    /* 1 */ ONEPOINT_SET_CS_ATTENTION,     // Camera_Demo5, then CS_C
    /* 2 */ ONEPOINT_SET_CS_C,             // Camera_Unique9
    /* 3 */ ONEPOINT_SET_FREE2,            // Camera_Unique6
    /* 4 */ ONEPOINT_SET_CS_3,             // Camera_Demo9
    /* 5 */ ONEPOINT_SET_PIVOT_VERTICAL,   // Camera_Special0
    /* 6 */ ONEPOINT_SET_FOREST_DEFEAT_POE, // Camera_Demo6
    /* 7 */ ONEPOINT_SET_FIRE_STAIRCASE,   // Camera_Special4
    /* 8 */ ONEPOINT_SET_TURN_AROUND       // Camera_Keep4, not ported
} OnePointSettingOot;

// What OoT keeps in a one-point sub camera.
typedef struct {
    u8 inUse;
    u8 setting;
    s16 csId;
    s16 parentCamId; // OoT's parentCamIdx
    s16 childCamId;  // OoT's childCamIdx, CAM_ID_MAIN for none (OoT's SUBCAM_FREE)
    s16 timer;
    s16 animState;
    s16 interfaceFlags; // OoT's sCameraInterfaceFlags for this camera
    Actor* target;
    s32 sfxId; // OoT's data1 for attention shots
    // CS_C (OoT's Unique9OnePointCs)
    OnePointCsFullOot* keyFrames;
    s32 keyFrameCnt;
    Unique9AnimOot anim;
    // CS_3 (OoT's OnePointCsCamera and Demo9Anim)
    CutsceneCameraPointOot* atPoints;
    CutsceneCameraPointOot* eyePoints;
    s16 actionParameters;
    s16 initTimer;
    Demo9AnimOot demo9;
    // FOREST_DEFEAT_POE (OoT's Demo6Anim) and FIRE_STAIRCASE (OoT's Special4)
    Vec3f demo6AtTarget;
    s16 demo6AnimTimer;
    s16 spec4InitialTimer;
} OnePointCamOot;

// By MM camera id; the main camera's entry only holds its child.
static OnePointCamOot sCamsOot[NUM_CAMS];

// OoT's D_8011D3AC: the category of the actor the last attention shot turned to. OoT never resets it.
static s32 sAttentionCategoryOot = -1;
static s32 sDemo5PrevSfxFrameOot = -200;
static s32 sDemo5PrevAction12FrameOot = -16;
static s32 sPrevFrameCs1100 = -4096;

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

// OoT's one-point keyframes and spline points (z_onepointdemo_data.inc).
#include "HyruleOnePointData.inc"

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

static void Camera_LERPCeilVec3fOot(Vec3f* target, Vec3f* cur, f32 yStepScale, f32 xzStepScale, f32 minDiff) {
    cur->x = Camera_LERPCeilFOot(target->x, cur->x, xzStepScale, minDiff);
    cur->y = Camera_LERPCeilFOot(target->y, cur->y, yStepScale, minDiff);
    cur->z = Camera_LERPCeilFOot(target->z, cur->z, xzStepScale, minDiff);
}

// OoT's Camera_RotateAroundPoint: pos turned by the yaw of posRot and moved to its position.
static Vec3f Camera_RotateAroundPointOot(PosRot* posRot, Vec3f* pos) {
    VecGeo geo = OLib_Vec3fToVecGeo(pos);

    geo.yaw += posRot->rot.y;
    return OLib_AddVecGeoToVec3f(&posRot->pos, &geo);
}

// OoT's Camera_XZAngle.
static s16 Camera_XZAngleOot(Vec3f* to, Vec3f* from) {
    return DEGF_TO_BINANG(RADF_TO_DEGF(Math_FAtan2F(from->x - to->x, from->z - to->z)));
}

// OoT's Camera_GetFloorY.
static f32 Camera_GetFloorYOot(PlayState* play, Vec3f* pos) {
    CollisionPoly* floorPoly;
    s32 bgId;
    Vec3f posCheck = *pos;

    posCheck.y += 80.0f;
    return BgCheck_EntityRaycastFloor3(&play->colCtx, &floorPoly, &bgId, &posCheck);
}

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

// OoT's z_onepointdemo.c helpers.
static void OnePointCutscene_AddVecSphToVec3f(Vec3f* dst, Vec3f* src, VecGeo* geo) {
    *dst = OLib_AddVecGeoToVec3f(src, geo);
}

static s16 OnePointCutscene_Vec3fYaw(Vec3f* vec1, Vec3f* vec2) {
    return DEGF_TO_BINANG(RADF_TO_DEGF(Math_FAtan2F(vec2->x - vec1->x, vec2->z - vec1->z)));
}

static void OnePointCutscene_Vec3sToVec3f(Vec3f* src, Vec3s* dst) {
    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
}

static s32 OnePointCutscene_BgCheckLineTest(CollisionContext* colCtx, Vec3f* vec1, Vec3f* vec2) {
    Vec3f posResult;
    s32 bgId;
    CollisionPoly* outPoly = NULL;

    return BgCheck_CameraLineTest1(colCtx, vec1, vec2, &posResult, &outPoly, true, true, true, false, &bgId);
}

static f32 OnePointCutscene_RaycastFloor(CollisionContext* colCtx, Vec3f* pos) {
    CollisionPoly* outPoly;
    s32 bgId;

    return BgCheck_EntityRaycastFloor3(colCtx, &outPoly, &bgId, pos);
}

static void OnePointCutscene_SetCsCamPointsOot(OnePointCamOot* att, s16 actionParameters, s16 initTimer,
                                               CutsceneCameraPointOot* atPoints, CutsceneCameraPointOot* eyePoints) {
    att->atPoints = atPoints;
    att->eyePoints = eyePoints;
    att->actionParameters = actionParameters;
    att->initTimer = initTimer;
}

// OoT's func_800C0808 and Play_CameraChangeSetting for a one-point camera: the setting's camera function starts over.
static void OnePoint_SetSettingOot(PlayState* play, s16 camId, s32 setting) {
    if ((camId < CAM_ID_SUB_FIRST) || (camId >= NUM_CAMS)) {
        return;
    }
    if (sCamsOot[camId].setting != setting) {
        sCamsOot[camId].setting = setting;
        sCamsOot[camId].animState = 0;
    }
}

// OoT's Play_CameraSetAtEye.
static void OnePoint_SetAtEyeOot(PlayState* play, s16 camId, Vec3f* at, Vec3f* eye) {
    Camera* camera = play->cameraPtrs[(camId == CAM_ID_NONE) ? play->activeCamId : camId];

    camera->at = *at;
    camera->eye = camera->eyeNext = *eye;
    camera->dist = OLib_Vec3fDist(at, eye);
}

// OoT's OnePointCutscene_SetInfo, with MM's names.
#include "HyruleOnePointInfo.inc"

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

// OoT's OnePointCutscene_RemoveCamera: the parent camera if the removed one is active, else CAM_ID_NONE.
static s16 OnePointCutscene_RemoveCameraOot(PlayState* play, s16 camId) {
    OnePointCamOot* att = &sCamsOot[camId];
    s16 nextCamId;

    if ((att->childCamId != CAM_ID_MAIN) && (sCamsOot[att->childCamId].parentCamId == camId)) {
        sCamsOot[att->childCamId].parentCamId = att->parentCamId;
    }
    if (sCamsOot[att->parentCamId].childCamId == camId) {
        sCamsOot[att->parentCamId].childCamId = att->childCamId;
    }
    nextCamId = (play->activeCamId == camId) ? att->parentCamId : CAM_ID_NONE;
    memset(att, 0, sizeof(*att));
    Play_ClearCamera(play, camId);
    return nextCamId;
}

s16 OnePointCutscene_Init(PlayState* play, s16 csId, s16 timer, Actor* actor, s16 parentCamId) {
    Camera* subCam;
    OnePointCamOot* att;
    s16 subCamId;
    s16 childCamId;
    s16 status;
    s16 curCamId;
    s16 nextCamId;

    if (parentCamId == CAM_ID_NONE) {
        parentCamId = play->activeCamId;
    }
    subCamId = Play_CreateSubCamera(play);
    if (subCamId == CAM_ID_NONE) {
        return CAM_ID_NONE;
    }
    att = &sCamsOot[subCamId];
    memset(att, 0, sizeof(*att));
    att->inUse = true;

    // Into the queue in front of the parent's camera.
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

    subCam = play->cameraPtrs[subCamId];
    Play_CopyCamera(play, subCamId, CAM_ID_MAIN);
    Play_ChangeCameraSetting(play, subCamId, CAM_SET_FREE0);

    att->timer = timer;
    att->target = actor;
    att->csId = csId;
    att->setting = ONEPOINT_SET_NONE;
    att->interfaceFlags = CAM_LETTERBOX_IGNORE | CAM_HUD_VISIBILITY_IGNORE;

    subCam->at = play->view.at;
    subCam->eye = subCam->eyeNext = play->view.eye;
    subCam->fov = play->view.fovy;

    Play_ChangeCameraStatus(play, parentCamId, (parentCamId == CAM_ID_MAIN) ? CAM_STATUS_UNK3 : CAM_STATUS_WAIT);
    OnePointCutscene_SetInfoOot(play, subCamId, csId, actor, timer);
    Play_ChangeCameraStatus(play, subCamId, status);

    // The lower priority shots in front of this one leave the queue.
    curCamId = subCamId;
    nextCamId = sCamsOot[subCamId].childCamId;
    while (nextCamId >= CAM_ID_SUB_FIRST) {
        if ((sCamsOot[nextCamId].csId / 100) < (sCamsOot[subCamId].csId / 100)) {
            if (sCamsOot[nextCamId].csId != ONEPOINT_CS_ATTENTION_OOT) {
                if ((nextCamId = OnePointCutscene_RemoveCameraOot(play, nextCamId)) != CAM_ID_NONE) {
                    Play_ChangeCameraStatus(play, nextCamId, CAM_STATUS_ACTIVE);
                }
            } else {
                curCamId = nextCamId;
                OnePointCutscene_EndCutscene(play, nextCamId);
            }
        } else {
            curCamId = nextCamId;
        }
        nextCamId = sCamsOot[curCamId].childCamId;
    }
    return subCamId;
}

s16 OnePointCutscene_EndCutscene(PlayState* play, s16 subCamId) {
    if (subCamId == CAM_ID_NONE) {
        subCamId = play->activeCamId;
    }
    if ((subCamId >= CAM_ID_SUB_FIRST) && (subCamId < NUM_CAMS) && sCamsOot[subCamId].inUse &&
        (play->cameraPtrs[subCamId] != NULL)) {
        // An attention shot still turns away over 5 frames.
        sCamsOot[subCamId].timer = (sCamsOot[subCamId].csId == ONEPOINT_CS_ATTENTION_OOT) ? 5 : 0;
    }
    return subCamId;
}

s32 OnePointCutscene_Attention(PlayState* play, Actor* actor) {
    Camera* mainCam = play->cameraPtrs[CAM_ID_MAIN];
    s16 camId = CAM_ID_MAIN;
    s16 parentCamId;
    s32 lastHigherCat = -1;
    s32 targetCat;
    s16 timer;
    s16 subCamId;

    if (mainCam->mode == CAM_MODE_FOLLOWBOOMERANG) {
        Camera_ChangeMode(mainCam, CAM_MODE_NORMAL);
    }

    // The first attention shot whose actor has a lower category, or the first other shot after one attention shot.
    while (sCamsOot[camId].childCamId != CAM_ID_MAIN) {
        camId = sCamsOot[camId].childCamId;
        if (!sCamsOot[camId].inUse || (play->cameraPtrs[camId] == NULL)) {
            break;
        } else if (sCamsOot[camId].setting != ONEPOINT_SET_CS_ATTENTION) {
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
    subCamId = OnePointCutscene_Init(play, ONEPOINT_CS_ATTENTION_OOT, timer, actor, parentCamId);
    if (subCamId != CAM_ID_NONE) {
        sCamsOot[subCamId].sfxId = NA_SE_SY_CORRECT_CHIME;
    }
    return subCamId;
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
        if (!sCamsOot[camId].inUse || (sCamsOot[camId].setting != ONEPOINT_SET_CS_ATTENTION)) {
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
 * The camera functions (OoT's z_camera.c)
 ********************************************************/

// OoT's Camera_Unique9: plays a shot's keyframes.
static void Camera_Unique9Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
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
    f32 spB4;
    s16 atInitFlags;
    s16 eyeInitFlags;
    s16 action;

    // CAM_SET_CS_C's interface: the large letterbox, the HUD as it was.
    att->interfaceFlags = 0x3F00;
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
                    Camera_UpdateInterface(0xF000 | ((anim->curKeyFrame->unk_01 & 0xF) << 8));
                } else if ((player->stateFlags1 & PLAYER_STATE1_8000000) &&
                           !PLAYER_BOOTS_WALKS_UNDERWATER(player->currentBoots)) {
                    Player_SetCsActionOot(play, att->target, 8);
                } else {
                    Player_SetCsActionOot(play, att->target, anim->curKeyFrame->unk_01);
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

        case 21:
            // The same, held until the actor that started the shot says to go on (OoT's func_8005B1A4, MM's
            // Camera_SetFinishedFlag).
            if (anim->unk_38 == 0) {
                anim->unk_38 = 1;
            } else if (camera->stateFlags & CAM_STATE_3) {
                anim->unk_38 = 0;
                camera->stateFlags &= ~CAM_STATE_3;
            }
            *at = anim->atTarget;
            *eyeNext = anim->eyeTarget;
            camera->fov = anim->fovTarget;
            camera->roll = anim->rollTarget;
            break;

        case 16:
            // The same, without clearing the flag.
            if (anim->unk_38 == 0) {
                anim->unk_38 = 1;
            } else if (camera->stateFlags & CAM_STATE_3) {
                anim->unk_38 = 0;
            }
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

        case 4:
            // Step the eye around at towards its target, in spherical coordinates.
            eyeNextAtOffset = OLib_Vec3fDiffToVecGeo(at, eyeNext);
            anim->atEyeOffsetTarget = OLib_Vec3fDiffToVecGeo(&anim->atTarget, &anim->eyeTarget);
            scratchGeo.r = Camera_LERPCeilFOot(anim->atEyeOffsetTarget.r, eyeNextAtOffset.r,
                                               anim->curKeyFrame->lerpStepScale, 0.1f);
            scratchGeo.pitch = Camera_LERPCeilSOot(anim->atEyeOffsetTarget.pitch, eyeNextAtOffset.pitch,
                                                   anim->curKeyFrame->lerpStepScale, 1);
            scratchGeo.yaw = Camera_LERPCeilSOot(anim->atEyeOffsetTarget.yaw, eyeNextAtOffset.yaw,
                                                 anim->curKeyFrame->lerpStepScale, 1);
            *eyeNext = OLib_AddVecGeoToVec3f(at, &scratchGeo);
            goto setAtFOVRoll;

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
        setAtFOVRoll:
            // Step at, fov and roll towards their targets.
            camera->at.x = Camera_LERPCeilFOot(anim->atTarget.x, camera->at.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.y = Camera_LERPCeilFOot(anim->atTarget.y, camera->at.y, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.z = Camera_LERPCeilFOot(anim->atTarget.z, camera->at.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->fov = Camera_LERPCeilFOot(anim->fovTarget, camera->fov, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->roll = Camera_LERPCeilSOot(anim->rollTarget, camera->roll, anim->curKeyFrame->lerpStepScale, 1);
            break;

        case 13:
            // Step at towards its target and turn the eye around it by eyeTargetInit.y degrees.
            camera->at.x = Camera_LERPCeilFOot(anim->atTarget.x, camera->at.x, anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->at.y += camera->unk_0F0.y * anim->curKeyFrame->lerpStepScale;
            camera->at.z = Camera_LERPCeilFOot(anim->atTarget.z, camera->at.z, anim->curKeyFrame->lerpStepScale, 1.0f);
            scratchGeo = OLib_Vec3fDiffToVecGeo(at, eyeNext);
            scratchGeo.yaw += DEGF_TO_BINANG_OOT(anim->curKeyFrame->eyeTargetInit.y);

            // 0x3A98 ~ 82.40 degrees
            if (scratchGeo.pitch >= 0x3A99) {
                scratchGeo.pitch = 0x3A98;
            }
            if (scratchGeo.pitch < -0x3A98) {
                scratchGeo.pitch = -0x3A98;
            }

            spB4 = scratchGeo.r;
            scratchGeo.r = !(spB4 < anim->curKeyFrame->eyeTargetInit.z)
                               ? Camera_LERPCeilFOot(anim->curKeyFrame->eyeTargetInit.z, spB4,
                                                     anim->curKeyFrame->lerpStepScale, 1.0f)
                               : scratchGeo.r;

            *eyeNext = OLib_AddVecGeoToVec3f(at, &scratchGeo);
            camera->fov = Camera_LERPCeilFOot(
                F32_LERPIMPINV(camera->fov, anim->curKeyFrame->fovTargetInit, anim->keyFrameTimer), camera->fov,
                anim->curKeyFrame->lerpStepScale, 1.0f);
            camera->roll = Camera_LERPCeilSOot(anim->rollTarget, camera->roll, anim->curKeyFrame->lerpStepScale, 1);
            break;

        case 24:
            // Jump to the keyframe given by the roll target.
            anim->curKeyFrameIdx = anim->rollTarget;
            break;

        case 19: {
            // The parent camera (or the main one) goes back to its normal mode, then as 18.
            s16 parentCamId = (att->parentCamId <= CAM_ID_NONE) ? CAM_ID_MAIN : att->parentCamId;

            Camera_ChangeModeFlags(play->cameraPtrs[parentCamId], CAM_MODE_NORMAL, true);
        }
            // fallthrough
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

// OoT's Camera_Demo5: an attention shot picks the keyframes for where the target is, then plays them with
// Camera_Unique9.
static void Camera_Demo5Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
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

    att->interfaceFlags = 0x3200;
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
    att->setting = ONEPOINT_SET_CS_C;
    att->animState = 0;
    Camera_Unique9Oot(play, camera, att);
}

// OoT's Camera_Unique6 (CAM_SET_FREE2): the camera holds where its shot put it until the timer runs out.
static void Camera_Unique6Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
    // FREE2's interface: unchanged, and the timer runs.
    att->interfaceFlags = 0xFF01;
    if (att->animState == 0) {
        att->animState++;
    }
    camera->dist = OLib_Vec3fDist(&camera->at, &camera->eye);
    if (att->timer > 0) {
        att->timer--;
    }
}

// OoT's Camera_Demo9 (CAM_SET_CS_3): the camera follows its at and eye splines.
static void Camera_Demo9Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
    Demo9AnimOot* anim = &att->demo9;
    Camera* mainCam = play->cameraPtrs[CAM_ID_MAIN];
    Player* player = GET_PLAYER(play);
    Vec3f csEyeUpdate;
    Vec3f csAtUpdate;
    Vec3f newEye;
    Vec3f newAt;
    f32 newRoll;
    PosRot focusPosRot;
    s32 finishAction;
    s16 onePointTimer;

    att->interfaceFlags = 0x3212;

    switch (att->animState) {
        case 0:
            anim->keyframe = 0;
            anim->finishAction = 0;
            anim->curFrame = 0.0f;
            att->animState++;
            anim->doLERPAt = false;
            finishAction = att->actionParameters & 0xF000;
            if (finishAction != 0) {
                anim->finishAction = finishAction;
                att->actionParameters &= 0xFFF;
            }
            anim->animTimer = att->initTimer;
            // fallthrough
        case 1:
            if (anim->animTimer > 0) {
                if (Hyrule_CsSplineStepOot(&csEyeUpdate, &newRoll, &camera->fov, att->eyePoints, &anim->keyframe,
                                           &anim->curFrame) ||
                    Hyrule_CsSplineStepOot(&csAtUpdate, &newRoll, &camera->fov, att->atPoints, &anim->keyframe,
                                           &anim->curFrame)) {
                    att->animState = 2;
                }

                if (att->actionParameters == 1) {
                    // Around the main camera's Link.
                    newEye = Camera_RotateAroundPointOot(&mainCam->focalActorPosRot, &csEyeUpdate);
                    newAt = Camera_RotateAroundPointOot(&mainCam->focalActorPosRot, &csAtUpdate);
                } else if (att->actionParameters == 4) {
                    // Around Link.
                    focusPosRot = Actor_GetWorld(&player->actor);
                    newEye = Camera_RotateAroundPointOot(&focusPosRot, &csEyeUpdate);
                    newAt = Camera_RotateAroundPointOot(&focusPosRot, &csAtUpdate);
                } else if (att->actionParameters == 8) {
                    // Around the shot's actor.
                    if (Hyrule_IsActorLoadedOot(play, att->target)) {
                        focusPosRot = Actor_GetWorld(att->target);
                        newEye = Camera_RotateAroundPointOot(&focusPosRot, &csEyeUpdate);
                        newAt = Camera_RotateAroundPointOot(&focusPosRot, &csAtUpdate);
                    } else {
                        att->target = NULL;
                        newEye = camera->eye;
                        newAt = camera->at;
                    }
                } else {
                    newEye = csEyeUpdate;
                    newAt = csAtUpdate;
                }

                camera->eyeNext = newEye;
                camera->eye = camera->eyeNext;
                if (anim->doLERPAt) {
                    Camera_LERPCeilVec3fOot(&newAt, &camera->at, 0.5f, 0.5f, 0.1f);
                } else {
                    camera->at = newAt;
                    anim->doLERPAt = true;
                }
                camera->roll = newRoll * 256.0f;
                anim->animTimer--;
                break;
            }
            // fallthrough
        case 3:
            // Finished: the shot ends, and may hand over.
            att->timer = 0;
            if (anim->finishAction != 0) {
                if (anim->finishAction != 0x1000) {
                    if (anim->finishAction == 0x2000) {
                        // Back to Link with OoT's shot 1020.
                        onePointTimer = (att->initTimer < 50) ? 5 : att->initTimer / 5;
                        OnePointCutscene_Init(play, 1020, onePointTimer, NULL, att->parentCamId);
                    }
                } else {
                    // The main camera goes on from this framing.
                    Camera_Copy(mainCam, camera);
                }
            }
            break;
        case 2:
            // The splines are over; wait for the timer.
            anim->animTimer--;
            if (anim->animTimer < 0) {
                att->animState++;
            }
            break;
        case 4:
            break;
    }
}

// OoT's Camera_Special0 (CAM_SET_PIVOT_VERTICAL): the eye stays, at follows the shot's actor.
static void Camera_Special0Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
    PosRot targetPosRot;

    att->interfaceFlags = 0x3200;
    if (att->animState == 0) {
        att->animState++;
    }
    if (!Hyrule_IsActorLoadedOot(play, att->target)) {
        att->target = NULL;
        return;
    }
    targetPosRot = Actor_GetFocus(att->target);
    // PIVOT_VERTICAL's lerpAtScale, 20%.
    Camera_LERPCeilVec3fOot(&targetPosRot.pos, &camera->at, 0.2f, 0.2f, 0.1f);
    camera->dist = OLib_Vec3fDist(&camera->at, &camera->eye);
    if (att->timer > 0) {
        att->timer--;
    }
}

// OoT's Camera_Demo6 (CAM_SET_FOREST_DEFEAT_POE): the Forest Temple's defeated Poes' flames, followed to their torches.
static void Camera_Demo6Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
    static const s16 sStateTimers[] = { 0, 0x37, 0x46, 0x5A };
    Camera* mainCam = play->cameraPtrs[CAM_ID_MAIN];
    Player* player = GET_PLAYER(play);
    Actor* camFocus = att->target;
    PosRot focusPosRot;
    VecGeo eyeOffset;

    att->interfaceFlags = 0x3200;
    if (!Hyrule_IsActorLoadedOot(play, camFocus)) {
        att->target = NULL;
        att->timer = 0;
        return;
    }

    switch (att->animState) {
        case 0:
            att->demo6AnimTimer = 0;
            camera->fov = 60.0f;
            focusPosRot = Actor_GetWorld(camFocus);
            camera->at.x = focusPosRot.pos.x;
            camera->at.y = focusPosRot.pos.y + 20.0f;
            camera->at.z = focusPosRot.pos.z;
            eyeOffset.r = 200.0f;
            eyeOffset.yaw = Camera_XZAngleOot(&focusPosRot.pos, &mainCam->focalActorPosRot.pos) + 0x7D0;
            eyeOffset.pitch = -0x3E8;
            camera->eyeNext = OLib_AddVecGeoToVec3f(&camera->at, &eyeOffset);
            camera->eye = camera->eyeNext;
            att->animState++;
            // fallthrough
        case 1:
            if (sStateTimers[att->animState] < att->demo6AnimTimer) {
                Player_SetCsActionWithHaltedActorsOot(play, &player->actor, 8);
                focusPosRot = Actor_GetWorld(camFocus);
                att->demo6AtTarget.x = focusPosRot.pos.x;
                att->demo6AtTarget.y = focusPosRot.pos.y - 20.0f;
                att->demo6AtTarget.z = focusPosRot.pos.z;
                att->animState++;
            } else {
                break;
            }
            // fallthrough
        case 2:
            Camera_LERPCeilVec3fOot(&att->demo6AtTarget, &camera->at, 0.1f, 0.1f, 8.0f);
            if (sStateTimers[att->animState] < att->demo6AnimTimer) {
                att->animState++;
            } else {
                break;
            }
            // fallthrough
        case 3:
            camera->fov = Camera_LERPCeilFOot(50.0f, camera->fov, 0.2f, 0.01f);
            if (sStateTimers[att->animState] < att->demo6AnimTimer) {
                att->timer = 0;
                return;
            }
            break;
    }

    att->demo6AnimTimer++;
}

// OoT's Camera_Special4 (CAM_SET_FIRE_STAIRCASE): the Fire Temple's falling staircase.
static void Camera_Special4Oot(PlayState* play, Camera* camera, OnePointCamOot* att) {
    PosRot curTargetPosRot;
    s16 sp3A;

    if (att->animState == 0) {
        att->interfaceFlags = 0x3200;
        camera->fov = 40.0f;
        att->animState++;
        att->spec4InitialTimer = att->timer;
    }

    camera->fov = Camera_LERPCeilFOot(80.0f, camera->fov, 1.0f / att->timer, 0.1f);
    if ((att->spec4InitialTimer - att->timer) < 0xF) {
        att->timer--;
        return;
    }
    if (!Hyrule_IsActorLoadedOot(play, att->target)) {
        att->target = NULL;
        att->timer = 0;
        return;
    }
    camera->roll = -0x1F4;
    curTargetPosRot = Actor_GetWorld(att->target);

    camera->at = curTargetPosRot.pos;
    camera->at.y -= 150.0f;

    // 0x3E8 ~ 5.49 degrees
    sp3A = BINANG_ROT180(curTargetPosRot.rot.y) + 0x3E8;
    camera->eye.x = camera->eyeNext.x = (Math_SinS(sp3A) * 780.0f) + camera->at.x;
    camera->eyeNext.y = camera->at.y;
    camera->eye.z = camera->eyeNext.z = (Math_CosS(sp3A) * 780.0f) + camera->at.z;
    camera->eye.y = curTargetPosRot.pos.y;
    camera->eye.y = Camera_GetFloorYOot(play, &camera->eye) + 20.0f;
    att->timer--;
}

// OoT's Camera_Finish: the shot is over, its parent camera takes over.
static void Hyrule_FinishOnePointCameraOot(PlayState* play, s16 camId) {
    OnePointCamOot* att = &sCamsOot[camId];
    Player* player = GET_PLAYER(play);
    s16 parentCamId = att->parentCamId;
    s16 childCamId = att->childCamId;

    Play_ChangeCameraStatus(play, parentCamId, CAM_STATUS_ACTIVE);

    if ((parentCamId == CAM_ID_MAIN) && (att->csId != 0)) {
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

static s32 Hyrule_IsOnePointCameraOot(PlayState* play, s16 camId) {
    return (camId >= CAM_ID_SUB_FIRST) && (camId < NUM_CAMS) && sCamsOot[camId].inUse &&
           (play->cameraPtrs[camId] != NULL);
}

void Hyrule_FinishOnePointCameraShotOot(PlayState* play) {
    s16 camId = play->activeCamId;

    // OoT's Camera_Finish, at the end of the frame the shot's timer ran out on.
    if (Hyrule_IsOnePointCameraOot(play, camId) && (sCamsOot[camId].timer == 0)) {
        Hyrule_FinishOnePointCameraOot(play, camId);
    }
}

void Hyrule_UpdateOnePointCameraShotOot(PlayState* play) {
    s16 camId = play->activeCamId;
    OnePointCamOot* att;
    Camera* camera;

    // A shot an actor ended this frame (OnePointCutscene_EndCutscene) finishes before it plays again.
    Hyrule_FinishOnePointCameraShotOot(play);
    camId = play->activeCamId;
    if (!Hyrule_IsOnePointCameraOot(play, camId)) {
        return;
    }

    att = &sCamsOot[camId];
    camera = play->cameraPtrs[camId];
    switch (att->setting) {
        case ONEPOINT_SET_CS_ATTENTION:
            Camera_Demo5Oot(play, camera, att);
            break;
        case ONEPOINT_SET_CS_C:
            Camera_Unique9Oot(play, camera, att);
            break;
        case ONEPOINT_SET_CS_3:
            Camera_Demo9Oot(play, camera, att);
            break;
        case ONEPOINT_SET_PIVOT_VERTICAL:
            Camera_Special0Oot(play, camera, att);
            break;
        case ONEPOINT_SET_FOREST_DEFEAT_POE:
            Camera_Demo6Oot(play, camera, att);
            break;
        case ONEPOINT_SET_FIRE_STAIRCASE:
            Camera_Special4Oot(play, camera, att);
            break;
        case ONEPOINT_SET_FREE2:
        case ONEPOINT_SET_TURN_AROUND:
        default:
            Camera_Unique6Oot(play, camera, att);
            break;
    }

    // OoT's Camera_UpdateInterface(sCameraInterfaceFlags): OoT's and MM's flags share their layout.
    Camera_UpdateInterface(att->interfaceFlags);
}

void Hyrule_ResetAttentionCameraOot(void) {
    memset(sCamsOot, 0, sizeof(sCamsOot));
}
