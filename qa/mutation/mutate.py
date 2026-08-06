#!/usr/bin/env python3
"""
Custom lightweight mutation testing driver for the sim core.

Why custom: mull (LLVM bitcode) and dextool both need a clang/LLVM or LDC
toolchain that isn't installed here, and installing one needs `sudo apt`,
which needs an interactive password unavailable in this environment. This
drives the existing GCC pipeline instead (the same one qa/coverage/ uses):
one mutation at a time, rebuild, rerun the full hand-written suite, record
whether any test caught it.

Mutation operators, applied only on lines the coverage report shows as
executed (mutating dead code just rediscovers a coverage gap, not oracle
weakness -- gcovr's JSON output from the coverage build is the source of
truth for "executed"):
  - ConditionNegation:       if (X)  ->  if (!(X))
  - RelationalOperatorSwap:  <->less-or-equal pairs, ==<->!=, &&<->||
                              (scoped to if/while/for(;COND;) condition
                              parens only, to avoid template <> ambiguity)
  - BooleanLiteralFlip:      return true; <-> return false;
Plus a curated ConstantPerturbation list for named physics constants
(SLOP, gravity, default restitution, MAX_CONSTRAINT_ITERATION, ...) that
line-coverage scanning wouldn't find on its own.

Safety: each target file is fully backed up before any mutation is applied
to it, restored immediately after every single mutant (success or failure,
via try/finally), and re-verified byte-for-byte identical to the backup
after the whole run. A per-mutant build+test timeout guards against a
mutant that hangs the suite (e.g. a negated loop condition).
"""
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path("/mnt/c/Users/dorcs/Desktop/Wheel")
BUILD_DIR = Path(os.path.expanduser("~/wheel-mutation-build"))
COVERAGE_BUILD_DIR = Path(os.path.expanduser("~/wheel-coverage-build"))
GCOVR = Path(os.path.expanduser("~/gcovr-venv/bin/gcovr"))
RESULTS_PATH = Path(__file__).parent / "results.json"

BINARIES = [
    BUILD_DIR / "qa/Google_tests/Google_Tests_run",
    BUILD_DIR / "qa/harness_tests/QA_Harness_Tests",
    BUILD_DIR / "qa/oracle_tests/QA_Oracle_Tests",
    BUILD_DIR / "qa/agent_tests/Agent_Tests",
]

# Richest target first, per the brief: the constraint solver, then the
# narrow-phase collision routines. Easy to extend -- just add more paths here.
TARGET_FILES = [
    "wheel/physics/CollisionConstraintSolver.cpp",
    "wheel/physics/JointConstraintSolver.cpp",
    "wheel/physics/Constraint2DResolver.cpp",
    "wheel/physics/Physics2DSystem.cpp",
    "wheel/collision/BoxBoxCollision2D.cpp",
    "wheel/collision/CircleBoxCollision2D.cpp",
    "wheel/collision/CircleCircleCollision2D.cpp",
]

# Hand-picked, not line-coverage gated: named constants whose exact value is
# physically meaningful. (file, unique substring, replacement, description).
# Each "unique substring" is asserted to appear exactly once in its file.
CONSTANT_MUTANTS = [
    ("wheel/physics/CollisionConstraintSolver.cpp",
     "const float SLOP = 0.01f;", "const float SLOP = 0.03f;",
     "solver's allowed penetration slop tripled"),
    ("wheel/physics/CollisionConstraintSolver.cpp",
     "const float MAX_LINEAR_CORRECTION = 0.2f;", "const float MAX_LINEAR_CORRECTION = 0.0f;",
     "position correction disabled"),
    ("wheel/core/Globals.h",
     "#define MAX_CONSTRAINT_ITERATION 6", "#define MAX_CONSTRAINT_ITERATION 1",
     "solver iteration budget cut from 6 to 1"),
    ("wheel/physics/Rigidbody2D.h",
     "float restitution = 0.02f;", "float restitution = 0.5f;",
     "default restitution raised (default bodies become bouncier)"),
    ("wheel/physics/Physics2DSystem.h",
     "Math::Vector2 m_Gravity{0.0f, -9.81f};", "Math::Vector2 m_Gravity{0.0f, -6.81f};",
     "gravity constant weakened"),
]

REL_OP_PATTERN = re.compile(
    r'<=|>=|==|!=|&&|\|\||(?<!<)<(?!<)|(?<!-)(?<!>)>(?!>)'
)
REL_OP_SWAP = {'<': '<=', '<=': '<', '>': '>=', '>=': '>', '==': '!=', '!=': '==', '&&': '||', '||': '&&'}

RETURN_TRUE = re.compile(r'\breturn\s+true\s*;')
RETURN_FALSE = re.compile(r'\breturn\s+false\s*;')

# Reflection macro lines (REFLECT_BEGIN/FIELD/REFLECT_END) are debug-only
# descriptor plumbing, not testable behaviour, and are never exercised by the
# headless suite -- never mutate them. Mirrors the --exclude-lines-by-pattern
# rule in qa/coverage/run_coverage.sh so both tools agree on what to ignore.
EXCLUDE_LINE_PATTERN = re.compile(r'\s*(REFLECT_BEGIN|FIELD|REFLECT_END)\s*\(')


def line_of(text, offset):
    return text.count('\n', 0, offset) + 1


def excluded_line_numbers(text):
    """1-based line numbers matching EXCLUDE_LINE_PATTERN (reflection macros)."""
    return {i for i, line in enumerate(text.splitlines(), 1)
            if EXCLUDE_LINE_PATTERN.match(line)}


def balanced_paren_span(text, open_paren_index):
    """Given the index of an opening '(', return the index just past its matching ')'."""
    depth = 0
    i = open_paren_index
    in_str = in_chr = False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == '\\':
                i += 1
            elif c == '"':
                in_str = False
        elif in_chr:
            if c == '\\':
                i += 1
            elif c == "'":
                in_chr = False
        elif c == '"':
            in_str = True
        elif c == "'":
            in_chr = True
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced parens")


def find_conditions(text):
    """Yields (keyword, cond_start, cond_end) for if(...)/while(...)/for(;COND;)."""
    for m in re.finditer(r'\b(if|while)\s*\(', text):
        open_idx = m.end() - 1
        close_idx = balanced_paren_span(text, open_idx) - 1
        yield m.group(1), open_idx + 1, close_idx
    for m in re.finditer(r'\bfor\s*\(', text):
        open_idx = m.end() - 1
        close_idx = balanced_paren_span(text, open_idx) - 1
        inner = text[open_idx + 1:close_idx]
        parts = inner.split(';')
        if len(parts) == 3:
            cond_offset_in_inner = len(parts[0]) + 1  # past the first ';'
            cond_start = open_idx + 1 + cond_offset_in_inner
            cond_end = cond_start + len(parts[1])
            yield 'for', cond_start, cond_end


def load_covered_lines(target_files=None, build_dir=None):
    """Returns {relative_file_path: set(covered line numbers)} from an existing
    coverage build's .gcda data (not rebuilt here)."""
    filter_args = []
    for f in (target_files if target_files is not None else TARGET_FILES):
        filter_args += ["--filter", f".*/{re.escape(f)}"]
    proc = subprocess.run(
        [str(GCOVR), "--root", str(REPO_ROOT), *filter_args, "--json", "-", "."],
        cwd=(build_dir if build_dir is not None else COVERAGE_BUILD_DIR),
        capture_output=True, text=True, check=True,
    )
    data = json.loads(proc.stdout)
    covered = {}
    for file_entry in data["files"]:
        lines = {ln["line_number"] for ln in file_entry["lines"] if ln["count"] > 0}
        covered[file_entry["file"]] = lines
    return covered


def generate_structural_mutants(covered_lines, target_files=None):
    mutants = []
    for rel_path in (target_files if target_files is not None else TARGET_FILES):
        abs_path = REPO_ROOT / rel_path
        text = abs_path.read_text()
        this_file_covered = covered_lines.get(rel_path, set())
        this_file_excluded = excluded_line_numbers(text)

        for keyword, cond_start, cond_end in find_conditions(text):
            if line_of(text, cond_start) not in this_file_covered:
                continue
            if line_of(text, cond_start) in this_file_excluded:
                continue
            cond_text = text[cond_start:cond_end]

            if keyword == 'if':
                mutated = text[:cond_start] + "!(" + cond_text + ")" + text[cond_end:]
                mutants.append({
                    "file": rel_path, "line": line_of(text, cond_start),
                    "operator": "ConditionNegation",
                    "description": f"if ({cond_text.strip()}) -> if (!({cond_text.strip()}))",
                    "mutated_text": mutated,
                })

            for m in REL_OP_PATTERN.finditer(cond_text):
                op = m.group(0)
                replacement = REL_OP_SWAP[op]
                abs_op_start = cond_start + m.start()
                abs_op_end = cond_start + m.end()
                mutated = text[:abs_op_start] + replacement + text[abs_op_end:]
                mutants.append({
                    "file": rel_path, "line": line_of(text, abs_op_start),
                    "operator": "RelationalOperatorSwap",
                    "description": f"'{op}' -> '{replacement}' in: {cond_text.strip()}",
                    "mutated_text": mutated,
                })

        for pattern, flip_to in ((RETURN_TRUE, "return false;"), (RETURN_FALSE, "return true;")):
            for m in pattern.finditer(text):
                if line_of(text, m.start()) not in this_file_covered:
                    continue
                if line_of(text, m.start()) in this_file_excluded:
                    continue
                mutated = text[:m.start()] + flip_to + text[m.end():]
                mutants.append({
                    "file": rel_path, "line": line_of(text, m.start()),
                    "operator": "BooleanLiteralFlip",
                    "description": f"'{m.group(0)}' -> '{flip_to}'",
                    "mutated_text": mutated,
                })
    return mutants


def generate_constant_mutants(constant_mutants=None):
    mutants = []
    for rel_path, find, replace, description in (constant_mutants if constant_mutants is not None else CONSTANT_MUTANTS):
        abs_path = REPO_ROOT / rel_path
        text = abs_path.read_text()
        count = text.count(find)
        if count != 1:
            print(f"SKIP constant mutant ({description}): expected exactly 1 occurrence "
                  f"of {find!r} in {rel_path}, found {count}", file=sys.stderr)
            continue
        if line_of(text, text.index(find)) in excluded_line_numbers(text):
            print(f"SKIP constant mutant ({description}): target line is an excluded "
                  f"reflection macro line in {rel_path}", file=sys.stderr)
            continue
        mutated = text.replace(find, replace)
        mutants.append({
            "file": rel_path, "line": line_of(text, text.index(find)),
            "operator": "ConstantPerturbation",
            "description": description,
            "mutated_text": mutated,
        })
    return mutants


def build():
    r = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--target",
         "Google_Tests_run", "QA_Harness_Tests", "QA_Oracle_Tests", "Agent_Tests",
         "-j", str(os.cpu_count())],
        capture_output=True, text=True, timeout=300,
    )
    return r.returncode == 0, r.stdout + r.stderr


def run_tests():
    for binary in BINARIES:
        try:
            r = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
        except subprocess.TimeoutExpired:
            return "timeout", str(binary)
        if r.returncode != 0:
            return "killed", str(binary)
    return "survived", None


def verify_green_baseline():
    """Build and run the whole suite with NO mutation applied, and abort unless
    every test binary passes.

    A mutant is scored 'killed' whenever any binary exits non-zero (see
    run_tests). So if the *unmutated* baseline is already red -- even for a
    reason unrelated to the mutated code -- every single mutant is exited
    non-zero for that same pre-existing reason and counted as killed, producing
    a meaningless 100% score. This gate makes that failure mode loud instead of
    silent."""
    print("Verifying the unmutated baseline builds and is all-green...")
    ok, log = build()
    if not ok:
        print("ABORT: the project does not build without any mutation applied. "
              "Fix the build before running mutation testing.\n" + log[-2000:],
              file=sys.stderr)
        sys.exit(1)
    status, where = run_tests()
    if status != "survived":
        print(f"ABORT: the unmutated test suite is not green -- '{where}' reported "
              f"'{status}'. Mutation scores are only meaningful against an "
              f"all-passing baseline; a red baseline marks every mutant 'killed' "
              f"and reports a false 100%. Fix the failing/hanging tests and re-run.",
              file=sys.stderr)
        sys.exit(1)
    print("Baseline is green -- all test binaries pass unmutated.\n")


def main():
    print("Loading covered-line data from the step-3 coverage build...")
    covered = load_covered_lines()
    for f, lines in covered.items():
        print(f"  {f}: {len(lines)} covered lines")

    mutants = generate_structural_mutants(covered) + generate_constant_mutants()
    print(f"\n{len(mutants)} candidate mutants generated.\n")

    # A red baseline would score every mutant as a false 'killed' -- refuse to
    # run until the unmutated suite is green.
    verify_green_baseline()

    backups = {rel: (REPO_ROOT / rel).read_text() for rel in TARGET_FILES}
    # ConstantPerturbation may touch files outside TARGET_FILES (Globals.h,
    # Rigidbody2D.h, Physics2DSystem.h) -- back those up too.
    for rel_path, *_ in CONSTANT_MUTANTS:
        if rel_path not in backups:
            backups[rel_path] = (REPO_ROOT / rel_path).read_text()

    results = []
    try:
        for i, mut in enumerate(mutants, 1):
            abs_path = REPO_ROOT / mut["file"]
            original = backups[mut["file"]]
            print(f"[{i}/{len(mutants)}] {mut['file']}:{mut['line']} {mut['operator']} "
                  f"- {mut['description']}", end=" ... ", flush=True)
            t0 = time.time()
            abs_path.write_text(mut["mutated_text"])
            try:
                ok, log = build()
                if not ok:
                    status = "stillborn"
                else:
                    status, _ = run_tests()
            finally:
                abs_path.write_text(original)
            dt = time.time() - t0
            print(f"{status} ({dt:.1f}s)")
            results.append({**{k: v for k, v in mut.items() if k != "mutated_text"},
                             "status": status})
    finally:
        print("\nRestoring all target files from backup and verifying...")
        mismatches = []
        for rel_path, original in backups.items():
            abs_path = REPO_ROOT / rel_path
            abs_path.write_text(original)
            if abs_path.read_text() != original:
                mismatches.append(rel_path)
        if mismatches:
            print(f"ERROR: failed to cleanly restore: {mismatches}", file=sys.stderr)
            sys.exit(1)
        print("All target files verified restored to their original content.")

    killed = sum(1 for r in results if r["status"] in ("killed", "timeout"))
    survived = sum(1 for r in results if r["status"] == "survived")
    stillborn = sum(1 for r in results if r["status"] == "stillborn")
    denom = killed + survived
    score = killed / denom if denom else float("nan")

    print(f"\n=== Mutation score: {killed}/{denom} = {score:.1%} "
          f"({stillborn} stillborn/excluded) ===")
    survivors = [r for r in results if r["status"] == "survived"]
    if survivors:
        print(f"\n{len(survivors)} surviving mutants (tests didn't notice these):")
        for r in survivors:
            print(f"  {r['file']}:{r['line']} [{r['operator']}] {r['description']}")

    RESULTS_PATH.write_text(json.dumps({
        "killed": killed, "survived": survived, "stillborn": stillborn,
        "score": score, "results": results,
    }, indent=2))
    print(f"\nFull results written to {RESULTS_PATH}")


if __name__ == "__main__":
    main()