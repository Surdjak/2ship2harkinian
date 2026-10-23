// OoT's quest status page (z_kaleido_collect.c in OoT), Hyrule's quest page on MM's pause menu ring
// (docs/PAUSE.md): medallions, spiritual stones, OoT's songs, the Stone of Agony, Gerudo's Card, the Gold
// Skulltula count and the heart pieces. Songs and heart pieces both games have read the shared save
// (Hyrule_CheckQuestItemOot). OoT's song playback on A is not ported yet.

#include "global.h"
#include "HyruleKaleidoQuest.h"
#include "HyruleKaleido.h"
#include "HyruleInventory.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/oot/textures/icon_item_static/icon_item_static.h"
#include "assets/oot/textures/icon_item_nes_static/icon_item_nes_static.h"
#include "assets/oot/textures/icon_item_24_static/icon_item_24_static.h"
#include "interface/parameter_static/parameter_static.h"

static TexturePtr sQuestPageBgTextures[] = {
    // Column 0
    (TexturePtr)gPauseQuestStatus00ENGTex,
    (TexturePtr)gPauseQuestStatus01Tex,
    (TexturePtr)gPauseQuestStatus02Tex,
    (TexturePtr)gPauseQuestStatus03Tex,
    (TexturePtr)gPauseQuestStatus04Tex,
    // Column 1
    (TexturePtr)gPauseQuestStatus10ENGTex,
    (TexturePtr)gPauseQuestStatus11Tex,
    (TexturePtr)gPauseQuestStatus12Tex,
    (TexturePtr)gPauseQuestStatus13Tex,
    (TexturePtr)gPauseQuestStatus14Tex,
    // Column 2
    (TexturePtr)gPauseQuestStatus20ENGTex,
    (TexturePtr)gPauseQuestStatus21Tex,
    (TexturePtr)gPauseQuestStatus22Tex,
    (TexturePtr)gPauseQuestStatus23Tex,
    (TexturePtr)gPauseQuestStatus24Tex,
};

TexturePtr* Hyrule_GetQuestPageBgTextures(void) {
    return sQuestPageBgTextures;
}

TexturePtr Hyrule_GetToQuestStatusTexture(void) {
    return (TexturePtr)gPauseToQuestStatusENGTex;
}

// The page's points, as in OoT: QUEST_*_Oot for 0 to 23 (medallions, songs, stones, Agony, Gerudo's Card,
// the Gold Skulltula token), then the heart pieces.
#define QUEST_POINT_HEART_PIECES 24
#define QUEST_POINT_COUNT 25
// After the points, the Gold Skulltula count: three digits in shadow, then three in front.
#define QUEST_DIGIT_COUNT 6
#define QUEST_VTX_DIGITS (QUEST_POINT_COUNT * 4)
#define QUEST_VTX_COUNT ((QUEST_POINT_COUNT + QUEST_DIGIT_COUNT) * 4)

// Each point's top-left corner and size (KaleidoScope_InitVertices in OoT).
static const s16 sQuestPointX[QUEST_POINT_COUNT] = {
    74,   74,  46,  18,  18,  46,                 // medallions
    -108, -90, -72, -54, -36, -18,                // warp songs
    -108, -90, -72, -54, -36, -18,                // Zelda's Lullaby to the Song of Storms
    20,   46,  72,                                // stones
    -110, -86, -110,                              // Agony, Gerudo's Card, Gold Skulltula
    -54,                                          // heart pieces
};
static const s16 sQuestPointY[QUEST_POINT_COUNT] = {
    38,  6,   -12, 6,   38,  56,  -20, -20, -20, -20, -20, -20, 2,
    2,   2,   2,   2,   2,   -46, -46, -46, 58,  58,  34,  58,
};
static const s16 sQuestDigitX[QUEST_DIGIT_COUNT] = { -88, -81, -72, -90, -83, -74 };
static const s16 sQuestDigitY[QUEST_DIGIT_COUNT] = { 34, 34, 34, 36, 36, 36 };

#define QUEST_POINT_SIZE(point) (((point) == QUEST_POINT_HEART_PIECES) ? 48 : 24)
#define QUEST_POINT_IS_SONG(point) (((point) >= QUEST_SONG_MINUET_Oot) && ((point) <= QUEST_SONG_STORMS_Oot))

// Cursor links between points, as in OoT: up, down, left, right. LINK_NONE stays; LINK_PAGE_* leaves the
// page on that side.
#define LINK_NONE -1
#define LINK_PAGE_RIGHT -2
#define LINK_PAGE_LEFT -3
static const s8 sQuestPointLinks[QUEST_POINT_COUNT][4] = {
    { 5, 1, 5, LINK_PAGE_RIGHT },        { 0, 2, 2, LINK_PAGE_RIGHT },  { LINK_NONE, 19, 3, 1 },
    { 4, 2, 17, 2 },                     { 5, 3, 24, 5 },               { LINK_NONE, LINK_NONE, 4, 0 },
    { 12, LINK_NONE, LINK_PAGE_LEFT, 7 }, { 13, LINK_NONE, 6, 8 },       { 14, LINK_NONE, 7, 9 },
    { 15, LINK_NONE, 8, 10 },            { 16, LINK_NONE, 9, 11 },      { 17, LINK_NONE, 10, 18 },
    { 23, 6, LINK_PAGE_LEFT, 13 },       { 23, 7, 12, 14 },             { 23, 8, 13, 15 },
    { 24, 9, 14, 16 },                   { 24, 10, 15, 17 },            { 24, 11, 16, 3 },
    { 2, LINK_NONE, 11, 19 },            { 2, LINK_NONE, 18, 20 },      { 2, LINK_NONE, 19, LINK_PAGE_RIGHT },
    { LINK_NONE, 23, LINK_PAGE_LEFT, 22 }, { LINK_NONE, 23, 21, 24 },   { 21, 12, LINK_PAGE_LEFT, 24 },
    { LINK_NONE, 16, 22, 4 },
};

// Medallion glows and song note colours (OoT's quest page).
static const u8 sMedallionEnvColors[6][3] = {
    { 0, 60, 0 }, { 90, 0, 0 }, { 0, 40, 110 }, { 80, 40, 0 }, { 70, 0, 90 }, { 90, 90, 0 },
};
static const u8 sSongNoteColors[12][3] = {
    { 150, 255, 100 }, { 255, 80, 40 },   { 100, 150, 255 }, { 255, 160, 0 },  { 255, 100, 255 }, { 255, 240, 100 },
    { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 },
};

static TexturePtr sQuestIcons[] = {
    (TexturePtr)gQuestIconMedallionForestTex, (TexturePtr)gQuestIconMedallionFireTex,
    (TexturePtr)gQuestIconMedallionWaterTex,  (TexturePtr)gQuestIconMedallionSpiritTex,
    (TexturePtr)gQuestIconMedallionShadowTex, (TexturePtr)gQuestIconMedallionLightTex,
};
static TexturePtr sQuestLowerIcons[] = {
    (TexturePtr)gQuestIconKokiriEmeraldTex,   (TexturePtr)gQuestIconGoronRubyTex,
    (TexturePtr)gQuestIconZoraSapphireTex,    (TexturePtr)gQuestIconStoneOfAgonyTex,
    (TexturePtr)gQuestIconGerudosCardTex,     (TexturePtr)gQuestIconGoldSkulltulaTex,
};
static TexturePtr sHeartPieceIcons[] = {
    (TexturePtr)gHeartPieceIcon1Tex,
    (TexturePtr)gHeartPieceIcon2Tex,
    (TexturePtr)gHeartPieceIcon3Tex,
};
static TexturePtr sDigitTextures[] = {
    (TexturePtr)gCounterDigit0Tex, (TexturePtr)gCounterDigit1Tex, (TexturePtr)gCounterDigit2Tex,
    (TexturePtr)gCounterDigit3Tex, (TexturePtr)gCounterDigit4Tex, (TexturePtr)gCounterDigit5Tex,
    (TexturePtr)gCounterDigit6Tex, (TexturePtr)gCounterDigit7Tex, (TexturePtr)gCounterDigit8Tex,
    (TexturePtr)gCounterDigit9Tex,
};

static s32 Hyrule_GetHeartPieceCount(void) {
    return GET_QUEST_HEART_PIECE_COUNT;
}

static s32 Hyrule_IsQuestPointOwned(s32 point) {
    if (point == QUEST_POINT_HEART_PIECES) {
        return Hyrule_GetHeartPieceCount() != 0;
    }
    return Hyrule_CheckQuestItemOot(point);
}

// The item a point names (OoT's ids follow the quest bits); ITEM_NONE if not owned.
static u16 Hyrule_GetQuestPointItem(s32 point) {
    if (!Hyrule_IsQuestPointOwned(point)) {
        return PAUSE_ITEM_NONE;
    }
    if (point == QUEST_POINT_HEART_PIECES) {
        return ITEM_HEART_PIECE_Oot;
    }
    if (point <= QUEST_MEDALLION_LIGHT_Oot) {
        return ITEM_MEDALLION_FOREST_Oot + point;
    }
    if (point <= QUEST_SONG_STORMS_Oot) {
        return ITEM_SONG_MINUET_Oot + (point - QUEST_SONG_MINUET_Oot);
    }
    return ITEM_KOKIRI_EMERALD_Oot + (point - QUEST_KOKIRI_EMERALD_Oot);
}

static void Hyrule_SetQuestQuad(Vtx* vtx, s16 x, s16 y, s16 width, s16 height, s16 texWidth, s16 texHeight) {
    s32 i;

    for (i = 0; i < 4; i++) {
        vtx[i].v.ob[0] = x + ((i & 1) ? width : 0);
        vtx[i].v.ob[1] = y - ((i & 2) ? height : 0);
        vtx[i].v.ob[2] = 0;
        vtx[i].v.flag = 0;
        vtx[i].v.tc[0] = (i & 1) ? texWidth << 5 : 0;
        vtx[i].v.tc[1] = (i & 2) ? texHeight << 5 : 0;
        vtx[i].v.cn[0] = vtx[i].v.cn[1] = vtx[i].v.cn[2] = vtx[i].v.cn[3] = 255;
    }
}

void Hyrule_KaleidoSetQuestVertices(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 point;
    s32 i;

    pauseCtx->questOotVtx = GRAPH_ALLOC(play->state.gfxCtx, QUEST_VTX_COUNT * sizeof(Vtx));

    for (point = 0; point < QUEST_POINT_COUNT; point++) {
        s16 x = sQuestPointX[point];
        s16 y = sQuestPointY[point] + pauseCtx->offsetY;
        s16 size = QUEST_POINT_SIZE(point);

        if (point <= QUEST_MEDALLION_LIGHT_Oot) {
            Hyrule_SetQuestQuad(&pauseCtx->questOotVtx[point * 4], x, y, size, size, size, size);
        } else if (QUEST_POINT_IS_SONG(point)) {
            // A 16x24 note, inset as OoT does
            Hyrule_SetQuestQuad(&pauseCtx->questOotVtx[point * 4], x + 2, y - 2, 12, 20, 16, 24);
        } else {
            Hyrule_SetQuestQuad(&pauseCtx->questOotVtx[point * 4], x + 2, y - 2, size - 4, size - 4, size, size);
        }
    }
    for (i = 0; i < QUEST_DIGIT_COUNT; i++) {
        Hyrule_SetQuestQuad(&pauseCtx->questOotVtx[QUEST_VTX_DIGITS + i * 4], sQuestDigitX[i],
                            sQuestDigitY[i] + pauseCtx->offsetY - 6, 8, 16, 8, 16);
    }
}

static void Hyrule_DrawQuestQuadRGBA32(PlayState* play, s32 point, TexturePtr texture) {
    PauseContext* pauseCtx = &play->pauseCtx;

    OPEN_DISPS(play->state.gfxCtx);
    gSPVertex(POLY_OPA_DISP++, &pauseCtx->questOotVtx[point * 4], 4, 0);
    CLOSE_DISPS(play->state.gfxCtx);
    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, texture, 24, 24, 0);
}

void Hyrule_KaleidoDrawQuest(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 point;
    s32 i;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

    // Medallions, glowing in their colours
    for (point = QUEST_MEDALLION_FOREST_Oot; point <= QUEST_MEDALLION_LIGHT_Oot; point++) {
        if (Hyrule_IsQuestPointOwned(point)) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, sMedallionEnvColors[point][0], sMedallionEnvColors[point][1],
                           sMedallionEnvColors[point][2], 0);
            Hyrule_DrawQuestQuadRGBA32(play, point, sQuestIcons[point]);
        }
    }

    // Songs: notes in OoT's colours
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gSongNoteTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 24, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (point = QUEST_SONG_MINUET_Oot; point <= QUEST_SONG_STORMS_Oot; point++) {
        if (Hyrule_IsQuestPointOwned(point)) {
            const u8* color = sSongNoteColors[point - QUEST_SONG_MINUET_Oot];

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, color[0], color[1], color[2], pauseCtx->alpha);
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->questOotVtx[point * 4], 4, 0);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    // Stones, Agony, Gerudo's Card, the Gold Skulltula token
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    for (point = QUEST_KOKIRI_EMERALD_Oot; point <= QUEST_SKULL_TOKEN_Oot; point++) {
        if (Hyrule_IsQuestPointOwned(point)) {
            Hyrule_DrawQuestQuadRGBA32(play, point, sQuestLowerIcons[point - QUEST_KOKIRI_EMERALD_Oot]);
        }
    }

    // Heart pieces, shared with Termina
    if (Hyrule_GetHeartPieceCount() != 0) {
        s32 count = CLAMP(Hyrule_GetHeartPieceCount(), 1, ARRAY_COUNT(sHeartPieceIcons));

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 0, 0, pauseCtx->alpha);
        gSPVertex(POLY_OPA_DISP++, &pauseCtx->questOotVtx[QUEST_POINT_HEART_PIECES * 4], 4, 0);
        POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, sHeartPieceIcons[count - 1], 48, 48, 0);
    }

    // The Gold Skulltula count, in shadow then in front (red at 100)
    if (Hyrule_IsQuestPointOwned(QUEST_SKULL_TOKEN_Oot)) {
        s32 tokens = gSaveContext.save.saveInfo.inventoryOot.gsTokens;
        s32 digits[3] = { tokens / 100, (tokens / 10) % 10, tokens % 10 };

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        for (i = 0; i < QUEST_DIGIT_COUNT; i++) {
            s32 digit = i % 3;

            if (i == 0) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, pauseCtx->alpha);
            } else if (i == 3) {
                if (tokens >= 100) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 200, 50, 50, pauseCtx->alpha);
                } else {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                }
            }
            if ((digit < 2) && (digits[0] == 0) && ((digit == 0) || (digits[1] == 0))) {
                continue; // no leading zeros
            }
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->questOotVtx[QUEST_VTX_DIGITS + i * 4], 4, 0);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sDigitTextures[digits[digit]], G_IM_FMT_I, G_IM_SIZ_8b, 8, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// The cursor is drawn in its own view (KaleidoScope_DrawCursor); see HyruleKaleido.c.
#define PAGE_TO_CURSOR_X(x) ((x) * 0.8125f + 3.0f)
#define PAGE_TO_CURSOR_Y(y) ((y) * 0.8125f - 3.125f)

void Hyrule_KaleidoSetQuestCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 point = pauseCtx->cursorPoint[PAUSE_QUEST_OOT];
    s16 size = QUEST_POINT_SIZE(point);
    s16 width = QUEST_POINT_IS_SONG(point) ? 16 : size;

    pauseCtx->cursorX = PAGE_TO_CURSOR_X(sQuestPointX[point] + width / 2);
    pauseCtx->cursorY = PAGE_TO_CURSOR_Y(sQuestPointY[point] - size / 2);
    if (point == QUEST_POINT_HEART_PIECES) {
        pauseCtx->cursorWidth = pauseCtx->cursorHeight = 25.0f;
    } else if (QUEST_POINT_IS_SONG(point)) {
        pauseCtx->cursorWidth = pauseCtx->cursorHeight = 12.0f;
    } else {
        pauseCtx->cursorWidth = pauseCtx->cursorHeight = 17.0f;
    }
}

static void Hyrule_SetQuestCursorPoint(PauseContext* pauseCtx, s16 point) {
    pauseCtx->cursorPoint[PAUSE_QUEST_OOT] = point;
    pauseCtx->cursorSlot[PAUSE_QUEST_OOT] = point;
}

void Hyrule_KaleidoUpdateQuestCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 oldPoint = pauseCtx->cursorPoint[PAUSE_QUEST_OOT];

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;

    if ((pauseCtx->state != PAUSE_STATE_MAIN) || (pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE) ||
        (pauseCtx->pageIndex != PAUSE_QUEST_OOT)) {
        return;
    }

    if (pauseCtx->cursorSpecialPos == 0) {
        s32 direction = -1;
        s32 link;

        if (pauseCtx->stickAdjY > 30) {
            direction = 0;
        } else if (pauseCtx->stickAdjY < -30) {
            direction = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            direction = 2;
        } else if (pauseCtx->stickAdjX > 30) {
            direction = 3;
        }
        if (direction >= 0) {
            link = sQuestPointLinks[oldPoint][direction];
            if (link == LINK_PAGE_LEFT) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            } else if (link == LINK_PAGE_RIGHT) {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            } else if (link != LINK_NONE) {
                Hyrule_SetQuestCursorPoint(pauseCtx, link);
                pauseCtx->cursorShrinkRate = 4.0f;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        }
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) && (pauseCtx->stickAdjX > 30)) {
        // OoT enters on the Stone of Agony from the left, on the Forest Medallion from the right.
        Hyrule_SetQuestCursorPoint(pauseCtx, QUEST_STONE_OF_AGONY_Oot);
        KaleidoScope_MoveCursorFromSpecialPos(play);
    } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT) && (pauseCtx->stickAdjX < -30)) {
        Hyrule_SetQuestCursorPoint(pauseCtx, QUEST_MEDALLION_FOREST_Oot);
        KaleidoScope_MoveCursorFromSpecialPos(play);
    }

    if (pauseCtx->cursorSpecialPos != 0) {
        pauseCtx->cursorItem[PAUSE_QUEST_OOT] = PAUSE_ITEM_NONE;
        return;
    }
    pauseCtx->cursorItem[PAUSE_QUEST_OOT] = Hyrule_GetQuestPointItem(pauseCtx->cursorPoint[PAUSE_QUEST_OOT]);
    Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
}
