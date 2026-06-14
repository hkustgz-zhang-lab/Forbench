from pywasim import Dut


if __name__ == "__main__":
    dut = Dut('../../design/asynctest/comb/test.btor2')     # create dut

    # init dut
    dut.set_init()              # init state value
    
    dut.a.value = "a1"          # set input value
    dut.b.value = "b1"
    dut.start.value = 1
    dut.check_assertion(dut.result.value == dut.get_var('a1')+dut.get_var('b1'))
    dut.start.value = 0
    dut.check_assertion(dut.result.value == 0)
    

    
