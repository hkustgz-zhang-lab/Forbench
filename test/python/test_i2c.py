import pywasim_async as pywasim

# parameter
PRER_LO = 0b000
PRER_HI = 0b001
CTR     = 0b010
RXR     = 0b011
TXR     = 0b011
CR      = 0b100
SR      = 0b100

def wb_write(sim, dut, delay, addr, data):
    sim.wait_cycle(delay)
    dut.wb_adr_i.value_def =  addr
    dut.wb_dat_i.value_def =  data
    dut.wb_we_i .value_def =  1
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    dut.wb_adr_i.unset_def()
    dut.wb_dat_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task write", "addr:", addr, "data:", data)

def wb_read(sim, dut, delay, addr):
    sim.wait_cycle(delay)
    dut.wb_adr_i.value_def =  addr
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.wait_cycle()
    dut.wb_adr_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task read", "addr:", addr)

def wb_cmp(sim, dut, delay, addr, exp_data):
    sim.wait_cycle(delay)
    dut.wb_adr_i.value_def =  addr
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond(dut.wb_ack_o.value == 1)
    sim.check_assertion(dut.wb_dat_o.value == exp_data)
    sim.wait_cycle()
    dut.wb_adr_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task cmp", "addr:", addr, "exp_data:", exp_data)

def check_tip_bit(sim, dut, addr):
    dut.wb_adr_i.value_def =  addr
    dut.wb_we_i .value_def =  0
    dut.wb_stb_i.value_def =  1
    dut.wb_cyc_i.value_def =  1
    sim.wait_cond((dut.wb_ack_o.value == 1) & (dut.wb_dat_o.value[1] == 0)) # poll it until it is zero
    sim.wait_cycle()
    dut.wb_adr_i.unset_def()
    dut.wb_we_i .unset_def()
    dut.wb_stb_i.unset_def()
    dut.wb_cyc_i.value_def =  0
    print("task check_tip_bit")

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

    wb_write(sim, dut, 1, PRER_LO, 0x01)
    wb_write(sim, dut, 1, PRER_HI, 0x00)

    wb_cmp(sim, dut, 0, PRER_LO, 0x01)
    wb_cmp(sim, dut, 0, PRER_HI, 0x00)

    wb_write(sim, dut, 1, CTR, 0x80)

    """
        access slave (write)
    """

    # drive slave address
    wb_write(sim, dut, 1, TXR, 0xa0)
    wb_write(sim, dut, 0, CR, 0x90)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # send memory address
    wb_write(sim, dut, 1, TXR, 0x01)
    wb_write(sim, dut, 0, CR, 0x10)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # send memory contents
    wb_write(sim, dut, 1, TXR, 0xa5)
    wb_write(sim, dut, 0, CR, 0x10)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # send memory contents for next memory address (auto_inc)
    wb_write(sim, dut, 1, TXR, 0x5a)
    wb_write(sim, dut, 0, CR, 0x50)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # """
    #     access slave (read)
    # """

    # drive slave address
    # send memory contents
    wb_write(sim, dut, 1, TXR, 0xa0)
    wb_write(sim, dut, 0, CR, 0x90)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # send memory address
    wb_write(sim, dut, 1, TXR, 0x01)
    wb_write(sim, dut, 0, CR, 0x10)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # drive slave address
    wb_write(sim, dut, 1, TXR, 0xa1)
    wb_write(sim, dut, 0, CR, 0x90)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # read data from slave
    wb_write(sim, dut, 1, CR, 0x20)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # check data just received
    # wb_cmp(1, RXR, 0xa5)
    wb_read(sim, dut, 1, RXR)
    qq = dut.wb_dat_o.value
    print("RXR qq:", dut.wb_dat_o.value)
    # sim.check_assertion(qq == 0xa5)

    # read data from slave
    wb_write(sim, dut, 1, CR, 0x20)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # # check data just received
    # wb_cmp(1, RXR, 0x5a)
    wb_read(sim, dut, 1, RXR)
    qq = dut.wb_dat_o.value
    print("RXR qq:", dut.wb_dat_o.value)
    # sim.check_assertion(qq == 0xa5)

    # read data from slave
    wb_write(sim, dut, 1, CR, 0x20)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # # check data just received
    wb_read(sim, dut, 1, RXR)
    print("RXR qq:", dut.wb_dat_o.value)
    print("Expected 3th XX")

    # read data from slave
    wb_write(sim, dut, 1, CR, 0x28)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # # check data just received
    wb_read(sim, dut, 1, RXR)
    print("RXR qq:", dut.wb_dat_o.value)
    print("Expected 4th XX")

    # """
    #     check invalid slave memory address
    # """

    # drive slave address
    wb_write(sim, dut, 1, TXR, 0xa0)
    wb_write(sim, dut, 0, CR, 0x90)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    # send memory address
    wb_write(sim, dut, 1, TXR, 0x10)
    wb_write(sim, dut, 0, CR, 0x10)
    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)
    
    # slave should have send NACK
    q = dut.wb_dat_o.value
    sim.check_assertion(q[7] == 1)

    # read data from slave
    wb_write(sim, dut, 1, CR, 0x40)

    # check tip bit
    wb_read(sim, dut, 1, SR)
    check_tip_bit(sim, dut, SR)

    print("run1 done")

dut = pywasim.Dut('../../design/pywasim-test/i2c_master_slave_top.btor2')
sim = pywasim.async_simulator(dut)
sim.globalvars = globals()  # make helper tasks visible to the tracked coroutine

dut.set_init({"i2c_slave.sda_o" : 1, "i2c_slave.state" : 0b000})
dut.print_curr_sv_all_branches()

run1(sim, dut, pywasim)

pywasim.start_loop(sim, dut, 10000)
print("branch num:", len(dut.branch_list))
    
  
