#!/usr/bin/env python3

"""Small symbolic FIFO testbench.

Each scenario uses symbolic data, so one solver query covers every possible
8-bit value rather than just the few constants chosen by a normal testbench.
"""

import argparse
import contextlib
import io
import sys
from pathlib import Path

THIS_DIR = Path(__file__).resolve().parent
ROOT_DIR = THIS_DIR.parent
sys.path.insert(0, str(THIS_DIR))

from pywasim import Dut  # noqa: E402


def new_dut(model_path):
    dut = Dut(str(model_path))
    # gen_all.sh writes the post-reset state into the BTOR2 initial state.
    dut.set_init()
    dut.clk.value_def = 0  # The BTOR transition already represents one edge.
    dut.rst.value_def = 0
    dut.wr_en.value_def = 0
    dut.rd_en.value_def = 0
    dut.din.value_def = 0
    return dut


def write(dut, value):
    dut.din.value = value
    dut.wr_en.value = 1
    dut.rd_en.value = 0
    captured_value = dut.din.value
    dut.step()
    return captured_value


def read(dut):
    dut.wr_en.value = 0
    dut.rd_en.value = 1
    dut.step()


def prove(dut, expression):
    # Keep the solver's formula output quiet. On a failed assertion,
    # check_assertion() automatically attempts to write cex.vcd.
    with contextlib.redirect_stdout(io.StringIO()):
        return dut.check_assertion(expression)


def check_first_word(model_path):
    """One symbolic write followed by one read catches bad basic addressing."""
    dut = new_dut(model_path)
    first = write(dut, "first_word")
    read(dut)
    return prove(dut, dut.dout.value == first)


def check_simultaneous_count(model_path):
    """An accepted read and write must preserve count and return the old head."""
    dut = new_dut(model_path)
    first = write(dut, "old_head")

    dut.din.value = "new_tail"
    dut.wr_en.value = 1
    dut.rd_en.value = 1
    dut.step()

    return prove(dut, (dut.count.value == 1) & (dut.dout.value == first))


def check_full_depth_wrap(model_path):
    """Sixteen symbolic writes must not overwrite the oldest FIFO entry."""
    dut = new_dut(model_path)
    first = None
    for index in range(16):
        value = write(dut, f"word_{index}")
        if index == 0:
            first = value

    status_ok = prove(dut, (dut.full.value == 1) & (dut.count.value == 16))
    read(dut)
    data_ok = prove(dut, dut.dout.value == first)
    return status_ok and data_ok


SCENARIOS = (
    ("first-word ordering", check_first_word),
    ("simultaneous read/write", check_simultaneous_count),
    ("full-depth pointer wrap", check_full_depth_wrap),
)


def main():

    model_path = "../../design/w_array/fifo.btor2"
    failures = 0
    print(f"Symbolic checks for {model_path}:")
    for name, scenario in SCENARIOS:
        passed = scenario(model_path)
        print(f"  {'PASS' if passed else 'FAIL'}  {name}")
        failures += not passed

    if failures:
        print(f"RESULT: BUG FOUND ({failures} failed symbolic check(s))")
        return 1

    print("RESULT: all symbolic checks proved")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
