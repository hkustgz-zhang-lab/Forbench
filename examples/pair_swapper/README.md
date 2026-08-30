# Pair Swapper Example

This example checks a streaming byte pair-swapper:

```text
input:  d0 d1 d2 d3
output: d1 d0 d3 d2
```

The RTL is under:

```text
design/asynctest/pair-swapper/pair_swapper.v
```

If you want to regenerate the BTOR2 file:

```bash
cd design/asynctest/pair-swapper
yosys -q tobtor.ys
```

Run the coroutine testbench from the repository root:

```bash
python3 examples/pair_swapper/check_pair_swapper.py
```

Run the simple deterministic-send symbolic testbench:

```bash
python3 examples/pair_swapper/check_pair_swapper_simple.py
```

Run the passive monitor-style symbolic testbench:

```bash
python3 examples/pair_swapper/check_pair_swapper_monitor.py
```
