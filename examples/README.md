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
