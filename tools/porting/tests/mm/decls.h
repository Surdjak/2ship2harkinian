#pragma once
// 2Ship signatures used by calls.c.
typedef struct { float x, y, z; } Vec3f;
typedef struct { short x, y, z; } Vec3s;
typedef struct { Vec3f pos; Vec3s rot; } PosRot;
typedef struct Actor { PosRot world; } Actor;
typedef struct PlayState PlayState;
typedef struct Mtx Mtx;
typedef struct Lights Lights;
typedef struct LightNode LightNode;

PosRot Actor_GetFocus(Actor* actor);
Vec3f OLib_Vec3fDistNormalize(Vec3f* a, Vec3f* b);
Mtx* Matrix_ToMtx(Mtx* dest);
void Lights_BindAll(Lights* lights, LightNode* node, Vec3f* pos, PlayState* play);
void Inventory_ChangeEquipment(short equipment);
float Use(PosRot* posRot);

