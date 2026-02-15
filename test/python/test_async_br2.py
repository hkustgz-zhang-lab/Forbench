import pywasim_async as pywasim

@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cycle()
    dut.start.value = 0
    triggered = False
    # 2 questions: why sim.check_assertion has to be inside
    # why triggered not ... ?
    # simplier way to write string as assertions f"`a0` == {i}"   ,  "dut.result == {3'b0,`a0`}*{3'b0,`b0`}"
    # configuration? 
    # coroutine id: python function name & branch id
    # know which is assertion fails and also the surroundings?  debug in this context...
    for i in range(0,8): # not including 8
        if not triggered and dut.valid.value == 1:
            print ('i=',i)
            sim.check_assertion(sim.get_var('b0') == i)
            sim.check_assertion(dut.result.value == pywasim.zero_extend(sim.get_var('a0'),3)*pywasim.zero_extend(sim.get_var('b0'),8))
            triggered = True
        sim.wait_cycle()
        print ('i=',i,  'branch id=',dut.curr_branch_idx, '  triggered=',triggered)

    print ('eof branch id=',dut.curr_branch_idx, '  triggered=',triggered)
    assert triggered
        
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
    
  
