# MicroRV Example

This example checks one symbolic instruction execution of the MicroRV design
against the RISC-V formal instruction reference model.

Run from the repository root:

```bash
python3 examples/microrv/check_microrv.py
```

## Buggy MicroRV Variants

Convert the buggy Verilog variants to BTOR2:

```bash
design/asynctest/buggy-microrv32/convert_buggy_to_btor2.sh
```

Check the original design and all generated buggy variants:

```bash
examples/microrv/run_buggy_microrv_checks.sh
```

Check a single variant:

```bash
python3 examples/microrv/check_microrv.py \
  design/asynctest/buggy-microrv32/RiscV32Core-bug0.btor2 --expect bug
```
