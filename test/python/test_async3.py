import pywasim_async as pywasim

# always block 1
@pywasim.register_task
def run1(sim, dut):  # 
    dut.a.value = 'a0'
    print("run1")
    bval = f1(sim, dut, 'b0')
    sim.check_assertion(dut.out1.value == pywasim.zero_extend(sim.get_var('a0'),1) + pywasim.zero_extend(bval,1))

def f1(sim, dut, val):
    dut.b.value = val
    print('f1:', val)
    v = f2(sim, dut, val)
    return v

def f2(sim,dut, val):
    print('f2')
    sim.wait_cycle()
    return sim.get_var(val)

# always block 2
@pywasim.register_task
def run2(sim, dut):  # 
    dut.c.value = 'c0'
    dut.d.value = 'd0'
    print("run2")
    sim.wait_cycle()
    sim.check_assertion(dut.out2.value == pywasim.zero_extend(sim.get_var('c0'),1) + pywasim.zero_extend(sim.get_var('d0'),1))

dut = pywasim.Dut('../../design/pywasim-test/adder_async.btor2')
sim = pywasim.async_simulator(dut)

dut.set_init()
run1(sim, dut)  
pywasim.run_later(run2, sim, dut) # same as `run2(sim, dut)`
sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
