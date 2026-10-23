// OoT's equipment on MM's Player (docs/PAUSE.md). OoT's child shields come from OoT's object_link_child;
// adult Link's pieces wait for the adult body.

#include "global.h"
#include "HyrulePlayer.h"
#include "HyruleInventory.h"
#include "HyrulePort.h"
#include "assets/oot/objects/object_link_child/object_link_child.h"

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
