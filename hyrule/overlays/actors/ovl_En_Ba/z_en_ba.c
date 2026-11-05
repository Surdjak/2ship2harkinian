/*
 * File: z_en_ba.c
 * Overlay: ovl_En_Ba
 * Description: Tentacle from inside Lord Jabu-Jabu
 */

#include "z_en_ba.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_bxa/object_bxa.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnBa_Init(Actor* thisx, PlayState* play);
static void EnBa_Destroy(Actor* thisx, PlayState* play);
static void EnBa_Update(Actor* thisx, PlayState* play);
static void EnBa_Draw(Actor* thisx, PlayState* play);

static void EnBa_SetupIdle(EnBa* this);
static void EnBa_SetupFallAsBlob(EnBa* this);
static void EnBa_Idle(EnBa* this, PlayState* play);
static void EnBa_FallAsBlob(EnBa* this, PlayState* play);
static void EnBa_SwingAtPlayer(EnBa* this, PlayState* play);
static void EnBa_RecoilFromDamage(EnBa* this, PlayState* play);
static void EnBa_Die(EnBa* this, PlayState* play);
static void EnBa_SetupSwingAtPlayer(EnBa* this);

ActorProfile En_Ba_Oot_Profile = {
    /**/ ACTOR_EN_BA_Oot,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_BXA_Oot,
    /**/ sizeof(EnBa),
    /**/ EnBa_Init,
    /**/ EnBa_Destroy,
    /**/ EnBa_Update,
    /**/ EnBa_Draw,
};

static Vec3f D_809B8080 = { 0.0f, 0.0f, 32.0f };

static ColliderJntSphElementInit sJntSphElementInit[2] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { DMG_FROM_OOT(0x00000010), 0x00, 0x00 },
            ATELEM_NONE,
            ACELEM_ON,
            OCELEM_NONE,
        },
        { 8, { { 0, 0, 0 }, 20 }, 100 },
    },
    {
        {
            ELEM_MATERIAL_UNK0,
            { DMG_FROM_OOT(0x20000000), 0x00, 0x04 },
            { 0x00000000, 0x00, 0x00 },
            ATELEM_ON | ATELEM_SFX_NORMAL,
            ACELEM_NONE,
            OCELEM_NONE,
        },
        { 13, { { 0, 0, 0 }, 25 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COL_MATERIAL_HIT0,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_JNTSPH,
    },
    2,
    sJntSphElementInit,
};

static void EnBa_SetupAction(EnBa* this, EnBaActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static Vec3f D_809B80E4 = { 0.01f, 0.01f, 0.01f };

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(hintId, TATL_HINT_ID_NONE, ICHAIN_CONTINUE), // PORT: OoT's Navi enemy info 0x15
    ICHAIN_F32(cullingVolumeScale, 1500, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 2500, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 0, ICHAIN_STOP),
};

static void EnBa_Init(Actor* thisx, PlayState* play) {
    EnBa* this = (EnBa*)thisx;
    Vec3f sp38 = D_809B80E4;
    s32 pad;
    s16 i;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    this->actor.world.pos.y = this->actor.home.pos.y + 100.0f;
    for (i = 13; i >= 0; i--) {
        this->unk_200[i] = sp38;
        this->unk_2A8[i].x = -0x4000;
        this->unk_158[i] = this->actor.world.pos;
        this->unk_158[i].y = this->actor.world.pos.y - (i + 1) * 32.0f;
    }

    this->actor.attentionRangeType = 4;
    this->upperParams = (thisx->params >> 8) & 0xFF;
    thisx->params &= 0xFF;
    this->epoch++;

    if (this->actor.params < EN_BA_DEAD_BLOB) {
        if (Flags_GetSwitch(play, this->upperParams)) {
            Actor_Kill(&this->actor);
            return;
        }
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 48.0f);
        Actor_SetScale(&this->actor, 0.01f);
        EnBa_SetupIdle(this);
        this->actor.colChkInfo.health = 4;
        this->actor.colChkInfo.mass = MASS_HEAVY;
        Collider_InitJntSph(play, &this->collider);
        Collider_SetJntSph(play, &this->collider, &this->actor, &sJntSphInit, this->colliderItems);
    } else {
        Actor_SetScale(&this->actor, 0.021f);
        EnBa_SetupFallAsBlob(this);
    }
}

static void EnBa_Destroy(Actor* thisx, PlayState* play) {
    EnBa* this = (EnBa*)thisx;
    Collider_DestroyJntSph(play, &this->collider);
}

static void EnBa_SetupIdle(EnBa* this) {
    this->unk_14C = 2;
    this->unk_31C = 1500;
    this->actor.speed = 10.0f;
    EnBa_SetupAction(this, EnBa_Idle);
}

static void EnBa_Idle(EnBa* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 i;
    s32 pad;
    Vec3s sp5C;

    if ((this->actor.colChkInfo.mass == MASS_IMMOVABLE) && (this->actor.xzDistToPlayer > 175.0f)) {
        Math_SmoothStepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 330.0f, 1.0f, 7.0f, 0.0f);
    } else {
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
        Math_SmoothStepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 100.0f, 1.0f, 10.0f, 0.0f);
    }
    this->unk_2FC = this->actor.world.pos;
    if (play->gameplayFrames % 16 == 0) {
        this->unk_308.z += Rand_CenteredFloat(180.0f);
        this->unk_314 += Rand_CenteredFloat(180.0f);
        this->unk_308.x = Math_SinF(this->unk_308.z) * 80.0f;
        this->unk_308.y = Math_CosF(this->unk_314) * 80.0f;
    }
    this->unk_2FC.y -= 448.0f;
    this->unk_2FC.x += this->unk_308.x;
    this->unk_2FC.z += this->unk_308.y;
    func_80033AEC(&this->unk_2FC, &this->unk_158[13], 1.0f, this->actor.speed, 0.0f, 0.0f);
    for (i = 12; i >= 0; i--) {
        Math_Vec3fDiffToRotOot(&this->unk_158[i + 1], &this->unk_158[i], &sp5C, 0);
        Matrix_Translate(this->unk_158[i + 1].x, this->unk_158[i + 1].y, this->unk_158[i + 1].z, MTXMODE_NEW);
        Matrix_RotateZYX(sp5C.x, sp5C.y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&D_809B8080, &this->unk_158[i]);
    }
    Math_Vec3fDiffToRotOot(&this->unk_158[0], &this->unk_2FC, &sp5C, 0);
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->unk_2A8[0].y, 3, this->unk_31C, 182);
    Math_SmoothStepToS(&this->actor.shape.rot.x, this->unk_2A8[0].x, 3, this->unk_31C, 182);
    Matrix_RotateZYX(this->actor.shape.rot.x - 0x8000, this->actor.shape.rot.y, 0, MTXMODE_APPLY);
    Matrix_MultVec3f(&D_809B8080, &this->unk_158[0]);
    this->unk_2A8[13].y = sp5C.y;
    this->unk_2A8[13].x = sp5C.x + 0x8000;

    for (i = 0; i < 13; i++) {
        Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
        Math_SmoothStepToS(&this->unk_2A8[i].y, this->unk_2A8[i + 1].y, 3, this->unk_31C, 182);
        Math_SmoothStepToS(&this->unk_2A8[i].x, this->unk_2A8[i + 1].x, 3, this->unk_31C, 182);
        Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&D_809B8080, &this->unk_158[i + 1]);
    }
    this->unk_2A8[13].x = this->unk_2A8[12].x;
    this->unk_2A8[13].y = this->unk_2A8[12].y;
    if (!(player->stateFlags1 & PLAYER_STATE1_4000000) && (this->actor.xzDistToPlayer <= 175.0f) &&
        (this->actor.world.pos.y == this->actor.home.pos.y + 100.0f)) {
        EnBa_SetupSwingAtPlayer(this);
    }
}

static void EnBa_SetupFallAsBlob(EnBa* this) {
    this->unk_14C = 0;
    this->actor.speed = Rand_CenteredFloat(8.0f);
    this->actor.world.rot.y = Rand_CenteredFloat(65535.0f);
    this->unk_318 = 20;
    this->actor.gravity = -2.0f;
    EnBa_SetupAction(this, EnBa_FallAsBlob);
}

/**
 * Action function of the pink fleshy blobs that spawn and fall to the floor when a tentacle dies
 */
static void EnBa_FallAsBlob(EnBa* this, PlayState* play) {
    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.scale.y -= 0.001f;
        this->actor.scale.x += 0.0005f;
        this->actor.scale.z += 0.0005f;
        this->unk_318--;
        if (this->unk_318 == 0) {
            Actor_Kill(&this->actor);
        }
    } else {
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 30.0f, 28.0f, 80.0f, 0x405);
    }
}

static void EnBa_SetupSwingAtPlayer(EnBa* this) {
    this->unk_14C = 3;
    this->unk_318 = 20;
    this->unk_31A = 0;
    this->unk_31C = 1500;
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->actor.speed = 20.0f;
    EnBa_SetupAction(this, EnBa_SwingAtPlayer);
}

static void EnBa_SwingAtPlayer(EnBa* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 temp;
    s16 i;
    Vec3s sp58;
    s16 phi_fp;

    Math_SmoothStepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 60.0f, 1.0f, 10.0f, 0.0f);
    if ((this->actor.xzDistToPlayer <= 175.0f) || (this->unk_31A != 0)) {
        if (this->unk_318 == 20) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_KONB_DEAD_JUMP_OLD);
            this->unk_31C = 1500;
        }
        if (this->unk_318 != 0) {
            this->unk_31A = 10;
            this->unk_318--;
            if (this->unk_318 >= 11) {
                this->unk_2FC = player->actor.world.pos;
                this->unk_2FC.y += 30.0f;
                phi_fp = this->actor.yawTowardsPlayer;
            } else {
                phi_fp = Math_Vec3f_Yaw(&this->actor.world.pos, &this->unk_2FC);
            }
            Math_SmoothStepToS(&this->unk_31C, 1500, 1, 30, 0);
            Math_Vec3fDiffToRotOot(&this->actor.world.pos, &this->unk_158[0], &sp58, 0);
            Math_SmoothStepToS(&this->actor.shape.rot.y, sp58.y, 1, this->unk_31C, 0);
            Math_SmoothStepToS(&this->actor.shape.rot.x, (sp58.x + 0x8000), 1, this->unk_31C, 0);
            Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
            Matrix_RotateZYX((this->actor.shape.rot.x - 0x8000), this->actor.shape.rot.y, 0, MTXMODE_APPLY);
            Matrix_MultVec3f(&D_809B8080, &this->unk_158[0]);

            for (i = 0; i < 13; i++) {
                Math_SmoothStepToS(&this->unk_2A8[i].x, (i * 1200) - 0x4000, 1, this->unk_31C, 0);
                Math_SmoothStepToS(&this->unk_2A8[i].y, phi_fp, 1, this->unk_31C, 0);
                Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
                Matrix_RotateZYX((this->unk_2A8[i].x - 0x8000), this->unk_2A8[i].y, 0, MTXMODE_APPLY);
                Matrix_MultVec3f(&D_809B8080, &this->unk_158[i + 1]);
            }
        } else {
            if (this->unk_31A == 10) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_KONB_BOUND2_OLD);
            }
            if (this->unk_31A != 0) {
                this->unk_31C = 8000;
                this->actor.speed = 30.0f;
                phi_fp = Math_Vec3f_Yaw(&this->actor.world.pos, &this->unk_2FC);
                temp = Math_Vec3f_Pitch(&this->actor.world.pos, &this->unk_158[0]) + 0x8000;
                Math_SmoothStepToS(&this->actor.shape.rot.y, phi_fp, 1, this->unk_31C, 0);
                Math_SmoothStepToS(&this->actor.shape.rot.x, temp, 1, this->unk_31C, 0);
                Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                                 MTXMODE_NEW);
                Matrix_RotateZYX(this->actor.shape.rot.x - 0x8000, this->actor.shape.rot.y, 0, MTXMODE_APPLY);
                Matrix_MultVec3f(&D_809B8080, this->unk_158);

                for (i = 0; i < 13; i++) {
                    temp = -Math_CosS(this->unk_31A * 0xCCC) * (i * 1200);
                    Math_SmoothStepToS(&this->unk_2A8[i].x, temp - 0x4000, 1, this->unk_31C, 0);
                    Math_SmoothStepToS(&this->unk_2A8[i].y, phi_fp, 1, this->unk_31C, 0);
                    Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
                    Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
                    Matrix_MultVec3f(&D_809B8080, &this->unk_158[i + 1]);
                }
                this->unk_31A--;
            } else if ((this->actor.xzDistToPlayer > 175.0f) || (player->stateFlags1 & PLAYER_STATE1_4000000)) {
                EnBa_SetupIdle(this);
            } else {
                EnBa_SetupSwingAtPlayer(this);
                this->unk_318 = 27;
                this->unk_31C = 750;
            }
        }
        this->unk_2A8[13].x = this->unk_2A8[12].x;
        this->unk_2A8[13].y = this->unk_2A8[12].y;
        if (this->collider.base.atFlags & 2) {
            this->collider.base.atFlags &= ~2;
            if (this->collider.base.at == &player->actor) {
                func_800B8D98(play, &this->actor, 8.0f, this->actor.yawTowardsPlayer, 8.0f);
            }
        }
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        return;
    }
    if ((this->actor.xzDistToPlayer > 175.0f) || (player->stateFlags1 & PLAYER_STATE1_4000000)) {
        EnBa_SetupIdle(this);
    } else {
        EnBa_SetupSwingAtPlayer(this);
        this->unk_318 = 27;
        this->unk_31C = 750;
    }
}

static void func_809B7174(EnBa* this) {
    this->unk_14C = 1;
    this->unk_31C = 1500;
    this->unk_318 = 20;
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->actor.speed = 10.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_KONB_DEAD_JUMP2_OLD);
    Actor_SetColorFilter(&this->actor, 0x4000, 255, 0, 12);
    EnBa_SetupAction(this, EnBa_RecoilFromDamage);
}

static void EnBa_RecoilFromDamage(EnBa* this, PlayState* play) {
    s32 i;
    Vec3s sp6C;

    Math_SmoothStepToF(&this->actor.world.pos.y, this->actor.home.pos.y + 330.0f, 1.0f, 30.0f, 0.0f);
    this->unk_2FC = this->actor.world.pos;
    if (play->gameplayFrames % 16 == 0) {
        this->unk_308.z += Rand_CenteredFloat(180.0f);
        this->unk_314 += Rand_CenteredFloat(180.0f);
        this->unk_308.x = Math_SinF(this->unk_308.z) * 80.0f;
        this->unk_308.y = Math_CosF(this->unk_314) * 80.0f;
    }
    this->unk_2FC.y -= 448.0f;
    this->unk_2FC.x += this->unk_308.x;
    this->unk_2FC.z += this->unk_308.y;
    func_80033AEC(&this->unk_2FC, &this->unk_158[13], 1.0f, this->actor.speed, 0.0f, 0.0f);
    for (i = 12; i >= 0; i--) {
        Math_Vec3fDiffToRotOot(&this->unk_158[i + 1], &this->unk_158[i], &sp6C, 0);
        Matrix_Translate(this->unk_158[i + 1].x, this->unk_158[i + 1].y, this->unk_158[i + 1].z, MTXMODE_NEW);
        Matrix_RotateZYX(sp6C.x, sp6C.y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&D_809B8080, &this->unk_158[i]);
    }
    Math_Vec3fDiffToRotOot(&this->actor.world.pos, &this->unk_158[0], &sp6C, 0);
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Math_SmoothStepToS(&this->actor.shape.rot.y, sp6C.y, 3, this->unk_31C, 182);
    Math_SmoothStepToS(&this->actor.shape.rot.x, sp6C.x + 0x8000, 3, this->unk_31C, 182);
    Matrix_RotateZYX(this->actor.shape.rot.x - 0x8000, this->actor.shape.rot.y, 0, MTXMODE_APPLY);
    Matrix_MultVec3f(&D_809B8080, &this->unk_158[0]);

    for (i = 0; i < 13; i++) {
        Math_Vec3fDiffToRotOot(&this->unk_158[i], &this->unk_158[i + 1], &sp6C, 0);
        Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
        Math_SmoothStepToS(&this->unk_2A8[i].y, sp6C.y, 3, this->unk_31C, 182);
        Math_SmoothStepToS(&this->unk_2A8[i].x, sp6C.x + 0x8000, 3, this->unk_31C, 182);
        Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&D_809B8080, &this->unk_158[i + 1]);
    }

    this->unk_2A8[13].x = this->unk_2A8[12].x;
    this->unk_2A8[13].y = this->unk_2A8[12].y;
    this->unk_318--;
    if (this->unk_318 == 0) {
        EnBa_SetupIdle(this);
    }
}

static void func_809B75A0(EnBa* this, PlayState* play2) {
    s16 unk_temp;
    s32 i;
    Vec3f sp74 = { 0.0f, 0.0f, 0.0f };
    PlayState* play = play2;

    this->unk_31C = 2500;
    EffectSsDeadSound_SpawnStationary(play, &this->actor.projectedPos, NA_SE_EN_FROG_JUMP, 1, 1, 40);
    this->unk_14C = 0;

    for (i = 7; i < 14; i++) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BA_Oot, this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, 0,
                    0, 0, EN_BA_DEAD_BLOB);
    }
    unk_temp = Math_Vec3f_Pitch(&this->actor.world.pos, &this->unk_158[0]) + 0x8000;
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, this->unk_31C, 0);
    Math_SmoothStepToS(&this->actor.shape.rot.x, unk_temp, 1, this->unk_31C, 0);
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateZYX(this->actor.shape.rot.x - 0x8000, this->actor.shape.rot.y, 0, MTXMODE_APPLY);
    Matrix_MultVec3f(&D_809B8080, &this->unk_158[0]);
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    for (i = 5; i < 13; i++) {
        Math_SmoothStepToS(&this->unk_2A8[i].x, this->unk_2A8[5].x, 1, this->unk_31C, 0);
        Math_SmoothStepToS(&this->unk_2A8[i].y, this->unk_2A8[5].y, 1, this->unk_31C, 0);
        Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
        Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&sp74, &this->unk_158[i + 1]);
    }
    this->unk_31A = 15;
    EnBa_SetupAction(this, EnBa_Die);
}

static void EnBa_Die(EnBa* this, PlayState* play) {
    Vec3f sp6C = { 0.0f, 0.0f, 0.0f };
    s16 temp;
    s32 i;

    if (this->unk_31A != 0) {
        this->actor.speed = 30.0f;
        this->unk_31C = 8000;
        this->actor.world.pos.y += 8.0f;
        temp = Math_Vec3f_Pitch(&this->actor.world.pos, &this->unk_158[0]) + 0x8000;
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, this->unk_31C, 0);
        Math_SmoothStepToS(&this->actor.shape.rot.x, temp, 1, this->unk_31C, 0);
        Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
        Matrix_RotateZYX(this->actor.shape.rot.x - 0x8000, this->actor.shape.rot.y, 0, MTXMODE_APPLY);
        Matrix_MultVec3f(&D_809B8080, &this->unk_158[0]);
        for (i = 0; i < 5; i++) {
            temp = -Math_CosS(this->unk_31A * 0x444) * (i * 400);
            Math_SmoothStepToS(&this->unk_2A8[i].x, temp - 0x4000, 1, this->unk_31C, 0);
            Math_SmoothStepToS(&this->unk_2A8[i].y, this->actor.yawTowardsPlayer, 1, this->unk_31C, 0);
            Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
            Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
            Matrix_MultVec3f(&D_809B8080, &this->unk_158[i + 1]);
        }
        for (i = 5; i < 13; i++) {
            Math_SmoothStepToS(&this->unk_2A8[i].x, this->unk_2A8[5].x, 1, this->unk_31C, 0);
            Math_SmoothStepToS(&this->unk_2A8[i].y, this->unk_2A8[5].y, 1, this->unk_31C, 0);
            Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
            Matrix_RotateZYX(this->unk_2A8[i].x - 0x8000, this->unk_2A8[i].y, 0, MTXMODE_APPLY);
            Matrix_MultVec3f(&sp6C, &this->unk_158[i + 1]);
        }
        this->unk_31A--;
    } else {
        Flags_SetSwitch(play, this->upperParams);
        Actor_Kill(&this->actor);
    }
}

static void EnBa_Update(Actor* thisx, PlayState* play) {
    EnBa* this = (EnBa*)thisx;

    if ((this->actor.params < EN_BA_DEAD_BLOB) && (this->collider.base.acFlags & 2)) {
        this->collider.base.acFlags &= ~2;
        this->actor.colChkInfo.health--;
        if (this->actor.colChkInfo.health == 0) {
            func_809B75A0(this, play);
        } else {
            func_809B7174(this);
        }
    }
    this->actionFunc(this, play);
    if (this->actor.params < EN_BA_DEAD_BLOB) {
        this->actor.focus.pos = this->unk_158[6];
    }
    if (this->unk_14C >= 2) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void* D_809B8118[] = {
    object_bxa_Tex_0024F0,
    object_bxa_Tex_0027F0,
    object_bxa_Tex_0029F0,
};

static void EnBa_Draw(Actor* thisx, PlayState* play) {
    EnBa* this = (EnBa*)thisx;
    s32 pad;
    s16 i;
    Mtx* mtx = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx) * 14);
    Vec3f unused = { 0.0f, 0.0f, 448.0f };

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    if (this->actor.params < EN_BA_DEAD_BLOB) {
        Matrix_Push();
        gSPSegment(POLY_OPA_DISP++, 0x0C, mtx);
        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(D_809B8118[this->actor.params]));
        gSPSegment(POLY_OPA_DISP++, 0x09,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 16, 16, 1, 0, (play->gameplayFrames * -10) % 128, 32,
                                      32, 0, 0, 0, -10));
        for (i = 0; i < 14; i++, mtx++) {
            FrameInterpolation_RecordOpenChild(this, this->epoch + i * 25);

            Matrix_Translate(this->unk_158[i].x, this->unk_158[i].y, this->unk_158[i].z, MTXMODE_NEW);
            Matrix_RotateZYX(this->unk_2A8[i].x, this->unk_2A8[i].y, this->unk_2A8[i].z, MTXMODE_APPLY);
            Matrix_Scale(this->unk_200[i].x, this->unk_200[i].y, this->unk_200[i].z, MTXMODE_APPLY);
            if ((i == 6) || (i == 13)) {
                switch (i) {
                    case 13:
                        Collider_UpdateSpheres(i, &this->collider);
                        break;
                    default:
                        Matrix_Scale(0.5f, 0.5f, 1.0f, MTXMODE_APPLY);
                        Collider_UpdateSpheres(8, &this->collider);
                        break;
                }
            }
            Matrix_ToMtx(mtx);

            FrameInterpolation_RecordCloseChild();
        }
        Matrix_Pop();
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, object_bxa_DL_000890);
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (play->gameplayFrames * 2) % 128,
                                      (play->gameplayFrames * 2) % 128, 32, 32, 1, (play->gameplayFrames * -5) % 128,
                                      (play->gameplayFrames * -5) % 128, 32, 32, 2, 2, -5, -5));
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 125, 100, 255);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, object_bxa_DL_001D80);
    }
    CLOSE_DISPS(play->state.gfxCtx);
}
