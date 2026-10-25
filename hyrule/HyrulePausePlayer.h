#pragma once

// Link's figure on the equipment page (HyrulePausePlayer.c).

#include "PR/ultratypes.h"
#include "PR/gbi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

// Draws Link into the figure's framebuffer; call once per frame while the equipment page is drawn.
void Hyrule_DrawPausePlayer(struct PlayState* play);
// Draws the figure onto a 64x112 quad of the page.
void Hyrule_DrawPausePlayerImage(struct PlayState* play, Vtx* quad, u8 alpha);

#ifdef __cplusplus
}
#endif
