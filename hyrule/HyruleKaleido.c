// OoT's equipment page (z_kaleido_equipment.c in OoT), placed on MM's pause menu ring (docs/PAUSE.md).
// The grid is OoT's 4x4: an upgrade column, then three cells each for swords, shields, tunics and boots.
// A cell can hold several pieces (MM's swords, the Master and Great Fairy's Swords, the Deku, Hero's and MM
// Mirror shields); it shows the one worn, or else the first owned, and A wears it or the next one.

#include "global.h"
#include "HyruleKaleido.h"
#include "HyruleInventory.h"
#include "HyrulePort.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/oot/textures/icon_item_static/icon_item_static.h"
#include "assets/oot/textures/icon_item_nes_static/icon_item_nes_static.h"
#include "assets/oot/textures/item_name_static/item_name_static.h"
#include "interface/parameter_static/parameter_static.h"

static TexturePtr sEquipPageBgTextures[] = {
    // Column 0
    (TexturePtr)gPauseEquipment00Tex,
    (TexturePtr)gPauseEquipment01Tex,
    (TexturePtr)gPauseEquipment02Tex,
    (TexturePtr)gPauseEquipment03Tex,
    (TexturePtr)gPauseEquipment04Tex,
    // Column 1
    (TexturePtr)gPauseEquipment10ENGTex,
    (TexturePtr)gPauseEquipment11Tex,
    (TexturePtr)gPauseEquipment12Tex,
    (TexturePtr)gPauseEquipment13Tex,
    (TexturePtr)gPauseEquipment14Tex,
    // Column 2
    (TexturePtr)gPauseEquipment20Tex,
    (TexturePtr)gPauseEquipment21Tex,
    (TexturePtr)gPauseEquipment22Tex,
    (TexturePtr)gPauseEquipment23Tex,
    (TexturePtr)gPauseEquipment24Tex,
};

TexturePtr* Hyrule_GetEquipPageBgTextures(void) {
    return sEquipPageBgTextures;
}

TexturePtr Hyrule_GetToEquipmentTexture(void) {
    return (TexturePtr)gPauseToEquipmentENGTex;
}

#define EQUIP_ROWS 4
#define EQUIP_COLS 4
#define EQUIP_CELL_PIECES_MAX 3

// The bodies a piece fits. Deku, Goron, Zora and Fierce Deity Link wear none.
#define BODY_CHILD (1 << 0)
#define BODY_ADULT (1 << 1)

typedef struct EquipPiece {
    /* 0x0 */ u8 owned;   // EquipOwned* bit within the row's type
    /* 0x1 */ u8 value;   // EQUIP_VALUE_* worn
    /* 0x2 */ u16 item;   // for the icon and the name: MM's id for MM's pieces, else OoT's
    /* 0x4 */ u8 bodies;  // BODY_*
} EquipPiece;              // size = 0x6

typedef struct EquipCell {
    /* 0x0 */ u8 count;
    /* 0x2 */ EquipPiece pieces[EQUIP_CELL_PIECES_MAX]; // in display priority
} EquipCell;

// Rows are EQUIP_TYPE_SWORD, EQUIP_TYPE_SHIELD, EQUIP_TYPE_TUNIC and EQUIP_TYPE_BOOTS_Oot; columns 1 to 3.
static const EquipCell sEquipCells[EQUIP_ROWS][EQUIP_COLS - 1] = {
    {
        // MM's swords: only one exists at a time.
        { 3,
          { { EQUIP_OWNED_SWORD_GILDED, EQUIP_VALUE_SWORD_GILDED, ITEM_SWORD_GILDED, BODY_CHILD },
            { EQUIP_OWNED_SWORD_RAZOR, EQUIP_VALUE_SWORD_RAZOR, ITEM_SWORD_RAZOR, BODY_CHILD },
            { EQUIP_OWNED_SWORD_KOKIRI, EQUIP_VALUE_SWORD_KOKIRI, ITEM_SWORD_KOKIRI, BODY_CHILD } } },
        { 2,
          { { EQUIP_OWNED_SWORD_MASTER, EQUIP_VALUE_SWORD_MASTER_Oot, ITEM_SWORD_MASTER_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_GREAT_FAIRY, EQUIP_VALUE_SWORD_GREAT_FAIRY_Oot, ITEM_SWORD_GREAT_FAIRY,
              BODY_CHILD | BODY_ADULT } } },
        { 3,
          { { EQUIP_OWNED_SWORD_BIGGORON, EQUIP_VALUE_SWORD_BIGGORON_Oot, ITEM_SWORD_BGS_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_KNIFE, EQUIP_VALUE_SWORD_KNIFE_Oot, ITEM_SWORD_KNIFE_Oot, BODY_ADULT },
            { EQUIP_OWNED_SWORD_KNIFE_BROKEN, EQUIP_VALUE_SWORD_KNIFE_Oot, ITEM_SWORD_BROKEN_Oot, BODY_ADULT } } },
    },
    {
        { 3,
          { { EQUIP_OWNED_SHIELD_MIRROR_MM, EQUIP_VALUE_SHIELD_MIRROR, ITEM_SHIELD_MIRROR, BODY_CHILD },
            { EQUIP_OWNED_SHIELD_HERO, EQUIP_VALUE_SHIELD_HERO, ITEM_SHIELD_HERO, BODY_CHILD | BODY_ADULT },
            { EQUIP_OWNED_SHIELD_DEKU, EQUIP_VALUE_SHIELD_DEKU_Oot, ITEM_SHIELD_DEKU_Oot, BODY_CHILD } } },
        { 1, { { EQUIP_OWNED_SHIELD_HYLIAN, EQUIP_VALUE_SHIELD_HYLIAN_Oot, ITEM_SHIELD_HYLIAN_Oot,
                 BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_SHIELD_MIRROR_OOT, EQUIP_VALUE_SHIELD_MIRROR_Oot, ITEM_SHIELD_MIRROR_Oot, BODY_ADULT } } },
    },
    {
        { 1, { { EQUIP_OWNED_KOKIRI, EQUIP_VALUE_TUNIC_KOKIRI, ITEM_TUNIC_KOKIRI_Oot, BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_GORON_IRON, EQUIP_VALUE_TUNIC_GORON, ITEM_TUNIC_GORON_Oot, BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_ZORA_HOVER, EQUIP_VALUE_TUNIC_ZORA, ITEM_TUNIC_ZORA_Oot, BODY_ADULT } } },
    },
    {
        { 1, { { EQUIP_OWNED_KOKIRI, EQUIP_VALUE_BOOTS_KOKIRI_Oot, ITEM_BOOTS_KOKIRI_Oot, BODY_CHILD | BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_GORON_IRON, EQUIP_VALUE_BOOTS_IRON_Oot, ITEM_BOOTS_IRON_Oot, BODY_ADULT } } },
        { 1, { { EQUIP_OWNED_ZORA_HOVER, EQUIP_VALUE_BOOTS_HOVER_Oot, ITEM_BOOTS_HOVER_Oot, BODY_ADULT } } },
    },
};

static const u8 sEquipRowTypes[EQUIP_ROWS] = { EQUIP_TYPE_SWORD, EQUIP_TYPE_SHIELD, EQUIP_TYPE_TUNIC,
                                               EQUIP_TYPE_BOOTS_Oot };

// The upgrade column (U4): Bullet Bag, Strength, Scale, Wallet; each shows its level's item.
static const u8 sUpgradeRowTypes[EQUIP_ROWS] = { UPG_BULLET_BAG, UPG_STRENGTH, UPG_SCALE, UPG_WALLET };
static const u16 sUpgradeItems[EQUIP_ROWS][3] = {
    { ITEM_BULLET_BAG_30_Oot, ITEM_BULLET_BAG_40_Oot, ITEM_BULLET_BAG_50_Oot },
    { ITEM_BRACELET_Oot, ITEM_GAUNTLETS_SILVER_Oot, ITEM_GAUNTLETS_GOLD_Oot },
    { ITEM_SCALE_SILVER_Oot, ITEM_SCALE_GOLDEN_Oot, ITEM_NONE },
    { ITEM_WALLET_ADULT, ITEM_WALLET_GIANT, ITEM_NONE }, // MM's wallets
};

// OoT's cell corners on the page (KaleidoScope_SetVertices in OoT).
static const s16 sEquipColumnX[EQUIP_COLS] = { -114, 12, 44, 76 };
#define EQUIP_ROW_Y(row) (58 - 32 * (row))

// The cursor is drawn in its own view (KaleidoScope_DrawCursor); these map a page point to it, as MM's cursor
// tables do for its item grid.
#define PAGE_TO_CURSOR_X(x) ((x) * 0.8125f + 3.0f)
#define PAGE_TO_CURSOR_Y(y) ((y) * 0.8125f - 3.125f)

static u8 Hyrule_GetBody(void) {
    if (GET_PLAYER_FORM != PLAYER_FORM_HUMAN) {
        return 0;
    }
    return LINK_IS_ADULT_OOT ? BODY_ADULT : BODY_CHILD;
}

static s32 Hyrule_IsPieceWearable(u8 type, const EquipPiece* piece) {
    return CHECK_OWNED_EQUIP_OOT(type, piece->owned) && (piece->bodies & Hyrule_GetBody());
}

// The piece a cell shows: the one worn, else the first wearable one owned. NULL if none.
static const EquipPiece* Hyrule_GetCellPiece(s32 row, s32 col) {
    const EquipCell* cell = &sEquipCells[row][col - 1];
    u8 type = sEquipRowTypes[row];
    const EquipPiece* first = NULL;
    s32 i;

    for (i = 0; i < cell->count; i++) {
        const EquipPiece* piece = &cell->pieces[i];

        if (!Hyrule_IsPieceWearable(type, piece)) {
            continue;
        }
        if (GET_CUR_EQUIP_VALUE(type) == piece->value) {
            return piece;
        }
        if (first == NULL) {
            first = piece;
        }
    }
    return first;
}

static u16 Hyrule_GetUpgradeItem(s32 row) {
    s32 level = GET_CUR_UPG_VALUE(sUpgradeRowTypes[row]);

    return (level == 0) ? ITEM_NONE : sUpgradeItems[row][level - 1];
}

// What a cell shows, as an item id; ITEM_NONE if empty.
static u16 Hyrule_GetCellItem(s32 row, s32 col) {
    const EquipPiece* piece;

    if (col == 0) {
        return Hyrule_GetUpgradeItem(row);
    }
    piece = Hyrule_GetCellPiece(row, col);
    return (piece != NULL) ? piece->item : ITEM_NONE;
}

static TexturePtr Hyrule_GetEquipIcon(u16 item) {
    switch (item) {
        case ITEM_SWORD_MASTER_Oot:
            return (TexturePtr)gItemIconSwordMasterTex;
        case ITEM_SWORD_BGS_Oot:
            return (TexturePtr)gItemIconSwordBiggoronTex;
        case ITEM_SWORD_KNIFE_Oot:
            return (TexturePtr)gItemIconSwordBiggoronTex; // OoT shows the Giant's Knife with Biggoron's icon
        case ITEM_SWORD_BROKEN_Oot:
            return (TexturePtr)gItemIconBrokenGiantsKnifeTex;
        case ITEM_SHIELD_DEKU_Oot:
            return (TexturePtr)gItemIconShieldDekuTex;
        case ITEM_SHIELD_HYLIAN_Oot:
            return (TexturePtr)gItemIconShieldHylianTex;
        case ITEM_SHIELD_MIRROR_Oot:
            return (TexturePtr)gItemIconShieldMirrorTex;
        case ITEM_TUNIC_KOKIRI_Oot:
            return (TexturePtr)gItemIconTunicKokiriTex;
        case ITEM_TUNIC_GORON_Oot:
            return (TexturePtr)gItemIconTunicGoronTex;
        case ITEM_TUNIC_ZORA_Oot:
            return (TexturePtr)gItemIconTunicZoraTex;
        case ITEM_BOOTS_KOKIRI_Oot:
            return (TexturePtr)gItemIconBootsKokiriTex;
        case ITEM_BOOTS_IRON_Oot:
            return (TexturePtr)gItemIconBootsIronTex;
        case ITEM_BOOTS_HOVER_Oot:
            return (TexturePtr)gItemIconBootsHoverTex;
        case ITEM_BULLET_BAG_30_Oot:
            return (TexturePtr)gItemIconBulletBag30Tex;
        case ITEM_BULLET_BAG_40_Oot:
            return (TexturePtr)gItemIconBulletBag40Tex;
        case ITEM_BULLET_BAG_50_Oot:
            return (TexturePtr)gItemIconBulletBag50Tex;
        case ITEM_BRACELET_Oot:
            return (TexturePtr)gItemIconGoronsBraceletTex;
        case ITEM_GAUNTLETS_SILVER_Oot:
            return (TexturePtr)gItemIconSilverGauntletsTex;
        case ITEM_GAUNTLETS_GOLD_Oot:
            return (TexturePtr)gItemIconGoldenGauntletsTex;
        case ITEM_SCALE_SILVER_Oot:
            return (TexturePtr)gItemIconScaleSilverTex;
        case ITEM_SCALE_GOLDEN_Oot:
            return (TexturePtr)gItemIconScaleGoldenTex;
        default:
            // MM's own pieces and wallets
            return (item < ARRAY_COUNT(gItemIcons)) ? gItemIcons[item] : NULL;
    }
}

TexturePtr Hyrule_GetItemNameTexture(u16 item) {
    switch (item) {
        case ITEM_SWORD_MASTER_Oot:
            return (TexturePtr)gMasterSwordItemNameENGTex;
        case ITEM_SWORD_BGS_Oot:
            return (TexturePtr)gBiggoronsSwordItemNameENGTex;
        case ITEM_SWORD_KNIFE_Oot:
            return (TexturePtr)gGiantsKnifeItemNameENGTex;
        case ITEM_SWORD_BROKEN_Oot:
            return (TexturePtr)gBrokenGiantsKnifeItemNameENGTex;
        case ITEM_SHIELD_DEKU_Oot:
            return (TexturePtr)gDekuShieldItemNameENGTex;
        case ITEM_SHIELD_HYLIAN_Oot:
            return (TexturePtr)gHylianShieldItemNameENGTex;
        case ITEM_SHIELD_MIRROR_Oot:
            return (TexturePtr)gMirrorShieldItemNameENGTex;
        case ITEM_TUNIC_KOKIRI_Oot:
            return (TexturePtr)gKokiriTunicItemNameENGTex;
        case ITEM_TUNIC_GORON_Oot:
            return (TexturePtr)gGoronTunicItemNameENGTex;
        case ITEM_TUNIC_ZORA_Oot:
            return (TexturePtr)gZoraTunicItemNameENGTex;
        case ITEM_BOOTS_KOKIRI_Oot:
            return (TexturePtr)gKokiriBootsItemNameENGTex;
        case ITEM_BOOTS_IRON_Oot:
            return (TexturePtr)gIronBootsItemNameENGTex;
        case ITEM_BOOTS_HOVER_Oot:
            return (TexturePtr)gHoverBootsItemNameENGTex;
        case ITEM_BULLET_BAG_30_Oot:
            return (TexturePtr)gBulletBag30ItemNameENGTex;
        case ITEM_BULLET_BAG_40_Oot:
            return (TexturePtr)gBulletBag40ItemNameENGTex;
        case ITEM_BULLET_BAG_50_Oot:
            return (TexturePtr)gBulletBag50ItemNameENGTex;
        case ITEM_BRACELET_Oot:
            return (TexturePtr)gGoronsBraceletItemNameENGTex;
        case ITEM_GAUNTLETS_SILVER_Oot:
            return (TexturePtr)gSilverGauntletsItemNameENGTex;
        case ITEM_GAUNTLETS_GOLD_Oot:
            return (TexturePtr)gGoldenGauntletsItemNameENGTex;
        case ITEM_SCALE_SILVER_Oot:
            return (TexturePtr)gSilverScaleItemNameENGTex;
        case ITEM_SCALE_GOLDEN_Oot:
            return (TexturePtr)gGoldenScaleItemNameENGTex;
        default:
            return NULL;
    }
}

// The piece worn of a type, as an item id; ITEM_NONE if none. Pieces sharing a value (the Giant's Knife,
// whole or broken) are told apart by ownership.
u16 Hyrule_GetWornEquipItem(u8 type) {
    u16 found = ITEM_NONE;
    s32 row;
    s32 col;
    s32 i;

    for (row = 0; row < EQUIP_ROWS; row++) {
        if (sEquipRowTypes[row] != type) {
            continue;
        }
        for (col = 1; col < EQUIP_COLS; col++) {
            const EquipCell* cell = &sEquipCells[row][col - 1];

            for (i = 0; i < cell->count; i++) {
                if (GET_CUR_EQUIP_VALUE(type) != cell->pieces[i].value) {
                    continue;
                }
                if (CHECK_OWNED_EQUIP_OOT(type, cell->pieces[i].owned)) {
                    return cell->pieces[i].item;
                }
                if (found == ITEM_NONE) {
                    found = cell->pieces[i].item;
                }
            }
        }
    }
    return found;
}

TexturePtr Hyrule_GetWornEquipIcon(u8 type) {
    u16 item = Hyrule_GetWornEquipItem(type);

    return (item == ITEM_NONE) ? NULL : Hyrule_GetEquipIcon(item);
}

static void Hyrule_SetQuad(Vtx* vtx, s16 x, s16 y, s16 size, u8 alpha) {
    s32 i;

    for (i = 0; i < 4; i++) {
        vtx[i].v.ob[0] = x + ((i & 1) ? size : 0);
        vtx[i].v.ob[1] = y - ((i & 2) ? size : 0);
        vtx[i].v.ob[2] = 0;
        vtx[i].v.flag = 0;
        vtx[i].v.tc[0] = (i & 1) ? 32 << 5 : 0;
        vtx[i].v.tc[1] = (i & 2) ? 32 << 5 : 0;
        vtx[i].v.cn[0] = vtx[i].v.cn[1] = vtx[i].v.cn[2] = 255;
        vtx[i].v.cn[3] = alpha;
    }
}

// equipVtx: a 28x28 icon quad per cell, then a 32x32 outline quad per row for the piece worn.
#define EQUIP_VTX_ICON(row, col) (((row) * EQUIP_COLS + (col)) * 4)
#define EQUIP_VTX_OUTLINE(row) ((EQUIP_ROWS * EQUIP_COLS + (row)) * 4)
#define EQUIP_VTX_COUNT ((EQUIP_ROWS * EQUIP_COLS + EQUIP_ROWS) * 4)

void Hyrule_KaleidoSetEquipmentVertices(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 row;
    s32 col;

    pauseCtx->equipVtx = GRAPH_ALLOC(play->state.gfxCtx, EQUIP_VTX_COUNT * sizeof(Vtx));

    for (row = 0; row < EQUIP_ROWS; row++) {
        s16 y = EQUIP_ROW_Y(row) + pauseCtx->offsetY;

        for (col = 0; col < EQUIP_COLS; col++) {
            Hyrule_SetQuad(&pauseCtx->equipVtx[EQUIP_VTX_ICON(row, col)], sEquipColumnX[col] + 2, y - 2, 28, 255);
        }
        // Moved onto the worn piece's cell when drawn.
        Hyrule_SetQuad(&pauseCtx->equipVtx[EQUIP_VTX_OUTLINE(row)], 0, y, 32, 255);
    }
}

void Hyrule_KaleidoDrawEquipment(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 row;
    s32 col;

    Hyrule_SyncEquipmentOwnership();

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    // Outline the piece worn of each type, as OoT does.
    for (row = 0; row < EQUIP_ROWS; row++) {
        for (col = 1; col < EQUIP_COLS; col++) {
            const EquipPiece* piece = Hyrule_GetCellPiece(row, col);

            if ((piece != NULL) && (GET_CUR_EQUIP_VALUE(sEquipRowTypes[row]) == piece->value)) {
                Vtx* outline = &pauseCtx->equipVtx[EQUIP_VTX_OUTLINE(row)];
                s32 i;

                for (i = 0; i < 4; i++) {
                    outline[i].v.ob[0] = sEquipColumnX[col] + ((i & 1) ? 32 : 0);
                }
                gSPVertex(POLY_OPA_DISP++, outline, 4, 0);
                POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gEquippedItemOutlineTex, 32, 32, 0);
                break;
            }
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    for (row = 0; row < EQUIP_ROWS; row++) {
        for (col = 0; col < EQUIP_COLS; col++) {
            TexturePtr icon;
            u16 item = Hyrule_GetCellItem(row, col);

            if ((item == ITEM_NONE) || ((icon = Hyrule_GetEquipIcon(item)) == NULL)) {
                continue;
            }
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->equipVtx[EQUIP_VTX_ICON(row, col)], 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, icon, 32, 32, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Hyrule_KaleidoSetEquipCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 col = pauseCtx->cursorXIndex[PAUSE_EQUIP];
    s16 row = pauseCtx->cursorYIndex[PAUSE_EQUIP];

    pauseCtx->cursorX = PAGE_TO_CURSOR_X(sEquipColumnX[col] + 16);
    pauseCtx->cursorY = PAGE_TO_CURSOR_Y(EQUIP_ROW_Y(row) - 16);
    pauseCtx->cursorWidth = 15.0f;
    pauseCtx->cursorHeight = 15.0f;
}

static void Hyrule_SetEquipCursorCell(PauseContext* pauseCtx, s16 row, s16 col) {
    pauseCtx->cursorXIndex[PAUSE_EQUIP] = col;
    pauseCtx->cursorYIndex[PAUSE_EQUIP] = row;
    pauseCtx->cursorPoint[PAUSE_EQUIP] = row * EQUIP_COLS + col;
}

// Enters the page from one of its sides: the first filled cell from that side, column by column.
static s32 Hyrule_EnterEquipPage(PauseContext* pauseCtx, s32 fromTheLeft) {
    s32 i;
    s32 row;

    for (i = 0; i < EQUIP_COLS; i++) {
        s32 col = fromTheLeft ? i : (EQUIP_COLS - 1 - i);

        for (row = 0; row < EQUIP_ROWS; row++) {
            if (Hyrule_GetCellItem(row, col) != ITEM_NONE) {
                Hyrule_SetEquipCursorCell(pauseCtx, row, col);
                return true;
            }
        }
    }
    return false;
}

// Moves the cursor one filled cell along a row or column; returns false at the edge.
static s32 Hyrule_MoveEquipCursor(PauseContext* pauseCtx, s32 dRow, s32 dCol) {
    s32 row = pauseCtx->cursorYIndex[PAUSE_EQUIP] + dRow;
    s32 col = pauseCtx->cursorXIndex[PAUSE_EQUIP] + dCol;

    while ((row >= 0) && (row < EQUIP_ROWS) && (col >= 0) && (col < EQUIP_COLS)) {
        if (Hyrule_GetCellItem(row, col) != ITEM_NONE) {
            Hyrule_SetEquipCursorCell(pauseCtx, row, col);
            return true;
        }
        row += dRow;
        col += dCol;
    }
    return false;
}

static void Hyrule_WearPiece(PlayState* play, u8 type, const EquipPiece* piece) {
    SET_EQUIP_VALUE(type, piece->value);

    if (type == EQUIP_TYPE_SWORD) {
        // MM keeps the sword on B (Item_Give). Only human Link reaches here.
        CUR_FORM_EQUIP(EQUIP_SLOT_B) = piece->item;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
    }
    Player_SetEquipmentData(play, GET_PLAYER(play));
}

// A: wears the piece shown, or, if it is already worn, the cell's next wearable piece (U5).
static void Hyrule_EquipCell(PlayState* play, s32 row, s32 col) {
    const EquipCell* cell = &sEquipCells[row][col - 1];
    const EquipPiece* shown = Hyrule_GetCellPiece(row, col);
    u8 type = sEquipRowTypes[row];
    const EquipPiece* next = shown;

    if (GET_CUR_EQUIP_VALUE(type) == shown->value) {
        s32 index = shown - cell->pieces;
        s32 i;

        for (i = 1; i < cell->count; i++) {
            const EquipPiece* piece = &cell->pieces[(index + i) % cell->count];

            if (Hyrule_IsPieceWearable(type, piece)) {
                next = piece;
                break;
            }
        }
    }

    Hyrule_WearPiece(play, type, next);
    Audio_PlaySfx(NA_SE_SY_DECIDE);
}

void Hyrule_KaleidoUpdateEquipCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    Input* input = CONTROLLER1(&play->state);
    s16 oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
    s16 row;
    s16 col;

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;

    if ((pauseCtx->state != PAUSE_STATE_MAIN) || (pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE) ||
        (pauseCtx->pageIndex != PAUSE_EQUIP)) {
        return;
    }

    if (pauseCtx->cursorSpecialPos == 0) {
        // The cell's contents can change (Deku Link, a piece lost); leave for a filled one, or the side.
        if (Hyrule_GetCellItem(pauseCtx->cursorYIndex[PAUSE_EQUIP], pauseCtx->cursorXIndex[PAUSE_EQUIP]) ==
            ITEM_NONE) {
            if (!Hyrule_EnterEquipPage(pauseCtx, true)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            }
        } else if (pauseCtx->stickAdjX < -30) {
            if (!Hyrule_MoveEquipCursor(pauseCtx, 0, -1)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            }
        } else if (pauseCtx->stickAdjX > 30) {
            if (!Hyrule_MoveEquipCursor(pauseCtx, 0, 1)) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            }
        } else if (pauseCtx->stickAdjY > 30) {
            Hyrule_MoveEquipCursor(pauseCtx, -1, 0);
        } else if (pauseCtx->stickAdjY < -30) {
            Hyrule_MoveEquipCursor(pauseCtx, 1, 0);
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) && (pauseCtx->stickAdjX > 30)) {
        if (Hyrule_EnterEquipPage(pauseCtx, true)) {
            KaleidoScope_MoveCursorFromSpecialPos(play); // plays the cursor sound
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT) && (pauseCtx->stickAdjX < -30)) {
        if (Hyrule_EnterEquipPage(pauseCtx, false)) {
            KaleidoScope_MoveCursorFromSpecialPos(play);
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_EQUIP];
        }
    }

    if (pauseCtx->cursorSpecialPos != 0) {
        pauseCtx->cursorItem[PAUSE_EQUIP] = PAUSE_ITEM_NONE;
        return;
    }

    row = pauseCtx->cursorYIndex[PAUSE_EQUIP];
    col = pauseCtx->cursorXIndex[PAUSE_EQUIP];
    pauseCtx->cursorItem[PAUSE_EQUIP] = Hyrule_GetCellItem(row, col);
    pauseCtx->cursorSlot[PAUSE_EQUIP] = pauseCtx->cursorPoint[PAUSE_EQUIP];

    if (pauseCtx->cursorPoint[PAUSE_EQUIP] != oldCursorPoint) {
        pauseCtx->cursorShrinkRate = 4.0f;
        Audio_PlaySfx(NA_SE_SY_CURSOR);
    }

    if (col != 0) {
        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_YELLOW;
        Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);

        if (CHECK_BTN_ALL(input->press.button, BTN_A) && (pauseCtx->cursorItem[PAUSE_EQUIP] != ITEM_NONE)) {
            Hyrule_EquipCell(play, row, col);
        }
    } else {
        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
    }
}
