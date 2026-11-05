/*
 * File: z_obj_timeblock.c
 * Overlay: ovl_Obj_Timeblock
 * Description: Song of Time Block
 */

#include "z_obj_timeblock.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyruleOcarina.h"
#include "assets/oot/objects/object_timeblock/object_timeblock.h"

#define FLAGS                                                                                               \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA | \
     ACTOR_FLAG_LOCK_ON_DISABLED)

static void ObjTimeblock_Init(Actor* thisx, PlayState* play);
static void ObjTimeblock_Destroy(Actor* thisx, PlayState* play);
static void ObjTimeblock_Update(Actor* thisx, PlayState* play);
static void ObjTimeblock_Draw(Actor* thisx, PlayState* play);

static void ObjTimeblock_SetupNormal(ObjTimeblock* this);
static void ObjTimeblock_SetupAltBehaviorVisible(ObjTimeblock* this);
static void ObjTimeblock_SetupAltBehaviourNotVisible(ObjTimeblock* this);

static s32 ObjTimeblock_WaitForOcarina(ObjTimeblock* this, PlayState* play);
static s32 ObjTimeblock_WaitForSong(ObjTimeblock* this, PlayState* play);
static void ObjTimeblock_DoNothing(ObjTimeblock* this, PlayState* play);
static void ObjTimeblock_Normal(ObjTimeblock* this, PlayState* play);
static void ObjTimeblock_AltBehaviorVisible(ObjTimeblock* this, PlayState* play);
static void ObjTimeblock_AltBehaviourNotVisible(ObjTimeblock* this, PlayState* play);

ActorProfile Obj_Timeblock_Oot_Profile = {
    /**/ ACTOR_OBJ_TIMEBLOCK_Oot,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_TIMEBLOCK_Oot,
    /**/ sizeof(ObjTimeblock),
    /**/ ObjTimeblock_Init,
    /**/ ObjTimeblock_Destroy,
    /**/ ObjTimeblock_Update,
    /**/ ObjTimeblock_Draw,
};

typedef struct {
    /* 0x00 */ f32 scale;
    /* 0x04 */ f32 height;
    /* 0x08 */ s16 demoEffectParams;
} ObjTimeblockSizeOptions; // size = 0x0C

static ObjTimeblockSizeOptions sSizeOptions[] = {
    { 1.0, 60.0, 0x0018 },
    { 0.60, 40.0, 0x0019 },
};

static f32 sRanges[] = { 60.0, 100.0, 140.0, 180.0, 220.0, 260.0, 300.0, 300.0 };

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(attentionRangeType, 2, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDistance, 1800, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeScale, 300, ICHAIN_CONTINUE),
    ICHAIN_F32(cullingVolumeDownward, 1500, ICHAIN_STOP),
};

static Color_RGB8 sPrimColors[] = {
    { 100, 120, 140 }, { 80, 140, 200 }, { 100, 150, 200 }, { 100, 200, 240 },
    { 80, 110, 140 },  { 70, 160, 225 }, { 80, 100, 130 },  { 100, 110, 190 },
};

static u32 ObjTimeblock_CalculateIsVisible(ObjTimeblock* this) {
    if (!((this->dyna.actor.params >> 10) & 1)) {
        if (this->unk_177 == 0) {
            return this->unk_175;
        } else {
            u8 temp = ((this->dyna.actor.params >> 15) & 1) ? true : false;

            if (this->unk_177 == 1) {
                return this->unk_174 ^ temp;
            } else {
                u8 linkIsYoung = (LINK_AGE_IN_YEARS_OOT == YEARS_CHILD) ? true : false;

                return this->unk_174 ^ temp ^ linkIsYoung;
            }
        }
    } else {
        return (((this->dyna.actor.params >> 15) & 1) ? true : false) ^ this->unk_174;
    }
}

static void ObjTimeblock_SpawnDemoEffect(ObjTimeblock* this, PlayState* play) {
    Actor_Spawn(&play->actorCtx, play, ACTOR_DEMO_EFFECT_Oot, this->dyna.actor.world.pos.x, this->dyna.actor.world.pos.y,
                this->dyna.actor.world.pos.z, 0, 0, 0,
                sSizeOptions[(this->dyna.actor.params >> 8) & 1].demoEffectParams);
}

static void ObjTimeblock_ToggleSwitchFlag(PlayState* play, s32 flag) {
    if (Flags_GetSwitch(play, flag)) {
        Flags_UnsetSwitch(play, flag);
    } else {
        Flags_SetSwitch(play, flag);
    }
}

static void ObjTimeblock_Init(Actor* thisx, PlayState* play) {
    ObjTimeblock* this = (ObjTimeblock*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&this->dyna, 0);
    this->dyna.actor.world.rot.z = this->dyna.actor.shape.rot.z = 0;

    CollisionHeader_GetVirtual(&gSongOfTimeBlockCol, &colHeader);

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    Actor_SetScale(&this->dyna.actor, sSizeOptions[(this->dyna.actor.params >> 8) & 1].scale);

    if ((this->dyna.actor.params >> 6) & 1) {
        this->unk_177 = 0;
    } else {
        this->unk_177 = ((this->dyna.actor.params & 0x3F) < 0x38) ? 2 : 1;
    }

    this->songObserverFunc = ObjTimeblock_WaitForOcarina;

    Actor_SetFocus(&this->dyna.actor, sSizeOptions[(this->dyna.actor.params >> 8) & 1].height);

    this->unk_174 = (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) ? true : false;
    this->unk_175 = ((this->dyna.actor.params >> 15) & 1) ? true : false;
    this->isVisible = ObjTimeblock_CalculateIsVisible(this);

    if (!((this->dyna.actor.params >> 10) & 1)) {
        ObjTimeblock_SetupNormal(this);
    } else if (this->isVisible) {
        ObjTimeblock_SetupAltBehaviorVisible(this);
    } else {
        ObjTimeblock_SetupAltBehaviourNotVisible(this);
    }

}

static void ObjTimeblock_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    ObjTimeblock* this = (ObjTimeblock*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

static u8 ObjTimeblock_PlayerIsInRange(ObjTimeblock* this, PlayState* play) {
    if (this->isVisible && DynaPolyActor_IsPlayerAbove(&this->dyna)) {
        return false;
    }

    if (this->dyna.actor.xzDistToPlayer <= sRanges[(this->dyna.actor.params >> 11) & 7]) {
        Vec3f distance;
        f32 blockSize;

        Actor_WorldToActorCoords(&this->dyna.actor, &distance, &GET_PLAYER(play)->actor.world.pos);
        blockSize = this->dyna.actor.scale.x * 50.0f + 6.0f;
        // Return true if player's xz position is not inside the block
        if (blockSize < fabsf(distance.x) || blockSize < fabsf(distance.z)) {
            return true;
        }
    }

    return false;
}

static s32 ObjTimeblock_WaitForOcarina(ObjTimeblock* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (ObjTimeblock_PlayerIsInRange(this, play)) {
        if (Hyrule_OcarinaAttemptedOot(play, (Actor*)this)) {
            Message_StartOcarinaOot(play, OCARINA_ACTION_FREE_PLAY_Oot);
            this->songObserverFunc = ObjTimeblock_WaitForSong;
        } else {
            Hyrule_OfferOcarinaOot(play, (Actor*)this);
        }
    }
    return false;
}

static s32 ObjTimeblock_WaitForSong(ObjTimeblock* this, PlayState* play) {
    if (Hyrule_GetOcarinaModeOot(play) == OCARINA_MODE_04_Oot) {
        this->songObserverFunc = ObjTimeblock_WaitForOcarina;
    }
    if (play->msgCtx.lastPlayedSong == OCARINA_SONG_TIME_Oot) {
        if (this->unk_172 == 254) {
            this->songEndTimer = 110;
        } else {
            this->songEndTimer--;
            if (this->songEndTimer == 0) {
                return true;
            }
        }
    }
    return false;
}

static void ObjTimeblock_SetupDoNothing(ObjTimeblock* this) {
    this->actionFunc = ObjTimeblock_DoNothing;
}

static void ObjTimeblock_DoNothing(ObjTimeblock* this, PlayState* play) {
}

static void ObjTimeblock_SetupNormal(ObjTimeblock* this) {
    this->actionFunc = ObjTimeblock_Normal;
}

static void ObjTimeblock_Normal(ObjTimeblock* this, PlayState* play) {
    u32 newIsVisible;

    if (this->songObserverFunc(this, play) && this->demoEffectTimer <= 0) {
        ObjTimeblock_SpawnDemoEffect(this, play);
        this->demoEffectTimer = 160;

        if ((true)) {
            // Possibly points the camera to this actor
            OnePointCutscene_Attention(play, &this->dyna.actor);
        }


        this->demoEffectFirstPartTimer = 12;

        if (this->unk_177 == 0) {
            this->dyna.actor.params ^= 0x8000;
        } else {
            ObjTimeblock_ToggleSwitchFlag(play, this->dyna.actor.params & 0x3F);
        }
    }

    this->unk_172 = play->msgCtx.lastPlayedSong;
    if (this->demoEffectFirstPartTimer > 0) {
        this->demoEffectFirstPartTimer--;
        if (this->demoEffectFirstPartTimer == 0) {
            if (this->unk_177 == 0) {
                this->unk_175 = ((this->dyna.actor.params >> 15) & 1) ? true : false;
            } else {
                this->unk_174 = (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) ? true : false;
            }
        }
    }

    newIsVisible = ObjTimeblock_CalculateIsVisible(this);
    if (this->unk_177 == 1 && newIsVisible != this->isVisible) {
        ObjTimeblock_SetupDoNothing(this);
    }
    this->isVisible = newIsVisible;

    if (this->demoEffectTimer == 50) {
        Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
    }
}

static void func_80BA06AC(ObjTimeblock* this, PlayState* play) {
    s32 switchFlag = this->dyna.actor.params & 0x3F;

    this->unk_172 = play->msgCtx.lastPlayedSong;

    if (this->demoEffectFirstPartTimer > 0 && --this->demoEffectFirstPartTimer == 0) {
        this->unk_174 = (Flags_GetSwitch(play, switchFlag)) ? true : false;
    }

    this->isVisible = ObjTimeblock_CalculateIsVisible(this);
    this->unk_176 = (Flags_GetSwitch(play, switchFlag)) ? true : false;
}

static void ObjTimeblock_SetupAltBehaviorVisible(ObjTimeblock* this) {
    this->actionFunc = ObjTimeblock_AltBehaviorVisible;
}

static void ObjTimeblock_AltBehaviorVisible(ObjTimeblock* this, PlayState* play) {
    if (this->songObserverFunc(this, play) && this->demoEffectTimer <= 0) {
        this->demoEffectFirstPartTimer = 12;
        ObjTimeblock_SpawnDemoEffect(this, play);
        this->demoEffectTimer = 160;

        if ((true)) {
            OnePointCutscene_Attention(play, &this->dyna.actor);
        }

        ObjTimeblock_ToggleSwitchFlag(play, this->dyna.actor.params & 0x3F);
    }

    func_80BA06AC(this, play);

    if (this->demoEffectTimer == 50) {
        Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
    }

    if (!this->isVisible && this->demoEffectTimer <= 0) {
        ObjTimeblock_SetupAltBehaviourNotVisible(this);
    }
}

static void ObjTimeblock_SetupAltBehaviourNotVisible(ObjTimeblock* this) {
    this->actionFunc = ObjTimeblock_AltBehaviourNotVisible;
}

static void ObjTimeblock_AltBehaviourNotVisible(ObjTimeblock* this, PlayState* play) {
    s32 switchFlag = this->dyna.actor.params & 0x3F;
    s8 switchFlagIsSet = (Flags_GetSwitch(play, switchFlag)) ? true : false;

    if (this->unk_176 ^ switchFlagIsSet && switchFlagIsSet ^ (((this->dyna.actor.params >> 15) & 1) ? true : false)) {
        if (this->demoEffectTimer <= 0) {
            ObjTimeblock_SpawnDemoEffect(this, play);
            this->demoEffectTimer = 160;
        }
        this->demoEffectFirstPartTimer = 12;
    }

    func_80BA06AC(this, play);

    if (this->isVisible && this->demoEffectTimer <= 0) {
        ObjTimeblock_SetupAltBehaviorVisible(this);
    }
}

static void ObjTimeblock_Update(Actor* thisx, PlayState* play) {
    ObjTimeblock* this = (ObjTimeblock*)thisx;

    this->actionFunc(this, play);

    if (this->demoEffectTimer > 0) {
        this->demoEffectTimer--;
    }

    if (this->isVisible) {
        DynaPoly_EnableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    } else {
        DynaPoly_DisableCollision(play, &play->colCtx.dyna, this->dyna.bgId);
    }
}

static void ObjTimeblock_Draw(Actor* thisx, PlayState* play) {
    if (((ObjTimeblock*)thisx)->isVisible) {
        Color_RGB8* primColor = &sPrimColors[thisx->home.rot.z & 7];

        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        if (0) {
            Color_RGB8 color = *primColor;
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, color.r, color.g, color.b, 255);
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, primColor->r, primColor->g, primColor->b, 255);
        }

        gSPDisplayList(POLY_OPA_DISP++, gSongOfTimeBlockDL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
