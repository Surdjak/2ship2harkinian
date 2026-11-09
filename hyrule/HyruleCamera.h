#ifndef HYRULE_CAMERA_H
#define HYRULE_CAMERA_H

// OoT's camera calls that ported actors make on the active camera, which MM's camera has no counterpart for. See
// hyrule/README.md (Cutscenes).

#include "ultra64.h"
#include "HyruleCutscene.h"

struct PlayState;

// OoT's CAM_SET_FREE0 with a set angle: the camera holds the given at, eye, roll and fov.
void Hyrule_SetCsCameraAngleOot(struct PlayState* play, CutsceneCameraAngleOot* angle);
// OoT's CAM_SET_CS_0 with Camera_SetCSParams: the camera follows the eye and at splines, from Link's position and yaw
// when relativeToPlayer, as OoT's Camera_Demo1 does, until they end; then it holds still with its animState 2, which
// OoT's actors wait for.
void Hyrule_SetCsCameraMoveOot(struct PlayState* play, CutsceneCameraMoveOot* move);
// Each frame from Play_Update, before the cameras update: moves the active camera along its splines.
void Hyrule_UpdateCsCameraOot(struct PlayState* play);
// Stops a spline move (a camera setting requested by an actor, a scene change).
void Hyrule_StopCsCameraOot(void);

// OoT's one-point shots (HyruleOnePoint.c), each frame from Play_Update before the cameras update: plays the active one.
void Hyrule_UpdateOnePointCameraShotOot(struct PlayState* play);
// After the cameras update: a shot whose timer ran out hands back to the camera it took over from (OoT's Camera_Finish).
void Hyrule_FinishOnePointCameraShotOot(struct PlayState* play);
// Forgets the shots' queue, whose sub cameras a new scene clears.
void Hyrule_ResetAttentionCameraOot(void);

#endif
