#!/usr/bin/env bash

set -euo pipefail

root_dir="."
out_dir="$root_dir"

designs=(fifo)

for top in "${designs[@]}"; do
    yosys -q -p "
        read_verilog -formal $root_dir/$top.v
        prep -top $top
        flatten
        memory -nomap
        memory_nordff
        hierarchy -check
        setundef -undriven -init -expose
        sim -clock clk -reset rst -rstlen 1 -n 1 -w $top
        write_btor -s $out_dir/$top.btor2
    "
    echo "generated $out_dir/$top.btor2"
done
