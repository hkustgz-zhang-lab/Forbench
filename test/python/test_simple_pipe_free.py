import pywasim_async as pywasim

# parameter
OP_NOP  = 0b00
OP_ADD  = 0b01
OP_SET  = 0b10
OP_NAND = 0b11

@pywasim.register_task
def test(sim, dut, pywasim):
    # keep not reset
    dut.rst.value_def = 0
    
    # set invariant
    dut.set_constraint(dut.get_signal('scoreboard[0]').value[1] == dut.id_ex_valid.value & dut.id_ex_reg_wen.value & (dut.id_ex_rd.value == 0))
    dut.set_constraint(dut.get_signal('scoreboard[0]').value[0] == dut.ex_wb_valid.value & dut.ex_wb_reg_wen.value & (dut.ex_wb_rd.value == 0))
    dut.set_constraint(dut.get_signal('scoreboard[1]').value[1] == dut.id_ex_valid.value & dut.id_ex_reg_wen.value & (dut.id_ex_rd.value == 1))
    dut.set_constraint(dut.get_signal('scoreboard[1]').value[0] == dut.ex_wb_valid.value & dut.ex_wb_reg_wen.value & (dut.ex_wb_rd.value == 1))
    dut.set_constraint(dut.get_signal('scoreboard[2]').value[1] == dut.id_ex_valid.value & dut.id_ex_reg_wen.value & (dut.id_ex_rd.value == 2))
    dut.set_constraint(dut.get_signal('scoreboard[2]').value[0] == dut.ex_wb_valid.value & dut.ex_wb_reg_wen.value & (dut.ex_wb_rd.value == 2))
    dut.set_constraint(dut.get_signal('scoreboard[3]').value[1] == dut.id_ex_valid.value & dut.id_ex_reg_wen.value & (dut.id_ex_rd.value == 3))
    dut.set_constraint(dut.get_signal('scoreboard[3]').value[0] == dut.ex_wb_valid.value & dut.ex_wb_reg_wen.value & (dut.ex_wb_rd.value == 3), True)

    # when ready high, set input instruction
    dut.set_constraint(dut.inst_ready.value == 1, True)   # assume inst_ready pull up
    dut.inst.value = "inst"
    dut.inst_valid.value = 1
    
    # wait flag signal to trace the instruction
    sim.check_assertion(dut.FV_if_go.value == 1)    # FV_if_go = inst_valid && inst_ready
    sim.wait_cond(dut.FV_id_go.value == 1)
    sim.wait_cond(dut.FV_ex_go.value == 1)
    sim.wait_cond(dut.FV_wb_go.value == 1)

      # record for post checking
    inst = sim.get_var('inst')
    op = inst[7:6]
    rs1= inst[5:4]
    rs2= inst[3:2]
    rd = inst[1:0]
    immd = pywasim.zero_extend(inst[5:2], 4)

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
    
    # check instruction result
    dut.set_constraint(op == OP_SET)  # assume OP == SET
    sim.check_assertion(immd == rs_val)
    dut.unset_constraint(op == OP_SET)
    
    dut.set_constraint(op == OP_ADD)  # assume OP == ADD
    sim.check_assertion(rs1_val + rs2_val == rs_val)
    dut.unset_constraint(op == OP_ADD)
    
    dut.set_constraint(op == OP_NAND)  # assume OP == NAND
    sim.check_assertion(~(rs1_val & rs2_val) == rs_val)
    dut.unset_constraint(op == OP_NAND)
    
    # check instruction result in other way
    # sim.check_assertion(pywasim.ite(op == OP_SET , immd == rs_val,
    #                     pywasim.ite(op == OP_ADD , rs1_val + rs2_val == rs_val,
    #                     pywasim.ite(op == OP_NAND, ~(rs1_val & rs2_val) == rs_val, dut.make_bool(1)))))
    
dut = pywasim.Dut('../../design/asynctest/simplepipe-3stage/simple_pipe_stall.btor2')
sim = pywasim.async_simulator(dut)

dut.free_init()
test(sim, dut, pywasim)

sim.globalvars = globals()
pywasim.start_loop(sim, dut, 10)
print("branch num:", len(dut.branch_list))
