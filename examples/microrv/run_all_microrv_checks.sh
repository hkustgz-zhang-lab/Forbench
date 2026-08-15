#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
repo_root="$(cd "$script_dir/../.." >/dev/null 2>&1 && pwd)"
bug_dir="$repo_root/design/asynctest/buggy-microrv32"
correct_btor="$repo_root/design/asynctest/microriscv/rv32.btor2"

echo "== Checking Original Design (expected clean) =="
python3 "$script_dir/check_microrv.py" "$correct_btor" --expect clean

for btor in "$bug_dir"/RiscV32Core-bug*.btor2; do
  echo "== Checking $(basename "$btor") (expected bug) =="
  python3 "$script_dir/check_microrv.py" "$btor" --expect bug
  mv dut_cex.vcd $(basename "$btor").vcd
  mv refdesign_cex.vcd $(basename "$btor")_ref.vcd
done
