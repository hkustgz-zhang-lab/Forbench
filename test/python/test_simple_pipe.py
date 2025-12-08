import pywasim_async as pywasim

# parameter
OP_NOP  = 0b00
OP_ADD  = 0b01
OP_SET  = 0b10
OP_NAND = 0b11

def reset(sim, dut):
    print("reset")
    dut.rst.value_def = 1
    sim.wait_cycle(5)
    dut.rst.value_def = 0
    sim.wait_cycle()

@pywasim.register_task
def test_set(sim, dut, pywasim):
    print("test_set")
    reset(sim, dut)
    dut.inst.value_def = "inst"
    dut.inst_valid.value_def = 1
    dut.stallex.value_def = 0
    dut.stallwb.value_def = 0
    
    op = dut.inst.value[7:6]
    rd = dut.inst.value[1:0]
    immd = pywasim.zero_extend(dut.inst.value[5:2], 4)
    dut.set_constraint(op == OP_SET) # assume OP == SET
    
    sim.wait_cond(dut.FV_wb_go.value == 1)
    sim.wait_cycle()
    dut.dummy_read_rf.value = rd
    sim.check_assertion(immd == dut.dummy_rf_data.value)

@pywasim.register_task
def test_add(sim, dut, pywasim):
    print("test_add")
    reset(sim, dut)
    dut.inst.value_def = "inst"
    # dut.inst.value_def = 0b01000111 # ADD r0 r1 -> wb r2
    dut.inst_valid.value_def = 1
    dut.stallex.value_def = 0
    dut.stallwb.value_def = 0
    
    op = dut.inst.value[7:6]
    rs1= dut.inst.value[5:4]
    rs2= dut.inst.value[3:2]
    rd = dut.inst.value[1:0]
    dut.set_constraint(op == OP_ADD)  # assume OP == ADD
    dut.set_constraint(rd != rs1)   # assume wb rs != rs1 and rs2
    dut.set_constraint(rd != rs2)
    
    sim.wait_cond((dut.FV_wb_go.value & dut.ex_wb_reg_wen.value) == 1)
    sim.wait_cycle()

    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    dut.dummy_read_rf.value = rd
    sim.check_assertion(rs1_val + rs2_val == dut.dummy_rf_data.value)


dut = pywasim.Dut('../../design/asynctest/simplepipe-3stage/simple_pipe_stall.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
# test_set(sim, dut, pywasim)
test_add(sim, dut, pywasim)

sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
