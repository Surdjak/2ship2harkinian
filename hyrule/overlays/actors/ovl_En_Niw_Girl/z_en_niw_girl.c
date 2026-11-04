/*
 * File: z_en_niw_girl.c
 * Overlay: ovl_En_Niw_Girl
 * Description: Girl that chases a cuckoo around in Hyrule Market and Kakariko Village
 */

#include "z_en_niw_girl.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_gr/object_gr.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnNiwGirl_Init(Actor* thisx, PlayState* play);
static void EnNiwGirl_Destroy(Actor* thisx, PlayState* play);
static void EnNiwGirl_Update(Actor* thisx, PlayState* play);
static void EnNiwGirl_Draw(Actor* thisx, PlayState* play);

static void EnNiwGirl_Talk(EnNiwGirl* this, PlayState* play);
static void func_80AB94D0(EnNiwGirl* this, PlayState* play);
static void func_80AB9210(EnNiwGirl* this, PlayState* play);

ActorProfile En_Niw_Girl_Oot_Profile = {
    /**/ ACTOR_EN_NIW_GIRL_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_GR_Oot,
    /**/ sizeof(EnNiwGirl),
    /**/ EnNiwGirl_Init,
    /**/ EnNiwGirl_Destroy,
    /**/ EnNiwGirl_Update,
    /**/ EnNiwGirl_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 10, 30, 0, { 0, 0, 0 } },
};

static void EnNiwGirl_Init(Actor* thisx, PlayState* play) {
    EnNiwGirl* this = (EnNiwGirl*)thisx;
    s32 pad;
    Vec3f vec1;
    Vec3f vec2;
    s32 pad2;

    SkelAnime_InitFlex(play, &this->skelAnime, &gNiwGirlSkel, &gNiwGirlRunAnim, this->jointTable, this->morphTable, 17);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.attentionRangeType = 6;
    if (this->actor.params < 0) {
        this->actor.params = 0;
    }
    this->path = ((this->actor.params >> 8) & 0xFF);
    this->actor.gravity = -3.0f;
    Matrix_RotateYF((this->actor.shape.rot.y / 32768.0f) * M_PI, MTXMODE_NEW);
    vec2.x = vec2.y = vec2.z = 0.0f;
    vec1.x = vec1.y = 0.0f;
    vec1.z = 50.0;
    Matrix_MultVec3f(&vec1, &vec2);
    this->chasedEnNiw = (EnNiw*)Actor_SpawnAsChild(
        &play->actorCtx, &this->actor, play, ACTOR_EN_NIW_Oot, this->actor.world.pos.x + vec2.x,
        this->actor.world.pos.y + vec2.y, this->actor.world.pos.z + vec2.z, 0, this->actor.world.rot.y, 0, 0xA);
    if (this->chasedEnNiw != NULL) {
        this->actor.colChkInfo.mass = MASS_IMMOVABLE;
        this->actionFunc = EnNiwGirl_Talk;
    } else {
        Actor_Kill(&this->actor);
    }
}

static void EnNiwGirl_Destroy(Actor* thisx, PlayState* play) {
}

static void EnNiwGirl_Jump(EnNiwGirl* this, PlayState* play) {
    f32 frameCount = Animation_GetLastFrame(&gNiwGirlRunAnim);
    Animation_Change(&this->skelAnime, &gNiwGirlRunAnim, 1.0f, 0.0f, frameCount, 0, -10.0f);
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->actionFunc = func_80AB9210;
}

static void func_80AB9210(EnNiwGirl* this, PlayState* play) {
    Path* path = &play->setupPathList[this->path];
    f32 xDistBetween;
    f32 zDistBetween;

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->actor.speed, 3.0f, 0.2f, 0.4f);

    // Find the X and Z distance between the girl and the cuckoo she is chasing
    xDistBetween = this->chasedEnNiw->actor.world.pos.x - this->actor.world.pos.x;
    zDistBetween = this->chasedEnNiw->actor.world.pos.z - this->actor.world.pos.z;
    if (Message_GetState(&play->msgCtx) != TEXT_STATE_NONE) {
        this->chasedEnNiw->path = 0;
    }
    if (sqrtf(SQ(xDistBetween) + SQ(zDistBetween)) < 70.0f) {
        this->chasedEnNiw->path = (this->path + 1);
        this->chasedEnNiw->unk_2EC = path->count;
    } else if (sqrtf(SQ(xDistBetween) + SQ(zDistBetween)) > 150.0f) {
        this->chasedEnNiw->path = 0;
    }

    // Change her angle so that she is always facing the cuckoo
    Math_SmoothStepToS(&this->actor.shape.rot.y, Math_FAtan2F(xDistBetween, zDistBetween) * (0x8000 / M_PI), 3,
                       this->unk_27C, 0);
    Math_ApproachF(&this->unk_27C, 5000.0f, 30.0f, 150.0f);
    this->actor.world.rot.y = this->actor.shape.rot.y;

    // Only allow Link to talk to her when she is playing the jumping animation
    if ((this->jumpTimer == 0) || (Player_GetMaskOot(play) != PLAYER_MASK_NONE_Oot)) {
        this->jumpTimer = 60;
        this->actionFunc = EnNiwGirl_Talk;
    }
}

static void EnNiwGirl_Talk(EnNiwGirl* this, PlayState* play) {
    Animation_Change(&this->skelAnime, &gNiwGirlJumpAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gNiwGirlJumpAnim), 0,
                     -10.0f);
    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    this->actor.textId = TEXT_OOT(0x7000);
    if ((GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot)) && (this->unk_27A == 0)) {
        this->actor.textId = TEXT_OOT(0x70EA);
    }
    switch (Player_GetMaskOot(play)) {
        case PLAYER_MASK_KEATON_Oot:
            this->actor.textId = TEXT_OOT(0x7118);
            break;
        case PLAYER_MASK_SPOOKY_Oot:
            this->actor.textId = TEXT_OOT(0x7119);
            break;
        case PLAYER_MASK_BUNNY_Oot:
        case PLAYER_MASK_ZORA_Oot:
        case PLAYER_MASK_GERUDO_Oot:
            this->actor.textId = TEXT_OOT(0x711A);
            break;
        case PLAYER_MASK_SKULL_Oot:
        case PLAYER_MASK_GORON_Oot:
        case PLAYER_MASK_TRUTH_Oot:
            this->actor.textId = TEXT_OOT(0x711B);
            break;
    }
    this->unk_270 = 6;
    this->actionFunc = func_80AB94D0;
}

static void func_80AB94D0(EnNiwGirl* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Message_GetState(&play->msgCtx) != TEXT_STATE_NONE) {
        this->chasedEnNiw->path = 0;
    }
    Math_ApproachZeroF(&this->actor.speed, 0.8f, 0.2f);
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if (this->actor.textId == TEXT_OOT(0x70EA)) {
            this->unk_27A = 1;
        }
    } else {
        if ((this->jumpTimer == 0) && Message_GetState(&play->msgCtx) == TEXT_STATE_NONE) {
            this->jumpTimer = Rand_ZeroFloat(100.0f) + 250.0f;
            this->actionFunc = EnNiwGirl_Jump;
        } else {
            Actor_OfferTalk(&this->actor, play, 100.0f);
        }
    }
}

static void EnNiwGirl_Update(Actor* thisx, PlayState* play) {
    EnNiwGirl* this = (EnNiwGirl*)thisx;
    EnNiwGirlActionFunc tempActionFunc;
    Player* player = GET_PLAYER(play);

    Actor_SetScale(&this->actor, 0.013f);
    this->unkUpTimer++;
    tempActionFunc = func_80AB94D0;
    if (this->blinkTimer == 0) {
        this->eyeIndex++;
        if (this->eyeIndex >= 3) {
            this->eyeIndex = 0;
            this->blinkTimer = (s16)Rand_ZeroFloat(60.0f) + 20;
        }
    }
    this->unk_280 = 30.0f;
    Actor_SetFocus(&this->actor, 30.0f);
    if (tempActionFunc == this->actionFunc) {
        this->interactInfo.trackPos = player->actor.world.pos;
        if (!LINK_IS_ADULT_OOT) {
            this->interactInfo.trackPos.y = player->actor.world.pos.y - 10.0f;
        }
        Npc_TrackPoint(&this->actor, &this->interactInfo, 2, NPC_TRACKING_FULL_BODY);
        this->headRot = this->interactInfo.headRot;
        this->torsoRot = this->interactInfo.torsoRot;
    } else {
        Math_SmoothStepToS(&this->torsoRot.y, 0, 5, 3000, 0);
        Math_SmoothStepToS(&this->headRot.y, 0, 5, 3000, 0);
        Math_SmoothStepToS(&this->headRot.z, 0, 5, 3000, 0);
    }
    if (this->blinkTimer != 0) {
        this->blinkTimer--;
    }
    if (this->jumpTimer != 0) {
        this->jumpTimer--;
    }
    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 100.0f, 100.0f, 200.0f, 0x41C);
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

s32 EnNiwGirlOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnNiwGirl* this = (EnNiwGirl*)thisx;

    if (limbIndex == 3) {
        rot->x += this->torsoRot.y;
    }
    if (limbIndex == 4) {
        rot->x += this->headRot.y;
        rot->z += this->headRot.z;
    }
    return false;
}

static Vec3f sConstVec3f = { 0.2f, 0.2f, 0.2f };

static void EnNiwGirl_Draw(Actor* thisx, PlayState* play) {
    static void* eyeTextures[] = { gNiwGirlEyeOpenTex, gNiwGirlEyeHalfTex, gNiwGirlEyeClosedTex };
    EnNiwGirl* this = (EnNiwGirl*)thisx;
    s32 pad;
    Vec3f sp4C = sConstVec3f;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(eyeTextures[this->eyeIndex]));
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnNiwGirlOverrideLimbDraw, NULL, this);
    func_800BC620(&this->actor.world.pos, &sp4C, 255, play);

    CLOSE_DISPS(play->state.gfxCtx);
}
