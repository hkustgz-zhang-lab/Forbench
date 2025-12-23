import pywasim_async as pywasim

# always block 1
@pywasim.register_task
def run1(sim, dut):  # 
    for i in range(10):
        n_cycle = sim.current_cycle()
        print (f'iteration:{i} current cycle:{n_cycle}')
        avar = f'a{i}'
        bvar = f'b{i}'
        dut.a.value = avar
        dut.b.value = bvar
        sim.wait_cycle()
        sim.check_assertion(dut.out1.value == pywasim.zero_extend(sim.get_var(avar),1) + pywasim.zero_extend(sim.get_var(bvar),1))

dut = pywasim.Dut('../../design/pywasim-test/adder_async.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut)  # pywasim.run_later(run1(sim, dut, pywasim))
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
