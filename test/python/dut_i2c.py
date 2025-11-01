# from pywasim import Dut
from pywasim import Dut
	# // wishbone signals
	# input        wb_clk_i;     // master clock input
	# input        wb_rst_i;     // synchronous active high reset
	# input        arst_i;       // asynchronous reset
	# input  [2:0] wb_adr_i;     // lower address bits
	# input  [7:0] wb_dat_i;     // databus input
	# output [7:0] wb_dat_o;     // databus output
	# input        wb_we_i;      // write enable input
	# input        wb_stb_i;     // stobe/core select signal
	# input        wb_cyc_i;     // valid bus cycle input
	# output       wb_ack_o;     // bus cycle acknowledge output
	# output       wb_inta_o;    // interrupt request signal output

    # // I2C signals
	# // i2c clock line
	# input  scl_pad_i;       // SCL-line input
	# output scl_pad_o;       // SCL-line output (always 1'b0)
	# output scl_padoen_o;    // SCL-line output enable (active low)

	# // i2c data line
	# input  sda_pad_i;       // SDA-line input
	# output sda_pad_o;       // SDA-line output (always 1'b0)
	# output sda_padoen_o;    // SDA-line output enable (active low)


def task_reset(dut):
    # arst_i = 1
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0

    dut.step()

    # arst_i = 0
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 0
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0

    dut.step()

    # arst_i = 1
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1
    dut.wb_we_i .value =  0
    dut.wb_stb_i.value =  0
    dut.wb_cyc_i.value =  0

    dut.step()

    print("task reset done")

def task_write(dut, addr, data):
    # wr request
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1

    dut.wb_adr_i.value =  addr
    dut.wb_dat_i.value =  data
    dut.wb_we_i .value =  1   # pull up
    dut.wb_stb_i.value =  1   # pull up
    dut.wb_cyc_i.value =  1   # pull up
    
    dut.step()

    # write(addr, data)
    while not dut.check_assertion(dut.wb_ack_o.value == 1):
        # keep waiting wb_ack_o == 1
        dut.wb_rst_i.value = 0
        dut.arst_i.value = 1

        dut.wb_adr_i.value =  addr
        dut.wb_dat_i.value =  data
        dut.wb_we_i .value =  1
        dut.wb_stb_i.value =  1
        dut.wb_cyc_i.value =  1

        dut.step()
    
    # write
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1

    dut.wb_adr_i.value =  addr
    dut.wb_dat_i.value =  data
    dut.wb_we_i .value =  1
    dut.wb_stb_i.value =  1
    dut.wb_cyc_i.value =  1
    
    dut.step()

    print("task write done", "( addr:", addr, "data:", data, ")")

def task_read(dut, addr):
    # wr request
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1

    dut.wb_adr_i.value =  addr
    dut.wb_dat_i.value =  0   # X
    dut.wb_we_i .value =  1   # pull up
    dut.wb_stb_i.value =  1   # pull up
    dut.wb_cyc_i.value =  1   # pull up
    
    dut.step()

    while not dut.check_assertion(dut.wb_ack_o.value == 1):
        # keep waiting wb_ack_o == 1
        dut.wb_rst_i.value = 0
        dut.arst_i.value = 1

        dut.wb_adr_i.value =  addr
        dut.wb_dat_i.value =  0   # X
        dut.wb_we_i .value =  1   # pull up
        dut.wb_stb_i.value =  1   # pull up
        dut.wb_cyc_i.value =  1   # pull up

    # rd
    dut.wb_rst_i.value = 0
    dut.arst_i.value = 1
    
    dut.wb_adr_i.value =  addr
    dut.wb_dat_i.value =  0     # X
    dut.wb_we_i .value =  0     # X

    dut.wb_stb_i.value =  0     # X
    dut.wb_cyc_i.value =  0

    dut.step()
    
    print("task read done ", "( addr:", addr, ")")
    return dut.wb_dat_o.value

def task_cmp(dut, addr, data_exp):
    data = task_read(dut, addr)

    # if data_exp is symbolic, need to get noderef by solver -> get_symbol()
    # data_exp = ...

    # if data_exp is constant, just check
    if not dut.check_assertion(data == data_exp):
        raise Exception(f"Data mismatch at addr {addr}: expected {data_exp}, got {data}")
    
    print("task cmp done")

if __name__ == "__main__":
    dut = Dut('../../design/pywasim-test/i2c.btor2')     # create dut

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

    # init dut
    dut.set_init()
    dut.print_curr_sv()
    
    task_reset(dut) # reset i2c

    task_write(dut, PRER_LO, "PRER_LO")
    task_cmp(dut, PRER_LO, dut.simulator.get_var("PRER_LO"))

    task_write(dut, PRER_LO, 0xfa)
    task_write(dut, PRER_LO, 0xc8)
    task_write(dut, PRER_HI, 0x00)

    task_cmp(dut, PRER_LO, 0xc8)
    task_cmp(dut, PRER_HI, 0x00)

    task_write(dut, CTR, 0x80)
    task_write(dut, TXR, (SADR << 1) | WR)
    task_write(dut,  CR, 0x90)
    
    q = task_read(dut, SR)
    # while q[1]:                 # wait for tip == 0
    #     q = task_read(dut, SR)  # need i2c_slave to respond

