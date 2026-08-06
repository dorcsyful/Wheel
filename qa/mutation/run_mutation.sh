#!/usr/bin/env bash
# Mutation testing for the sim core (see mutate.py for the full explanation
# and the mutation operators). Run from WSL/Linux.
#
# Prerequisite: qa/coverage/run_coverage.sh must have been run at least once
# -- this script reads its .gcda data (in ~/wheel-coverage-build) to know
# which lines are covered, but does not rebuild or rerun it.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$HOME/wheel-mutation-build"
COVERAGE_BUILD_DIR="$HOME/wheel-coverage-build"

if [ ! -f "$COVERAGE_BUILD_DIR/qa/oracle_tests/QA_Oracle_Tests" ]; then
    echo "No coverage build found at $COVERAGE_BUILD_DIR." >&2
    echo "Run qa/coverage/run_coverage.sh first (mutation testing reads its coverage data)." >&2
    exit 1
fi

echo "Configuring plain (non-instrumented) build in $BUILD_DIR..."
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug >/dev/null

echo "Building once so per-mutant rebuilds are incremental..."
cmake --build "$BUILD_DIR" --target Google_Tests_run QA_Harness_Tests QA_Oracle_Tests -j"$(nproc)"

echo ""
python3 "$SCRIPT_DIR/mutate.py"