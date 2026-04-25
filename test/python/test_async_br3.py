import pywasim_async as pywasim

@pywasim.register_task
def test_full(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cycle()
    dut.start.value = 0
    triggered = False
    # simplier way to write string as assertions f"`a0` == {i}"   ,  "dut.result == {3'b0,`a0`}*{3'b0,`b0`}"
    # configuration? 
    # coroutine id: python function name & branch id
    # know which is assertion fails and also the surroundings?  debug in this context...
    for i in range(0,8): # not including 8
        if not triggered:
            if dut.valid.value == 1:
                print (f'i:{i} valid: 0->1')
                sim.check_assertion(sim.get_var('b0') == i)
                sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
                triggered = True
        else: # triggered
            print (f'i:{i} valid: stable@1')
            sim.check_assertion(dut.valid.value == 1)
            sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
        sim.wait_cycle()
        dut.start.value = 0
    assert triggered
    sim.check_assertion(dut.valid.value == 1)
    sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
        

@pywasim.register_task
def test_simple(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cycle()
    dut.start.value = 0
    triggered = False
    # simplier way to write string as assertions f"`a0` == {i}"   ,  "dut.result == {3'b0,`a0`}*{3'b0,`b0`}"
    # configuration? 
    # coroutine id: python function name & branch id
    # know which is assertion fails and also the surroundings?  debug in this context...
    for i in range(0,8): # not including 8

        print('---------------------------')
        print('cycle:',i)
        print('state of branch #',sim.get_current_branch_id())
        print('rega = ', dut.rega.value)
        print('regb = ', dut.regb.value)
        print('---------------------------')

        if not triggered:
            if dut.valid.value == 1:
                print (f'i:{i} valid: 0->1')
                sim.check_assertion(sim.get_var('b0') == i)
                sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
                triggered = True
        sim.wait_cycle()
        dut.start.value = 0
    assert triggered

dut = pywasim.Dut('../../design/asynctest/mul/mul_free_restart.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
test_full(sim, dut, pywasim) # you should only uncomment 1 of them
#test_simple(sim, dut, pywasim)
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
    
  
