#include "z_demo_du.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_du/object_du.h"
#include "../ovl_Demo_Effect/z_demo_effect.h"
#include "../ovl_Door_Warp1/z_door_warp1.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

typedef void (*DemoDuActionFunc)(DemoDu*, PlayState*);
typedef void (*DemoDuDrawFunc)(Actor*, PlayState*);

static void DemoDu_Init(Actor* thisx, PlayState* play);
static void DemoDu_Destroy(Actor* thisx, PlayState* play);
static void DemoDu_Update(Actor* thisx, PlayState* play);
static void DemoDu_Draw(Actor* thisx, PlayState* play);
static void DemoDu_Reset(void);

// static s32 sUnused = 0;

#include "z_demo_du_cutscene_data.c" EARLY

static void* sEyeTextures[] = { gDaruniaEyeOpenTex, gDaruniaEyeOpeningTex, gDaruniaEyeShutTex, gDaruniaEyeClosingTex };
static void* sMouthTextures[] = { gDaruniaMouthSeriousTex, gDaruniaMouthGrinningTex, gDaruniaMouthOpenTex,
                                  gDaruniaMouthHappyTex };

/**
 * Cs => Cutscene
 *
 * FM => Fire Medallion
 * GR => Goron's Ruby
 * AG => In the chamber of sages, just After the final blow on Ganon.
 * CR => Credits
 *
 */

// Each macro maps its argument to an index of sUpdateFuncs.
#define OOT_CS_FIREMEDALLION_SUBSCENE(x) (0 + (x))      // DEMO_DU_CS_FIREMEDALLION
#define OOT_CS_GORONSRUBY_SUBSCENE(x) (7 + (x))         // DEMO_DU_CS_GORONS_RUBY
#define OOT_CS_CHAMBERAFTERGANON_SUBSCENE(x) (21 + (x)) // DEMO_DU_CS_CHAMBER_AFTER_GANON
#define OOT_CS_CREDITS_SUBSCENE(x) (24 + (x))           // DEMO_DU_CS_CREDITS

static void DemoDu_Destroy(Actor* thisx, PlayState* play) {
    DemoDu* this = (DemoDu*)thisx;

    SkelAnime_Free(&this->skelAnime, play);
}

static void DemoDu_UpdateEyes(DemoDu* this) {
    s16* blinkTimer = &this->blinkTimer;
    s16* eyeTexIndex = &this->eyeTexIndex;
    s32 pad[3];

    if (DECR(*blinkTimer) == 0) {
        *blinkTimer = Rand_S16Offset(60, 60);
    }

    *eyeTexIndex = *blinkTimer;
    if (*eyeTexIndex >= 3) {
        *eyeTexIndex = 0;
    }
}

static void DemoDu_SetEyeTexIndex(DemoDu* this, s16 eyeTexIndex) {
    this->eyeTexIndex = eyeTexIndex;
}

static void DemoDu_SetMouthTexIndex(DemoDu* this, s16 mouthTexIndex) {
    this->mouthTexIndex = mouthTexIndex;
}

// Resets all the values used in this cutscene.
static void DemoDu_CsAfterGanon_Reset(DemoDu* this) {
    this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(0);
    this->drawIndex = 0;
    this->shadowAlpha = 0;
    this->demo6KSpawned = 0;
    this->actor.shape.shadowAlpha = 0;
    this->unk_1A4 = 0.0f;
}

static s32 D_8096CE94 = false;

static void DemoDu_CsAfterGanon_CheckIfShouldReset(DemoDu* this, PlayState* play) {

    if (gCsCtxOot.state == CS_STATE_IDLE_Oot) {
        if (D_8096CE94) {
            if (this->actor.params == DEMO_DU_CS_CHAMBER_AFTER_GANON) {
                DemoDu_CsAfterGanon_Reset(this);
            }
            D_8096CE94 = false;
            return;
        }
    } else if (!D_8096CE94) {
        D_8096CE94 = true;
    }
}

static s32 DemoDu_UpdateSkelAnime(DemoDu* this) {
    return SkelAnime_Update(&this->skelAnime);
}

static void DemoDu_UpdateBgCheckInfo(DemoDu* this, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f, 0x405);
}

static CsCmdActorCueOot* DemoDu_GetNpcAction(PlayState* play, s32 idx) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        return gCsCtxOot.npcActions[idx];
    }
    return NULL;
}

static s32 DemoDu_IsNpcDoingThisAction(DemoDu* this, PlayState* play, u16 action, s32 idx) {
    CsCmdActorCueOot* npcAction = DemoDu_GetNpcAction(play, idx);

    if ((npcAction != NULL) && (npcAction->action == action)) {
        return true;
    }
    return false;
}

static s32 DemoDu_IsNpcNotDoingThisAction(DemoDu* this, PlayState* play, u16 action, s32 idx) {
    CsCmdActorCueOot* npcAction = DemoDu_GetNpcAction(play, idx);

    if ((npcAction != NULL) && (npcAction->action != action)) {
        return true;
    }
    return false;
}

static void DemoDu_MoveToNpcPos(DemoDu* this, PlayState* play, s32 idx) {
    CsCmdActorCueOot* npcAction = DemoDu_GetNpcAction(play, idx);
    s32 pad;

    if (npcAction != NULL) {
        this->actor.world.pos.x = npcAction->startPos.x;
        this->actor.world.pos.y = npcAction->startPos.y;
        this->actor.world.pos.z = npcAction->startPos.z;

        this->actor.world.rot.y = this->actor.shape.rot.y = npcAction->rot.y;
    }
}

static void func_80969DDC(DemoDu* this, AnimationHeader* animation, u8 mode, f32 morphFrames, s32 arg4) {
    f32 startFrame;
    s16 lastFrame = Animation_GetLastFrame(animation);
    f32 endFrame;
    f32 playSpeed;

    if (arg4 == 0) {
        startFrame = 0.0f;
        endFrame = lastFrame;
        playSpeed = 1.0f;
    } else {
        endFrame = 0.0f;
        playSpeed = -1.0f;
        startFrame = lastFrame;
    }
    Animation_Change(&this->skelAnime, animation, playSpeed, startFrame, endFrame, mode, morphFrames);
}

static void DemoDu_InitCs_FireMedallion(DemoDu* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gDaruniaSkel, &gDaruniaIdleAnim, NULL, NULL, 0);
    this->actor.shape.yOffset = -10000.0f;
    DemoDu_SetEyeTexIndex(this, 1);
    DemoDu_SetMouthTexIndex(this, 3);
}

// A.k.a Warp portal
static void DemoDu_CsFireMedallion_SpawnDoorWarp(DemoDu* this, PlayState* play) {
    f32 posX = this->actor.world.pos.x;
    f32 posY = this->actor.world.pos.y;
    f32 posZ = this->actor.world.pos.z;

    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_WARP1_Oot, posX, posY, posZ, 0, 0, 0, WARP_SAGES);
}

// Gives the Fire Medallion to Link.
static void func_80969F38(DemoDu* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 posX = player->actor.world.pos.x;
    f32 posY = player->actor.world.pos.y + 80.0f;
    f32 posZ = player->actor.world.pos.z;

    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DEMO_EFFECT_Oot, posX, posY, posZ, 0, 0, 0,
                       DEMO_EFFECT_MEDAL_FIRE);
    if ((true)) {
        Item_Give(play, ITEM_MEDALLION_FIRE_Oot);
    }
}

static void func_80969FB4(DemoDu* this, PlayState* play) {
    this->actor.shape.yOffset += 250.0f / 3.0f;
}

// Gives the Fire Medallion to Link too.
static void DemoDu_CsFireMedallion_AdvanceTo01(DemoDu* this, PlayState* play) {
    s32 pad[2];

    if ((gSaveContext.chamberCutsceneNum == 1) && (gSaveContext.sceneLayer < 4)) {
        Player* player = GET_PLAYER(play);

        this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(1);
        gCsCtxOot.segment = D_8096C1A4;
        Hyrule_TriggerCutsceneOot(2);
        if ((true)) {
            Item_Give(play, ITEM_MEDALLION_FIRE_Oot);
        }

        player->actor.world.rot.y = player->actor.shape.rot.y = this->actor.world.rot.y + 0x8000;
    }
}

static void DemoDu_CsFireMedallion_AdvanceTo02(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 1)) {
            this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(2);
            this->drawIndex = 1;
            DemoDu_CsFireMedallion_SpawnDoorWarp(this, play);
        }
    }
}

static void DemoDu_CsFireMedallion_AdvanceTo03(DemoDu* this) {
    if (this->actor.shape.yOffset >= 0.0f) {
        this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(3);
        this->actor.shape.yOffset = 0.0f;
    }
}

static void DemoDu_CsFireMedallion_AdvanceTo04(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 2)) {
            Animation_Change(&this->skelAnime, &gDaruniaItemGiveAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gDaruniaItemGiveAnim), 2, 0.0f);
            this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(4);
        }
    }
}

static void DemoDu_CsFireMedallion_AdvanceTo05(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaItemGiveIdleAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gDaruniaItemGiveIdleAnim), 0, 0.0f);
        this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(5);
    }
}

static void DemoDu_CsFireMedallion_AdvanceTo06(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[6];

        if ((npcAction != NULL) && (npcAction->action == 2)) {
            this->updateIndex = OOT_CS_FIREMEDALLION_SUBSCENE(6);
            func_80969F38(this, play);
        }
    }
}

static void DemoDu_UpdateCs_FM_00(DemoDu* this, PlayState* play) {
    DemoDu_CsFireMedallion_AdvanceTo01(this, play);
}

static void DemoDu_UpdateCs_FM_01(DemoDu* this, PlayState* play) {
    DemoDu_CsFireMedallion_AdvanceTo02(this, play);
}

static void DemoDu_UpdateCs_FM_02(DemoDu* this, PlayState* play) {
    func_80969FB4(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsFireMedallion_AdvanceTo03(this);
}

static void DemoDu_UpdateCs_FM_03(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsFireMedallion_AdvanceTo04(this, play);
}

static void DemoDu_UpdateCs_FM_04(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsFireMedallion_AdvanceTo05(this, animFinished);
}

static void DemoDu_UpdateCs_FM_05(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsFireMedallion_AdvanceTo06(this, play);
}

static void DemoDu_UpdateCs_FM_06(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
}

static void DemoDu_InitCs_GoronsRuby(DemoDu* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gDaruniaSkel, NULL, NULL, NULL, 0);
    this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(0);
}

// Cutscene: Darunia gives Link the Goron's Ruby.
// Sfx played when Darunia lands at the floor at the start of the cutscene.
static void DemoDu_CsPlaySfx_GoronLanding(DemoDu* this) {
    Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_EN_GOLON_LAND_BIG);
}

// Cutscene: Darunia gives Link the Goron's Ruby.
// Sfx played when Darunia is falling at the start of the cutscene.
static void DemoDu_CsPlaySfx_DaruniaFalling(PlayState* play) {
    if (gCsCtxOot.frames == 160) {
        Audio_PlaySfx_2(NA_SE_EV_HONEYCOMB_FALL);
    }
}

// Cutscene: Darunia gives Link the Goron's Ruby.
static void DemoDu_CsPlaySfx_DaruniaHitsLink(PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 pad;

    Audio_PlaySfx_AtPos(&player->actor.projectedPos, NA_SE_EN_STALKIDS_LAUGH_MD2);
    Audio_PlaySfx_AtPos(&player->actor.projectedPos, NA_SE_VO_DUMMY_37);
}

// Cutscene: Darunia gives Link the Goron's Ruby.
static void DemoDu_CsPlaySfx_HitBreast(DemoDu* this) {
    Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_EN_STALKIDS_STRETCH - SFX_FLAG);
}

// Cutscene: Darunia gives Link the Goron's Ruby.
// Sfx played when Link is escaping from the gorons at the end of the scene.
static void DemoDu_CsPlaySfx_LinkEscapeFromGorons(PlayState* play) {
    if (gCsCtxOot.frames == 1400) {
        Player* player = GET_PLAYER(play);

        Audio_PlaySfx_AtPos(&player->actor.projectedPos, NA_SE_VO_NAVY_ENEMY);
    }
}

// Cutscene: Darunia gives Link the Goron's Ruby.
// Sfx played when Link is surprised by Darunia falling from the sky.
static void DemoDu_CsPlaySfx_LinkSurprised(PlayState* play) {
    if (gCsCtxOot.frames == 174) {
        Player* player = GET_PLAYER(play);

        AudioSfx_PlaySfx(NA_SE_VO_DUMMY_54, &player->actor.projectedPos, 4U, &gSfxDefaultFreqAndVolScale,
                             &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }
}

static void DemoDu_CsGoronsRuby_UpdateFaceTextures(DemoDu* this, PlayState* play) {
    u16* frames = &gCsCtxOot.frames;

    if (*frames < 260) {
        DemoDu_UpdateEyes(this);
        DemoDu_SetMouthTexIndex(this, 0);
    } else if (*frames < 335) {
        DemoDu_UpdateEyes(this);
        DemoDu_SetMouthTexIndex(this, 3);
    } else if (*frames < 365) {
        DemoDu_SetEyeTexIndex(this, 3);
        DemoDu_SetMouthTexIndex(this, 1);
    } else if (*frames < 395) {
        DemoDu_SetEyeTexIndex(this, 0);
        DemoDu_SetMouthTexIndex(this, 3);
    } else if (*frames < 410) {
        DemoDu_UpdateEyes(this);
        DemoDu_SetMouthTexIndex(this, 0);
    } else {
        DemoDu_UpdateEyes(this);
        DemoDu_SetMouthTexIndex(this, 3);
    }
}

static void func_8096A630(DemoDu* this, PlayState* play) {
    s32 pad;
    Vec3f pos = this->actor.world.pos;

    pos.y += kREG(5);
    func_800BBFB0(play, &pos, kREG(1) + 100.0f, kREG(2) + 10, kREG(3) + 300, kREG(4), 0);
    DemoDu_CsPlaySfx_GoronLanding(this);
}

static void DemoDu_CsGoronsRuby_SpawnDustWhenHittingLink(DemoDu* this, PlayState* play) {
    static Vec3f dustPosOffsets[] = {
        { 11.0f, -11.0f, -6.0f }, { 0.0f, 14.0f, -13.0f },  { 14.0f, -2.0f, -10.0f }, { 10.0f, -6.0f, -8.0f },
        { 8.0f, 6.0f, 8.0f },     { 13.0f, 8.0f, -10.0f },  { -14.0f, 1.0f, -14.0f }, { 5.0f, 12.0f, -9.0f },
        { 11.0f, 6.0f, -7.0f },   { 14.0f, 14.0f, -14.0f },
    };

    if (Animation_OnFrame(&this->skelAnime, 31.0f) || Animation_OnFrame(&this->skelAnime, 41.0f)) {
        s32 pad[2];
        s32 i;
        Player* player = GET_PLAYER(play);
        Vec3f* headPos = &player->bodyPartsPos[PLAYER_LIMB_HEAD];
        Vec3f velocity = { 0.0f, 0.0f, 0.0f };
        Vec3f accel = { 0.0f, 0.3f, 0.0f };
        s32 pad2;

        for (i = 4; i >= 0; --i) {
            Color_RGBA8 primColor = { 190, 150, 110, 255 };
            Color_RGBA8 envColor = { 120, 80, 40, 255 };
            s32 colorDelta;
            Vec3f position;

            if (Animation_OnFrame(&this->skelAnime, 31.0f)) {
                position.x = dustPosOffsets[i + 5].x + headPos->x;
                position.y = dustPosOffsets[i + 5].y + headPos->y;
                position.z = dustPosOffsets[i + 5].z + headPos->z;
            } else {
                position.x = dustPosOffsets[i + 0].x + headPos->x;
                position.y = dustPosOffsets[i + 0].y + headPos->y;
                position.z = dustPosOffsets[i + 0].z + headPos->z;
            }

            colorDelta = Rand_ZeroOne() * 20.0f - 10.0f;

            primColor.r += colorDelta;
            primColor.g += colorDelta;
            primColor.b += colorDelta;
            envColor.r += colorDelta;
            envColor.g += colorDelta;
            envColor.b += colorDelta;

            func_8002829C(play, &position, &velocity, &accel, &primColor, &envColor, Rand_ZeroOne() * 40.0f + 200.0f,
                          0);
        }

        DemoDu_CsPlaySfx_DaruniaHitsLink(play);
    }
}

static void DemoDu_CsGoronsRuby_DaruniaFalling(DemoDu* this, PlayState* play) {
    s32 pad;
    CutsceneContextOot* csCtx = &gCsCtxOot;

    if (csCtx->state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = csCtx->npcActions[2];
        Vec3f startPos;
        Vec3f endPos;
        Vec3f* pos = &this->actor.world.pos;

        if (npcAction != NULL) {
            f32 traveledPercent = Environment_LerpWeight(npcAction->endFrame, npcAction->startFrame, csCtx->frames);

            startPos.x = npcAction->startPos.x;
            startPos.y = npcAction->startPos.y;
            startPos.z = npcAction->startPos.z;

            endPos.x = npcAction->endPos.x;
            endPos.y = npcAction->endPos.y;
            endPos.z = npcAction->endPos.z;

            pos->x = ((endPos.x - startPos.x) * traveledPercent) + startPos.x;
            pos->y = ((endPos.y - startPos.y) * traveledPercent) + startPos.y;
            pos->z = ((endPos.z - startPos.z) * traveledPercent) + startPos.z;
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo01(DemoDu* this, PlayState* play) {
    this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(1);
}

static void DemoDu_CsGoronsRuby_AdvanceTo02(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 1)) {
            Animation_Change(&this->skelAnime, &gDaruniaStandUpAfterFallingAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gDaruniaStandUpAfterFallingAnim), 2, 0.0f);
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(2);
            this->drawIndex = 1;
            DemoDu_CsGoronsRuby_DaruniaFalling(this, play);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo03(DemoDu* this, PlayState* play) {
    CutsceneContextOot* csCtx = &gCsCtxOot;

    if (csCtx->state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = csCtx->npcActions[2];

        if ((npcAction != NULL) && (csCtx->frames >= npcAction->endFrame)) {
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(3);
            func_8096A630(this, play);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo04(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 2)) {
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(4);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo05(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaIdleAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gDaruniaIdleAnim),
                         ANIMMODE_LOOP, 0.0f);
        this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(5);
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo06(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 3)) {
            Animation_Change(&this->skelAnime, &gDaruniaHitBreastAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gDaruniaHitBreastAnim), 2, -4.0f);
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(6);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo07(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaIdleAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gDaruniaIdleAnim),
                         ANIMMODE_LOOP, 0.0f);
        this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(7);
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo08(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 4)) {
            Animation_Change(&this->skelAnime, &gDaruniaHitLinkAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gDaruniaHitLinkAnim), 2, 0.0f);
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(8);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo09(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaHitBreastAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gDaruniaHitBreastAnim), 2, 0.0f);
        this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(9);
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo10(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaIdleAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gDaruniaIdleAnim),
                         ANIMMODE_LOOP, 0.0f);
        this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(10);
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo11(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 5)) {
            Animation_Change(&this->skelAnime, &gDaruniaItemGiveAnim, 1.0f, 0.0f,
                             Animation_GetLastFrame(&gDaruniaItemGiveAnim), 2, 0.0f);
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(11);
        }
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo12(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        Animation_Change(&this->skelAnime, &gDaruniaItemGiveIdleAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gDaruniaItemGiveIdleAnim), 0, 0.0f);
        this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(12);
    }
}

static void DemoDu_CsGoronsRuby_AdvanceTo13(DemoDu* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        CsCmdActorCueOot* npcAction = gCsCtxOot.npcActions[2];

        if ((npcAction != NULL) && (npcAction->action != 6)) {
            Animation_Change(&this->skelAnime, &gDaruniaIdleAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gDaruniaIdleAnim),
                             ANIMMODE_LOOP, 0.0f);
            this->updateIndex = OOT_CS_GORONSRUBY_SUBSCENE(13);
        }
    }
}

static void DemoDu_UpdateCs_GR_00(DemoDu* this, PlayState* play) {
    DemoDu_CsPlaySfx_DaruniaFalling(play);
    DemoDu_CsGoronsRuby_AdvanceTo01(this, play);
}

static void DemoDu_UpdateCs_GR_01(DemoDu* this, PlayState* play) {
    DemoDu_CsPlaySfx_DaruniaFalling(play);
    DemoDu_CsPlaySfx_LinkSurprised(play);
    DemoDu_CsGoronsRuby_AdvanceTo02(this, play);
}

static void DemoDu_UpdateCs_GR_02(DemoDu* this, PlayState* play) {
    DemoDu_CsGoronsRuby_DaruniaFalling(this, play);
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_CsPlaySfx_DaruniaFalling(play);
    DemoDu_CsPlaySfx_LinkSurprised(play);
    DemoDu_CsGoronsRuby_AdvanceTo03(this, play);
}

static void DemoDu_UpdateCs_GR_03(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_CsPlaySfx_LinkSurprised(play);
    DemoDu_CsGoronsRuby_AdvanceTo04(this, play);
}

static void DemoDu_UpdateCs_GR_04(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo05(this, animFinished);
}

static void DemoDu_UpdateCs_GR_05(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo06(this, play);
}

static void DemoDu_UpdateCs_GR_06(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsPlaySfx_HitBreast(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo07(this, animFinished);
}

static void DemoDu_UpdateCs_GR_07(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo08(this, play);
}

static void DemoDu_UpdateCs_GR_08(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_SpawnDustWhenHittingLink(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo09(this, animFinished);
}

static void DemoDu_UpdateCs_GR_09(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsPlaySfx_HitBreast(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo10(this, animFinished);
}

static void DemoDu_UpdateCs_GR_10(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo11(this, play);
}

static void DemoDu_UpdateCs_GR_11(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo12(this, animFinished);
}

static void DemoDu_UpdateCs_GR_12(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsGoronsRuby_AdvanceTo13(this, play);
}

static void DemoDu_UpdateCs_GR_13(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_CsGoronsRuby_UpdateFaceTextures(this, play);
    DemoDu_CsPlaySfx_LinkEscapeFromGorons(play);
}

static void DemoDu_InitCs_AfterGanon(DemoDu* this, PlayState* play) {
    s32 pad[3];
    f32 lastFrame = Animation_GetLastFrame(&gDaruniaSageFormationAnim);

    SkelAnime_InitFlex(play, &this->skelAnime, &gDaruniaSkel, NULL, NULL, NULL, 0);
    Animation_Change(&this->skelAnime, &gDaruniaSageFormationAnim, 1.0f, 0.0f, lastFrame, ANIMMODE_ONCE, 0.0f);
    this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(0);
    this->actor.shape.shadowAlpha = 0;
}

static void DemoDu_CsPlaySfx_WhiteOut() {
    Audio_PlaySfx_2(NA_SE_SY_WHITE_OUT_T);
}

static void DemoDu_CsAfterGanon_SpawnDemo6K(DemoDu* this, PlayState* play) {
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DEMO_6K_Oot, this->actor.world.pos.x,
                       kREG(16) + 22.0f + this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 3);
}

static void DemoDu_CsAfterGanon_AdvanceTo01(DemoDu* this, PlayState* play) {
    if (DemoDu_IsNpcDoingThisAction(this, play, 4, 2)) {
        this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(1);
        this->drawIndex = 2;
        this->shadowAlpha = 0;
        this->actor.shape.shadowAlpha = 0;
        this->unk_1A4 = 0.0f;
        DemoDu_CsPlaySfx_WhiteOut();
    }
}

static void DemoDu_CsAfterGanon_AdvanceTo02(DemoDu* this, PlayState* play) {
    f32* unk_1A4 = &this->unk_1A4;
    s32 shadowAlpha = 255;

    if (DemoDu_IsNpcDoingThisAction(this, play, 4, 2)) {
        *unk_1A4 += 1.0f;
        if (*unk_1A4 >= kREG(5) + 10.0f) {
            this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(2);
            this->drawIndex = 1;
            *unk_1A4 = kREG(5) + 10.0f;
            this->shadowAlpha = shadowAlpha;
            this->actor.shape.shadowAlpha = shadowAlpha;
            return;
        }
    } else {
        *unk_1A4 -= 1.0f;
        if (*unk_1A4 <= 0.0f) {
            this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(0);
            this->drawIndex = 0;
            *unk_1A4 = 0.0f;
            this->shadowAlpha = 0;
            this->actor.shape.shadowAlpha = 0;
            return;
        }
    }
    shadowAlpha = (*unk_1A4 / (kREG(5) + 10.0f)) * 255.0f;
    this->shadowAlpha = shadowAlpha;
    this->actor.shape.shadowAlpha = shadowAlpha;
}

static void DemoDu_CsAfterGanon_BackTo01(DemoDu* this, PlayState* play) {
    if (DemoDu_IsNpcNotDoingThisAction(this, play, 4, 2)) {
        this->updateIndex = OOT_CS_CHAMBERAFTERGANON_SUBSCENE(1);
        this->drawIndex = 2;
        this->unk_1A4 = kREG(5) + 10.0f;
        this->shadowAlpha = 255;
        if (!this->demo6KSpawned) {
            DemoDu_CsAfterGanon_SpawnDemo6K(this, play);
            this->demo6KSpawned = 1;
        }
        this->actor.shape.shadowAlpha = 255;
    }
}

static void DemoDu_UpdateCs_AG_00(DemoDu* this, PlayState* play) {
    DemoDu_CsAfterGanon_AdvanceTo01(this, play);
    DemoDu_CsAfterGanon_CheckIfShouldReset(this, play);
}

static void DemoDu_UpdateCs_AG_01(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsAfterGanon_AdvanceTo02(this, play);
    DemoDu_CsAfterGanon_CheckIfShouldReset(this, play);
}

static void DemoDu_UpdateCs_AG_02(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsAfterGanon_BackTo01(this, play);
    DemoDu_CsAfterGanon_CheckIfShouldReset(this, play);
}

// Similar to DemoDu_Draw_01, but this uses POLY_XLU_DISP. Also uses this->shadowAlpha for setting the env color.
static void DemoDu_Draw_02(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    DemoDu* this = (DemoDu*)thisx;
    s16 eyeTexIndex = this->eyeTexIndex;
    void* eyeTexture = sEyeTextures[eyeTexIndex];
    s32 pad;
    s16 mouthTexIndex = this->mouthTexIndex;
    void* mouthTexture = sMouthTextures[mouthTexIndex];
    SkelAnime* skelAnime = &this->skelAnime;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTexture));
    gSPSegment(POLY_XLU_DISP++, 0x09, Lib_SegmentedToVirtual(mouthTexture));
    gSPSegment(POLY_XLU_DISP++, 0x0A, Lib_SegmentedToVirtual(gDaruniaNoseSeriousTex));

    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, this->shadowAlpha);

    gSPSegment(POLY_XLU_DISP++, 0x0C, &D_80116280_Oot[0]);

    POLY_XLU_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount, 0, 0, 0,
                                       POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void DemoDu_InitCs_Credits(DemoDu* this, PlayState* play) {
    SkelAnime_InitFlex(play, &this->skelAnime, &gDaruniaSkel, &gDaruniaCreditsIdleAnim, NULL, NULL, 0);
    this->updateIndex = OOT_CS_CREDITS_SUBSCENE(0);
    this->drawIndex = 0;
    this->actor.shape.shadowAlpha = 0;
    DemoDu_SetMouthTexIndex(this, 3);
}

static void DemoDu_CsCredits_UpdateShadowAlpha(DemoDu* this) {
    s32 shadowAlpha = 255;
    f32 temp_f0;
    f32* unk_1A4;

    this->unk_1A4 += 1.0f;
    temp_f0 = kREG(17) + 10.0f;
    unk_1A4 = &this->unk_1A4;

    if (temp_f0 <= *unk_1A4) {
        this->shadowAlpha = shadowAlpha;
        this->actor.shape.shadowAlpha = shadowAlpha;
    } else {
        shadowAlpha = *unk_1A4 / temp_f0 * 255.0f;
        this->shadowAlpha = shadowAlpha;
        this->actor.shape.shadowAlpha = shadowAlpha;
    }
}

static void DemoDu_CsCredits_AdvanceTo01(DemoDu* this, PlayState* play) {
    DemoDu_MoveToNpcPos(this, play, 2);
    this->updateIndex = OOT_CS_CREDITS_SUBSCENE(1);
    this->drawIndex = 2;
}

static void DemoDu_CsCredits_AdvanceTo02(DemoDu* this) {
    if (this->unk_1A4 >= kREG(17) + 10.0f) {
        this->updateIndex = OOT_CS_CREDITS_SUBSCENE(2);
        this->drawIndex = 1;
    }
}

static void DemoDu_CsCredits_AdvanceTo03(DemoDu* this) {
    func_80969DDC(this, &gDaruniaLookingUpToSariaAnim, ANIMMODE_ONCE, -8.0f, 0);
    this->updateIndex = OOT_CS_CREDITS_SUBSCENE(3);
}

static void DemoDu_CsCredits_AdvanceTo04(DemoDu* this) {
    func_80969DDC(this, &gDaruniaCreditsHitBreastAnim, ANIMMODE_ONCE, 0.0f, 0);
    this->updateIndex = OOT_CS_CREDITS_SUBSCENE(4);
}

static void DemoDu_CsCredits_BackTo02(DemoDu* this, s32 animFinished) {
    if (animFinished) {
        func_80969DDC(this, &gDaruniaCreditsIdleAnim, ANIMMODE_LOOP, 0.0f, 0);
        this->updateIndex = OOT_CS_CREDITS_SUBSCENE(2);
    }
}

static void DemoDu_CsCredits_HandleSubscenesByNpcAction(DemoDu* this, PlayState* play) {
    CsCmdActorCueOot* npcAction = DemoDu_GetNpcAction(play, 2);

    if (npcAction != NULL) {
        s32 action = npcAction->action;
        s32 lastAction = this->lastAction;

        if (action != lastAction) {
            switch (action) {
                case 9:
                    DemoDu_CsCredits_AdvanceTo01(this, play);
                    break;
                case 10:
                    DemoDu_CsCredits_AdvanceTo03(this);
                    break;
                case 11:
                    DemoDu_CsCredits_AdvanceTo04(this);
                    break;
                default:
                    break;
            }
            this->lastAction = action;
        }
    }
}

static void DemoDu_UpdateCs_CR_00(DemoDu* this, PlayState* play) {
    DemoDu_CsCredits_HandleSubscenesByNpcAction(this, play);
}

static void DemoDu_UpdateCs_CR_01(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsCredits_UpdateShadowAlpha(this);
    DemoDu_CsCredits_AdvanceTo02(this);
}

static void DemoDu_UpdateCs_CR_02(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsCredits_HandleSubscenesByNpcAction(this, play);
}

static void DemoDu_UpdateCs_CR_03(DemoDu* this, PlayState* play) {
    DemoDu_UpdateBgCheckInfo(this, play);
    DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsCredits_HandleSubscenesByNpcAction(this, play);
}

static void DemoDu_UpdateCs_CR_04(DemoDu* this, PlayState* play) {
    s32 animFinished;

    DemoDu_UpdateBgCheckInfo(this, play);
    animFinished = DemoDu_UpdateSkelAnime(this);
    DemoDu_UpdateEyes(this);
    DemoDu_CsCredits_BackTo02(this, animFinished);
}

static DemoDuActionFunc sUpdateFuncs[] = {
    DemoDu_UpdateCs_FM_00, DemoDu_UpdateCs_FM_01, DemoDu_UpdateCs_FM_02, DemoDu_UpdateCs_FM_03, DemoDu_UpdateCs_FM_04,
    DemoDu_UpdateCs_FM_05, DemoDu_UpdateCs_FM_06, DemoDu_UpdateCs_GR_00, DemoDu_UpdateCs_GR_01, DemoDu_UpdateCs_GR_02,
    DemoDu_UpdateCs_GR_03, DemoDu_UpdateCs_GR_04, DemoDu_UpdateCs_GR_05, DemoDu_UpdateCs_GR_06, DemoDu_UpdateCs_GR_07,
    DemoDu_UpdateCs_GR_08, DemoDu_UpdateCs_GR_09, DemoDu_UpdateCs_GR_10, DemoDu_UpdateCs_GR_11, DemoDu_UpdateCs_GR_12,
    DemoDu_UpdateCs_GR_13, DemoDu_UpdateCs_AG_00, DemoDu_UpdateCs_AG_01, DemoDu_UpdateCs_AG_02, DemoDu_UpdateCs_CR_00,
    DemoDu_UpdateCs_CR_01, DemoDu_UpdateCs_CR_02, DemoDu_UpdateCs_CR_03, DemoDu_UpdateCs_CR_04,
};

static void DemoDu_Update(Actor* thisx, PlayState* play) {
    DemoDu* this = (DemoDu*)thisx;

    if (this->updateIndex < 0 || this->updateIndex >= 29 || sUpdateFuncs[this->updateIndex] == NULL) {
        return;
    }
    sUpdateFuncs[this->updateIndex](this, play);
}

static void DemoDu_Init(Actor* thisx, PlayState* play) {
    DemoDu* this = (DemoDu*)thisx;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 30.0f);
    switch (this->actor.params) {
        case DEMO_DU_CS_GORONS_RUBY:
            DemoDu_InitCs_GoronsRuby(this, play);
            break;

        case DEMO_DU_CS_CHAMBER_AFTER_GANON:
            DemoDu_InitCs_AfterGanon(this, play);
            break;

        case DEMO_DU_CS_CREDITS:
            DemoDu_InitCs_Credits(this, play);
            break;

        default:
            DemoDu_InitCs_FireMedallion(this, play);
            break;
    }
}

static void DemoDu_Draw_NoDraw(Actor* thisx, PlayState* play2) {
}

// Similar to DemoDu_Draw_02, but this uses POLY_OPA_DISP. Sets the env color to 255.
static void DemoDu_Draw_01(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    DemoDu* this = (DemoDu*)thisx;
    s16 eyeTexIndex = this->eyeTexIndex;
    void* eyeTexture = sEyeTextures[eyeTexIndex];
    s32 pad;
    s16 mouthTexIndex = this->mouthTexIndex;
    void* mouthTexture = sMouthTextures[mouthTexIndex];
    SkelAnime* skelAnime = &this->skelAnime;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTexture));
    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(mouthTexture));
    gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(gDaruniaNoseSeriousTex));

    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

    gSPSegment(POLY_OPA_DISP++, 0x0C, &D_80116280_Oot[2]);

    SkelAnime_DrawSkeletonOpaOot(play, skelAnime, NULL, NULL, this);

    CLOSE_DISPS(play->state.gfxCtx);
}

static DemoDuDrawFunc sDrawFuncs[] = {
    DemoDu_Draw_NoDraw,
    DemoDu_Draw_01,
    DemoDu_Draw_02,
};

static void DemoDu_Draw(Actor* thisx, PlayState* play) {
    DemoDu* this = (DemoDu*)thisx;

    if (this->drawIndex < 0 || this->drawIndex >= 3 || sDrawFuncs[this->drawIndex] == NULL) {
        return;
    }
    sDrawFuncs[this->drawIndex](thisx, play);
}

ActorProfile Demo_Du_Oot_Profile = {
    /**/ ACTOR_DEMO_DU_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_DU_Oot,
    /**/ sizeof(DemoDu),
    /**/ DemoDu_Init,
    /**/ DemoDu_Destroy,
    /**/ DemoDu_Update,
    /**/ DemoDu_Draw,
    /**/ DemoDu_Reset,
};

static void DemoDu_Reset(void) {
    D_8096CE94 = 0;
}
