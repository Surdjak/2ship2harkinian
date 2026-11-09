#pragma once

// OoT's place-name title cards. See HyruleTitleCard.c.

#include "PR/ultratypes.h"

struct PlayState;

// Shows the place-name card of the OoT scene Link is in, if it has one (OoT's TitleCard_InitPlaceName).
void Hyrule_InitPlaceNameOot(struct PlayState* play);

// Whether an entrance into the OoT scene Link is in shows its place name (OoT's Player_Init).
s32 Hyrule_ShowsPlaceNameOot(struct PlayState* play);
