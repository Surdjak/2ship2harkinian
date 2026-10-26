// OoT's equipment on MM's Player (docs/PAUSE.md). OoT's child shields come from OoT's object_link_child;
// adult Link's pieces wait for the adult body.

#include "global.h"
#include "HyrulePlayer.h"
#include "HyruleInventory.h"
#include "HyruleIds.h"
#include "HyrulePort.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "assets/oot/objects/object_link_child/object_link_child.h"
#include "assets/oot/objects/gameplay_keep/gameplay_keep.h"
// hyrule/overlays/actors, not MM's Zora boomerang.
#include "./overlays/actors/ovl_En_Boom/z_en_boom.h"

// The shield part of gLinkChildHylianShieldAndSheathNearDL, declared in 3Ship's object_link_child.xml: MM
// Link keeps his own sheath. OoT's far version draws the shield first, so it cannot be split; near is used
// at both distances.
#define dgLinkChildHylianShieldNearDL "__OTR__oot/objects/object_link_child/gLinkChildHylianShieldNearDL"
static const ALIGN_ASSET(2) char gLinkChildHylianShieldNearDL[] = dgLinkChildHylianShieldNearDL;

static Gfx* sHandHoldingDekuShieldDLs[] = {
    (Gfx*)gLinkChildRightFistAndDekuShieldNearDL,
    (Gfx*)gLinkChildRightFistAndDekuShieldFarDL,
};

Gfx** Hyrule_GetHandHoldingShieldDLs(s32 shield) {
    return (shield == PLAYER_SHIELD_DEKU_Oot) ? sHandHoldingDekuShieldDLs : NULL;
}

Gfx* Hyrule_GetBackShieldDL(s32 shield) {
    switch (shield) {
        case PLAYER_SHIELD_DEKU_Oot:
            return (Gfx*)gLinkChildDekuShieldWithMatrixDL;
        case PLAYER_SHIELD_HYLIAN_Oot:
            return (Gfx*)gLinkChildHylianShieldNearDL;
        default:
            return NULL;
    }
}

void Hyrule_SetupBackShieldDraw(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    // What gLinkChildHylianShieldAndSheathNearDL sets before its shield part: no TLUT, no environment
    // mapping (MM's Gilded Sword leaves it on), fog and lighting, and OoT's render mode.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_NONE);
    gSPClearGeometryMode(POLY_OPA_DISP++, G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gSPSetGeometryMode(POLY_OPA_DISP++, G_FOG | G_LIGHTING | G_CULL_BACK);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2);

    CLOSE_DISPS(play->state.gfxCtx);
}

u8 Hyrule_GetShieldColMaterial(s32 shield) {
    return (shield == PLAYER_SHIELD_DEKU_Oot) ? COL_MATERIAL_WOOD : COL_MATERIAL_METAL;
}

s32 Hyrule_IsChildWithHylianShield(Player* player) {
    return (player->transformation == PLAYER_FORM_HUMAN) && !LINK_IS_ADULT_OOT &&
           (player->currentShield == PLAYER_SHIELD_HYLIAN_Oot);
}

u16 Hyrule_SwordValueToItem(u8 value) {
    switch (value) {
        case EQUIP_VALUE_SWORD_KOKIRI:
            return ITEM_SWORD_KOKIRI;
        case EQUIP_VALUE_SWORD_RAZOR:
            return ITEM_SWORD_RAZOR;
        case EQUIP_VALUE_SWORD_GILDED:
            return ITEM_SWORD_GILDED;
        case EQUIP_VALUE_SWORD_DIETY:
            return ITEM_SWORD_DEITY;
        case EQUIP_VALUE_SWORD_MASTER_Oot:
            return ITEM_SWORD_MASTER_Oot;
        case EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot:
            return ITEM_SWORD_GREAT_FAIRY;
        case EQUIP_VALUE_SWORD_KNIFE_Oot:
            return ITEM_SWORD_KNIFE_Oot;
        case EQUIP_VALUE_SWORD_BIGGORON_Oot:
            return ITEM_SWORD_BGS_Oot;
        default:
            return ITEM_NONE;
    }
}

u8 Hyrule_GetOwnedMmSwordValue(void) {
    if (CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_GILDED)) {
        return EQUIP_VALUE_SWORD_GILDED;
    }
    if (CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_RAZOR)) {
        return EQUIP_VALUE_SWORD_RAZOR;
    }
    if (CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KOKIRI)) {
        return EQUIP_VALUE_SWORD_KOKIRI;
    }
    return EQUIP_VALUE_SWORD_NONE;
}

static Gfx* sHandHoldingSlingshotDLs[] = {
    (Gfx*)gLinkChildRightHandHoldingSlingshotNearDL,
    (Gfx*)gLinkChildRightHandHoldingSlingshotFarDL,
};

s32 Hyrule_HoldsSlingshot(Player* player) {
    return (player->transformation == PLAYER_FORM_HUMAN) &&
           ((player->heldItemAction == PLAYER_IA_SLINGSHOT_Oot) || (player->heldItemId == ITEM_SLINGSHOT_Oot));
}

Gfx** Hyrule_GetHandHoldingSlingshotDLs(void) {
    return sHandHoldingSlingshotDLs;
}

Gfx* Hyrule_GetSlingshotFirstPersonDL(void) {
    return (Gfx*)gLinkChildRightArmStretchedSlingshotDL;
}

Gfx* Hyrule_GetSlingshotStringDL(void) {
    return (Gfx*)gLinkChildSlingshotStringDL;
}

void Hyrule_GetSlingshotStringPos(f32* x, f32* y, f32* z) {
    // OoT's sBowStringData for child Link.
    *x = 606.0f;
    *y = 236.0f;
    *z = 0.0f;
}

static void* sBoomerangAnims[] = {
    (void*)gPlayerAnim_link_boom_throw_wait2waitR,
    (void*)gPlayerAnim_link_boom_throwR,
    (void*)gPlayerAnim_link_boom_throwL,
    (void*)gPlayerAnim_link_boom_catch,
};

void* Hyrule_GetBoomerangAnim(s32 anim) {
    return sBoomerangAnims[anim];
}

Actor* Hyrule_ThrowBoomerang(PlayState* play, Player* player) {
    // OoT's func_808359FC.
    f32 posX = (Math_SinS(player->actor.shape.rot.y) * 10.0f) + player->actor.world.pos.x;
    f32 posZ = (Math_CosS(player->actor.shape.rot.y) * 10.0f) + player->actor.world.pos.z;
    s32 yaw = (player->focusActor != NULL) ? player->actor.shape.rot.y + 14000 : player->actor.shape.rot.y;
    EnBoomOot* boomerang =
        (EnBoomOot*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOOM_Oot, posX, player->actor.world.pos.y + 30.0f,
                                posZ, player->actor.focus.rot.x, yaw, 0, 0);

    if (boomerang == NULL) {
        return NULL;
    }
    boomerang->moveTo = player->focusActor;
    boomerang->returnTimer = 20;
    return &boomerang->actor;
}

static Gfx* sHandHoldingBoomerangDLs[] = {
    (Gfx*)gLinkChildLeftFistAndBoomerangNearDL,
    (Gfx*)gLinkChildLeftFistAndBoomerangFarDL,
};

Gfx** Hyrule_GetHandHoldingBoomerangDLs(void) {
    return sHandHoldingBoomerangDLs;
}

static Gfx* sMaskDLs_Oot[] = {
    (Gfx*)gLinkChildKeatonMaskDL, (Gfx*)gLinkChildSkullMaskDL, (Gfx*)gLinkChildSpookyMaskDL,
    (Gfx*)gLinkChildBunnyHoodDL,  (Gfx*)gLinkChildGoronMaskDL, (Gfx*)gLinkChildZoraMaskDL,
    (Gfx*)gLinkChildGerudoMaskDL, (Gfx*)gLinkChildMaskOfTruthDL,
};

// OoT's sBunnyEarKinematics.
static struct {
    Vec3s rot;
    Vec3s angVel;
} sBunnyEars;

static s32 Hyrule_IsMaskOnButton(PlayState* play, u16 item) {
    s32 i;

    for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
        if (GET_CUR_FORM_BTN_ITEM(i) == item) {
            return true;
        }
    }
    if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
        for (i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++) {
            if (DPAD_GET_CUR_FORM_BTN_ITEM(i) == item) {
                return true;
            }
        }
    }
    return false;
}

s32 Player_GetMaskOot(PlayState* play) {
    Player* player = GET_PLAYER(play);

    return (player->transformation == PLAYER_FORM_HUMAN) ? gSaveContext.save.equippedMaskOot : PLAYER_MASK_NONE_Oot;
}

void Hyrule_ToggleMaskOot(Player* player, s32 mask) {
    if (gSaveContext.save.equippedMaskOot == mask) {
        gSaveContext.save.equippedMaskOot = PLAYER_MASK_NONE_Oot;
    } else {
        gSaveContext.save.equippedMaskOot = mask;
        player->currentMask = PLAYER_MASK_NONE;
        gSaveContext.save.equippedMask = PLAYER_MASK_NONE;
    }
}

static void Hyrule_UpdateBunnyEars(Player* player) {
    Vec3s force;
    s16 angle;

    // Damping: decay by 1/8 the previous value each frame
    sBunnyEars.angVel.x -= sBunnyEars.angVel.x >> 3;
    sBunnyEars.angVel.y -= sBunnyEars.angVel.y >> 3;

    // Elastic restorative force
    sBunnyEars.angVel.x += -sBunnyEars.rot.x >> 2;
    sBunnyEars.angVel.y += -sBunnyEars.rot.y >> 2;

    // Forcing from motion relative to shape frame
    angle = player->actor.world.rot.y - player->actor.shape.rot.y;
    force.x = (s32)(player->actor.speed * -200.0f * Math_CosS(angle) * (Rand_CenteredFloat(2.0f) + 10.0f)) & 0xFFFF;
    force.y = (s32)(player->actor.speed * 100.0f * Math_SinS(angle) * (Rand_CenteredFloat(2.0f) + 10.0f)) & 0xFFFF;

    sBunnyEars.angVel.x += force.x >> 2;
    sBunnyEars.angVel.y += force.y >> 2;

    // Clamp both angular velocities to [-6000, 6000]
    sBunnyEars.angVel.x = CLAMP(sBunnyEars.angVel.x, -6000, 6000);
    sBunnyEars.angVel.y = CLAMP(sBunnyEars.angVel.y, -6000, 6000);

    // Add angular velocity to rotations
    sBunnyEars.rot.x += sBunnyEars.angVel.x;
    sBunnyEars.rot.y += sBunnyEars.angVel.y;

    // swivel ears outwards if bending backwards
    sBunnyEars.rot.z = (sBunnyEars.rot.x < 0) ? (sBunnyEars.rot.x >> 1) : 0;
}

void Hyrule_UpdateMaskOot(PlayState* play, Player* player) {
    s32 mask = gSaveContext.save.equippedMaskOot;

    if (mask == PLAYER_MASK_NONE_Oot) {
        return;
    }
    if ((player->transformation != PLAYER_FORM_HUMAN) || (player->currentMask != PLAYER_MASK_NONE) ||
        !Hyrule_IsMaskOnButton(play, ITEM_MASK_KEATON_Oot + mask - 1)) {
        gSaveContext.save.equippedMaskOot = PLAYER_MASK_NONE_Oot;
        return;
    }
    if (mask == PLAYER_MASK_BUNNY_Oot) {
        Hyrule_UpdateBunnyEars(player);
    }
}

void Hyrule_DrawMaskOot(PlayState* play, Player* player) {
    s32 mask = gSaveContext.save.equippedMaskOot;

    if ((mask == PLAYER_MASK_NONE_Oot) || (player->transformation != PLAYER_FORM_HUMAN)) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    if (mask == PLAYER_MASK_BUNNY_Oot) {
        Mtx* earMtx = GRAPH_ALLOC(play->state.gfxCtx, 2 * sizeof(Mtx));
        Vec3s earRot;

        Matrix_Push();
        gSPSegment(POLY_OPA_DISP++, 0x0B, earMtx);

        // Right ear
        earRot.x = sBunnyEars.rot.y + 0x3E2;
        earRot.y = sBunnyEars.rot.z + 0xDBE;
        earRot.z = sBunnyEars.rot.x - 0x348A;
        Matrix_SetTranslateRotateYXZ(97.0f, -1203.0f, -240.0f, &earRot);
        Matrix_ToMtx(earMtx++);

        // Left ear
        earRot.x = sBunnyEars.rot.y - 0x3E2;
        earRot.y = -0xDBE - sBunnyEars.rot.z;
        earRot.z = sBunnyEars.rot.x - 0x348A;
        Matrix_SetTranslateRotateYXZ(97.0f, -1203.0f, 240.0f, &earRot);
        Matrix_ToMtx(earMtx);
        Matrix_Pop();
    }

    gSPDisplayList(POLY_OPA_DISP++, sMaskDLs_Oot[mask - 1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_OnNewDay(void) {
    if (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) == ITEM_WEIRD_EGG_Oot) {
        gSaveContext.weirdEggHatchedOot = true;
    }
}

void Hyrule_UpdateEggHatching(PlayState* play) {
    // OoT's Inventory_HatchWeirdEgg, then its chicken cry and "the egg hatched" message.
    if (!gSaveContext.weirdEggHatchedOot || (play->csCtx.state != CS_STATE_IDLE) ||
        (Message_GetState(&play->msgCtx) != TEXT_STATE_NONE) || Player_InCsMode(play)) {
        return;
    }
    gSaveContext.weirdEggHatchedOot = false;
    if (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) == ITEM_WEIRD_EGG_Oot) {
        Inventory_ReplaceButtonItem(play, ITEM_WEIRD_EGG_Oot, ITEM_CHICKEN_Oot);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) = ITEM_CHICKEN_Oot;
        Audio_PlaySfx(NA_SE_EV_CHICKEN_CRY_M);
        Message_StartTextbox(play, TEXT_OOT(0x3066), NULL);
    }
}
