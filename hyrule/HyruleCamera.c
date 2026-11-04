// OoT's camera calls for ported actors (HyruleCamera.h). The camera is MM's FREE0, which leaves its at and eye to whoever
// sets them; here they are set as OoT's camera functions would.

#include "global.h"
#include "HyruleCamera.h"

typedef struct {
    /* 0x00 */ CutsceneCameraMoveOot* move;
    /* 0x04 */ PosRot playerPosRot;
    /* 0x18 */ f32 curFrame;
    /* 0x1C */ s16 keyframe;
    /* 0x1E */ s16 active;
} CsCameraMoveOot;

static CsCameraMoveOot sCsCameraMoveOot;

void Hyrule_StopCsCameraOot(void) {
    sCsCameraMoveOot.active = false;
}

void Hyrule_SetCsCameraAngleOot(PlayState* play, CutsceneCameraAngleOot* angle) {
    Camera* activeCam = GET_ACTIVE_CAM(play);

    Hyrule_StopCsCameraOot();
    Camera_ChangeSetting(activeCam, CAM_SET_FREE0);
    activeCam->at = angle->at;
    activeCam->eye = activeCam->eyeNext = angle->eye;
    activeCam->roll = angle->roll;
    activeCam->fov = angle->fov;
}

void Hyrule_SetCsCameraMoveOot(PlayState* play, CutsceneCameraMoveOot* move) {
    Camera* activeCam = GET_ACTIVE_CAM(play);

    Camera_ChangeSetting(activeCam, CAM_SET_FREE0);
    activeCam->animState = 0; // OoT's Camera_ResetAnim
    sCsCameraMoveOot.move = move;
    sCsCameraMoveOot.keyframe = 0;
    sCsCameraMoveOot.curFrame = 0.0f;
    sCsCameraMoveOot.active = true;
}

// OoT's Camera_RotateAroundPoint: pos turned by the yaw of posRot and moved to its position.
static Vec3f Hyrule_RotateAroundPointOot(PosRot* posRot, Vec3f* pos) {
    VecGeo geo = OLib_Vec3fToVecGeo(pos);

    geo.yaw += posRot->rot.y;
    return OLib_AddVecGeoToVec3f(&posRot->pos, &geo);
}

void Hyrule_UpdateCsCameraOot(PlayState* play) {
    Camera* activeCam = GET_ACTIVE_CAM(play);
    CutsceneCameraMoveOot* move = sCsCameraMoveOot.move;
    Player* player = GET_PLAYER(play);
    Vec3f eye;
    Vec3f at;
    f32 roll;
    s32 done;

    if (!sCsCameraMoveOot.active) {
        return;
    }
    if (activeCam->setting != CAM_SET_FREE0) {
        Hyrule_StopCsCameraOot();
        return;
    }

    // As OoT's Camera_Demo1, both splines step on the same keyframe.
    done = Hyrule_CsSplineStepOot(&eye, &roll, &activeCam->fov, move->eyePoints, &sCsCameraMoveOot.keyframe,
                                  &sCsCameraMoveOot.curFrame) ||
           Hyrule_CsSplineStepOot(&at, &roll, &activeCam->fov, move->atPoints, &sCsCameraMoveOot.keyframe,
                                  &sCsCameraMoveOot.curFrame);

    if (move->relativeToPlayer && (player != NULL)) {
        PosRot playerPosRot = Actor_GetWorld(&player->actor);

        activeCam->eyeNext = Hyrule_RotateAroundPointOot(&playerPosRot, &eye);
        activeCam->at = Hyrule_RotateAroundPointOot(&playerPosRot, &at);
    } else {
        activeCam->eyeNext = eye;
        activeCam->at = at;
    }
    activeCam->eye = activeCam->eyeNext;
    activeCam->roll = roll * 256.0f;
    activeCam->dist = OLib_Vec3fDist(&activeCam->at, &activeCam->eye);

    if (done) {
        // As OoT's Camera_Demo1, which actors wait on (animState 2: the move is over).
        activeCam->animState = 2;
        Hyrule_StopCsCameraOot();
    }
}
