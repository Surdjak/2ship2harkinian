/*
 * File: z_bg_dy_yoseizo.c
 * Overlay: ovl_Bg_Dy_Yoseizo
 * Description: Great Fairy
 */

#include "z_bg_dy_yoseizo.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "HyruleCutscene.h"
#include "assets/oot/objects/object_dy_obj/object_dy_obj.h"
#include "../ovl_Demo_Effect/z_demo_effect.h"
#include "assets/oot/scenes/indoors/yousei_izumi_yoko/yousei_izumi_yoko_scene.h"
#include "assets/oot/scenes/indoors/daiyousei_izumi/daiyousei_izumi_scene.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

typedef enum {
    /* 0 */ FAIRY_UPGRADE_MAGIC,
    /* 1 */ FAIRY_UPGRADE_DOUBLE_MAGIC,
    /* 2 */ FAIRY_UPGRADE_HALF_DAMAGE
} BgDyYoseizoRewardType;

typedef enum {
    /* 0 */ FAIRY_SPELL_FARORES_WIND,
    /* 1 */ FAIRY_SPELL_DINS_FIRE,
    /* 2 */ FAIRY_SPELL_NAYRUS_LOVE
} BgDyYoseizoSpellType;

static void BgDyYoseizo_Init(Actor* thisx, PlayState* play);
static void BgDyYoseizo_Destroy(Actor* thisx, PlayState* play);
static void BgDyYoseizo_Update(Actor* thisx, PlayState* play);
static void BgDyYoseizo_Draw(Actor* thisx, PlayState* play);

static void BgDyYoseizo_CheckMagicAcquired(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_ChooseType(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SetupSpinGrow_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SpinGrow_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_CompleteSpinGrow_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SetupGreetPlayer_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_GreetPlayer_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SetupHealPlayer_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_HealPlayer_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SayFarewell_NoReward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SetupSpinShrink(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SpinShrink(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_Vanish(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SetupSpinGrow_Reward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_SpinGrowSetupGive_Reward(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_Give_Reward(BgDyYoseizo* this, PlayState* play);

static void BgDyYoseizo_ParticleInit(BgDyYoseizo* this, Vec3f* initPos, Vec3f* initVelocity, Vec3f* accel,
                              Color_RGB8* primColor, Color_RGB8* envColor, f32 scale, s16 life, s16 type);
static void BgDyYoseizo_ParticleUpdate(BgDyYoseizo* this, PlayState* play);
static void BgDyYoseizo_ParticleDraw(BgDyYoseizo* this, PlayState* play);

static s32 sUnusedGetItemIds[] = { GI_FARORES_WIND_Oot, GI_NAYRUS_LOVE_Oot, GI_DINS_FIRE_Oot };

ActorProfile Bg_Dy_Yoseizo_Oot_Profile = {
    /**/ ACTOR_BG_DY_YOSEIZO_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_DY_OBJ_Oot,
    /**/ sizeof(BgDyYoseizo),
    /**/ BgDyYoseizo_Init,
    /**/ BgDyYoseizo_Destroy,
    /**/ BgDyYoseizo_Update,
    /**/ NULL,
};

static void BgDyYoseizo_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BgDyYoseizo* this = (BgDyYoseizo*)thisx;

    this->fountainType = play->curSpawn;

    if (this->fountainType < 0) {
        this->fountainType = 0;
    }

    this->vanishHeight = this->actor.world.pos.y;
    this->grownHeight = this->vanishHeight + 40.0f;
    this->actor.focus.pos = this->actor.world.pos;

    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        SkelAnime_InitFlex(play, &this->skelAnime, &gGreatFairySkel, &gGreatFairySittingTransitionAnim,
                           this->jointTable, this->morphTable, 28);
    } else {
        SkelAnime_InitFlex(play, &this->skelAnime, &gGreatFairySkel, &gGreatFairyLayingDownTransitionAnim,
                           this->jointTable, this->morphTable, 28);
    }
    this->actionFunc = BgDyYoseizo_CheckMagicAcquired;
}

static void BgDyYoseizo_Destroy(Actor* thisx, PlayState* play) {
}

static Color_RGB8 sParticlePrimColors[] = {
    { 255, 255, 255 }, { 255, 255, 100 }, { 100, 255, 100 }, { 255, 100, 100 }, { 255, 255, 170 },
    { 255, 255, 100 }, { 100, 255, 100 }, { 255, 100, 100 }, { 255, 255, 170 },
};

static Color_RGB8 sParticleEnvColors[] = {
    { 155, 255, 255 }, { 255, 255, 100 }, { 100, 255, 100 }, { 255, 100, 100 }, { 255, 100, 255 },
    { 255, 255, 100 }, { 100, 255, 100 }, { 255, 100, 100 }, { 100, 255, 255 },
};

static void BgDyYoseizo_SpawnParticles(BgDyYoseizo* this, PlayState* play, s16 type) {
    Vec3f particleInitVelocity = { 0.0f, 0.0f, 0.0f };
    Vec3f particleAccel;
    Vec3f particleInitPos;
    Color_RGB8 particlePrimColor;
    Color_RGB8 particleEnvColor;
    f32 spawnPosVariation;
    s32 particleType;
    f32 particleScale;
    s32 i;
    s16 particleLife;

    if (!(this->scale < 0.01f)) {
        spawnPosVariation = this->scale * 3500.0f;
        particleAccel.x = Rand_ZeroOne() - 0.5f;
        particleAccel.y = Rand_ZeroOne() - 0.5f;
        particleAccel.z = Rand_ZeroOne() - 0.5f;
        for (i = 0; i < 2; i++) {
            if (type == 0) {
                particleType = 0;
                particleScale = 0.4f;
                particleLife = 90;
                particleInitPos.x = this->actor.world.pos.x;
                particleInitPos.y = this->actor.world.pos.y + spawnPosVariation +
                                    ((Rand_ZeroOne() - 0.5f) * (spawnPosVariation * 0.5f));
                particleInitPos.z = this->actor.world.pos.z + 30.0f;
            } else {
                particleLife = 50;
                particleType = type;
                particleScale = 0.2f;
                particleInitPos.x = this->actor.world.pos.x + Rand_CenteredFloat(10.0f);

                if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
                    particleInitPos.y = this->actor.world.pos.y + spawnPosVariation + 50.0f +
                                        ((Rand_ZeroOne() - 0.5f) * (spawnPosVariation * 0.1f));
                    particleInitPos.z = this->actor.world.pos.z + 30.0f;
                } else {
                    particleInitPos.y = this->actor.world.pos.y + spawnPosVariation - 30.0f +
                                        ((Rand_ZeroOne() - 0.5f) * (spawnPosVariation * 0.1f));
                    particleInitPos.z = this->actor.world.pos.z + 60.0f;
                }

                if (LINK_IS_ADULT_OOT) {
                    particleInitPos.y += 20.0f;
                }
            }

            particlePrimColor.r = sParticlePrimColors[particleType].r;
            particlePrimColor.g = sParticlePrimColors[particleType].g;
            particlePrimColor.b = sParticlePrimColors[particleType].b;
            particleEnvColor.r = sParticleEnvColors[particleType].r;
            particleEnvColor.g = sParticleEnvColors[particleType].g;
            particleEnvColor.b = sParticleEnvColors[particleType].b;
            BgDyYoseizo_ParticleInit(this, &particleInitPos, &particleInitVelocity, &particleAccel, &particlePrimColor,
                                     &particleEnvColor, particleScale, particleLife, particleType);
        }
    }
}

static void BgDyYoseizo_Bob(BgDyYoseizo* this, PlayState* play) {
    this->targetHeight = this->grownHeight + this->bobOffset;
    Math_ApproachF(&this->actor.world.pos.y, this->targetHeight, 0.1f, 10.0f);
    Math_ApproachF(&this->bobOffset, 10.0f, 0.1f, 0.5f);

    if (gCsCtxOot.state == CS_STATE_IDLE_Oot) {
        this->actor.velocity.y = Math_SinS(this->bobTimer);
    } else {
        this->actor.velocity.y = Math_SinS(this->bobTimer) * 0.4f;
    }
}

static void BgDyYoseizo_CheckMagicAcquired(BgDyYoseizo* this, PlayState* play) {
    u8 isEligible = true;
    if (Flags_GetSwitch(play, 0x38)) {
        Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);

        if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
            if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired && (this->fountainType != FAIRY_UPGRADE_MAGIC)) {
                isEligible = false;
            }
        } else {
            if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired) {
                isEligible = false;
            }
        }

        if (!(isEligible)) {
            Actor_Kill(&this->actor);
            return;
        }

        Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
        this->actionFunc = BgDyYoseizo_ChooseType;
    }
}

static void BgDyYoseizo_ChooseType(BgDyYoseizo* this, PlayState* play) {
    s32 givingReward;

    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
    givingReward = false;

    if (play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        switch (this->fountainType) {
            case FAIRY_SPELL_FARORES_WIND:
                if (!GET_ITEMGETINF_OOT(ITEMGETINF_18_Oot)) {
                    givingReward = true;
                }
                break;
            case FAIRY_SPELL_DINS_FIRE:
                if (!GET_ITEMGETINF_OOT(ITEMGETINF_19_Oot)) {
                    givingReward = true;
                }
                break;
            case FAIRY_SPELL_NAYRUS_LOVE:
                if (!GET_ITEMGETINF_OOT(ITEMGETINF_1A_Oot)) {
                    givingReward = true;
                }
                break;
        }
    } else {
        switch (this->fountainType) {
            case FAIRY_UPGRADE_MAGIC:
                if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired || BREG(2)) {
                    this->givingSpell = true;
                    givingReward = true;
                }
                break;
            case FAIRY_UPGRADE_DOUBLE_MAGIC:
                if (!gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired) {
                    this->givingSpell = true;
                    givingReward = true;
                }
                break;
            case FAIRY_UPGRADE_HALF_DAMAGE:
                if (!gSaveContext.save.saveInfo.playerData.doubleDefense) {
                    this->givingSpell = true;
                    givingReward = true;
                }
                break;
        }
    }

    if (givingReward) {
        if (gSaveContext.sceneLayer < 4) {
            if (play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
                switch (this->fountainType) {
                    case FAIRY_SPELL_FARORES_WIND:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyFaroresWindCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                    case FAIRY_SPELL_DINS_FIRE:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyDinsFireCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                    case FAIRY_SPELL_NAYRUS_LOVE:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyNayrusLoveCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                }
            } else {
                switch (this->fountainType) {
                    case FAIRY_UPGRADE_MAGIC:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyMagicCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                    case FAIRY_UPGRADE_DOUBLE_MAGIC:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyDoubleMagicCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                    case FAIRY_UPGRADE_HALF_DAMAGE:
                        gCsCtxOot.segment = Lib_SegmentedToVirtual(gGreatFairyDoubleDefenseCs);
                        Hyrule_TriggerCutsceneOot(1);
                        break;
                }
            }
        }
        this->actionFunc = BgDyYoseizo_SetupSpinGrow_Reward;
        return;
    }

    play->envCtx.lightSettingOverride = 2;

    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        OnePointCutscene_Init(play, 8603, -99, NULL, CAM_ID_MAIN);
    } else {
        OnePointCutscene_Init(play, 8604, -99, NULL, CAM_ID_MAIN);
    };

    Actor_PlaySfx(&this->actor, NA_SE_EV_GREAT_FAIRY_APPEAR);
    this->actor.draw = BgDyYoseizo_Draw;
    this->actionFunc = BgDyYoseizo_SetupSpinGrow_NoReward;
}

// Sets animations for spingrow
static void BgDyYoseizo_SetupSpinGrow_NoReward(BgDyYoseizo* this, PlayState* play) {
    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        this->frameCount = Animation_GetLastFrame(&gGreatFairySittingTransitionAnim);
        Animation_Change(&this->skelAnime, &gGreatFairySittingTransitionAnim, 1.0f, 0.0f, this->frameCount,
                         ANIMMODE_ONCE, -10.0f);
    } else {
        this->frameCount = Animation_GetLastFrame(&gGreatFairyLayingDownTransitionAnim);
        Animation_Change(&this->skelAnime, &gGreatFairyLayingDownTransitionAnim, 1.0f, 0.0f, this->frameCount,
                         ANIMMODE_ONCE, -10.0f);
    }

    Actor_PlaySfx(&this->actor, NA_SE_VO_FR_LAUGH_0);
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
    this->actionFunc = BgDyYoseizo_SpinGrow_NoReward;
}

static void BgDyYoseizo_SpinGrow_NoReward(BgDyYoseizo* this, PlayState* play) {
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
    Math_ApproachF(&this->actor.world.pos.y, this->grownHeight, this->heightFraction, 100.0f);
    Math_ApproachF(&this->scale, 0.035f, this->scaleFraction, 0.005f);
    Math_ApproachF(&this->heightFraction, 0.8f, 0.1f, 0.02f);
    Math_ApproachF(&this->scaleFraction, 0.2f, 0.03f, 0.05f);
    // Finished growing
    if (this->scale >= 0.034f) {
        if ((this->actor.shape.rot.y > -8000) && (this->actor.shape.rot.y < 1000)) {
            SkelAnime_Update(&this->skelAnime);
            // Turn to front
            Math_SmoothStepToS(&this->actor.shape.rot.y, 0, 5, 1000, 0);
            if (fabsf(this->actor.shape.rot.y) < 50.0f) {
                this->actionFunc = BgDyYoseizo_CompleteSpinGrow_NoReward;
            }
        } else {
            this->actor.shape.rot.y += 3000;
        }
    } else {
        this->actor.shape.rot.y += 3000;
    }
    BgDyYoseizo_SpawnParticles(this, play, 0);
}

static void BgDyYoseizo_CompleteSpinGrow_NoReward(BgDyYoseizo* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);

    if ((this->frameCount * 1273.0f) <= this->bobTimer) {
        this->bobTimer = 0.0f;
    }

    SkelAnime_Update(&this->skelAnime);

    if ((this->frameCount <= curFrame) && !this->animationChanged) {
        this->actionFunc = BgDyYoseizo_SetupGreetPlayer_NoReward;
    }
}

static void BgDyYoseizo_SetupGreetPlayer_NoReward(BgDyYoseizo* this, PlayState* play) {
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);

    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        this->frameCount = Animation_GetLastFrame(&gGreatFairySittingAnim);
        Animation_Change(&this->skelAnime, &gGreatFairySittingAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_LOOP,
                         -10.0f);
    } else {
        this->frameCount = Animation_GetLastFrame(&gGreatFairyLayingSidewaysAnim);
        Animation_Change(&this->skelAnime, &gGreatFairyLayingSidewaysAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_LOOP,
                         -10.0f);
    }

    this->actor.textId = TEXT_OOT(0xDB);
    this->dialogState = TEXT_STATE_EVENT;
    Message_StartTextbox(play, this->actor.textId, NULL);
    BgDyYoseizo_SpawnParticles(this, play, 0);
    this->actionFunc = BgDyYoseizo_GreetPlayer_NoReward;
}

static void BgDyYoseizo_GreetPlayer_NoReward(BgDyYoseizo* this, PlayState* play) {
    Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
    this->bobTimer = this->skelAnime.curFrame * 1273.0f;

    if ((this->frameCount * 1273.0f) <= this->bobTimer) {
        this->bobTimer = 0.0f;
    }

    SkelAnime_Update(&this->skelAnime);

    if ((this->dialogState == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        Interface_SetHudVisibility(5);
        this->actionFunc = BgDyYoseizo_SetupHealPlayer_NoReward;
    }

    BgDyYoseizo_Bob(this, play);
    BgDyYoseizo_SpawnParticles(this, play, 0);
}

static void BgDyYoseizo_SetupHealPlayer_NoReward(BgDyYoseizo* this, PlayState* play) {
    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        this->frameCount = Animation_GetLastFrame(&gGreatFairyGivingUpgradeAnim);
        Animation_Change(&this->skelAnime, &gGreatFairyGivingUpgradeAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_ONCE,
                         -10.0f);
    } else {
        this->frameCount = Animation_GetLastFrame(&gGreatFairyAnim_005810);
        Animation_Change(&this->skelAnime, &gGreatFairyAnim_005810, 1.0f, 0.0f, this->frameCount, ANIMMODE_ONCE,
                         -10.0f);
    }

    Actor_PlaySfx(&this->actor, NA_SE_VO_FR_SMILE_0);
    this->mouthState = 1;
    this->actionFunc = BgDyYoseizo_HealPlayer_NoReward;
}

static void BgDyYoseizo_HealPlayer_NoReward(BgDyYoseizo* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 curFrame = this->skelAnime.curFrame;
    Vec3f beamPos;
    s16 beamParams;

    if (this->animationChanged) {
        this->bobTimer = this->skelAnime.curFrame * 1300.0f;
        if ((this->frameCount * 1300.0f) <= this->bobTimer) {
            this->bobTimer = 0.0f;
        }
    }

    SkelAnime_Update(&this->skelAnime);
    if ((this->frameCount <= curFrame) && !(this->animationChanged)) {
        if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
            this->frameCount = Animation_GetLastFrame(&gGreatFairyAfterUpgradeAnim);
            Animation_Change(&this->skelAnime, &gGreatFairyAfterUpgradeAnim, 1.0f, 0.0f, this->frameCount,
                             ANIMMODE_LOOP, -10.0f);
        } else {
            this->frameCount = Animation_GetLastFrame(&gGreatFairyAfterSpellAnim);
            Animation_Change(&this->skelAnime, &gGreatFairyAfterSpellAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_LOOP,
                             -10.0f);
        }
        this->healingTimer = 150;
        this->animationChanged = true;
        if (!this->givingSpell) {
            beamPos.x = player->actor.world.pos.x;
            beamPos.y = player->actor.world.pos.y + 200.0f;
            beamPos.z = player->actor.world.pos.z;

            beamParams = ((play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) ? 0 : 1);

            this->beam = (EnDyExtra*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_DY_EXTRA_Oot,
                                                        beamPos.x, beamPos.y, beamPos.z, 0, 0, 0, beamParams);
        }
    }
    if (this->refillTimer > 1) {
        this->refillTimer--;
    }

    if (this->healingTimer >= 110) {
        this->healingTimer--;
    }

    if (this->healingTimer == 110) {
        gSaveContext.healthAccumulator = (20 * 0x10);
        Magic_Add(play, MAGIC_FILL_TO_CAPACITY);
        this->refillTimer = 200;
    }

    if (((gSaveContext.save.saveInfo.playerData.healthCapacity == gSaveContext.save.saveInfo.playerData.health) && (gSaveContext.save.saveInfo.playerData.magic == gSaveContext.magicCapacity)) ||
        (this->refillTimer == 1)) {
        this->healingTimer--;
        if (this->healingTimer == 90) {
            if (!this->givingSpell) {
                this->beam->trigger = 1;
            }
            this->givingSpell = false;
        }
    }

    if (this->healingTimer == 1) {
        this->actor.textId = TEXT_OOT(0xDA);
        this->dialogState = TEXT_STATE_EVENT;
        Message_ContinueTextbox(play, this->actor.textId);
        this->actionFunc = BgDyYoseizo_SayFarewell_NoReward;
        return;
    }
    BgDyYoseizo_Bob(this, play);
}

static void BgDyYoseizo_SayFarewell_NoReward(BgDyYoseizo* this, PlayState* play) {
    this->bobTimer = this->skelAnime.curFrame * 1400.0f;

    if (this->bobTimer >= (this->frameCount * 1400.0f)) {
        this->bobTimer = 0.0f;
    }

    SkelAnime_Update(&this->skelAnime);

    if ((this->dialogState == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        this->mouthState = 0;
        this->actionFunc = BgDyYoseizo_SetupSpinShrink;
        Camera_SetFinishedFlag(GET_ACTIVE_CAM(play));
    }

    BgDyYoseizo_Bob(this, play);
    BgDyYoseizo_SpawnParticles(this, play, 0);
}

static void BgDyYoseizo_SetupSpinShrink(BgDyYoseizo* this, PlayState* play) {
    if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
        this->frameCount = Animation_GetLastFrame(&gGreatFairyJewelFountainSpinShrinkAnim);
        Animation_Change(&this->skelAnime, &gGreatFairyJewelFountainSpinShrinkAnim, 1.0f, 0.0f, this->frameCount,
                         ANIMMODE_ONCE, -10.0f);
    } else {
        this->frameCount = Animation_GetLastFrame(&gGreatFairySpellFountainSpinShrinkAnim);
        Animation_Change(&this->skelAnime, &gGreatFairySpellFountainSpinShrinkAnim, 1.0f, 0.0f, this->frameCount,
                         ANIMMODE_ONCE, -10.0f);
    }

    this->vanishTimer = 5;
    this->scaleFraction = 0.0f;
    this->heightFraction = 0.0f;
    Actor_PlaySfx(&this->actor, NA_SE_VO_FR_LAUGH_0);
    Actor_PlaySfx(&this->actor, NA_SE_EV_GREAT_FAIRY_VANISH);
    this->actionFunc = BgDyYoseizo_SpinShrink;
}

static void BgDyYoseizo_SpinShrink(BgDyYoseizo* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (this->vanishTimer == 0) {
        if (this->scale < 0.003f) {
            this->vanishTimer = 30;
            this->actionFunc = BgDyYoseizo_Vanish;
        } else {
            Math_ApproachF(&this->actor.world.pos.y, this->vanishHeight, this->heightFraction, 100.0f);
            Math_ApproachZeroF(&this->scale, this->scaleFraction, 0.005f);
            Math_ApproachF(&this->heightFraction, 0.8f, 0.1f, 0.02f);
            Math_ApproachF(&this->scaleFraction, 0.2f, 0.03f, 0.05f);
            this->actor.shape.rot.y += 3000;
            BgDyYoseizo_SpawnParticles(this, play, 0);
        }
    }
}

static void BgDyYoseizo_Vanish(BgDyYoseizo* this, PlayState* play) {
    Actor* findOcarinaSpot;

    if (this->vanishTimer == 0) {
        Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 7);
        play->envCtx.lightSettingOverride = 0;
        findOcarinaSpot = play->actorCtx.actorLists[ACTORCAT_PROP].first;

        while (findOcarinaSpot != NULL) {
            if (findOcarinaSpot->id != ACTOR_EN_OKARINA_TAG_Oot) {
                findOcarinaSpot = findOcarinaSpot->next;
                continue;
            }
            Actor_Kill(findOcarinaSpot);
            break;
        }

        Flags_UnsetSwitch(play, 0x38);
        Actor_Kill(&this->actor);
    }
}

static void BgDyYoseizo_SetupSpinGrow_Reward(BgDyYoseizo* this, PlayState* play) {
    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        if ((gCsCtxOot.npcActions[0] != NULL) && (gCsCtxOot.npcActions[0]->action == 2)) {
            this->actor.draw = BgDyYoseizo_Draw;
            Player_SetCsActionWithHaltedActorsOot(play, &this->actor, 1);
            this->finishedSpinGrow = false;

            if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
                this->frameCount = Animation_GetLastFrame(&gGreatFairySittingTransitionAnim);
                Animation_Change(&this->skelAnime, &gGreatFairySittingTransitionAnim, 1.0f, 0.0f, this->frameCount,
                                 ANIMMODE_ONCE, -10.0f);
            } else {
                this->frameCount = Animation_GetLastFrame(&gGreatFairyLayingDownTransitionAnim);
                Animation_Change(&this->skelAnime, &gGreatFairyLayingDownTransitionAnim, 1.0f, 0.0f, this->frameCount,
                                 ANIMMODE_ONCE, -10.0f);
            }

            Actor_PlaySfx(&this->actor, NA_SE_EV_GREAT_FAIRY_APPEAR);
            this->actionFunc = BgDyYoseizo_SpinGrowSetupGive_Reward;
        }
    }
}

static void BgDyYoseizo_SpinGrowSetupGive_Reward(BgDyYoseizo* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    if (!this->finishedSpinGrow) {
        Math_ApproachF(&this->actor.world.pos.y, this->grownHeight, this->heightFraction, 100.0f);
        Math_ApproachF(&this->scale, 0.035f, this->scaleFraction, 0.005f);
        Math_ApproachF(&this->heightFraction, 0.8f, 0.1f, 0.02f);
        Math_ApproachF(&this->scaleFraction, 0.2f, 0.03f, 0.05f);
        // Finished growing
        if (this->scale >= 0.034f) {
            if ((this->actor.shape.rot.y > -8000) && (this->actor.shape.rot.y < 1000)) {
                SkelAnime_Update(&this->skelAnime);
                // Spin until facing front
                Math_ApproachS(&this->actor.shape.rot.y, 0, 5, 1000);
                if (fabsf(this->actor.shape.rot.y) < 50.0f) {
                    this->finishedSpinGrow = true;
                }
            } else {
                this->actor.shape.rot.y += 3000;
            }
        } else {
            this->actor.shape.rot.y += 3000;
        }
    } else {
        SkelAnime_Update(&this->skelAnime);

        if ((this->frameCount <= curFrame) && !this->animationChanged) {
            if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
                this->frameCount = Animation_GetLastFrame(&gGreatFairySittingAnim);
                Animation_Change(&this->skelAnime, &gGreatFairySittingAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_LOOP,
                                 -10.0f);
            } else {
                this->frameCount = Animation_GetLastFrame(&gGreatFairyLayingSidewaysAnim);
                Animation_Change(&this->skelAnime, &gGreatFairyLayingSidewaysAnim, 1.0f, 0.0f, this->frameCount,
                                 ANIMMODE_LOOP, -10.0f);
            }
            this->animationChanged = true;
        }

        if ((gCsCtxOot.state != CS_STATE_IDLE_Oot) &&
            ((gCsCtxOot.npcActions[0] != NULL) && (gCsCtxOot.npcActions[0]->action == 3))) {
            this->finishedSpinGrow = this->animationChanged = false;
            if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
                this->frameCount = Animation_GetLastFrame(&gGreatFairyGivingUpgradeAnim);
                Animation_Change(&this->skelAnime, &gGreatFairyGivingUpgradeAnim, 1.0f, 0.0f, this->frameCount,
                                 ANIMMODE_ONCE, -10.0f);
            } else {
                this->frameCount = Animation_GetLastFrame(&gGreatFairyAnim_005810);
                Animation_Change(&this->skelAnime, &gGreatFairyAnim_005810, 1.0f, 0.0f, this->frameCount, ANIMMODE_ONCE,
                                 -10.0f);
            }
            this->mouthState = 1;
            this->actionFunc = BgDyYoseizo_Give_Reward;
        }
    }
    BgDyYoseizo_SpawnParticles(this, play, 0);
}

static s16 sDemoEffectLightColors[] = { DEMO_EFFECT_LIGHT_GREEN, DEMO_EFFECT_LIGHT_RED, DEMO_EFFECT_LIGHT_BLUE };

static s16 sExItemTypes[] = { EXITEM_MAGIC_WIND, EXITEM_MAGIC_FIRE, EXITEM_MAGIC_DARK };

static s16 sItemGetFlags[] = { 0x100, 0x200, 0x400 };

static u8 sItemIds[] = { ITEM_FARORES_WIND_Oot, ITEM_DINS_FIRE_Oot, ITEM_NAYRUS_LOVE_Oot };

static void BgDyYoseizo_Give_Reward(BgDyYoseizo* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;
    Player* player = GET_PLAYER(play);
    s16 actionIndex;
    s16 demoEffectParams;
    Vec3f itemPos;

    if (this->animationChanged) {
        this->bobTimer = this->skelAnime.curFrame * 1400.0f;
        if ((this->frameCount * 1400.0f) <= this->bobTimer) {
            this->bobTimer = 0.0f;
        }
    }
    SkelAnime_Update(&this->skelAnime);

    if ((this->frameCount <= curFrame) && !this->animationChanged) {
        if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
            this->frameCount = Animation_GetLastFrame(&gGreatFairyAfterUpgradeAnim);
            Animation_Change(&this->skelAnime, &gGreatFairyAfterUpgradeAnim, 1.0f, 0.0f, this->frameCount,
                             ANIMMODE_LOOP, -10.0f);
        } else {
            this->frameCount = Animation_GetLastFrame(&gGreatFairyAfterSpellAnim);
            Animation_Change(&this->skelAnime, &gGreatFairyAfterSpellAnim, 1.0f, 0.0f, this->frameCount, ANIMMODE_LOOP,
                             -10.0f);
        }
        this->animationChanged = true;
    }

    if (gCsCtxOot.npcActions[0]->action == 13) {
        this->actionFunc = BgDyYoseizo_SetupSpinShrink;
        return;
    }

    if ((gCsCtxOot.npcActions[0]->action >= 4) && (gCsCtxOot.npcActions[0]->action < 7)) {
        actionIndex = gCsCtxOot.npcActions[0]->action - 4;
        if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
            actionIndex++;
            BgDyYoseizo_SpawnParticles(this, play, actionIndex);

        } else if (!this->lightBallSpawned) {
            demoEffectParams = ((s16)(sDemoEffectLightColors[actionIndex] << 0xC) | DEMO_EFFECT_LIGHT);
            Actor_Spawn(&play->actorCtx, play, ACTOR_DEMO_EFFECT_Oot, this->actor.world.pos.x, this->actor.world.pos.y,
                        this->actor.world.pos.z, 0, 0, 0, (s32)demoEffectParams);
            this->lightBallSpawned = true;
        }
    } else {
        BgDyYoseizo_SpawnParticles(this, play, 0);
    }

    if ((play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) && (gCsCtxOot.npcActions[0]->action >= 10) &&
        (gCsCtxOot.npcActions[0]->action < 13)) {
        actionIndex = gCsCtxOot.npcActions[0]->action - 10;

        switch (actionIndex) {
            case FAIRY_UPGRADE_MAGIC:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                gSaveContext.magicFillTarget = MAGIC_NORMAL_METER;
                Interface_SetHudVisibility(9);
                break;
            case FAIRY_UPGRADE_DOUBLE_MAGIC:
                if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired) {
                    gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                }
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
                gSaveContext.magicFillTarget = MAGIC_DOUBLE_METER;
                gSaveContext.save.saveInfo.playerData.magicLevel = 0;
                Interface_SetHudVisibility(9);
                break;
            case FAIRY_UPGRADE_HALF_DAMAGE:
                gSaveContext.save.saveInfo.playerData.doubleDefense = true;
                Interface_SetHudVisibility(9);
                break;
        }

        if (!this->healing) {
            gSaveContext.healthAccumulator = (20 * 0x10);
            this->healing = true;
            if (actionIndex == 2) {
                Magic_Add(play, MAGIC_FILL_TO_CAPACITY);
            }
        }
    }

    if ((play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) && (gCsCtxOot.npcActions[0]->action >= 14) &&
        (gCsCtxOot.npcActions[0]->action < 17)) {
        actionIndex = gCsCtxOot.npcActions[0]->action - 14;

        if (!this->itemSpawned) {
            itemPos.x = player->actor.world.pos.x;
            itemPos.y = (LINK_IS_ADULT_OOT ? player->actor.world.pos.y + 73.0f : player->actor.world.pos.y + 53.0f);
            itemPos.z = player->actor.world.pos.z;

            this->item = (EnExItem*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_EX_ITEM_Oot, itemPos.x,
                                                       itemPos.y, itemPos.z, 0, 0, 0, sExItemTypes[actionIndex]);

            if (this->item != NULL) {
                if (gSaveContext.save.saveInfo.playerData.isMagicAcquired == 0) {
                    gSaveContext.save.saveInfo.playerData.isMagicAcquired = 1;
                } else {
                    Magic_Add(play, MAGIC_FILL_TO_CAPACITY);
                }

                this->itemSpawned = true;
                gSaveContext.healthAccumulator = (20 * 0x10);
                Interface_SetHudVisibility(9);
                gSaveContext.save.saveInfo.itemGetInfOot[1] |= sItemGetFlags[actionIndex];
                Item_Give(play, sItemIds[actionIndex]);
            }
        } else {
            this->item->actor.world.pos.x = player->actor.world.pos.x;
            this->item->actor.world.pos.y =
                (LINK_IS_ADULT_OOT ? player->actor.world.pos.y + 73.0f : player->actor.world.pos.y + 53.0f);
            this->item->actor.world.pos.z = player->actor.world.pos.z;
            this->item->scale = 0.3f;
        }
    }

    if ((play->sceneId != SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) && (gCsCtxOot.npcActions[0]->action == 17) &&
        (this->item != NULL)) {
        Actor_Kill(&this->item->actor);
        this->item = NULL;
    }

    if ((play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) && (gCsCtxOot.npcActions[0]->action == 18)) {
        this->giveDefenseHearts = true;
    }

    if (this->giveDefenseHearts) {
        if (gSaveContext.save.saveInfo.inventory.defenseHearts < 20) {
            gSaveContext.save.saveInfo.inventory.defenseHearts++;
        }
    }

    if ((gCsCtxOot.npcActions[0]->action >= 19) && (gCsCtxOot.npcActions[0]->action < 22) &&
        !this->warpEffectSpawned) {
        actionIndex = gCsCtxOot.npcActions[0]->action - 11;
        Actor_Spawn(&play->actorCtx, play, ACTOR_DOOR_WARP1_Oot, player->actor.world.pos.x, player->actor.world.pos.y,
                    player->actor.world.pos.z, 0, 0, 0, actionIndex);
        this->warpEffectSpawned = true;
    }
    BgDyYoseizo_Bob(this, play);
}

static void BgDyYoseizo_Update(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BgDyYoseizo* this = (BgDyYoseizo*)thisx;
    s32 phi_v1;

    this->absoluteTimer++;

    if (this->vanishTimer != 0) {
        this->vanishTimer--;
    }
    if (this->blinkTimer != 0) {
        this->blinkTimer--;
    }
    if (this->unusedTimer != 0) {
        this->unusedTimer--;
    }

    this->actionFunc(this, play);

    if (gCsCtxOot.state != CS_STATE_IDLE_Oot) {
        phi_v1 = 0;
        if (play->sceneId == SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC_Oot) {
            if ((gCsCtxOot.frames == 32) || (gCsCtxOot.frames == 291) || (gCsCtxOot.frames == 426) ||
                (gCsCtxOot.frames == 851)) {
                phi_v1 = 1;
            }
            if (gCsCtxOot.frames == 101) {
                phi_v1 = 2;
            }
        } else {
            if ((gCsCtxOot.frames == 35) || (gCsCtxOot.frames == 181) || (gCsCtxOot.frames == 462) ||
                (gCsCtxOot.frames == 795)) {
                phi_v1 = 1;
            }
            if (gCsCtxOot.frames == 90) {
                phi_v1 = 2;
            }
        }

        if (phi_v1 == 1) {
            Actor_PlaySfx(&this->actor, NA_SE_VO_FR_SMILE_0);
        }
        if (phi_v1 == 2) {
            Actor_PlaySfx(&this->actor, NA_SE_VO_FR_LAUGH_0);
        }
    }

    if ((this->blinkTimer == 0) && (this->actionFunc != BgDyYoseizo_HealPlayer_NoReward)) {
        this->eyeState++;
        this->eyeState2++;
        if (this->eyeState >= 3) {
            this->eyeState = this->eyeState2 = 0;
            this->blinkTimer = (s16)Rand_ZeroFloat(60.0f) + 20;
        }
    }

    Actor_MoveWithGravity(&this->actor);
    this->heightOffset = this->scale * 7500.0f;
    Actor_SetFocus(&this->actor, this->heightOffset);
    this->actor.focus.pos.y = this->heightOffset;
    Actor_TrackPlayer(play, &this->actor, &this->headRot, &this->torsoRot, this->actor.focus.pos);
    BgDyYoseizo_ParticleUpdate(this, play);
    Actor_SetScale(&this->actor, this->scale);
}

static s32 BgDyYoseizo_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BgDyYoseizo* this = (BgDyYoseizo*)thisx;

    if (limbIndex == 8) { // Torso
        rot->x += this->torsoRot.y;
    }
    if (limbIndex == 15) { // Head
        rot->x += this->headRot.y;
        rot->z += this->headRot.z;
    }
    return 0;
}

static void* sEyeTextures[] = {
    gGreatFairyEyeOpenTex,   // Open
    gGreatFairyEyeHalfTex,   // Half
    gGreatFairyEyeClosedTex, // Closed
};

static void* sMouthTextures[] = {
    gGreatFairyMouthClosedTex, // Closed
    gGreatFairyMouthOpenTex,   // Open
};

static void BgDyYoseizo_Draw(Actor* thisx, PlayState* play) {
    BgDyYoseizo* this = (BgDyYoseizo*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    if (this->actionFunc != BgDyYoseizo_Vanish) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);

        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeState]));

        // This was probably intended to allow this actor to wink, but segment 09 is not used in the dList for the head,
        // so it can only blink
        gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(sEyeTextures[this->eyeState2]));

        gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(sMouthTextures[this->mouthState]));

        SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, BgDyYoseizo_OverrideLimbDraw, NULL, this);
    }
    CLOSE_DISPS(play->state.gfxCtx);
    BgDyYoseizo_ParticleDraw(this, play);
}

static void BgDyYoseizo_ParticleInit(BgDyYoseizo* this, Vec3f* initPos, Vec3f* initVelocity, Vec3f* accel,
                              Color_RGB8* primColor, Color_RGB8* envColor, f32 scale, s16 life, s16 type) {
    BgDyYoseizoParticle* particle;
    s16 i;

    particle = this->particles;

    for (i = 0; i < 200; i++, particle++) {
        if (particle->alive == 0) {
            particle->alive = 1;
            particle->pos = *initPos;
            particle->velocity = *initVelocity;
            particle->accel = *accel;
            particle->primColor = *primColor;
            particle->alpha = 0;
            particle->envColor = *envColor;
            particle->scale = scale;
            particle->timer = life;
            particle->type = type;
            particle->pitch = 0.0f;
            particle->yaw = Rand_CenteredFloat(30000.0f);
            particle->roll = 0.0f;
            particle->epoch++;
            return;
        }
    }
}

static void BgDyYoseizo_ParticleUpdate(BgDyYoseizo* this, PlayState* play) {
    BgDyYoseizoParticle* particle = this->particles;
    Player* player = GET_PLAYER(play);
    Vec3f sp94;
    Vec3f sp88;
    f32 goalPitch;
    f32 goalYaw;
    s16 i = 0;

    for (i = 0; i < 200; i++, particle++) {
        if (particle->alive != 0) {
            particle->roll += 3000.0f;

            if (particle->type == 0) {
                particle->pos.x += particle->velocity.x;
                particle->pos.y += particle->velocity.y;
                particle->pos.z += particle->velocity.z;
                particle->velocity.x += particle->accel.x;
                particle->velocity.y += particle->accel.y;
                particle->velocity.z += particle->accel.z;
            } else {
                Actor_PlaySfx(&this->actor, NA_SE_EV_HEALING - SFX_FLAG);

                sp94 = player->actor.world.pos;
                sp94.y = player->actor.world.pos.y - 150.0f;
                sp94.z = player->actor.world.pos.z - 50.0f;

                goalPitch = Math_Vec3f_Pitch(&particle->pos, &sp94);
                goalYaw = Math_Vec3f_Yaw(&particle->pos, &sp94);

                Math_ApproachF(&particle->pitch, goalPitch, 0.9f, 5000.0f);
                Math_ApproachF(&particle->yaw, goalYaw, 0.9f, 5000.0f);
                Matrix_Push();
                Matrix_RotateYF(BINANG_TO_RAD(particle->yaw), MTXMODE_NEW);
                Matrix_RotateXFApply(BINANG_TO_RAD(particle->pitch));

                sp94.x = sp94.y = sp94.z = 3.0f;

                Matrix_MultVec3f(&sp94, &sp88);
                Matrix_Pop();
                particle->pos.x += sp88.x;
                particle->pos.y += sp88.y;
                particle->pos.z += sp88.z;
            }
        }

        // fade up, fade down, vanish and reset
        if (particle->timer != 0) {
            particle->timer--;
            particle->alpha += 30;

            if (particle->alpha > 255) {
                particle->alpha = 255;
            }
        } else {
            particle->alpha -= 30;

            if (particle->alpha <= 0) {
                particle->alpha = particle->alive = 0;
            }
        }
    }
}

static void BgDyYoseizo_ParticleDraw(BgDyYoseizo* this, PlayState* play) {
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    u8 phi_s3 = 0;
    BgDyYoseizoParticle* particle = this->particles;
    s16 i;

    OPEN_DISPS(gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    for (i = 0; i < 200; i++, particle++) {
        FrameInterpolation_RecordOpenChild(particle, particle->epoch);

        if (particle->alive == 1) {
            if (phi_s3 == 0) {
                gSPDisplayList(POLY_XLU_DISP++, Lib_SegmentedToVirtual(gGreatFairyParticleMaterialDL));
                gDPPipeSync(POLY_XLU_DISP++);

                phi_s3++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, particle->primColor.r, particle->primColor.g, particle->primColor.b,
                            particle->alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, particle->envColor.r, particle->envColor.g, particle->envColor.b, 0);

            Matrix_Translate(particle->pos.x, particle->pos.y, particle->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(particle->scale, particle->scale, 1.0f, MTXMODE_APPLY);
            Matrix_RotateZF(particle->roll, MTXMODE_APPLY);

            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, Lib_SegmentedToVirtual(gGreatFairyParticleModelDL));
        }

        FrameInterpolation_RecordCloseChild();
    }

    CLOSE_DISPS(gfxCtx);
}
