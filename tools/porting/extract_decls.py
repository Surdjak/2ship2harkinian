#!/usr/bin/env python3
"""Extract the function declarations a decomp's headers make, using clang with the game's own flags.

The compile command of one decomp source file is read from the Ninja build, rerun with clang on a
translation unit that includes every top-level header in --include-dir, and the JSON AST is reduced
to the functions declared in that directory. Scalar typedefs (s32, f32, ...) are recorded so signatures
can be compared across games independently of how a type was spelled.
"""
import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

# Headers that are not meant to be included on their own or that only hold data.
SKIP_HEADERS = {
    "prevent_bss_reordering.h", "prevent_bss_reordering2.h", "segment_symbols.h", "macro.inc",
    "message_data_fmt.h", "message_data_fmt_nes.h", "message_data_fmt_staff.h", "message_data_static.h",
    # SoH: no include guard, and global.h already includes it.
    "z64scene_enum.h",
}


def compile_command(build_dir, target, source_match):
    out = subprocess.run(["ninja", "-C", build_dir, "-t", "commands", target], capture_output=True, text=True,
                         check=True).stdout
    for line in out.splitlines():
        if source_match in line and " -c " in line:
            return shlex.split(line)
    sys.exit(f"no compile command for {source_match} in {target}")


def clang_args(command):
    """Keeps the defines, include paths and language options of a compile command."""
    keep = []
    it = iter(command[1:])
    for arg in it:
        if arg in ("-c", "-o", "-MF", "-MT", "-MQ"):
            next(it, None)
        elif arg in ("-MD", "-MMD") or arg.startswith("-W") or arg.startswith("-O") or arg == "-g":
            continue
        elif arg in ("-include", "-isystem", "-I", "-D", "-U"):
            keep += [arg, next(it)]
        elif arg.startswith(("-D", "-U", "-I", "-std=", "-include", "-f", "-m")):
            keep.append(arg)
    return keep


def type_of(node):
    t = node.get("type", {})
    return t.get("qualType", ""), t.get("desugaredQualType")


def walk(ast, include_dir):
    """Yields (node, file) for top-level declarations, tracking the file clang only names on change."""
    current = None
    for node in ast.get("inner", []):
        loc = node.get("loc", {})
        for candidate in (loc, loc.get("expansionLoc", {}), loc.get("spellingLoc", {})):
            if "file" in candidate:
                current = candidate["file"]
                break
        yield node, current


def extract(ast, include_dir):
    include_dir = os.path.realpath(include_dir)
    functions, typedefs = {}, {}
    for node, path in walk(ast, include_dir):
        if node.get("isImplicit"):
            continue
        in_dir = path is not None and os.path.realpath(path).startswith(include_dir + os.sep)
        kind = node.get("kind")
        if kind == "TypedefDecl":
            qual, desugared = type_of(node)
            typedefs[node["name"]] = desugared or qual
        if kind != "FunctionDecl" or not in_dir:
            continue
        name = node["name"]
        if name in functions:
            continue
        qual, _ = type_of(node)
        params = [
            {"name": p.get("name", ""), "type": type_of(p)[0]}
            for p in node.get("inner", []) if p.get("kind") == "ParmVarDecl"
        ]
        functions[name] = {
            "ret": qual.split("(", 1)[0].strip(),
            "params": params,
            "variadic": node.get("variadic", False) or qual.rstrip().endswith("...)"),
            "storage": node.get("storageClass", ""),
            "file": os.path.relpath(os.path.realpath(path), include_dir),
            "line": node.get("loc", {}).get("line"),
        }
    return functions, typedefs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--target", required=True, help="Ninja target whose commands include --source")
    parser.add_argument("--source", required=True, help="part of a decomp source path to copy flags from")
    parser.add_argument("--include-dir", required=True)
    parser.add_argument("--first", default="global.h", help="header to include before the others")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    flags = clang_args(compile_command(args.build_dir, args.target, args.source))
    headers = sorted(h for h in os.listdir(args.include_dir)
                     if h.endswith(".h") and h not in SKIP_HEADERS and h != args.first)
    with tempfile.TemporaryDirectory() as tmp:
        tu = os.path.join(tmp, "all_headers.c")
        with open(tu, "w") as f:
            f.write(f'#include "{args.first}"\n')
            f.writelines(f'#include "{h}"\n' for h in headers)
        cmd = ["clang", "-x", "c", "-fsyntax-only", "-ferror-limit=0", "-w", "-Xclang", "-ast-dump=json", *flags, tu]
        proc = subprocess.run(cmd, capture_output=True, text=True)
    errors = [l for l in proc.stderr.splitlines() if ": error:" in l]
    if not proc.stdout:
        sys.exit("clang produced no AST:\n" + proc.stderr[-4000:])

    functions, typedefs = extract(json.loads(proc.stdout), args.include_dir)
    with open(args.out, "w") as f:
        json.dump({"functions": functions, "typedefs": typedefs, "headers": len(headers) + 1,
                   "clang_errors": errors}, f, indent=1, sort_keys=True)
    print(f"{len(functions)} functions from {len(headers) + 1} headers, {len(errors)} clang errors -> {args.out}")
    for e in errors[:10]:
        print("  " + e)


if __name__ == "__main__":
    main()
