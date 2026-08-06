import pywasim_async as pywasim

@pywasim.register_task
def multiply(sim, dut, pywasim):
    dut.a.value = "a0"
    dut.b.value = "b0"
    dut.start.value = 1

    sim.wait_cond(dut.valid.value == 1)

    expected = (
        pywasim.zero_extend(sim.get_var("a0"), 3)
        * pywasim.zero_extend(sim.get_var("b0"), 8)
    )
    sim.check_assertion(dut.result.value == expected)

dut = pywasim.Dut("../../design/asynctest/mul/mul.btor2")
sim = pywasim.async_simulator(dut)

dut.set_init()
multiply(sim, dut, pywasim)
pywasim.start_loop(sim, dut, 100)

