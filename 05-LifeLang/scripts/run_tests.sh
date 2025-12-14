#!/usr/bin/env bash

BIN=./lifec

fail=0
for t in tests/*.lf; do
  echo "[TEST] $t"
  $BIN "$t" --no-run >/dev/null
  $BIN "$t" --no-sdl >/dev/null
  rc=$?
  if [[ $rc -ne 0 ]]; then
    echo "  FAIL (exit=$rc)"
    fail=1
  else
    echo "  OK"
  fi
done

if [[ $fail -ne 0 ]]; then
  echo "Some tests failed"
  exit 1
fi

echo "All tests OK"
