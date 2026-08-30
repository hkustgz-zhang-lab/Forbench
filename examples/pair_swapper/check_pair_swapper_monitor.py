import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
TEST_PYTHON_DIR = REPO_ROOT / "test" / "python"
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


PAIR_SWAPPER_BTOR2 = REPO_ROOT / "design/asynctest/pair-swapper/pair_swapper.btor2"
BOUND = 10


@pywasim.register_task
def monitor_pair_swapper(sim, dut):
    dut.clk.value_def = 1
    dut.reset.value_def = 0

    have_first = False
    first = None
    expect_q = []

    for _ in range(BOUND):
        # Current output is checked before current input updates the reference
        # model, matching clocked SVA sampling semantics.
        if dut.out_valid.value == 1:
            assert len(expect_q) > 0
            expected = expect_q.pop(0)
            sim.check_assertion(dut.out_data.value == expected)
	
        if dut.in_valid.value == 1:
            data = dut.in_data.value
            if have_first:
                expect_q.append(data)
                expect_q.append(first)
                have_first = False
            else:
                first = data
                have_first = True

        sim.wait_cycle()
        


def main():
    dut = pywasim.Dut(str(PAIR_SWAPPER_BTOR2))
    sim = pywasim.async_simulator(dut)

    dut.set_init({
        "have_first": 0,
        "first_byte": 0,
        "emit_first": 0,
        "emit_byte": 0,
        "out_valid": 0,
        "out_data": 0,
    })

    monitor_pair_swapper(sim, dut)
    sim.globalvars = globals()
    pywasim.start_loop(sim, dut, BOUND + 2)

    print("PAIR SWAPPER MONITOR CHECK PASS")
    print("branch num:", len(dut.branch_list))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
