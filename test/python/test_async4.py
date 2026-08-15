import pywasim_async as pywasim

# always block 1
@pywasim.register_task
def run1(sim, dut):  # 
    dut.a.value = 'a0'
    print (dut.a.value)
    sim.wait_cycle()
    print (dut.a.value) # this should be different from the first print!

dut = pywasim.Dut('../../design/pywasim-test/adder_async.btor2')
sim = pywasim.async_simulator(dut)
dut.set_init()

run1(sim, dut)  

sim.globalvars = globals()
pywasim.start_loop(sim, dut, 100)
print("branch num:", len(dut.branch_list))
