import pywasim_async as pywasim

def implies(a,b):
    return (~a) | (b)

Method = 1 # 2/3   # Method 2 seems slower, but why?

@pywasim.register_task
def run1(sim, dut):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    for i in range(64):
        sim.wait_cycle()
        n_cycle = sim.current_cycle()
        dut.start.value = 0
        # with sim.if(dut.valid.value == 1):
        #     pass
        #     sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
        #     sim.check_assertion('result == a0*b0')
        #     f = dut.interpret('result == { 3'b0 , `a0` }*{ 8'd0 , `b0` }')  # symbolic value can be referred by `name`
        
        # this will fail
        # sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
        if Method == 1:
            with sim.assume(dut.valid.value == 1): # this push an asumption (will be pop out after leaving this context)
                # this will not branch. `as possible` is not required. and you should not use this unless, you turn on `check_possible`
                # if you don't turn on check possible, possible will always be true ...
                print (f'M1: Cycle {n_cycle}: assume cond may sat. Check assertion then.')
                sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
                # sim.wait_cycle() # this should raise error, you are not allow to step the DUT in if ...
                # but you are allow to step the design in if ...
        elif Method == 2:
            with sim.assume(dut.valid.value == 1, check_possible=True) as possible:
                if possible:
                    print (f'M2: Cycle {n_cycle}: cond may sat. Check assertion then.')
                    sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
                else:
                    print (f'M2: Cycle {n_cycle}: cond cannot sat.')
        elif Method == 3:
            cond = (dut.valid.value == 1)
            can_sat = sim.check_sat(cond)
            if can_sat:
                f = implies(dut.valid.value == 1, dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
                # print (f)
                print (f'M3: Cycle {n_cycle}: cond may sat. Check assertion then.')
                sim.check_assertion(f)
            else:
                print (f'M3: Cycle {n_cycle}: cond cannot sat.')
        else:
            assert False

# Note that, you don't really need to go to 64 cycle in fact
# But currently we don't know that ...


dut = pywasim.Dut('../../design/asynctest/mul/mul_free_restart.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut)
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
