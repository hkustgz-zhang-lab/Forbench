import pywasim_async as pywasim

def implies(a,b):
    return (~a) | (b)

@pywasim.register_task
def run1(sim, dut):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    for i in range(64):
        sim.wait_cycle()
        n_cycle = sim.current_cycle()
        dut.start.value = 0
        cond = (dut.valid.value == 1)
        can_sat = sim.check_sat(cond)
        if can_sat:
            f = implies(dut.valid.value == 1, dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
            # print (f)
            print (f'Cycle {n_cycle}: cond may sat. Check assertion then.')
            sim.check_assertion(f)
        else:
            print (f'Cycle {n_cycle}: cond cannot sat.')

# Note that, you don't really need to go to 64 cycle in fact
# But currently we don't know that ...


dut = pywasim.Dut('../../design/asynctest/mul/mul_free_restart.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut)
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
