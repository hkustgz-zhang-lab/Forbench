import pywasim_async as pywasim

# alwasys block 1
@pywasim.register_task
def run1(sim, dut, pywasim):  # 
    dut.a.value = 'a0'
    dut.b.value = 'b0'
    print("run1")
    sim.wait_cycle()
    sim.check_valid(dut.out1.value == pywasim.zero_extend(sim.get_var('a0'),1) + pywasim.zero_extend(sim.get_var('b0'),1))

# alwasys block 2
@pywasim.register_task
def run2(sim, dut, pywasim):  # 
    dut.c.value = 'c0'
    dut.d.value = 'd0'
    print("run2")
    sim.wait_cycle()
    sim.check_valid(dut.out2.value == pywasim.zero_extend(sim.get_var('c0'),1) + pywasim.zero_extend(sim.get_var('d0'),1))

dut = pywasim.Dut('../../design/pywasim-test/adder_async.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut, pywasim)  # pywasim.run_later(run1(sim, dut, pywasim))
run2(sim, dut, pywasim)
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
