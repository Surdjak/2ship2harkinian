// Input for test_translate_calls.py: calls the translator rewrites by itself. The output must compile
// against mm/decls.h.
#include "decls.h"

#define SELF(x) (x)

void Test(PlayState* play, Actor* actor, Mtx* mtx, Lights* lights, LightNode* node, Vec3f* a, Vec3f* b) {
    PosRot focus;
    Vec3f dir;

    focus = Actor_GetFocus(actor);
    Use((focus = Actor_GetFocus(actor), &focus));
    dir = OLib_Vec3fDistNormalize(a, b);
    dir = OLib_Vec3fDistNormalize(&(focus = Actor_GetFocus(actor), &focus)->pos, b);
    focus = Actor_GetFocus(SELF(actor));
    Matrix_ToMtx(mtx);
    /* PORT: review appended play */ Lights_BindAll(lights, node, &focus.pos, play);
}
