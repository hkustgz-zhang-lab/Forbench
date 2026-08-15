# AES Examples

This directory contains symbolic checks for AES designs under
`design/smtsweep-test/aes`.

## Pipeline Versus Combinational

`check_pipeline_vs_comb.py` compares the sequential pipelined AES-128 design
against the pure combinational AES encryption design.

Default DUT:

```text
design/smtsweep-test/aes/AES-Pipeline/aes_128.btor2
```

Default reference:

```text
design/smtsweep-test/aes/AES-Comb/AES_Encrypt_ite.btor2
```

The testbench drives one symbolic plaintext/key pair into the pipeline, keeps
the inputs stable, waits for the default 20-cycle pipeline latency, then checks
that `aes_128.out` equals the combinational `AES_Encrypt.out`.

Run the full symbolic equivalence check:

```bash
python3 examples/aes/check_pipeline_vs_comb.py
```

The full check is expected to be expensive because it compares a full AES
pipeline output against a full AES combinational reference for symbolic
128-bit plaintext and key inputs.

## Python Reference

`aes128_ref.py` is a `cryptography`-backed AES-128 block encryption reference
for concrete known-answer checks.

Install the Python dependency if needed:

```bash
python3 -m pip install cryptography
```

Run its built-in test vectors:

```bash
python3 examples/aes/aes128_ref.py --self-test
```

Encrypt one block:

```bash
python3 examples/aes/aes128_ref.py \
  00112233445566778899aabbccddeeff \
  000102030405060708090a0b0c0d0e0f
```
