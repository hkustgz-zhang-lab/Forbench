import pywasim_async as pywasim
from collections import deque

pywasim._debug = False
# check each inst?
# - SLLI/IW, reserved ?
# - SRLI/IW, ... ?
# - ADDI, SUB
# - JAL, BNE, BEQ
# - LBU, LB, LW

def rvformal_addr_valid(in_addr):
    return True

def implies(c1, c2):
    return (~(c1) | (c2))

def rvformal_addr_eq(a1,a2):
    return (rvformal_addr_valid(a1) == rvformal_addr_valid(a2)) & implies(rvformal_addr_valid(a1), a1==a2)

def check_insn(sim, dut, spec_model, inst, check_extra):
    in_rvfi_insn      = spec_model.lookup('SPEC::rvfi_insn')
    in_rvfi_mem_rdata = spec_model.lookup('SPEC::rvfi_mem_rdata')
    in_rvfi_pc_rdata  = spec_model.lookup('SPEC::rvfi_pc_rdata')
    in_rvfi_rs1_rdata = spec_model.lookup('SPEC::rvfi_rs1_rdata')
    in_rvfi_rs2_rdata = spec_model.lookup('SPEC::rvfi_rs2_rdata')
    in_rvfi_valid     = spec_model.lookup('SPEC::rvfi_valid')

    out_valid     = spec_model.lookup('SPEC::spec_valid') # assume
    out_trap      = spec_model.lookup('SPEC::spec_trap')
    out_rs1_addr  = spec_model.lookup('SPEC::spec_rs1_addr')
    out_rs2_addr  = spec_model.lookup('SPEC::spec_rs2_addr')
    out_rd_addr   = spec_model.lookup('SPEC::spec_rd_addr')
    out_rd_wdata  = spec_model.lookup('SPEC::spec_rd_wdata')
    out_pc_wdata  = spec_model.lookup('SPEC::spec_pc_wdata')
    out_mem_addr  = spec_model.lookup('SPEC::spec_mem_addr')
    out_mem_rmask = spec_model.lookup('SPEC::spec_mem_rmask')
    out_mem_wmask = spec_model.lookup('SPEC::spec_mem_wmask')
    out_mem_wdata = spec_model.lookup('SPEC::spec_mem_wdata')

    dut_trap        = dut.get_signal('rvfi_trap').value
    dut_rs1_addr    = dut.get_signal('rvfi_rs1_addr').value
    dut_rs2_addr    = dut.get_signal('rvfi_rs2_addr').value
    dut_rs1_rdata   = dut.get_signal('rvfi_rs1_rdata').value
    dut_rs2_rdata   = dut.get_signal('rvfi_rs2_rdata').value
    dut_rd_addr     = dut.get_signal('rvfi_rd_addr').value
    dut_rd_wdata    = dut.get_signal('rvfi_rd_wdata').value
    dut_pc_rdata    = dut.get_signal('rvfi_pc_rdata').value
    dut_pc_wdata    = dut.get_signal('rvfi_pc_wdata').value
    dut_mem_addr    = dut.get_signal('rvfi_mem_addr').value
    dut_mem_rmask   = dut.get_signal('rvfi_mem_rmask').value
    dut_mem_wmask   = dut.get_signal('rvfi_mem_wmask').value
    dut_mem_rdata   = dut.get_signal('rvfi_mem_rdata').value
    dut_mem_wdata   = dut.get_signal('rvfi_mem_wdata').value

    submap = { in_rvfi_insn : inst,\
               in_rvfi_mem_rdata : dut_mem_rdata, \
               in_rvfi_pc_rdata : dut_pc_rdata, \
               in_rvfi_rs1_rdata : dut_rs1_rdata, \
               in_rvfi_rs2_rdata : dut_rs2_rdata, \
               in_rvfi_valid :  dut.make_constant(1, 1) }

    spec_valid      = out_valid.substitute(submap)    
    spec_trap       = out_trap.substitute(submap)      
    spec_rs1_addr   = out_rs1_addr.substitute(submap)  
    spec_rs2_addr   = out_rs2_addr.substitute(submap)  
    spec_rd_addr    = out_rd_addr.substitute(submap)   
    spec_rd_wdata   = out_rd_wdata.substitute(submap)  
    spec_pc_wdata   = out_pc_wdata.substitute(submap)  
    spec_mem_addr   = out_mem_addr.substitute(submap)  
    spec_mem_rmask  = out_mem_rmask.substitute(submap) 
    spec_mem_wmask  = out_mem_wmask.substitute(submap) 
    spec_mem_wdata  = out_mem_wdata.substitute(submap) 

    if check_extra:
      out_other_type_decode = spec_model.lookup('SPEC::spec_is_fence_sys_csr')
      spec_other_type_decode = out_other_type_decode.substitute(submap)
      
      with sim.assume((spec_valid == 0) & (spec_other_type_decode == 0)): # mret fence,ecall, csr
        sim.check_assertion(dut_trap == 1)

    with sim.assume(spec_valid == 1):
        sim.check_assertion(dut_rs1_rdata == 0, asmpts = [dut_rs1_addr == 0])
        sim.check_assertion(dut_rs2_rdata == 0, asmpts = [dut_rs2_addr == 0])
        sim.check_assertion(spec_trap == dut_trap)

        with sim.assume(spec_trap == 0) as possible:
            if possible:
                sim.check_assertion(spec_rs1_addr == dut_rs1_addr , asmpts = [spec_rs1_addr != 0])
                sim.check_assertion(spec_rs2_addr == dut_rs2_addr , asmpts = [spec_rs2_addr != 0])
                sim.check_assertion(spec_rd_addr == dut_rd_addr)
                sim.check_assertion(spec_rd_wdata == dut_rd_wdata)
                
                sim.check_assertion(rvformal_addr_eq(spec_pc_wdata, dut_pc_wdata))
            with sim.assume((spec_mem_rmask != 0) | (spec_mem_wmask != 0)) as p2:
                if p2:
                    sim.check_assertion(rvformal_addr_eq(spec_mem_addr, dut_mem_addr))
            for i in range(4): #0->3
                with sim.assume(spec_mem_wmask[i] == 1):
                    sim.check_assertion(dut_mem_wmask[i] == 1)
                    sim.check_assertion(spec_mem_wdata[ i*8+7 : i*8] == dut_mem_wdata[i*8+7 : i*8])
                with sim.assume((spec_mem_wmask[i] == 0) & (dut_mem_wmask[i] == 1)):
                    sim.check_assertion(dut_mem_rmask[i] == 1)
                    sim.check_assertion(dut_mem_rdata[ i*8+7 : i*8] == dut_mem_wdata[i*8+7 : i*8])
                sim.check_assertion(implies(spec_mem_rmask[i], dut_mem_rmask[i]))


@pywasim.register_task
def test1(sim, dut):
    # global assumptions, register[0] is always 0
    # (because in the adaptation for symex they made it symbolic)
    # (so we need to restore it)
    dut.set_constraint(dut.get_signal('regs.regFile[0]').value == 0)

    # start from here
    print("test_set")
    dut.reset.value_def = 0 # default: not reset
    dut.io_memIF_IMem_instructionReady.value = 0

    can_sat = dut.check_sat(dut.io_memIF_IMem_fetchEnable.value == 1, [])
    while not can_sat:
        sim.wait_cycle()
        dut.io_memIF_IMem_instructionReady.value = 0
        can_sat = dut.check_sat(dut.io_memIF_IMem_fetchEnable.value == 1, [])
    # apply input
    dut.io_memIF_IMem_instructionReady.value = 1
    dut.io_memIF_IMem_instruction.value = 'inst' # assign symbolic instruction
    print('-- cycle:',sim.current_cycle(), 'inst applied')
    sim.wait_cycle()
    while dut.rvfi_valid.value == 0:
        sim.wait_cycle()
    print('-- cycle:',sim.current_cycle(), 'inst check')
    inst = sim.get_var('inst')
    sim.check_assertion(dut.rvfi_insn.value == inst)
    check_insn(sim, dut, spec_model, inst, True)
    # https://ret.futo.org/riscv/  endienness
    # https://msyksphinz-self.github.io/riscv-isadoc/  for instruction decodes
    

#load dut    
dut = pywasim.Dut('../../design/asynctest/microriscv/rv32.btor2')
sim = pywasim.async_simulator(dut)

#load spec
spec_model = pywasim.TransSys('../../design/asynctest/microriscv/riscv-formal-rv32i-spec.btor2', dut.solver, 'SPEC::')

dut.set_init()
test1(sim, dut)

sim.globalvars = globals()
pywasim.start_loop(sim, dut, 10)
print("branch num:", len(dut.branch_list))
