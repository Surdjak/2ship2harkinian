#!/usr/bin/env python3
"""Port SoH's z_draw.c (OoT's get-item model draw table) to hyrule/HyruleGetItemDraw.c.

    python3 tools/porting/port_oot_get_item_draw.py ../Shipwright/soh

OoT's draw functions become static GetItemOot_* functions over a static table, drawn by Hyrule_GetItemDrawOot
for draw ids from GID_OOT_BASE (GetItem_Draw in z_draw.c). Asset includes point at assets/oot/, OoT's display
setup and matrix calls become MM's, SoH's cosmetic options are dropped, and the entries SoH added with its own
assets (Triforce pieces, the fishing pole) draw nothing.
"""

import argparse
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "hyrule", "HyruleGetItemDraw.c")

SOH_ONLY_DLISTS = ("gTriforcePiece", "gGiFishingPoleDL", "gFishingLure")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("soh", help="SoH's soh directory")
    args = parser.parse_args()
    text = open(os.path.join(args.soh, "src", "code", "z_draw.c")).read()

    # SoH-only: the custom-item path and the entries drawn from SoH's own assets.
    text = re.sub(r"\n/\*\*\n \* Draw \"Get Item\" Model from a `GetItemEntry`.*?\n}\n", "\n", text, flags=re.S)
    for name in ("GetItem_DrawTriforcePiece", "GetItem_DrawTriforce", "GetItem_DrawFishingPole"):
        text = re.sub(r"\nvoid " + name + r"\(PlayState\* play, s16 drawId\) \{\n.*?\n\}(\n|$)", "\n", text, flags=re.S)
        text = text.replace(f"void {name}(PlayState* play, s16 drawId);\n", "")
        text = text.replace(f"{{ {name}, ", "{ GetItem_DrawNothing, ")
    text = re.sub(r"\{ GetItem_DrawNothing, \{[^}]*\} \}", "{ GetItem_DrawNothing, { NULL } }", text)

    # Cosmetic options: keep the defaults.
    text = re.sub(r"\n *if \(CVarGetInteger\(CVAR_COSMETIC\([^)]*\), 0\)\) \{\n(?:[^\n]*\n)*? *\}", "", text)
    assert "CVar" not in text, "cosmetic option left"

    # Includes.
    text = re.sub(r'#include "objects/(\w+)/(\w+)\.h"', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include "soh/[^"]+"\n', "", text)
    text = text.replace('#include "soh_assets.h"\n', "")  # SoH's own assets (the Triforce pieces)
    text = text.replace('#include "global.h"\n', '#include "global.h"\n#include "HyruleIds.h"\n', 1)

    # MM's names.
    text = re.sub(r"\bGfx_SetupDL_(\d+)(Opa|Xlu)\b", r"Gfx_SetupDL\1_\2", text)
    text = re.sub(r"\bMatrix_RotateY\(", "Matrix_RotateYF(", text)
    text = re.sub(r"\bMatrix_RotateZ\(", "Matrix_RotateZF(", text)
    text = re.sub(r"gSPMatrix\((\w+_DISP\+\+),\s*(?:MATRIX_NEWMTX|Matrix_NewMtx)\(([^,)]*)[^;]*;",
                  lambda m: f"MATRIX_FINALIZE_AND_LOAD({m.group(1)}, {m.group(2)});", text)
    assert "gSPMatrix" not in text and "Matrix_NewMtx" not in text, "matrix call left"

    # Our names: static functions over a static table.
    text = text.replace("DrawItemTableEntry", "DrawItemTableEntryOot")
    text = text.replace("sDrawItemTable", "sDrawItemTableOot")
    text = re.sub(r"\bGetItem_Draw(\w+)\b", r"GetItemOot_Draw\1", text)
    text = re.sub(r"^void GetItemOot_", "static void GetItemOot_", text, flags=re.M)
    text = re.sub(r"^DrawItemTableEntryOot sDrawItemTableOot", "static DrawItemTableEntryOot sDrawItemTableOot", text,
                  flags=re.M)
    text = re.sub(r"^(/\* 0x\w+ \*/ u8 \w+\[3\];)", r"static \1", text, flags=re.M)
    text = re.sub(r"/\*\*\n \* Draw \"Get Item\" Model\n.*?\nvoid GetItem_Draw\(PlayState\* play, s16 drawId\) \{\n"
                  r"    sDrawItemTableOot\[drawId\]\.drawFunc\(play, drawId\);\n\}",
                  "// OoT's GetItem_Draw, for draw ids from GID_OOT_BASE.\n"
                  "void Hyrule_GetItemDrawOot(PlayState* play, s16 drawId) {\n"
                  "    drawId -= GID_OOT_BASE;\n"
                  "    sDrawItemTableOot[drawId].drawFunc(play, drawId);\n}", text, flags=re.S)
    assert "Hyrule_GetItemDrawOot" in text, "entry point not found"
    text = text.replace("// \"Get Item\" Model Draw Functions\n",
                        "// \"Get Item\" Model Draw Functions\nstatic void GetItemOot_DrawNothing(PlayState* play, s16 drawId);\n", 1)
    text = text.replace("\n// All remaining functions",
                        "\nstatic void GetItemOot_DrawNothing(PlayState* play, s16 drawId) {\n}\n\n// All remaining functions", 1)
    text = re.sub(r"\bGetItem_DrawNothing\b", "GetItemOot_DrawNothing", text)

    header = ("// OoT's get-item models (docs/OOT_ITEMS.md): OoT's z_draw.c, ported from SoH by\n"
              "// tools/porting/port_oot_get_item_draw.py. Do not edit by hand.\n\n")
    with open(OUT, "w") as f:
        f.write(header + text)
    objects = sorted(set(re.findall(r'assets/oot/objects/(\w+)/', text)))
    print(f"written to {os.path.normpath(OUT)}; objects: {' '.join(objects)}")


if __name__ == "__main__":
    main()
