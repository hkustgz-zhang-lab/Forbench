# Buggy MicroRV32 Designs

This directory contains ten deliberately buggy variants of
`design/asynctest/microriscv/RiscV32Core.v`.

## Convert To BTOR2

Run from the repository root:

```bash
design/asynctest/buggy-microrv32/convert_buggy_to_btor2.sh
```

The script applies the same Yosys flow as `design/asynctest/microriscv/tobtor.ys`
to every `RiscV32Core-bug*.v` file and writes matching `RiscV32Core-bug*.btor2`
files.

## Check With Forbench

Run the symbolic checker for the original MicroRV design and all buggy BTOR2
designs:

```bash
examples/microrv/run_buggy_microrv_checks.sh
```

Or check one design:

```bash
python3 examples/microrv/check_microrv.py \
  design/asynctest/buggy-microrv32/RiscV32Core-bug0.btor2 --expect bug
```

The checker drives one symbolic instruction into the DUT, waits for RVFI
retirement, and compares the DUT RVFI outputs against the RISC-V formal BTOR2
reference model through `RefDesign`.

## Inserted Bugs

- `RiscV32Core-bug0.v`: relaxes SLLI legality checking from full `funct7 == 0`
  to only `funct7[6:1] == 0`, so reserved encodings with bit 0 set can decode
  as valid.
- `RiscV32Core-bug1.v`: relaxes SRLI legality checking from full `funct7 == 0`
  to only `funct7[6:1] == 0`, so reserved encodings with bit 0 set can decode
  as valid.
- `RiscV32Core-bug2.v`: relaxes SRAI legality checking from full
  `funct7 == 7'h20` to only `funct7[6:1] == 6'h10`, so reserved encodings with
  bit 0 set can decode as valid.
- `RiscV32Core-bug3.v`: corrupts the ADD result by forcing bit 0 of the ALU
  addition output to zero.
- `RiscV32Core-bug4.v`: corrupts the SUB result by forcing bit 31 of the ALU
  subtraction output to zero.
- `RiscV32Core-bug5.v`: corrupts JAL control flow and exception handling by
  unconditionally enabling PC/register writes, selecting increment-PC instead
  of the JAL target, and skipping the misaligned-jump trap transition.
- `RiscV32Core-bug6.v`: implements BNE using the equality predicate, making BNE
  branch decisions behave like BEQ.
- `RiscV32Core-bug7.v`: corrupts LBU data extraction by reading byte bits
  `[31:24]` instead of `[7:0]`.
- `RiscV32Core-bug8.v`: corrupts LB sign extension by zero-extending the loaded
  byte instead of sign-extending it.
- `RiscV32Core-bug9.v`: adds an unsigned halfword load path for `funct3 == 010`,
  corrupting LW/load data behavior by selecting zero-extended 16-bit data.

The original design and all ten generated BTOR2 variants were checked with
`examples/microrv/run_buggy_microrv_checks.sh`. The original reported
`NO BUG DETECTED`, and every buggy variant reported `BUG DETECTED`.
