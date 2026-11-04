/*
 * File: z_en_guest.c
 * Overlay: ovl_En_Guest
 * Description: Happy Mask Shop Customer
 */

#include "z_en_guest.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_os_anime/object_os_anime.h"
#include "assets/oot/objects/object_boj/object_boj.h"
#include <assert.h>

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_UPDATE_CULLING_DISABLED)

static void EnGuest_Init(Actor* thisx, PlayState* play);
static void EnGuest_Destroy(Actor* thisx, PlayState* play);
static void EnGuest_Update(Actor* thisx, PlayState* play);
static void EnGuest_Draw(Actor* thisx, PlayState* play);

static void func_80A50518(EnGuest* this, PlayState* play);
static void func_80A5057C(EnGuest* this, PlayState* play);
static void func_80A505CC(Actor* thisx, PlayState* play);

ActorProfile En_Guest_Oot_Profile = {
    /**/ ACTOR_EN_GUEST_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_BOJ_Oot,
    /**/ sizeof(EnGuest),
    /**/ EnGuest_Init,
    /**/ EnGuest_Destroy,
    /**/ EnGuest_Update,
    /**/ NULL,
};

static ColliderCylinderInitType1 sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        COLSHAPE_CYLINDER,
    },
    { 0x00, { 0x00000000, 0x00, 0x00 }, { 0x00000000, 0x00, 0x00 }, 0x00, 0x00, 0x01 },
    { 10, 60, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 6, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 500, ICHAIN_STOP),
};

static void EnGuest_Init(Actor* thisx, PlayState* play) {
    EnGuest* this = (EnGuest*)thisx;

    if (GET_INFTABLE_OOT(INFTABLE_SHOWED_ZELDAS_LETTER_TO_GATE_GUARD_Oot)) {
        Actor_Kill(&this->actor);
    } else {
        this->osAnimeBankIndex = Object_GetSlot(&play->objectCtx, OBJECT_OS_ANIME_Oot);
        if (this->osAnimeBankIndex < 0) {
            assert(this->osAnimeBankIndex < 0);
        }
    }
}

static void EnGuest_Destroy(Actor* thisx, PlayState* play) {
    EnGuest* this = (EnGuest*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void EnGuest_Update(Actor* thisx, PlayState* play) {
    EnGuest* this = (EnGuest*)thisx;
    s32 pad;

    if (Object_IsLoaded(&play->objectCtx, this->osAnimeBankIndex)) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_CULLING_DISABLED;
        Actor_ProcessInitChain(&this->actor, sInitChain);

        SkelAnime_InitFlex(play, &this->skelAnime, &object_boj_Skel_0000F0, NULL, this->jointTable, this->morphTable,
                           16);
        gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->osAnimeBankIndex].segment);
        Animation_Change(&this->skelAnime, &gObjOsAnim_42AC, 1.0f, 0.0f, Animation_GetLastFrame(&gObjOsAnim_42AC),
                         ANIMMODE_LOOP, 0.0f);

        this->actor.draw = EnGuest_Draw;
        this->actor.update = func_80A505CC;

        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinderType1(play, &this->collider, &this->actor, &sCylinderInit);

        Actor_SetFocus(&this->actor, 60.0f);

        this->unk_30E = 0;
        this->unk_30D = 0;
        this->unk_2CA = 0;
        this->actor.textId = TEXT_OOT(0x700D);
        this->actionFunc = func_80A50518;
    }
}

static void func_80A5046C(EnGuest* this) {
    if (this->unk_30D == 0) {
        if (this->unk_2CA != 0) {
            this->unk_2CA--;
        } else {
            this->unk_30D = 1;
        }
    } else {
        if (this->unk_2CA != 0) {
            this->unk_2CA--;
        } else {
            this->unk_30E += 1;
            if (this->unk_30E >= 3) {
                this->unk_30E = 0;
                this->unk_30D = 0;
                this->unk_2CA = (s32)Rand_ZeroFloat(60.0f) + 20;
            } else {
                this->unk_2CA = 1;
            }
        }
    }
}

static void func_80A50518(EnGuest* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = func_80A5057C;
    } else if (this->actor.xzDistToPlayer < 100.0f) {
        Actor_OfferTalk(&this->actor, play, 100.0f);
    }
}

static void func_80A5057C(EnGuest* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(play)) {
        this->actionFunc = func_80A50518;
    }
}

static void func_80A505CC(Actor* thisx, PlayState* play) {
    EnGuest* this = (EnGuest*)thisx;
    s32 pad;
    Player* player;

    player = GET_PLAYER(play);
    this->unk_2C8++;

    func_80A5046C(this);
    this->actionFunc(this, play);

    this->interactInfo.trackPos = player->actor.world.pos;
    if (LINK_IS_ADULT_OOT) {
        this->interactInfo.yOffset = 10.0f;
    } else {
        this->interactInfo.yOffset = 20.0f;
    }
    Npc_TrackPoint(&this->actor, &this->interactInfo, 6, NPC_TRACKING_HEAD_AND_TORSO);

    Actor_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, 16);

    gSegments[6] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[this->osAnimeBankIndex].segment);

    SkelAnime_Update(&this->skelAnime);
    Actor_SetFocus(&this->actor, 60.0f);

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

Gfx* func_80A50708(GraphicsContext* gfxCtx, u8 r, u8 g, u8 b, u8 a) {
    Gfx* dlist;

    dlist = GRAPH_ALLOC(gfxCtx, 2 * sizeof(Gfx));
    gDPSetEnvColor(dlist, r, g, b, a);
    gSPEndDisplayList(dlist + 1);

    return dlist;
}

static s32 EnGuest_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnGuest* this = (EnGuest*)thisx;
    Vec3s sp3C;

    OPEN_DISPS(play->state.gfxCtx);

    if (limbIndex == 15) {
        *dList = object_boj_DL_0059B0;
        Matrix_Translate(1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        sp3C = this->interactInfo.headRot;
        Matrix_RotateXFApply((sp3C.y / 32768.0f) * M_PI);
        Matrix_RotateZF((sp3C.x / 32768.0f) * M_PI, MTXMODE_APPLY);
        Matrix_Translate(-1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }

    if (limbIndex == 8) {
        sp3C = this->interactInfo.torsoRot;
        Matrix_RotateXFApply((-sp3C.y / 32768.0f) * M_PI);
        Matrix_RotateZF((sp3C.x / 32768.0f) * M_PI, MTXMODE_APPLY);
    }

    if (limbIndex == 8 || limbIndex == 9 || limbIndex == 12) {
        rot->y += Math_SinS(this->fidgetTableY[limbIndex]) * 200.0f;
        rot->z += Math_CosS(this->fidgetTableZ[limbIndex]) * 200.0f;
    }

    CLOSE_DISPS(play->state.gfxCtx);

    return false;
}

static void EnGuest_Draw(Actor* thisx, PlayState* play) {
    static void* D_80A50BA4[] = {
        object_boj_Tex_0005FC,
        object_boj_Tex_0006FC,
        object_boj_Tex_0007FC,
    };
    EnGuest* this = (EnGuest*)thisx;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, func_80A50708(play->state.gfxCtx, 255, 255, 255, 255));
    gSPSegment(POLY_OPA_DISP++, 0x09, func_80A50708(play->state.gfxCtx, 160, 60, 220, 255));
    gSPSegment(POLY_OPA_DISP++, 0x0A, Lib_SegmentedToVirtual(D_80A50BA4[this->unk_30E]));

    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnGuest_OverrideLimbDraw, NULL, this);

    CLOSE_DISPS(play->state.gfxCtx);
}
