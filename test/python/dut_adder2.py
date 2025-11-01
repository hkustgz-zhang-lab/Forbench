from pywasim import Dut, zero_extend


if __name__ == "__main__":
    dut = Dut('../../design/pywasim-test/adder2.btor2')    # create dut

    # init dut
    dut.set_init()              # init state value
    dut.print_curr_sv()         # print current state value

    dut.a.value = "a1"          # set input value
    dut.b.value = "b1"
    a1 = dut.a.value
    dut.step()                  # sim one step

    # next cycle
    dut.print_curr_sv()
    dut.a.value = "a2"
    dut.b.value = "b2"
    b2 = dut.b.value
    dut.step()

    # next cycle
    dut.print_curr_sv()
    dut.a.value = "a3"
    dut.b.value = "b3"
    a3 = dut.a.value
    out = dut.out.value
    # assert
    dut.check_assertion(out == zero_extend(a1, 1) + zero_extend(b2, 1) + zero_extend(a3, 1))