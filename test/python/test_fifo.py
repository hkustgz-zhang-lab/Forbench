import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):
    # reset dut
    dut.rst_n.value = 0
    sim.wait_cycle()

    # wait condition, get data_in
    dut.rst_n.value_def = 1
    dut.wr_en.value = 1
    dut.data_in.value = "data_in"
    data_in = dut.data_in.value

    # wait condition, check data_out
    sim.wait_cond(dut.valid_o.value == 1)
    sim.check_assertion(data_in == dut.data_out.value)
    
    
dut = pywasim.Dut('../../design/pywasim-test/fifo.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
dut.print_curr_sv()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))

pywasim.start_loop(sim, dut, 10)
  
