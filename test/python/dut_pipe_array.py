from pywasim import Dut


if __name__ == "__main__":
    dut = Dut('../../design/asynctest/simplepipe-3stage/simple_pipe_stall_array.btor2')     # create dut

    # init dut
    dut.set_init()              # init state value
    dut.print_curr_sv()         # print current state value
    
    dut.inst.value = "inst1"          # set input value
    dut.rst.value = 0
    dut.stallex.value = 0
    dut.stallwb.value = 0
    dut.step()                  # sim one step

    # next cycle
    dut.print_curr_sv()
    dut.inst.value = "inst2"          # set input value
    dut.rst.value = 0
    dut.stallex.value = 1
    dut.stallwb.value = 0
    dut.step()

    # next cycle
    dut.print_curr_sv()
    
