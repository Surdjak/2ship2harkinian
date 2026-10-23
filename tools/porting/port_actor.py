#!/usr/bin/env python3
"""Copy a SoH actor into hyrule/overlays/actors/ and apply the mechanical renames of hyrule/README.md.

    python3 tools/porting/port_actor.py ../Shipwright/soh/src/overlays/actors/ovl_En_Ko

What it does (each a plain text rewrite):
- ActorInit becomes an ActorProfile named <Name>_Oot_Profile, with _Oot actor and object ids; OoT's
  gameplay_keep becomes MM's GAMEPLAY_KEEP, which is always loaded.
- Actor and object ids elsewhere get the _Oot suffix, and SoH asset includes point at assets/oot/.
- Actor_UpdateBgCheckInfo flags with OoT's floor-and-water check (4) gain MM's water check (0x400).
- Collider, init chain and actor field names change to MM's (COL_MATERIAL_, ATELEM_, cullingVolume*,
  terminalVelocity, speed, depthInWater, objectSlot, attentionRangeType, lockOnArrowOffset...).
- OoT flags use HyruleFlags.h, OoT text ids in returns, cases and textId assignments get TEXT_OOT, and
  OoT age checks use HyrulePort.h.
- Debug prints are removed, SoH's GameInteractor_Should(VB_..., cond) hooks become (cond), and OoT's
  matrix, allocation, segment and default-sound calls become MM's. SoH's cvars and stats, and anything
  else that needs a decision, are listed at the end for review.
- Functions become static.

Data files that the actor #includes (such as z_en_kanban_gfx.c) need renaming by hand, since every .c
file under hyrule/ is compiled.

The result needs a build and a review: semantic differences (Player cutscene actions, sound effects,
damage masks, drop tables) are not handled, and the report lists what to check.
"""
import argparse
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_ROOT = os.path.join(HERE, "..", "..", "hyrule", "overlays", "actors")

RENAMES = [
    (r"\bCOLTYPE_", "COL_MATERIAL_"),
    (r"\bELEMTYPE_", "ELEM_MATERIAL_"),
    (r"\bTOUCH_ON\b", "ATELEM_ON"),
    (r"\bTOUCH_NONE\b", "ATELEM_NONE"),
    (r"\bTOUCH_SFX_", "ATELEM_SFX_"),
    (r"\bBUMP_ON\b", "ACELEM_ON"),
    (r"\bBUMP_NONE\b", "ACELEM_NONE"),
    (r"\bBUMP_HOOKABLE\b", "ACELEM_HOOKABLE"),
    (r"\buncullZoneForward\b", "cullingVolumeDistance"),
    (r"\buncullZoneScale\b", "cullingVolumeScale"),
    (r"\buncullZoneDownward\b", "cullingVolumeDownward"),
    (r"\bminVelocityY\b", "terminalVelocity"),
    (r"\bactor\.speedXZ\b", "actor.speed"),
    (r"\byDistToWater\b", "depthInWater"),
    (r"\bobjBankIndex\b", "objectSlot"),
    (r"\btargetMode\b", "attentionRangeType"),
    (r"\btargetArrowOffset\b", "lockOnArrowOffset"),
    (r"\bisTargeted\b", "isLockedOn"),
    (r"\bTEXT_STATE_DONE_FADING\b", "TEXT_STATE_FADING"),
    (r"\bTEXT_STATE_DONE_HAS_NEXT\b", "TEXT_STATE_NEXT"),
    (r"\bActor_ProcessTalkRequest\((&?[\w>.-]+), play\)", r"Actor_TalkOfferAccepted(\1, &play->state)"),
    (r"\bSEGMENTED_TO_VIRTUAL\(", "Lib_SegmentedToVirtual("),
    (r"\bQuake_Add\(([^,]+), (\d)\)", r"Quake_Request(\1, QUAKE_TYPE_\2)"),
    (r"\bQuake_SetQuakeValues\(", "Quake_SetPerturbations("),
    (r"\bWaterBox_GetSurfaceImpl\(", "BgCheck_GetWaterSurfaceNoBgId("),
    (r"\bActor_UpdateVelocityXZGravity\(", "Actor_UpdateVelocityWithGravity("),
    (r"\bMatrix_RotateAxis\(", "Matrix_RotateAxisF("),
    (r"\bMatrix_MtxFToYXZRotS\(", "Matrix_MtxFToYXZRot("),
    (r"\bQuake_SetCountdown\(", "Quake_SetDuration("),
    (r"\bfunc_80033480\(", "func_800BBFB0("),
    (r"\bActor_SetPlayerKnockbackLarge\(", "func_800B8D50("),
    (r"\bDamageTable_Get\(", "DamageTable_Get_Oot("),
    (r"\bIS_DAY\b", "IS_DAY_OOT"),
    (r"\bGET_GS_FLAGS\(", "GET_GS_FLAGS_OOT("),
    (r"\bSET_GS_FLAGS\(", "SET_GS_FLAGS_OOT("),
    (r"\bSkelAnime_DrawSkeletonOpa\(play, &([\w>.-]+), ", r"SkelAnime_DrawOpa(play, \1.skeleton, \1.jointTable, "),
    (r"\bDEGF_TO_RADF\(", "DEG_TO_RAD("),
    (r"\bIS_NIGHT\b", "IS_NIGHT_OOT"),
    (r"\bactorCtx\.unk_02\b", "actorCtx.unk2"),
    (r"\bPLAYER_STATE1_CLIMBING_LADDER\b", "PLAYER_STATE1_200000"),
    (r"\b(elements\[[^]]+\])\.info\.", r"\1.base."),
    (r"\bcollider\.info\.", "collider.elem."),
    (r"\.toucher\.", ".atDmgInfo."),
    (r"\.bumper\.", ".acDmgInfo."),
    (r"\.toucherFlags\b", ".atElemFlags"),
    (r"\.bumperFlags\b", ".acElemFlags"),
    (r"\bacHitInfo\b", "acHitElem"),
    (r"\batHitInfo\b", "atHitElem"),
    (r"\bPlayer_PlaySfx\(&GET_PLAYER\(play\)->actor, ", "Player_PlaySfx(GET_PLAYER(play), "),
    (r"\bPLAYER_STATE1_IN_ITEM_CS\b", "PLAYER_STATE1_10000000"),
    (r"\bPLAYER_STATE1_IN_CUTSCENE\b", "PLAYER_STATE1_20000000"),
    (r"\bVIRTUAL_TO_PHYSICAL\(", "OS_K0_TO_PHYSICAL("),
    (r"\bGraph_Alloc\(", "GRAPH_ALLOC("),
    (r"\bMatrix_RotateX\(([^;]*), MTXMODE_APPLY\);", r"Matrix_RotateXFApply(\1);"),
    (r"\bMatrix_RotateX\(([^;]*), MTXMODE_NEW\);", r"Matrix_RotateXFNew(\1);"),
    (r"\bMatrix_RotateY\(([^;]*), (MTXMODE_\w+)\);", r"Matrix_RotateYF(\1, \2);"),
    (r"\bMatrix_RotateZ\(([^;]*), (MTXMODE_\w+)\);", r"Matrix_RotateZF(\1, \2);"),
    (r"\bgSPMatrix\((POLY_\w+_DISP\+\+), MATRIX_NEWMTX\(([^)]*)\),\s*G_MTX_NOPUSH \| G_MTX_LOAD \| G_MTX_MODELVIEW\);",
     r"MATRIX_FINALIZE_AND_LOAD(\1, \2);"),
    (r"\bAudio_PlaySfxGeneral\((\w+), &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,\s*&gSfxDefaultFreqAndVolScale, "
     r"&gSfxDefaultReverb\);", r"Audio_PlaySfx(\1);"),
    (r"\bObject_GetIndex\(", "Object_GetSlot("),
    # OoT's Floor5 (with play) is MM's Floor5_2, and OoT's Floor4 is MM's Floor5; in that order.
    (r"\bBgCheck_EntityRaycastFloor5\(", "BgCheck_EntityRaycastFloor5_2("),
    (r"\bBgCheck_EntityRaycastFloor4\(", "BgCheck_EntityRaycastFloor5("),
    (r"\bActor_MoveXZGravity\(", "Actor_MoveWithGravity("),
    (r"\bAudio_PlayActorSound2\(", "Actor_PlaySfx("),
    (r"\bGfx_SetupDL_25Opa\(", "Gfx_SetupDL25_Opa("),
    (r"\bGfx_SetupDL_25Xlu\(", "Gfx_SetupDL25_Xlu("),
    (r"\bAnimation_ChangeByInfo\(", "Actor_ChangeAnimationByInfo("),
    (r"\bACTOR_FLAG_GRASS_DESTROYED\b", "ACTOR_FLAG_800"),
    (r"\bplay->sceneNum\b", "play->sceneId"),
    (r"\bLINK_IS_ADULT\b", "LINK_IS_ADULT_OOT"),
    (r"\bLINK_IS_CHILD\b", "LINK_IS_CHILD_OOT"),
    (r"\bCHECK_QUEST_ITEM\((QUEST_\w+)\)", r"CHECK_QUEST_ITEM_OOT(\1_Oot)"),
    (r"\bINV_CONTENT\(", "INV_CONTENT_OOT("),
    (r"\bActor_GetPlayerExchangeItemId\(", "ACTOR_GET_PLAYER_EXCHANGE_ITEM_ID_OOT("),
    (r"\bCUR_EQUIP_VALUE\(", "GET_CUR_EQUIP_VALUE("),
    (r"\bFlags_GetEventChkInf\((\w+)\)", r"GET_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_SetEventChkInf\((\w+)\)", r"SET_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_UnsetEventChkInf\((\w+)\)", r"CLEAR_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_GetInfTable\((\w+)\)", r"GET_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_SetInfTable\((\w+)\)", r"SET_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_UnsetInfTable\((\w+)\)", r"CLEAR_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_GetItemGetInf\((\w+)\)", r"GET_ITEMGETINF_OOT(\1_Oot)"),
    (r"\bFlags_SetItemGetInf\((\w+)\)", r"SET_ITEMGETINF_OOT(\1_Oot)"),
    (r"\bFlags_GetEventInf\((\w+)\)", r"GET_EVENTINF_OOT(\1_Oot)"),
    (r"\bFlags_SetEventInf\((\w+)\)", r"SET_EVENTINF_OOT(\1_Oot)"),
    (r"\b(ACTOR_(?!FLAG_)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(OBJECT_(?!GAMEPLAY_KEEP\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\bOBJECT_GAMEPLAY_KEEP\b", "GAMEPLAY_KEEP"),
    (r"\b(SCENE_[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(ITEM_(?!NONE\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
]
# Lines that are SoH-only and dropped when they stand alone.
DROP_LINE = re.compile(r"^\s*#include \"(vt\.h|soh/[^\"]+)\"\s*$|^\s*#include <libultraship/bridge/[^>]+>\s*$")
TEXT_ID = re.compile(r"\b(0x[1-7][0-9A-Fa-f]{3})\b")
TEXT_LINE = re.compile(r"\breturn\b|\bcase\b|textId\b")
PROFILE = re.compile(
    r"const ActorInit (\w+)_InitVars = \{\s*(\w+),\s*(\w+),\s*(\w+),\s*(\w+),\s*sizeof\((\w+)\),\s*"
    r"(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),"
    r"\s*(?:\(ActorResetFunc\))?(\w+),?\s*\};")


def profile(m):
    name, actor, cat, flags, obj, size, init, destroy, update, draw, reset = m.groups()
    lines = [f"ActorProfile {name}_Oot_Profile = {{"]
    for v in (actor, cat, flags, obj, f"sizeof({size})", init, destroy, update, draw):
        lines.append(f"    /**/ {v},")
    if reset != "NULL":
        lines.append(f"    /**/ {reset},")
    lines.append("};")
    return "\n".join(lines)


def strip_game_interactor(text):
    """GameInteractor_Should(VB_X, cond[, ...]) is SoH's hook around vanilla behavior: keep (cond)."""
    out, i = [], 0
    while True:
        j = text.find("GameInteractor_Should(", i)
        if j < 0:
            out.append(text[i:])
            return "".join(out)
        out.append(text[i:j])
        k = j + len("GameInteractor_Should(")
        depth, args, start = 1, [], k
        while depth:
            c = text[k]
            if c in "([{":
                depth += 1
            elif c in ")]}":
                depth -= 1
            elif c == "," and depth == 1:
                args.append(text[start:k])
                start = k + 1
            k += 1
        args.append(text[start:k - 1])
        out.append("(" + args[1].strip() + ")" if len(args) > 1 else text[j:k])
        i = k


def convert(text, header):
    out = []
    for line in text.splitlines():
        if DROP_LINE.match(line):
            continue
        if TEXT_LINE.search(line) and not line.lstrip().startswith("//"):
            line = TEXT_ID.sub(r"TEXT_OOT(\1)", line)
        out.append(line)
    text = "\n".join(out) + "\n"
    text = re.sub(r'#include "objects/(\w+)/(\w+)\.h"', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include <objects/(\w+)/(\w+)\.h>', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include "overlays/(ovl_\w+)/(\w+)\.h"', r'#include "assets/oot/overlays/\1/\2.h"', text)
    # One-line debug prints.
    text = re.sub(r"\n[ \t]*// \"[^\n]*\"(?=\n[ \t]*osSyncPrintf)", "", text)
    text = re.sub(r"\n[ \t]*osSyncPrintf\((?:[^;]|\n)*?\);", "", text)
    for pattern, repl in RENAMES:
        text = re.sub(pattern, repl, text)
    text = strip_game_interactor(text)
    # OoT's background check flag 4 covers floor and water; MM checks water only with 0x400.
    text = re.sub(r"(Actor_UpdateBgCheckInfo\((?:[^;()]|\([^()]*\))*, )(0x[0-9A-Fa-f]+|\d+)\)",
                  lambda m: m.group(0) if not (int(m.group(2), 0) & 4) else
                  f"{m.group(1)}0x{int(m.group(2), 0) | 0x400:X})", text)
    # Collider damage masks (the first field of an element's AT and AC info) take MM's bits too.
    text = re.sub(r"\{ (0x[0-9A-Fa-f]{8}), (0x[0-9A-Fa-f]{2}), (0x[0-9A-Fa-f]{2}) \}",
                  lambda m: m.group(0) if int(m.group(1), 16) == 0 else
                  f"{{ DMG_FROM_OOT({m.group(1)}), {m.group(2)}, {m.group(3)} }}", text)
    if not header:
        text = PROFILE.sub(profile, text)
        text = re.sub(r"^(void|s32|u16|s16|f32|u8|u32) (\w+_\w+\()", r"static \1 \2", text, flags=re.M)
        # After the actor's own header, which comes first.
        extra = '#include "HyruleFlags.h"\n#include "HyrulePort.h"\n#include "HyruleActor.h"\n'
        extra += '#include "z64quake.h"\n' if "Quake_" in text else ""
        extra += '#include "z64rumble.h"\n' if "Rumble_" in text else ""
        text = re.sub(r'(#include "[^"]+"\n)', lambda m: m.group(1) + extra, text, count=1)
    else:
        text = text.replace("#include <libultraship/libultra.h>\n", "")
        text = re.sub(r"#ifndef (\w+)_H\n#define \1_H", r"#ifndef \1_OOT_H\n#define \1_OOT_H", text, count=1)
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("soh_actor_dir", help="SoH's soh/src/overlays/actors/ovl_<Name> directory")
    args = parser.parse_args()
    src = os.path.normpath(args.soh_actor_dir)
    out = os.path.join(OUT_ROOT, os.path.basename(src))
    os.makedirs(out, exist_ok=True)
    review = re.compile(r"GameInteractor|CVar|IS_RANDO|Randomizer|gSaveContext\.ship|Sfx_Play|Audio_Play|"
                        r"Player_SetCsAction|func_8002DF54|dmgFlags|"
                        r"Item_Drop|ITEM00_|MATRIX_NEWMTX|#include \"overlays/")
    for name in sorted(os.listdir(src)):
        if not name.endswith((".c", ".h")):
            continue
        with open(os.path.join(src, name)) as f:
            text = f.read()
        result = convert(text, name.endswith(".h"))
        with open(os.path.join(out, name), "w") as f:
            f.write(result)
        for i, line in enumerate(result.splitlines(), 1):
            if review.search(line):
                print(f"{os.path.join(out, name)}:{i}: review: {line.strip()[:110]}")
    print(f"ported into {os.path.normpath(out)}")


if __name__ == "__main__":
    main()
