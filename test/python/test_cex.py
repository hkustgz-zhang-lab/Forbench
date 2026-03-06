import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cond(dut.valid.value == 1)
    # this is expected to fail
    x = 1; y=x-x; z=x/y
    sim.check_assertion(dut.result.value == 0)

dut = pywasim.Dut('../../design/asynctest/mul/mul.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
