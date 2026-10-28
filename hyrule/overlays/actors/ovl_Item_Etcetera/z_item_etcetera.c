/*
 * File: z_item_etcetera.c
 * Overlay: ovl_Item_Etcetera
 * Description: Collectible Items
 */

#include "z_item_etcetera.h"
#include "HyruleFlags.h"
#include "HyrulePort.h"
#include "HyruleActor.h"
#include <assert.h>

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

static void ItemEtcetera_Init(Actor* thisx, PlayState* play);
static void ItemEtcetera_Destroy(Actor* thisx, PlayState* play);
static void ItemEtcetera_Update(Actor* thisx, PlayState* play);
static void ItemEtcetera_DrawThroughLens(Actor* thisx, PlayState* play);
static void ItemEtcetera_Draw(Actor* thisx, PlayState* play);

static void ItemEtcetera_WaitForObject(ItemEtcetera* this, PlayState* play);
static void func_80B85824(ItemEtcetera* this, PlayState* play);
static void func_80B858B4(ItemEtcetera* this, PlayState* play);
static void ItemEtcetera_SpawnSparkles(ItemEtcetera* this, PlayState* play);
static void ItemEtcetera_MoveFireArrowDown(ItemEtcetera* this, PlayState* play);
static void func_80B85B28(ItemEtcetera* this, PlayState* play);
static void ItemEtcetera_UpdateFireArrow(ItemEtcetera* this, PlayState* play);

ActorProfile Item_Etcetera_Oot_Profile = {
    /**/ ACTOR_ITEM_ETCETERA_Oot,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ItemEtcetera),
    /**/ ItemEtcetera_Init,
    /**/ ItemEtcetera_Destroy,
    /**/ ItemEtcetera_Update,
    /**/ NULL,
};

static s16 sObjectIds[] = {
    OBJECT_GI_BOTTLE_Oot, OBJECT_GI_BOTTLE_LETTER_Oot, OBJECT_GI_SHIELD_2_Oot, OBJECT_GI_ARROWCASE_Oot, OBJECT_GI_SCALE_Oot,
    OBJECT_GI_SCALE_Oot,  OBJECT_GI_KEY_Oot,           OBJECT_GI_M_ARROW_Oot,  OBJECT_GI_RUPY_Oot,      OBJECT_GI_RUPY_Oot,
    OBJECT_GI_RUPY_Oot,   OBJECT_GI_RUPY_Oot,          OBJECT_GI_HEARTS_Oot,   OBJECT_GI_KEY_Oot,
};

// Indexes passed to the item table in z_draw.c
static s16 sDrawItemIndexes[] = {
    GID_BOTTLE_Oot,       GID_LETTER_RUTO_Oot,  GID_SHIELD_HYLIAN_Oot, GID_QUIVER_40_Oot,   GID_SCALE_SILVER_Oot,
    GID_SCALE_GOLDEN_Oot, GID_KEY_SMALL_Oot,    GID_ARROW_FIRE_Oot,    GID_RUPEE_GREEN_Oot, GID_RUPEE_BLUE_Oot,
    GID_RUPEE_RED_Oot,    GID_RUPEE_PURPLE_Oot, GID_HEART_PIECE_Oot,   GID_KEY_SMALL_Oot,
};

static s16 sGetItemIds[] = {
    GI_BOTTLE_Oot,     GI_LETTER_RUTO_Oot, GI_SHIELD_HYLIAN_Oot, GI_QUIVER_40_Oot, GI_SCALE_SILVER_Oot, GI_SCALE_GOLDEN_Oot, GI_KEY_SMALL_Oot,
    GI_ARROW_FIRE_Oot, GI_NONE,        GI_NONE,          GI_NONE,      GI_NONE,         GI_NONE,         GI_NONE,
};

static void ItemEtcetera_SetupAction(ItemEtcetera* this, ItemEtceteraActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

static void ItemEtcetera_Init(Actor* thisx, PlayState* play) {
    ItemEtcetera* this = (ItemEtcetera*)thisx;
    s32 pad;
    s32 type;
    s32 objectSlot;

    type = this->actor.params & 0xFF;
    objectSlot = Object_GetSlot(&play->objectCtx, sObjectIds[type]);
    if (objectSlot < 0) {
        assert(objectSlot < 0);
    } else {
        this->objectSlot = objectSlot;
    }
    this->giDrawId = sDrawItemIndexes[type];
    this->getItemId = sGetItemIds[type];
    this->futureActionFunc = func_80B85824;
    this->drawFunc = ItemEtcetera_Draw;
    Actor_SetScale(&this->actor, 0.25f);
    ItemEtcetera_SetupAction(this, ItemEtcetera_WaitForObject);
    switch (type) {
        case ITEM_ETC_LETTER_Oot:
            Actor_SetScale(&this->actor, 0.5f);
            this->futureActionFunc = func_80B858B4;
            if (GET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_RUTOS_LETTER_Oot)) {
                Actor_Kill(&this->actor);
            }
            break;
        case ITEM_ETC_ARROW_FIRE_Oot:
            this->futureActionFunc = ItemEtcetera_UpdateFireArrow;
            Actor_SetScale(&this->actor, 0.5f);
            this->actor.draw = NULL;
            this->actor.shape.yOffset = 50.0f;
            break;
        case ITEM_ETC_RUPEE_GREEN_CHEST_GAME_Oot:
        case ITEM_ETC_RUPEE_BLUE_CHEST_GAME_Oot:
        case ITEM_ETC_RUPEE_RED_CHEST_GAME_Oot:
        case ITEM_ETC_RUPEE_PURPLE_CHEST_GAME_Oot:
        case ITEM_ETC_HEART_PIECE_CHEST_GAME_Oot:
        case ITEM_ETC_KEY_SMALL_CHEST_GAME_Oot:
            Actor_SetScale(&this->actor, 0.5f);
            this->futureActionFunc = func_80B85B28;
            this->drawFunc = ItemEtcetera_DrawThroughLens;
            this->actor.world.pos.y += 15.0f;
            break;
    }
}

static void ItemEtcetera_Destroy(Actor* thisx, PlayState* play) {
}

static void ItemEtcetera_WaitForObject(ItemEtcetera* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->objectSlot)) {
        this->actor.objectSlot = this->objectSlot;
        this->actor.draw = this->drawFunc;
        this->actionFunc = this->futureActionFunc;
    }
}

static void func_80B85824(ItemEtcetera* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play)) {
        if ((this->actor.params & 0xFF) == 1) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_RUTOS_LETTER_Oot);
            Flags_SetSwitch(play, 0xB);
        }
        Actor_Kill(&this->actor);
    } else {
        Actor_OfferGetItem(&this->actor, play, this->getItemId, 30.0f, 50.0f);
    }
}

static void func_80B858B4(ItemEtcetera* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play)) {
        if ((this->actor.params & 0xFF) == 1) {
            SET_EVENTCHKINF_OOT(EVENTCHKINF_OBTAINED_RUTOS_LETTER_Oot);
            Flags_SetSwitch(play, 0xB);
        }

        Actor_Kill(&this->actor);
    } else {
        Actor_OfferGetItem(&this->actor, play, this->getItemId, 30.0f, 50.0f);

        if ((play->gameplayFrames & 0xD) == 0) {
            EffectSsBubble_Spawn(play, &this->actor.world.pos, 0.0f, 0.0f, 10.0f, 0.13f);
        }
    }
}

static void ItemEtcetera_SpawnSparkles(ItemEtcetera* this, PlayState* play) {
    static Vec3f velocity = { 0.0f, 0.2f, 0.0f };
    static Vec3f accel = { 0.0f, 0.05f, 0.0f };
    static Color_RGBA8 primColor = { 255, 255, 255, 0 };
    static Color_RGBA8 envColor = { 255, 50, 50, 0 };
    Vec3f pos;

    velocity.x = Rand_CenteredFloat(3.0f);
    velocity.z = Rand_CenteredFloat(3.0f);
    velocity.y = -0.05f;
    accel.y = -0.025f;
    pos.x = Rand_CenteredFloat(12.0f) + this->actor.world.pos.x;
    pos.y = (Rand_ZeroOne() * 6.0f) + this->actor.world.pos.y;
    pos.z = Rand_CenteredFloat(12.0f) + this->actor.world.pos.z;
    EffectSsKirakira_SpawnDispersed(play, &pos, &velocity, &accel, &primColor, &envColor, 5000, 16);
}

static void ItemEtcetera_MoveFireArrowDown(ItemEtcetera* this, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 10.0f, 0.0f, 0x405);
    Actor_MoveWithGravity(&this->actor);
    if (!(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        ItemEtcetera_SpawnSparkles(this, play);
    }
    this->actor.shape.rot.y += 0x400;
    func_80B85824(this, play);
}

static void func_80B85B28(ItemEtcetera* this, PlayState* play) {
    if (Flags_GetTreasure(play, (this->actor.params >> 8) & 0x1F)) {
        Actor_Kill(&this->actor);
    }
}

static void ItemEtcetera_UpdateFireArrow(ItemEtcetera* this, PlayState* play) {
    if ((play->csCtx.state != CS_STATE_IDLE) && (play->csCtx.actorCues[0] != NULL)) {
        if (play->csCtx.actorCues[0]->id == 2) {
            this->actor.draw = ItemEtcetera_Draw;
            this->actor.gravity = -0.1f;
            this->actor.terminalVelocity = -4.0f;
            this->actionFunc = ItemEtcetera_MoveFireArrowDown;
        }
    } else {
        this->actor.gravity = -0.1f;
        this->actor.terminalVelocity = -4.0f;
        this->actionFunc = ItemEtcetera_MoveFireArrowDown;
    }
}

static void ItemEtcetera_Update(Actor* thisx, PlayState* play) {
    ItemEtcetera* this = (ItemEtcetera*)thisx;
    this->actionFunc(this, play);
}

static void ItemEtcetera_DrawThroughLens(Actor* thisx, PlayState* play) {
    ItemEtcetera* this = (ItemEtcetera*)thisx;
    if (play->actorCtx.lensActive) {
        func_800B8050(&this->actor, play, 0);
        func_800B8118(&this->actor, play, 0);
        GetItem_Draw(play, this->giDrawId);
    }
}

static void ItemEtcetera_Draw(Actor* thisx, PlayState* play) {
    ItemEtcetera* this = (ItemEtcetera*)thisx;

    func_800B8050(&this->actor, play, 0);
    func_800B8118(&this->actor, play, 0);
    GetItem_Draw(play, this->giDrawId);
}
