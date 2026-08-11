import argparse
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
TEST_PYTHON_DIR = REPO_ROOT / "test" / "python"
if str(TEST_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_PYTHON_DIR))

import pywasim_async as pywasim


DEFAULT_DUT = REPO_ROOT / "design/asynctest/microriscv/rv32.btor2"
REF_MODEL = REPO_ROOT / "design/asynctest/microriscv/riscv-formal-rv32i-spec.btor2"


def rvformal_addr_valid(in_addr):
    return True


def implies(c1, c2):
    return (~c1) | c2


def rvformal_addr_eq(a1, a2):
    return (
        (rvformal_addr_valid(a1) == rvformal_addr_valid(a2))
        & implies(rvformal_addr_valid(a1), a1 == a2)
    )


def check_insn(sim, dut, ref_model, inst, check_extra):
    dut_trap = dut.get_signal("rvfi_trap").value
    dut_rs1_addr = dut.get_signal("rvfi_rs1_addr").value
    dut_rs2_addr = dut.get_signal("rvfi_rs2_addr").value
    dut_rs1_rdata = dut.get_signal("rvfi_rs1_rdata").value
    dut_rs2_rdata = dut.get_signal("rvfi_rs2_rdata").value
    dut_rd_addr = dut.get_signal("rvfi_rd_addr").value
    dut_rd_wdata = dut.get_signal("rvfi_rd_wdata").value
    dut_pc_rdata = dut.get_signal("rvfi_pc_rdata").value
    dut_pc_wdata = dut.get_signal("rvfi_pc_wdata").value
    dut_mem_addr = dut.get_signal("rvfi_mem_addr").value
    dut_mem_rmask = dut.get_signal("rvfi_mem_rmask").value
    dut_mem_wmask = dut.get_signal("rvfi_mem_wmask").value
    dut_mem_rdata = dut.get_signal("rvfi_mem_rdata").value
    dut_mem_wdata = dut.get_signal("rvfi_mem_wdata").value

    ref_model.rvfi_insn.value = inst
    ref_model.rvfi_mem_rdata.value = dut_mem_rdata
    ref_model.rvfi_pc_rdata.value = dut_pc_rdata
    ref_model.rvfi_rs1_rdata.value = dut_rs1_rdata
    ref_model.rvfi_rs2_rdata.value = dut_rs2_rdata
    ref_model.rvfi_valid.value = 1

    spec_valid = ref_model.spec_valid.value
    spec_trap = ref_model.spec_trap.value
    spec_rs1_addr = ref_model.spec_rs1_addr.value
    spec_rs2_addr = ref_model.spec_rs2_addr.value
    spec_rd_addr = ref_model.spec_rd_addr.value
    spec_rd_wdata = ref_model.spec_rd_wdata.value
    spec_pc_wdata = ref_model.spec_pc_wdata.value
    spec_mem_addr = ref_model.spec_mem_addr.value
    spec_mem_rmask = ref_model.spec_mem_rmask.value
    spec_mem_wmask = ref_model.spec_mem_wmask.value
    spec_mem_wdata = ref_model.spec_mem_wdata.value

    if check_extra:
        spec_other_type_decode = ref_model.spec_is_fence_sys_csr.value
        with sim.assume((spec_valid == 0) & (spec_other_type_decode == 0)):
            sim.check_assertion(dut_trap == 1)

    with sim.assume(spec_valid == 1):
        sim.check_assertion(dut_rs1_rdata == 0, asmpts=[dut_rs1_addr == 0])
        sim.check_assertion(dut_rs2_rdata == 0, asmpts=[dut_rs2_addr == 0])
        sim.check_assertion(spec_trap == dut_trap)

        with sim.assume(spec_trap == 0) as possible:
            if possible:
                sim.check_assertion(spec_rs1_addr == dut_rs1_addr, asmpts=[spec_rs1_addr != 0])
                sim.check_assertion(spec_rs2_addr == dut_rs2_addr, asmpts=[spec_rs2_addr != 0])
                sim.check_assertion(spec_rd_addr == dut_rd_addr)
                sim.check_assertion(spec_rd_wdata == dut_rd_wdata)
                sim.check_assertion(rvformal_addr_eq(spec_pc_wdata, dut_pc_wdata))

            with sim.assume((spec_mem_rmask != 0) | (spec_mem_wmask != 0)) as possible_mem:
                if possible_mem:
                    sim.check_assertion(rvformal_addr_eq(spec_mem_addr, dut_mem_addr))

            for i in range(4):
                with sim.assume(spec_mem_wmask[i] == 1):
                    sim.check_assertion(dut_mem_wmask[i] == 1)
                    sim.check_assertion(
                        spec_mem_wdata[i * 8 + 7 : i * 8]
                        == dut_mem_wdata[i * 8 + 7 : i * 8]
                    )
                with sim.assume((spec_mem_wmask[i] == 0) & (dut_mem_wmask[i] == 1)):
                    sim.check_assertion(dut_mem_rmask[i] == 1)
                    sim.check_assertion(
                        dut_mem_rdata[i * 8 + 7 : i * 8]
                        == dut_mem_wdata[i * 8 + 7 : i * 8]
                    )
                sim.check_assertion(implies(spec_mem_rmask[i], dut_mem_rmask[i]))


@pywasim.register_task
def check_one_instruction(sim, dut):
    dut.set_constraint(dut.get_signal("regs.regFile[0]").value == 0)

    dut.reset.value_def = 0
    dut.io_memIF_IMem_instructionReady.value = 0

    can_fetch = dut.check_sat(dut.io_memIF_IMem_fetchEnable.value == 1, [])
    while not can_fetch:
        sim.wait_cycle()
        dut.io_memIF_IMem_instructionReady.value = 0
        can_fetch = dut.check_sat(dut.io_memIF_IMem_fetchEnable.value == 1, [])

    dut.io_memIF_IMem_instructionReady.value = 1
    dut.io_memIF_IMem_instruction.value = "inst"
    print("-- cycle:", sim.current_cycle(), "inst applied")
    sim.wait_cycle()

    while dut.rvfi_valid.value == 0:
        sim.wait_cycle()

    print("-- cycle:", sim.current_cycle(), "inst check")
    inst = sim.get_var("inst")
    sim.check_assertion(dut.rvfi_insn.value == inst)
    check_insn(sim, dut, ref_model, inst, True)


def run_check(design_btor2, bound):
    global ref_model

    dut = pywasim.Dut(str(design_btor2))
    sim = pywasim.async_simulator(dut)

    ref_model = pywasim.RefDesign(str(REF_MODEL), dut.solver)
    ref_model.simulator.free_init({})

    dut.set_init()
    check_one_instruction(sim, dut)
    sim.globalvars = globals()

    try:
        pywasim.start_loop(sim, dut, bound)
    except pywasim.PywasimAssertionFailure as err:
        print("BUG DETECTED:", err)
        print("branch num:", len(dut.branch_list))
        return True

    print("NO BUG DETECTED")
    print("branch num:", len(dut.branch_list))
    return False


def main():
    parser = argparse.ArgumentParser(description="Check one MicroRV BTOR2 design.")
    parser.add_argument(
        "design",
        nargs="?",
        type=Path,
        default=DEFAULT_DUT,
        help="Path to a MicroRV BTOR2 file. Defaults to the checked-in correct design.",
    )
    parser.add_argument("--bound", type=int, default=10, help="Cycle bound for async simulation")
    parser.add_argument(
        "--expect",
        choices=("any", "clean", "bug"),
        default="any",
        help="Expected result. Use clean for the reference design and bug for known buggy variants.",
    )
    args = parser.parse_args()

    design = args.design
    if not design.is_absolute():
        design = Path.cwd() / design

    bug_detected = run_check(design.resolve(), args.bound)
    if args.expect == "bug":
        return 0 if bug_detected else 1
    if args.expect == "clean":
        return 0 if not bug_detected else 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
