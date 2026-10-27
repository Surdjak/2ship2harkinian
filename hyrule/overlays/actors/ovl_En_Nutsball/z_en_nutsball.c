/*
 * File: z_en_nutsball.c
 * Overlay: ovl_En_Nutsball
 * Description: Projectile fired by deku scrubs
 */

#include "z_en_nutsball.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include "HyrulePlayer.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "assets/oot/objects/object_dekunuts/object_dekunuts.h"
#include "assets/oot/objects/object_hintnuts/object_hintnuts.h"
#include "assets/oot/objects/object_shopnuts/object_shopnuts.h"
#include "assets/oot/objects/object_dns/object_dns.h"
#include "assets/oot/objects/object_dnk/object_dnk.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void EnNutsball_Init(Actor* thisx, PlayState* play);
static void EnNutsball_Destroy(Actor* thisx, PlayState* play);
static void EnNutsball_Update(Actor* thisx, PlayState* play);
static void EnNutsball_Draw(Actor* thisx, PlayState* play);

static void func_80ABBB34(EnNutsball* this, PlayState* play);
static void func_80ABBBA8(EnNutsball* this, PlayState* play);

ActorProfile En_Nutsball_Oot_Profile = {
    /**/ ACTOR_EN_NUTSBALL_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnNutsball),
    /**/ EnNutsball_Init,
    /**/ EnNutsball_Destroy,
    /**/ EnNutsball_Update,
    /**/ NULL,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x08 },
        { DMG_FROM_OOT(0xFFCFFFFF), 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_WOOD,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 13, 13, 0, { 0 } },
};

static s16 sObjectIDs[] = {
    OBJECT_DEKUNUTS_Oot, OBJECT_HINTNUTS_Oot, OBJECT_SHOPNUTS_Oot, OBJECT_DNS_Oot, OBJECT_DNK_Oot,
};

static Gfx* sDLists[] = {
    gDekuNutsDekuNutDL, gHintNutsNutDL, gBusinessScrubDekuNutDL, gDntJijiNutDL, gDntStageNutDL,
};

static void EnNutsball_Init(Actor* thisx, PlayState* play) {
    EnNutsball* this = (EnNutsball*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 400.0f, ActorShadow_DrawCircle, 13.0f);
    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->objectSlot = Object_GetSlot(&play->objectCtx, sObjectIDs[this->actor.params]);

    if (this->objectSlot < 0) {
        Actor_Kill(&this->actor);
    } else {
        this->actionFunc = func_80ABBB34;
    }
}

static void EnNutsball_Destroy(Actor* thisx, PlayState* play) {
    EnNutsball* this = (EnNutsball*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

static void func_80ABBB34(EnNutsball* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->objectSlot)) {
        this->actor.objectSlot = this->objectSlot;
        this->actor.draw = EnNutsball_Draw;
        this->actor.shape.rot.y = 0;
        this->timer = 30;
        this->actionFunc = func_80ABBBA8;
        this->actor.speed = 10.0f;
    }
}

static void func_80ABBBA8(EnNutsball* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3s sp4C;
    Vec3f sp40;

    this->timer--;

    if (this->timer == 0) {
        this->actor.gravity = -1.0f;
    }

    this->actor.home.rot.z += 0x2AA8;

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) || (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        (this->collider.base.atFlags & AT_HIT) || (this->collider.base.acFlags & AC_HIT) ||
        (this->collider.base.ocFlags1 & OC1_HIT)) {
        // Checking if the player is using a shield that reflects projectiles
        // And if so, reflects the projectile on impact
        // MM's Hero's Shield, in the Deku Shield's place, sends nuts back as it does MM's scrubs'.
        if ((player->currentShield == PLAYER_SHIELD_DEKU_Oot) ||
            (player->currentShield == PLAYER_SHIELD_HEROS_SHIELD) ||
            ((player->currentShield == PLAYER_SHIELD_HYLIAN_Oot) && LINK_IS_ADULT_OOT)) {
            if ((this->collider.base.atFlags & AT_HIT) && (this->collider.base.atFlags & AT_TYPE_ENEMY) &&
                (this->collider.base.atFlags & AT_BOUNCED)) {
                this->collider.base.atFlags &= ~AT_TYPE_ENEMY & ~AT_BOUNCED & ~AT_HIT;
                this->collider.base.atFlags |= AT_TYPE_PLAYER;

                this->collider.elem.atDmgInfo.dmgFlags = DMG_FROM_OOT(0x2);
                Matrix_MtxFToYXZRot(&player->shieldMf, &sp4C, 0);
                this->actor.world.rot.y = sp4C.y + 0x8000;
                this->timer = 30;
                return;
            }
        }

        sp40.x = this->actor.world.pos.x;
        sp40.y = this->actor.world.pos.y + 4;
        sp40.z = this->actor.world.pos.z;

        EffectSsHahen_SpawnBurst(play, &sp40, 6.0f, 0, 7, 3, 15, HAHEN_OBJECT_DEFAULT, 10, NULL);
        SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 20, NA_SE_EN_OCTAROCK_ROCK);
        Actor_Kill(&this->actor);
    } else {
        if (this->timer == -300) {
            Actor_Kill(&this->actor);
        }
    }
}

static void EnNutsball_Update(Actor* thisx, PlayState* play) {
    EnNutsball* this = (EnNutsball*)thisx;
    Player* player = GET_PLAYER(play);
    s32 pad;

    if (!(player->stateFlags1 &
          (PLAYER_STATE1_TALKING | PLAYER_STATE1_DEAD | PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000)) ||
        (this->actionFunc == func_80ABBB34)) {
        this->actionFunc(this, play);

        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 10, sCylinderInit.dim.radius, sCylinderInit.dim.height, 0x405);
        Collider_UpdateCylinder(&this->actor, &this->collider);

        this->actor.flags |= ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT;

        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
}

static void EnNutsball_Draw(Actor* thisx, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);

    Matrix_RotateZF(thisx->home.rot.z * 9.58738e-05f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sDLists[thisx->params]);

    CLOSE_DISPS(play->state.gfxCtx);
}
