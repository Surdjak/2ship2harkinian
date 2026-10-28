#include "z_en_heishi4.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "assets/oot/objects/object_sd/object_sd.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY)

static void EnHeishi4_Init(Actor* thisx, PlayState* play);
static void EnHeishi4_Destroy(Actor* thisx, PlayState* play);
static void EnHeishi4_Update(Actor* thisx, PlayState* play);
static void EnHeishi4_Draw(Actor* thisx, PlayState* play);

static void func_80A56544(EnHeishi4* this, PlayState* play);
static void func_80A5673C(EnHeishi4* this, PlayState* play);
static void func_80A56328(EnHeishi4* this, PlayState* play);
static void func_80A563BC(EnHeishi4* this, PlayState* play);
static void func_80A56B40(EnHeishi4* this, PlayState* play);
static void func_80A56614(EnHeishi4* this, PlayState* play);
static void func_80A56874(EnHeishi4* this, PlayState* play);
static void func_80A56900(EnHeishi4* this, PlayState* play);
static void func_80A56994(EnHeishi4* this, PlayState* play);
static void func_80A56A50(EnHeishi4* this, PlayState* play);
static void func_80A56ACC(EnHeishi4* this, PlayState* play);

ActorProfile En_Heishi4_Oot_Profile = {
    /**/ ACTOR_EN_HEISHI4_Oot,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_SD_Oot,
    /**/ sizeof(EnHeishi4),
    /**/ EnHeishi4_Init,
    /**/ EnHeishi4_Destroy,
    /**/ EnHeishi4_Update,
    /**/ EnHeishi4_Draw,
};

static u32 sFaceReactionSets[] = { 6, 7 };

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
    { 33, 40, 0, { 0, 0, 0 } },
};

static void EnHeishi4_Init(Actor* thisx, PlayState* play) {
    EnHeishi4* this = (EnHeishi4*)thisx;

    Actor_SetScale(thisx, 0.01f);
    this->type = thisx->params & 0xFF;
    thisx->colChkInfo.mass = MASS_IMMOVABLE;
    this->pos = thisx->world.pos;
    thisx->attentionRangeType = 6;
    if (this->type == HEISHI4_AT_MARKET_DYING) {
        this->height = 30.0f;
        ActorShape_Init(&thisx->shape, 0.0f, NULL, 30.0f);
        SkelAnime_Init(play, &this->skelAnime, &gEnHeishiSkel, &gEnHeishiDyingGuardAnim_00C444, this->jointTable,
                       this->morphTable, 17);
    } else {
        this->height = 60.0f;
        ActorShape_Init(&thisx->shape, 0.0f, ActorShadow_DrawCircle, 30.0f);
        SkelAnime_Init(play, &this->skelAnime, &gEnHeishiSkel, &gEnHeishiIdleAnim, this->jointTable, this->morphTable,
                       17);
    }
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, thisx, &sCylinderInit);
    this->collider.dim.yShift = 0;
    this->collider.dim.radius = 15;
    this->collider.dim.height = 70;
    switch (this->type) {
        case HEISHI4_AT_KAKRIKO_ENTRANCE:
        case HEISHI4_AT_IMPAS_HOUSE:
            this->actionFunc = func_80A56328;
            break;
        case HEISHI4_AT_MARKET_DYING:
            this->collider.dim.radius = 28;
            this->collider.dim.height = 5;
            this->actionFunc = func_80A5673C;
            break;
        case HEISHI4_AT_MARKET_NIGHT:
            this->actionFunc = func_80A56544;
            break;
    }
    this->unk_27C = ((thisx->params >> 8) & 0xFF);
}

static void EnHeishi4_Destroy(Actor* thisx, PlayState* play) {
    EnHeishi4* this = (EnHeishi4*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void func_80A56328(EnHeishi4* this, PlayState* play) {
    f32 frames = Animation_GetLastFrame(&gEnHeishiIdleAnim);

    Animation_Change(&this->skelAnime, &gEnHeishiIdleAnim, 1.0f, 0.0f, (s16)frames, ANIMMODE_LOOP, -10.0f);
    this->actionFunc = func_80A563BC;
}

static void func_80A563BC(EnHeishi4* this, PlayState* play) {
    s16 reactionOffset;

    this->unk_2B4 = 0;
    reactionOffset = this->type - 4;
    if (reactionOffset < 0) {
        reactionOffset = 0;
    }
    if (reactionOffset >= 3) {
        reactionOffset = 1;
    }
    if (Text_GetFaceReactionOot(play, sFaceReactionSets[reactionOffset]) != 0) {
        this->actor.textId = Text_GetFaceReactionOot(play, sFaceReactionSets[reactionOffset]);
        this->unk_2B4 = 1;
        this->actionFunc = func_80A56B40;
    } else {
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot)) {
            this->actor.textId = TEXT_OOT(0x5065);
            this->actionFunc = func_80A56B40;
            return;
        }
        if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot)) {
            this->actor.textId = TEXT_OOT(0x5068);
            this->actionFunc = func_80A56B40;
            return;
        }
        if (this->type == HEISHI4_AT_IMPAS_HOUSE) {
            if (this->unk_284 == 0) {
                this->actor.textId = TEXT_OOT(0x5079);
            } else {
                this->actor.textId = TEXT_OOT(0x507A);
            }
        } else if (IS_DAY_OOT) {
            if (this->unk_284 == 0) {
                this->actor.textId = TEXT_OOT(0x5063);
            } else {
                this->actor.textId = TEXT_OOT(0x5064);
            }
        } else if (this->unk_284 == 0) {
            this->actor.textId = TEXT_OOT(0x5066);
        } else {
            this->actor.textId = TEXT_OOT(0x5067);
        }

        this->actionFunc = func_80A56B40;
    }
}

static void func_80A56544(EnHeishi4* this, PlayState* play) {
    f32 frames = Animation_GetLastFrame(&gEnHeishiIdleAnim);

    Animation_Change(&this->skelAnime, &gEnHeishiIdleAnim, 1.0f, 0.0f, (s16)frames, ANIMMODE_LOOP, -10.0f);
    if (LINK_AGE_IN_YEARS_OOT != YEARS_CHILD) {
        Actor_Kill(&this->actor);
    } else {
        this->actionFunc = func_80A56614;
    }
}

static void func_80A56614(EnHeishi4* this, PlayState* play) {
    s16 reactionOffset;

    reactionOffset = this->type - 4;
    this->unk_2B4 = 0;
    if (reactionOffset < 0) {
        reactionOffset = 0;
    }
    if (reactionOffset >= 3) {
        reactionOffset = 1;
    }
    if (Text_GetFaceReactionOot(play, sFaceReactionSets[reactionOffset]) != 0) {
        this->actor.textId = Text_GetFaceReactionOot(play, sFaceReactionSets[reactionOffset]);
        this->unk_2B4 = 1;
        this->actionFunc = func_80A56B40;
        return;
    }
    if (play->sceneId == SCENE_MARKET_GUARD_HOUSE_Oot) {
        if (IS_DAY_OOT) {
            this->actor.textId = TEXT_OOT(0x7004);
        } else {
            this->actor.textId = TEXT_OOT(0x709A);
        }
    } else if (play->sceneId != SCENE_MARKET_NIGHT_Oot) {
        if (IS_DAY_OOT) {
            this->actor.textId = TEXT_OOT(0x7002);
        } else {
            this->actor.textId = TEXT_OOT(0x7003);
        }
    } else {
        this->actor.textId = TEXT_OOT(0x7003);
    }

    this->actionFunc = func_80A56B40;
}

static void func_80A5673C(EnHeishi4* this, PlayState* play) {
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_PULLED_MASTER_SWORD_FROM_PEDESTAL_Oot)) {
        Actor_Kill(&this->actor);
        return;
    }
    this->unk_284 = 0;
    if (GET_EVENTCHKINF_OOT(EVENTCHKINF_ZELDA_FLED_HYRULE_CASTLE_Oot)) {
        if (!GET_INFTABLE_OOT(INFTABLE_6C_Oot)) {
            f32 frames = Animation_GetLastFrame(&gEnHeishiDyingGuardAnim_00C444);
            Animation_Change(&this->skelAnime, &gEnHeishiDyingGuardAnim_00C444, 1.0f, 0.0f, (s16)frames, ANIMMODE_LOOP,
                             -10.0f);
            this->actor.textId = TEXT_OOT(0x7007);
            this->unk_282 = TEXT_STATE_EVENT;
            this->unk_284 = 1;
        } else {
            this->actor.textId = TEXT_OOT(0x7008);
            this->unk_282 = TEXT_STATE_DONE;
        }
        this->actionFunc = func_80A56874;
    } else {
        Actor_Kill(&this->actor);
    }
}

static void func_80A56874(EnHeishi4* this, PlayState* play) {
    if (this->unk_284 != 0) {
        SkelAnime_Update(&this->skelAnime);
    }
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if (this->unk_284 == 0) {
            this->actionFunc = func_80A5673C;

        } else {
            this->actionFunc = func_80A56900;
        }
    } else {
        Actor_OfferTalk(&this->actor, play, 100.0f);
    }
}

static void func_80A56900(EnHeishi4* this, PlayState* play) {
    f32 frames = Animation_GetLastFrame(&gEnHeishiDyingGuardTalkAnim);

    Animation_Change(&this->skelAnime, &gEnHeishiDyingGuardTalkAnim, 1.0f, 0.0f, (s16)frames, ANIMMODE_LOOP, -10.0f);
    this->actionFunc = func_80A56994;
}

static void func_80A56994(EnHeishi4* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Actor_TrackPlayer(play, &this->actor, &this->headRot, &this->torsoRot, this->actor.focus.pos);
    if ((this->unk_282 == Message_GetState(&play->msgCtx)) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        SET_INFTABLE_OOT(INFTABLE_6C_Oot);
        Player_SetCsActionWithHaltedActors(play, NULL, 8);
        this->actionFunc = func_80A56A50;
    }
}

static void func_80A56A50(EnHeishi4* this, PlayState* play) {
    f32 frames = Animation_GetLastFrame(&gEnHeishiDyingGuardDieAnim);

    this->unk_288 = frames;
    Animation_Change(&this->skelAnime, &gEnHeishiDyingGuardDieAnim, 1.0f, 0.0f, frames, ANIMMODE_ONCE, -10.0f);
    this->actionFunc = func_80A56ACC;
}

static void func_80A56ACC(EnHeishi4* this, PlayState* play) {
    f32 currentFrame = this->skelAnime.curFrame;

    SkelAnime_Update(&this->skelAnime);
    if (this->unk_288 <= currentFrame) {
        Player_SetCsActionWithHaltedActors(play, NULL, 7);
        this->actionFunc = func_80A5673C;
    }
}

static void func_80A56B40(EnHeishi4* this, PlayState* play) {
    s16 reactionOffset;

    SkelAnime_Update(&this->skelAnime);
    reactionOffset = (this->type - 4);
    if (reactionOffset < 0) {
        reactionOffset = 0;
    }
    if (reactionOffset >= 3) {
        reactionOffset = 1;
    }
    if (Text_GetFaceReactionOot(play, sFaceReactionSets[reactionOffset]) != 0) {
        if (this->unk_2B4 == 0) {
            if ((this->type == HEISHI4_AT_KAKRIKO_ENTRANCE) || (this->type == HEISHI4_AT_IMPAS_HOUSE)) {
                this->actionFunc = func_80A563BC;
                return;
            }
            if (this->type == HEISHI4_AT_MARKET_NIGHT) {
                this->actionFunc = func_80A56614;
                return;
            }
        }
    } else {
        if (this->unk_2B4 != 0) {
            if ((this->type == HEISHI4_AT_KAKRIKO_ENTRANCE) || (this->type == HEISHI4_AT_IMPAS_HOUSE)) {
                this->actionFunc = func_80A563BC;
                return;
            }
            if (this->type == HEISHI4_AT_MARKET_NIGHT) {
                this->actionFunc = func_80A56614;
                return;
            }
        }
    }
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        if ((this->type == HEISHI4_AT_KAKRIKO_ENTRANCE) || (this->type == HEISHI4_AT_IMPAS_HOUSE)) {
            this->unk_284 = 1;
            this->actionFunc = func_80A563BC;
            return;
        }
        if (this->type == HEISHI4_AT_MARKET_NIGHT) {
            if ((true)) {
                this->actionFunc = func_80A56614;
                return;
            }
        }
    }
    Actor_OfferTalkNearColChkInfoCylinder(&this->actor, play);
}

static void EnHeishi4_Update(Actor* thisx, PlayState* play) {
    EnHeishi4* this = (EnHeishi4*)thisx;
    s32 pad;
    Player* player = GET_PLAYER(play);

    thisx->world.pos.x = this->pos.x;
    thisx->world.pos.y = this->pos.y;
    thisx->world.pos.z = this->pos.z;
    Actor_SetFocus(thisx, this->height);
    if (this->type != HEISHI4_AT_MARKET_DYING) {
        this->interactInfo.trackPos = player->actor.world.pos;
        if (!LINK_IS_ADULT_OOT) {
            this->interactInfo.trackPos.y = player->actor.world.pos.y - 10.0f;
        }
        Npc_TrackPoint(thisx, &this->interactInfo, 2, NPC_TRACKING_FULL_BODY);
        this->headRot = this->interactInfo.headRot;
        this->torsoRot = this->interactInfo.torsoRot;
    }
    this->unk_27E += 1;
    this->actionFunc(this, play);
    Actor_MoveWithGravity(thisx);
    Actor_UpdateBgCheckInfo(play, thisx, 10.0f, 10.0f, 30.0f, 0x41D);
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

static s32 EnHeishi_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnHeishi4* this = (EnHeishi4*)thisx;

    if (limbIndex == 9) {
        rot->x += this->torsoRot.y;
    }
    if (limbIndex == 16) {
        rot->x += this->headRot.y;
        rot->z += this->headRot.z;
    }
    return false;
}

static void EnHeishi4_Draw(Actor* thisx, PlayState* play) {
    EnHeishi4* this = (EnHeishi4*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawSkeletonOpaOot(play, &this->skelAnime, EnHeishi_OverrideLimbDraw, NULL, this);
}
