#!/usr/bin/bash
set -uo pipefail

# --- config ---
BIN="./build/debug"
FIXTURES_DIR="./test/original"
EXPECTED_DIR="./test/expected"
ACTUAL_DIR="./test/actual"
DIFF_DIR="./test/diff"

mkdir -p "$ACTUAL_DIR" "$DIFF_DIR"
rm -f "$ACTUAL_DIR"/*.csv "$DIFF_DIR"/*

# --- colors ---
if [ -t 1 ]; then
  GREEN='\033[0;32m'
  RED='\033[0;31m'
  RESET='\033[0m'
else
  GREEN=''
  RED=''
  RESET=''
fi

# --- counters ---
pass_count=0
fail_count=0
total_count=0

echo "Running tests..."
echo "-----------------------------------"

#
"$BIN"

for expected_file in "$EXPECTED_DIR"/*.csv; do
  name=$(basename "$expected_file")
  actual_file="$ACTUAL_DIR/$name"
  diff_file="$DIFF_DIR/${name%.csv}.diff"

  total_count=$((total_count + 1))

  if [ ! -f "$actual_file" ]; then
    echo -e "${RED}FAIL${RESET}: ${name} (no actual output produced)"
    fail_count=$((fail_count + 1))
    continue
  fi

  if diff -q <(sort "$expected_file") <(sort "$actual_file") >/dev/null; then
    echo -e "${GREEN}PASS${RESET}: $name"
    pass_count=$((pass_count + 1))
    rm -f "$diff_file"
  else
    echo -e "${RED}FAIL${RESET}: $name"
    diff <(sort "$expected_file") <(sort "$actual_file") >"$diff_file"
    fail_count=$((fail_count + 1))
  fi
done

echo "-----------------------------------"
echo "Results: $pass_count/$total_count passed"

if [ "$fail_count" -gt 0 ]; then
  exit 1
else
  exit 0
fi
