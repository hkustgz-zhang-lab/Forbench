import pywasim_async as pywasim

# always block 1
@pywasim.register_task
def run1(sim, dut):  # 
    for i in range(10):
        n_cycle = sim.current_cycle()
        print (f'run1: iteration:{i} current cycle:{n_cycle}')
        avar = f'a{n_cycle}'
        bvar = f'b{n_cycle}'
        dut.a.value = avar
        dut.b.value = bvar
        sim.wait_cycle()
        sim.check_assertion(dut.out1.value == pywasim.zero_extend(sim.get_var(avar),1) + pywasim.zero_extend(sim.get_var(bvar),1))
        sim.wait_cycle()


# always block 2
@pywasim.register_task
def run2(sim, dut):  # 
    for i in range(10):
        sim.wait_cycle()
        n_cycle = sim.current_cycle()
        print (f'run2: iteration:{i} current cycle:{n_cycle}')
        avar = f'a{n_cycle}'
        bvar = f'b{n_cycle}'
        dut.a.value = avar
        dut.b.value = bvar
        sim.wait_cycle()
        sim.check_assertion(dut.out1.value == pywasim.zero_extend(sim.get_var(avar),1) + pywasim.zero_extend(sim.get_var(bvar),1))

dut = pywasim.Dut('../../design/pywasim-test/adder_async.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut)  # pywasim.run_later(run1(sim, dut, pywasim))
run2(sim, dut) 
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
