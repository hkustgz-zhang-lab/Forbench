import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cond(dut.valid.value == 1)
    sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))

# note if the design allows restart
# this will not work

"""
@pywasim.register_task
def run2(sim, dut, a,b):  # 
    task1 = run1(sim, dut, a,b)
    sim.wait_task(task1)  # this will handle execution to run1
    assert(task1.finished)
    sim.wait_task(task1)  # this should return immediately
    return task1.return_value()
""" 

dut = pywasim.Dut('../../design/asynctest/mul.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
