from pywasim import Dut, zero_extend

dut = Dut("../../design/pywasim-test/adder.btor2")
dut.set_init()

dut.a.value = "a1"
dut.b.value = "b1"
a1 = dut.a.value
dut.step()

dut.a.value = "a2"
dut.b.value = "b2"
b2 = dut.b.value
dut.step()

dut.check_assertion(
    dut.out.value == zero_extend(a1, 1) + zero_extend(b2, 1)
)
