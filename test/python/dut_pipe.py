from pywasim import Dut


if __name__ == "__main__":
    dut = Dut('pipe.btor2')     # create dut

    # init dut
    dut.set_init()              # init state value
    dut.print_curr_sv()         # print current state value
    
    dut.a.value = "a1"          # set input value
    dut.b.value = "b1"
    dut.rst.value = 1
    dut.step()                  # sim one step

    # next cycle
    dut.print_curr_sv()
    dut.check_prop()            # check property
    dut.a.value = "a2"
    dut.b.value = "b2"
    dut.rst.value = 0
    dut.step()

    # next cycle
    dut.print_curr_sv()
    dut.check_prop()
