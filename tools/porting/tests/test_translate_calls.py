#!/usr/bin/env python3
"""Tests for translate_calls.py. Run: python3 tools/porting/tests/test_translate_calls.py"""
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.join(HERE, "..", "translate_calls.py")


def translate(name):
    out = tempfile.NamedTemporaryFile(suffix=".c", delete=False).name
    proc = subprocess.run([sys.executable, "-I", TOOL, os.path.join(HERE, name), "-o", out,
                           f"--flags=-I{os.path.join(HERE, 'oot')}"], capture_output=True, text=True, check=True)
    with open(out) as f:
        text = f.read()
    os.unlink(out)
    return text, proc.stdout


class TranslateCalls(unittest.TestCase):
    def test_rewrites_match_expected(self):
        for name in ("calls.c", "review.c"):
            with self.subTest(name=name):
                text, _ = translate(name)
                with open(os.path.join(HERE, "expected", name)) as f:
                    self.assertEqual(text, f.read())

    def test_rewritten_calls_compile_against_mm(self):
        text, _ = translate("calls.c")
        with tempfile.NamedTemporaryFile("w", suffix=".c", dir=HERE, delete=False) as f:
            f.write(text)
        try:
            proc = subprocess.run(["clang", "-x", "c", "-fsyntax-only", "-Werror=incompatible-pointer-types",
                                   f"-I{os.path.join(HERE, 'mm')}", f.name], capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
        finally:
            os.unlink(f.name)

    def test_macro_calls_are_reported(self):
        _, report = translate("review.c")
        self.assertIn("Actor_GetFocus: inside a macro expansion, not rewritten", report)


if __name__ == "__main__":
    unittest.main()
