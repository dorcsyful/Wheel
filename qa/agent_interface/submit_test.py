#!/usr/bin/env python3
"""
The tool interface step 5 asks for: something that reads a source file
(that's just a normal file read -- nothing custom needed), writes a test
against the oracle API, and runs the suite, returning one structured
verdict:

    {
      "build": {"success": bool, "log": str (only on failure)},
      "tests": {"total": int, "passed": int, "failed": int,
                "failures": [{"suite": str, "name": str, "message": str}]},
      "coverage_delta": {"newly_covered_lines": int, "by_file": {file: count}},
      "mutation": null | {"new_mutants": int, "killed": int, "survived": int,
                           "stillborn": int, "surviving": [...]}
    }

Mutation has two scopes. --with-mutation is delta-scoped (only lines this test
newly covered vs baseline). --full-mutation <files> mutates every covered line
of the named files regardless of baseline, so a test can strengthen assertions
on code the hand-written suite already covers -- which the delta scope reports
as zero mutants. Full scope adds "scope"/"files" to the mutation object, and
refuses (returns an "error") if the unmutated suite isn't green.

This is what steps 6-8 (seeding, the closed loop, crash/bug isolation) call.
Safety boundary: the only thing a submission can create or modify is one file
under qa/agent_tests/ -- never wheel/, qa/harness/, or qa/oracles/. That's
enforced in write_test(), not left to agent good behavior.

Coverage/mutation scope matches the QA agent's brief: math, collision,
physics, constraint solver, and the QA library itself -- not the renderer,
not ECS internals.

Run from WSL/Linux (needs gcov). See run_agent_interface.sh for the CLI.
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "mutation"))
import mutate  # noqa: E402

REPO_ROOT = Path("/mnt/c/Users/dorcs/Desktop/Wheel")
AGENT_TESTS_DIR = REPO_ROOT / "qa/agent_tests"
BUILD_DIR = Path(os.path.expanduser("~/wheel-agent-build"))
BASELINE_PATH = Path(__file__).parent / "baseline_coverage.json"
GCOVR = Path(os.path.expanduser("~/gcovr-venv/bin/gcovr"))

ALL_TARGETS = ["Google_Tests_run", "QA_Harness_Tests", "QA_Oracle_Tests", "Agent_Tests"]
BINARY_PATHS = {
    "Google_Tests_run": "qa/Google_tests/Google_Tests_run",
    "QA_Harness_Tests": "qa/harness_tests/QA_Harness_Tests",
    "QA_Oracle_Tests": "qa/oracle_tests/QA_Oracle_Tests",
    "Agent_Tests": "qa/agent_tests/Agent_Tests",
}

COVERAGE_SCOPE_FILTERS = [
    r".*/wheel/math/.*", r".*/wheel/collision/.*", r".*/wheel/physics/.*",
    r".*/wheel/common/.*", r".*/qa/harness/.*", r".*/qa/oracles/.*",
]


def _validate_filename(filename):
    """No path traversal, no absolute paths, no touching another directory
    or the placeholder -- a bare *.cpp name inside qa/agent_tests/ is the
    only thing this will ever write."""
    if Path(filename).name != filename or not filename.endswith(".cpp"):
        raise ValueError(f"filename must be a bare *.cpp name, got {filename!r}")
    if filename == "_Placeholder.cpp":
        raise ValueError("refusing to overwrite the placeholder file")
    return AGENT_TESTS_DIR / filename


def write_test(filename, content, overwrite=False):
    path = _validate_filename(filename)
    if path.exists() and not overwrite:
        raise FileExistsError(f"{filename} already exists (pass overwrite=True to replace it)")
    path.write_text(content)
    return path


def configure_if_needed():
    if not (BUILD_DIR / "CMakeCache.txt").exists():
        subprocess.run(
            ["cmake", "-S", str(REPO_ROOT), "-B", str(BUILD_DIR),
             "-DCMAKE_BUILD_TYPE=Debug",
             "-DCMAKE_CXX_FLAGS=--coverage -O0 -g",
             "-DCMAKE_C_FLAGS=--coverage -O0 -g",
             "-DCMAKE_EXE_LINKER_FLAGS=--coverage",
             "-DCMAKE_SHARED_LINKER_FLAGS=--coverage"],
            check=True, capture_output=True, text=True,
        )


def build():
    r = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--target", *ALL_TARGETS, "-j", str(os.cpu_count())],
        capture_output=True, text=True, timeout=300,
    )
    return r.returncode == 0, r.stdout + r.stderr


def run_tests_structured():
    """Runs every binary with gtest's own JSON reporter and merges per-test
    results. A binary that crashes outright (not just a failed EXPECT) exits
    nonzero without producing a report -- surfaced as a synthetic failure so
    the agent sees "this crashed" instead of the tool silently dropping that
    suite's results."""
    total = passed = failed = 0
    failures = []
    for target in ALL_TARGETS:
        binary = BUILD_DIR / BINARY_PATHS[target]
        report_path = BUILD_DIR / f"{target}.report.json"
        report_path.unlink(missing_ok=True)
        proc = subprocess.run([str(binary), f"--gtest_output=json:{report_path}"],
                               capture_output=True, text=True, timeout=60)
        if not report_path.exists():
            total += 1
            failed += 1
            failures.append({"suite": target, "name": "(process)",
                              "message": f"binary exited {proc.returncode} without producing a "
                                         f"report (crash/timeout). stderr tail: {proc.stderr[-2000:]}"})
            continue
        data = json.loads(report_path.read_text())
        for suite in data.get("testsuites", []):
            for case in suite.get("testsuite", []):
                total += 1
                case_failures = case.get("failures")
                if case_failures:
                    failed += 1
                    msg = "; ".join(f.get("failure", "") for f in case_failures)
                    failures.append({"suite": suite["name"], "name": case["name"], "message": msg})
                else:
                    passed += 1
    return {"total": total, "passed": passed, "failed": failed, "failures": failures}


def _covered_lines(build_dir):
    filter_args = []
    for pattern in COVERAGE_SCOPE_FILTERS:
        filter_args += ["--filter", pattern]
    proc = subprocess.run(
        [str(GCOVR), "--root", str(REPO_ROOT), *filter_args, "--json", "-", "."],
        cwd=build_dir, capture_output=True, text=True, check=True,
    )
    data = json.loads(proc.stdout)
    covered = {}
    for file_entry in data["files"]:
        covered[file_entry["file"]] = {ln["line_number"] for ln in file_entry["lines"] if ln["count"] > 0}
    return covered


def capture_baseline():
    """Coverage from the hand-written suite alone (qa/agent_tests reduced to
    just its placeholder). Every later submit_test() coverage_delta is
    measured against this. Re-run whenever the hand-written suite changes."""
    configure_if_needed()
    ok, log = build()
    if not ok:
        raise RuntimeError(f"baseline build failed:\n{log}")
    for target in ALL_TARGETS:
        try:
            subprocess.run([str(BUILD_DIR / BINARY_PATHS[target])], capture_output=True, timeout=60)
        except subprocess.TimeoutExpired:
            pass
    covered = _covered_lines(BUILD_DIR)
    BASELINE_PATH.write_text(json.dumps({f: sorted(lines) for f, lines in covered.items()}, indent=2))
    return covered


def coverage_delta(current_covered):
    if not BASELINE_PATH.exists():
        raise RuntimeError("no baseline captured yet -- run "
                            "`run_agent_interface.sh --capture-baseline` first")
    baseline = {f: set(lines) for f, lines in json.loads(BASELINE_PATH.read_text()).items()}
    delta = {}
    for f, lines in current_covered.items():
        new_lines = lines - baseline.get(f, set())
        if new_lines:
            delta[f] = sorted(new_lines)
    return delta


def _run_mutation(covered_by_file):
    """Run structural mutants over the given {file: covered line numbers} map
    and report kill/survival. Callers choose the scope:

    - delta scope (run_mutation_on_delta): only the lines this submission newly
      covered vs baseline. Cheap, and correct when the point is "did *this*
      test add adequacy" -- a mutant on an untouched line couldn't have changed
      status because of one submission.
    - full scope (run_mutation_full): every covered line of an explicitly named
      set of files, regardless of baseline. This is what lets a submission
      strengthen assertions on code the hand-written suite *already* covers --
      the delta scope is blind to that by construction (newly covered == 0), so
      full-sweep survivors on already-covered lines can only be attacked here."""
    if not covered_by_file:
        return {"new_mutants": 0, "killed": 0, "survived": 0, "stillborn": 0, "surviving": []}

    target_files = list(covered_by_file.keys())
    covered_as_sets = {f: set(lines) for f, lines in covered_by_file.items()}
    mutants = mutate.generate_structural_mutants(covered_as_sets, target_files=target_files)

    killed = survived = stillborn = 0
    surviving = []
    backups = {f: (REPO_ROOT / f).read_text() for f in target_files}
    try:
        for mut in mutants:
            path = REPO_ROOT / mut["file"]
            path.write_text(mut["mutated_text"])
            try:
                ok, _ = build()
                if not ok:
                    status = "stillborn"
                else:
                    status = "survived"
                    for target in ALL_TARGETS:
                        binary = BUILD_DIR / BINARY_PATHS[target]
                        try:
                            r = subprocess.run([str(binary)], capture_output=True, timeout=30)
                        except subprocess.TimeoutExpired:
                            status = "killed"
                            break
                        if r.returncode != 0:
                            status = "killed"
                            break
            finally:
                path.write_text(backups[mut["file"]])

            if status == "killed":
                killed += 1
            elif status == "stillborn":
                stillborn += 1
            else:
                survived += 1
                surviving.append({k: v for k, v in mut.items() if k != "mutated_text"})
    finally:
        for f, original in backups.items():
            (REPO_ROOT / f).write_text(original)

    return {"new_mutants": len(mutants), "killed": killed, "survived": survived,
            "stillborn": stillborn, "surviving": surviving}


def run_mutation_on_delta(delta):
    """Delta scope: mutate only the lines newly covered since baseline."""
    return _run_mutation(delta)


def run_mutation_full(current_covered, files):
    """Full scope: mutate every covered line of each named file, ignoring the
    baseline. Use when a submission strengthens assertions on already-covered
    code (the delta scope reports zero mutants for that, by construction).

    `files` are repo-relative paths as they appear in coverage output, e.g.
    "wheel/collision/BoxBoxCollision2D.cpp". A named file with no coverage is
    reported so a typo doesn't masquerade as "nothing to kill." """
    covered_by_file = {}
    uncovered = []
    for f in files:
        lines = current_covered.get(f)
        if lines:
            covered_by_file[f] = sorted(lines)
        else:
            uncovered.append(f)
    result = _run_mutation(covered_by_file)
    result["scope"] = "full"
    result["files"] = list(covered_by_file.keys())
    if uncovered:
        result["files_with_no_coverage"] = uncovered
    return result


def submit_test(filename, content, overwrite=False, run_mutation=False,
                full_mutation_files=None):
    write_test(filename, content, overwrite=overwrite)
    configure_if_needed()

    build_ok, build_log = build()
    result = {"build": {"success": build_ok}}
    if not build_ok:
        result["build"]["log"] = build_log[-4000:]
        result["tests"] = None
        result["coverage_delta"] = None
        result["mutation"] = None
        return result

    result["tests"] = run_tests_structured()

    current_covered = _covered_lines(BUILD_DIR)
    delta = coverage_delta(current_covered)
    result["coverage_delta"] = {
        "newly_covered_lines": sum(len(v) for v in delta.values()),
        "by_file": {f: len(v) for f, v in delta.items()},
    }

    if full_mutation_files:
        # A mutant is scored 'killed' when a test binary exits non-zero, so a
        # baseline that is already red would false-kill every mutant. Refuse
        # rather than report a meaningless score (same lesson as the mutation
        # driver's green-baseline gate).
        if result["tests"]["failed"] > 0:
            result["mutation"] = {
                "error": "unmutated suite is not green (%d failing test(s)); refusing "
                         "full-scope mutation, which would false-kill every mutant. "
                         "Fix the failures and resubmit." % result["tests"]["failed"],
            }
        else:
            result["mutation"] = run_mutation_full(current_covered, full_mutation_files)
    elif run_mutation:
        result["mutation"] = run_mutation_on_delta(delta)
    else:
        result["mutation"] = None
    return result


def main():
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture-baseline", action="store_true")
    parser.add_argument("--file", help="bare filename inside qa/agent_tests/, e.g. MyTest.cpp")
    parser.add_argument("--content-file", help="path to a file whose contents become the test source")
    parser.add_argument("--overwrite", action="store_true")
    parser.add_argument("--with-mutation", action="store_true",
                        help="delta-scoped mutation: only lines this test newly covers vs baseline")
    parser.add_argument("--full-mutation", metavar="FILES",
                        help="comma-separated repo-relative files (e.g. "
                             "wheel/collision/BoxBoxCollision2D.cpp) to mutate in FULL -- every "
                             "covered line, ignoring baseline. Use to strengthen assertions on "
                             "already-covered code, which --with-mutation cannot score.")
    args = parser.parse_args()

    if args.capture_baseline:
        covered = capture_baseline()
        print(json.dumps({"baseline_captured": True,
                           "files": {f: len(lines) for f, lines in covered.items()}}, indent=2))
        return

    if not args.file or not args.content_file:
        parser.error("--file and --content-file are required unless --capture-baseline is given")

    full_files = [f.strip() for f in args.full_mutation.split(",")] if args.full_mutation else None
    content = Path(args.content_file).read_text()
    result = submit_test(args.file, content, overwrite=args.overwrite,
                         run_mutation=args.with_mutation, full_mutation_files=full_files)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()