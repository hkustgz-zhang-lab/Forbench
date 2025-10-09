import pywasim_async as pywasim
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

def wb_write(addr, data):
    dut.wb_adr_i.value_def =  addr
    dut.wb_dat_i.value_def =  data
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    print("task write", "addr:", addr, "data:", data)

def wb_read(addr):
    dut.wb_adr_i.value_def =  addr
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    print("task read", "addr:", addr)

@pywasim.register_task
def run1(sim, dut, pywasim):  # program internal registers
    """
        reset system
    """

    dut.arst_i.value = 1
    dut.wb_rst_i.value_def = 0
    dut.wb_cyc_i.value_def =  0
    sim.wait_cycle()
    dut.arst_i.value_def = 0
    sim.wait_cycle(20)
    dut.arst_i.value_def = 1    # forever arst_i = 1
    sim.wait_cycle()
    print("task reset done")
    
    """
        program internal registers
    """

    # sim.wait_cycle()
    wb_write(PRER_LO, "PRER_LO")    # 0xc8  we can use symbolic simulation to verify PRER_LO and PRER_HI register
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()

    wb_write(PRER_HI, "PRER_HI")    # 0x00
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()

    wb_read(PRER_LO)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_LO"))   # check read data 0xc8
    sim.wait_cycle()
        # find new error -> dut.wb_dat_o.value: (ite (= #b1 (bvand wb_we_iX7 (ite (= #b000 wb_adr_iX7) #b1 #b0))) wb_dat_iX7 PRER_LO)
        # so need to remove sim.wait_cycle() before first write, it maybe rewrite "PRER_LO" register in this cycle

    wb_read(PRER_HI)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.check_valid(dut.wb_dat_o.value == sim.get_var("PRER_HI"))   # check read data 0x00
    sim.wait_cycle()
    
    wb_write(CTR, 0x80) # enable core
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()

    """
        access slave (write)
    """

    wb_write(TXR, 0xa0)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x90)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(TXR, 0x01)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x10)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(TXR, 0xa5)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x10)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(TXR, 0x5a)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x50)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    """
        access slave (read)
    """

    wb_write(TXR, 0xa0)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x90)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(TXR, 0x01)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x10)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(TXR, 0xa1)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    wb_write(CR, 0x90)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_write(CR, 0x20)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    # check tip bit
    wb_read(SR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cond(dut.wb_dat_o.value[1] == 0)   # poll it until it is zero

    wb_read(RXR)
    sim.wait_cond(dut.wb_ack_o.value == 1)
    print("RXR qq:", dut.wb_dat_o.value)
    # sim.check_valid(dut.wb_dat_o.value ==  0xa5)   # check read data, the signal var depends on sda_pad_i and scl_pad_i, so need the i2c_slave_model respond sda_pad_i, scl_pad_i
    sim.wait_cycle()

    print("run1 done")

dut = pywasim.Dut('../../design/pywasim-test/i2c.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
dut.print_curr_sv()

run1(sim, dut, pywasim)

pywasim.start_loop(sim, dut, 11000)
print("branch num:", len(dut.branch_list))
    
  
