import os
import sys

script_path = os.path.realpath(__file__)
parent_dir = os.path.dirname(os.path.dirname(os.path.dirname(script_path)))
build_dir = os.path.join(parent_dir, 'build')
sys.path.append(build_dir)

import pywasimbase
ts = pywasimbase.TransSys('../../design/test/pipe.btor2','A::')

slv = ts.get_solver()
ts2 = pywasimbase.TransSys('../../design/test/pipe.btor2', slv, 'B::')

updates = ts.state_updates()
for s,e in updates.items():
  print(s,e.to_string())
  print (e.get_op())
  

prop = ts.prop()[0]
prop_prev = prop.substitute(updates)
print (prop)
print (prop_prev)

init_constant = ts.init_constants()
print (init_constant)
init_constant2 = ts2.init_constants()
print (init_constant2)

sim = pywasimbase.Symsimulator(ts)
v1 = sim.set_var(8, 'v1')
print(v1)
v2 = sim.set_var(8, 'v2')
print(v1 + v2)



