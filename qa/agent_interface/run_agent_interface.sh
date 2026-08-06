#!/usr/bin/env bash
# Thin CLI wrapper over submit_test.py. Run from WSL/Linux.
#
# First time (or whenever the hand-written suite changes):
#   ./run_agent_interface.sh --capture-baseline
#
# Submitting a test:
#   ./run_agent_interface.sh --file MyTest.cpp --content-file /path/to/MyTest.cpp [--with-mutation]

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python3 "$SCRIPT_DIR/submit_test.py" "$@"