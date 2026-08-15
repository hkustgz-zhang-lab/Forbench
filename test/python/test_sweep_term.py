import sys
from pathlib import Path


TEST_PYTHON_DIR = Path(__file__).resolve().parent
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

REPO_ROOT = Path(__file__).resolve().parents[2]
BUILD_DIR = REPO_ROOT / "build"
if str(BUILD_DIR) not in sys.path:
    sys.path.append(str(BUILD_DIR))

from pywasimbase import TransSys, sweep_term
import pywasim_async as pywasim


DUT_BTOR2 = REPO_ROOT / "design/test/mismatch/add_lat0.btor2"
SWEEP_OPTIONS = {
    "iterations": 4,
    "find_unsat": 2,
    "find_sat": 1,
}


def check_direct_sweep_term():
    ts = TransSys(str(DUT_BTOR2))
    a = ts.lookup("a")
    b = ts.lookup("b")
    expr = (a + b) == (b + a)

    swept = sweep_term(expr, SWEEP_OPTIONS, [])
    print(expr)
    print(swept)

    solver = ts.get_solver()
    solver.push()
    solver.assert_formula(~swept)
    can_fail = solver.check_sat()
    solver.pop()

    if can_fail:
        raise AssertionError("swept commutativity expression is not valid")


@pywasim.register_task
def check_async_sweep(sim, dut):
    dut.a.value = "a"
    dut.b.value = "b"
    sim.check_assertion(
        (dut.a.value + dut.b.value) == (dut.b.value + dut.a.value),
        smt_sweep_enabled=True,
    )


def check_async_assertion_sweep():
    dut = pywasim.Dut(str(DUT_BTOR2))
    sim = pywasim.async_simulator(dut)

    dut.free_init()
    check_async_sweep(sim, dut)
    sim.globalvars = globals()
    pywasim.start_loop(sim, dut, 1)


def main():
    check_direct_sweep_term()
    check_async_assertion_sweep()
    print("sweep_term tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
