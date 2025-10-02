import pywasim_async as pywasim
pywasim._extra_globals = globals()  # send global env to pywasim_async

# parameter
PRER_LO = 0b000
PRER_HI = 0b001
CTR     = 0b010
RXR     = 0b011
TXR     = 0b011
CR      = 0b100
SR      = 0b100
TXR_R   = 0b101
CR_R    = 0b110
RD      = 0b1
WR      = 0b0
SADR    = 0b0010000

@pywasim.register_task
def reset(sim, dut, pywasim):  # reset design
    dut.arst_i.value = 1
    dut.wb_rst_i.value_def = 0
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0
    sim.wait_cycle()
    dut.arst_i.value = 0
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0
    sim.wait_cycle()
    dut.arst_i.value_def = 1
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0
    sim.wait_cycle()
    print("task reset done")

@pywasim.register_task
def run1(sim, dut, pywasim):  # program internal registers
    task1 = pywasim._all_states[0]
    sim.wait_task(task1)

    """
        program internal registers
    """
    # wr PRER_LO, write(addr, data)
    # sim.wait_cycle()
    dut.wb_adr_i.value_def =  PRER_LO
    dut.wb_dat_i.value_def =  "PRER_LO"
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_adr_i.unset_def()
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", PRER_LO, "data:", "PRER_LO", ")")

    # wr PRER_HI, write(addr, data)
    # sim.wait_cycle()
    dut.wb_adr_i.value_def =  PRER_HI
    dut.wb_dat_i.value_def =  "PRER_HI"
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_adr_i.unset_def()
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", PRER_HI, "data:", "PRER_HI", ")")

    # rd PRER_LO, rd(addr)
    dut.wb_adr_i.value_def =  PRER_LO
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_adr_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task read done ", "( addr:", PRER_LO, ")")
    # check read data
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_LO"))
        # new find error -> dut.wb_dat_o.value: (ite (= #b1 (bvand wb_we_iX7 (ite (= #b000 wb_adr_iX7) #b1 #b0))) wb_dat_iX7 PRER_LO)
        # so need to remove line 63 sim.wait_cycle(), it maybe rewrite PRER_LO in this cycle

    # rd PRER_HI, rd(addr)
    dut.wb_adr_i.value_def =  PRER_HI
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_adr_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task read done ", "( addr:", PRER_HI, ")")
    # check read data
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_HI"))
    
    # wb_write(1, CTR,     8'h80)
    dut.wb_adr_i.value_def =  CTR
    dut.wb_dat_i.value_def =  0x80
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", CTR, "data:", 0x80, ")")

    """
        access slave (write)
    """
    # wb_write(1, TXR, {SADR,WR} )
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  (SADR << 1) | WR
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", TXR, "data:", (SADR << 1) | WR, ")")

    # wb_write(0, CR,      8'h90 )
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x90
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", CR, "data:", 0x90, ")")

    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, TXR,     8'h01)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0x01
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", TXR, "data:", 0x01, ")")

    # wb_write(0, CR,      8'h10)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x10
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", CR, "data:", 0x10, ")")

    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, TXR,     8'ha5)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0xa5
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", TXR, "data:", 0xa5, ")")

    # wb_write(0, CR,      8'h10)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x10
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task write done", "( addr:", CR, "data:", 0x10, ")")

    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cycle()
    sim.wait_cond(dut.wb_ack_o.value == 1)
    dut.wb_cyc_i.value_def =  0
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    print("run1 done")

dut = pywasim.Dut('../../design/pywasim-test/i2c.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
dut.print_curr_sv()

reset(sim, dut, pywasim)
run1(sim, dut, pywasim)

pywasim.start_loop(sim, dut, 50)
print("branch num:", len(dut.branch_list))
    
  
