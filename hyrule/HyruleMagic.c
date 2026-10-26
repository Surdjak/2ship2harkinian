// OoT's spells on MM's Player (docs/OOT_ITEMS.md). Farore's Wind's pointer is OoT's func_8002FA60 and
// Actor_DrawFaroresWindPointer (z_actor.c), ported from SoH.

#include "global.h"
#include "HyruleMagic.h"
#include "HyruleIds.h"
#include "HyruleScenes.h"
#include "HyrulePort.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"

static const s16 sSpellCosts[HYRULE_SPELL_MAX] = { 12, 24, 12 }; // OoT's sMagicSpellCosts[3-5]

static const s16 sSpellActorIds[HYRULE_SPELL_MAX] = {
    ACTOR_MAGIC_WIND_Oot,
    ACTOR_MAGIC_DARK_Oot,
    ACTOR_MAGIC_FIRE_Oot,
};

// OoT's sRestrictionFlags (z_parameter.c) for its spells: OoT scene, Farore's Wind, Din's Fire and Nayru's Love,
// non-zero when not allowed. Scenes missing allow both.
typedef struct {
    u8 ootScene;
    u8 farores;
    u8 dinsNayrus;
} SpellRestriction;

static const SpellRestriction sOotSpellRestrictions[] = {
    { 0x00, 0, 0 }, // DEKU_TREE
    { 0x01, 0, 0 }, // DODONGOS_CAVERN
    { 0x02, 0, 0 }, // JABU_JABU
    { 0x03, 0, 0 }, // FOREST_TEMPLE
    { 0x04, 0, 0 }, // FIRE_TEMPLE
    { 0x05, 0, 0 }, // WATER_TEMPLE
    { 0x06, 0, 0 }, // SPIRIT_TEMPLE
    { 0x07, 0, 0 }, // SHADOW_TEMPLE
    { 0x08, 0, 0 }, // BOTTOM_OF_THE_WELL
    { 0x09, 0, 0 }, // ICE_CAVERN
    { 0x0A, 0, 0 }, // GANONS_TOWER
    { 0x0B, 1, 0 }, // GERUDO_TRAINING_GROUND
    { 0x0C, 1, 0 }, // THIEVES_HIDEOUT
    { 0x0D, 1, 0 }, // INSIDE_GANONS_CASTLE
    { 0x0E, 1, 0 }, // GANONS_TOWER_COLLAPSE_INTERIOR
    { 0x0F, 1, 0 }, // INSIDE_GANONS_CASTLE_COLLAPSE
    { 0x10, 1, 1 }, // TREASURE_BOX_SHOP
    { 0x11, 1, 0 }, // DEKU_TREE_BOSS
    { 0x12, 1, 0 }, // DODONGOS_CAVERN_BOSS
    { 0x13, 1, 0 }, // JABU_JABU_BOSS
    { 0x14, 1, 0 }, // FOREST_TEMPLE_BOSS
    { 0x15, 1, 0 }, // FIRE_TEMPLE_BOSS
    { 0x16, 1, 0 }, // WATER_TEMPLE_BOSS
    { 0x17, 1, 0 }, // SPIRIT_TEMPLE_BOSS
    { 0x18, 1, 0 }, // SHADOW_TEMPLE_BOSS
    { 0x19, 1, 0 }, // GANONDORF_BOSS
    { 0x1A, 1, 0 }, // GANONS_TOWER_COLLAPSE_EXTERIOR
    { 0x1B, 1, 1 }, // MARKET_ENTRANCE_DAY
    { 0x1C, 1, 1 }, // MARKET_ENTRANCE_NIGHT
    { 0x1D, 1, 1 }, // MARKET_ENTRANCE_RUINS
    { 0x1E, 1, 1 }, // BACK_ALLEY_DAY
    { 0x1F, 1, 1 }, // BACK_ALLEY_NIGHT
    { 0x20, 1, 1 }, // MARKET_DAY
    { 0x21, 1, 1 }, // MARKET_NIGHT
    { 0x22, 1, 1 }, // MARKET_RUINS
    { 0x23, 1, 1 }, // TEMPLE_OF_TIME_EXTERIOR_DAY
    { 0x24, 1, 1 }, // TEMPLE_OF_TIME_EXTERIOR_NIGHT
    { 0x25, 1, 1 }, // TEMPLE_OF_TIME_EXTERIOR_RUINS
    { 0x26, 1, 1 }, // KNOW_IT_ALL_BROS_HOUSE
    { 0x27, 1, 1 }, // TWINS_HOUSE
    { 0x28, 1, 1 }, // MIDOS_HOUSE
    { 0x29, 1, 1 }, // SARIAS_HOUSE
    { 0x2A, 1, 1 }, // KAKARIKO_CENTER_GUEST_HOUSE
    { 0x2B, 1, 1 }, // BACK_ALLEY_HOUSE
    { 0x2C, 1, 1 }, // BAZAAR
    { 0x2D, 1, 1 }, // KOKIRI_SHOP
    { 0x2E, 1, 1 }, // GORON_SHOP
    { 0x2F, 1, 1 }, // ZORA_SHOP
    { 0x30, 1, 1 }, // POTION_SHOP_KAKARIKO
    { 0x31, 1, 1 }, // POTION_SHOP_MARKET
    { 0x32, 1, 1 }, // BOMBCHU_SHOP
    { 0x33, 1, 1 }, // HAPPY_MASK_SHOP
    { 0x34, 1, 1 }, // LINKS_HOUSE
    { 0x35, 1, 1 }, // DOG_LADY_HOUSE
    { 0x36, 1, 1 }, // STABLE
    { 0x37, 1, 1 }, // IMPAS_HOUSE
    { 0x38, 1, 1 }, // LAKESIDE_LABORATORY
    { 0x39, 1, 1 }, // CARPENTERS_TENT
    { 0x3A, 1, 1 }, // GRAVEKEEPERS_HUT
    { 0x3B, 1, 0 }, // GREAT_FAIRYS_FOUNTAIN_MAGIC
    { 0x3C, 1, 0 }, // FAIRYS_FOUNTAIN
    { 0x3D, 1, 0 }, // GREAT_FAIRYS_FOUNTAIN_SPELLS
    { 0x3E, 1, 0 }, // GROTTOS
    { 0x3F, 1, 0 }, // REDEAD_GRAVE
    { 0x40, 1, 0 }, // GRAVE_WITH_FAIRYS_FOUNTAIN
    { 0x41, 1, 0 }, // ROYAL_FAMILYS_TOMB
    { 0x42, 1, 1 }, // SHOOTING_GALLERY
    { 0x43, 1, 1 }, // TEMPLE_OF_TIME
    { 0x44, 2, 2 }, // CHAMBER_OF_THE_SAGES
    { 0x45, 1, 1 }, // CASTLE_COURTYARD_GUARDS_DAY
    { 0x46, 1, 1 }, // CASTLE_COURTYARD_GUARDS_NIGHT
    { 0x48, 1, 1 }, // WINDMILL_AND_DAMPES_GRAVE
    { 0x49, 1, 1 }, // FISHING_POND
    { 0x4A, 1, 1 }, // CASTLE_COURTYARD_ZELDA
    { 0x4B, 1, 1 }, // BOMBCHU_BOWLING_ALLEY
    { 0x4C, 1, 1 }, // LON_LON_BUILDINGS
    { 0x4D, 1, 1 }, // MARKET_GUARD_HOUSE
    { 0x4E, 1, 1 }, // POTION_SHOP_GRANNY
    { 0x4F, 1, 0 }, // GANON_BOSS
    { 0x50, 1, 1 }, // HOUSE_OF_SKULLTULA
    { 0x51, 1, 0 }, // HYRULE_FIELD
    { 0x52, 1, 0 }, // KAKARIKO_VILLAGE
    { 0x53, 1, 0 }, // GRAVEYARD
    { 0x54, 1, 0 }, // ZORAS_RIVER
    { 0x55, 1, 0 }, // KOKIRI_FOREST
    { 0x56, 1, 0 }, // SACRED_FOREST_MEADOW
    { 0x57, 1, 0 }, // LAKE_HYLIA
    { 0x58, 1, 0 }, // ZORAS_DOMAIN
    { 0x59, 1, 0 }, // ZORAS_FOUNTAIN
    { 0x5A, 1, 0 }, // GERUDO_VALLEY
    { 0x5B, 1, 0 }, // LOST_WOODS
    { 0x5C, 1, 0 }, // DESERT_COLOSSUS
    { 0x5D, 1, 0 }, // GERUDOS_FORTRESS
    { 0x5E, 1, 0 }, // HAUNTED_WASTELAND
    { 0x5F, 1, 0 }, // HYRULE_CASTLE
    { 0x60, 1, 0 }, // DEATH_MOUNTAIN_TRAIL
    { 0x61, 1, 0 }, // DEATH_MOUNTAIN_CRATER
    { 0x62, 1, 0 }, // GORON_CITY
    { 0x63, 1, 0 }, // LON_LON_RANCH
    { 0x64, 1, 0 }, // OUTSIDE_GANONS_CASTLE
};

s32 Hyrule_GetSpell(u16 item) {
    switch (item) {
        case ITEM_FARORES_WIND_Oot:
            return HYRULE_SPELL_FARORES_WIND;
        case ITEM_NAYRUS_LOVE_Oot:
            return HYRULE_SPELL_NAYRUS_LOVE;
        case ITEM_DINS_FIRE_Oot:
            return HYRULE_SPELL_DINS_FIRE;
        default:
            return -1;
    }
}

s32 Hyrule_GetSpellCost(s32 spell) {
    return sSpellCosts[spell];
}

s32 Hyrule_IsSpellAllowed(PlayState* play, s32 spell) {
    s32 ootScene = Hyrule_GetOotSceneId(play->sceneId);
    s32 i;

    if (ootScene >= 0) {
        for (i = 0; i < ARRAY_COUNT(sOotSpellRestrictions); i++) {
            if (sOotSpellRestrictions[i].ootScene == ootScene) {
                return (spell == HYRULE_SPELL_FARORES_WIND) ? (sOotSpellRestrictions[i].farores == 0)
                                                            : (sOotSpellRestrictions[i].dinsNayrus == 0);
            }
        }
        return true;
    }

    if (spell == HYRULE_SPELL_FARORES_WIND) {
        // MM's temples, as OoT allows its dungeons (not their boss rooms).
        switch (play->sceneId) {
            case SCENE_MITURIN:
            case SCENE_HAKUGIN:
            case SCENE_SEA:
            case SCENE_INISIE_N:
            case SCENE_INISIE_R:
                return true;
            default:
                return false;
        }
    }

    // Not in Clock Town, as OoT keeps them out of its towns.
    switch (play->sceneId) {
        case SCENE_CLOCKTOWER:
        case SCENE_TOWN:
        case SCENE_BACKTOWN:
        case SCENE_ICHIBA:
        case SCENE_ALLEY:
            return false;
        default:
            return true;
    }
}

s32 Hyrule_IsSpellItemAllowed(PlayState* play, u16 item) {
    s32 spell = Hyrule_GetSpell(item);

    return (spell < 0) || Hyrule_IsSpellAllowed(play, spell);
}

Actor* Hyrule_SpawnSpell(PlayState* play, Player* player, s32 spell) {
    return Actor_Spawn(&play->actorCtx, play, sSpellActorIds[spell], player->actor.world.pos.x,
                       player->actor.world.pos.y, player->actor.world.pos.z, 0, 0, 0, 0);
}

// Farore's Wind's pointer: OoT's respawn[RESPAWN_MODE_TOP] while in a scene (data grows to 40 when the point is
// set, past 40 when it is used, and back to 0 when dispelled), and OoT's D_8015BC00-D_8015BC18.
static RespawnData sPointer;
static LightInfo sLightInfo;
static LightNode* sLightNode;
static s32 sPointerWait;
static f32 sPointerTravel;
static s32 sArriving;

#define FARORES_WIND (gSaveContext.save.saveInfo.faroresWindOot)

s32 Hyrule_IsFaroresWindSet(void) {
    return FARORES_WIND.data != 0;
}

void Hyrule_InitFaroresWind(PlayState* play) {
    Vec3f lightPos;

    if (Hyrule_IsFaroresWindSet()) {
        sPointer = FARORES_WIND;
        sPointer.data = 40;
    } else {
        sPointer.data = 0;
        sPointer.pos.x = sPointer.pos.y = sPointer.pos.z = 0.0f;
    }

    lightPos.x = sPointer.pos.x;
    lightPos.y = sPointer.pos.y + 80.0f;
    lightPos.z = sPointer.pos.z;
    Lights_PointNoGlowSetInfo(&sLightInfo, lightPos.x, lightPos.y, lightPos.z, 255, 255, 255, -1);
    sLightNode = LightContext_InsertLight(play, &play->lightCtx, &sLightInfo);
    sPointerWait = 0;
    sPointerTravel = 0.0f;
}

void Hyrule_DestroyFaroresWind(PlayState* play) {
    LightContext_RemoveLight(play, &play->lightCtx, sLightNode);
}

void Hyrule_SetFaroresWind(PlayState* play) {
    Player* player = GET_PLAYER(play);
    u32 entrance = gSaveContext.save.entrance;

    // OoT's Play_SetupRespawnPoint(RESPAWN_MODE_TOP): the pointer starts on Link...
    sPointer.entrance = Entrance_Create(ENTRANCE_GET_SCENE(entrance), 0, entrance & 0xF);
    sPointer.roomIndex = play->roomCtx.curRoom.num;
    sPointer.pos = player->actor.world.pos;
    sPointer.yaw = player->actor.shape.rot.y;
    sPointer.data = 1;
    // ...and the point is where Link came into the room, his void-out respawn point.
    FARORES_WIND = gSaveContext.respawn[RESPAWN_MODE_DOWN];
    FARORES_WIND.data = 1;
}

void Hyrule_DispelFaroresWind(void) {
    sPointer.data = -sPointer.data;
    FARORES_WIND.data = 0;
    Audio_PlaySfx_AtPos(&sPointer.pos, NA_SE_PL_MAGIC_WIND_VANISH);
}

void Hyrule_ClearFaroresWind(void) {
    FARORES_WIND.data = 0;
}

void Hyrule_ReturnToFaroresWind(PlayState* play) {
    gSaveContext.respawn[RESPAWN_MODE_TOP] = FARORES_WIND;
    gSaveContext.respawnFlag = 3; // RESPAWN_MODE_TOP, as in OoT
    play->nextEntrance = FARORES_WIND.entrance;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
    sArriving = true;
}

s32 Hyrule_TakeFaroresWindArrival(void) {
    s32 arriving = sArriving;

    sArriving = false;
    return arriving;
}

void Hyrule_ArriveAtFaroresWind(PlayState* play) {
    sPointer.data++;
    Audio_PlaySfx_AtPos(&sPointer.pos, NA_SE_PL_MAGIC_WIND_WARP);
}

void Hyrule_DrawFaroresWind(PlayState* play) {
    s32 lightRadius = -1;
    s32 params;

    OPEN_DISPS(play->state.gfxCtx);

    params = sPointer.data;

    if (params) {
        f32 yOffset = LINK_IS_ADULT_OOT ? 80.0f : 60.0f;
        f32 ratio = 1.0f;
        s32 alpha = 255;
        s32 temp = params - 40;

        if (temp < 0) {
            sPointer.data = ++params;
            ratio = ABS(params) * 0.025f;
            sPointerWait = 60;
            sPointerTravel = 1.0f;
        } else if (sPointerWait) {
            sPointerWait--;
        } else if (sPointerTravel > 0.0f) {
            static Vec3f effectVel = { 0.0f, -0.05f, 0.0f };
            static Vec3f effectAccel = { 0.0f, -0.025f, 0.0f };
            static Color_RGBA8 effectPrimCol = { 255, 255, 255, 0 };
            static Color_RGBA8 effectEnvCol = { 100, 200, 0, 0 };
            Vec3f* curPos = &sPointer.pos;
            Vec3f* nextPos = &FARORES_WIND.pos;
            f32 prevNum = sPointerTravel;
            Vec3f dist;
            f32 diff = Math_Vec3f_DistXYZAndStoreDiff(nextPos, curPos, &dist);
            Vec3f effectPos;
            f32 factor;
            f32 length;
            f32 dx;
            f32 speed;

            if (diff < 20.0f) {
                sPointerTravel = 0.0f;
                Math_Vec3f_Copy(curPos, nextPos);
            } else {
                length = diff * (1.0f / sPointerTravel);
                speed = 20.0f / length;
                speed = CLAMP_MIN(speed, 0.05f);
                Math_StepToF(&sPointerTravel, 0.0f, speed);
                factor = (diff * (sPointerTravel / prevNum)) / diff;
                curPos->x = nextPos->x + (dist.x * factor);
                curPos->y = nextPos->y + (dist.y * factor);
                curPos->z = nextPos->z + (dist.z * factor);
                length *= 0.5f;
                dx = diff - length;
                yOffset += sqrtf(SQ(length) - SQ(dx)) * 0.2f;
            }

            effectPos.x = curPos->x + Rand_CenteredFloat(6.0f);
            effectPos.y = curPos->y + 80.0f + (6.0f * Rand_ZeroOne());
            effectPos.z = curPos->z + Rand_CenteredFloat(6.0f);

            EffectSsKirakira_SpawnDispersed(play, &effectPos, &effectVel, &effectAccel, &effectPrimCol,
                                            &effectEnvCol, 1000, 16);

            if (sPointerTravel == 0.0f) {
                sPointer = FARORES_WIND;
                sPointer.data = 40;
            }
        } else if (temp > 0) {
            Vec3f* curPos = &sPointer.pos;
            f32 nextRatio = 1.0f - temp * 0.1f;
            f32 curRatio = 1.0f - (f32)(temp - 1) * 0.1f;
            Vec3f eye;
            Vec3f dist;
            f32 diff;

            if (nextRatio > 0.0f) {
                eye.x = play->view.eye.x;
                eye.y = play->view.eye.y - yOffset;
                eye.z = play->view.eye.z;
                diff = Math_Vec3f_DistXYZAndStoreDiff(&eye, curPos, &dist);
                diff = (diff * (nextRatio / curRatio)) / diff;
                curPos->x = eye.x + (dist.x * diff);
                curPos->y = eye.y + (dist.y * diff);
                curPos->z = eye.z + (dist.z * diff);
            }

            alpha = 255 - (temp * 30);

            if (alpha < 0) {
                // Link has arrived: the point is used up.
                FARORES_WIND.data = 0;
                sPointer.data = 0;
                alpha = 0;
            } else {
                sPointer.data = ++params;
            }

            ratio = 1.0f + ((f32)temp * 0.2);
        }

        lightRadius = 500.0f * ratio;

        if ((play->csCtx.state == CS_STATE_IDLE) &&
            (ENTRANCE_GET_SCENE(sPointer.entrance) == ENTRANCE_GET_SCENE(gSaveContext.save.entrance)) &&
            (sPointer.roomIndex == play->roomCtx.curRoom.num)) {
            f32 scale = 0.025f * ratio;

            POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, SETUPDL_25);

            Matrix_Translate(sPointer.pos.x, sPointer.pos.y + yOffset, sPointer.pos.z, MTXMODE_NEW);
            Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
            Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
            Matrix_Push();

            gDPPipeSync(POLY_XLU_DISP++);
            gDPSetPrimColor(POLY_XLU_DISP++, 128, 128, 255, 255, 200, alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, 100, 200, 0, 255);

            Matrix_RotateZF(((play->gameplayFrames * 1500) & 0xFFFF) * M_PI / 32768.0f, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);

            Matrix_Pop();
            Matrix_RotateZF(~((play->gameplayFrames * 1200) & 0xFFFF) * M_PI / 32768.0f, MTXMODE_APPLY);

            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);
        }

        Lights_PointNoGlowSetInfo(&sLightInfo, sPointer.pos.x, sPointer.pos.y + yOffset, sPointer.pos.z, 255, 255,
                                  255, lightRadius);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
