// Input for test_translate_calls.py: calls the translator rewrites by itself. The output must compile
// against mm/decls.h.
#include "decls.h"

#define SELF(x) (x)

void Test(PlayState* play, Actor* actor, Mtx* mtx, Lights* lights, LightNode* node, Vec3f* a, Vec3f* b) {
    PosRot focus;
    Vec3f dir;

    Actor_GetFocus(&focus, actor);
    Use(Actor_GetFocus(&focus, actor));
    OLib_Vec3fDistNormalize(&dir, a, b);
    OLib_Vec3fDistNormalize(&dir, &Actor_GetFocus(&focus, actor)->pos, b);
    Actor_GetFocus(&focus, SELF(actor));
    Matrix_ToMtx(mtx, "file,with(comma).c", 42);
    Lights_BindAll(lights, node, &focus.pos);
}
