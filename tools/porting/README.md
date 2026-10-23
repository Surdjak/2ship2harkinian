# Porting tools

Tools for moving OoT code from SoH onto 2Ship's engine. They use clang with each game's real compile
flags, read from the Ninja builds, so they see the code exactly as the compiler does.

| Tool | Purpose |
|------|---------|
| `extract_decls.py` | Lists the functions declared in a decomp's headers, with exact types. |
| `compare_signatures.py` | Compares SoH's and 2Ship's lists and writes [docs/SIGNATURES.md](../../docs/SIGNATURES.md). |
| `translate_calls.py` | Rewrites calls in an OoT source file to 2Ship's signatures, following `call_rules.json`. |
| `gen_oot_tables.py` | Generates the OoT parts of 2Ship's actor and object tables from SoH's (see [hyrule/README.md](../../hyrule/README.md)). |

Both builds must exist: this repo's `build-cmake` and a SoH build (see
[harness/README.md](../../harness/README.md)).

## Regenerating the signature report

```bash
python3 tools/porting/extract_decls.py --build-dir ../Shipwright/build-cmake --target soh/soh.elf --source src/code/z_actor.c --include-dir ../Shipwright/soh/include --out soh.json
```

```bash
python3 tools/porting/extract_decls.py --build-dir build-cmake --target mm/2s2h.elf --source src/code/z_actor.c --include-dir mm/include --out mm.json
```

```bash
python3 tools/porting/compare_signatures.py soh.json mm.json --json signatures.json --markdown docs/SIGNATURES.md
```

Only declarations in each decomp's `include` directory are compared. Port headers (`soh/soh`,
`mm/2s2h`) and prototypes written in `.c` files are not.

## Translating calls

```bash
python3 tools/porting/translate_calls.py ../Shipwright/soh/src/overlays/actors/ovl_En_Bw/z_en_bw.c -o z_en_bw.c --soh-build ../Shipwright/build-cmake
```

Each rule in `call_rules.json` has one of these kinds:

| Kind | Effect |
|------|--------|
| `out_param_to_return` | `F(&x, a);` becomes `x = F(a);`; inside an expression, `(x = F(a), &x)`. |
| `drop_args` | Removes arguments, such as OoT's debug file and line. |
| `append_args` | Appends an expression such as `play`, with a `PORT: review` comment. |
| `manual` | Leaves the call and adds a `PORT:` comment explaining what changed. |

Calls written inside a macro expansion are listed in the report but not rewritten.

Run over every SoH file that calls a ruled function, the translator rewrote 99 calls, marked 109 for
review and reported 9 inside macros.

Run the tests after changing the translator or the rules:

```bash
python3 tools/porting/tests/test_translate_calls.py
```

## Coccinelle

The translator uses clang, which is installed with the toolchain. Coccinelle (`spatch`) is not
installed here. It would suit rewrites that need to match statement patterns rather than single
calls.
