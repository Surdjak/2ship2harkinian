// OoT's equipment on MM's Player (docs/PAUSE.md). OoT's child shields come from OoT's object_link_child;
// adult Link's pieces wait for the adult body.

#include "global.h"
#include <assert.h>
#include "HyrulePlayer.h"
#include "HyruleInventory.h"
#include "HyruleIds.h"
#include "HyrulePort.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "assets/oot/objects/object_link_child/object_link_child.h"
#include "assets/oot/objects/object_link_boy/object_link_boy.h"
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
            // OoT's ITEM_SWORD_BGS is the whole Giant's Knife (and Biggoron's Sword), ITEM_SWORD_KNIFE the broken one.
            return CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE) ? ITEM_SWORD_BGS_Oot
                                                                                  : ITEM_SWORD_KNIFE_Oot;
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

void Player_UnsetMaskOot(PlayState* play) {
    gSaveContext.save.equippedMaskOot = PLAYER_MASK_NONE_Oot;
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

// Adult Link's head matrix, from his head's post-limb draw this frame.
static MtxF sAdultHeadMtxF;
#define ADULT_HEAD_MTX_COUNT 8

void Hyrule_SetAdultHeadMatrix(void) {
    Matrix_Get(&sAdultHeadMtxF);
}

void Hyrule_FitMaskToAdultHead(void) {
    // From the two heads' models (limb space: x up from the neck, -y forward, z across): adult Link's face is as
    // wide as child Link's, a little deeper and about a quarter longer; the chins line up.
    Matrix_Translate(-10.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(1.25f, 1.15f, 1.05f, MTXMODE_APPLY);
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

    if (Hyrule_IsAdultPlayer(player)) {
        // OoT's masks load the head's matrix from the skeleton's (segment 0x0D, limb matrix 7): here, the fitted one
        // for each limb matrix.
        Mtx* headMtx = GRAPH_ALLOC(play->state.gfxCtx, ADULT_HEAD_MTX_COUNT * sizeof(Mtx));
        s32 i;

        Matrix_Push();
        Matrix_Put(&sAdultHeadMtxF);
        Hyrule_FitMaskToAdultHead();
        for (i = 0; i < ADULT_HEAD_MTX_COUNT; i++) {
            Matrix_ToMtx(&headMtx[i]);
        }
        Matrix_Pop();
        gSPSegment(POLY_OPA_DISP++, 0x0D, headMtx);
    }
    gSPDisplayList(POLY_OPA_DISP++, sMaskDLs_Oot[mask - 1]);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_OnNewDay(void) {
    if ((INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) == ITEM_WEIRD_EGG_Oot) ||
        (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) == ITEM_POCKET_EGG_Oot)) {
        gSaveContext.weirdEggHatchedOot = true;
    }
}

void Hyrule_UpdateEggHatching(PlayState* play) {
    // OoT's Inventory_HatchWeirdEgg and Inventory_HatchPocketCucco, then the chicken cry and "the egg hatched".
    if (!gSaveContext.weirdEggHatchedOot || (play->csCtx.state != CS_STATE_IDLE) ||
        (Message_GetState(&play->msgCtx) != TEXT_STATE_NONE) || Player_InCsMode(play)) {
        return;
    }
    gSaveContext.weirdEggHatchedOot = false;
    if ((INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) != ITEM_WEIRD_EGG_Oot) &&
        (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) != ITEM_POCKET_EGG_Oot)) {
        return;
    }
    if (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) == ITEM_WEIRD_EGG_Oot) {
        Inventory_ReplaceButtonItem(play, ITEM_WEIRD_EGG_Oot, ITEM_CHICKEN_Oot);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_CHILD) = ITEM_CHICKEN_Oot;
    }
    if (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) == ITEM_POCKET_EGG_Oot) {
        Inventory_ReplaceButtonItem(play, ITEM_POCKET_EGG_Oot, ITEM_POCKET_CUCCO_Oot);
        INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) = ITEM_POCKET_CUCCO_Oot;
    }
    Audio_PlaySfx(NA_SE_EV_CHICKEN_CRY_M);
    Message_StartTextbox(play, TEXT_OOT(0x3066), NULL);
}

void Hyrule_RevertSpoilingItemsOot(PlayState* play) {
    // OoT's gSpoilingItems and gSpoilingItemReverts.
    static const u16 sSpoiling[][2] = {
        { ITEM_ODD_MUSHROOM_Oot, ITEM_COJIRO_Oot },
        { ITEM_FROG_Oot, ITEM_PRESCRIPTION_Oot },
        { ITEM_EYEDROPS_Oot, ITEM_PRESCRIPTION_Oot },
    };
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sSpoiling); i++) {
        if (INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) == sSpoiling[i][0]) {
            INV_CONTENT_SLOT_OOT(SLOT_OOT_TRADE_ADULT) = sSpoiling[i][1];
            Inventory_ReplaceButtonItem(play, sSpoiling[i][0], sSpoiling[i][1]);
        }
    }
}

// #region Adult Link: OoT's adult model (object_link_boy) on MM's Player, as OoT's z_player_lib.c draws it.

s32 Hyrule_IsAdultPlayer(Player* player) {
    return (player->actor.id == ACTOR_PLAYER) && (player->transformation == PLAYER_FORM_HUMAN) && LINK_IS_ADULT_OOT;
}

FlexSkeletonHeader* Hyrule_GetAdultSkeleton(void) {
    return (FlexSkeletonHeader*)gLinkAdultSkel;
}

// MM's eye order (PlayerEyes) on OoT's adult textures.
static TexturePtr sAdultEyesTextures[PLAYER_EYES_MAX] = {
    (TexturePtr)gLinkAdultEyesOpenTex,      (TexturePtr)gLinkAdultEyesHalfTex,     // PLAYER_EYES_OPEN, HALF
    (TexturePtr)gLinkAdultEyesClosedfTex,   (TexturePtr)gLinkAdultEyesRollRightTex, // PLAYER_EYES_CLOSED, RIGHT
    (TexturePtr)gLinkAdultEyesRollLeftTex,  (TexturePtr)gLinkAdultEyesShockTex,     // PLAYER_EYES_LEFT, UP
    (TexturePtr)gLinkAdultEyesUnk1Tex,      (TexturePtr)gLinkAdultEyesUnk2Tex,      // PLAYER_EYES_DOWN, WINCING
};

static TexturePtr sAdultMouthTextures[PLAYER_MOUTH_MAX] = {
    (TexturePtr)gLinkAdultMouth1Tex,
    (TexturePtr)gLinkAdultMouth2Tex,
    (TexturePtr)gLinkAdultMouth3Tex,
    (TexturePtr)gLinkAdultMouth4Tex,
};

// OoT's sTunicColors, by MM's tunic values (EQUIP_VALUE_TUNIC_*, which MM kept from OoT).
static Color_RGB8 sTunicColors[] = {
    { 30, 105, 27 }, // Kokiri
    { 100, 20, 0 },  // Goron
    { 0, 60, 100 },  // Zora
};

Gfx* Hyrule_SetupAdultDraw(Gfx* gfx, s32 eyeIndex, s32 mouthIndex) {
    s32 tunic = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_TUNIC) - EQUIP_VALUE_TUNIC_KOKIRI;
    Color_RGB8* color;

    if ((tunic < 0) || (tunic >= ARRAY_COUNT(sTunicColors))) {
        tunic = 0;
    }
    color = &sTunicColors[tunic];

    gSPSegment(gfx++, 0x08, Lib_SegmentedToVirtual(sAdultEyesTextures[eyeIndex]));
    gSPSegment(gfx++, 0x09, Lib_SegmentedToVirtual(sAdultMouthTextures[mouthIndex]));
    gDPSetEnvColor(gfx++, color->r, color->g, color->b, 0);
    return gfx;
}

// Near and far display lists for each hand, sheath and waist, from OoT's sPlayerDListGroups (adult entries).
static Gfx* sAdultLeftHandOpenDLs[] = { (Gfx*)gLinkAdultLeftHandNearDL, (Gfx*)gLinkAdultLeftHandFarDL };
static Gfx* sAdultLeftHandClosedDLs[] = { (Gfx*)gLinkAdultLeftHandClosedNearDL, (Gfx*)gLinkAdultLeftHandClosedFarDL };
static Gfx* sAdultLeftHandMasterSwordDLs[] = { (Gfx*)gLinkAdultLeftHandHoldingMasterSwordNearDL,
                                               (Gfx*)gLinkAdultLeftHandHoldingMasterSwordFarDL };
static Gfx* sAdultLeftHandBgsDLs[] = { (Gfx*)gLinkAdultLeftHandHoldingBgsNearDL,
                                       (Gfx*)gLinkAdultLeftHandHoldingBgsFarDL };
static Gfx* sAdultLeftHandBrokenKnifeDLs[] = { (Gfx*)gLinkAdultHandHoldingBrokenGiantsKnifeDL,
                                               (Gfx*)gLinkAdultHandHoldingBrokenGiantsKnifeFarDL };
static Gfx* sAdultLeftHandHammerDLs[] = { (Gfx*)gLinkAdultLeftHandHoldingHammerNearDL,
                                          (Gfx*)gLinkAdultLeftHandHoldingHammerFarDL };
static Gfx* sAdultLeftHandBottleDLs[] = { (Gfx*)gLinkAdultLeftHandOutNearDL, (Gfx*)gLinkAdultLeftHandOutNearDL };
static Gfx* sAdultRightHandOpenDLs[] = { (Gfx*)gLinkAdultRightHandNearDL, (Gfx*)gLinkAdultRightHandFarDL };
static Gfx* sAdultRightHandClosedDLs[] = { (Gfx*)gLinkAdultRightHandClosedNearDL,
                                           (Gfx*)gLinkAdultRightHandClosedFarDL };
static Gfx* sAdultRightHandHylianShieldDLs[] = { (Gfx*)gLinkAdultRightHandHoldingHylianShieldNearDL,
                                                 (Gfx*)gLinkAdultRightHandHoldingHylianShieldFarDL };
static Gfx* sAdultRightHandMirrorShieldDLs[] = { (Gfx*)gLinkAdultRightHandHoldingMirrorShieldNearDL,
                                                 (Gfx*)gLinkAdultRightHandHoldingMirrorShieldFarDL };
static Gfx* sAdultRightHandBowDLs[] = { (Gfx*)gLinkAdultRightHandHoldingBowNearDL,
                                        (Gfx*)gLinkAdultRightHandHoldingBowFarDL };
static Gfx* sAdultRightHandOcarinaDLs[] = { (Gfx*)gLinkAdultRightHandHoldingOotNearDL,
                                            (Gfx*)gLinkAdultRightHandHoldingOotFarDL };
// The far hookshot hand exists but OoT does not use it.
static Gfx* sAdultRightHandHookshotDLs[] = { (Gfx*)gLinkAdultRightHandHoldingHookshotNearDL,
                                             (Gfx*)gLinkAdultRightHandHoldingHookshotNearDL };
static Gfx* sAdultWaistDLs[] = { (Gfx*)gLinkAdultWaistNearDL, (Gfx*)gLinkAdultWaistFarDL };
// The sheath on his back, by what it holds: [shield on the back: none, Hylian, Mirror][sword sheathed][lod].
static Gfx* sAdultSheathDLs[3][2][2] = {
    { { (Gfx*)gLinkAdultSheathNearDL, (Gfx*)gLinkAdultSheathFarDL },
      { (Gfx*)gLinkAdultMasterSwordAndSheathNearDL, (Gfx*)gLinkAdultMasterSwordAndSheathFarDL } },
    { { (Gfx*)gLinkAdultHylianShieldAndSheathNearDL, (Gfx*)gLinkAdultHylianShieldAndSheathFarDL },
      { (Gfx*)gLinkAdultHylianShieldSwordAndSheathNearDL, (Gfx*)gLinkAdultHylianShieldSwordAndSheathFarDL } },
    { { (Gfx*)gLinkAdultMirrorShieldAndSheathNearDL, (Gfx*)gLinkAdultMirrorShieldAndSheathFarDL },
      { (Gfx*)gLinkAdultMirrorShieldSwordAndSheathNearDL, (Gfx*)gLinkAdultMirrorShieldSwordAndSheathFarDL } },
};

// Which of OoT's adult shields stands for Player's: 0 none, 1 Hylian (also MM's Hero's Shield, the same design),
// 2 Mirror (OoT's, and for now MM's).
static s32 Hyrule_GetAdultShield(Player* player) {
    switch (player->currentShield) {
        case PLAYER_SHIELD_HEROS_SHIELD:
        case PLAYER_SHIELD_HYLIAN_Oot:
            return 1;
        case PLAYER_SHIELD_MIRROR_SHIELD:
        case PLAYER_SHIELD_MIRROR_Oot:
            return 2;
        default:
            return 0;
    }
}

s32 Hyrule_OverrideLimbDrawAdult(Player* player, s32 limbIndex, Gfx** dList, s32 lod, PlayerModelType* leftHandType,
                                 PlayerModelType* rightHandType) {
    Gfx** dLists;

    if (limbIndex == PLAYER_LIMB_LEFT_HAND) {
        switch (*leftHandType) {
            case PLAYER_MODELTYPE_LH_CLOSED:
                dLists = sAdultLeftHandClosedDLs;
                break;
            case PLAYER_MODELTYPE_LH_ONE_HAND_SWORD:
                dLists = sAdultLeftHandMasterSwordDLs;
                break;
            case PLAYER_MODELTYPE_LH_TWO_HAND_SWORD:
                // Biggoron's Sword's hand also holds the Great Fairy's Sword, which OoT's adult Link never held.
                if (player->heldItemAction == PLAYER_IA_HAMMER_Oot) {
                    dLists = sAdultLeftHandHammerDLs;
                } else if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) == ITEM_SWORD_KNIFE_Oot) {
                    dLists = sAdultLeftHandBrokenKnifeDLs;
                } else {
                    dLists = sAdultLeftHandBgsDLs;
                }
                break;
            case PLAYER_MODELTYPE_LH_BOTTLE:
                dLists = sAdultLeftHandBottleDLs;
                break;
            default:
                dLists = sAdultLeftHandOpenDLs;
                // A running open hand closes, as MM's and OoT's do.
                if ((player->actor.speed > 2.0f) && !(player->stateFlags1 & PLAYER_STATE1_8000000)) {
                    dLists = sAdultLeftHandClosedDLs;
                    *leftHandType = PLAYER_MODELTYPE_LH_CLOSED;
                }
                break;
        }
        *dList = dLists[lod];
        return true;
    }

    if (limbIndex == PLAYER_LIMB_RIGHT_HAND) {
        switch (*rightHandType) {
            case PLAYER_MODELTYPE_RH_CLOSED:
                dLists = sAdultRightHandClosedDLs;
                break;
            case PLAYER_MODELTYPE_RH_SHIELD:
                switch (Hyrule_GetAdultShield(player)) {
                    case 1:
                        dLists = sAdultRightHandHylianShieldDLs;
                        break;
                    case 2:
                        dLists = sAdultRightHandMirrorShieldDLs;
                        break;
                    default:
                        dLists = sAdultRightHandClosedDLs;
                        break;
                }
                break;
            case PLAYER_MODELTYPE_RH_BOW:
                dLists = sAdultRightHandBowDLs;
                break;
            case PLAYER_MODELTYPE_RH_INSTRUMENT:
                dLists = sAdultRightHandOcarinaDLs;
                break;
            case PLAYER_MODELTYPE_RH_HOOKSHOT:
                dLists = sAdultRightHandHookshotDLs;
                break;
            default:
                dLists = sAdultRightHandOpenDLs;
                if ((player->actor.speed > 2.0f) && !(player->stateFlags1 & PLAYER_STATE1_8000000)) {
                    dLists = sAdultRightHandClosedDLs;
                    *rightHandType = PLAYER_MODELTYPE_RH_CLOSED;
                }
                break;
        }
        *dList = dLists[lod];
        return true;
    }

    if (limbIndex == PLAYER_LIMB_SHEATH) {
        // MM's sheath types 12 and 14 hold the sword; the shield is on his back unless it is in his hand.
        s32 swordSheathed = (player->sheathType == PLAYER_MODELTYPE_SHEATH_12) ||
                            (player->sheathType == PLAYER_MODELTYPE_SHEATH_14);
        s32 backShield = (*rightHandType == PLAYER_MODELTYPE_RH_SHIELD) ? 0 : Hyrule_GetAdultShield(player);

        *dList = sAdultSheathDLs[backShield][swordSheathed][lod];
        return true;
    }

    if (limbIndex == PLAYER_LIMB_WAIST) {
        *dList = sAdultWaistDLs[lod];
        return true;
    }

    return false;
}

Gfx* Hyrule_GetAdultFirstPersonDL(Player* player, s32 limbIndex) {
    switch (limbIndex) {
        case PLAYER_LIMB_LEFT_FOREARM:
            return (Gfx*)gLinkAdultRightArmOutNearDL;
        case PLAYER_LIMB_LEFT_HAND:
            return (Gfx*)gLinkAdultRightHandOutNearDL;
        case PLAYER_LIMB_RIGHT_SHOULDER:
            return (Gfx*)gLinkAdultRightShoulderNearDL;
        case PLAYER_LIMB_RIGHT_FOREARM:
            return (Gfx*)gLinkAdultLeftArmOutNearDL;
        case PLAYER_LIMB_RIGHT_HAND:
            return (Gfx*)(Player_IsHoldingHookshot(player) ? gLinkAdultRightHandHoldingHookshotFarDL
                                                           : gLinkAdultRightHandHoldingBowFirstPersonDL);
        default:
            return NULL;
    }
}

Gfx* Hyrule_GetAdultBowStringDL(void) {
    return (Gfx*)gLinkAdultBowStringDL;
}

void Hyrule_GetAdultBowStringPos(f32* x, f32* y, f32* z) {
    *x = 0.0f;
    *y = -360.4f;
    *z = 0.0f;
}
// #endregion

// #region Adult equipment (docs/ADULT_LINK.md, step 3)

typedef enum SwordClassOot {
    SWORD_CLASS_KOKIRI,
    SWORD_CLASS_MASTER,
    SWORD_CLASS_GIANT,
} SwordClassOot;

// OoT's damage types by class, slash and jump (OoT's D_80854488).
static const u64 sSwordDmgFlagsOot[][2] = {
    { DMG_SLASH_KOKIRI_Oot, DMG_JUMP_KOKIRI_Oot },
    { DMG_SLASH_MASTER_Oot, DMG_JUMP_MASTER_Oot },
    { DMG_SLASH_GIANT_Oot, DMG_JUMP_GIANT_Oot },
};

s32 Hyrule_HoldsBrokenKnife(Player* player) {
    return (player->transformation == PLAYER_FORM_HUMAN) && (player->heldItemAction == PLAYER_IA_SWORD_TWO_HANDED) &&
           (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) == ITEM_SWORD_KNIFE_Oot);
}

void Hyrule_SetSwordDamage(Player* player, s32 strong, u64* dmgFlags, s32* damage) {
    s32 swordClass;

    if ((player->transformation != PLAYER_FORM_HUMAN) || !(*dmgFlags & DMG_SWORD)) {
        return;
    }
    if (player->heldItemAction == PLAYER_IA_HAMMER_Oot) {
        // OoT's hammer swing and jump, and MM's Goron punch and pound, which stand in for them (DMG_FROM_OOT), as
        // hard as the Master Sword (OoT's tables mostly give both 2, and 4 jumping).
        *dmgFlags = strong ? (DMG_GORON_POUND | DMG_HAMMER_JUMP_Oot) : (DMG_GORON_PUNCH | DMG_HAMMER_SWING_Oot);
        *damage = strong ? 4 : 2;
        return;
    }
    switch (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD)) {
        case EQUIP_VALUE_SWORD_KOKIRI:
            swordClass = SWORD_CLASS_KOKIRI;
            break;
        case EQUIP_VALUE_SWORD_MASTER_Oot:
            swordClass = SWORD_CLASS_MASTER;
            // MM's one-handed path is the Gilded Sword's: the Master Sword hits as the Razor Sword.
            *damage = strong ? 4 : 2;
            break;
        case EQUIP_VALUE_SWORD_KNIFE_Oot:
            if (Hyrule_HoldsBrokenKnife(player)) {
                swordClass = SWORD_CLASS_KOKIRI;
                *damage = strong ? 2 : 1;
            } else {
                swordClass = SWORD_CLASS_GIANT;
            }
            break;
        case EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot:
        case EQUIP_VALUE_SWORD_BIGGORON_Oot:
            swordClass = SWORD_CLASS_GIANT;
            break;
        default: // the Razor and Gilded Swords
            swordClass = SWORD_CLASS_MASTER;
            break;
    }
    *dmgFlags |= sSwordDmgFlagsOot[swordClass][strong ? 1 : 0];
}

s32 Hyrule_WearGiantsKnife(PlayState* play, Player* player) {
    InventoryOot* inventory = &gSaveContext.save.saveInfo.inventoryOot;

    if ((player->transformation != PLAYER_FORM_HUMAN) || (player->heldItemAction != PLAYER_IA_SWORD_TWO_HANDED) ||
        (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) != EQUIP_VALUE_SWORD_KNIFE_Oot) ||
        (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) != ITEM_SWORD_BGS_Oot)) {
        return false;
    }
    if (inventory->knifeHealth > 0) {
        if (--inventory->knifeHealth <= 0) {
            // OoT's func_800849EC: the Giant's Knife becomes the broken one, in hand and on B.
            EffectSsStick_Spawn(play, &player->bodyPartsPos[PLAYER_BODYPART_RIGHT_HAND],
                                BINANG_ADD(player->actor.shape.rot.y, 0x8000));
            inventory->equipmentOwned &= ~EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE);
            inventory->equipmentOwned |= EQUIP_OWNED_BIT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_KNIFE_BROKEN);
            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_KNIFE_Oot;
            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
            Player_PlaySfx(player, NA_SE_IT_MAJIN_SWORD_BROKEN);
        }
    }
    return true;
}

u8 Hyrule_GetAdultBoots(void) {
    switch (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_BOOTS_Oot)) {
        case EQUIP_VALUE_BOOTS_IRON_Oot:
            return PLAYER_BOOTS_IRON_Oot;
        case EQUIP_VALUE_BOOTS_HOVER_Oot:
            return PLAYER_BOOTS_HOVER_Oot;
        default:
            return PLAYER_BOOTS_KOKIRI_ADULT_Oot;
    }
}

// OoT's PLAYER_ANIMGROUP_heavy_run.
static void* sHeavyRunAnims[PLAYER_ANIMTYPE_MAX] = {
    (void*)gPlayerAnim_link_normal_heavy_run_free, (void*)gPlayerAnim_link_normal_heavy_run,
    (void*)gPlayerAnim_link_normal_heavy_run_free, (void*)gPlayerAnim_link_fighter_heavy_run_long,
    (void*)gPlayerAnim_link_normal_heavy_run_free, (void*)gPlayerAnim_link_normal_heavy_run_free,
};

void* Hyrule_GetHeavyRunAnim(s32 modelAnimType) {
    return sHeavyRunAnims[modelAnimType];
}

void Hyrule_DrawAdultBoots(PlayState* play) {
    // OoT's sBootDListGroups; the boots load the feet's matrices themselves.
    static Gfx* sBootDLs[][2] = {
        { (Gfx*)gLinkAdultLeftIronBootDL, (Gfx*)gLinkAdultRightIronBootDL },
        { (Gfx*)gLinkAdultLeftHoverBootDL, (Gfx*)gLinkAdultRightHoverBootDL },
    };
    s32 boots = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_BOOTS_Oot);

    if ((boots != EQUIP_VALUE_BOOTS_IRON_Oot) && (boots != EQUIP_VALUE_BOOTS_HOVER_Oot)) {
        return;
    }
    OPEN_DISPS(play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sBootDLs[boots - EQUIP_VALUE_BOOTS_IRON_Oot][0]);
    gSPDisplayList(POLY_OPA_DISP++, sBootDLs[boots - EQUIP_VALUE_BOOTS_IRON_Oot][1]);
    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_DrawHoverBootsGlow(PlayState* play, Player* player) {
    static Vec3s sRot = { 0, 0, 0 };
    s32 timer = player->hoverBootsTimerOot;
    s32 alpha;

    if ((player->currentBoots != PLAYER_BOOTS_HOVER_Oot) || (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        (player->stateFlags1 & PLAYER_STATE1_800000) || (timer == 0) || (timer >= 19)) {
        return;
    }
    // Fades in over 4 frames, then pulses out.
    if (timer >= 15) {
        alpha = (19 - timer) * 51.0f;
    } else {
        s32 t = MIN(timer, 9);
        s32 angle = (-t * 4) + 36;

        alpha = ((s32)((Math_CosS(angle * angle) * 100.0f) + 100.0f) + 55.0f) * (t * (1.0f / 9.0f));
    }

    OPEN_DISPS(play->state.gfxCtx);
    Matrix_Push();
    Matrix_SetTranslateRotateYXZ(player->actor.world.pos.x, player->actor.world.pos.y + 2.0f,
                                 player->actor.world.pos.z, &sRot);
    Matrix_Scale(4.0f, 4.0f, 4.0f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 16, 32, 1, 0, (play->gameplayFrames * -15) % 128, 16,
                                  32, 0, 0, 0, -15));
    gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 255, alpha);
    gDPSetEnvColor(POLY_XLU_DISP++, 120, 90, 30, 128);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gHoverBootsCircleDL);
    Matrix_Pop();
    CLOSE_DISPS(play->state.gfxCtx);
}

s32 Hyrule_GetHammerGroundPos(PlayState* play, Player* player, Vec3f* pos) {
    // OoT's func_8083973C with D_80854A40: the floor under a point 40 up and 45 ahead, within 40 of Link's feet.
    CollisionPoly* poly;
    s32 bgId;
    f32 sin = Math_SinS(player->actor.shape.rot.y);
    f32 cos = Math_CosS(player->actor.shape.rot.y);
    f32 floorY;

    pos->x = player->actor.world.pos.x + (45.0f * sin);
    pos->y = player->actor.world.pos.y + 40.0f;
    pos->z = player->actor.world.pos.z + (45.0f * cos);
    floorY = BgCheck_EntityRaycastFloor5(&play->colCtx, &poly, &bgId, &player->actor, pos);
    pos->y = floorY;
    return ((player->actor.world.pos.y - floorY) > -40.0f) && ((player->actor.world.pos.y - floorY) < 40.0f);
}

f32 Hyrule_GetDiveDepth(void) {
    static const f32 sDiveDepths[] = { 120.0f, 240.0f, 360.0f };

    return sDiveDepths[CLAMP_MAX(CUR_UPG_VALUE(UPG_SCALE), ARRAY_COUNT(sDiveDepths) - 1)];
}

s32 Hyrule_GetHookshotReach(Player* player) {
    return (player->heldItemId == ITEM_HOOKSHOT_Oot) ? 13 : 26;
}

static const u8 sExchangeItemActionsOot[EXCH_ITEM_MAX] = {
    PLAYER_IA_NONE,                // EXCH_ITEM_NONE
    PLAYER_IA_LETTER_ZELDA_Oot,    // EXCH_ITEM_LETTER_ZELDA
    PLAYER_IA_WEIRD_EGG_Oot,       // EXCH_ITEM_WEIRD_EGG
    PLAYER_IA_CHICKEN_Oot,         // EXCH_ITEM_CHICKEN
    PLAYER_IA_MAGIC_BEANS,         // EXCH_ITEM_BEAN
    PLAYER_IA_POCKET_EGG_Oot,      // EXCH_ITEM_POCKET_EGG
    PLAYER_IA_POCKET_CUCCO_Oot,    // EXCH_ITEM_POCKET_CUCCO
    PLAYER_IA_COJIRO_Oot,          // EXCH_ITEM_COJIRO
    PLAYER_IA_ODD_MUSHROOM_Oot,    // EXCH_ITEM_ODD_MUSHROOM
    PLAYER_IA_ODD_POTION_Oot,      // EXCH_ITEM_ODD_POTION
    PLAYER_IA_SAW_Oot,             // EXCH_ITEM_SAW
    PLAYER_IA_SWORD_BROKEN_Oot,    // EXCH_ITEM_SWORD_BROKEN
    PLAYER_IA_PRESCRIPTION_Oot,    // EXCH_ITEM_PRESCRIPTION
    PLAYER_IA_FROG_Oot,            // EXCH_ITEM_FROG
    PLAYER_IA_EYEDROPS_Oot,        // EXCH_ITEM_EYEDROPS
    PLAYER_IA_CLAIM_CHECK_Oot,     // EXCH_ITEM_CLAIM_CHECK
    PLAYER_IA_MASK_SKULL_Oot,      // EXCH_ITEM_MASK_SKULL
    PLAYER_IA_MASK_SPOOKY_Oot,     // EXCH_ITEM_MASK_SPOOKY
    PLAYER_IA_MASK_KEATON_Oot,     // EXCH_ITEM_MASK_KEATON
    PLAYER_IA_MASK_BUNNY_Oot,      // EXCH_ITEM_MASK_BUNNY
    PLAYER_IA_MASK_TRUTH_Oot,      // EXCH_ITEM_MASK_TRUTH
    PLAYER_IA_MASK_GORON_Oot,      // EXCH_ITEM_MASK_GORON
    PLAYER_IA_MASK_ZORA_Oot,       // EXCH_ITEM_MASK_ZORA
    PLAYER_IA_MASK_GERUDO_Oot,     // EXCH_ITEM_MASK_GERUDO
    PLAYER_IA_BOTTLE_FISH,         // EXCH_ITEM_FISH
    PLAYER_IA_BOTTLE_1C,           // EXCH_ITEM_BLUE_FIRE, MM's unused Blue Fire bottle
    PLAYER_IA_BOTTLE_BUG,          // EXCH_ITEM_BUG
    PLAYER_IA_BOTTLE_POE,          // EXCH_ITEM_POE
    PLAYER_IA_BOTTLE_BIG_POE,      // EXCH_ITEM_BIG_POE
    PLAYER_IA_NONE,                // EXCH_ITEM_LETTER_RUTO
};

PlayerItemAction Hyrule_ExchangeItemToActionOot(s32 exchangeItem) {
    return ((exchangeItem > EXCH_ITEM_NONE) && (exchangeItem < EXCH_ITEM_MAX)) ? sExchangeItemActionsOot[exchangeItem]
                                                                               : PLAYER_IA_NONE;
}

s32 Hyrule_ActionToExchangeItemOot(PlayerItemAction itemAction) {
    s32 i;

    if (itemAction <= PLAYER_IA_NONE) {
        return EXCH_ITEM_NONE;
    }
    for (i = EXCH_ITEM_NONE + 1; i < EXCH_ITEM_MAX; i++) {
        if (sExchangeItemActionsOot[i] == itemAction) {
            return i;
        }
    }
    return EXCH_ITEM_NONE;
}

s32 Actor_GetPlayerExchangeItemIdOot(PlayState* play) {
    return Hyrule_ActionToExchangeItemOot(GET_PLAYER(play)->exchangeItemAction);
}

s32 Hyrule_IsExchangeAcceptedOot(PlayerItemAction offer, PlayerItemAction shown) {
    return (offer > PLAYER_IA_NONE) && ((offer == shown) || (offer == PLAYER_IA_BOTTLE_1C) ||
                                        ((offer == PLAYER_IA_BOTTLE_POE) && (shown == PLAYER_IA_BOTTLE_BIG_POE)));
}

void Player_SetCsActionWithHaltedActorsOot(PlayState* play, Actor* csActor, u8 ootCsAction) {
    u8 csAction;

    switch (ootCsAction) {
        case 7:
            csAction = PLAYER_CSACTION_END;
            break;
        case 8:
            csAction = PLAYER_CSACTION_WAIT;
            break;
        case 95:
            csAction = PLAYER_CSACTION_26;
            break;
        default:
            // 0 and 1 are the same in both games.
            assert((ootCsAction == PLAYER_CSACTION_NONE) || (ootCsAction == PLAYER_CSACTION_1));
            csAction = ootCsAction;
            break;
    }
    Player_SetCsActionWithHaltedActors(play, csActor, csAction);
}

// OoT's sReactionTextIds (z_face_reaction.c): per reaction set, the text for no mask then each PlayerMask.
static const u16 sReactionTextIdsOot[][PLAYER_MASK_TRUTH_Oot + 1] = {
    { 0x0000, 0x7124, 0x7127, 0x7126, 0x7125, 0x7127, 0x7124, 0x7125, 0x7127 },
    { 0x0000, 0x7128, 0x7129, 0x7128, 0x7128, 0x7128, 0x7128, 0x712A, 0x712B },
    { 0x0000, 0x7128, 0x712B, 0x7128, 0x7128, 0x7129, 0x7128, 0x712B, 0x7128 },
    { 0x0000, 0x7128, 0x7129, 0x7128, 0x7128, 0x7128, 0x7128, 0x712A, 0x712B },
    { 0x0000, 0x7128, 0x7129, 0x712B, 0x7128, 0x7128, 0x7128, 0x7129, 0x7128 },
    { 0x0000, 0x712D, 0x712D, 0x712D, 0x712D, 0x712D, 0x712D, 0x712D, 0x712F },
    { 0x0000, 0x712C, 0x712C, 0x712C, 0x712E, 0x712C, 0x712C, 0x712F, 0x712F },
    { 0x0000, 0x712C, 0x712C, 0x712C, 0x712F, 0x712C, 0x712C, 0x712F, 0x712F },
    { 0x0000, 0x7130, 0x7132, 0x7133, 0x7130, 0x7130, 0x7131, 0x7132, 0x7131 },
    { 0x0000, 0x7134, 0x7137, 0x7135, 0x7134, 0x7136, 0x7135, 0x7134, 0x7135 },
    { 0x0000, 0x7138, 0x713A, 0x7138, 0x7139, 0x713A, 0x7138, 0x7139, 0x713B },
    { 0x0000, 0x7144, 0x7146, 0x7144, 0x7146, 0x7147, 0x7145, 0x7145, 0x7147 },
    { 0x0000, 0x7148, 0x7149, 0x7149, 0x714A, 0x714A, 0x714B, 0x7149, 0x714B },
    { 0x0000, 0x714C, 0x714D, 0x714C, 0x714C, 0x714E, 0x714C, 0x714E, 0x714F },
    { 0x0000, 0x7150, 0x7153, 0x7152, 0x7150, 0x7151, 0x7153, 0x7153, 0x7151 },
    { 0x0000, 0x7155, 0x7156, 0x7157, 0x7154, 0x7156, 0x7156, 0x7156, 0x7156 },
    { 0x0000, 0x715A, 0x7159, 0x715B, 0x715A, 0x715A, 0x7158, 0x7158, 0x715B },
    { 0x0000, 0x715E, 0x715D, 0x715D, 0x715F, 0x715E, 0x715C, 0x715C, 0x715D },
    { 0x0000, 0x7163, 0x7162, 0x7160, 0x7163, 0x7160, 0x7161, 0x7161, 0x7160 },
    { 0x0000, 0x7164, 0x7166, 0x7164, 0x7167, 0x7164, 0x7164, 0x7164, 0x7167 },
    { 0x0000, 0x716B, 0x7169, 0x7168, 0x716B, 0x716A, 0x716B, 0x716B, 0x716A },
    { 0x0000, 0x716C, 0x716D, 0x716F, 0x716C, 0x716E, 0x716E, 0x716E, 0x716F },
    { 0x0000, 0x7171, 0x7173, 0x7170, 0x7172, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x0000, 0x7176, 0x7177, 0x7174, 0x7174, 0x7175, 0x7174, 0x7174, 0x7177 },
    { 0x0000, 0x7178, 0x7179, 0x7179, 0x717B, 0x717A, 0x717B, 0x717A, 0x717B },
    { 0x0000, 0x717D, 0x717C, 0x717C, 0x717D, 0x717F, 0x717C, 0x717E, 0x717D },
    { 0x0000, 0x7183, 0x7181, 0x7180, 0x7183, 0x7182, 0x7183, 0x7181, 0x7183 },
    { 0x0000, 0x7184, 0x7186, 0x7185, 0x7186, 0x7184, 0x7187, 0x7186, 0x7184 },
    { 0x0000, 0x71A4, 0x71A6, 0x71A5, 0x0000, 0x71A6, 0x71A6, 0x71A6, 0x71A7 },
    { 0x0000, 0x7188, 0x7188, 0x7189, 0x7188, 0x7189, 0x718B, 0x718A, 0x7189 },
    { 0x0000, 0x718C, 0x718C, 0x718D, 0x718C, 0x718E, 0x718F, 0x718D, 0x718C },
    { 0x0000, 0x7190, 0x7190, 0x7191, 0x7192, 0x7191, 0x7193, 0x7190, 0x7191 },
    { 0x0000, 0x7196, 0x7194, 0x7195, 0x7196, 0x7197, 0x7194, 0x7196, 0x7195 },
    { 0x0000, 0x7199, 0x719A, 0x7198, 0x7198, 0x719A, 0x719A, 0x719B, 0x7198 },
    { 0x0000, 0x719D, 0x719C, 0x719E, 0x719D, 0x719D, 0x719C, 0x719F, 0x719E },
    { 0x0000, 0x71A1, 0x71A0, 0x71A1, 0x71A2, 0x71A1, 0x71A2, 0x71A3, 0x71A2 },
    { 0x0000, 0x711C, 0x711E, 0x711C, 0x711F, 0x711E, 0x711C, 0x711D, 0x711F },
    { 0x0000, 0x7104, 0x7105, 0x7107, 0x7107, 0x7105, 0x7106, 0x7107, 0x7107 },
    { 0x0000, 0x7107, 0x7105, 0x7107, 0x7107, 0x7106, 0x7107, 0x7107, 0x7105 },
    { 0x0000, 0x7113, 0x7117, 0x7113, 0x7110, 0x7112, 0x7112, 0x7116, 0x7112 },
    { 0x0000, 0x7113, 0x7113, 0x7113, 0x7113, 0x7113, 0x7113, 0x7111, 0x7113 },
    { 0x0000, 0x7113, 0x7117, 0x7113, 0x7110, 0x7112, 0x7112, 0x7116, 0x7112 },
    { 0x0000, 0x7117, 0x7117, 0x7117, 0x7117, 0x7117, 0x7117, 0x7117, 0x7113 },
    { 0x0000, 0x7101, 0x7100, 0x7102, 0x7103, 0x7101, 0x7100, 0x7102, 0x7103 },
    { 0x0000, 0x7100, 0x7102, 0x7100, 0x7100, 0x7100, 0x7100, 0x7100, 0x7102 },
    { 0x0000, 0x710A, 0x7109, 0x7109, 0x710A, 0x710B, 0x7108, 0x7109, 0x710B },
    { 0x0000, 0x7117, 0x7112, 0x7113, 0x7110, 0x710C, 0x7117, 0x710E, 0x7112 },
    { 0x0000, 0x710D, 0x710F, 0x710C, 0x7112, 0x710D, 0x710C, 0x710C, 0x710F },
    { 0x0000, 0x710A, 0x7109, 0x711A, 0x710A, 0x7109, 0x7108, 0x710B, 0x7109 },
    { 0x0000, 0x710C, 0x710F, 0x7113, 0x7110, 0x710D, 0x7112, 0x7116, 0x710D },
    { 0x0000, 0x7115, 0x7114, 0x7114, 0x7115, 0x7114, 0x7114, 0x7116, 0x7117 },
    { 0x0000, 0x7113, 0x710F, 0x7113, 0x7110, 0x710C, 0x711A, 0x710D, 0x7112 },
    { 0x0000, 0x7101, 0x7102, 0x7103, 0x7101, 0x7100, 0x7100, 0x7102, 0x7100 },
    { 0x0000, 0x7112, 0x710E, 0x7112, 0x710E, 0x710D, 0x7112, 0x710E, 0x710F },
    { 0x0000, 0x7142, 0x7141, 0x7142, 0x7143, 0x7140, 0x7140, 0x7141, 0x7143 },
    { 0x0000, 0x713C, 0x713D, 0x713D, 0x713E, 0x713E, 0x713F, 0x713D, 0x713F },
    { 0x0000, 0x7101, 0x7102, 0x7103, 0x7101, 0x7100, 0x7100, 0x7102, 0x7100 },
    { 0x0000, 0x7113, 0x7117, 0x7113, 0x7110, 0x7112, 0x7112, 0x7116, 0x7112 },
    { 0x0000, 0x7104, 0x7105, 0x7107, 0x7105, 0x7105, 0x7105, 0x7107, 0x7107 },
    { 0x0000, 0x7104, 0x7105, 0x7107, 0x7105, 0x710C, 0x7105, 0x7107, 0x7107 },
};

u16 Text_GetFaceReactionOot(PlayState* play, u32 reactionSet) {
    u16 textId = sReactionTextIdsOot[reactionSet][Player_GetMaskOot(play)];

    return (textId != 0) ? TEXT_OOT(textId) : 0;
}
