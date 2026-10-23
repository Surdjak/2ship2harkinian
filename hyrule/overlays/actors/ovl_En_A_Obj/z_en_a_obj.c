/*
 * File: z_en_a_obj.c
 * Overlay: En_A_Obj (part of code in OoT)
 * Description: Signposts, blocks, grass clumps, tree stumps and boulder fragments
 *
 * 3Ship [Hyrule] Ported from SoH's z_en_a_keep.c. See hyrule/README.md for the porting conventions. MM has
 * its own EnAObjOot, so this one is EnAObjOot.
 */

#include "z_en_a_obj.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
#include "assets/oot/objects/object_d_hsblock/object_d_hsblock.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnAObj_Init(Actor* thisx, PlayState* play);
static void EnAObj_Destroy(Actor* thisx, PlayState* play);
static void EnAObj_Update(Actor* thisx, PlayState* play);
static void EnAObj_Draw(Actor* thisx, PlayState* play);

static void EnAObj_WaitFinishedTalking(EnAObjOot* this, PlayState* play);
static void EnAObj_WaitTalk(EnAObjOot* this, PlayState* play);
static void EnAObj_BlockRot(EnAObjOot* this, PlayState* play);
static void EnAObj_BoulderFragment(EnAObjOot* this, PlayState* play);
static void EnAObj_Block(EnAObjOot* this, PlayState* play);

static void EnAObj_SetupWaitTalk(EnAObjOot* this, s16 type);
static void EnAObj_SetupBlockRot(EnAObjOot* this, s16 type);
static void EnAObj_SetupBoulderFragment(EnAObjOot* this, s16 type);
static void EnAObj_SetupBlock(EnAObjOot* this, s16 type);

ActorProfile En_A_Obj_Oot_Profile = {
    /**/ ACTOR_EN_A_OBJ_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnAObjOot),
    /**/ EnAObj_Init,
    /**/ EnAObj_Destroy,
    /**/ EnAObj_Update,
    /**/ EnAObj_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_ALL,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { 0x00000000, 0x00, 0x00 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 25, 60, 0, { 0, 0, 0 } },
};

static CollisionHeader* sColHeaders[] = {
    &gLargerCubeCol,       // A_OBJ_GRASS_CLUMP, A_OBJ_TREE_STUMP
    &gLargerCubeCol,       // A_OBJ_BLOCK_LARGE, A_OBJ_BLOCK_HUGE
    &gSmallerFlatBlockCol, // unused
    &gLargerFlatBlockCol,  // A_OBJ_BLOCK_SMALL_ROT, A_OBJ_BLOCK_LARGE_ROT
    &gSmallerCubeCol,      // unused
    //&D_06000730,           // A_OBJ_UNKNOWN_6 // OTRTODO
};

static Gfx* sDLists[] = {
    gFlatBlockDL,
    gFlatBlockDL,
    gFlatBlockDL,
    gFlatRotBlockDL,
    gFlatRotBlockDL,
    gSmallCubeDL,
    gHookshotPostDL,
    /* gHookshotPostDL ? */ // 0x06000210, // OTRTODO!
    gGrassBladesDL,
    gTreeStumpDL,
    gSignRectangularDL,
    gSignDirectionalDL,
    gBoulderFragmentsDL,
};

static void EnAObj_SetupAction(EnAObjOot* this, EnAObjOotActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void EnAObj_Init(Actor* thisx, PlayState* play) {
    CollisionHeader* colHeader = NULL;
    s32 pad;
    EnAObjOot* this = (EnAObjOot*)thisx;
    f32 shadowScale = 6.0f;

    this->textId = (thisx->params >> 8) & 0xFF;
    thisx->params &= 0xFF;

    switch (thisx->params) {
        case A_OBJ_BLOCK_SMALL:
            Actor_SetScale(thisx, 0.025f);
            break;
        case A_OBJ_BLOCK_LARGE:
            Actor_SetScale(thisx, 0.05f);
            break;
        case A_OBJ_BLOCK_HUGE:
        case A_OBJ_CUBE_SMALL:
        case A_OBJ_UNKNOWN_6:
            Actor_SetScale(thisx, 0.1f);
            break;
        case A_OBJ_BLOCK_SMALL_ROT:
            Actor_SetScale(thisx, 0.005f);
            break;
        case A_OBJ_BLOCK_LARGE_ROT:
        default:
            Actor_SetScale(thisx, 0.01f);
            break;
    }

    if (thisx->params >= A_OBJ_SIGNPOST_OBLONG) {
        shadowScale = 12.0f;
    }

    ActorShape_Init(&thisx->shape, 0.0f, ActorShadow_DrawCircle, shadowScale);

    thisx->focus.pos = thisx->world.pos;
    this->dyna.bgId = BGACTOR_NEG_ONE;
    this->dyna.interactFlags = 0;
    this->dyna.transformFlags = DYNA_TRANSFORM_POS | DYNA_TRANSFORM_ROT_Y;
    thisx->cullingVolumeDownward = 1200.0f;
    thisx->cullingVolumeScale = 200.0f;

    switch (thisx->params) {
        case A_OBJ_BLOCK_LARGE:
        case A_OBJ_BLOCK_HUGE:
            this->dyna.bgId = 1;
            Actor_ChangeCategory(play, &play->actorCtx, thisx, ACTORCAT_BG);
            EnAObj_SetupBlock(this, thisx->params);
            break;
        case A_OBJ_BLOCK_SMALL_ROT:
        case A_OBJ_BLOCK_LARGE_ROT:
            this->dyna.bgId = 3;
            Actor_ChangeCategory(play, &play->actorCtx, thisx, ACTORCAT_BG);
            EnAObj_SetupBlockRot(this, thisx->params);
            break;
        case A_OBJ_UNKNOWN_6:
            // clang-format off
            thisx->flags |= ACTOR_FLAG_ATTENTION_ENABLED; this->dyna.bgId = 5; this->focusYoffset = 10.0f;
            // clang-format on
            thisx->gravity = -2.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
        case A_OBJ_GRASS_CLUMP:
        case A_OBJ_TREE_STUMP:
            this->dyna.bgId = 0;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            thisx->textId = TEXT_OOT((this->textId & 0xFF) | 0x300);
            // clang-format off
            thisx->flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY; thisx->lockOnArrowOffset = 500.0f;
            // clang-format on
            this->focusYoffset = 45.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            Collider_InitCylinder(play, &this->collider);
            Collider_SetCylinder(play, &this->collider, thisx, &sCylinderInit);
            thisx->colChkInfo.mass = MASS_IMMOVABLE;
            thisx->attentionRangeType = ATTENTION_RANGE_0;
            break;
        case A_OBJ_BOULDER_FRAGMENT:
            thisx->gravity = -1.5f;
            EnAObj_SetupBoulderFragment(this, thisx->params);
            break;
        default:
            thisx->gravity = -2.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
    }

    if (thisx->params <= A_OBJ_BLOCK_LARGE_ROT) { // A_OBJ_BLOCK_*
        thisx->colChkInfo.mass = MASS_IMMOVABLE;
    }

    if (this->dyna.bgId != BGACTOR_NEG_ONE) {
        CollisionHeader_GetVirtual(sColHeaders[this->dyna.bgId], &colHeader);
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    }
}

static void EnAObj_Destroy(Actor* thisx, PlayState* play) {
    EnAObjOot* this = (EnAObjOot*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);

    switch (this->dyna.actor.params) {
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            Collider_DestroyCylinder(play, &this->collider);
            break;
    }
}

static void EnAObj_WaitFinishedTalking(EnAObjOot* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->dyna.actor, play)) {
        EnAObj_SetupWaitTalk(this, this->dyna.actor.params);
    }
}

static void EnAObj_SetupWaitTalk(EnAObjOot* this, s16 type) {
    EnAObj_SetupAction(this, EnAObj_WaitTalk);
}

static void EnAObj_WaitTalk(EnAObjOot* this, PlayState* play) {
    s16 relYawTowardsPlayer;

    if (this->dyna.actor.textId != 0) {
        relYawTowardsPlayer = this->dyna.actor.yawTowardsPlayer - this->dyna.actor.shape.rot.y;
        if (ABS(relYawTowardsPlayer) < 0x2800 ||
            (this->dyna.actor.params == A_OBJ_SIGNPOST_ARROW && ABS(relYawTowardsPlayer) > 0x5800)) {
            if (Actor_TalkOfferAccepted(&this->dyna.actor, &play->state)) {
                EnAObj_SetupAction(this, EnAObj_WaitFinishedTalking);
            } else {
                Actor_OfferTalkNearColChkInfoCylinder(&this->dyna.actor, play);
            }
        }
    }
}

static void EnAObj_SetupBlockRot(EnAObjOot* this, s16 type) {
    this->rotateState = 0;
    this->rotateWaitTimer = 10;
    this->dyna.actor.world.rot.y = 0;
    this->dyna.actor.shape.rot = this->dyna.actor.world.rot;
    EnAObj_SetupAction(this, EnAObj_BlockRot);
}

static void EnAObj_BlockRot(EnAObjOot* this, PlayState* play) {
    if (this->rotateState == 0) {
        if (this->dyna.interactFlags != 0) {
            this->rotateState++;
            this->rotateForTimer = 20;

            if ((s16)(this->dyna.actor.yawTowardsPlayer + 0x4000) < 0) {
                this->rotSpeedX = -0x3E8;
            } else {
                this->rotSpeedX = 0x3E8;
            }

            if (this->dyna.actor.yawTowardsPlayer < 0) {
                this->rotSpeedY = -this->rotSpeedX;
            } else {
                this->rotSpeedY = this->rotSpeedX;
            }
        }
    } else {
        if (this->rotateWaitTimer != 0) {
            this->rotateWaitTimer--;
        } else {
            this->dyna.actor.shape.rot.y += this->rotSpeedY;
            this->dyna.actor.shape.rot.x += this->rotSpeedX;
            this->rotateForTimer--;
            this->dyna.actor.gravity = -1.0f;

            if (this->rotateForTimer == 0) {
                this->dyna.actor.world.pos = this->dyna.actor.home.pos;
                this->rotateState = 0;
                this->rotateWaitTimer = 10;
                this->dyna.actor.velocity.y = 0.0f;
                this->dyna.actor.gravity = 0.0f;
                this->dyna.actor.shape.rot = this->dyna.actor.world.rot;
            }
        }
    }
}

static void EnAObj_SetupBoulderFragment(EnAObjOot* this, s16 type) {
    EnAObj_SetupAction(this, EnAObj_BoulderFragment);
}

static void EnAObj_BoulderFragment(EnAObjOot* this, PlayState* play) {
    Math_SmoothStepToF(&this->dyna.actor.speed, 1.0f, 1.0f, 0.5f, 0.0f);
    this->dyna.actor.shape.rot.x += this->dyna.actor.world.rot.x >> 1;
    this->dyna.actor.shape.rot.z += this->dyna.actor.world.rot.z >> 1;

    if (this->dyna.actor.speed != 0.0f && this->dyna.actor.bgCheckFlags & 0x8) {
        this->dyna.actor.world.rot.y =
            this->dyna.actor.wallYaw - this->dyna.actor.world.rot.y + this->dyna.actor.wallYaw - 0x8000;
        this->dyna.actor.bgCheckFlags &= ~0x8;
    }

    if (this->dyna.actor.bgCheckFlags & 0x2) {
        if (this->dyna.actor.velocity.y < -8.0f) {
            this->dyna.actor.velocity.y *= -0.6f;
            this->dyna.actor.speed *= 0.6f;
            this->dyna.actor.bgCheckFlags &= ~0x3;
        } else {
            Actor_Kill(&this->dyna.actor);
        }
    }
}

static void EnAObj_SetupBlock(EnAObjOot* this, s16 type) {
    this->dyna.actor.cullingVolumeDownward = 1200.0f;
    this->dyna.actor.cullingVolumeScale = 720.0f;
    EnAObj_SetupAction(this, EnAObj_Block);
}

static void EnAObj_Block(EnAObjOot* this, PlayState* play) {
    this->dyna.actor.speed += this->dyna.pushForce;
    this->dyna.actor.world.rot.y = this->dyna.yRotation;
    this->dyna.actor.speed = CLAMP(this->dyna.actor.speed, -2.5f, 2.5f);

    Math_SmoothStepToF(&this->dyna.actor.speed, 0.0f, 1.0f, 1.0f, 0.0f);

    if (this->dyna.actor.speed != 0.0f) {
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_ROCK_SLIDE - SFX_FLAG);
    }

    this->dyna.unk14C = 0.0f;
    this->dyna.pushForce = 0.0f;
}

static void EnAObj_Update(Actor* thisx, PlayState* play) {
    EnAObjOot* this = (EnAObjOot*)thisx;

    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->dyna.actor);

    if (this->dyna.actor.gravity != 0.0f) {
        if (this->dyna.actor.params != A_OBJ_BOULDER_FRAGMENT) {
            Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 5.0f, 40.0f, 0.0f, 0x1D);
        } else {
            Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 5.0f, 20.0f, 0.0f, 0x1D);
        }
    }

    this->dyna.actor.focus.pos = this->dyna.actor.world.pos;
    this->dyna.actor.focus.pos.y += this->focusYoffset;

    switch (this->dyna.actor.params) {
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            Collider_UpdateCylinder(&this->dyna.actor, &this->collider);
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            break;
    }
}

static void EnAObj_Draw(Actor* thisx, PlayState* play) {
    s32 type = thisx->params;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    if (type >= A_OBJ_MAX) {
        type = A_OBJ_BOULDER_FRAGMENT;
    }

    if (thisx->params == A_OBJ_BOULDER_FRAGMENT) {
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 1, 60, 60, 60, 50);
    }

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDLists[type]);

    CLOSE_DISPS(play->state.gfxCtx);
}
