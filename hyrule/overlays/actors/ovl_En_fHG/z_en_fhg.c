/*
 * File: z_en_fhg.c
 * Overlay: ovl_En_fHG
 * Description: Phantom Ganon's Horse
 */

#include "z_en_fhg.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/textures/boss_title_cards/object_fhg.h"
#include "assets/oot/objects/object_fhg/object_fhg.h"
#include "../ovl_Door_Shutter/z_door_shutter.h"
#include "../ovl_Boss_Ganondrof/z_boss_ganondrof.h"
#include "../ovl_En_Fhg_Fire/z_en_fhg_fire.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

typedef struct {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ s16 yRot;
} EnfHGPainting; // size = 0x10;

typedef enum {
    /*  0 */ INTRO_WAIT,
    /*  1 */ INTRO_START,
    /*  2 */ INTRO_FENCE,
    /*  3 */ INTRO_BACK,
    /*  4 */ INTRO_REVEAL,
    /*  5 */ INTRO_CUT,
    /*  6 */ INTRO_LAUGH,
    /*  7 */ INTRO_TITLE,
    /*  8 */ INTRO_RETREAT,
    /*  9 */ INTRO_FINISH,
    /* 15 */ INTRO_READY = 15
} EnfHGIntroState;

static void EnfHG_Init(Actor* thisx, PlayState* play);
static void EnfHG_Destroy(Actor* thisx, PlayState* play);
static void EnfHG_Update(Actor* thisx, PlayState* play);
static void EnfHG_Draw(Actor* thisx, PlayState* play);

static void EnfHG_SetupIntro(EnfHG* this, PlayState* play);
static void EnfHG_Intro(EnfHG* this, PlayState* play);
static void EnfHG_SetupApproach(EnfHG* this, PlayState* play, s16 paintingIndex);
static void EnfHG_Approach(EnfHG* this, PlayState* play);
static void EnfHG_Attack(EnfHG* this, PlayState* play);
static void EnfHG_Damage(EnfHG* this, PlayState* play);
static void EnfHG_Retreat(EnfHG* this, PlayState* play);
static void EnfHG_Done(EnfHG* this, PlayState* play);

ActorProfile En_fHG_Oot_Profile = {
    /**/ ACTOR_EN_FHG_Oot,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_FHG_Oot,
    /**/ sizeof(EnfHG),
    /**/ EnfHG_Init,
    /**/ EnfHG_Destroy,
    /**/ EnfHG_Update,
    /**/ EnfHG_Draw,
};

static EnfHGPainting sPaintings[] = {
    { { 0.0f, 60.0f, -315.0f }, 0x0000 },   { { -260.0f, 60.0f, -145.0f }, 0x2AAA },
    { { -260.0f, 60.0f, 165.0f }, 0x5554 }, { { 0.0f, 60.0f, 315.0f }, 0x7FFE },
    { { 260.0f, 60.0f, 155.0f }, 0xAAA8 },  { { 260.0f, 60.0f, -155.0f }, 0xD552 },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x1A
    ICHAIN_F32(cullingVolumeScale, 1200, ICHAIN_STOP),
};

static void EnfHG_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnfHG* this = (EnfHG*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Flags_SetSwitch(play, 0x14);
    Actor_SetScale(&this->actor, 0.011499999f);
    this->actor.gravity = -3.5f;
    ActorShape_Init(&this->actor.shape, -2600.0f, NULL, 20.0f);
    this->actor.speed = 0.0f;
    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += 70.0f;
    Skin_Init(play, &this->skin, &gPhantomHorseSkel, &gPhantomHorseRunningAnim);

    if (this->actor.params >= GND_FAKE_BOSS) {
        EnfHG_SetupApproach(this, play, this->actor.params - GND_FAKE_BOSS);
    } else {
        EnfHG_SetupIntro(this, play);
    }
}

static void EnfHG_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    EnfHG* this = (EnfHG*)thisx;

    Skin_Free(play, &this->skin);
}

static void EnfHG_SetupIntro(EnfHG* this, PlayState* play) {
    Animation_PlayLoop(&this->skin.skelAnime, &gPhantomHorseIdleAnim);
    this->actionFunc = EnfHG_Intro;
    this->actor.world.pos.x = GND_BOSSROOM_CENTER_X;
    this->actor.world.pos.y = GND_BOSSROOM_CENTER_Y - 267.0f;
    this->actor.world.pos.z = GND_BOSSROOM_CENTER_Z;
}

static void EnfHG_Intro(EnfHG* this, PlayState* play) {
    static Vec3f audioVec = { 0.0f, 0.0f, 50.0f };
    s32 pad64;
    Player* player = GET_PLAYER(play);
    BossGanondrof* bossGnd = (BossGanondrof*)this->actor.parent;
    s32 pad58;
    s32 pad54;

    if (this->cutsceneState != INTRO_FINISH) {
        SkelAnime_Update(&this->skin.skelAnime);
    }
    switch (this->cutsceneState) {
        case INTRO_WAIT:
            if ((fabsf(player->actor.world.pos.x - (GND_BOSSROOM_CENTER_X + 0.0f)) < 150.0f) &&
                (fabsf(player->actor.world.pos.z - (GND_BOSSROOM_CENTER_Z + 0.0f)) < 150.0f)) {
                this->cutsceneState = INTRO_READY;
            }
            break;
        case INTRO_READY:
            if ((fabsf(player->actor.world.pos.x - (GND_BOSSROOM_CENTER_X + 0.0f)) < 100.0f) &&
                (fabsf(player->actor.world.pos.z - (GND_BOSSROOM_CENTER_Z + 315.0f)) < 100.0f)) {
                this->cutsceneState = INTRO_START;
                if (GET_EVENTCHKINF_OOT(EVENTCHKINF_BEGAN_PHANTOM_GANON_BATTLE_Oot)) {
                    this->timers[0] = 57;
                }
            }
            break;
        case INTRO_START:
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_BEGAN_PHANTOM_GANON_BATTLE_Oot)) {
                if (this->timers[0] == 55) {
                    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_SHUTTER_Oot,
                                       GND_BOSSROOM_CENTER_X + 0.0f, GND_BOSSROOM_CENTER_Y - 97.0f,
                                       GND_BOSSROOM_CENTER_Z + 308.0f, 0, 0, 0, (SHUTTER_PG_BARS << 6));
                }
                if (this->timers[0] == 51) {
                    Actor_PlaySfx(this->actor.child, NA_SE_EV_SPEAR_FENCE);
                    AudioSeq_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS);
                }
                if (this->timers[0] == 0) {
                    EnfHG_SetupApproach(this, play, Rand_ZeroOne() * 5.99f);
                    this->bossGndSignal = FHG_START_FIGHT;
                }
                break;
            }
            Cutscene_StartManualOot(play);
            Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
            this->cutsceneCamera = Play_CreateSubCamera(play);
            Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
            Play_ChangeCameraStatus(play, this->cutsceneCamera, CAM_STATUS_ACTIVE);
            this->cutsceneState = INTRO_FENCE;
            this->timers[0] = 60;
            this->actor.world.pos.y = GND_BOSSROOM_CENTER_Y - 7.0f;
            AudioSeq_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0x100FF);
            SET_EVENTCHKINF_OOT(EVENTCHKINF_BEGAN_PHANTOM_GANON_BATTLE_Oot);
            Flags_SetSwitch(play, 0x23);
        case INTRO_FENCE:
            player->actor.world.pos.x = GND_BOSSROOM_CENTER_X + 0.0f;
            player->actor.world.pos.y = GND_BOSSROOM_CENTER_Y + 7.0f;
            player->actor.world.pos.z = GND_BOSSROOM_CENTER_Z + 155.0f;
            player->actor.world.rot.y = player->actor.shape.rot.y = 0;
            player->actor.speed = 0.0f;
            this->cameraEye.x = GND_BOSSROOM_CENTER_X + 0.0f;
            this->cameraEye.y = GND_BOSSROOM_CENTER_Y + 37.0f;
            this->cameraEye.z = GND_BOSSROOM_CENTER_Z + 170.0f;
            this->cameraAt.x = GND_BOSSROOM_CENTER_X + 0.0f;
            this->cameraAt.y = GND_BOSSROOM_CENTER_Y + 47.0f;
            this->cameraAt.z = GND_BOSSROOM_CENTER_Z + 315.0f;
            if (this->timers[0] == 25) {
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_SHUTTER_Oot,
                                   GND_BOSSROOM_CENTER_X + 0.0f, GND_BOSSROOM_CENTER_Y - 97.0f,
                                   GND_BOSSROOM_CENTER_Z + 308.0f, 0, 0, 0, (SHUTTER_PG_BARS << 6));
            }
            if (this->timers[0] == 21) {
                Actor_PlaySfx(this->actor.child, NA_SE_EV_SPEAR_FENCE);
            }
            if (this->timers[0] == 0) {
                this->cutsceneState = INTRO_BACK;
                this->timers[0] = 80;
            }
            break;
        case INTRO_BACK:
            if (this->timers[0] == 25) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_EXPLSION_LONG);
            }
            if (this->timers[0] == 20) {
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 9);
            }
            if (this->timers[0] == 1) {
                AudioSeq_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | SEQ_OOT(0x23));
            }
            Math_ApproachF(&this->cameraEye.x, GND_BOSSROOM_CENTER_X + 40.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraEye.y, GND_BOSSROOM_CENTER_Y + 37.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraEye.z, GND_BOSSROOM_CENTER_Z + 80.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.x, GND_BOSSROOM_CENTER_X - 100.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.y, GND_BOSSROOM_CENTER_Y + 47.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.z, GND_BOSSROOM_CENTER_Z + 335.0f, 0.05f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraSpeedMod, 1.0f, 1.0f, 0.01f);
            if (this->timers[0] == 0) {
                this->cutsceneState = INTRO_REVEAL;
                this->timers[0] = 50;
                this->cameraSpeedMod = 0.0f;
            }
            break;
        case INTRO_REVEAL:
            Math_ApproachF(&this->cameraEye.x, GND_BOSSROOM_CENTER_X + 70.0f, 0.1f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraEye.y, GND_BOSSROOM_CENTER_Y + 7.0f, 0.1f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraEye.z, GND_BOSSROOM_CENTER_Z + 200.0f, 0.1f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.x, GND_BOSSROOM_CENTER_X - 150.0f, 0.1f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.y, GND_BOSSROOM_CENTER_Y + 107.0f, 0.1f, this->cameraSpeedMod * 20.0f);
            Math_ApproachF(&this->cameraAt.z, GND_BOSSROOM_CENTER_Z - 65.0f, 0.1f, this->cameraSpeedMod * 40.0f);
            Math_ApproachF(&this->cameraSpeedMod, 1.0f, 1.0f, 0.05f);
            if (this->timers[0] == 5) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_KID_HORSE_SANDDUST);
            }
            if (this->timers[0] == 0) {
                this->cutsceneState = INTRO_CUT;
                this->timers[0] = 50;
                this->cameraSpeedMod = 0.0f;
            }
            break;
        case INTRO_CUT:
            this->cutsceneState = INTRO_LAUGH;
            this->cameraEye.x = GND_BOSSROOM_CENTER_X + 50.0f;
            this->cameraEye.y = GND_BOSSROOM_CENTER_Y + 17.0f;
            this->cameraEye.z = GND_BOSSROOM_CENTER_Z + 110.0f;
            this->cameraAt.x = GND_BOSSROOM_CENTER_X - 150.0f;
            this->cameraAt.y = GND_BOSSROOM_CENTER_Y + 207.0f;
            this->cameraAt.z = GND_BOSSROOM_CENTER_Z - 155.0f;
            this->cameraEyeVel.x = fabsf(this->cameraEye.x - (GND_BOSSROOM_CENTER_X + 20.0f));
            this->cameraEyeVel.y = fabsf(this->cameraEye.y - (GND_BOSSROOM_CENTER_Y + 102.0f));
            this->cameraEyeVel.z = fabsf(this->cameraEye.z - (GND_BOSSROOM_CENTER_Z + 25.0f));
            this->cameraAtVel.x = fabsf(this->cameraAt.x - (GND_BOSSROOM_CENTER_X - 150.0f));
            this->cameraAtVel.y = fabsf(this->cameraAt.y - (GND_BOSSROOM_CENTER_Y + 197.0f));
            this->cameraAtVel.z = fabsf(this->cameraAt.z - (GND_BOSSROOM_CENTER_Z - 65.0f));
            this->timers[0] = 250;
        case INTRO_LAUGH:
            Math_ApproachF(&this->cameraEye.x, GND_BOSSROOM_CENTER_X + 20.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraEyeVel.x);
            Math_ApproachF(&this->cameraEye.y, GND_BOSSROOM_CENTER_Y + 102.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraEyeVel.y);
            Math_ApproachF(&this->cameraEye.z, GND_BOSSROOM_CENTER_Z + 25.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraEyeVel.z);
            Math_ApproachF(&this->cameraAt.x, GND_BOSSROOM_CENTER_X - 150.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraAtVel.x);
            Math_ApproachF(&this->cameraAt.y, GND_BOSSROOM_CENTER_Y + 197.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraAtVel.y);
            Math_ApproachF(&this->cameraAt.z, GND_BOSSROOM_CENTER_Z - 65.0f, 0.05f,
                           this->cameraSpeedMod * this->cameraAtVel.z);
            Math_ApproachF(&this->cameraSpeedMod, 0.01f, 1.0f, 0.001f);
            if ((this->timers[0] == 245) || (this->timers[0] == 3)) {
                Animation_MorphToPlayOnce(&this->skin.skelAnime, &gPhantomHorseRearingAnim, -8.0f);
                this->bossGndSignal = FHG_REAR;
                Actor_PlaySfx(&this->actor, NA_SE_EV_WATERDROP_GRD);
                if (this->timers[0] == 3) {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_FANTOM_VOICE);
                }
            }
            if (this->timers[0] == 192) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_KID_HORSE_SANDDUST);
            }
            if (this->timers[0] == 212) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_KID_HORSE_LAND2);
                Animation_Change(&this->skin.skelAnime, &gPhantomHorseIdleAnim, 0.3f, 0.0f, 5.0f, ANIMMODE_LOOP_INTERP,
                                 -10.0f);
            }
            if (this->timers[0] == 90) {
                play->envCtx.lightSettingOverride = 2;
                play->envCtx.lightBlendRateOverride = 0x14;
            }
            if (this->timers[0] == 100) {
                this->bossGndSignal = FHG_LIGHTNING;
            }
            if (this->timers[0] == 60) {
                this->bossGndSignal = FHG_RIDE;
            }
            if (this->timers[0] == 130) {
                AudioSeq_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0x5000FF);
            }
            if (this->timers[0] == 30) {
                bossGnd->work[GND_EYE_STATE] = GND_EYESTATE_BRIGHTEN;
            }
            if (this->timers[0] == 35) {
                Audio_PlaySfx_AtPos(&audioVec, NA_SE_EN_WIZ_EXP);
            }
            if (this->timers[0] == 130) {
                bossGnd->work[GND_EYE_STATE] = GND_EYESTATE_FADE;
                Audio_PlaySfx_AtPos(&audioVec, NA_SE_EN_LAST1_DAMAGE2_OLD);
            }
            if (this->timers[0] == 20) {
                AudioSeq_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS);
            }
            if (this->timers[0] == 2) {
                this->cameraSpeedMod = 0.0f;
                this->cutsceneState = INTRO_TITLE;
                this->cameraEyeVel.x = fabsf(this->cameraEye.x - (GND_BOSSROOM_CENTER_X + 180.0f));
                this->cameraEyeVel.y = fabsf(this->cameraEye.y - (GND_BOSSROOM_CENTER_Y + 7.0f));
                this->cameraEyeVel.z = fabsf(this->cameraEye.z - (GND_BOSSROOM_CENTER_Z + 140.0f));
                this->timers[0] = 100;
                this->timers[1] = 34;
                this->cameraAtVel.x = fabsf(this->cameraAt.x - this->actor.world.pos.x);
                this->cameraAtVel.y = fabsf(this->cameraAt.y - ((this->actor.world.pos.y + 70.0f) - 20.0f));
                this->cameraAtVel.z = fabsf(this->cameraAt.z - this->actor.world.pos.z);
            }
            break;
        case INTRO_TITLE:
            if (this->timers[1] == 1) {
                Animation_Change(&this->skin.skelAnime, &gPhantomHorseIdleAnim, 0.5f, 0.0f,
                                 Animation_GetLastFrame(&gPhantomHorseIdleAnim), ANIMMODE_LOOP_INTERP, -3.0f);
            }
            Math_ApproachF(&this->cameraEye.x, GND_BOSSROOM_CENTER_X + 180.0f, 0.1f,
                           this->cameraSpeedMod * this->cameraEyeVel.x);
            Math_ApproachF(&this->cameraEye.y, GND_BOSSROOM_CENTER_Y + 7.0f, 0.1f,
                           this->cameraSpeedMod * this->cameraEyeVel.y);
            Math_ApproachF(&this->cameraEye.z, this->cameraPanZ + (GND_BOSSROOM_CENTER_Z + 140.0f), 0.1f,
                           this->cameraSpeedMod * this->cameraEyeVel.z);
            Math_ApproachF(&this->cameraPanZ, -100.0f, 0.1f, 1.0f);
            Math_ApproachF(&this->cameraAt.x, this->actor.world.pos.x, 0.1f, this->cameraSpeedMod * 10.0f);
            Math_ApproachF(&this->cameraAt.y, (this->actor.world.pos.y + 70.0f) - 20.0f, 0.1f,
                           this->cameraSpeedMod * 10.0f);
            Math_ApproachF(&this->cameraAt.z, this->actor.world.pos.z, 0.1f, this->cameraSpeedMod * 10.0f);
            Math_ApproachF(&this->actor.world.pos.y, 60.0f, 0.1f, 2.0f);
            this->actor.world.pos.y += 2.0f * Math_SinS(this->gallopTimer * 0x5DC);
            Math_ApproachF(&this->cameraSpeedMod, 1.0f, 1.0f, 0.05f);
            if (this->timers[0] == 75) {
                TitleCard_InitBossName(&play->state, &play->actorCtx.titleCtx,
                                       Lib_SegmentedToVirtual(gPhantomGanonTitleCardENGTex), 160, 180, 128, 40);
            }
            if (this->timers[0] == 0) {
                this->cutsceneState = INTRO_RETREAT;
                this->timers[0] = 200;
                this->timers[1] = 23;
                this->cameraSpeedMod = 0.0f;
                Animation_Change(&this->skin.skelAnime, &gPhantomHorseLeapAnim, 1.0f, 0.0f,
                                 Animation_GetLastFrame(&gPhantomHorseLeapAnim), ANIMMODE_ONCE_INTERP, -4.0f);
                this->bossGndSignal = FHG_SPUR;
                Actor_PlaySfx(&this->actor, NA_SE_EN_FANTOM_VOICE);
                Actor_PlaySfx(&this->actor, NA_SE_EV_WATERDROP_GRD);
            }
            break;
        case INTRO_RETREAT:
            if (this->timers[1] == 1) {
                Animation_Change(&this->skin.skelAnime, &gPhantomHorseLandAnim, 0.5f, 0.0f,
                                 Animation_GetLastFrame(&gPhantomHorseLandAnim), ANIMMODE_ONCE_INTERP, -3.0f);
                this->bossGndSignal = FHG_FINISH;
            }
            if (this->timers[0] == 170) {
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 8);
                Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_VOICE);
            }
            Math_ApproachF(&this->cameraEye.z, this->cameraPanZ + (GND_BOSSROOM_CENTER_Z + 100.0f), 0.1f,
                           this->cameraSpeedMod * 1.5f);
            Math_ApproachF(&this->cameraPanZ, -100.0f, 0.1f, 1.0f);
            Math_ApproachF(&this->actor.world.pos.z, GND_BOSSROOM_CENTER_Z + 400.0f - 0.5f, 1.0f,
                           this->cameraSpeedMod * 10.0f);
            Math_ApproachF(&this->cameraSpeedMod, 1.0f, 1.0f, 0.05f);
            if ((fabsf(this->actor.world.pos.z - (GND_BOSSROOM_CENTER_Z + 400.0f - 0.5f)) < 300.0f) &&
                !this->spawnedWarp) {
                this->spawnedWarp = true;
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FHG_FIRE_Oot, GND_BOSSROOM_CENTER_X + 0.0f,
                                   this->actor.world.pos.y + 50.0f, GND_BOSSROOM_CENTER_Z + 400.0f - 0.5f, 0,
                                   this->actor.shape.rot.y, 0, FHGFIRE_WARP_RETREAT);
                this->fhgFireKillWarp = true;
            }
            Math_ApproachF(&this->cameraAt.x, this->actor.world.pos.x, 0.2f, 50.0f);
            Math_ApproachF(&this->cameraAt.z, this->actor.world.pos.z, 0.2f, 50.0f);
            if (fabsf(this->actor.world.pos.z - (GND_BOSSROOM_CENTER_Z + 400.0f - 0.5f)) < 1.0f) {
                play->envCtx.lightSettingOverride = 0;
                play->envCtx.lightBlendRateOverride = 0x14;
                this->cutsceneState = INTRO_FINISH;
                Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseRunningAnim, -3.0f);
                this->bossGndSignal = FHG_START_FIGHT;
                this->timers[1] = 75;
                this->timers[0] = 140;
            }
            break;
        case INTRO_FINISH:
            EnfHG_Retreat(this, play);
            Math_ApproachF(&this->cameraEye.z, this->cameraPanZ + (GND_BOSSROOM_CENTER_Z + 100.0f), 0.1f,
                           this->cameraSpeedMod * 1.5f);
            Math_ApproachF(&this->cameraPanZ, -100.0f, 0.1f, 1.0f);
            Math_ApproachF(&this->cameraAt.y, (this->actor.world.pos.y + 70.0f) - 20.0f, 0.1f,
                           this->cameraSpeedMod * 10.0f);
            if (this->timers[1] == 0) {
                Camera* camera = Play_GetCamera(play, 0);

                camera->eye = this->cameraEye;
                camera->eyeNext = this->cameraEye;
                camera->at = this->cameraAt;
                func_80169AFC(play, this->cutsceneCamera, 0);
                this->cutsceneCamera = 0;
                Cutscene_StopManualOot(play);
                Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
                this->actionFunc = EnfHG_Retreat;
            }
            break;
    }
    if (this->cutsceneCamera != 0) {
        Play_SetCameraAtEye(play, this->cutsceneCamera, &this->cameraAt, &this->cameraEye);
    }
}

static void EnfHG_SetupApproach(EnfHG* this, PlayState* play, s16 paintingIndex) {
    s16 oppositeIndex[6] = { 3, 4, 5, 0, 1, 2 };

    Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseRunningAnim, 0.0f);
    this->actionFunc = EnfHG_Approach;
    this->curPainting = paintingIndex;
    this->targetPainting = oppositeIndex[this->curPainting];


    this->actor.world.pos.x = (1.3f * sPaintings[this->curPainting].pos.x) + (GND_BOSSROOM_CENTER_X - 4.0f);
    this->actor.world.pos.y = sPaintings[this->curPainting].pos.y + (GND_BOSSROOM_CENTER_Y + 153.0f);
    this->actor.world.pos.z = (1.3f * sPaintings[this->curPainting].pos.z) - -(GND_BOSSROOM_CENTER_Z - 10.0f);
    this->actor.world.rot.y = sPaintings[this->curPainting].yRot;
    // Teleporting to another painting, don't blend across the room


    this->inPaintingPos.x = (sPaintings[this->targetPainting].pos.x * 1.3f) + (GND_BOSSROOM_CENTER_X - 4.0f);
    this->inPaintingPos.y = sPaintings[this->targetPainting].pos.y + (GND_BOSSROOM_CENTER_Y + 33.0f);
    this->inPaintingPos.z = (sPaintings[this->targetPainting].pos.z * 1.3f) - -(GND_BOSSROOM_CENTER_Z - 10.0f);
    this->inPaintingVelX = (fabsf(this->inPaintingPos.x - this->actor.world.pos.x) * 2) * 0.01f;

    if (this->inPaintingVelX < 1.0f) {
        this->inPaintingVelX = 1.0f;
    }
    this->inPaintingVelZ = (fabsf(this->inPaintingPos.z - this->actor.world.pos.z) * 2) * 0.01f;
    if (this->inPaintingVelZ < 1.0f) {
        this->inPaintingVelZ = 1.0f;
    }

    this->timers[0] = 100;
    this->actor.scale.x = 0.002f;
    this->actor.scale.y = 0.002f;
    this->actor.scale.z = 0.001f;
    this->approachRate = 0.0f;

    this->warpColorFilterR = play->lightCtx.fogColor[0];
    this->warpColorFilterG = play->lightCtx.fogColor[1];
    this->warpColorFilterB = play->lightCtx.fogColor[2];
    this->warpColorFilterUnk1 = 0.0f;
    this->warpColorFilterUnk2 = 0.0f;
    this->turnRot = 0;
    this->turnTarget = 0;
    this->spawnedWarp = false;
}

static void EnfHG_Approach(EnfHG* this, PlayState* play) {
    if (this->actor.params == GND_REAL_BOSS) {
        this->hoofSfxPos.x = this->actor.projectedPos.x / (this->actor.scale.x * 100.0f);
        this->hoofSfxPos.y = this->actor.projectedPos.y / (this->actor.scale.x * 100.0f);
        this->hoofSfxPos.z = this->actor.projectedPos.z / (this->actor.scale.x * 100.0f);
        if ((this->gallopTimer % 8) == 0) {
            Audio_PlaySfx_AtPos(&this->hoofSfxPos, NA_SE_EV_HORSE_RUN);
        }
    }
    SkelAnime_Update(&this->skin.skelAnime);
    Math_ApproachF(&this->actor.scale.x, 0.011499999f, 1.0f, this->approachRate);
    Math_ApproachF(&this->approachRate, 0.0002f, 1.0f, 0.0000015f);
    Math_ApproachF(&this->actor.world.pos.y, 60.0f, 0.1f, 1.0f);
    this->actor.scale.y = this->actor.scale.x;
    if (this->timers[0] == 0) {
        if (this->actor.params != GND_REAL_BOSS) {
            this->timers[0] = 140;
            this->actionFunc = EnfHG_Retreat;
            Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseRunningAnim, 0.0f);
            this->turnTarget = -0x8000;
        } else {
            this->actionFunc = EnfHG_Attack;
            Actor_PlaySfx(&this->actor, NA_SE_EV_WATERDROP_GRD);
            this->timers[0] = 40;
            Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FHG_FIRE_Oot, this->actor.world.pos.x,
                               this->actor.world.pos.y + 50.0f, this->actor.world.pos.z, 0,
                               this->actor.shape.rot.y + 0x8000, 0, FHGFIRE_WARP_EMERGE);
            this->fhgFireKillWarp = false;
        }
    }
}

static void EnfHG_Attack(EnfHG* this, PlayState* play) {
    this->bossGndInPainting = false;
    SkelAnime_Update(&this->skin.skelAnime);
    if (this->timers[0] != 0) {
        Math_ApproachF(&this->actor.scale.z, 0.011499999f, 1.0f, 0.0002f);
        if (this->timers[0] == 1) {
            this->bossGndSignal = FHG_RAISE_SPEAR;
            this->timers[1] = 50;
            Animation_MorphToPlayOnce(&this->skin.skelAnime, &gPhantomHorseLeapAnim, 0.0f);
        }
        Math_ApproachF(&this->warpColorFilterR, 255.0f, 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterG, 255.0f, 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterB, 255.0f, 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterUnk1, -60.0f, 1.0f, 5.0f);
    } else {
        Math_ApproachF(&this->warpColorFilterR, play->lightCtx.fogColor[0], 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterG, play->lightCtx.fogColor[0], 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterB, play->lightCtx.fogColor[0], 1.0f, 10.0f);
        Math_ApproachF(&this->warpColorFilterUnk1, 0.0f, 1.0f, 5.0f);
        if (this->timers[1] == 29) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_VOICE);
            Actor_PlaySfx(&this->actor, NA_SE_EN_FANTOM_VOICE);
        }
        if (this->hitTimer == 0) {
            if (this->timers[1] == 24) {
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FHG_FIRE_Oot, this->actor.world.pos.x,
                                   (this->actor.world.pos.y + 100.0f) + 25.0f, this->actor.world.pos.z, 0, 0, 0,
                                   FHGFIRE_LIGHTNING_STRIKE);
            }
            if (this->timers[1] == 45) {
                Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseAirAnim, 0.0f);
            }
            if (this->timers[1] == 38) {
                this->bossGndSignal = FHG_LIGHTNING;
            }
            if (this->timers[1] == 16) {
                Animation_MorphToPlayOnce(&this->skin.skelAnime, &gPhantomHorseLandAnim, 0.0f);
                this->bossGndSignal = FHG_RESET;
            }
        }
        Math_ApproachF(&this->actor.scale.z, 0.011499999f, 1.0f, 0.002f);
        Math_ApproachF(&this->actor.world.pos.x, this->inPaintingPos.x, 1.0f, this->inPaintingVelX);
        Math_ApproachF(&this->actor.world.pos.y, 60.0f, 0.1f, 1.0f);
        Math_ApproachF(&this->actor.world.pos.z, this->inPaintingPos.z, 1.0f, this->inPaintingVelZ);
    }
    if (this->hitTimer == 20) {
        this->actionFunc = EnfHG_Damage;
        this->spawnedWarp = false;
        Animation_Change(&this->skin.skelAnime, &gPhantomHorseLandAnim, -1.0f, 0.0f,
                         Animation_GetLastFrame(&gPhantomHorseLandAnim), ANIMMODE_ONCE, -5.0f);
        this->timers[0] = 10;
        this->bossGndSignal = FHG_RESET;
        this->damageSpeedMod = 1.0f;
    } else {
        f32 dx = this->actor.world.pos.x - this->inPaintingPos.x;
        f32 dz = this->actor.world.pos.z - this->inPaintingPos.z;
        f32 dxz = sqrtf(SQ(dx) + SQ(dz));

        if (dxz < 350.0f) {
            this->bossGndInPainting = true;
        }
        if ((dxz < 300.0f) && !this->spawnedWarp) {
            this->spawnedWarp = true;
            Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FHG_FIRE_Oot, this->inPaintingPos.x,
                               this->actor.world.pos.y + 50.0f, this->inPaintingPos.z, 0, this->actor.shape.rot.y, 0,
                               FHGFIRE_WARP_RETREAT);
            this->fhgFireKillWarp = true;
        }
        if (dxz == 0.0f) {
            this->timers[0] = 140;
            this->actionFunc = EnfHG_Retreat;
            Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseRunningAnim, 0.0f);
            this->bossGndSignal = FHG_RIDE;
        }
    }
}

static void EnfHG_Damage(EnfHG* this, PlayState* play) {
    f32 dx;
    f32 dz;
    f32 dxz2;

    SkelAnime_Update(&this->skin.skelAnime);
    Math_ApproachF(&this->warpColorFilterR, play->lightCtx.fogColor[0], 1.0f, 10.0f);
    Math_ApproachF(&this->warpColorFilterG, play->lightCtx.fogColor[0], 1.0f, 10.0f);
    Math_ApproachF(&this->warpColorFilterB, play->lightCtx.fogColor[0], 1.0f, 10.0f);
    Math_ApproachF(&this->warpColorFilterUnk1, 0.0f, 1.0f, 5.0f);
    Math_ApproachF(&this->actor.scale.z, 0.011499999f, 1.0f, 0.002f);
    if (this->timers[0] != 0) {
        Math_ApproachZeroF(&this->damageSpeedMod, 1.0f, 0.1f);
        if (this->timers[0] == 1) {
            this->targetPainting = this->curPainting;
            this->inPaintingPos.x = (sPaintings[this->targetPainting].pos.x * 1.3f) + (GND_BOSSROOM_CENTER_X - 4.0f);
            this->inPaintingPos.y = sPaintings[this->targetPainting].pos.y;
            this->inPaintingPos.z = (sPaintings[this->targetPainting].pos.z * 1.3f) - -(GND_BOSSROOM_CENTER_Z - 10.0f);
        }
    } else {
        Math_ApproachF(&this->damageSpeedMod, 1.0f, 1.0f, 0.1f);
    }
    Math_ApproachF(&this->actor.world.pos.x, this->inPaintingPos.x, 1.0f, this->damageSpeedMod * this->inPaintingVelX);
    Math_ApproachF(&this->actor.world.pos.y, 60.0f, 0.1f, 1.0f);
    Math_ApproachF(&this->actor.world.pos.z, this->inPaintingPos.z, 1.0f, this->damageSpeedMod * this->inPaintingVelZ);
    dx = this->actor.world.pos.x - this->inPaintingPos.x;
    dz = this->actor.world.pos.z - this->inPaintingPos.z;
    dxz2 = sqrtf(SQ(dx) + SQ(dz));
    if ((dxz2 < 300.0f) && (!this->spawnedWarp)) {
        this->spawnedWarp = true;
        Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FHG_FIRE_Oot, this->inPaintingPos.x,
                           this->actor.world.pos.y + 50.0f, this->inPaintingPos.z, 0, this->actor.shape.rot.y + 0x8000,
                           0, FHGFIRE_WARP_RETREAT);
    }
    if (dxz2 == 0.0f) {
        BossGanondrof* bossGnd = (BossGanondrof*)this->actor.parent;

        this->timers[0] = 140;
        this->actionFunc = EnfHG_Retreat;
        Animation_MorphToLoop(&this->skin.skelAnime, &gPhantomHorseRunningAnim, 0.0f);
        if (bossGnd->actor.colChkInfo.health > 24) {
            this->bossGndSignal = FHG_RIDE;
        } else {
            bossGnd->flyMode = GND_FLY_NEUTRAL;
        }
        this->turnTarget = -0x8000;
    }
}

static void EnfHG_Retreat(EnfHG* this, PlayState* play) {
    if (this->turnTarget != 0) {
        Math_ApproachS(&this->turnRot, this->turnTarget, 5, 2000);
    }
    if (this->actor.params == GND_REAL_BOSS) {
        this->hoofSfxPos.x = this->actor.projectedPos.x / (this->actor.scale.x * 100.0f);
        this->hoofSfxPos.y = this->actor.projectedPos.y / (this->actor.scale.x * 100.0f);
        this->hoofSfxPos.z = this->actor.projectedPos.z / (this->actor.scale.x * 100.0f);
        if ((this->gallopTimer % 8) == 0) {
            Audio_PlaySfx_AtPos(&this->hoofSfxPos, NA_SE_EV_HORSE_RUN);
        }
    }
    SkelAnime_Update(&this->skin.skelAnime);
    Math_ApproachF(&this->actor.scale.z, 0.001f, 1.0f, 0.001f);
    Math_ApproachF(&this->actor.scale.x, 0.002f, 0.05f, 0.0001f);
    Math_ApproachF(&this->actor.world.pos.y, 200.0f, 0.05f, 1.0f);
    this->actor.scale.y = this->actor.scale.x;
    if ((this->timers[0] == 80) && (this->actor.params == GND_REAL_BOSS)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_LAUGH2);
    }
    if (this->timers[0] == 0) {
        BossGanondrof* bossGnd = (BossGanondrof*)this->actor.parent;
        s16 paintingIdxReal;
        s16 paintingIdxFake;

        if (this->actor.params != GND_REAL_BOSS) {
            this->killActor = true;
            bossGnd->killActor = true;
        } else if (bossGnd->flyMode != GND_FLY_PAINTING) {
            this->actionFunc = EnfHG_Done;
            this->actor.draw = NULL;
        } else {
            paintingIdxReal = Rand_ZeroOne() * 5.99f;
            EnfHG_SetupApproach(this, play, paintingIdxReal);
            do {
                paintingIdxFake = Rand_ZeroOne() * 5.99f;
            } while (paintingIdxFake == paintingIdxReal);
        }
    }
}

static void EnfHG_Done(EnfHG* this, PlayState* play) {
    this->bossGndInPainting = false;
}

static void EnfHG_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnfHG* this = (EnfHG*)thisx;
    u8 i;

    if (this->killActor) {
        Actor_Kill(&this->actor);
        return;
    }
    this->gallopTimer++;
    this->bossGndInPainting = true;
    for (i = 0; i < 5; i++) {
        if (this->timers[i] != 0) {
            this->timers[i]--;
        }
    }

    this->actionFunc(this, play);

    if (this->hitTimer != 0) {
        this->hitTimer--;
    }

    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += 70.0f;
    this->actor.shape.rot.y = this->actor.world.rot.y;

    this->actor.shape.yOffset = Math_SinS(this->hitTimer * 0x9000) * 700.0f * (this->hitTimer / 20.0f);
    this->actor.shape.rot.z = (s16)(Math_SinS(this->hitTimer * 0x7000) * 1500.0f) * (this->hitTimer / 20.0f);
}

static void EnfHG_PostDraw(Actor* thisx, PlayState* play, Skin* skin) {
}

static void EnfHG_Draw(Actor* thisx, PlayState* play) {
    EnfHG* this = (EnfHG*)thisx;
    BossGanondrof* bossGnd = (BossGanondrof*)this->actor.parent;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    POLY_OPA_DISP = ((bossGnd->work[GND_INVINC_TIMER] & 4) && (bossGnd->flyMode == GND_FLY_PAINTING))
                        ? Gfx_SetFog(POLY_OPA_DISP, 255, 50, 0, 0, 900, 1099)
                        : Gfx_SetFog(POLY_OPA_DISP, (u32)this->warpColorFilterR, (u32)this->warpColorFilterG,
                                     (u32)this->warpColorFilterB, 0, (s32)this->warpColorFilterUnk1 + 995,
                                     (s32)this->warpColorFilterUnk2 + 1000);
    // PORT: OoT's skin code adds turnRot to the root joint's yaw (entry 1; 0 is the translation) for this horse.
    this->skin.skelAnime.jointTable[1].y += this->turnRot;
    func_80138228(&this->actor, play, &this->skin, EnfHG_PostDraw, true);
    this->skin.skelAnime.jointTable[1].y -= this->turnRot;
    POLY_OPA_DISP = Play_SetFog(play, POLY_OPA_DISP);
    CLOSE_DISPS(play->state.gfxCtx);
}
