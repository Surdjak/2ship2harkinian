// OoT's cutscene interpreter (SoH's z_demo.c), for OoT's scripts (docs/CUTSCENES.md). MM's cutscenes keep MM's.
// OoT's scene cutscenes (its entrance cutscene table and cutscene layers) are HyruleSceneCutscene.c.

#include "global.h"
#include "z64shrink_window.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "HyruleCutscene.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleScenes.h"
#include "HyruleOcarina.h"
#include "HyruleInventory.h"
#include "BenPort.h"
#include <string.h>

CutsceneContextOot gCsCtxOot;
s8 gLinkAgeOnLoadOot = -1;
// OoT's trigger for its own scripts (gSaveContext.cutsceneTrigger), kept apart from MM's, whose update would start
// MM's script instead.
static u8 sCutsceneTriggerOot;

u16 D_8011E1C0 = 0;
u16 D_8011E1C4 = 0;

typedef void (*CutsceneStateHandler)(PlayState*, CutsceneContextOot*);

static void func_80064720(PlayState* play, CutsceneContextOot* csCtx);
static void func_80064760(PlayState* play, CutsceneContextOot* csCtx);
static void func_800647C0(PlayState* play, CutsceneContextOot* csCtx);
static void func_80068C3C(PlayState* play, CutsceneContextOot* csCtx);
static void func_80068D84(PlayState* play, CutsceneContextOot* csCtx);
static void func_80068DC0(PlayState* play, CutsceneContextOot* csCtx);

CutsceneStateHandler sCsStateHandlers1[] = {
    func_80064720, func_80064760, func_80064720, func_80068D84, func_80064720,
};

CutsceneStateHandler sCsStateHandlers2[] = {
    func_80064720, func_800647C0, func_80068C3C, func_80068DC0, func_80068C3C,
};

u8 sTitleCsState = 0;

u16 D_8015FCC0;
u16 D_8015FCC2;
u16 D_8015FCC4;
s16 D_8015FCC6;
u8 D_8015FCC8;
s16 sQuakeIndex;
u16 D_8015FCCC;      // only written to, never read
char D_8015FCD0[20]; // unreferenced
u8 D_8015FCE4;       // only written to, never read

static void func_80068ECC(PlayState* play, CutsceneContextOot* csCtx);

// OoT's cutscene camera: the eye and at spline points of the current camera commands, followed as OoT's Camera_Demo1
// does (code_800BB0A0.c), on the cutscene's sub-camera.
static struct {
    s16 camId;
    CutsceneCameraPointOot* atPoints;
    CutsceneCameraPointOot* eyePoints;
    s16 relativeToPlayer;
    s16 keyframe;
    f32 curFrame;
    u8 active;
} sCsCamera;

// OoT's func_800BB0A0: a uniform cubic B-spline through four points (pos, roll and view angle).
static void CsCamera_Interpolate(f32 u, Vec3f* pos, f32* roll, f32* viewAngle, f32* point0, f32* point1, f32* point2,
                                 f32* point3) {
    f32 coeff[4];

    u = CLAMP_MAX(u, 1.0f);

    coeff[0] = (1.0f - u) * (1.0f - u) * (1.0f - u) / 6.0f;
    coeff[1] = u * u * u / 2.0f - u * u + 2.0f / 3.0f;
    coeff[2] = -u * u * u / 2.0f + u * u / 2.0f + u / 2.0f + 1.0f / 6.0f;
    coeff[3] = u * u * u / 6.0f;

    pos->x = (coeff[0] * point0[0]) + (coeff[1] * point1[0]) + (coeff[2] * point2[0]) + (coeff[3] * point3[0]);
    pos->y = (coeff[0] * point0[1]) + (coeff[1] * point1[1]) + (coeff[2] * point2[1]) + (coeff[3] * point3[1]);
    pos->z = (coeff[0] * point0[2]) + (coeff[1] * point1[2]) + (coeff[2] * point2[2]) + (coeff[3] * point3[2]);
    *roll = (coeff[0] * point0[3]) + (coeff[1] * point1[3]) + (coeff[2] * point2[3]) + (coeff[3] * point3[3]);
    *viewAngle = (coeff[0] * point0[4]) + (coeff[1] * point1[4]) + (coeff[2] * point2[4]) + (coeff[3] * point3[4]);
}

// OoT's func_800BB2B4: the point on the spline at the current keyframe, then steps along it; true at its end.
static s32 CsCamera_Step(Vec3f* pos, f32* roll, f32* fov, CutsceneCameraPointOot* point, s16* keyFrame,
                         f32* curFrame) {
    s32 ret = false;
    f32 pointData[4][5];
    s32 i;
    f32 progress = *curFrame;
    s32 key = *keyFrame;
    f32 speed1 = 0.0f;
    f32 speed2 = 0.0f;
    f32 advance;

    if (key < 0) {
        progress = 0.0f;
    }

    if ((point[key].continueFlag == -1) || (point[key + 1].continueFlag == -1) || (point[key + 2].continueFlag == -1)) {
        return true;
    }

    for (i = 0; i < 4; i++) {
        pointData[i][0] = point[key + i].pos.x;
        pointData[i][1] = point[key + i].pos.y;
        pointData[i][2] = point[key + i].pos.z;
        pointData[i][3] = point[key + i].cameraRoll;
        pointData[i][4] = point[key + i].viewAngle;
    }

    CsCamera_Interpolate(progress, pos, roll, fov, pointData[0], pointData[1], pointData[2], pointData[3]);

    if (point[*keyFrame + 1].nextPointFrame != 0) {
        speed1 = 1.0f / point[*keyFrame + 1].nextPointFrame;
    }

    if (point[*keyFrame + 2].nextPointFrame != 0) {
        speed2 = 1.0f / point[*keyFrame + 2].nextPointFrame;
    }
    advance = (*curFrame * (speed2 - speed1)) + speed1;
    if (advance < 0.0f) {
        advance = 0;
    }
    *curFrame += advance;
    if (*curFrame >= 1) {
        if (point[++*keyFrame + 3].continueFlag == -1) {
            *keyFrame = 0;
            ret = true;
        }
        *curFrame -= 1;
    }

    return ret;
}

// OoT's Camera_SetCSParams and Camera_ResetAnim.
static void CsCamera_Start(PlayState* play, s16 camId, CutsceneCameraPointOot* atPoints,
                           CutsceneCameraPointOot* eyePoints, s16 relativeToPlayer) {
    sCsCamera.camId = camId;
    sCsCamera.atPoints = atPoints;
    sCsCamera.eyePoints = eyePoints;
    sCsCamera.relativeToPlayer = relativeToPlayer;
    sCsCamera.keyframe = 0;
    sCsCamera.curFrame = 0.0f;
    sCsCamera.active = true;
}

// OoT's Camera_RotateAroundPoint: pos turned by the yaw of `at` and added to its position.
static void CsCamera_RotateAroundPoint(PosRot* at, Vec3f* pos, Vec3f* dst) {
    VecGeo geo = OLib_Vec3fToVecGeo(pos);

    geo.yaw += at->rot.y;
    *dst = OLib_AddVecGeoToVec3f(&at->pos, &geo);
}

// OoT's Camera_Demo1, run each frame while a spline is set.
static void CsCamera_Update(PlayState* play) {
    Camera* camera;
    Player* player = GET_PLAYER(play);
    Vec3f eye;
    Vec3f at;
    f32 roll;
    f32 fov;

    if (!sCsCamera.active || (sCsCamera.camId <= CAM_ID_MAIN) || (sCsCamera.atPoints == NULL) ||
        (sCsCamera.eyePoints == NULL)) {
        return;
    }
    camera = Play_GetCamera(play, sCsCamera.camId);
    if (camera == NULL) {
        return;
    }
    fov = camera->fov;
    if (CsCamera_Step(&eye, &roll, &fov, sCsCamera.eyePoints, &sCsCamera.keyframe, &sCsCamera.curFrame) ||
        CsCamera_Step(&at, &roll, &fov, sCsCamera.atPoints, &sCsCamera.keyframe, &sCsCamera.curFrame)) {
        sCsCamera.active = false;
    }
    if (sCsCamera.relativeToPlayer) {
        if ((player != NULL) && (player->actor.update != NULL)) {
            PosRot playerPosRot = player->actor.world;

            CsCamera_RotateAroundPoint(&playerPosRot, &eye, &eye);
            CsCamera_RotateAroundPoint(&playerPosRot, &at, &at);
        }
    }
    Play_SetCameraAtEye(play, sCsCamera.camId, &at, &eye);
    Play_SetCameraFov(play, sCsCamera.camId, fov);
    Play_SetCameraRoll(play, sCsCamera.camId, roll * 256.0f);
}

static void func_8006450C(PlayState* play, CutsceneContextOot* csCtx) {
    csCtx->state = CS_STATE_IDLE_Oot;
    csCtx->unk_0C = 0.0f;
}

static void func_80064520(PlayState* play, CutsceneContextOot* csCtx) {
    csCtx->state = CS_STATE_SKIPPABLE_INIT_Oot;
    csCtx->linkAction = NULL;
}

static void func_80064534(PlayState* play, CutsceneContextOot* csCtx) {
    if (csCtx->state != CS_STATE_UNSKIPPABLE_EXEC_Oot) {
        csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
    }
}

static void func_80064558(PlayState* play, CutsceneContextOot* csCtx) {
    if (gSaveContext.save.cutsceneIndex < 0xFFF0) {
        sCsStateHandlers1[csCtx->state](play, csCtx);
    }
}

static void func_800645A0(PlayState* play, CutsceneContextOot* csCtx) {
    Input* input = &play->state.input[0];

    if (false && CHECK_BTN_ALL(input->press.button, BTN_DLEFT) &&
        (csCtx->state == CS_STATE_IDLE_Oot) && (gSaveContext.sceneLayer >= 4)) {
        D_8015FCC8 = 0;
        gSaveContext.save.cutsceneIndex = 0xFFFD;
        Hyrule_TriggerCutsceneOot(1);
    }

    if (false && CHECK_BTN_ALL(input->press.button, BTN_DUP) &&
        (csCtx->state == CS_STATE_IDLE_Oot) && (gSaveContext.sceneLayer >= 4) && !gDbgCamEnabled) {
        D_8015FCC8 = 1;
        gSaveContext.save.cutsceneIndex = 0xFFFD;
        Hyrule_TriggerCutsceneOot(1);
    }

    if ((sCutsceneTriggerOot != 0) && (play->transitionTrigger == TRANS_TRIGGER_START)) {
        Hyrule_TriggerCutsceneOot(0);
    }

    if ((sCutsceneTriggerOot != 0) && (csCtx->state == CS_STATE_IDLE_Oot)) { // "Cutscene start request announcement!"
        gSaveContext.save.cutsceneIndex = 0xFFFD;
        Hyrule_TriggerCutsceneOot(1);
    }

    if (gSaveContext.save.cutsceneIndex >= 0xFFF0) {
        func_80068ECC(play, csCtx);
        sCsStateHandlers2[csCtx->state](play, csCtx);
    }
}

static void func_80064720(PlayState* play, CutsceneContextOot* csCtx) {
}

static u32 func_8006472C(PlayState* play, CutsceneContextOot* csCtx, f32 target) {
    return Math_StepToF(&csCtx->unk_0C, target, 0.1f);
}

static void func_80064760(PlayState* play, CutsceneContextOot* csCtx) {
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
    ShrinkWindow_Letterbox_SetSizeTarget(0x20);

    if (func_8006472C(play, csCtx, 1.0f)) {
        Audio_SetCutsceneFlag(1);
        csCtx->state++;
    }
}

static void func_800647C0(PlayState* play, CutsceneContextOot* csCtx) {
    func_80068C3C(play, csCtx);
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
    ShrinkWindow_Letterbox_SetSizeTarget(0x20);

    if (func_8006472C(play, csCtx, 1.0f)) {
        Audio_SetCutsceneFlag(1);
        csCtx->state++;
    }
}

// Command 3: Misc. Actions
static void func_80064824(PlayState* play, CutsceneContextOot* csCtx, CsCmdBaseOot* cmd) {
    Player* player = GET_PLAYER(play);
    f32 temp;
    u8 sp3F = 0;

    if ((csCtx->frames < cmd->startFrame) || ((csCtx->frames >= cmd->endFrame) && (cmd->endFrame != cmd->startFrame))) {
        return;
    }

    temp = Environment_LerpWeight(cmd->endFrame - 1, cmd->startFrame, csCtx->frames);

    if (csCtx->frames == cmd->startFrame) {
        sp3F = 1;
    }

    switch (cmd->base) {
        case 1:
            if (sp3F != 0) {
                Audio_SetAmbienceChannelIO(AMBIENCE_CHANNEL_RAIN, CHANNEL_IO_PORT_4, 0x3F);
                Audio_SetAmbienceChannelIO(AMBIENCE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 1);
                play->envCtx.precipitation[0] = 20;
            }
            break;
        case 2:
            if (sp3F != 0) {
                Audio_SetAmbienceChannelIO(AMBIENCE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_0, 0);
                Environment_AddLightningBolts(play, 3);
                gLightningStrike.state = LIGHTNING_STRIKE_START;
            }
            break;
        case 3:
            if (sp3F != 0) {
                Flags_SetEnvOot(play, 0);
                if (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x053 /* ENTR_TEMPLE_OF_TIME_ENTRANCE */)) {
                    Flags_SetEnvOot(play, 2);
                }
            }
            break;
        case 6:
            if (play->envCtx.adjLightSettings.zFar < 12800) {
                play->envCtx.adjLightSettings.zFar += 35;
            }
            break;
        case 7:
            if (sp3F != 0) {
                play->envCtx.changeSkyboxState = 1;
                play->envCtx.skyboxConfig = 1;
                play->envCtx.changeSkyboxNextConfig = 0;
                play->envCtx.changeSkyboxTimer = 0x3C;
                play->envCtx.changeLightEnabled = 1;
                play->envCtx.lightConfig = 0;
                play->envCtx.changeLightNextConfig = 1;
                play->envCtx.changeLightTimer = play->envCtx.changeDuration = 0x3C;
            }
            break;
        case 8:
            if (play->roomCtx.unk7A[0] < 0x80) {
                play->roomCtx.unk7A[0] += 4;
            }
            break;
        case 9:
            play->envCtx.precipitation[3] = 16;
            break;
        case 10:
            Flags_SetEnvOot(play, 1);
            break;
        case 11:
            if (play->roomCtx.unk7A[0] < 0x672) {
                play->roomCtx.unk7A[0] += 0x14;
            }
            if (csCtx->frames == 0x30F) {
                Audio_PlaySfx(NA_SE_EV_CHIBI_FAIRY_SAVED);
            } else if (csCtx->frames == 0x2CD) {
                play->roomCtx.unk7A[0] = 0;
            }
            break;
        case 12:
            if (sp3F != 0) {
                if (csCtx->state != CS_STATE_UNSKIPPABLE_EXEC_Oot) {
                    csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
                }
            }
            break;
        case 13:
            if (play->roomCtx.unk7A[1] == 0) {
                Audio_PlaySfx(NA_SE_EV_SUN_MARK_FLASH);
            }
            if (play->roomCtx.unk7A[1] < 0xFF) {
                play->roomCtx.unk7A[1] += 5;
            }
            break;
        case 14:
            if (sp3F != 0) {
                Hyrule_SetViewpoint(play, 1);
            }
            break;
        case 15:
            if (sp3F != 0) {
                // OoT's place-name card is MM's scene title text.
                Message_DisplaySceneTitleCard(play, play->loadedScene->titleTextId);
            }
            break;
        case 16:
            if (sp3F != 0) {
                sQuakeIndex = Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_6);
                Quake_SetSpeed(sQuakeIndex, 0x7FFF);
                Quake_SetPerturbations(sQuakeIndex, 4, 0, 1000, 0);
                Quake_SetDuration(sQuakeIndex, 800);
            }
            break;
        case 17:
            if (sp3F != 0) {
                Quake_RemoveRequest(sQuakeIndex);
            }
            break;
        case 18:
            play->envCtx.precipitation[0] = 0;
            play->envCtx.stormRequest = 2;
            if (gSaveContext.save.time < 0x4AAB) {
                gSaveContext.save.time += 30;
            }
            if (play->envCtx.precipitation[1] == 0) {
                gWeatherMode = 0;
                Audio_SetAmbienceChannelIO(AMBIENCE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 0);
            }
            break;
        case 19:
            SET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SONG_OF_STORMS_IN_WINDMILL_Oot);
            break;
        case 20:
            SET_EVENTCHKINF_OOT(EVENTCHKINF_DRAINED_WELL_IN_KAKARIKO_Oot);
            break;
        case 21:
            SET_EVENTCHKINF_OOT(EVENTCHKINF_RAISED_LAKE_HYLIA_WATER_Oot);
            break;
        case 22:
            gVisMonoColor.r = 255;
            gVisMonoColor.g = 255;
            gVisMonoColor.b = 255;
            gVisMonoColor.a = 255;
            break;
        case 23:
            gVisMonoColor.r = 255;
            gVisMonoColor.g = 180;
            gVisMonoColor.b = 100;
            gVisMonoColor.a = 255.0f * temp;
            break;
        case 24:
            play->roomCtx.curRoom.segment = NULL;
            break;
        case 25:
            gSaveContext.save.time += 30;
            if ((gSaveContext.save.time) > 0xCAAA) {
                gSaveContext.save.time = 0xCAAA;
            }
            break;
        case 26:
            if ((gSaveContext.save.time < 0x3000) || (gSaveContext.save.time >= 0x4555)) {
                if ((gSaveContext.save.time >= 0x4555) && (gSaveContext.save.time < 0xAAAB)) {
                    play->envCtx.lightSettingOverride = 1;
                } else if ((gSaveContext.save.time >= 0xAAAB) && (gSaveContext.save.time < 0xC556)) {
                    play->envCtx.lightSettingOverride = 2;
                } else {
                    play->envCtx.lightSettingOverride = 3;
                }
            }
            break;
        case 27:
            if (play->state.frames & 8) {
                if (play->envCtx.adjLightSettings.ambientColor[0] < 40) {
                    play->envCtx.adjLightSettings.ambientColor[0] += 2;
                    play->envCtx.adjLightSettings.light1Color[1] -= 3;
                    play->envCtx.adjLightSettings.light1Color[2] -= 3;
                }
            } else {
                if (play->envCtx.adjLightSettings.ambientColor[0] > 2) {
                    play->envCtx.adjLightSettings.ambientColor[0] -= 2;
                    play->envCtx.adjLightSettings.light1Color[1] += 3;
                    play->envCtx.adjLightSettings.light1Color[2] += 3;
                }
            }
            break;
        case 28:
            play->haltAllActors = 1;
            break;
        case 29:
            play->haltAllActors = 0;
            break;
        case 30:
            Flags_SetEnvOot(play, 3);
            break;
        case 31:
            Flags_SetEnvOot(play, 4);
            break;
        case 32:
            if (sp3F != 0) {
                play->envCtx.sandstormState = SANDSTORM_FILL;
            }
            Audio_PlaySfx_2(NA_SE_EV_SAND_STORM - SFX_FLAG);
            break;
        case 33:
            gSaveContext.sunsSongState = SUNSSONG_START;
            break;
        case 34:
            if (IS_DAY_OOT) {
                gSaveContext.save.time -= R_TIME_SPEED;
            } else {
                gSaveContext.save.time -= R_TIME_SPEED * 2;
            }
            break;
        case 35:
            AudioOcarina_PlayLongScarecrowSong();
            csCtx->frames = cmd->startFrame - 1;
            break;
    }
}

// Command 4: Set Environment Lighting
static void Cutscene_Command_SetLighting(PlayState* play, CutsceneContextOot* csCtx, CsCmdEnvLightingOot* cmd) {
    if (csCtx->frames == cmd->startFrame) {
        play->envCtx.lightSettingOverride = cmd->setting - 1;
        play->envCtx.lightBlend = 1.0f;
    }
}

// Command 0x56: Play Background Music
static void Cutscene_Command_PlayBGM(PlayState* play, CutsceneContextOot* csCtx, CsCmdMusicChangeOot* cmd) {
    if (csCtx->frames == cmd->startFrame) {
        Audio_PlaySequenceInCutscene(SEQ_OOT(cmd->sequence - 1));
    }
}

// Command 0x57: Stop Background Music
static void Cutscene_Command_StopBGM(PlayState* play, CutsceneContextOot* csCtx, CsCmdMusicChangeOot* cmd) {
    if (csCtx->frames == cmd->startFrame) {
        Audio_StopSequenceInCutscene(SEQ_OOT(cmd->sequence - 1));
    }
}

// Command 0x7C: Fade Background Music over duration
static void Cutscene_Command_FadeBGM(PlayState* play, CutsceneContextOot* csCtx, CsCmdMusicFadeOot* cmd) {
    u8 var1;

    if ((csCtx->frames == cmd->startFrame) && (csCtx->frames < cmd->endFrame)) {
        var1 = cmd->endFrame - cmd->startFrame;

        if (cmd->type == 3) {
            AudioSeq_QueueSeqCmd(var1 << 0x10 | (0x1 << 28 | SEQ_PLAYER_FANFARE << 24 | 0xFF));
        } else {
            AudioSeq_QueueSeqCmd(var1 << 0x10 | (0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0xFF));
        }
    }
}

// Command 9: ?
static void Cutscene_Command_09(PlayState* play, CutsceneContextOot* csCtx, CsCmdUnknown9Oot* cmd) {
    if (csCtx->frames == cmd->startFrame) {
        Rumble_Request(0.0f, cmd->unk_06, cmd->unk_07, cmd->unk_08);
    }
}

// Command 0x8C: Set Time of Day & Environment Time
static void func_80065134(PlayState* play, CutsceneContextOot* csCtx, CsCmdDayTimeOot* cmd) {
    s16 temp1;
    s16 temp2;

    if (csCtx->frames == cmd->startFrame) {
        temp1 = (cmd->hour * 60.0f) / (360.0f / 0x4000);
        temp2 = (cmd->minute + 1) / (360.0f / 0x4000);

        gSaveContext.save.time = temp1 + temp2;
        gSaveContext.skyboxTime = temp1 + temp2;
    }
}

// Command 0x3E8: Code Execution (& Terminates Cutscene?)
static void Cutscene_Command_Terminator(PlayState* play, CutsceneContextOot* csCtx, CsCmdBaseOot* cmd) {
    Player* player = GET_PLAYER(play);
    s32 temp = 0;

    bool shouldSkipCommand = cmd->base == 8 && !(true);
    bool debugCsSkip = CHECK_BTN_ALL(play->state.input[0].press.button, BTN_START) &&
                       (gSaveContext.fileNum != 0xFEDC) && false;

    if ((gSaveContext.gameMode != GAMEMODE_NORMAL) && (gSaveContext.gameMode != GAMEMODE_END_CREDITS) &&
        (play->sceneId != SCENE_HYRULE_FIELD_Oot) && (csCtx->frames > 20) &&
        (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A) ||
         CHECK_BTN_ALL(play->state.input[0].press.button, BTN_B) ||
         CHECK_BTN_ALL(play->state.input[0].press.button, BTN_START)) &&
        (gSaveContext.fileNum != 0xFEDC) && (play->transitionTrigger == TRANS_TRIGGER_OFF)) {
        Audio_PlaySfx(NA_SE_SY_PIECE_OF_HEART);
        temp = 1;
    }

    bool playCutscene = false;
    if (cmd->startFrame == csCtx->frames) {
        playCutscene = true;
    } else if (false) {
        u16 delay = 0;

        // HACK:  Align visual timing with audio during credits sequence
        switch (cmd->base) {
            case 55: // Gerudo fortress (second scene of credits roll)
                delay = 20;
                break;
            case 56: // Kakariko village
                delay = 40;
                break;
            case 57: // Death mountain trail
                delay = 20;
                break;
            case 58: // Goron city
                delay = 20;
                break;
            case 59: // Lake hylia
                delay = 20;
                break;
            case 62: // Kokiri forest (houses)
                delay = 40;
                break;
            case 63: // Kokiri forest (deku tree)
                delay = 40;
                break;
            case 74: // First gorons dancing
                delay = 100;
                break;
            case 75: // Magic carpet guy and old shop keepers
                delay = 180;
                break;
            case 77: // Sad mido and king zora (plays after scene 78)
                delay = 100;
                break;
            case 78: // Second gorons dancing
                delay = 160;
                break;
        }
        if (cmd->startFrame + delay == csCtx->frames) {
            playCutscene = true;
        }
    }

    if (playCutscene || (temp != 0) || ((csCtx->frames > 20) && (shouldSkipCommand || debugCsSkip))) {

        csCtx->state = CS_STATE_UNSKIPPABLE_EXEC_Oot;
        Audio_SetCutsceneFlag(0);
        gSaveContext.cutsceneTransitionControl = 1;
 // "Future fork designation=No. [%d]"


        gSaveContext.save.cutsceneIndex = 0;

        switch (cmd->base) {
            case 1:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 2:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_WHITE;
                break;
            case 3:
                play->nextEntrance = Hyrule_ExitFromOot(0x117); // OoT's ENTR_GERUDO_VALLEY_EAST_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_WHITE;
                break;
            case 4:
                play->nextEntrance = Hyrule_ExitFromOot(0x13D); // OoT's ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_WHITE;
                break;
            case 5:
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_WHITE;
                break;
            case 6:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_WHITE;
                break;
            case 7:
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_INSTANT;
                break;
            case 8:
                gSaveContext.save.saveInfo.faroresWindOot.data = 0;
                gSaveContext.respawn[RESPAWN_MODE_TOP].data = 0;
                if ((!GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot))) {
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot);
                    play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    gSaveContext.save.cutsceneIndex = 0xFFF3;
                    play->transitionType = TRANS_TYPE_INSTANT;
                } else {
                    if (gSaveContext.sceneLayer < 4) {
                        if (!LINK_IS_ADULT_OOT) {
                            gLinkAgeOnLoadOot = 0;
                        } else {
                            gLinkAgeOnLoadOot = 1;
                        }
                    }
                    play->nextEntrance = Hyrule_ExitFromOot(0x2CA); // OoT's ENTR_TEMPLE_OF_TIME_2
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_WHITE;
                    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                }
                break;
            case 9:
                play->nextEntrance = Hyrule_ExitFromOot(0x117); // OoT's ENTR_GERUDO_VALLEY_EAST_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FILL_BROWN;
                break;
            case 10:
                play->nextEntrance = Hyrule_ExitFromOot(0x0BB); // OoT's ENTR_LINKS_HOUSE_CHILD_SPAWN
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 11:
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF3;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 12:
                play->nextEntrance = Hyrule_ExitFromOot(0x47A); // OoT's ENTR_DEATH_MOUNTAIN_TRAIL_DODONGO_BLUE_WARP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 13:
                play->nextEntrance = Hyrule_ExitFromOot(0x10E); // OoT's ENTR_ZORAS_FOUNTAIN_JABU_JABU_BLUE_WARP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 14:
                play->nextEntrance = Hyrule_ExitFromOot(0x457); // OoT's ENTR_KOKIRI_FOREST_DEKU_TREE_BLUE_WARP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 15:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 16:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF5;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 17:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF6;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 18:
                SET_EVENTCHKINF_OOT(EVENTCHKINF_ENTERED_MASTER_SWORD_CHAMBER_Oot);
                play->nextEntrance = Hyrule_ExitFromOot(0x324); // OoT's ENTR_TEMPLE_OF_TIME_4
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 19:
                play->nextEntrance = Hyrule_ExitFromOot(0x13D); // OoT's ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                gSaveContext.save.cutsceneIndex = 0x8000;
                break;
            case 21:
                play->nextEntrance = Hyrule_ExitFromOot(0x102); // OoT's ENTR_LAKE_HYLIA_NORTH_EXIT
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 22:
                if ((true)) {
                    Item_Give(play, ITEM_SONG_REQUIEM_Oot);
                }
                play->nextEntrance = Hyrule_ExitFromOot(0x123); // OoT's ENTR_DESERT_COLOSSUS_EAST_EXIT
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 23:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF8;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 24:
                    play->nextEntrance = Hyrule_ExitFromOot(0x028); // OoT's ENTR_JABU_JABU_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 25:
                gLinkAgeOnLoadOot = 0;
                play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 26:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 27:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF5;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 28:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF6;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 29:
                play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.chamberCutsceneNum = 0;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 30:
                play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                if ((true)) {
                    Item_Give(play, ITEM_MEDALLION_FIRE_Oot);
                }
                gSaveContext.chamberCutsceneNum = 1;
                break;
            case 31:
                play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                gSaveContext.chamberCutsceneNum = 2;
                break;
            case 32:
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionType = TRANS_TYPE_INSTANT;
                break;
            case 33:
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 34:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF3;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 35:
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 38:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 39:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF9;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 40:
                gLinkAgeOnLoadOot = 0;
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFFA;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 41:
                play->nextEntrance = Hyrule_ExitFromOot(0x4E6); // OoT's ENTR_LAKE_HYLIA_5
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 42:
                play->nextEntrance = Hyrule_ExitFromOot(0x0DB); // OoT's ENTR_KAKARIKO_VILLAGE_FRONT_GATE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 43:
                play->nextEntrance = Hyrule_ExitFromOot(0x503); // OoT's ENTR_WINDMILL_AND_DAMPES_GRAVE_2
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 44:
                play->nextEntrance = Hyrule_ExitFromOot(0x320); // OoT's ENTR_TEMPLE_OF_TIME_3
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE_INSTANT;
                break;
            case 46:
                SET_EVENTCHKINF_OOT(EVENTCHKINF_ENTERED_MASTER_SWORD_CHAMBER_Oot);
                play->nextEntrance = Hyrule_ExitFromOot(0x324); // OoT's ENTR_TEMPLE_OF_TIME_4
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 47:
                if ((true)) {
                    Item_Give(play, ITEM_SONG_NOCTURNE_Oot);
                }
                SET_EVENTCHKINF_OOT(EVENTCHKINF_LEARNED_NOCTURNE_OF_SHADOW_Oot);
                play->nextEntrance = Hyrule_ExitFromOot(0x0DB); // OoT's ENTR_KAKARIKO_VILLAGE_FRONT_GATE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 48:
                play->nextEntrance = Hyrule_ExitFromOot(0x1ED); // OoT's ENTR_DESERT_COLOSSUS_4
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_SANDSTORM_END;
                gSaveContext.nextTransitionType = TRANS_TYPE_SANDSTORM_END;
                break;
            case 49:
                play->nextEntrance = Hyrule_ExitFromOot(0x58C); // OoT's ENTR_TEMPLE_OF_TIME_5
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
                break;
            case 50:
                play->nextEntrance = Hyrule_ExitFromOot(0x513); // OoT's ENTR_KAKARIKO_VILLAGE_13
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE_INSTANT;
                break;
            case 51:
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                gSaveContext.save.cutsceneIndex = 0xFFF8;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_CIRCLE;
                break;
            case 52:
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                gSaveContext.save.cutsceneIndex = 0xFFF7;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_INSTANT;
                break;
            case 53:
                play->nextEntrance = Hyrule_ExitFromOot(0x50F); // OoT's ENTR_HYRULE_FIELD_16
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 54:
                gSaveContext.gameMode = GAMEMODE_END_CREDITS;
                AudioSfx_MuteBanks(0x6F);
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x117); // OoT's ENTR_GERUDO_VALLEY_EAST_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 55:
                play->nextEntrance = Hyrule_ExitFromOot(0x129); // OoT's ENTR_GERUDOS_FORTRESS_EAST_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 56:
                play->nextEntrance = Hyrule_ExitFromOot(0x0DB); // OoT's ENTR_KAKARIKO_VILLAGE_FRONT_GATE
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 57:
                play->nextEntrance = Hyrule_ExitFromOot(0x13D); // OoT's ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF3;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 58:
                play->nextEntrance = Hyrule_ExitFromOot(0x14D); // OoT's ENTR_GORON_CITY_UPPER_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 59:
                play->nextEntrance = Hyrule_ExitFromOot(0x102); // OoT's ENTR_LAKE_HYLIA_NORTH_EXIT
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 60:
                play->nextEntrance = Hyrule_ExitFromOot(0x10E); // OoT's ENTR_ZORAS_FOUNTAIN_JABU_JABU_BLUE_WARP
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 61:
                play->nextEntrance = Hyrule_ExitFromOot(0x108); // OoT's ENTR_ZORAS_DOMAIN_ENTRANCE
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 62:
                gLinkAgeOnLoadOot = 0;
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF6;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 63:
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF7;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 64:
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                gSaveContext.save.cutsceneIndex = 0xFFF5;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 65:
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 66:
                play->nextEntrance = Hyrule_ExitFromOot(0x554); // OoT's ENTR_KAKARIKO_VILLAGE_OWL_DROP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 67:
                play->nextEntrance = Hyrule_ExitFromOot(0x27E); // OoT's ENTR_HYRULE_FIELD_OWL_DROP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 68:
                play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF5;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 69:
                play->nextEntrance = Hyrule_ExitFromOot(0x5E8); // OoT's ENTR_KOKIRI_FOREST_12
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 70:
                play->nextEntrance = Hyrule_ExitFromOot(0x13D); // OoT's ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 71:
                SET_EQUIP_VALUE(EQUIP_TYPE_TUNIC, EQUIP_VALUE_TUNIC_KOKIRI);
                Player_SetEquipmentData(play, player);
                SET_EQUIP_VALUE(EQUIP_TYPE_BOOTS_Oot, EQUIP_VALUE_BOOTS_KOKIRI_Oot);
                Player_SetEquipmentData(play, player);
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 72:
                play->nextEntrance = Hyrule_ExitFromOot(0x400); // OoT's ENTR_CASTLE_COURTYARD_ZELDA_0
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF0;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 73:
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF2;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 74:
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF3;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 75:
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF4;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 76:
                gLinkAgeOnLoadOot = 0;
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF5;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 77:
                gLinkAgeOnLoadOot = 1;
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF6;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 78:
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF7;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 79:
            case 80:
            case 81:
            case 82:
            case 83:
            case 84:
            case 85:
            case 86:
            case 87:
            case 88:
            case 89:
            case 90:
            case 91:
            case 92:
            case 93:
                play->nextEntrance = Hyrule_ExitFromOot(0x157); // OoT's ENTR_LON_LON_RANCH_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 94:
                play->nextEntrance = Hyrule_ExitFromOot(0x2AE); // OoT's ENTR_LON_LON_RANCH_1
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 95:
                if ((GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP_Oot)) &&
                    (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FIRE_TEMPLE_BLUE_WARP_Oot)) &&
                    (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_WATER_TEMPLE_BLUE_WARP_Oot))) {
                    play->nextEntrance = Hyrule_ExitFromOot(0x053); // OoT's ENTR_TEMPLE_OF_TIME_ENTRANCE
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    gSaveContext.save.cutsceneIndex = 0xFFF3;
                    play->transitionType = TRANS_TYPE_FADE_BLACK;
                } else {
                    switch (gSaveContext.sceneLayer) {
                        case 8:
                            play->nextEntrance = Hyrule_ExitFromOot(0x0FC); // OoT's ENTR_SACRED_FOREST_MEADOW_SOUTH_EXIT
                            play->transitionTrigger = TRANS_TRIGGER_START;
                            play->transitionType = TRANS_TYPE_FADE_BLACK;
                            break;
                        case 9:
                            play->nextEntrance = Hyrule_ExitFromOot(0x147); // OoT's ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT
                            play->transitionTrigger = TRANS_TRIGGER_START;
                            play->transitionType = TRANS_TYPE_FADE_BLACK;
                            break;
                        case 10:
                            play->nextEntrance = Hyrule_ExitFromOot(0x102); // OoT's ENTR_LAKE_HYLIA_NORTH_EXIT
                            play->transitionTrigger = TRANS_TRIGGER_START;
                            gSaveContext.save.cutsceneIndex = 0xFFF0;
                            play->transitionType = TRANS_TYPE_FADE_WHITE;
                            break;
                    }
                }
                break;
            case 96:
                if (CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_SHADOW_Oot)) {
                    play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    gSaveContext.save.cutsceneIndex = 0xFFF1;
                    play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
                } else {
                    SET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_SPIRIT_MEDALLION_Oot);
                    play->nextEntrance = Hyrule_ExitFromOot(0x610); // OoT's ENTR_DESERT_COLOSSUS_SPIRIT_TEMPLE_BLUE_WARP
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_WHITE;
                    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                }
                break;
            case 97:
                if (CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_SPIRIT_Oot)) {
                    play->nextEntrance = Hyrule_ExitFromOot(0x06B); // OoT's ENTR_CHAMBER_OF_THE_SAGES_0
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    gSaveContext.save.cutsceneIndex = 0xFFF1;
                    play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
                } else {
                    play->nextEntrance = Hyrule_ExitFromOot(0x580); // OoT's ENTR_GRAVEYARD_SHADOW_TEMPLE_BLUE_WARP
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_WHITE;
                    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                }
                break;
            case 98:
                play->nextEntrance = Hyrule_ExitFromOot(0x564); // OoT's ENTR_DEATH_MOUNTAIN_CRATER_FIRE_TEMPLE_BLUE_WARP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 99:
                play->nextEntrance = Hyrule_ExitFromOot(0x608); // OoT's ENTR_SACRED_FOREST_MEADOW_FOREST_TEMPLE_BLUE_WARP
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 100:
                play->nextEntrance = Hyrule_ExitFromOot(0x0EE); // OoT's ENTR_KOKIRI_FOREST_0
                gSaveContext.save.cutsceneIndex = 0xFFF8;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 101:
                play->nextEntrance = Hyrule_ExitFromOot(0x1F5); // OoT's ENTR_DESERT_COLOSSUS_6
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_SANDSTORM_END;
                break;
            case 102:
                play->nextEntrance = Hyrule_ExitFromOot(0x590); // OoT's ENTR_TEMPLE_OF_TIME_6
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 103:
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF3;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 104:
                switch (sTitleCsState) {
                    case 0:
                        play->nextEntrance = Hyrule_ExitFromOot(0x08D); // OoT's ENTR_SPIRIT_TEMPLE_BOSS_ENTRANCE
                        play->transitionTrigger = TRANS_TRIGGER_START;
                        gSaveContext.save.cutsceneIndex = 0xFFF2;
                        play->transitionType = TRANS_TYPE_FADE_BLACK;
                        sTitleCsState++;
                        break;
                    case 1:
                        play->nextEntrance = Hyrule_ExitFromOot(0x147); // OoT's ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT
                        play->transitionTrigger = TRANS_TRIGGER_START;
                        gSaveContext.save.cutsceneIndex = 0xFFF1;
                        play->transitionType = TRANS_TYPE_FADE_BLACK;
                        sTitleCsState++;
                        break;
                    case 2:
                        play->nextEntrance = Hyrule_ExitFromOot(0x0A0); // OoT's ENTR_CUTSCENE_MAP_0
                        play->transitionTrigger = TRANS_TRIGGER_START;
                        gSaveContext.save.cutsceneIndex = 0xFFF6;
                        play->transitionType = TRANS_TYPE_FADE_BLACK;
                        sTitleCsState = 0;
                        break;
                }
                break;
            case 105:
                play->nextEntrance = Hyrule_ExitFromOot(0x0E4); // OoT's ENTR_GRAVEYARD_ENTRANCE
                play->transitionTrigger = TRANS_TRIGGER_START;
                gSaveContext.save.cutsceneIndex = 0xFFF1;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 106:
                play->nextEntrance = Hyrule_ExitFromOot(0x574); // OoT's ENTR_ROYAL_FAMILYS_TOMB_1
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 107:
                play->nextEntrance = Hyrule_ExitFromOot(0x538); // OoT's ENTR_INSIDE_GANONS_CASTLE_2
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_FOREST_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[11] &= ~0x800;
                break;
            case 108:
                play->nextEntrance = Hyrule_ExitFromOot(0x53C); // OoT's ENTR_INSIDE_GANONS_CASTLE_3
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_WATER_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[11] &= ~0x1000;
                break;
            case 109:
                play->nextEntrance = Hyrule_ExitFromOot(0x540); // OoT's ENTR_INSIDE_GANONS_CASTLE_4
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_SHADOW_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[11] &= ~0x2000;
                break;
            case 110:
                play->nextEntrance = Hyrule_ExitFromOot(0x544); // OoT's ENTR_INSIDE_GANONS_CASTLE_5
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_FIRE_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[11] &= ~0x4000;
                break;
            case 111:
                play->nextEntrance = Hyrule_ExitFromOot(0x548); // OoT's ENTR_INSIDE_GANONS_CASTLE_6
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_LIGHT_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[11] &= ~0x8000;
                break;
            case 112:
                play->nextEntrance = Hyrule_ExitFromOot(0x54C); // OoT's ENTR_INSIDE_GANONS_CASTLE_7
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                SET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_SPIRIT_TRIAL_Oot);
                gSaveContext.save.saveInfo.eventChkInfOot[10] &= ~0x2000;
                break;
            case 113:
                if (GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_FOREST_TRIAL_Oot) &&
                    GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_WATER_TRIAL_Oot) &&
                    GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_SHADOW_TRIAL_Oot) &&
                    GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_FIRE_TRIAL_Oot) &&
                    GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_LIGHT_TRIAL_Oot) &&
                    GET_EVENTCHKINF_OOT(EVENTCHKINF_COMPLETED_SPIRIT_TRIAL_Oot)) {
                    // PORT: Ganon's Tower's barrier cutscene is a scene cutscene, which comes with scene cutscenes.
                    gCsCtxOot.segment = NULL;
                    gCsCtxOot.frames = 0;
                    Hyrule_TriggerCutsceneOot(1);
                    gSaveContext.save.cutsceneIndex = 0xFFFF;
                    csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
                } else {
                    gSaveContext.save.cutsceneIndex = 0xFFFF;
                    csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
                }
                break;
            case 114:
                play->nextEntrance = Hyrule_ExitFromOot(0x185); // OoT's ENTR_HYRULE_FIELD_WOODED_EXIT
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 115:
                play->nextEntrance = Hyrule_ExitFromOot(0x594); // OoT's ENTR_HYRULE_FIELD_17
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 116:
                if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_SPIRIT_MEDALLION_Oot)) {
                    play->nextEntrance = Hyrule_ExitFromOot(0x580); // OoT's ENTR_GRAVEYARD_SHADOW_TEMPLE_BLUE_WARP
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_WHITE;
                } else {
                    play->nextEntrance = Hyrule_ExitFromOot(0x610); // OoT's ENTR_DESERT_COLOSSUS_SPIRIT_TEMPLE_BLUE_WARP
                    play->transitionTrigger = TRANS_TRIGGER_START;
                    play->transitionType = TRANS_TYPE_FADE_WHITE;
                }
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 117:
                gSaveContext.gameMode = GAMEMODE_END_CREDITS;
                AudioSfx_MuteBanks(0x6F);
                gLinkAgeOnLoadOot = 0;
                play->nextEntrance = Hyrule_ExitFromOot(0x0CD); // OoT's ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN
                gSaveContext.save.cutsceneIndex = 0xFFF7;
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
            case 118:
                gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = Hyrule_ExitFromOot(0x41F); // OoT's ENTR_GANON_BOSS_0
                func_80169EFC(play);
                gSaveContext.respawnFlag = -2;
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
                break;
            case 119:
                gSaveContext.save.time = 0x8000;
                gSaveContext.skyboxTime = 0x8000;
                play->nextEntrance = Hyrule_ExitFromOot(0x5F0); // OoT's ENTR_CASTLE_COURTYARD_ZELDA_1
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
                break;
        }
    }
}

// Command 0x2D: Transition Effects
static void Cutscene_Command_TransitionFX(PlayState* play, CutsceneContextOot* csCtx, CsCmdBaseOot* cmd) {
    f32 temp;

    if ((csCtx->frames >= cmd->startFrame) && (csCtx->frames <= cmd->endFrame)) {
        play->envCtx.fillScreen = true;
        temp = Environment_LerpWeight(cmd->endFrame, cmd->startFrame, csCtx->frames);

        switch (cmd->base) {
            case 1:
            case 5:
                play->envCtx.screenFillColor[0] = 160;
                play->envCtx.screenFillColor[1] = 160;
                play->envCtx.screenFillColor[2] = 160;
                if (cmd->base == 1) {
                    play->envCtx.screenFillColor[3] = 255.0f * temp;
                    if ((temp == 0.0f) && (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x06B /* ENTR_CHAMBER_OF_THE_SAGES_0 */))) {
                        Audio_PlaySfx(NA_SE_SY_WHITE_OUT_S);
                    } else if ((temp == 0.0f) &&
                               ((gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x053 /* ENTR_TEMPLE_OF_TIME_ENTRANCE */)) ||
                                (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x138 /* ENTR_CASTLE_GROUNDS_SOUTH_EXIT */)) ||
                                (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x371 /* ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_FARORES_ZF */)))) {
                        Audio_PlaySfx(NA_SE_EV_S_STONE_FLASH);
                    } else if ((temp == 0.0f) && (play->sceneId == SCENE_INSIDE_GANONS_CASTLE_Oot)) {
                        Audio_PlaySfx_2(NA_SE_EV_S_STONE_FLASH);
                    }
                } else {
                    play->envCtx.screenFillColor[3] = (1.0f - temp) * 255.0f;
                }
                break;
            case 2:
            case 6:
                play->envCtx.screenFillColor[0] = 0;
                play->envCtx.screenFillColor[1] = 0;
                play->envCtx.screenFillColor[2] = 255;
                if (cmd->base == 2) {
                    play->envCtx.screenFillColor[3] = 255.0f * temp;
                } else {
                    play->envCtx.screenFillColor[3] = (1.0f - temp) * 255.0f;
                }
                break;
            case 3:
            case 7:
                play->envCtx.screenFillColor[0] = 255;
                play->envCtx.screenFillColor[1] = 0;
                play->envCtx.screenFillColor[2] = 0;
                if (cmd->base == 3) {
                    play->envCtx.screenFillColor[3] = (1.0f - temp) * 255.0f;
                } else {
                    play->envCtx.screenFillColor[3] = 255.0f * temp;
                }
                break;
            case 4:
            case 8:
                play->envCtx.screenFillColor[0] = 0;
                play->envCtx.screenFillColor[1] = 255;
                play->envCtx.screenFillColor[2] = 0;
                if (cmd->base == 4) {
                    play->envCtx.screenFillColor[3] = (1.0f - temp) * 255.0f;
                } else {
                    play->envCtx.screenFillColor[3] = 255.0f * temp;
                }
                break;
            case 9:
                gSaveContext.cutsceneTransitionControl = 1;
                break;
            case 10:
            case 11:
                play->envCtx.screenFillColor[0] = 0;
                play->envCtx.screenFillColor[1] = 0;
                play->envCtx.screenFillColor[2] = 0;
                if (cmd->base == 10) {
                    play->envCtx.screenFillColor[3] = (1.0f - temp) * 255.0f;
                } else {
                    play->envCtx.screenFillColor[3] = 255.0f * temp;
                }
                break;
            case 12:
                gSaveContext.cutsceneTransitionControl = 255.0f - (155.0f * temp);
                break;
            case 13:
                play->envCtx.screenFillColor[0] = 0;
                play->envCtx.screenFillColor[1] = 0;
                play->envCtx.screenFillColor[2] = 0;
                play->envCtx.screenFillColor[3] = 255.0f - ((1.0f - temp) * 155.0f);
                break;
        }
    }
}

// Command 0x1 & 0x5: Camera Positions
size_t Cutscene_Command_CameraPositions(PlayState* play, CutsceneContextOot* csCtx, u8* cmd, u8 relativeToLink) {
    s32 shouldContinue = true;
    CsCmdBaseOot* cmdBase = (CsCmdBaseOot*)cmd;
    size_t size;

    cmd += sizeof(CutsceneData) * 2;
    size = sizeof(CutsceneData) * 2;

    if ((cmdBase->startFrame < csCtx->frames) && (csCtx->frames < cmdBase->endFrame) &&
        ((csCtx->unk_18 < cmdBase->startFrame) || (csCtx->unk_18 >= 0xF000))) {
        csCtx->unk_1B = 1;
        csCtx->cameraPosition = (CutsceneCameraPointOot*)cmd;
        if (csCtx->unk_1A != 0) {
            csCtx->unk_18 = cmdBase->startFrame;
            if (D_8015FCC8 != 0) {
                // OoT's CAM_SET_CS_0 is MM's FREE0 for scripts, which leaves the interface to the cutscene.
                Play_ChangeCameraSetting(play, csCtx->unk_14, CAM_SET_FREE0);
                Play_ChangeCameraStatus(play, D_8015FCC6, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, csCtx->unk_14, CAM_STATUS_ACTIVE);
                CsCamera_Start(play, csCtx->unk_14, csCtx->cameraFocus, csCtx->cameraPosition, relativeToLink);
            }
        }
    }

    while (shouldContinue) {
        if (((CutsceneCameraPointOot*)cmd)->continueFlag == CS_CMD_STOP_Oot) {
            shouldContinue = false;
        }
        cmd += sizeof(CutsceneData) * 4;
        size += sizeof(CutsceneData) * 4;
    }

    return size;
}

// Command 0x2 & 0x6: Camera Focus Points
size_t Cutscene_Command_CameraFocus(PlayState* play, CutsceneContextOot* csCtx, u8* cmd, u8 relativeToLink) {
    s32 shouldContinue = true;
    CsCmdBaseOot* cmdBase = (CsCmdBaseOot*)cmd;
    size_t size;

    cmd += sizeof(CutsceneData) * 2;
    size = sizeof(CutsceneData) * 2;

    if ((cmdBase->startFrame < csCtx->frames) && (csCtx->frames < cmdBase->endFrame) &&
        ((D_8015FCC0 < cmdBase->startFrame) || (D_8015FCC0 >= 0xF000))) {
        csCtx->unk_1A = 1;
        csCtx->cameraFocus = (CutsceneCameraPointOot*)cmd;
        if (csCtx->unk_1B != 0) {
            D_8015FCC0 = cmdBase->startFrame;
            if (D_8015FCC8 != 0) {
                // OoT's CAM_SET_CS_0 is MM's FREE0 for scripts, which leaves the interface to the cutscene.
                Play_ChangeCameraSetting(play, csCtx->unk_14, CAM_SET_FREE0);
                Play_ChangeCameraStatus(play, D_8015FCC6, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, csCtx->unk_14, CAM_STATUS_ACTIVE);
                CsCamera_Start(play, csCtx->unk_14, csCtx->cameraFocus, csCtx->cameraPosition, relativeToLink);
            }
        }
    }

    while (shouldContinue) {
        if (((CutsceneCameraPointOot*)cmd)->continueFlag == CS_CMD_STOP_Oot) {
            shouldContinue = false;
        }
        cmd += sizeof(CutsceneData) * 4;
        size += sizeof(CutsceneData) * 4;
    }

    return size;
}

// Command 0x7: ? (Related to camera positons)
size_t Cutscene_Command_07(PlayState* play, CutsceneContextOot* csCtx, u8* cmd, u8 unused) {
    CsCmdBaseOot* cmdBase = (CsCmdBaseOot*)cmd;
    size_t size;
    Vec3f sp3C;
    Vec3f sp30;
    Camera* sp2C;
    f32 sp28;

    cmd += sizeof(CutsceneData) * 2;
    size = sizeof(CutsceneData) * 2;

    if ((cmdBase->startFrame < csCtx->frames) && (csCtx->frames < cmdBase->endFrame) &&
        ((D_8015FCC2 < cmdBase->startFrame) || (D_8015FCC2 >= 0xF000))) {
        csCtx->unk_1B = 1;
        csCtx->cameraPosition = (CutsceneCameraPointOot*)cmd;
        if (csCtx->unk_1A != 0) {
            D_8015FCC2 = cmdBase->startFrame;
            if (D_8015FCC8 != 0) {
                sp2C = Play_GetCamera(play, csCtx->unk_14);
                sp2C->focalActor = NULL;
                Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, csCtx->unk_14, CAM_STATUS_ACTIVE);
                Play_ChangeCameraSetting(play, csCtx->unk_14, CAM_SET_FREE0);
                sp28 = csCtx->cameraFocus->cameraRoll * 1.40625f;
                Play_SetCameraRoll(play, csCtx->unk_14, DEG_TO_BINANG(sp28));
                sp3C.x = csCtx->cameraFocus->pos.x;
                sp3C.y = csCtx->cameraFocus->pos.y;
                sp3C.z = csCtx->cameraFocus->pos.z;
                sp30.x = csCtx->cameraPosition->pos.x;
                sp30.y = csCtx->cameraPosition->pos.y;
                sp30.z = csCtx->cameraPosition->pos.z;
                Play_SetCameraAtEye(play, csCtx->unk_14, &sp3C, &sp30);
                Play_SetCameraFov(play, csCtx->unk_14, csCtx->cameraPosition->viewAngle);
            }
        }
    }

    size += sizeof(CutsceneData) * 4;

    return size;
}

// Command 0x8: ? (Related to camera focus points)
size_t Cutscene_Command_08(PlayState* play, CutsceneContextOot* csCtx, u8* cmd, u8 unused) {
    CsCmdBaseOot* cmdBase = (CsCmdBaseOot*)cmd;
    size_t size;
    Vec3f sp3C;
    Vec3f sp30;
    Camera* sp2C;
    f32 sp28;

    cmd += sizeof(CutsceneData) * 2;
    size = sizeof(CutsceneData) * 2;

    if ((cmdBase->startFrame < csCtx->frames) && (csCtx->frames < cmdBase->endFrame) &&
        ((D_8015FCC4 < cmdBase->startFrame) || (D_8015FCC4 >= 0xF000))) {
        csCtx->unk_1A = 1;
        csCtx->cameraFocus = (CutsceneCameraPointOot*)cmd;
        if (csCtx->unk_1B != 0) {
            D_8015FCC4 = cmdBase->startFrame;
            if (D_8015FCC8 != 0) {
                sp2C = Play_GetCamera(play, csCtx->unk_14);
                sp2C->focalActor = NULL;
                Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, csCtx->unk_14, CAM_STATUS_ACTIVE);
                Play_ChangeCameraSetting(play, csCtx->unk_14, CAM_SET_FREE0);
                sp3C.x = csCtx->cameraFocus->pos.x;
                sp3C.y = csCtx->cameraFocus->pos.y;
                sp3C.z = csCtx->cameraFocus->pos.z;
                sp30.x = csCtx->cameraPosition->pos.x;
                sp30.y = csCtx->cameraPosition->pos.y;
                sp30.z = csCtx->cameraPosition->pos.z;
                Play_SetCameraAtEye(play, csCtx->unk_14, &sp3C, &sp30);
                Play_SetCameraFov(play, csCtx->unk_14, csCtx->cameraPosition->viewAngle);
            }
        }
    }

    size += sizeof(CutsceneData) * 4;

    return size;
}

// Command 0x13: Textbox
static void Cutscene_Command_Textbox(PlayState* play, CutsceneContextOot* csCtx, CsCmdTextboxOot* cmd) {
    u8 dialogState;
    s16 originalCsFrames;

    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
        if (cmd->type != 2) {
            if (D_8011E1C0 != cmd->base) {
                D_8011E1C0 = cmd->base;
                if ((cmd->type == 3) && CHECK_QUEST_ITEM_OOT(QUEST_ZORA_SAPPHIRE_Oot)) {
                    Message_StartTextbox(play, TEXT_OOT(cmd->textId1), NULL);
                } else if ((cmd->type == 4) && CHECK_QUEST_ITEM_OOT(QUEST_GORON_RUBY_Oot)) {
                    Message_StartTextbox(play, TEXT_OOT(cmd->textId1), NULL);
                } else {
                    Message_StartTextbox(play, TEXT_OOT(cmd->base), NULL);
                }
                return;
            }
        } else {
            if (D_8011E1C4 != cmd->base) {
                D_8011E1C4 = cmd->base;
                Message_StartOcarinaOot(play, Hyrule_OcarinaActionFromOot(cmd->base));
                return;
            }
        }

        if (csCtx->frames >= cmd->endFrame) {
            originalCsFrames = csCtx->frames;
            dialogState = Message_GetState(&play->msgCtx);

            if ((dialogState != TEXT_STATE_CLOSING) && (dialogState != TEXT_STATE_NONE) &&
                (dialogState != TEXT_STATE_SONG_DEMO_DONE) && (dialogState != TEXT_STATE_8)) {
                csCtx->frames--;

                if ((dialogState == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
                    if (play->msgCtx.choiceIndex == 0) {
                        if (cmd->textId1 != 0xFFFF) {
                            // LUSLOG_INFO("Cutscene_Command_Textbox D: base:0x%x textId1:0x%x textId2:0x%x", cmd->base,
                            // cmd->textId1, cmd->textId2);
                            Message_ContinueTextbox(play, TEXT_OOT(cmd->textId1));
                        } else {
                            csCtx->frames++;
                        }
                    } else {
                        if (cmd->textId2 != 0xFFFF) {
                            // LUSLOG_INFO("Cutscene_Command_Textbox E: base:0x%x textId1:0x%x textId2:0x%x", cmd->base,
                            // cmd->textId1, cmd->textId2);
                            Message_ContinueTextbox(play, TEXT_OOT(cmd->textId2));
                        } else {
                            csCtx->frames++;
                        }
                    }
                }

                if (dialogState == TEXT_STATE_9) {
                    if (cmd->textId1 != 0xFFFF) {
                        // LUSLOG_INFO("Cutscene_Command_Textbox F: base:0x%x textId1:0x%x textId2:0x%x", cmd->base,
                        // cmd->textId1, cmd->textId2);
                        Message_ContinueTextbox(play, TEXT_OOT(cmd->textId1));
                    } else {
                        csCtx->frames++;
                    }
                }

                if (dialogState == TEXT_STATE_EVENT) {
                    if (Message_ShouldAdvance(play)) {
                        Message_StartOcarinaOot(play, Hyrule_OcarinaActionFromOot(cmd->base));
                    }
                }
            }

            if (csCtx->frames == originalCsFrames) {
                Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
                D_8011E1C0 = 0;
                D_8011E1C4 = 0;
            }
        }
    }
}

static void Cutscene_ProcessCommands(PlayState* play, CutsceneContextOot* csCtx, u8* cutscenePtr) {
    s16 i;
    s32 totalEntries;
    s32 cmdType;
    s32 cmdEntries;
    CsCmdBaseOot* cmd;
    s32 cutsceneEndFrame;
    s16 j;

    if (ResourceMgr_OTRSigCheck(cutscenePtr))
        cutscenePtr = ResourceMgr_LoadCSByName(cutscenePtr);

    memcpy(&totalEntries, cutscenePtr, sizeof(CutsceneData));
    cutscenePtr += sizeof(CutsceneData);
    memcpy(&cutsceneEndFrame, cutscenePtr, sizeof(CutsceneData));
    cutscenePtr += sizeof(CutsceneData);

    if ((cutsceneEndFrame < csCtx->frames) && (csCtx->state != CS_STATE_UNSKIPPABLE_EXEC_Oot)) {
        csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
        return;
    }

    if (false &&
        CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DRIGHT)) {
        csCtx->state = CS_STATE_UNSKIPPABLE_INIT_Oot;
        return;
    }

    for (i = 0; i < totalEntries; i++) {
        memcpy(&cmdType, cutscenePtr, sizeof(CutsceneData));
        cutscenePtr += sizeof(CutsceneData);

        if (cmdType == -1) {
            return;
        }

        // printf("CmdType: %04X\n", cmdType);

        switch (cmdType) {
            case CS_CMD_MISC_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    func_80064824(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_LIGHTING_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    Cutscene_Command_SetLighting(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_PLAYBGM_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    Cutscene_Command_PlayBGM(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_STOPBGM_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    Cutscene_Command_StopBGM(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_FADEBGM_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    Cutscene_Command_FadeBGM(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_09_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    Cutscene_Command_09(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 3;
                }
                break;
            case CS_CMD_SETTIME_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    func_80065134(play, csCtx, (void*)cutscenePtr);
                    cutscenePtr += sizeof(CutsceneData) * 3;
                }
                break;
            case CS_CMD_SET_PLAYER_ACTION_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->linkAction = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_1_Oot:
            case 17:
            case 18:
            case 23:
            case 34:
            case 39:
            case 46:
            case 76:
            case 85:
            case 93:
            case 105:
            case 107:
            case 110:
            case 119:
            case 123:
            case 138:
            case 139:
            case 144:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[0] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_2_Oot:
            case 16:
            case 24:
            case 35:
            case 40:
            case 48:
            case 64:
            case 68:
            case 70:
            case 78:
            case 80:
            case 94:
            case 116:
            case 118:
            case 120:
            case 125:
            case 131:
            case 141:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[1] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_3_Oot:
            case 36:
            case 41:
            case 50:
            case 67:
            case 69:
            case 72:
            case 74:
            case 81:
            case 106:
            case 117:
            case 121:
            case 126:
            case 132:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[2] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_4_Oot:
            case 37:
            case 42:
            case 51:
            case 53:
            case 63:
            case 65:
            case 66:
            case 75:
            case 82:
            case 108:
            case 127:
            case 133:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[3] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_5_Oot:
            case 38:
            case 43:
            case 47:
            case 54:
            case 79:
            case 83:
            case 128:
            case 135:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[4] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_6_Oot:
            case 55:
            case 77:
            case 84:
            case 90:
            case 129:
            case 136:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[5] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_7_Oot:
            case 52:
            case 57:
            case 58:
            case 88:
            case 115:
            case 130:
            case 137:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[6] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_8_Oot:
            case 60:
            case 89:
            case 111:
            case 114:
            case 134:
            case 142:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[7] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_9_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[8] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_SET_ACTOR_ACTION_10_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if ((cmd->startFrame < csCtx->frames) && (csCtx->frames <= cmd->endFrame)) {
                        csCtx->npcActions[9] = (void*)cutscenePtr;
                    }
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
            case CS_CMD_CAM_EYE_Oot:
                cutscenePtr += Cutscene_Command_CameraPositions(play, csCtx, (void*)cutscenePtr, 0);
                break;
            case CS_CMD_CAM_EYE_REL_TO_PLAYER_Oot:
                cutscenePtr += Cutscene_Command_CameraPositions(play, csCtx, (void*)cutscenePtr, 1);
                break;
            case CS_CMD_CAM_AT_Oot:
                cutscenePtr += Cutscene_Command_CameraFocus(play, csCtx, (void*)cutscenePtr, 0);
                break;
            case CS_CMD_CAM_AT_REL_TO_PLAYER_Oot:
                cutscenePtr += Cutscene_Command_CameraFocus(play, csCtx, (void*)cutscenePtr, 1);
                break;
            case CS_CMD_07_Oot:
                cutscenePtr += Cutscene_Command_07(play, csCtx, (void*)cutscenePtr, 0);
                break;
            case CS_CMD_08_Oot:
                cutscenePtr += Cutscene_Command_08(play, csCtx, (void*)cutscenePtr, 0);
                break;
            case CS_CMD_TERMINATOR_Oot:
                cutscenePtr += 4;
                Cutscene_Command_Terminator(play, csCtx, (void*)cutscenePtr);
                cutscenePtr += 8;
                break;
            case CS_CMD_TEXTBOX_Oot:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += 4;
                for (j = 0; j < cmdEntries; j++) {
                    cmd = (CsCmdBaseOot*)cutscenePtr;
                    if (cmd->base != 0xFFFF) {
                        Cutscene_Command_Textbox(play, csCtx, (void*)cutscenePtr);
                    }
                    cutscenePtr += sizeof(CutsceneData) * 3;
                }
                break;
            case CS_CMD_SCENE_TRANS_FX_Oot:
                cutscenePtr += sizeof(CutsceneData);
                Cutscene_Command_TransitionFX(play, csCtx, (void*)cutscenePtr);
                cutscenePtr += sizeof(CutsceneData) * 2;
                break;
            default:
                memcpy(&cmdEntries, cutscenePtr, sizeof(CutsceneData));
                cutscenePtr += sizeof(CutsceneData);
                for (j = 0; j < cmdEntries; j++) {
                    cutscenePtr += sizeof(CutsceneData) * 12;
                }
                break;
        }
    }
}

static void func_80068C3C(PlayState* play, CutsceneContextOot* csCtx) {
    Gfx* displayList;
    Gfx* prevDisplayList;

    if (gSaveContext.save.cutsceneIndex >= 0xFFF0) {


        csCtx->frames++;
        Cutscene_ProcessCommands(play, csCtx, gCsCtxOot.segment);
        CsCamera_Update(play);
    }
}

static void func_80068D84(PlayState* play, CutsceneContextOot* csCtx) {
    if (func_8006472C(play, csCtx, 0.0f)) {
        Audio_SetCutsceneFlag(0);
        csCtx->state = CS_STATE_IDLE_Oot;
    }
}

static void func_80068DC0(PlayState* play, CutsceneContextOot* csCtx) {
    s16 i;

    if (func_8006472C(play, csCtx, 0.0f)) {
        csCtx->linkAction = NULL;

        for (i = 0; i < 10; i++) {
            csCtx->npcActions[i] = NULL;
        }
 // "Right here, huh"
        gSaveContext.save.cutsceneIndex = 0;
        gSaveContext.gameMode = GAMEMODE_NORMAL;

        if (D_8015FCC8 != 0) {
            // OoT's ENTR_HYRULE_FIELD_11, 12, 13 and 15: Epona's jumps over Hyrule Field's fences.
            if ((gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x28A)) ||
                (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x28E)) ||
                (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x292)) ||
                (gSaveContext.save.entrance == Hyrule_EntranceFromOot(0x476))) {
                Play_CopyCamera(play, D_8015FCC6, csCtx->unk_14);
            }

            Play_ChangeCameraStatus(play, D_8015FCC6, CAM_STATUS_ACTIVE);
            Play_ClearCamera(play, csCtx->unk_14);
            Camera_SetFinishedFlag(play->cameraPtrs[D_8015FCC6]);
        }

        Audio_SetCutsceneFlag(0);
        csCtx->state = CS_STATE_IDLE_Oot;
    }
}

static void func_80068ECC(PlayState* play, CutsceneContextOot* csCtx) {
    u8 i;

    if ((sCutsceneTriggerOot != 0) && (csCtx->state == CS_STATE_IDLE_Oot) && !Player_InCsMode(play)) {
        gSaveContext.save.cutsceneIndex = 0xFFFD;
    }

    if ((gSaveContext.save.cutsceneIndex >= 0xFFF0) && (csCtx->state == CS_STATE_IDLE_Oot)) {
        Flags_UnsetEnvOot(play, 0);

        D_8011E1C0 = 0;
        D_8011E1C4 = 0;
        csCtx->unk_12 = 0;
        csCtx->linkAction = NULL;

        for (i = 0; i < 10; i++) {
            csCtx->npcActions[i] = NULL;
        }

        csCtx->state++;

        if (csCtx->state == CS_STATE_SKIPPABLE_INIT_Oot) {
            Audio_SetCutsceneFlag(1);

            csCtx->frames = 0xFFFF;
            csCtx->unk_18 = 0xFFFF;
            D_8015FCC0 = 0xFFFF;
            D_8015FCC2 = 0xFFFF;
            D_8015FCC4 = 0xFFFF;
            csCtx->unk_1A = 0;
            csCtx->unk_1B = 0;
            D_8015FCC6 = play->activeCamId;

            if (D_8015FCC8 != 0) {
                csCtx->unk_14 = Play_CreateSubCamera(play);
            }

            if (sCutsceneTriggerOot == 0) {
                Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
                ShrinkWindow_Letterbox_SetSizeTarget(0x20);
                ShrinkWindow_Letterbox_SetSize(0x20);
                csCtx->state++;
            }

            func_80068C3C(play, csCtx);
        }

        Hyrule_TriggerCutsceneOot(0);
    }
}

static void func_80069048(PlayState* play) {
    s16 i;

    D_8015FCCC = 0;
    for (i = 0; i < 20; i++) {
        ; // Empty Loop
    }
    D_8015FCE4 = 0;
}

static void func_8006907C(PlayState* play) {
    if (D_8015FCCC != 0) {
        D_8015FCCC = 0;
    }
}


// OoT's play->envFlags.
static u16 sEnvFlagsOot[20];

s32 Flags_GetEnvOot(PlayState* play, s16 flag) {
    return sEnvFlagsOot[flag >> 4] & (1 << (flag & 0xF));
}

void Flags_SetEnvOot(PlayState* play, s16 flag) {
    sEnvFlagsOot[flag >> 4] |= (1 << (flag & 0xF));
}

void Flags_UnsetEnvOot(PlayState* play, s16 flag) {
    sEnvFlagsOot[flag >> 4] &= ~(1 << (flag & 0xF));
}

void Hyrule_InitCutsceneOot(PlayState* play) {
    memset(sEnvFlagsOot, 0, sizeof(sEnvFlagsOot));
    memset(&gCsCtxOot, 0, sizeof(gCsCtxOot));
    gCsCtxOot.state = CS_STATE_IDLE_Oot;
    sCutsceneTriggerOot = 0;
    sCsCamera.active = false;
    // OoT's Environment_Init: the cutscene camera is on.
    D_8015FCC8 = 1;
}

void Hyrule_TriggerCutsceneOot(u8 trigger) {
    sCutsceneTriggerOot = trigger;
}

void Hyrule_StartCutsceneOot(PlayState* play, void* script) {
    gCsCtxOot.segment = script;
    sCutsceneTriggerOot = 1;
}

void Hyrule_StartSceneCutsceneOot(PlayState* play, void* script) {
    // OoT keeps the cutscene index at 0xFFF0 and up through the load; MM clears it once the layer is chosen.
    gCsCtxOot.segment = script;
    gSaveContext.save.cutsceneIndex = 0xFFF0;
}

s32 Hyrule_IsCutsceneOotPlaying(void) {
    return (gCsCtxOot.state != CS_STATE_IDLE_Oot) || (sCutsceneTriggerOot != 0) ||
           ((gCsCtxOot.segment != NULL) && (gSaveContext.save.cutsceneIndex >= 0xFFF0));
}

void Hyrule_UpdateCutsceneOot(PlayState* play) {
    // OoT's Cutscene_UpdateManual and Cutscene_UpdateScripted. The cutscene index means the same in both games (0xFFFD
    // while a script plays, 0xFFF0 and up for the next scene's cutscene layer) and is shared.
    func_80064558(play, &gCsCtxOot);
    func_800645A0(play, &gCsCtxOot);

    // MM's engine (Player, the interface) sees a cutscene while OoT's plays.
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        play->csCtx.state = CS_STATE_RUN;
        play->csCtx.curFrame = gCsCtxOot.frames;
    } else if (play->csCtx.state == CS_STATE_RUN) {
        play->csCtx.state = CS_STATE_IDLE;
    }
}

void Hyrule_ApplyAgeOnLoadOot(void) {
    if (gLinkAgeOnLoadOot >= 0) {
        Hyrule_SetAge(gLinkAgeOnLoadOot == 0);
        gLinkAgeOnLoadOot = -1;
    }
}

s32 Hyrule_CsSplineStepOot(Vec3f* pos, f32* roll, f32* fov, CutsceneCameraPointOot* point, s16* keyFrame,
                           f32* curFrame) {
    return CsCamera_Step(pos, roll, fov, point, keyFrame, curFrame);
}
