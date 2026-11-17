#!/bin/bash
# Spustí programy z tests/gen/*.ifj26, přeloží je překladačem a spustí v ic26int.
# Výstup se porovná se souborem *.out, případný očekávaný kód interpretu je v *.code, vstup v *.in.
# TODO: doplnit označení, že skript vznikl s pomocí AI (prompt).
COMPILER=${COMPILER:-./compiler}
INTERPRETER=${IC26INT:-./ic26int-windows-x86_64.exe}
pass=0
fail=0
tmp=$(mktemp -d -p .)
for src in tests/gen/*.ifj26; do
  base=${src%.ifj26}
  name=$(basename "$base")
  if ! "$COMPILER" < "$src" > "$tmp/prog.code" 2> /dev/null; then
    echo "FAIL (překlad): $name"
    fail=$((fail + 1))
    continue
  fi
  input=/dev/null
  [ -f "$base.in" ] && input="$base.in"
  "$INTERPRETER" "$tmp/prog.code" < "$input" 2> /dev/null | tr -d "\r" > "$tmp/prog.out"
  code=${PIPESTATUS[0]}
  expected_code=0
  [ -f "$base.code" ] && expected_code=$(cat "$base.code")
  if diff -q "$tmp/prog.out" "$base.out" > /dev/null && [ "$code" = "$expected_code" ]; then
    echo "PASS: $name"
    pass=$((pass + 1))
  else
    echo "FAIL: $name (kod $code, čekáno $expected_code)"
    fail=$((fail + 1))
  fi
done
rm -rf "$tmp"
echo "Pass $pass, Fail $fail"
[ "$fail" -eq 0 ]
