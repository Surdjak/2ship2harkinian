/*
 * File: z_en_weather_tag.c
 * Overlay: ovl_En_Weather_Tag
 * Description: Proximity Activated Weather Effects
 */

#include "z_en_weather_tag.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnWeatherTag_Init(Actor* thisx, PlayState* play);
static void EnWeatherTag_Destroy(Actor* thisx, PlayState* play);
static void EnWeatherTag_Update(Actor* thisx, PlayState* play);

static void EnWeatherTag_DisabledCloudyHyruleMarket(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledCloudyHyruleMarket(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledCloudyLonLonRanch(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledCloudyLonLonRanch(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledCloudySnow(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledCloudySnow(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledRainLakeHylia(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledRainLakeHylia(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledCloudyDeathMountain(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledCloudyDeathMountain(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledCloudyRainThunderKakariko(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledCloudyRainThunderKakariko(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_SetSandstormIntensity(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_DisabledRainThunder(EnWeatherTag* this, PlayState* play);
static void EnWeatherTag_EnabledRainThunder(EnWeatherTag* this, PlayState* play);

#define WEATHER_TAG_RANGE100(x) ((x >> 8) * 100.0f)

// 3Ship [Hyrule] OoT's gSkyboxIsChanging (SoH's gSkyboxBlendingEnabled), a z_kankyo.c global in MM too.
extern u8 sSkyboxIsChanging;

ActorProfile En_Weather_Tag_Oot_Profile = {
    /**/ ACTOR_EN_WEATHER_TAG_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnWeatherTag),
    /**/ EnWeatherTag_Init,
    /**/ EnWeatherTag_Destroy,
    /**/ EnWeatherTag_Update,
    /**/ NULL,
};

static void EnWeatherTag_SetupAction(EnWeatherTag* this, EnWeatherTagActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void EnWeatherTag_Destroy(Actor* thisx, PlayState* play) {
}

static void EnWeatherTag_Init(Actor* thisx, PlayState* play) {
    EnWeatherTag* this = (EnWeatherTag*)thisx;

    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;

    switch (this->actor.params & 0xF) {
        case EN_WEATHER_TAG_TYPE_CLOUDY_MARKET:
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyHyruleMarket);
            break;
        case EN_WEATHER_TAG_TYPE_CLOUDY_LON_LON_RANCH:
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_EPONA_OBTAINED_Oot)) {
                Actor_Kill(&this->actor);
            }
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyLonLonRanch);
            break;
        case EN_WEATHER_TAG_TYPE_SNOW_ZORAS_DOMAIN:

            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_WATER_TEMPLE_BLUE_WARP_Oot)) {
                Actor_Kill(&this->actor);
            }
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudySnow);
            break;
        case EN_WEATHER_TAG_TYPE_RAIN_LAKE_HYLIA:

            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_WATER_TEMPLE_BLUE_WARP_Oot)) {
                Actor_Kill(&this->actor);
            }
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledRainLakeHylia);
            break;
        case EN_WEATHER_TAG_TYPE_CLOUDY_DEATH_MOUNTAIN:
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FIRE_TEMPLE_BLUE_WARP_Oot)) {
                Actor_Kill(&this->actor);
            }
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyDeathMountain);
            break;
        case EN_WEATHER_TAG_TYPE_THUNDERSTORM_KAKARIKO:

            if (!GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP_Oot) ||
                !GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_FIRE_TEMPLE_BLUE_WARP_Oot) ||
                !GET_EVENTCHKINF_OOT(EVENTCHKINF_USED_WATER_TEMPLE_BLUE_WARP_Oot) ||
                CHECK_QUEST_ITEM_OOT(QUEST_MEDALLION_SHADOW_Oot)) {
                Actor_Kill(&this->actor);
            }
            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyRainThunderKakariko);
            break;
        case EN_WEATHER_TAG_TYPE_SANDSTORM_INTENSITY:
            EnWeatherTag_SetupAction(this, EnWeatherTag_SetSandstormIntensity);
            break;
        case EN_WEATHER_TAG_TYPE_THUNDERSTORM_GRAVEYARD:

            EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledRainThunder);
            break;
    }
}

static u8 WeatherTag_CheckEnableWeatherEffect(EnWeatherTag* this, PlayState* play, u8 arg2, u8 arg3, u8 arg4, u8 arg5,
                                       u16 arg6, u8 weatherMode) {
    s32 pad;
    u8 ret = false;
    Player* player = GET_PLAYER(play);

    if (Actor_WorldDistXZToActor(&player->actor, &this->actor) < WEATHER_TAG_RANGE100(this->actor.params)) {
        if ((play->envCtx.lightMode != LIGHT_MODE_TIME) || !sSkyboxIsChanging ||
            (play->skyboxId != SKYBOX_NORMAL_SKY && play->envCtx.lightConfig == play->envCtx.changeLightNextConfig)) {
            gInterruptSongOfStorms = 1;
            if (play->envCtx.stormRequest == STORM_REQUEST_NONE &&
                (play->envCtx.lightMode != LIGHT_MODE_TIME || (play->envCtx.lightConfig != 1 && play->envCtx.changeLightEnabled == 0))) {
                gInterruptSongOfStorms = 0;
                if (gWeatherMode != WEATHER_MODE_OOT(weatherMode)) {
                    gWeatherMode = WEATHER_MODE_OOT(weatherMode);
                    if (play->envCtx.stormRequest == STORM_REQUEST_NONE) {
                        play->envCtx.changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                        play->envCtx.skyboxConfig = arg2;
                        play->envCtx.changeSkyboxNextConfig = arg3;
                        play->envCtx.changeSkyboxTimer = arg6;
                        play->envCtx.changeLightEnabled = 1;
                        play->envCtx.lightConfig = arg4;
                        play->envCtx.changeLightNextConfig = arg5;
                        gLightConfigAfterUnderwater = arg5;
                        play->envCtx.changeDuration = arg6;
                        play->envCtx.changeLightTimer = play->envCtx.changeDuration;
                    }
                }
                ret = true;
            }
        } else {
            if (R_TIME_SPEED != 0) {
                gSaveContext.save.time += 0x14;
            }
        }
    }

    return ret;
}

static u8 WeatherTag_CheckRestoreWeather(EnWeatherTag* this, PlayState* play, u8 arg2, u8 arg3, u8 arg4, u8 arg5, u16 arg6) {
    s32 pad;
    u8 ret = false;
    Player* player = GET_PLAYER(play);

    if ((WEATHER_TAG_RANGE100(this->actor.params) + 100.0f) < Actor_WorldDistXZToActor(&player->actor, &this->actor)) {
        if (play->envCtx.lightMode != LIGHT_MODE_TIME || !sSkyboxIsChanging ||
            (play->skyboxId != SKYBOX_NORMAL_SKY && play->envCtx.lightConfig == play->envCtx.changeLightNextConfig)) {
            gInterruptSongOfStorms = 1;
            if ((play->envCtx.stormRequest == STORM_REQUEST_NONE) &&
                (play->envCtx.lightMode != LIGHT_MODE_TIME || (play->envCtx.lightConfig != 1 && play->envCtx.changeLightEnabled == 0))) {
                gInterruptSongOfStorms = 0;
                gWeatherMode = WEATHER_MODE_CLEAR;
                play->envCtx.changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                play->envCtx.skyboxConfig = arg2;
                play->envCtx.changeSkyboxNextConfig = arg3;
                play->envCtx.changeSkyboxTimer = arg6;
                play->envCtx.changeLightEnabled = 1;
                play->envCtx.lightConfig = arg4;
                play->envCtx.changeLightNextConfig = arg5;
                gLightConfigAfterUnderwater = arg5;
                play->envCtx.changeDuration = arg6;
                play->envCtx.changeLightTimer = play->envCtx.changeDuration;

                ret = true;
            }
        } else if (R_TIME_SPEED != 0) {
            gSaveContext.save.time += 0x14;
        }
    }
    return ret;
}

static void EnWeatherTag_DisabledCloudyHyruleMarket(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 3, 60, 1)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledCloudyHyruleMarket);
    }
}

static void EnWeatherTag_EnabledCloudyHyruleMarket(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 3, 0, 60)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyHyruleMarket);
    }
}

static void EnWeatherTag_DisabledCloudyLonLonRanch(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 2, 100, 2)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledCloudyLonLonRanch);
    }
}

static void EnWeatherTag_EnabledCloudyLonLonRanch(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 2, 0, 100)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyLonLonRanch);
    }
}

static void EnWeatherTag_DisabledCloudyDeathMountain(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 2, 60, 2)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledCloudyDeathMountain);
    }
}

static void EnWeatherTag_EnabledCloudyDeathMountain(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 2, 0, 60)) {
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyLonLonRanch);
    }
}

static void EnWeatherTag_DisabledCloudySnow(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 2, 60, 3)) {
        play->envCtx.precipitation[PRECIP_SNOW_MAX] = 64;
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledCloudySnow);
    }
}

static void EnWeatherTag_EnabledCloudySnow(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 2, 0, 60)) {
        play->envCtx.precipitation[PRECIP_SNOW_MAX] = 0;
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudySnow);
    }
}

static void EnWeatherTag_DisabledRainLakeHylia(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 2, 100, 4)) {
        Environment_PlayStormNatureAmbience(play);
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 25;
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledRainLakeHylia);
    }
}

static void EnWeatherTag_EnabledRainLakeHylia(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 2, 0, 100)) {
        Environment_StopStormNatureAmbience(play);
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 0;
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledRainLakeHylia);
    }
}

static void EnWeatherTag_DisabledCloudyRainThunderKakariko(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckEnableWeatherEffect(this, play, 0, 1, 0, 4, 100, 5)) {
        Environment_PlayStormNatureAmbience(play);
        play->envCtx.lightningState = LIGHTNING_ON;
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 30;
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledCloudyRainThunderKakariko);
    }
}

static void EnWeatherTag_EnabledCloudyRainThunderKakariko(EnWeatherTag* this, PlayState* play) {
    if (WeatherTag_CheckRestoreWeather(this, play, 1, 0, 4, 0, 100)) {
        Environment_StopStormNatureAmbience(play);
        play->envCtx.lightningState = LIGHTNING_LAST;
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 0;
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledCloudyRainThunderKakariko);
    }
}

static void EnWeatherTag_SetSandstormIntensity(EnWeatherTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Actor_WorldDistXZToActor(&player->actor, &this->actor) < WEATHER_TAG_RANGE100(this->actor.params)) {
        Math_SmoothStepToS(&play->envCtx.adjLightSettings.fogNear, -0x50, 1, 2, 1);
        Math_SmoothStepToS(&play->envCtx.adjLightSettings.zFar, -0x7D0, 1, 50, 1);
    } else {
        Math_SmoothStepToS(&play->envCtx.adjLightSettings.fogNear, 0, 1, 1, 1);
        Math_SmoothStepToS(&play->envCtx.adjLightSettings.zFar, 0, 1, 25, 1);
    }
}

static void EnWeatherTag_DisabledRainThunder(EnWeatherTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (Actor_WorldDistXZToActor(&player->actor, &this->actor) < WEATHER_TAG_RANGE100(this->actor.params)) {
        Environment_PlayStormNatureAmbience(play);
        play->envCtx.lightningState = LIGHTNING_ON;
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 25;
        EnWeatherTag_SetupAction(this, EnWeatherTag_EnabledRainThunder);
    }
}

static void EnWeatherTag_EnabledRainThunder(EnWeatherTag* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if ((WEATHER_TAG_RANGE100(this->actor.params) + 10.0f) < Actor_WorldDistXZToActor(&player->actor, &this->actor)) {
        Environment_StopStormNatureAmbience(play);
        play->envCtx.lightningState = LIGHTNING_LAST;
        play->envCtx.precipitation[PRECIP_RAIN_MAX] = 0;
        play->envCtx.precipitation[PRECIP_RAIN_CUR] = 10;
        EnWeatherTag_SetupAction(this, EnWeatherTag_DisabledRainThunder);
    }
}

static void EnWeatherTag_Update(Actor* thisx, PlayState* play) {
    EnWeatherTag* this = (EnWeatherTag*)thisx;

    this->actionFunc(this, play);
}
