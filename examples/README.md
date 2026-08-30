# Forbench Examples

This directory contains runnable examples that demonstrate Forbench workflows
outside of the internal `test/python` smoke-test layout.

## MicroRV

`microrv/check_microrv.py` demonstrates coroutine-based symbolic checking of a
small RISC-V core against a BTOR2 reference model loaded through `RefDesign`.

Run it from the repository root with:

```bash
python3 examples/microrv/check_microrv.py
```

The same script can check any MicroRV BTOR2 design:

```bash
python3 examples/microrv/check_microrv.py \
  design/asynctest/buggy-microrv32/RiscV32Core-bug0.btor2 --expect bug
```

`microrv/run_buggy_microrv_checks.sh` checks the original MicroRV design plus
all ten generated buggy variants under `design/asynctest/buggy-microrv32`.

## AES

`aes/check_pipeline_vs_comb.py` compares the pipelined AES-128 BTOR2 design
against the pure combinational AES encryption BTOR2 design through `RefDesign`.

The full symbolic check is expected to be expensive:

```bash
python3 examples/aes/check_pipeline_vs_comb.py
```

## Pair Swapper

The pair-swapper examples check a streaming byte transform:

```text
input:  d0 d1 d2 d3
output: d1 d0 d3 d2
```

`pair_swapper/check_pair_swapper.py` is the randomized-send symbolic testbench.
It uses two overlapping async coroutines: one coroutine decides symbolically
whether to send on each cycle until all requests are sent, and the other waits
for valid outputs and checks the swapped order.

```bash
python3 examples/pair_swapper/check_pair_swapper.py
```

`pair_swapper/check_pair_swapper_simple.py` is the deterministic-send version.
It sends `N` symbolic bytes in consecutive cycles, with no random or symbolic
choice about whether to send, then checks the same swapped output order.

```bash
python3 examples/pair_swapper/check_pair_swapper_simple.py
```

`pair_swapper/check_pair_swapper_monitor.py` is the passive monitor-style
testbench. It does not explicitly drive `in_valid` or `in_data`; it observes the
symbolic input stream and checks outputs against branch-local Python reference
state.

```bash
python3 examples/pair_swapper/check_pair_swapper_monitor.py
```
