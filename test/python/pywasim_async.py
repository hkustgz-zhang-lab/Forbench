import os
import sys
import inspect
import ast
import copy
from functools import wraps

script_path = os.path.realpath(__file__)
parent_dir = os.path.dirname(os.path.dirname(os.path.dirname(script_path)))
build_dir = os.path.join(parent_dir, 'build')
sys.path.append(build_dir)

from pywasimbase import *
# TransSys, Simsimulator

_all_coroutine = []
_all_states = []
cur_branch_idx = 0  # current running branch, init 0 / every state has a branch_idx
max_branch_idx = 0  # increment when create new branch
multi_branch = True

class Dut_Branch:
    def __init__(self, iv_term_dict, iv_term_dict_default, constraints):
        self.iv_term_dict = dict(iv_term_dict)
        self.iv_term_dict_default = dict(iv_term_dict_default)
        self.constraints = list(constraints)

        self.finished = False

class Dut:
    def __init__(self, btorname):
        self.ts = TransSys(btorname)
        self.simulator = Symsimbranch(self.ts)
        self.solver = self.simulator.get_solver()
        
        self._do_not_interpret_var = False # if true, will not return SignalProxy
        self.inputvars_list = self.ts.inputvars()
        self.statevars_list = self.ts.statevars()

        self.iv_term_dict = {}
        self.iv_term_dict_default = {}
        self.constraints = []

        self.initialized = False
        # self.prop = self._get_property()

        self.combination = (len(self.statevars_list) == 0)    # comb -> True, seq -> False

        self.branch_list = []

    # func for branch
    def create_origin_branch(self):
        self.branch_list.append(Dut_Branch(self.iv_term_dict, self.iv_term_dict_default, self.constraints))
        self.simulator.create_origin_branch()

    def create_branch(self):
        self.branch_list.append(Dut_Branch(self.branch_list[cur_branch_idx].iv_term_dict, self.branch_list[cur_branch_idx].iv_term_dict_default, self.branch_list[cur_branch_idx].constraints))
        global max_branch_idx
        max_branch_idx += 1
        self.simulator.create_branch(cur_branch_idx)

    def finish_branch(self):
        self.branch_list[cur_branch_idx].finished = True
        self.simulator.finish_branch(cur_branch_idx)
    
    # not use
    def _get_property(self):
        prop_list = self.ts.prop()
        if not prop_list:
            print("No property to check!")
            return None
        elif len(prop_list) == 1:
            print("property:", prop_list[0])
            return prop_list[0]
        else:
            prop_i = prop_list[0]
            for idx in range(1, len(prop_list)):
                # prop_i = pywasim.make_term("And", prop_i, prop_list[idx])
                prop_i = prop_i & prop_list[idx]
            print("property:", prop_i)
            return prop_i

    def _create_iv_dict(self):
        assert len(self.branch_list) != 0
        # create same X inputvars for all branches
        iv_dict = {}
        idx = str(self.step_cycle())
        for iv in self.inputvars_list:
            iv_dict[iv.to_string()] = iv.to_string()+ "X" + idx
        self.iv_term_dict = self.simulator.convert(iv_dict)

        # set X inputvars to each branch and update different branch default inputvars
        for branch in self.branch_list:
            if branch.finished:
                continue
            branch.iv_term_dict = self.iv_term_dict
            branch.iv_term_dict.update(branch.iv_term_dict_default)

    def set_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        assert len(self.branch_list) == 0
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.init(var_dict)
        self.create_origin_branch()     # create origin branch
        self._create_iv_dict()          # create new inputvars

    def free_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        assert len(self.branch_list) == 0
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.free_init(var_dict)
        self.create_origin_branch()     # create origin branch
        self._create_iv_dict()          # create new inputvars

    def set_constraint(self, constr):
        self.branch_list[cur_branch_idx].constraints.append(constr)
    
    def unset_constraint(self, constr):
        del self.branch_list[cur_branch_idx].constraints[constr]

    def clear_constraint(self):
        self.branch_list[cur_branch_idx].constraints = []

    def step(self, num = 1, asmpt = []):
        for _ in range(num):
            for branch_idx, branch in enumerate(self.branch_list):
                if branch.finished:
                    continue
                branch.iv_term_dict.update(branch.iv_term_dict_default)
                self.simulator.set_input(branch.iv_term_dict, asmpt, branch_idx)
                self.simulator.sim_one_step(branch_idx)
            self._create_iv_dict()
            print (f'<cycle:{self.step_cycle()-1}>')
    
    # not use
    def back_step(self):
        self.simulator.backtrack(cur_branch_idx)
        self.simulator.undo_set_input(cur_branch_idx)
        self._create_iv_dict()  # create new inputvars

    def step_cycle(self):
        return self.simulator.tracelen()    # return origin branch tracelen

    def check_prop(self):
        cur_prop = self.simulator.interpret_state_expr_on_curr_frame(self.prop, cur_branch_idx)
        assumptions = self.simulator.all_assumptions(cur_branch_idx)
        print(f"property: {cur_prop.to_string()}")
        for a in assumptions:
            print(f"assumption: {a.to_string()}")

        self.solver.push()
        for a in assumptions:
            self.solver.assert_formula(a)
        f = ~cur_prop   # make_term(not, cur_prop)
        self.solver.assert_formula(f)
        res = self.solver.check_sat()
        self.solver.pop()

        if res:
            print("check prop result: fail!")
        else:
            print("check prop result: pass!")
        return not res  # unsat -> return True

    def check_sat(self, asst, asmpts):
        print('dut.check_sat')
        asmpts_all = self.simulator.all_assumptions(cur_branch_idx)
        asmpts_all.extend(asmpts)
        asmpts_all.extend(self.branch_list[cur_branch_idx].constraints)
        asmpts_all.append(asst)
        return self.solver.check_sat_assuming(asmpts_all)

    def check_assertion(self, assertion):
        print(f"assertion: {assertion.to_string()}")

        self.solver.push()
        formula = ~assertion    # make_term(not, assertion)
        self.solver.assert_formula(formula)
        res = self.solver.check_sat()
        self.solver.pop()

        if res:
            print("check assertion result: fail!")
        else:
            print("check assertion result: pass!")
        return not res

    def print_curr_sv(self):
        self.simulator.print_current_step() # print all running branch info

    def print_curr_assumptions(self):
        self.simulator.print_current_step_assumptions() # print all running branch info

    def __getattr__(self, signal_name):
        v = self.ts.lookup(signal_name) # add a check to make sure the signal_name do exist
        if self._do_not_interpret_var:
            return VarProxy(self, signal_name, v)
        else:
            return SignalProxy(self, signal_name)
    
    def get_signal(self, signal_name):
        # just in case some signals have the same name as the class method
        # you can still use this function to get it
        v = self.ts.lookup(signal_name)
        if self._do_not_interpret_var:
            return VarProxy(self, signal_name, v)
        else:
            return SignalProxy(self, signal_name)
        
    def expr_simplify_ite(self, expr, asspt):
        return expr_simplify_ite(expr, asspt, self.solver)  # public func in pywasimbase
    
class VarProxy:
    def __init__(self, dut, name, var):
        self.dut = dut  # class Dut instance
        self.name = name
        self.var = var  # example: "a"
    @property
    def value(self):
        return self.var
        
    @value.setter
    def value(self, iv):
        raise RuntimeError(f"You cannot set value to '{self.name}'.")
    
    

class SignalProxy:
    def __init__(self, dut, signal_name):
        self.dut = dut           # class Dut instance
        self.name = signal_name  # example: "a"

    @property
    def value(self):
        # if you have assigned, get the one you assigned
        nf = self.dut.simulator.var(self.name)
        if nf in self.dut.iv_term_dict:
            return self.dut.branch_list[cur_branch_idx].iv_term_dict[nf]
        
        # get current term of signal
        try:
            signal_nr = self.dut.simulator.interpret_state_expr_on_curr_frame(nf, cur_branch_idx)   # only have state vars
            return signal_nr
        except Exception:
            if(self.dut.combination):
                signal_nr = nf.substitute(self.dut.branch_list[cur_branch_idx].iv_term_dict)
            else:
                signal_nr = self.dut.simulator.interpret_input_and_state_expr_on_curr_frame(nf, self.dut.branch_list[cur_branch_idx].iv_term_dict, cur_branch_idx)  # have state vars and input vars
                print(f"Warning: expr(dut.{self.name}.value) contains current inputvars; Modifying related inputvars afterward may cause (dut.{self.name}.value) changed.")
            return signal_nr

    @value.setter
    def value(self, iv):
        # set input_signal <-> value
        try:
            iv_nr = self.dut.simulator.var(self.name)
            if self.dut.ts.is_input_var(iv_nr):
                iv_dict = self.dut.simulator.convert({self.name : iv})
                self.dut.branch_list[cur_branch_idx].iv_term_dict.update(iv_dict)
            else:
                raise ValueError(f"No such input variable '{self.name}'.")
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)

    @property
    def value_def(self):
        return None

    @value_def.setter
    def value_def(self, iv):
        try:
            iv_nr = self.dut.simulator.var(self.name)
            if self.dut.ts.is_input_var(iv_nr):
                iv_dict = self.dut.simulator.convert({self.name : iv})
                self.dut.branch_list[cur_branch_idx].iv_term_dict.update(iv_dict)
                self.dut.branch_list[cur_branch_idx].iv_term_dict_default.update(iv_dict)
            else:
                raise ValueError(f"No such input variable '{self.name}'.")
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)
        
    def unset_def(self):
        iv_nr = self.dut.simulator.var(self.name)
        if iv_nr not in self.dut.branch_list[cur_branch_idx].iv_term_dict_default:
            raise ValueError(f"No such default assignment to variable '{self.name}'.")
        del self.dut.branch_list[cur_branch_idx].iv_term_dict_default[iv_nr]
        # reset to X signal
        xvar = self.dut.simulator.get_var(self.name + "X" + str(self.dut.step_cycle()))
        self.dut.branch_list[cur_branch_idx].iv_term_dict.update({iv_nr : xvar})



class async_simulator(object):
    def __init__(self, dut):
        self._state_ptr = None # should point to a pywasim_local_state object
        self.dut = dut
        self.finished = False

    def get_var(self, name):
        return self.dut.simulator.get_var(name)
    
    # not use
    def set_var(self, name, width:int):
        return self.dut.simulator.set_var(width, name)

    def check_assertion(self, expr):    # check_valid
        assert (self._state_ptr)
        print('<solver call>')
        # we can use this func to simplify the expr first
        # simplify_expr = self.dut.expr_simplify_ite(expr, self._state_ptr.branch_cond)
        # can_sat = self.dut.check_sat(~simplify_expr, self._state_ptr.branch_cond)
        can_sat = self.dut.check_sat(~expr, self._state_ptr.branch_cond)
        print('<end solver call>')
        if can_sat:
            # the behavior here should be controllable
            # it should also be debuggable
            # maybe dump waveform
            print(f"<Error branch: {cur_branch_idx}>")
            # for cond in self._state_ptr.branch_cond:
            #     print(f"Error path conditions: {cond}")
            raise AssertionError('sim.check_assertion failed')
            print("sim.check_assertion failed")
            return False
        print("sim.check_assertion pass")
        return True        
        
    def _set_stateptr(self, ptr):
        self._state_ptr = ptr
        
    # branch condition?
    def finish(self):
        self.finished = True

    def wait_cycle(self, num:int = 1):
        assert(self._state_ptr)
        if(num == 0): return
        assert(num > 0)
        self._state_ptr.await_cond = await_condition(cycle = num)
        
    def wait_task(self, task):
        assert(isinstance(task, pywasim_local_state))
        assert(self._state_ptr)
        self._state_ptr.await_cond = await_condition(execthread = task)        
        
    def wait_cond(self, cond):
        assert(self._state_ptr)
        self._state_ptr.await_cond = await_condition(cond = cond)
        
    def wait_posedge(self, signal):
        assert(self._state_ptr)
        assert False # not implemented
        pass
        
    def wait_negedge(self, signal):
        assert(self._state_ptr)
        assert False # not implemented
        pass
        

class await_condition(object):
    def __init__(self, cycle = 0, cond = None, execthread = None):
        self.cycle = cycle
        self.cond = cond
        self.execthread = execthread
    # you will need to test if this is ready

class pywasim_local_state(object):
    def __init__(self, coroutine, args, kwargs):
        self.coroutine = coroutine
        self.pc = -1
        self.finished = False
        self.retval = None
        self.args = args
        self.kwargs = kwargs
        self.local = {}
        self.await_cond = None  # await condition could be clock(n)
        self.branch_cond = []

        # new for branch
        self.branch_idx = 0
        
    def clone(self): # it returns a passthrough object
        ret = pywasim_local_state(self.coroutine, [], {}) # you don't need to clone args and kwargs because it will not branch at invocation
        ret.pc = self.pc
        ret.finished = self.finished
        ret.retval = self.retval
        ret.await_cond = None  # you don't need to deepcopy this
        # this is because when you branch, one thread will have its await set to None to let it continue
        ret.local = self.local.copy()
        ret.branch_cond = self.branch_cond.copy()

        # new for branch
        ret.branch_idx = max_branch_idx

        return ret
        
    def return_value(self):
        return self.retval
    
    def step(self):
        if self.finished:
            print ('<coroutine finished>')
            return
        if self.pc >= len(self.coroutine.funbody):
            print ('<coroutine finished>')
            self.finished = True
            return
                    
        print(f'<coroutine.pc:{self.pc}>')
        self.local['sim']._set_stateptr(self)
        if isinstance(self.coroutine.funbody[self.pc], ast.Expr):
            expr = self.coroutine.funbody[self.pc]
            if isinstance(expr.value, ast.Call):
                # old:
                # if isinstance(expr.value.func, ast.Attribute) and expr.value.func.value.id == 'sim' and expr.value.func.attr == 'wait_cond':
                #     # in sim.wait_cond(...), you should not immediately
                #     # interpret variables
                #     self.local['sim'].dut._do_not_interpret_var = True

                # new: avoid attribute error
                if isinstance(expr.value.func, ast.Attribute):
                    func_value = expr.value.func.value
                    if (isinstance(func_value, ast.Name) and func_value.id == 'sim' and expr.value.func.attr == 'wait_cond'):
                        # in sim.wait_cond(...), you should not immediately
                        # interpret variables
                        self.local['sim'].dut._do_not_interpret_var = True

                # new: need to handle func call like reset(sim, dut), directly inline the def func() to self.coroutine.funbody
                # now we can use sim.wait in def func(), but not support sim.wait in if else / while loop yet
                if isinstance(expr.value.func, ast.Name):
                    global_env = {**globals(), **getattr(sys.modules[__name__], "_extra_globals", {})}
                    func_name = expr.value.func.id
                    func_obj = global_env.get(func_name, None)
                    # print(func_name)
                    if inspect.isfunction(func_obj):
                        # get func source code, and parse it to ast
                        try:
                            src = inspect.getsource(func_obj)
                            parsed = ast.parse(src)
                            func_def = next(n for n in parsed.body if isinstance(n, ast.FunctionDef))
                        except Exception:
                            print("Warning: cannot get source code of function", func_name)
                        else:
                            # create param map
                            param_map = {}
                            for idx, arg_node in enumerate(expr.value.args):
                                arg_val = arg_node
                                # If it is a constant, you can directly use ast.Constant
                                if isinstance(arg_val, ast.Constant):
                                    param_map[func_def.args.args[idx].arg] = copy.deepcopy(arg_val)
                                else:
                                    # If it is a variable or expression, keep it as Name/expression AST
                                    param_map[func_def.args.args[idx].arg] = arg_val

                            body_copy = copy.deepcopy(func_def.body)
                            # substitute parameters with arguments
                            class ParamReplacer(ast.NodeTransformer):
                                def __init__(self, param_map):
                                    self.param_map = param_map
                                def visit_Name(self, node):
                                    if node.id in self.param_map:
                                        return self.param_map[node.id]
                                    return node

                            replacer = ParamReplacer(param_map)
                            body_copy = [replacer.visit(stmt) for stmt in body_copy]

                            # insert into funbody
                            # print("old:", self.coroutine.funbody)
                            # print("body_copy:", body_copy)
                            self.coroutine.funbody[self.pc+1:self.pc+1] = body_copy
                            # print("new:", self.coroutine.funbody)

                            self.local['sim']._set_stateptr(None)
                            self.local['sim'].dut._do_not_interpret_var = False
                            self.pc += 1
                            return
        
        if isinstance(self.coroutine.funbody[self.pc], ast.Return):
            ret = self.coroutine.funbody[self.pc]
            self.retval = eval(ret.value, {},  self.local)
            print ('<coroutine finished>')
            self.finished = True
        else:
            # sim.await will set await_cond
            tmp_stmt = ast.Module(body=[self.coroutine.funbody[self.pc]], type_ignores=[])
            # exec(compile(tmp_stmt, "<ast>", "exec"), {}, self.local)
            exec(compile(tmp_stmt, "<ast>", "exec"), {**globals(), **getattr(sys.modules[__name__], "_extra_globals", {})}, self.local) # allow to get global env in test file
            
        self.local['sim']._set_stateptr(None)
        self.local['sim'].dut._do_not_interpret_var = False
        self.pc += 1
        
        # currently, it is only a quick and dirty implementation
        # in general it is not as easy as it seems to be
        # you should check if it is a while-loop and maintain the frames yourself
        # in this way, you can also handle for example:
        #    if dut.a.value == 0:
        #        ...
        #    else:
        #        ...
        # 
        # and create branches as you see fit
        #
        # currently you can write it as
        #     if sim.check_sat(dut.a.value == 0):
        #        ...
        #
        # but this does not create execution branches
        
    def parse_arg(self):
        assert (self.pc < 0 and not self.finished)
        # parse its args, set the local variables
        func_node = self.coroutine.astnodes.body[0]
        assert isinstance(func_node, ast.FunctionDef)
        args = [arg.arg for arg in func_node.args.args]
        idx = 0
        for arg in args:
            if idx < len(self.args):
                self.local[arg] = self.args[idx]
            elif arg in self.kwargs:
                self.local[arg] = self.kwargs[arg]
                del self.kwargs[arg]
            else:
                raise RuntimeError('no arg for ' + arg)
            idx += 1
        if idx < len(self.args):
            if func_node.args.vararg:
                self.local[func_node.args.vararg] = self.args[idx:]
            else:
                raise RuntimeError('too many arguments ' + self.args[idx:])
        if len(self.kwargs):
            if func_node.args.kwarg:
                self.local[func_node.args.kwarg] = self.kwargs
            else:
                raise RuntimeError('too many arguments ' + self.kwargs)
        self.pc = 0
                
                
# create pointers
class pywasim_coroutine(object):
    def __init__(self, lines):
        self.lines = lines.split(sep = '\n')
        self.astnodes = ast.parse(lines)
        assert (len(self.astnodes.body) == 1)
        self.funbody = self.astnodes.body[0].body
        # print (self.funbody[3])
        
    def invoke(self, *args, **kwargs):
        _all_states.append(pywasim_local_state( coroutine = self, args = args, kwargs = kwargs))
        return _all_states[-1]  # you can use sim.wait_task() on this

def register_task(func):
    code = inspect.getsource(func)
    _all_coroutine.append(pywasim_coroutine(lines = code))
    l = len(_all_coroutine)
    def wrapper(*args, **kwargs):
        _all_coroutine[l-1].invoke(*args, **kwargs)
    return wrapper # this is used to register the args
    
def start_loop(sim, dut, bound = -1):
    if len(_all_states) == 0:
        return

    if sim.dut is not dut:
        raise RuntimeError("Simulator and DUT do not match")

    if not dut.initialized:
        raise RuntimeError("Simulator has not been initialized")
    curr_step = 0
    while bound < 0 or curr_step < bound:
        if sim.finished:
            break
        async_one_step(sim, dut)
        curr_step += 1


def async_one_step(sim, dut):
    if len(_all_states) == 0:
        return

    any_runnable = True
    all_finished = True

    global cur_branch_idx  # use global var

    while any_runnable:
        any_runnable = False
        for idx,st in enumerate(_all_states):  # list of pywasim_local_state
            # print(f'<coroutine #{idx}>')
            cur_branch_idx = st.branch_idx
            if st.finished:
                if dut.branch_list[cur_branch_idx].finished:    # states in same branch all finished
                    continue

                # check if all states (with same cur_branch_idx) finished
                for st_i in _all_states:
                    if st_i.branch_idx == cur_branch_idx and not st_i.finished:
                        break   # found one not finished
                else:
                    dut.finish_branch() # all states with this branch_idx finished
                    print(f'<branch #{cur_branch_idx} finished>')
                # dut.finish_branch()
                continue
            #else
            print(f'<coroutine #{idx}>')
            all_finished = False

            if st.pc < 0:
                # parse its args, set the local variables
                st.parse_arg()
            if st.await_cond is not None and st.await_cond.execthread is not None:
              # if the task it waits has finished
              # we can remove its blocker so that it can continue
              if st.await_cond.execthread.finished:
                st.await_cond = None
                
            # execute the corountine until we need to wait
            while st.await_cond is None and not st.finished:
                st.step()
                any_runnable = True

    if all_finished:
        print('<finished>')
        sim.finish()
        return

    # TODO: branch before step
    print('<dut.step>')
    dut.step()
    # dut.print_curr_sv()
    # next go through _all_states and decrease cycle or check condition
    for idx,st in enumerate(_all_states):
        print(f'<coroutine #{idx} post>')
        cur_branch_idx = st.branch_idx
        # assert (st.await_cond) # is not None
        # as we append passthrough to _all_states, its await_condition may be None 
        if st.await_cond is None:
            continue # just skip them
        # print (st.await_cond)
        if st.await_cond.cycle:
            st.await_cond.cycle -= 1
            if st.await_cond.cycle <= 0:
                st.await_cond = None # remove its blocker so it can continue
                continue
        elif st.await_cond.cond is not None:
            # check if this condition can be true
            # check if this condition can be false
            cond_curr = dut.simulator.interpret_input_and_state_expr_on_curr_frame(st.await_cond.cond, dut.iv_term_dict, cur_branch_idx)
            maybe_true = dut.check_sat(cond_curr, st.branch_cond )
            maybe_false = dut.check_sat(~cond_curr, st.branch_cond )
            print('branch:',maybe_true, maybe_false)
            if maybe_true and not maybe_false:
                st.await_cond = None
                st.branch_cond.append(cond_curr)
            elif maybe_false and not maybe_true:
                st.branch_cond.append(~cond_curr)  # record this as false
            else:
                assert(maybe_true and maybe_false)
                if multi_branch:
                    dut.create_branch() # increment max_branch_idx, must before st.clone()
                    passthrough = st.clone()
                    passthrough.branch_cond.append(cond_curr)
                    st.branch_cond.append(~cond_curr)
                    _all_states.append(passthrough)
                else:
                    st.branch_cond.append(cond_curr)    # single branch, if maybe_true and maybe_false all true, always satisfy condition
                
                
            
    
    

    
    
