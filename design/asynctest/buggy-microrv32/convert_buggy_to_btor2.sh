#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

for verilog in "$script_dir"/RiscV32Core-bug*.v; do
  stem="$(basename "$verilog" .v)"
  btor="$script_dir/${stem}.btor2"
  yosys_script="$(mktemp)"

  cat > "$yosys_script" <<YOSYS
read_verilog -formal $verilog
prep -top RiscV32Core
async2sync; dffunmap
flatten
memory -nordff
hierarchy -check
setundef -undriven -init -expose
sim -clock clk -reset reset -rstlen 10 -n 10 -w RiscV32Core
write_btor -s $btor
YOSYS

  echo "Converting $(basename "$verilog") -> $(basename "$btor")"
  yosys -q -s "$yosys_script"
  rm -f "$yosys_script"
done
