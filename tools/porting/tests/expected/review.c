// Input for test_translate_calls.py: calls the translator leaves for a person, with a PORT comment or a
// report line.
#include "decls.h"

#define GET_FOCUS(dest, actor) Actor_GetFocus(dest, actor)

void Test(Actor* actor) {
    PosRot focus;

    /* PORT: MM takes only the equipment slot */ Inventory_ChangeEquipment(1, 2);
    GET_FOCUS(&focus, actor);
}
