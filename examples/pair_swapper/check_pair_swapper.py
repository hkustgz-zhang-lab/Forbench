import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
TEST_PYTHON_DIR = REPO_ROOT / "test" / "python"
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


PAIR_SWAPPER_BTOR2 = REPO_ROOT / "design/asynctest/pair-swapper/pair_swapper.btor2"
N = 4

# Shared queue of expected outputs.
send_q = [None]*N
expect_q = [None]*N


def new_byte(sim, name):
    return sim.new_bv_var(name, 8)

def send_byte(sim, dut, data):
    dut.in_valid.value = 1
    dut.in_data.value = data

@pywasim.register_task
def stream_in(sim, dut):
    dut.reset.value_def = 0
    for i in range(N):
        v = new_byte(sim, f"data{i}")
        send_q[i] = v

    for i in range(N):
        if i % 2 == 0:  # 0 -> 1 , 2 -> 3
            expect_q[i + 1] = send_q[i]
        else: # 1 -> 0 , 3 -> 2
            expect_q[i - 1] = send_q[i]

    i = 0
    while i < N:
        bridx = sim.get_current_branch_id()
        ncycle = sim.current_cycle()
        c1 = sim.new_bool_var(f'cycle_{i}_send_br{bridx}')
        print(f'br #{bridx} @{ncycle} i={i}')
        if c1: # add randomness in sending
            data = send_q[i]
            # Drive symbolic input stream.
            send_byte(sim, dut, data)

            bridx = sim.get_current_branch_id()
            ncycle = sim.current_cycle()
            print(f'br #{bridx} @{ncycle} i={i} send:',data)

            i += 1
        else:
            # Turn off input valid signal otherwise
            bridx = sim.get_current_branch_id()
            ncycle = sim.current_cycle()
            print(f'br #{bridx} @{ncycle} i={i} skipped')

            dut.in_valid.value = 0
        # Wait for the next cycle
        sim.wait_cycle()

        # if i%2 == 0 and i != 0 and c1: # put inside
        #   sim.abstract((emit_first == 1) & (have_first == 0) & (out_valid == 1))


@pywasim.register_task
def stream_out(sim, dut):
    for i in range(N):
        sim.wait_cond(dut.out_valid.value == 1)

        bridx = sim.get_current_branch_id()
        ncycle = sim.current_cycle()
        print(f'br #{bridx} @{ncycle} i={i} expect:', expect_q[i])
        sim.check_assertion(dut.out_data.value == expect_q[i])


def main():
    dut = pywasim.Dut(str(PAIR_SWAPPER_BTOR2))
    sim = pywasim.async_simulator(dut)

    # {"out_valid":0,"out_data":0,"have_first":0,"first_byte":0,"emit_first":0,"emit_byte":0}
    dut.set_init()

    stream_in(sim, dut)
    stream_out(sim, dut)
    sim.globalvars = globals()
    #pywasim.enable_debug()
    pywasim.start_loop(sim, dut, 20)

    print("PAIR SWAPPER CHECK PASS")
    print("branch num:", len(dut.branch_list))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
