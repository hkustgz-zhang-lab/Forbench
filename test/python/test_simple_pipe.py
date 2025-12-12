import pywasim_async as pywasim
from collections import deque

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

    sim.wait_cond(dut.FV_wb_go.value == 1)

    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    sim.wait_cycle()
    dut.dummy_read_rf.value = rd
    sim.check_assertion(rs1_val + rs2_val == dut.dummy_rf_data.value)

@pywasim.register_task
def test_add_stall(sim, dut, pywasim):
    print("test_add_stall")
    reset(sim, dut)
    dut.inst.value_def = "inst"
    dut.inst_valid.value = 1
    
    op = dut.inst.value[7:6]
    rs1= dut.inst.value[5:4]
    rs2= dut.inst.value[3:2]
    rd = dut.inst.value[1:0]

    sim.wait_cond(dut.FV_wb_go.value == 1)

    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    sim.wait_cycle()
    rs_val  = pywasim.ite(rd == 0, dut.FV_r0.value,
                    pywasim.ite(rd == 1, dut.FV_r1.value,
                    pywasim.ite(rd == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    dut.set_constraint(op == OP_ADD)  # assume OP == ADD
    sim.check_assertion(rs1_val + rs2_val == rs_val)

@pywasim.register_task
def test_pipeline1(sim, dut, pywasim):
    print("test_pipeline")
    reset(sim, dut)
    
    dut.stallex.value_def = 0
    dut.stallwb.value_def = 0
    
    # SET
    dut.inst.value = "inst1"
    dut.inst_valid.value = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_SET)
    sim.wait_cycle()
    
    # SET
    dut.inst.value = "inst2"
    dut.inst_valid.value = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_SET)
    sim.wait_cycle()
    
    # ADD
    dut.inst.value_def = "inst3"
    dut.inst_valid.value_def = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_ADD)

def check_set(sim, dut, pywasim, inst_string):
    sim.wait_cycle()
    inst = sim.get_var(inst_string)
    dut.dummy_read_rf.value = inst[1:0]
    sim.check_assertion(pywasim.zero_extend(inst[5:2], 4) == dut.dummy_rf_data.value)
    # if if(expr) can be used to fork different branches, we can use one function to check all instructions
    
def check_add(sim, dut, pywasim, inst_string):
    inst = sim.get_var(inst_string)
    rs1= inst[5:4]
    rs2= inst[3:2]
    rd = inst[1:0]
    
    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    sim.wait_cycle()
    dut.dummy_read_rf.value = rd
    sim.check_assertion(rs1_val + rs2_val == dut.dummy_rf_data.value)

@pywasim.register_task
def test_pipeline2(sim, dut, pywasim):
    # waiting write back to check
    sim.wait_cond(dut.FV_wb_go.value == 1)
    check_set(sim, dut, pywasim, 'inst1')
    check_set(sim, dut, pywasim, 'inst2')
    check_add(sim, dut, pywasim, 'inst3')
    
    sim.wait_cycle()
    
@pywasim.register_task
def test_pipeline_with_stall1(sim, dut, pywasim):
    print("test_pipeline_stall")
    reset(sim, dut)
    
    # dut.stallex.value_def = 0
    # dut.stallwb.value_def = 0
    
    # SET
    dut.inst.value = "inst1"
    dut.inst_valid.value = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_SET)
    sim.wait_cycle()
    
    # SET
    dut.inst.value = "inst2"
    dut.inst_valid.value = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_SET)
    sim.wait_cycle()
    
    # ADD
    dut.inst.value_def = "inst3"
    dut.inst_valid.value_def = 1
    dut.set_constraint(dut.inst.value[7:6] == OP_ADD)

def check_set_stall(sim, dut, pywasim, inst_string):
    # sim.wait_cycle()
    inst = sim.get_var(inst_string)
    dut.dummy_read_rf.value = inst[1:0]
    sim.check_assertion(pywasim.zero_extend(inst[5:2], 4) == dut.dummy_rf_data.value)
    # if if(expr) can be used to fork different branches, we can easily use one function to check all instructions
    
@pywasim.register_task
def test_pipeline_with_stall2(sim, dut, pywasim):
    sim.wait_cond(dut.FV_wb_go.value == 1)
    sim.wait_cond(dut.FV_wb_go.value == 1)
    check_set_stall(sim, dut, pywasim, 'inst1')
    sim.wait_cond(dut.FV_wb_go.value == 1)
    check_set_stall(sim, dut, pywasim, 'inst2')
    
    inst = sim.get_var('inst3')
    rs1= inst[5:4]
    rs2= inst[3:2]
    rd = inst[1:0]
    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    sim.wait_cond(dut.FV_wb_go.value == 1)
    rs_val  = pywasim.ite(rd == 0, dut.FV_r0.value,
                    pywasim.ite(rd == 1, dut.FV_r1.value,
                    pywasim.ite(rd == 2, dut.FV_r2.value, dut.FV_r3.value)))
    sim.check_assertion(rs1_val + rs2_val == rs_val)    

@pywasim.register_task
def test(sim, dut, pywasim):
    print("test")
    dut.rst.value_def = 1
    sim.wait_cycle(5)
    dut.rst.value_def = 0
    
    sim.wait_cond(dut.inst_valid.value == 1)
    inst = dut.inst.value
    op = dut.inst.value[7:6]
    rs1= dut.inst.value[5:4]
    rs2= dut.inst.value[3:2]
    rd = dut.inst.value[1:0]
    immd = pywasim.zero_extend(dut.inst.value[5:2], 4)
    
    sim.wait_cond(dut.FV_wb_go.value == 1)
    rs1_val = pywasim.ite(rs1 == 0, dut.FV_r0.value,
                    pywasim.ite(rs1 == 1, dut.FV_r1.value,
                    pywasim.ite(rs1 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    rs2_val = pywasim.ite(rs2 == 0, dut.FV_r0.value,
                    pywasim.ite(rs2 == 1, dut.FV_r1.value,
                    pywasim.ite(rs2 == 2, dut.FV_r2.value, dut.FV_r3.value)))
    sim.wait_cycle()
    rs_val  = pywasim.ite(rd == 0, dut.FV_r0.value,
                    pywasim.ite(rd == 1, dut.FV_r1.value,
                    pywasim.ite(rd == 2, dut.FV_r2.value, dut.FV_r3.value)))
    
    # # check SET
    # dut.set_constraint(op == OP_SET)
    # sim.check_assertion(immd == rs_val)
    # dut.clear_constraint()
    
    # # check ADD
    # dut.set_constraint(op == OP_ADD)
    # sim.check_assertion(rs1_val + rs2_val == rs_val)
    # dut.clear_constraint()
    
    # # check NAND
    # dut.set_constraint(op == OP_NAND)
    # sim.check_assertion(~(rs1_val & rs2_val) == rs_val)
    # dut.clear_constraint()
    
    # create a noderef for 1
    dut.dummy_read_rf.value = 1
    nop_val = dut.dummy_read_rf.value[0]
    # check any instruction
    sim.check_assertion(pywasim.ite(op == OP_SET , immd == rs_val,
                        pywasim.ite(op == OP_ADD , rs1_val + rs2_val == rs_val,
                        pywasim.ite(op == OP_NAND, ~(rs1_val & rs2_val) == rs_val, nop_val))))

    
dut = pywasim.Dut('../../design/asynctest/simplepipe-3stage/simple_pipe_stall.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
# test_set(sim, dut, pywasim)
# test_add(sim, dut, pywasim)
test_add_stall(sim, dut, pywasim)
# test_pipeline1(sim, dut, pywasim)
# test_pipeline2(sim, dut, pywasim)
# test_pipeline_with_stall1(sim, dut, pywasim)
# test_pipeline_with_stall2(sim, dut, pywasim)
# test(sim, dut, pywasim)


sim.globalvars = globals()
pywasim.start_loop(sim, dut, 20)
print("branch num:", len(dut.branch_list))
