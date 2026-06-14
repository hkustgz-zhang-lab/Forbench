import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.check_assertion(dut.result.value == sim.get_var('a0')+sim.get_var('b0'))
    dut.start.value = 0
    sim.check_assertion(dut.result.value == 0)
    
    

dut = pywasim.Dut('../../design/asynctest/comb/test.btor2')
sim = pywasim.async_simulator(dut)

dut.free_init()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
