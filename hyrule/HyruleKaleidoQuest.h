#pragma once

// OoT's quest status page (HyruleKaleidoQuest.c), for HyruleKaleido.c's page dispatch.

#include "PR/ultratypes.h"

struct PlayState;

TexturePtr* Hyrule_GetQuestPageBgTextures(void);
TexturePtr Hyrule_GetToQuestStatusTexture(void);
void Hyrule_KaleidoSetQuestVertices(struct PlayState* play);
void Hyrule_KaleidoDrawQuest(struct PlayState* play);
void Hyrule_KaleidoSetQuestCursor(struct PlayState* play);
void Hyrule_KaleidoUpdateQuestCursor(struct PlayState* play);
