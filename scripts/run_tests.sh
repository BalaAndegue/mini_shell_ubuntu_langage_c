#!/usr/bin/env bash
# Integration test runner for minishell
# Usage: bash scripts/run_tests.sh [test_name]

set -euo pipefail

SHELL_BIN="./build/bin/minishell"
TEST_DIR="tests/integration"
PASS=0
FAIL=0
RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

if [ ! -x "$SHELL_BIN" ]; then
    echo "Binary not found — run 'make' first"
    exit 1
fi

run_test() {
    local name="$1"
    local input_file="$TEST_DIR/$name.input"
    local expected_file="$TEST_DIR/$name.expected"

    if [ ! -f "$input_file" ] || [ ! -f "$expected_file" ]; then
        echo "SKIP  $name (missing .input or .expected)"
        return
    fi

    actual=$("$SHELL_BIN" < "$input_file" 2>/dev/null || true)
    expected=$(cat "$expected_file")

    if [ "$actual" = "$expected" ]; then
        echo -e "${GREEN}PASS${RESET}  $name"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${RESET}  $name"
        echo "     expected: $(echo "$expected" | head -3)"
        echo "     actual  : $(echo "$actual"   | head -3)"
        FAIL=$((FAIL + 1))
    fi
}

# Run a single test if specified, otherwise run all
if [ "${1-}" != "" ]; then
    run_test "$1"
else
    for f in "$TEST_DIR"/*.input; do
        [ -f "$f" ] || continue
        run_test "$(basename "$f" .input)"
    done
fi

echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
