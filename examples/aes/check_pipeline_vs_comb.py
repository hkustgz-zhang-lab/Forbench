import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
TEST_PYTHON_DIR = REPO_ROOT / "test" / "python"
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


PIPELINE_DUT = REPO_ROOT / "design/smtsweep-test/aes/AES-Pipeline/aes_128.btor2"
COMB_REF = REPO_ROOT / "design/smtsweep-test/aes/AES-Comb/AES_Encrypt_ite.btor2"
PIPELINE_LATENCY = 21
BOUND = PIPELINE_LATENCY + 2


@pywasim.register_task
def compare_one_symbolic_block(sim, dut):
    dut.state.value = "aes_state"
    dut.key.value = "aes_key"

    ref_model.get_signal("in").value = dut.state.value
    ref_model.key.value = dut.key.value
    expected_ciphertext = ref_model.out.value

    print("-- cycle:", sim.current_cycle(), "symbolic AES block applied")
    sim.wait_cycle(PIPELINE_LATENCY)

    print("-- cycle:", sim.current_cycle(), "checking pipeline output")
    sim.check_assertion(dut.out.value == expected_ciphertext, smt_sweep_enabled=True)


dut = pywasim.Dut(str(PIPELINE_DUT))
sim = pywasim.async_simulator(dut)

ref_model = pywasim.RefDesign(str(COMB_REF), dut.solver)
ref_model.free_init()

dut.free_init()
compare_one_symbolic_block(sim, dut)
sim.globalvars = globals()

try:
    pywasim.start_loop(sim, dut, BOUND)
except pywasim.PywasimAssertionFailure as err:
    print("MISMATCH DETECTED:", err)
    print("branch num:", len(dut.branch_list))
    raise SystemExit(1)

print("MATCH")
print("branch num:", len(dut.branch_list))
