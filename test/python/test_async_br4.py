import pywasim_async as pywasim

# pywasim._debug= True

@pywasim.register_task
def stimulus(sim, dut):
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    dut.start.value = 1
    sim.wait_cycle()
    dut.start.value = 0
    while True:
        if dut.valid.value == 1:
            break
        sim.wait_cycle()
        dut.start.value = 0


@pywasim.register_task
def monitor(sim, dut):  # 
    sim.wait_cycle()
    triggered = False
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


dut = pywasim.Dut('../../design/asynctest/mul/mul_free_restart.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
stimulus(sim, dut)
monitor(sim, dut)
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))

