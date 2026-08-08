import os
import sys

script_path = os.path.realpath(__file__)
parent_dir = os.path.dirname(os.path.dirname(os.path.dirname(script_path)))
build_dir = os.path.join(parent_dir, 'build')
sys.path.append(build_dir)

from pywasimbase import *
# TransSys, Simsimulator
from pywasim_log import debug_log, msg_log, warn_signal_contains_current_input

class Dut:
    def __init__(self, btorname):
        self.ts = TransSys(btorname)
        self.simulator = Symsimulator(self.ts)
        self.solver = self.simulator.get_solver()

        self.inputvars_list = self.ts.inputvars()
        self.statevars_list = self.ts.statevars()

        self.iv_term_dict = {}
        self.iv_term_dict_default = {}
        self.constraints = []

        self.initialized = False
        self.prop = self._get_property()

    def _get_property(self):
        prop_list = self.ts.prop()
        if not prop_list:
            msg_log("No property to check!")
            return None
        elif len(prop_list) == 1:
            msg_log("property:", prop_list[0])
            return prop_list[0]
        else:
            prop_i = prop_list[0]
            for idx in range(1, len(prop_list)):
                # prop_i = pywasim.make_term("And", prop_i, prop_list[idx])
                prop_i = prop_i & prop_list[idx]
            msg_log("property:", prop_i)
            return prop_i

    def _create_iv_dict(self):
        self.iv_term_dict = self.simulator.create_input_Xvars("") # create default inputvars term dict
        self.iv_term_dict.update(self.iv_term_dict_default) # set default inputvars

    def set_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.init(var_dict)
        self._create_iv_dict()  # create new inputvars

    def free_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.free_init(var_dict)
        self._create_iv_dict()  # create new inputvars

    def set_constraint(self, constr):
        self.constraints.append(constr)
    
    def unset_constraint(self, constr):
        self.constraints.remove(constr)

    def clear_constraint(self):
        self.constraints.clear()

    def _merge_dict_replace_X(self, iv_term_dict, iv_term_dict_default):
        # check every k,v in iv_term_dict_default, if the corresponding value in iv_term_dict is X then replace it
        # else don't replace
        retd = iv_term_dict.copy()
        for k,v in iv_term_dict_default.items():
            if k in retd:
                old_v = retd[k]
                if self.simulator.is_Xvar(old_v):
                    retd[k] = v
            else:
                retd[k] = v
        return retd
        
    def step(self, num = 1, asmpt = []):
        for _ in range(num):
            iv_term_dict = self._merge_dict_replace_X(self.iv_term_dict, self.iv_term_dict_default)
            self.simulator.set_input(iv_term_dict, asmpt)
            self.simulator.sim_one_step()
            self._create_iv_dict()  # create new inputvars

    def back_step(self):
        self.simulator.backtrack()
        self.simulator.undo_set_input()
        self._create_iv_dict()  # create new inputvars

    def current_cycle(self):
        # cycle starts from 0
        return self.simulator.tracelen()-1

    def check_prop(self):
        cur_prop = self.simulator.interpret_state_expr_on_curr_frame(self.prop)
        assumptions = self.simulator.all_assumptions()
        msg_log(f"property: {cur_prop.to_string()}")
        for a in assumptions:
            msg_log(f"assumption: {a.to_string()}")

        self.solver.push()
        for a in assumptions:
            self.solver.assert_formula(a)
        f = ~cur_prop   # make_term(not, cur_prop)
        self.solver.assert_formula(f)
        res = self.solver.check_sat()
        self.solver.pop()

        if res:
            msg_log("check prop result: fail!")
        else:
            msg_log("check prop result: pass!")
        return not res  # unsat -> return True

    def check_sat(self, asst, asmpts):
        debug_log('dut.check_sat')
        asmpts_all = self.simulator.all_assumptions()
        asmpts_all.extend(asmpts)
        asmpts_all.extend(self.constraints)
        asmpts_all.append(asst)
        return self.solver.check_sat_assuming(asmpts_all)

    def check_assertion(self, assertion):
        msg_log(f"dut.check_assertion: {assertion.to_string()}")

        self.solver.push()
        formula = ~assertion    # make_term(not, assertion)
        self.solver.assert_formula(formula)
        res = self.solver.check_sat()
        if res:
            msg_log("check assertion result: fail!")
            self.simulator.dump_waveform('cex.vcd', self.iv_term_dict, True)
        else:
            msg_log("check assertion result: pass!")
        self.solver.pop()

        return not res

    def print_curr_sv(self):
        self.simulator.print_current_step()

    def print_curr_assumptions(self):
        self.simulator.print_current_step_assumptions()

    def get_var(self, name):
        return self.simulator.get_var(name)
    
    def __getattr__(self, signal_name):
        self.ts.lookup(signal_name) # add a check to make sure the signal_name do exist
        return SignalProxy(self, signal_name)
    
    def get_signal(self, signal_name):
        # just in case some signals have the same name as the class method
        # you can still use this function to get it
        self.ts.lookup(signal_name)
        return SignalProxy(self, signal_name)
    


class SignalProxy:
    def __init__(self, dut, signal_name):
        self.dut = dut           # class Dut instance
        self.name = signal_name  # example: "a"

    @property
    def value(self):
        # if you have assigned, get the one you assigned
        if self.dut.current_cycle() < 0:
            raise Exception('Combinational circuits also need initialization (free_init)')
        iv_term_dict = self.dut.iv_term_dict

        nf = self.dut.simulator.var(self.name)
        if nf in iv_term_dict:
            warn_signal_contains_current_input(self.name)
            return iv_term_dict[nf]
        
        # get current term of signal
        try:
            signal_nr = self.dut.simulator.interpret_state_expr_on_curr_frame(nf)   # only have state vars
            return signal_nr
        except Exception:
            signal_nr = self.dut.simulator.interpret_input_and_state_expr_on_curr_frame(nf, iv_term_dict)  # have state vars and input vars
            warn_signal_contains_current_input(self.name)
            return signal_nr

    @value.setter
    def value(self, iv):
        # set input_signal <-> value
        iv_term_dict = self.dut.iv_term_dict
        try:
            iv_nr = self.dut.simulator.var(self.name)
            if self.dut.ts.is_input_var(iv_nr):
                iv_dict = self.dut.simulator.convert({self.name : iv})
                iv_term_dict.update(iv_dict)
            else:
                raise ValueError(f"No such input variable '{self.name}'.")
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)

    @property
    def value_def(self):
        return None

    @value_def.setter
    def value_def(self, iv):
        iv_term_dict = self.dut.iv_term_dict
        iv_term_dict_default = self.dut.iv_term_dict_default
        try:
            iv_nr = self.dut.simulator.var(self.name)
            if self.dut.ts.is_input_var(iv_nr):
                iv_dict = self.dut.simulator.convert({self.name : iv})
                iv_term_dict_default.update(iv_dict)
                # will only replace the value in iv_term_dict if iv_term_dict has an X there
                iv_term_dict_updated = self.dut._merge_dict_replace_X(iv_term_dict, iv_dict)
                self.dut.iv_term_dict = iv_term_dict_updated
            else:
                raise ValueError(f"No such input variable '{self.name}'.")
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)
        
    def unset_def(self):
        iv_term_dict = self.dut.iv_term_dict
        iv_term_dict_default = self.dut.iv_term_dict_default

        iv_nr = self.dut.simulator.var(self.name)
        if iv_nr not in self.dut.iv_term_dict_default:
            raise ValueError(f"No such default assignment to variable '{self.name}'.")
        def_val = iv_term_dict_default[iv_nr]
        del iv_term_dict_default[iv_nr]
        # reset to X signal only if iv_term_dict has the same value there
        # if you set the new value, unset_def will not affect this
        if hash(iv_term_dict[iv_nr]) == hash(def_val):
            xvar_dict = self.dut.simulator.create_input_Xvars(self.name)
            xvar = list(xvar_dict.items())[0][1]
            iv_term_dict[iv_nr] = xvar
