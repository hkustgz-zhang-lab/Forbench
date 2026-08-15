import sys
from pathlib import Path

TEST_PYTHON_DIR = Path(__file__).resolve().parent
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


REPO_ROOT = Path(__file__).resolve().parents[2]
DUT_BTOR2 = REPO_ROOT / "design/test/mismatch/add_plus_one_lat2.btor2"
REF_BTOR2 = REPO_ROOT / "design/test/mismatch/add_lat0.btor2"


@pywasim.register_task
def check_mismatch(sim, dut):
    dut.a.value_def = "a"
    dut.b.value_def = "b"

    ref_model.a.value = dut.a.value
    ref_model.b.value = dut.b.value
    expected = ref_model.out.value
    
    sim.wait_cycle(2)

    sim.check_assertion(dut.out.value == expected)


def main():
    global ref_model

    dut = pywasim.Dut(str(DUT_BTOR2))
    sim = pywasim.async_simulator(dut)

    ref_model = pywasim.RefDesign(str(REF_BTOR2), dut.solver)

    dut.set_init({"stage0": 0, "stage1": 0})
    ref_model.free_init()

    check_mismatch(sim, dut)
    sim.globalvars = globals()

    try:
        pywasim.start_loop(sim, dut, 4)
    except pywasim.PywasimAssertionFailure as err:
        print("EXPECTED MISMATCH DETECTED:", err)
        print("branch num:", len(dut.branch_list))
        return 0

    print("ERROR: mismatch was not detected")
    print("branch num:", len(dut.branch_list))
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
