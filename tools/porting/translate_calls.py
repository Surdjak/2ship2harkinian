#!/usr/bin/env python3
"""Rewrite calls in an OoT source file to 2Ship's engine signatures, following call_rules.json.

The file is parsed by clang with SoH's compile flags (or with explicit flags for tests), so calls are
found from the AST rather than by pattern matching. Rule kinds:

  out_param_to_return  OoT writes its result through argument `out`; 2Ship returns it.
                       `F(&x, a);` becomes `x = F(a);`. Inside an expression the call becomes
                       `(*(dest) = F(a), (dest))`, which keeps the pointer value OoT returned.
  drop_args            Remove the arguments at `args` (indexes into the OoT call).
  append_args          Append the expressions in `args`. They are inserted as written, so the
                       result must be reviewed; a PORT comment is added.
  manual               Leave the call alone and add a PORT comment with the rule's note.

Calls written inside macro expansions are reported but not rewritten.
"""
import argparse
import json
import os
import shlex
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extract_decls import clang_args, compile_command  # noqa: E402


def load_ast(path, flags):
    cmd = ["clang", "-x", "c", "-fsyntax-only", "-ferror-limit=0", "-w", "-Xclang", "-ast-dump=json", *flags, path]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if not proc.stdout:
        sys.exit("clang produced no AST:\n" + proc.stderr[-4000:])
    return json.loads(proc.stdout)


class Collector:
    """Finds calls to ruled functions in the main file, with argument offsets."""

    def __init__(self, main_file, source, rules):
        self.main = os.path.realpath(main_file)
        self.src = source
        self.rules = rules
        self.file = None
        self.line = None
        self.calls = []
        self.in_macro = []
        self.skipped = []

    def track(self, node):
        for key in ("loc", "range"):
            value = node.get(key, {})
            for part in (value, value.get("begin", {}), value.get("end", {})):
                for loc in (part, part.get("spellingLoc", {}), part.get("expansionLoc", {})):
                    if "file" in loc:
                        self.file = os.path.realpath(loc["file"])
        # Like the file, clang only writes the line when it changes.
        begin = node.get("range", {}).get("begin", {})
        begin = begin.get("expansionLoc", begin)
        if "line" in begin:
            self.line = begin["line"]

    @staticmethod
    def span(node):
        r = node.get("range", {})
        begin, end = r.get("begin", {}), r.get("end", {})
        if "offset" not in begin or "offset" not in end:
            return None
        return begin["offset"], end["offset"] + end.get("tokLen", 0)

    def split_args(self, start, end):
        """Spans of the top-level arguments of the call written at [start, end).

        Argument ranges are taken from the source text because clang gives arguments that contain
        macros only macro-relative positions.
        """
        src, i = self.src, self.src.index("(", start) + 1
        close = end - 1
        if src[close] != ")":
            return None
        spans, arg_start, depth = [], i, 0
        while i < close:
            c = src[i]
            if c in "\"'":
                i += 1
                while src[i] != c:
                    i += 2 if src[i] == "\\" else 1
            elif src.startswith("/*", i):
                i = src.index("*/", i) + 1
            elif src.startswith("//", i):
                i = src.index("\n", i)
            elif c in "([{":
                depth += 1
            elif c in ")]}":
                depth -= 1
            elif c == "," and depth == 0:
                spans.append((arg_start, i))
                arg_start = i + 1
            i += 1
        if src[arg_start:close].strip() or spans:
            spans.append((arg_start, close))
        return [(a + len(src[a:b]) - len(src[a:b].lstrip()), b - (len(src[a:b]) - len(src[a:b].rstrip())))
                for a, b in spans]

    @staticmethod
    def callee_name(call):
        node = call["inner"][0]
        while node.get("kind") in ("ImplicitCastExpr", "ParenExpr") and node.get("inner"):
            node = node["inner"][0]
        if node.get("kind") == "DeclRefExpr":
            return node.get("referencedDecl", {}).get("name")
        return None

    def visit(self, node, parent_kind=None):
        self.track(node)
        if node.get("kind") == "CallExpr" and self.file == self.main:
            name = self.callee_name(node)
            if name in self.rules:
                r = node.get("range", {})
                if "spellingLoc" in r.get("begin", {}) or "expansionLoc" in r.get("begin", {}):
                    self.in_macro.append((name, self.line))
                else:
                    span = self.span(node)
                    args = self.split_args(*span) if span else None
                    if args is None or len(args) != len(node["inner"]) - 1:
                        self.skipped.append((name, self.line))
                    else:
                        self.calls.append({"name": name, "span": span, "args": args,
                                           "statement": parent_kind == "CompoundStmt",
                                           "line": self.line})
        for child in node.get("inner", []):
            self.visit(child, node.get("kind"))


class Rewriter:
    def __init__(self, source, calls, rules):
        self.src = source
        self.calls = sorted(calls, key=lambda c: (c["span"][0], -c["span"][1]))
        self.rules = rules
        self.report = []

    def render(self, start, end):
        """Source text of [start, end) with every ruled call inside it rewritten."""
        out, pos = [], start
        for call in self.calls:
            s, e = call["span"]
            if s < pos or e > end:
                continue
            out.append(self.src[pos:s])
            out.append(self.rewrite(call))
            pos = e
        out.append(self.src[pos:end])
        return "".join(out)

    def rewrite(self, call):
        rule = self.rules[call["name"]]
        args = [self.render(s, e) for s, e in call["args"]]
        name, kind = call["name"], rule["kind"]
        self.report.append(f"line {call['line']}: {name}: {kind}")

        if kind == "drop_args":
            kept = [a for i, a in enumerate(args) if i not in rule["args"]]
            return f"{name}({', '.join(kept)})"
        if kind == "append_args":
            return f"/* PORT: review appended {', '.join(rule['args'])} */ {name}({', '.join(args + rule['args'])})"
        if kind == "out_param_to_return":
            out = args[rule["out"]].strip()
            rest = [a for i, a in enumerate(args) if i != rule["out"]]
            value = f"{name}({', '.join(rest)})"
            if call["statement"]:
                target = out[1:].strip() if out.startswith("&") else f"*({out})"
                return f"{target} = {value}"
            if out.startswith("&"):
                target = out[1:].strip()
                return f"({target} = {value}, &{target})"
            return f"(*({out}) = {value}, ({out}))"
        if kind == "manual":
            return f"/* PORT: {rule['note']} */ {name}({', '.join(args)})"
        sys.exit(f"unknown rule kind {kind} for {name}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input")
    parser.add_argument("-o", "--output", required=True)
    parser.add_argument("--rules", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "call_rules.json"))
    flags = parser.add_mutually_exclusive_group(required=True)
    flags.add_argument("--soh-build", help="SoH build directory to take compile flags from")
    flags.add_argument("--flags", help="explicit clang flags, e.g. for tests")
    args = parser.parse_args()

    rules = {k: v for k, v in json.load(open(args.rules)).items() if not k.startswith("_")}
    if args.soh_build:
        clang_flags = clang_args(compile_command(args.soh_build, "soh/soh.elf", "src/code/z_actor.c"))
    else:
        clang_flags = shlex.split(args.flags)

    # clang reports byte offsets; Latin-1 maps each byte to one character and back unchanged.
    with open(args.input, encoding="latin-1") as f:
        source = f.read()
    collector = Collector(args.input, source, rules)
    collector.visit(load_ast(args.input, clang_flags))

    rewriter = Rewriter(source, collector.calls, rules)
    result = rewriter.render(0, len(source))
    with open(args.output, "w", encoding="latin-1") as f:
        f.write(result)

    for line in rewriter.report:
        print(line)
    for name, line in collector.in_macro:
        print(f"line {line}: {name}: inside a macro expansion, not rewritten")
    for name, line in collector.skipped:
        print(f"line {line}: {name}: arguments could not be read, not rewritten")
    print(f"{len(rewriter.report)} calls rewritten, {len(collector.in_macro)} left in macros, "
          f"{len(collector.skipped)} unreadable -> {args.output}")


if __name__ == "__main__":
    main()
