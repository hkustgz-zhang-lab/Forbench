import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
TEST_PYTHON_DIR = REPO_ROOT / "test" / "python"
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


PAIR_SWAPPER_BTOR2 = REPO_ROOT / "design/asynctest/pair-swapper/pair_swapper.btor2"
N = 4

send_q = [None] * N
expect_q = [None] * N


def new_byte(sim, name):
    return sim.new_bv_var(name, 8)


def send_byte(sim, dut, data):
    dut.in_valid.value = 1
    dut.in_data.value = data


def recv_check(sim, dut, exp):
    sim.check_assertion(dut.out_data.value == exp)


@pywasim.register_task
def stream_in(sim, dut):
    dut.clk.value_def = 1
    dut.reset.value_def = 0

    for i in range(N):
        send_q[i] = new_byte(sim, f"data{i}")

    for i in range(N):
        if i % 2 == 0:
            expect_q[i + 1] = send_q[i]
        else:
            expect_q[i - 1] = send_q[i]

    for i in range(N):
        send_byte(sim, dut, send_q[i])
        sim.wait_cycle()

    dut.in_valid.value = 0


@pywasim.register_task
def stream_out(sim, dut):
    dut.clk.value_def = 1
    dut.reset.value_def = 0

    for i in range(N):
        sim.wait_cond(dut.out_valid.value == 1)
        recv_check(sim, dut, expect_q[i])


def main():
    dut = pywasim.Dut(str(PAIR_SWAPPER_BTOR2))
    sim = pywasim.async_simulator(dut)

    dut.set_init()
    stream_in(sim, dut)
    stream_out(sim, dut)
    sim.globalvars = globals()
    pywasim.start_loop(sim, dut, 20)

    print("PAIR SWAPPER SIMPLE CHECK PASS")
    print("branch num:", len(dut.branch_list))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
