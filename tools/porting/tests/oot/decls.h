#pragma once
// OoT (SoH) signatures used by calls.c.
typedef struct { float x, y, z; } Vec3f;
typedef struct { short x, y, z; } Vec3s;
typedef struct { Vec3f pos; Vec3s rot; } PosRot;
typedef struct Actor { PosRot world; } Actor;
typedef struct PlayState PlayState;
typedef struct Mtx Mtx;
typedef struct Lights Lights;
typedef struct LightNode LightNode;

PosRot* Actor_GetFocus(PosRot* dest, Actor* actor);
Vec3f* OLib_Vec3fDistNormalize(Vec3f* dest, Vec3f* a, Vec3f* b);
Mtx* Matrix_ToMtx(Mtx* dest, char* file, int line);
void Lights_BindAll(Lights* lights, LightNode* node, Vec3f* pos);
void Inventory_ChangeEquipment(short equipment, unsigned short value);
float Use(PosRot* posRot);

