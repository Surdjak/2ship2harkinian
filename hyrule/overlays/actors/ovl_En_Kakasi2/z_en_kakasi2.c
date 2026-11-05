/*
 * File: z_en_kakasi2.c
 * Overlay: ovl_En_Kakasi2
 * Description: Pierre the Scarecrow Spawn
 */

#include "z_en_kakasi2.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "assets/oot/objects/object_ka/object_ka.h"

#define FLAGS                                                                                               \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | \
     ACTOR_FLAG_UPDATE_DURING_OCARINA | ACTOR_FLAG_LOCK_ON_DISABLED)

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON | ACELEM_HOOKABLE,
        OCELEM_ON,
    },
    { 20, 70, 0, { 0, 0, 0 } },
};

static void EnKakasi2_Init(Actor* thisx, PlayState* play);
static void EnKakasi2_Destroy(Actor* thisx, PlayState* play);
static void EnKakasi2_Update(Actor* thisx, PlayState* play);
static void func_80A90948(Actor* thisx, PlayState* play);

static void func_80A9062C(EnKakasi2* this, PlayState* play);
static void func_80A90264(EnKakasi2* this, PlayState* play);
static void func_80A904D8(EnKakasi2* this, PlayState* play);
static void func_80A90578(EnKakasi2* this, PlayState* play);
static void func_80A906C4(EnKakasi2* this, PlayState* play);

ActorProfile En_Kakasi2_Oot_Profile = {
    /**/ ACTOR_EN_KAKASI2_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_KA_Oot,
    /**/ sizeof(EnKakasi2),
    /**/ EnKakasi2_Init,
    /**/ EnKakasi2_Destroy,
    /**/ EnKakasi2_Update,
    /**/ NULL,
};

static void EnKakasi2_Init(Actor* thisx, PlayState* play) {
    EnKakasi2* this = (EnKakasi2*)thisx;
    s32 pad;
    f32 spawnRangeY;
    f32 spawnRangeXZ;


    this->switchFlag = this->actor.params & 0x3F;
    spawnRangeY = (this->actor.params >> 6) & 0xFF;
    spawnRangeXZ = this->actor.world.rot.z;
    if (this->switchFlag == 0x3F) {
        this->switchFlag = -1;
    }
    this->actor.attentionRangeType = 4;
    this->maxSpawnDistance.x = (spawnRangeY * 40.0f) + 40.0f;
    this->maxSpawnDistance.y = (spawnRangeXZ * 40.0f) + 40.0f;


    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->height = 60.0f;
    Actor_SetScale(&this->actor, 0.01f);
    this->actor.flags |= ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER;
    this->unk_198 = this->actor.shape.rot.y;

    if (this->switchFlag >= 0 && Flags_GetSwitch(play, this->switchFlag)) {
        this->actor.draw = func_80A90948;
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        SkelAnime_InitFlex(play, &this->skelAnime, &object_ka_Skel_0065B0, &object_ka_Anim_000214, NULL, NULL, 0);
        this->actionFunc = func_80A9062C;
    } else {
        this->actionFunc = func_80A90264;
        this->actor.shape.yOffset = -8000.0f;
    }
}

static void EnKakasi2_Destroy(Actor* thisx, PlayState* play) {
    EnKakasi2* this = (EnKakasi2*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
    SkelAnime_Free(&this->skelAnime, play);
}

static void func_80A90264(EnKakasi2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->unk_194++;

    if ((BREG(1) != 0) || (false) &&
                              (this->actor.xzDistToPlayer < this->maxSpawnDistance.x) &&
                              (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) < this->maxSpawnDistance.y)) {
        this->actor.draw = func_80A90948;
        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        SkelAnime_InitFlex(play, &this->skelAnime, &object_ka_Skel_0065B0, &object_ka_Anim_000214, NULL, NULL, 0);
        OnePointCutscene_Attention(play, &this->actor);
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_LOCK_ON_DISABLED;

        Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
        if (this->switchFlag >= 0) {
            Flags_SetSwitch(play, this->switchFlag);
        }

        this->actionFunc = func_80A904D8;
    } else if ((this->actor.xzDistToPlayer < this->maxSpawnDistance.x) &&
               (fabsf(player->actor.world.pos.y - this->actor.world.pos.y) < this->maxSpawnDistance.y) &&
               (GET_EVENTCHKINF_OOT(EVENTCHKINF_PLAYED_SONG_FOR_SCARECROW_AS_ADULT_Oot))) {

        this->unk_194 = 0;
        if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_0B_Oot) {
            if (this->switchFlag >= 0) {
                Flags_SetSwitch(play, this->switchFlag);
            }
            Hyrule_SetOcarinaModeOot(play, OCARINA_MODE_04_Oot);
            this->actor.draw = func_80A90948;
            Collider_InitCylinder(play, &this->collider);
            Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
            SkelAnime_InitFlex(play, &this->skelAnime, &object_ka_Skel_0065B0, &object_ka_Anim_000214, NULL, NULL, 0);
            OnePointCutscene_Attention(play, &this->actor);
            Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);

            this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_LOCK_ON_DISABLED;
            this->actionFunc = func_80A904D8;
        }
    }
}

static void func_80A904D8(EnKakasi2* this, PlayState* play) {
    f32 frameCount = Animation_GetLastFrame(&object_ka_Anim_000214);

    Animation_Change(&this->skelAnime, &object_ka_Anim_000214, 1.0f, 0.0f, (s16)frameCount, ANIMMODE_LOOP, -10.0f);
    Actor_PlaySfx(&this->actor, NA_SE_EV_COME_UP_DEKU_JR);
    this->actionFunc = func_80A90578;
}

static void func_80A90578(EnKakasi2* this, PlayState* play) {
    s16 currentFrame;

    SkelAnime_Update(&this->skelAnime);

    currentFrame = this->skelAnime.curFrame;
    if (currentFrame == 11 || currentFrame == 17) {
        Actor_PlaySfx(&this->actor, NA_SE_EV_KAKASHI_SWING);
    }

    this->actor.shape.rot.y += 0x800;
    Math_ApproachZeroF(&this->actor.shape.yOffset, 0.5f, 500.0f);

    if (this->actor.shape.yOffset > -100.0f) {
        this->actionFunc = func_80A9062C;
        this->actor.shape.yOffset = 0.0f;
    }
}

static void func_80A9062C(EnKakasi2* this, PlayState* play) {
    f32 frameCount = Animation_GetLastFrame(&object_ka_Anim_000214);

    Animation_Change(&this->skelAnime, &object_ka_Anim_000214, 0.0f, 0.0f, (s16)frameCount, ANIMMODE_ONCE, -10.0f);
    this->actionFunc = func_80A906C4;
}

static void func_80A906C4(EnKakasi2* this, PlayState* play) {
    if (this->skelAnime.curFrame != 0) {
        Math_ApproachZeroF(&this->skelAnime.curFrame, 0.5f, 1.0f);
    }
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->unk_198, 5, 0xBB8, 0);
    SkelAnime_Update(&this->skelAnime);
}

static void EnKakasi2_Update(Actor* thisx, PlayState* play2) {
    EnKakasi2* this = (EnKakasi2*)thisx;
    PlayState* play = play2;

    this->actor.world.rot = this->actor.shape.rot;
    Actor_SetFocus(&this->actor, this->height);
    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->actor);

    if (this->actor.shape.yOffset == 0.0f) {
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
    if (BREG(0) != 0) {
        if (BREG(5) != 0) {
        }
        if (this->actor.draw == NULL) {
            if (this->unk_194 != 0) {
                if ((this->unk_194 % 2) == 0) {
                    DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                           this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z,
                                           1.0f, 1.0f, 1.0f, 70, 70, 70, 255, 4, play->state.gfxCtx);
                }
            } else {
                DebugDisplay_AddObject(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                       this->actor.world.rot.x, this->actor.world.rot.y, this->actor.world.rot.z, 1.0f,
                                       1.0f, 1.0f, 0, 255, 255, 255, 4, play->state.gfxCtx);
            }
        }
    }
}

static void func_80A90948(Actor* thisx, PlayState* play) {
    EnKakasi2* this = (EnKakasi2*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, NULL, NULL, this);
}
