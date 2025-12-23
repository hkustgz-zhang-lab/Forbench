import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    i = 0
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cycle()
    while dut.valid.value == 0:
        sim.wait_cycle()
    sim.check_assertion(dut.valid.value == 1)
    sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
    
    # while True:
    #     if dut.valid.value == 1:
    #         sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
    #         break
    #     else:
    #         sim.wait_cycle()

    # This is different from
    # while dut.valid.value == 1:
    #    sim.check_assertion(...)
    #    sim.wait_cycle(...)
        


dut = pywasim.Dut('../../design/asynctest/mul/mul.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
