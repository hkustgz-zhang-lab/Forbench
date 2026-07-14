# Forbench: Symbolic Simulation Helps Make Your Testbench More Formal

Forbench makes hardware testbenches “more formal”: it preserves the procedural,
stimulus-driven workflow of RTL simulation while representing signals and state transitions symbolically
and using an SMT solver to check constraints and assertions.

The framework consumes a hardware design represented as a BTOR2 transition
system. A testbench may assign either concrete values or symbolic values to DUT
inputs, advance the design cycle by cycle, constrain the explored behaviors,
and prove or refute assertions over the resulting symbolic states.

## 📋 Table of Contents

- [📖 Introduction](#introduction)
  - [Prerequisite](#prerequisite)
  - [Setup](#setup)
- [🚀 Usage](#usage)
  - [Input design](#input-design)
  - [Active-stepping testbench](#active-stepping-testbench)
  - [Coroutine-based testbench](#coroutine-based-testbench)
- [🔗 Citation](#citation)

## 📖 Introduction

Conventional simulation is easy to deploy and offers an intuitive operational
model, but each run covers only one concrete execution trace. Formal property
verification can reason about all admissible inputs, but expressing procedural
transactions and variable-latency protocols as temporal properties often
requires substantial formal-method expertise.

Forbench provides an intermediate workflow. Verification intent remains in a
Python testbench, while values such as operands, control signals, and initial
states can be symbolic. Solver-backed assertion checking then covers all values
represented by the current symbolic state and path constraints.

The framework supports two complementary testbench styles:

- **Active stepping:** a single testbench explicitly drives inputs and calls
  `dut.step()` to advance the DUT. This style is useful for fixed-latency
  datapaths and cycle-accurate checks.
- **Coroutine based:** concurrent testbench tasks are registered with
  `@register_task` and synchronized using `wait_cycle`, `wait_cond`, and
  `wait_task`. When a `wait_cond` condition can be both true and false under
  the current symbolic constraints, Forbench forks the symbolic execution and
  explores both feasible timing behaviors.

Core capabilities include:

- concrete and symbolic input assignments;
- reset-based or unconstrained symbolic initialization;
- cycle-accurate symbolic state transitions;
- assumptions and path constraints;
- solver-backed assertion and embedded-property checking;
- symbolic branching for variable-latency and concurrent protocols;

### Prerequisite

    pip3 install toml
    sudo apt install build-essential cmake default-jre libgmp-dev libboost-all-dev

### SETUP

    ./contrib/setup-glog.sh
    ./contrib/setup-bison.sh
    ./contrib/setup-btor2tools.sh
    ./contrib/setup-smt-switch.sh
    ./contrib/setup-vexpparser.sh
    ./configure.sh --python
    cd build
    make

## 🚀 Usage

### Input design

Forbench loads a DUT from a BTOR2 file:

```python
from pywasim import Dut

dut = Dut("path/to/design.btor2")
```

The BTOR2 model must describe the DUT inputs, state variables, transition
relation, and optionally bad-state properties. BTOR2 files can, for example, be
generated from RTL using Yosys and its `write_btor` command.

The Python helpers automatically add the repository's `build/` directory to
the module search path. Run the supplied Python examples from
`test/python/`, because their BTOR2 paths are relative to that directory.

### Active-stepping testbench

The following example assigns symbolic values to an adder, advances the DUT,
and checks a relation over all values represented by those symbols:

```python
from pywasim import Dut, zero_extend

dut = Dut("../../design/pywasim-test/adder.btor2")
dut.set_init()

dut.a.value = "a1"
dut.b.value = "b1"
a1 = dut.a.value
dut.step()

dut.a.value = "a2"
dut.b.value = "b2"
b2 = dut.b.value
dut.step()

dut.check_assertion(
    dut.out.value == zero_extend(a1, 1) + zero_extend(b2, 1)
)
```

An integer assignment, such as `dut.a.value = 3`, is concrete. A string
assignment, such as `dut.a.value = "a1"`, creates or refers to a symbolic
value. Useful active-stepping operations include:

- `dut.set_init(overrides)` — initialize from the BTOR2 initial condition;
- `dut.free_init(overrides)` — start from unconstrained symbolic state;
- `dut.step(n)` — advance by one or more cycles;
- `dut.set_constraint(expr)` — restrict feasible symbolic behaviors;
- `dut.check_assertion(expr)` — prove the expression under current
  assumptions;
- `dut.check_prop()` — check a property embedded in the BTOR2 model;
- `dut.back_step()` — return to the previous symbolic cycle.

Run a small active-stepping example with:

```bash
cd test/python
python3 test_adder.py
```

### Coroutine-based testbench

Coroutine testbenches are useful for variable-latency DUTs and concurrent drivers:

```python
import pywasim_async as pywasim

@pywasim.register_task
def multiply(sim, dut, pywasim):
    dut.a.value = "a0"
    dut.b.value = "b0"
    dut.start.value = 1

    sim.wait_cond(dut.valid.value == 1)

    expected = (
        pywasim.zero_extend(sim.get_var("a0"), 3)
        * pywasim.zero_extend(sim.get_var("b0"), 8)
    )
    sim.check_assertion(dut.result.value == expected)

dut = pywasim.Dut("../../design/asynctest/mul/mul.btor2")
sim = pywasim.async_simulator(dut)

dut.set_init()
multiply(sim, dut, pywasim)
pywasim.start_loop(sim, dut, 100)
```

The main coroutine control operations are:

- `sim.wait_cycle(n)` — suspend the current task for `n` cycles;
- `sim.wait_cond(expr)` — wait until a symbolic condition holds, forking when
  both outcomes are feasible;
- `sim.wait_task(task)` — wait for another registered task to finish;
- `sim.check_assertion(expr)` — check an assertion on the current symbolic
  branch;
- `pywasim.start_loop(sim, dut, bound)` — run all registered tasks up to the
  specified cycle bound.

Run the multiplier example with:

```bash
cd test/python
python3 test_async.py
```

## 🔗 Citation

```bibtex
@inproceedings{yang2026forbench,
  title     = {Forbench: Symbolic Simulation Helps Make Your Testbench More Formal},
  author    = {Yang, Ziyi and Che, Wenbin and Zheng, Ziyue and Hu, Guangyu and Zhang, Hongce},
  booktitle = {Proceedings of the IEEE/ACM International Conference on Computer-Aided Design (ICCAD)},
  year      = {2026}
}
```
