/*
 * File: z_shot_sun.c
 * Overlay: ovl_Shot_Sun
 * Description: Lake Hylia Sun hitbox and Song of Storms Fairy spawner
 */

#include "z_shot_sun.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "HyruleCutscene.h"
#include "../ovl_En_Elf/z_en_elf.h"
#include "assets/oot/scenes/overworld/spot06/spot06_scene.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void ShotSun_Init(Actor* thisx, PlayState* play);
static void ShotSun_Destroy(Actor* thisx, PlayState* play);
static void ShotSun_Update(Actor* thisx, PlayState* play);

static void ShotSun_SpawnFairy(ShotSun* this, PlayState* play);
static void ShotSun_TriggerFairy(ShotSun* this, PlayState* play);
static void ShotSun_UpdateFairySpawner(ShotSun* this, PlayState* play);
static void ShotSun_UpdateHyliaSun(ShotSun* this, PlayState* play);

ActorProfile Shot_Sun_Oot_Profile = {
    /**/ ACTOR_SHOT_SUN_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ShotSun),
    /**/ ShotSun_Init,
    /**/ ShotSun_Destroy,
    /**/ ShotSun_Update,
    /**/ NULL,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0x00000020), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 30, 60, 0, { 0, 0, 0 } },
};

static void ShotSun_Init(Actor* thisx, PlayState* play) {
    ShotSun* this = (ShotSun*)thisx;
    s32 params;

    params = this->actor.params & 0xFF;
    if (params == 0x40 || params == 0x41) {
        this->fairySpawnerState = 0;
        this->actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        this->actor.flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA;
        this->actionFunc = ShotSun_UpdateFairySpawner;
        this->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
    } else {
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        this->actionFunc = ShotSun_UpdateHyliaSun;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    }
}

static void ShotSun_Destroy(Actor* thisx, PlayState* play) {
    ShotSun* this = (ShotSun*)thisx;
    s32 params = this->actor.params & 0xFF;

    if (params != 0x40 && params != 0x41) {
        Collider_DestroyCylinder(play, &this->collider);
    }
}

static void ShotSun_SpawnFairy(ShotSun* this, PlayState* play) {
    s32 params = this->actor.params & 0xFF;
    s32 fairyType;

    if (this->timer > 0) {
        this->timer--;
    } else {
        switch (params) {
            case 0x40:
                fairyType = FAIRY_HEAL_BIG;
                break;
            case 0x41:
                fairyType = FAIRY_HEAL_BIG;
                break;
        }

        //! @bug fairyType may be uninitialized
        if ((true)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_ELF_Oot, this->actor.home.pos.x, this->actor.home.pos.y,
                        this->actor.home.pos.z, 0, 0, 0, fairyType);
        }

        Actor_Kill(&this->actor);
    }
}

static void ShotSun_TriggerFairy(ShotSun* this, PlayState* play) {
    if ((Hyrule_GetAttentionCategoryOot() == this->actor.category) || (this->timer != 0)) {
        this->actionFunc = ShotSun_SpawnFairy;
        this->timer = 50;

        Actor_Spawn(&play->actorCtx, play, ACTOR_DEMO_KANKYO_Oot, this->actor.home.pos.x, this->actor.home.pos.y,
                    this->actor.home.pos.z, 0, 0, 0, 0x11);

        Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_EV_TRE_BOX_APPEAR);
    }
}

static void ShotSun_UpdateFairySpawner(ShotSun* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 pad;
    s32 params = this->actor.params & 0xFF;

    if (Math3D_Vec3fDistSq(&this->actor.world.pos, &player->actor.world.pos) > 22500.0f) {
        this->fairySpawnerState = 0;
    } else {
        if (this->fairySpawnerState == 0) {
            if (!(Hyrule_OcarinaAttemptedOot(play, (Actor*)this))) {
                Hyrule_OfferOcarinaOot(play, (Actor*)this);
                return;
            } else {
                this->fairySpawnerState = 1;
            }
        }
        if (this->fairySpawnerState == 1) {
            Message_StartOcarinaOot(play, OCARINA_ACTION_FREE_PLAY_Oot);
            this->fairySpawnerState = 2;
        } else if (this->fairySpawnerState == 2 && Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_04_Oot) {
            if ((params == 0x40 && play->msgCtx.lastPlayedSong == OCARINA_SONG_SUNS_Oot) ||
                (params == 0x41 && play->msgCtx.lastPlayedSong == OCARINA_SONG_STORMS_Oot)) {
                this->actionFunc = ShotSun_TriggerFairy;
                OnePointCutscene_Attention(play, &this->actor);
                this->timer = 0;
            } else {
                this->fairySpawnerState = 0;
            }
            this->fairySpawnerState = 0;
        }
    }
}

static void ShotSun_UpdateHyliaSun(ShotSun* this, PlayState* play) {
    Vec3s cylinderPos;
    Player* player = GET_PLAYER(play);
    EnItem00* collectible;
    s32 pad;
    Vec3f spawnPos;

    if (this->collider.base.acFlags & AC_HIT) {
        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        if ((INV_CONTENT_OOT(ITEM_ARROW_FIRE_Oot) == ITEM_NONE)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_ITEM_ETCETERA_Oot, 700.0f, -800.0f, 7261.0f, 0, 0, 0, 7);
            if ((true)) {
                gCsCtxOot.segment = Lib_SegmentedToVirtual(gLakeHyliaFireArrowsCS);
                Hyrule_TriggerCutsceneOot(1);
            }
        } else {
            spawnPos.x = 700.0f;
            spawnPos.y = -800.0f;
            spawnPos.z = 7261.0f;

            collectible = Item_DropCollectible(play, &spawnPos, ITEM00_MAGIC_JAR_BIG);
            if (collectible != NULL) {
                collectible->unk152 = 6000;
                collectible->actor.speed = 0.0f;
            }
        }
        Actor_Kill(&this->actor);
    } else {
        if (!(this->actor.xzDistToPlayer > 120.0f) && gSaveContext.save.time >= 0x4555 && gSaveContext.save.time < 0x5000) {
            cylinderPos.x = player->bodyPartsPos[7].x + play->envCtx.sunPos.x * (1.0f / 6.0f);
            cylinderPos.y = player->bodyPartsPos[7].y - 30.0f + play->envCtx.sunPos.y * (1.0f / 6.0f);
            cylinderPos.z = player->bodyPartsPos[7].z + play->envCtx.sunPos.z * (1.0f / 6.0f);

            this->hitboxPos = cylinderPos;

            Collider_SetCylinderPosition(&this->collider, &cylinderPos);
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        }
    }
}

static void ShotSun_Update(Actor* thisx, PlayState* play) {
    ShotSun* this = (ShotSun*)thisx;

    this->actionFunc(this, play);
}
