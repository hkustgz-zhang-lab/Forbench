import pywasim_async as pywasim
# import pywasim_async_test as pywasim
pywasim._extra_globals = globals()  # send global env to pywasim_async
pywasim.multi_branch = False

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
def run1(sim, dut, pywasim):  # program internal registers
    """
        reset system
    """

    dut.arst_i.value = 1
    dut.wb_rst_i.value_def = 0
    dut.wb_cyc_i.value_def =  0
    # dut.scl_pad_i.value_def =  1
    # dut.sda_pad_i.value_def =  1
    sim.wait_cycle()
    dut.arst_i.value = 0
    sim.wait_cycle(20)
    dut.arst_i.value_def = 1    # forever arst_i = 1
    sim.wait_cycle()
    print("task reset done")

    """
        program internal registers
    """

    # wb_write(1, PRER_LO, 8'hc8), we can use symbolic simulation to verify PRER_LO and PRER_HI register
    # sim.wait_cycle()
    dut.wb_adr_i.value_def =  PRER_LO
    dut.wb_dat_i.value_def =  "PRER_LO" # 0xc8
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr: PRER_LO, data: PRER_LO)")

    # wb_write(1, PRER_HI, 8'h00)
    dut.wb_adr_i.value_def =  PRER_HI
    dut.wb_dat_i.value_def =  "PRER_HI" # 0x00
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr: PRER_HI, data: PRER_HI)")

    # wb_cmp(0, PRER_LO, 8'hc8)
    dut.wb_adr_i.value_def =  PRER_LO
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr: PRER_LO)")
    # check read data
    print(dut.wb_dat_o.value)
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_LO"))
    # sim.check_valid(dut.wb_dat_o.value == 0xc8)
    sim.wait_cycle()
        # find new error -> dut.wb_dat_o.value: (ite (= #b1 (bvand wb_we_iX7 (ite (= #b000 wb_adr_iX7) #b1 #b0))) wb_dat_iX7 PRER_LO)
        # so need to remove line 42 sim.wait_cycle(), it maybe rewrite "PRER_LO" register in this cycle

    # wb_cmp(0, PRER_HI, 8'h00)
    dut.wb_adr_i.value_def =  PRER_HI
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr: PRER_HI)")
    # check read data
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_HI"))
    # sim.check_valid(dut.wb_dat_o.value == 0x00)
    sim.wait_cycle()
    
    # wb_write(1, CTR,     8'h80), enable core
    dut.wb_adr_i.value_def =  CTR
    dut.wb_dat_i.value_def =  0x80
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr: CTR, data: 0x80)")

    """
        access slave (write)
    """

    # wb_write(1, TXR,     8'ha0)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0xa0
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", 0xa0, ")")
    # wb_write(0, CR,      8'h90 )
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x90
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x90, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    # sim.wait_cond(dut.sda_padoen_o.value == 0)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero ------------------------------------------

    # wb_write(1, TXR,     8'h01)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0x01
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", 0x01, ")")
    # wb_write(0, CR,      8'h10)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x10
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x10, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, TXR,     8'ha5)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0xa5
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", 0xa5, ")")
    # wb_write(0, CR,      8'h10)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x10
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x10, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    """
        access slave (read)
    """

    # wb_write(1, TXR, {SADR,WR} )
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  (SADR << 1) | WR
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", (SADR << 1) | WR, ")")
    # wb_write(0, CR,      8'h90 )
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x90
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x90, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, TXR,     8'h01)
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  0x01
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", 0x01, ")")
    # wb_write(0, CR,      8'h10)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x10
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x10, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, TXR, {SADR,RD} )
    dut.wb_adr_i.value_def =  TXR
    dut.wb_dat_i.value_def =  (SADR << 1) | RD
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", TXR, "data:", (SADR << 1) | RD, ")")
    # wb_write(0, CR,      8'h90 )
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x90
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x90, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_write(1, CR,      8'h20)
    dut.wb_adr_i.value_def =  CR
    dut.wb_dat_i.value_def =  0x20
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    print("task write done", "( addr:", CR, "data:", 0x20, ")")
    # check tip bit
    # wb_read(1, SR, q)
    dut.wb_adr_i.value_def =  SR
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", SR, ")")
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    # wb_read(1, RXR, qq)
    dut.wb_adr_i.value_def =  RXR
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("task read done ", "( addr:", RXR, ")")
    print("RXR qq:", dut.wb_dat_o.value)
    # sim.check_valid(dut.wb_dat_o.value ==  0xa5)   # check read data
    # the signal var depends on sda_pad_i and scl_pad_i, so need the i2c_slave_model respond sda_pad_i, scl_pad_i
    sim.wait_cycle()

    print("run1 done")

dut = pywasim.Dut('../../design/pywasim-test/i2c.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
dut.print_curr_sv()

# reset(sim, dut, pywasim)
run1(sim, dut, pywasim)

pywasim.start_loop(sim, dut, 1000)
print("branch num:", len(dut.branch_list))
    
  
