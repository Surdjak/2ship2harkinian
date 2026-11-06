#include "z_en_box.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "2s2h/BenPort.h"
#include "assets/oot/objects/object_box/object_box.h"
#include <assert.h>

#define FLAGS 0

// movement flags

/*
set on init unless treasure flag is set
if clear, chest moves (Actor_MoveXZGravity) (falls, likely)
ends up cleared from SWITCH_FLAG_FALL types when switch flag is set
*/
#define ENBOX_MOVE_IMMOBILE (1 << 0)
/*
set in the logic for SWITCH_FLAG_FALL types
otherwise unused
*/
#define ENBOX_MOVE_UNUSED (1 << 1)
/*
set with 50% chance on init for SWITCH_FLAG_FALL types
only used for SWITCH_FLAG_FALL types
ends up "blinking" (set/clear every frame) once switch flag is set,
if some collision-related condition (?) is met
only used for signum of z rotation
*/
#define ENBOX_MOVE_FALL_ANGLE_SIDE (1 << 2)
/*
when set, gets cleared next EnBox_Update call and clip to the floor
*/
#define ENBOX_MOVE_STICK_TO_GROUND (1 << 4)

typedef enum {
    ENBOX_STATE_0, // waiting for player near / player available / player ? (IDLE)
    ENBOX_STATE_1, // used only temporarily, maybe "player is ready" ?
    ENBOX_STATE_2  // waiting for something message context-related
} EnBoxStateUnk1FB;

static void EnBox_Init(Actor* thisx, PlayState* play);
static void EnBox_Destroy(Actor* thisx, PlayState* play);
static void EnBox_Update(Actor* thisx, PlayState* play);
static void EnBox_Draw(Actor* thisx, PlayState* play);

static void EnBox_FallOnSwitchFlag(EnBox*, PlayState*);
static void func_809C9700(EnBox*, PlayState*);
static void EnBox_AppearOnSwitchFlag(EnBox*, PlayState*);
static void EnBox_AppearOnRoomClear(EnBox*, PlayState*);
static void EnBox_AppearInit(EnBox*, PlayState*);
static void EnBox_AppearAnimation(EnBox*, PlayState*);
static void EnBox_WaitOpen(EnBox*, PlayState*);
static void EnBox_Open(EnBox*, PlayState*);
static void EnBox_UpdateTexture(EnBox*, PlayState*);

ActorProfile En_Box_Oot_Profile = {
    /**/ ACTOR_EN_BOX_Oot,
    /**/ ACTORCAT_CHEST,
    /**/ FLAGS,
    /**/ OBJECT_BOX, // PORT: OoT's rooms load MM's chest object (OotToMm::ObjectId)
    /**/ sizeof(EnBox),
    /**/ EnBox_Init,
    /**/ EnBox_Destroy,
    /**/ EnBox_Update,
    /**/ EnBox_Draw,
};

static AnimationHeader* sAnimations[4] = { &gTreasureChestAnim_00024C, &gTreasureChestAnim_000128,
                                           &gTreasureChestAnim_00043C, &gTreasureChestAnim_00043C };

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 0, ICHAIN_STOP),
};

static UNK_TYPE sUnused;

static Gfx* EnBox_LoadChestDL(const char* dlName, const char* fallbackName) {
    Gfx* dl = ResourceMgr_LoadGfxByName(dlName);

    if (dl == NULL && fallbackName != NULL) {
        dl = ResourceMgr_LoadGfxByName(fallbackName);
    }

    return dl;
}

void EnBox_OpenOot(Actor* thisx, s16 openType) {
    // OoT's Player sets unk_1F4: 1 to open slowly (with the light and fanfare), -1 to kick the lid open.
    ((EnBox*)thisx)->unk_1F4 = openType;
}

static void EnBox_SetupAction(EnBox* this, EnBoxActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void EnBox_ClipToGround(EnBox* this, PlayState* play) {
    f32 newY;
    CollisionPoly* poly;
    s32 bgId;
    Vec3f pos;

    pos = this->dyna.actor.world.pos;
    pos.y += 1.0f;
    newY = BgCheck_EntityRaycastFloor5(&play->colCtx, &poly, &bgId, &this->dyna.actor, &pos);
    if (newY != BGCHECK_Y_MIN) {
        this->dyna.actor.world.pos.y = newY;
    }
}

static void EnBox_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnBox* this = (EnBox*)thisx;
    AnimationHeader* anim;
    CollisionHeader* colHeader;
    f32 animFrameStart;
    f32 endFrame;

    animFrameStart = 0.0f;
    anim = sAnimations[((void)0, LINK_AGE_OOT)];
    colHeader = NULL;
    endFrame = Animation_GetLastFrame(anim);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);

    DynaPolyActor_Init(&this->dyna, 0);
    CollisionHeader_GetVirtual(&gTreasureChestCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
    DynaPoly_DisableCeilingCollision(play, &play->colCtx.dyna, this->dyna.bgId);

    this->movementFlags = 0;
    this->type = thisx->params >> 12 & 0xF;
    this->iceSmokeTimer = 0;
    this->unk_1FB = ENBOX_STATE_0;
    this->dyna.actor.gravity = -5.5f;
    this->switchFlag = this->dyna.actor.world.rot.z;
    this->dyna.actor.terminalVelocity = -50.0f;

    if (play) {} // helps the compiler store play2 into s1

    if ((Flags_GetTreasure(play, this->dyna.actor.params & 0x1F))) {
        this->alpha = 255;
        this->iceSmokeTimer = 100;
        EnBox_SetupAction(this, EnBox_Open);
        this->movementFlags |= ENBOX_MOVE_STICK_TO_GROUND;
        animFrameStart = endFrame;
    } else if ((this->type == ENBOX_TYPE_SWITCH_FLAG_FALL_BIG || this->type == ENBOX_TYPE_SWITCH_FLAG_FALL_SMALL) &&
               !Flags_GetSwitch(play, this->switchFlag)) {
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        if (Rand_ZeroOne() < 0.5f) {
            this->movementFlags |= ENBOX_MOVE_FALL_ANGLE_SIDE;
        }
        this->unk_1A8 = -12;
        EnBox_SetupAction(this, EnBox_FallOnSwitchFlag);
        this->alpha = 0;
        this->movementFlags |= ENBOX_MOVE_IMMOBILE;
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    } else if ((this->type == ENBOX_TYPE_ROOM_CLEAR_BIG || this->type == ENBOX_TYPE_ROOM_CLEAR_SMALL) &&
               !Flags_GetClear(play, this->dyna.actor.room)) {
        EnBox_SetupAction(this, EnBox_AppearOnRoomClear);
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        this->movementFlags |= ENBOX_MOVE_IMMOBILE;
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - 50.0f;
        this->alpha = 0;
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    } else if (this->type == ENBOX_TYPE_9 || this->type == ENBOX_TYPE_10) {
        EnBox_SetupAction(this, func_809C9700);
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA;
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        this->movementFlags |= ENBOX_MOVE_IMMOBILE;
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - 50.0f;
        this->alpha = 0;
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    } else if (this->type == ENBOX_TYPE_SWITCH_FLAG_BIG && !Flags_GetSwitch(play, this->switchFlag)) {
        EnBox_SetupAction(this, EnBox_AppearOnSwitchFlag);
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
        this->movementFlags |= ENBOX_MOVE_IMMOBILE;
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y - 50.0f;
        this->alpha = 0;
        this->dyna.actor.flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;
    } else {
        if (this->type == ENBOX_TYPE_4 || this->type == ENBOX_TYPE_6) {
            this->dyna.actor.flags |= ACTOR_FLAG_REACT_TO_LENS;
        }
        EnBox_SetupAction(this, EnBox_WaitOpen);
        this->movementFlags |= ENBOX_MOVE_IMMOBILE;
        this->movementFlags |= ENBOX_MOVE_STICK_TO_GROUND;
    }

    this->dyna.actor.world.rot.y += 0x8000;
    this->dyna.actor.home.rot.z = this->dyna.actor.world.rot.z = this->dyna.actor.shape.rot.z = 0;

    SkelAnime_Init(play, &this->skelanime, &gTreasureChestSkel, anim, this->jointTable, this->morphTable, 5);
    Animation_Change(&this->skelanime, anim, 1.5f, animFrameStart, endFrame, ANIMMODE_ONCE, 0.0f);

    EnBox_UpdateTexture(this, play);
    // For SOH we spawn a chest actor instead of rendering the object from scratch for forest boss
    // key chest, and it's up on the wall so disable gravity for it.
    if (play->sceneId == SCENE_FOREST_TEMPLE_Oot && this->dyna.actor.params == 10222) {
        this->movementFlags = ENBOX_MOVE_IMMOBILE;
    }
}

static void EnBox_Destroy(Actor* thisx, PlayState* play) {
    EnBox* this = (EnBox*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static void EnBox_RandomDustKinematic(EnBox* this, Vec3f* pos, Vec3f* velocity, Vec3f* accel) {
    f32 randomRadius = Rand_ZeroOne() * 25.0f;
    s16 randomAngle = Rand_ZeroOne() * 0x10000;

    *pos = this->dyna.actor.world.pos;
    pos->x += Math_SinS(randomAngle) * randomRadius;
    pos->z += Math_CosS(randomAngle) * randomRadius;

    velocity->y = 1.0f;
    velocity->x = Math_SinS(randomAngle);
    velocity->z = Math_CosS(randomAngle);

    accel->x = 0.0f;
    accel->y = 0.0f;
    accel->z = 0.0f;
}

/**
 * Spawns dust randomly around the chest when the chest hits the ground after falling (FALL types)
 */
static void EnBox_SpawnDust(EnBox* this, PlayState* play) {
    s32 i;
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel;

    for (i = 0; i < 20; i++) {
        EnBox_RandomDustKinematic(this, &pos, &velocity, &accel);
        func_800B1280(play, &pos, &velocity, &accel, 100, 30, 15);
    }
}

/**
 * Used while the chest is falling (FALL types)
 */
static void EnBox_Fall(EnBox* this, PlayState* play) {
    f32 yDiff;

    this->alpha = 255;
    this->movementFlags &= ~ENBOX_MOVE_IMMOBILE;
    if (this->dyna.actor.bgCheckFlags & 1) {
        this->movementFlags |= ENBOX_MOVE_UNUSED;
        if (this->movementFlags & ENBOX_MOVE_FALL_ANGLE_SIDE) {
            this->movementFlags &= ~ENBOX_MOVE_FALL_ANGLE_SIDE;
        } else {
            this->movementFlags |= ENBOX_MOVE_FALL_ANGLE_SIDE;
        }
        if (this->type == ENBOX_TYPE_SWITCH_FLAG_FALL_BIG) {
            this->dyna.actor.velocity.y = -this->dyna.actor.velocity.y * 0.55f;
        } else {
            this->dyna.actor.velocity.y = -this->dyna.actor.velocity.y * 0.65f;
        }
        if (this->dyna.actor.velocity.y < 5.5f) {
            this->dyna.actor.shape.rot.z = 0;
            this->dyna.actor.world.pos.y = this->dyna.actor.floorHeight;
            EnBox_SetupAction(this, EnBox_WaitOpen);
            if ((true)) {
                OnePointCutscene_EndCutscene(play, this->subCamId);
            }
        }
        Audio_PlaySfx_AtPos(&this->dyna.actor.projectedPos, NA_SE_EV_TRE_BOX_BOUND);
        EnBox_SpawnDust(this, play);
    }
    yDiff = this->dyna.actor.world.pos.y - this->dyna.actor.floorHeight;
    if (this->movementFlags & ENBOX_MOVE_FALL_ANGLE_SIDE) {
        this->dyna.actor.shape.rot.z = yDiff * 50.0f;
    } else {
        this->dyna.actor.shape.rot.z = -yDiff * 50.0f;
    }
}

static void EnBox_FallOnSwitchFlag(EnBox* this, PlayState* play) {
    s32 treasureFlag = this->dyna.actor.params & 0x1F;

    if (treasureFlag >= ENBOX_TREASURE_FLAG_UNK_MIN && treasureFlag < ENBOX_TREASURE_FLAG_UNK_MAX) {
        Actor_SetClosestSecretDistance(&this->dyna.actor, play);
    }

    if (this->unk_1A8 >= 0) {
        EnBox_SetupAction(this, EnBox_Fall);
        this->subCamId = OnePointCutscene_Init(play, 4500, 9999, &this->dyna.actor, CAM_ID_MAIN);
        DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    } else if (this->unk_1A8 >= -11) {
        this->unk_1A8++;
    } else if (Flags_GetSwitch(play, this->switchFlag)) {
        this->unk_1A8++;
    }
}

// used for types 9, 10
static void func_809C9700(EnBox* this, PlayState* play) {
    s32 treasureFlag = this->dyna.actor.params & 0x1F;
    Player* player = GET_PLAYER(play);

    if (treasureFlag >= ENBOX_TREASURE_FLAG_UNK_MIN && treasureFlag < ENBOX_TREASURE_FLAG_UNK_MAX) {
        Actor_SetClosestSecretDistance(&this->dyna.actor, play);
    }

    if (Math3D_Vec3fDistSq(&this->dyna.actor.world.pos, &player->actor.world.pos) > 22500.0f) {
        this->unk_1FB = ENBOX_STATE_0;
    } else {
        if (this->unk_1FB == ENBOX_STATE_0) {
            // PORT: MM's Player offers the ocarina to an actor that offers itself, where OoT's plays for an actor
            // that asks (PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR).
            if (!Actor_OcarinaInteractionAccepted(&this->dyna.actor, &play->state)) {
                Actor_OfferOcarinaInteraction(&this->dyna.actor, play, 150.0f, 150.0f);
                return;
            }
            this->unk_1FB = ENBOX_STATE_1;
        }

        if (this->unk_1FB == ENBOX_STATE_1) {
            Message_StartOcarinaOot(play, OCARINA_ACTION_FREE_PLAY_Oot);
            this->unk_1FB = ENBOX_STATE_2;
        } else if (this->unk_1FB == ENBOX_STATE_2 && Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_04_Oot) {
            if ((play->msgCtx.lastPlayedSong == OCARINA_SONG_LULLABY_Oot && this->type == ENBOX_TYPE_9) ||
                (play->msgCtx.lastPlayedSong == OCARINA_SONG_SUNS_Oot && this->type == ENBOX_TYPE_10)) {
                this->dyna.actor.flags &= ~ACTOR_FLAG_UPDATE_DURING_OCARINA;
                EnBox_SetupAction(this, EnBox_AppearInit);
                OnePointCutscene_Attention(play, &this->dyna.actor);
                this->unk_1A8 = 0;
                this->unk_1FB = ENBOX_STATE_0;
            } else {
                this->unk_1FB = ENBOX_STATE_0;
            }
        }
    }
}

static void EnBox_AppearOnSwitchFlag(EnBox* this, PlayState* play) {
    s32 treasureFlag = this->dyna.actor.params & 0x1F;

    if (treasureFlag >= ENBOX_TREASURE_FLAG_UNK_MIN && treasureFlag < ENBOX_TREASURE_FLAG_UNK_MAX) {
        Actor_SetClosestSecretDistance(&this->dyna.actor, play);
    }

    if (Flags_GetSwitch(play, this->switchFlag)) {
        OnePointCutscene_Attention(play, &this->dyna.actor);
        EnBox_SetupAction(this, EnBox_AppearInit);
        this->unk_1A8 = -30;
    }
}

static void EnBox_AppearOnRoomClear(EnBox* this, PlayState* play) {
    s32 treasureFlag = this->dyna.actor.params & 0x1F;

    if (treasureFlag >= ENBOX_TREASURE_FLAG_UNK_MIN && treasureFlag < ENBOX_TREASURE_FLAG_UNK_MAX) {
        Actor_SetClosestSecretDistance(&this->dyna.actor, play);
    }

    if (Flags_GetClearTemp(play, this->dyna.actor.room) && !Player_InCsMode(play)) {
        Flags_SetClear(play, this->dyna.actor.room);
        EnBox_SetupAction(this, EnBox_AppearInit);
        OnePointCutscene_Attention(play, &this->dyna.actor);
        if (OnePointCutscene_CheckForCategory(play, this->dyna.actor.category)) {
            this->unk_1A8 = 0;
        } else {
            this->unk_1A8 = -30;
        }
    }
}

/**
 * The chest is ready to appear, possibly waiting for camera/cutscene-related stuff to happen
 */
static void EnBox_AppearInit(EnBox* this, PlayState* play) {
    if (Hyrule_GetAttentionCategoryOot() == this->dyna.actor.category || this->unk_1A8 != 0) {
        EnBox_SetupAction(this, EnBox_AppearAnimation);
        this->unk_1A8 = 0;
        Actor_Spawn(&play->actorCtx, play, ACTOR_DEMO_KANKYO_Oot, this->dyna.actor.home.pos.x, this->dyna.actor.home.pos.y,
                    this->dyna.actor.home.pos.z, 0, 0, 0, 0x0011);
        Audio_PlaySfx_AtPos(&this->dyna.actor.projectedPos, NA_SE_EV_TRE_BOX_APPEAR);
    }
}

static void EnBox_AppearAnimation(EnBox* this, PlayState* play) {
    DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);

    if (this->unk_1A8 < 0) {
        this->unk_1A8++;
    } else if (this->unk_1A8 < 40) {
        this->unk_1A8++;
        this->dyna.actor.world.pos.y += 1.25f;
    } else if (this->unk_1A8 < 60) {
        this->alpha += 12;
        this->unk_1A8++;
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y;
    } else {
        EnBox_SetupAction(this, EnBox_WaitOpen);
    }
}

/**
 * Chest is ready to be open
 */
static void EnBox_WaitOpen(EnBox* this, PlayState* play) {
    f32 frameCount;
    AnimationHeader* anim;
    s32 linkAge;
    s32 pad;
    Vec3f sp4C;
    Player* player;

    this->alpha = 255;
    this->movementFlags |= ENBOX_MOVE_IMMOBILE;
    if (this->unk_1F4 != 0) { // unk_1F4 is modified by player code
        linkAge = LINK_AGE_OOT;
        anim = sAnimations[(this->unk_1F4 < 0 ? 2 : 0) + linkAge];
        frameCount = Animation_GetLastFrame(anim);
        Animation_Change(&this->skelanime, anim, 1.5f, 0, frameCount, ANIMMODE_ONCE, 0.0f);
        EnBox_SetupAction(this, EnBox_Open);
        if (this->unk_1F4 > 0) {
            switch (this->type) {
                case ENBOX_TYPE_SMALL:
                case ENBOX_TYPE_6:
                case ENBOX_TYPE_ROOM_CLEAR_SMALL:
                case ENBOX_TYPE_SWITCH_FLAG_FALL_SMALL:
                    break;
                default:
                    Actor_SpawnAsChild(&play->actorCtx, &this->dyna.actor, play, ACTOR_DEMO_TRE_LGT_Oot,
                                       this->dyna.actor.world.pos.x, this->dyna.actor.world.pos.y,
                                       this->dyna.actor.world.pos.z, this->dyna.actor.shape.rot.x,
                                       this->dyna.actor.shape.rot.y, this->dyna.actor.shape.rot.z, 0xFFFF);
                    Audio_PlayFanfare(NA_BGM_OPEN_CHEST); // 3Ship [Hyrule] OoT's | 0x900 arguments crash 2Ship's fanfare
            }
        }
        if ((true)) {
            Flags_SetTreasure(play, this->dyna.actor.params & 0x1F);
        }
    } else {
        player = GET_PLAYER(play);
        Actor_WorldToActorCoords(&this->dyna.actor, &sp4C, &player->actor.world.pos);
        if (sp4C.z > -50.0f && sp4C.z < 0.0f && fabsf(sp4C.y) < 10.0f && fabsf(sp4C.x) < 20.0f &&
            Player_IsFacingActor(&this->dyna.actor, 0x3000, play)) {
            Actor_OfferGetItemNearby(&this->dyna.actor, play, -(GI_OOT_BASE + (this->dyna.actor.params >> 5 & 0x7F)));
        }
        if ((Flags_GetTreasure(play, this->dyna.actor.params & 0x1F))) {
            EnBox_SetupAction(this, EnBox_Open);
        }
    }
}

/**
 * Plays an animation to its end, playing sounds at key points
 */
static void EnBox_Open(EnBox* this, PlayState* play) {
    u16 sfxId;

    this->dyna.actor.flags &= ~ACTOR_FLAG_REACT_TO_LENS;

    if (SkelAnime_Update(&this->skelanime)) {
        if (this->unk_1F4 > 0) {
            if (this->unk_1F4 < 120) {
                this->unk_1F4++;
            } else {
                Math_StepToF(&this->unk_1B0, 0.0f, 0.05f);
            }
        } else {
            if (this->unk_1F4 > -120) {
                this->unk_1F4--;
            } else {
                Math_StepToF(&this->unk_1B0, 0.0f, 0.05f);
            }
        }
    } else {
        sfxId = 0;

        if (Animation_OnFrame(&this->skelanime, 30.0f)) {
            sfxId = NA_SE_EV_TBOX_UNLOCK;
        } else if (Animation_OnFrame(&this->skelanime, 90.0f)) {
            sfxId = NA_SE_EV_TBOX_OPEN;
        }

        if (sfxId != 0) {
            Audio_PlaySfx_AtPos(&this->dyna.actor.projectedPos, sfxId);
        }

        if (this->skelanime.jointTable[3].z > 0) {
            this->unk_1B0 = (0x7D00 - this->skelanime.jointTable[3].z) * 0.00006f;
            if (this->unk_1B0 < 0.0f) {
                this->unk_1B0 = 0.0f;
            } else if (this->unk_1B0 > 1.0f) {
                this->unk_1B0 = 1.0f;
            }
        }
    }
}

static void EnBox_SpawnIceSmoke(EnBox* this, PlayState* play) {
    Vec3f pos;
    Vec3f vel = { 0.0f, 1.0f, 0.0f };
    Vec3f accel = { 0.0f, 0.0f, 0.0f };
    f32 f0;

    this->iceSmokeTimer++;
    Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EN_LAST3_COIL_ATTACK_OLD - SFX_FLAG);
    if (Rand_ZeroOne() < 0.3f) {
        f0 = 2.0f * Rand_ZeroOne() - 1.0f;
        pos = this->dyna.actor.world.pos;
        if (this->type == ENBOX_TYPE_SMALL || this->type == ENBOX_TYPE_6 || this->type == ENBOX_TYPE_ROOM_CLEAR_SMALL ||
            this->type == ENBOX_TYPE_SWITCH_FLAG_FALL_SMALL) {
            pos.x += f0 * 10.0f * Math_SinS(this->dyna.actor.world.rot.y + 0x4000);
            pos.z += f0 * 10.0f * Math_CosS(this->dyna.actor.world.rot.y + 0x4000);
            f0 = 2.0f * Rand_ZeroOne() - 1.0f;
            vel.x = f0 * 0.8f * Math_SinS(this->dyna.actor.world.rot.y);
            vel.y = 1.8f;
            vel.z = f0 * 0.8f * Math_CosS(this->dyna.actor.world.rot.y);
        } else {
            pos.x += f0 * 20.0f * Math_SinS(this->dyna.actor.world.rot.y + 0x4000);
            pos.z += f0 * 20.0f * Math_CosS(this->dyna.actor.world.rot.y + 0x4000);
            f0 = 2.0f * Rand_ZeroOne() - 1.0f;
            vel.x = f0 * 1.6f * Math_SinS(this->dyna.actor.world.rot.y);
            vel.y = 1.8f;
            vel.z = f0 * 1.6f * Math_CosS(this->dyna.actor.world.rot.y);
        }
        EffectSsIceSmoke_Spawn(play, &pos, &vel, &accel, 150);
    }
}

static void EnBox_Update(Actor* thisx, PlayState* play) {
    EnBox* this = (EnBox*)thisx;

    EnBox_UpdateTexture(this, play);

    if (this->movementFlags & ENBOX_MOVE_STICK_TO_GROUND) {
        this->movementFlags &= ~ENBOX_MOVE_STICK_TO_GROUND;
        EnBox_ClipToGround(this, play);
    }

    this->actionFunc(this, play);

    if (!(this->movementFlags & ENBOX_MOVE_IMMOBILE)) {
        Actor_MoveWithGravity(&this->dyna.actor);
        Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 0.0f, 0.0f, 0.0f, 0x41C);
    }

    switch (this->type) {
        case ENBOX_TYPE_SMALL:
        case ENBOX_TYPE_6:
        case ENBOX_TYPE_ROOM_CLEAR_SMALL:
        case ENBOX_TYPE_SWITCH_FLAG_FALL_SMALL:
            Actor_SetFocus(&this->dyna.actor, 20.0f);
            break;
        default:
            Actor_SetFocus(&this->dyna.actor, 40.0f);
    }

    if (((this->dyna.actor.params >> 5 & 0x7F) == GI_ICE_TRAP_Oot && this->actionFunc == EnBox_Open &&
                                  this->skelanime.curFrame > 45 && this->iceSmokeTimer < 100)) {
        EnBox_SpawnIceSmoke(this, play);
    }
}

static void EnBox_UpdateTexture(EnBox* this, PlayState* play) {
    switch (this->type) {
        case ENBOX_TYPE_SMALL:
        case ENBOX_TYPE_6:
        case ENBOX_TYPE_ROOM_CLEAR_SMALL:
        case ENBOX_TYPE_SWITCH_FLAG_FALL_SMALL:
            Actor_SetScale(&this->dyna.actor, 0.005f);
            Actor_SetFocus(&this->dyna.actor, 20.0f);
            break;
        default:
            Actor_SetScale(&this->dyna.actor, 0.01f);
            Actor_SetFocus(&this->dyna.actor, 40.0f);
    }

    if (this->type != ENBOX_TYPE_DECORATED_BIG) {
        this->boxBodyDL = EnBox_LoadChestDL(gTreasureChestChestFrontDL, NULL);
        this->boxLidDL = EnBox_LoadChestDL(gTreasureChestChestSideAndLidDL, NULL);
    } else {
        this->boxBodyDL = EnBox_LoadChestDL(gTreasureChestBossKeyChestFrontDL, gTreasureChestChestFrontDL);
        this->boxLidDL = EnBox_LoadChestDL(gTreasureChestBossKeyChestSideAndTopDL, gTreasureChestChestSideAndLidDL);
    }
}

static void EnBox_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx, Gfx** gfx) {
    EnBox* this = (EnBox*)thisx;
    s32 pad;

    if (limbIndex == 1) {
        gSPMatrix((*gfx)++, Matrix_Finalize(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList((*gfx)++, this->boxBodyDL);
    } else if (limbIndex == 3) {
        gSPMatrix((*gfx)++, Matrix_Finalize(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList((*gfx)++, this->boxLidDL);
    }
}

Gfx* EnBox_EmptyDList(GraphicsContext* gfxCtx) {
    Gfx* dListHead;
    Gfx* dList;

    dList = GRAPH_ALLOC(gfxCtx, sizeof(Gfx));
    assert(dList != NULL);

    dListHead = dList;
    gSPEndDisplayList(dListHead++);

    return dList;
}

// set render mode with a focus on transparency
Gfx* func_809CA4A0(GraphicsContext* gfxCtx) {
    Gfx* dList;
    Gfx* dListHead;

    dListHead = GRAPH_ALLOC(gfxCtx, 2 * sizeof(Gfx));
    assert(dListHead != NULL);

    dList = dListHead;
    gDPSetRenderMode(dListHead++,
                     AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU | FORCE_BL |
                         GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA),
                     AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU | FORCE_BL |
                         GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA));
    gSPEndDisplayList(dListHead++);

    return dList;
}

Gfx* func_809CA518(GraphicsContext* gfxCtx) {
    Gfx* dList;
    Gfx* dListHead;

    dListHead = GRAPH_ALLOC(gfxCtx, 2 * sizeof(Gfx));
    assert(dListHead != NULL);

    dList = dListHead;
    gDPSetRenderMode(dListHead++,
                     AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP | ZMODE_OPA | ALPHA_CVG_SEL |
                         GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA),
                     G_RM_AA_ZB_OPA_SURF2);
    gSPEndDisplayList(dListHead++);

    return dList;
}

static void EnBox_Draw(Actor* thisx, PlayState* play) {
    EnBox* this = (EnBox*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    /*
    this->dyna.actor.flags & ACTOR_FLAG_REACT_TO_LENS is set by Init (if type is 4 or 6)
    and cleared by Open
    */
    if ((this->alpha == 255 && !(this->type == ENBOX_TYPE_4 || this->type == ENBOX_TYPE_6)) ||
        (!CHECK_FLAG_ALL(this->dyna.actor.flags, ACTOR_FLAG_REACT_TO_LENS) &&
         (this->type == ENBOX_TYPE_4 || this->type == ENBOX_TYPE_6))) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        gSPSegment(POLY_OPA_DISP++, 0x08, EnBox_EmptyDList(play->state.gfxCtx));
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        POLY_OPA_DISP = SkelAnime_DrawSkeleton2Oot(play, &this->skelanime, NULL, EnBox_PostLimbDraw, this, POLY_OPA_DISP);
    } else if (this->alpha != 0) {
        gDPPipeSync(POLY_XLU_DISP++);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, this->alpha);
        if (this->type == ENBOX_TYPE_4 || this->type == ENBOX_TYPE_6) {
            gSPSegment(POLY_XLU_DISP++, 0x08, func_809CA518(play->state.gfxCtx));
        } else {
            gSPSegment(POLY_XLU_DISP++, 0x08, func_809CA4A0(play->state.gfxCtx));
        }
        POLY_XLU_DISP = SkelAnime_DrawSkeleton2Oot(play, &this->skelanime, NULL, EnBox_PostLimbDraw, this, POLY_XLU_DISP);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
