import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  #
    # reset dut
    dut.rst.value = 1
    sim.wait_cycle()

    # set dividend and divisor, pull start up
    dut.dividend.value = "dividend"
    dut.divisor.value = "divisor"
    dut.start.value = 1
    dut.rst.value_def = 0
    dut.set_constraint(dut.divisor.value != 0)
    sim.wait_cycle()

    # pull start down, wait condition
    dut.start.value_def = 0
    sim.wait_cond((dut.valid.value == 1))
    sim.check_valid(dut.quotient.value == sim.get_var('dividend') / sim.get_var('divisor'))

dut = pywasim.Dut('../../design/pywasim-test/div.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
dut.print_curr_sv()

run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
pywasim.start_loop(sim, dut, 100)
  
