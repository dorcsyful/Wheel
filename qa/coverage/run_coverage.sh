#!/usr/bin/env bash
# Branch-coverage report for the deterministic sim core (math, collision,
# physics, constraint solver) plus the QA harness/oracle library, run against
# the full hand-written test suite (Google_Tests_run, QA_Harness_Tests,
# QA_Oracle_Tests).
#
# Requires GCC + gcov (already present on any Linux/RPi toolchain for this
# project) and gcovr (pip install --user gcovr, or use the venv this script
# creates on first run). Run from WSL/Linux — MSVC doesn't produce gcov data.
#
# Usage: ./run_coverage.sh [build-dir]
#   build-dir defaults to ~/wheel-coverage-build, kept outside the repo since
#   it's fully regenerable and building on /mnt/c (9p) is slow.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${1:-$HOME/wheel-coverage-build}"
VENV_DIR="$HOME/gcovr-venv"
REPORT_DIR="$SCRIPT_DIR/report"

if [ ! -x "$VENV_DIR/bin/gcovr" ]; then
    echo "Setting up gcovr in $VENV_DIR (one-time)..."
    python3 -m venv "$VENV_DIR"
    "$VENV_DIR/bin/pip" install -q gcovr
fi

echo "Configuring coverage build in $BUILD_DIR..."
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
    -DCMAKE_C_FLAGS="--coverage -O0 -g" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    -DCMAKE_SHARED_LINKER_FLAGS="--coverage" >/dev/null

echo "Building the hand-written test suite..."
cmake --build "$BUILD_DIR" --target Google_Tests_run QA_Harness_Tests QA_Oracle_Tests -j"$(nproc)"

echo "Running the suite to generate coverage data..."
"$BUILD_DIR/qa/Google_tests/Google_Tests_run"
"$BUILD_DIR/qa/harness_tests/QA_Harness_Tests"
"$BUILD_DIR/qa/oracle_tests/QA_Oracle_Tests"

echo "Generating report..."
mkdir -p "$REPORT_DIR"
cd "$BUILD_DIR"
"$VENV_DIR/bin/gcovr" --root "$REPO_ROOT" \
    --filter '.*/wheel/math/.*' \
    --filter '.*/wheel/collision/.*' \
    --filter '.*/wheel/physics/.*' \
    --filter '.*/wheel/common/.*' \
    --filter '.*/qa/harness/.*' \
    --filter '.*/qa/oracles/.*' \
    --exclude-lines-by-pattern '\s*(REFLECT_BEGIN|FIELD|REFLECT_END)\s*\(.*' \
    --txt-metric branch --sort uncovered-percent --sort-reverse \
    --html-details "$REPORT_DIR/index.html" \
    .

echo ""
echo "HTML report: $REPORT_DIR/index.html"
echo "(wheel/core is deliberately excluded: it's ECS/ComponentPool template"
echo " boilerplate, out of scope per the QA agent's brief, and gcov double-"
echo " counts branches per template instantiation there, which distorts the"
echo " numbers badly.)"