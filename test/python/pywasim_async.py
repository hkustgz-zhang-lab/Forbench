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

_all_coroutine = [] # List of `pywasim_coroutine`
_all_states = []
# HZ: I don't like the use of these global vars...
# cur_branch_idx = 0  # current running branch, init 0 / every state has a branch_idx

class Dut_Branch:
    def __init__(self):
        """will create a copy of all the arguments"""
        self.iv_term_dict = {}  # a term->term map (after convert)
        self.iv_term_dict_default = {} # a term->term map (after convert)
        self.constraints = []
        #finished should not be associated with a branch

    def clone(self):
        ret = Dut_Branch()
        ret.iv_term_dict = self.iv_term_dict.copy()
        ret.iv_term_dict_default = self.iv_term_dict_default.copy()
        ret.constraints = self.constraints.copy()
        return ret

class Dut:
    # DUT now is having multiple branches
    def __init__(self, btorname):
        self.ts = TransSys(btorname)
        self.simulator = Symsimbranch(self.ts) # this is the C++ class
        self.solver = self.simulator.get_solver()
        
        self._do_not_interpret_var = False # if true, will not return SignalProxy
        self.inputvars_list = self.ts.inputvars()
        self.statevars_list = self.ts.statevars()

        #self.iv_term_dict = {}
        #self.iv_term_dict_default = {}
        #self.constraints = []

        self.initialized = False
        # self.prop = self._get_property()

        self.combination = (len(self.statevars_list) == 0)    # comb -> True, seq -> False

        self.branch_list = [Dut_Branch()] # list of Dut_Branch. Initially there is one branch
        self.curr_branch_idx = None # this must be set by sim, before it can call many functions

    # func for branch
    def _set_curr_branch(self, idx):
        self.curr_branch_idx = idx

    def _get_curr_branch_iv_term_dict(self):
        assert (self.curr_branch_idx is not None)
        return self.branch_list[self.curr_branch_idx].iv_term_dict

    def _get_curr_branch_iv_term_dict_default(self):
        assert (self.curr_branch_idx is not None)
        return self.branch_list[self.curr_branch_idx].iv_term_dict_default

    # None user-facing functions must be provided by branch_idx
    def fork_branch(self, branch_idx):
        self.branch_list.append(self.branch_list[branch_idx].clone())
        new_branch_id = self.simulator.fork_branch(branch_idx)
        assert (new_branch_id == len(self.branch_list)-1)
        return new_branch_id

    def num_of_branches(self):
        return len(self.branch_list)
    
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

    def _create_iv_dict(self, branch_idx):
        branch = self.branch_list[branch_idx]
        # should not use the same X inputvars for all branches
        branch.iv_term_dict = self.simulator.create_input_Xvars("") # returns a map: string -> term map
        # you can also call `_merge_dict_replace_X`` here, but the result would be the same anyway
        # because iv_term_dict are all Xs
        branch.iv_term_dict.update(branch.iv_term_dict_default)

    def set_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        assert len(self.branch_list) == 1
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.init(var_dict)
        self._create_iv_dict(branch_idx = 0)          # create new inputvars

    def free_init(self, d = {}):
        if self.initialized:
            raise RuntimeError("You cannot initialize simulator twice")
        assert len(self.branch_list) == 1
        self.initialized = True
        var_dict = self.simulator.convert(d)
        self.simulator.free_init(var_dict)
        self._create_iv_dict(branch_idx = 0)          # create new inputvars

    def set_constraint(self, constr):
        assert (self.curr_branch_idx is not None)
        self.branch_list[self.curr_branch_idx].constraints.append(constr)
    
    def clear_constraint(self):
        assert (self.curr_branch_idx is not None)
        self.branch_list[self.curr_branch_idx].constraints = []

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


    def _step(self, branch_idx, num = 1, asmpt = []):
        for _ in range(num):
            branch = self.branch_list[branch_idx]
            # iv_term_dict_default will replace iv_term_dict, only if iv_term_dict has an X there
            iv_term_dict = self._merge_dict_replace_X(branch.iv_term_dict, branch.iv_term_dict_default)
            #branch.iv_term_dict.update(branch.iv_term_dict_default) # FIXME: this does not look correct
            self.simulator.set_input(iv_term_dict, asmpt, branch_idx)
            self.simulator.sim_one_step(branch_idx)
            print (f'<dut.step br#{branch_idx} cycle:{self.step_cycle(branch_idx)-1}>')
        self._create_iv_dict(branch_idx)
    
    # not use
    def _back_step(self, branch_idx, num = 1):
        for _ in range(num):
            branch = self.branch_list[branch_idx]
            self.simulator.backtrack(branch_idx)
            self.simulator.undo_set_input(branch_idx)
        self._create_iv_dict(branch_idx)  # create new inputvars only after all these steps

    def step_cycle(self, branch_idx):
        """Currently """
        return self.simulator.tracelen(branch_idx)    # return origin branch tracelen

    def check_prop(self):
        assert (False)
        # TODO: this is also problematic, not using local assumptions etc.
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
        assert (self.curr_branch_idx is not None)
        asmpts_all = self.simulator.all_assumptions(self.curr_branch_idx)
        asmpts_all.extend(asmpts)
        asmpts_all.extend(self.branch_list[self.curr_branch_idx].constraints)
        asmpts_all.append(asst)
        return self.solver.check_sat_assuming(asmpts_all)
    
    def check_assertion(self, assertion):
        # TODO
        assert (False)
        if res:
            print("check assertion result: fail!")
        else:
            print("check assertion result: pass!")
        return not res

    def _check_assertion(self, assertion):
        # should not be called directly by the user
        # because it is not using an branch condition/branch local constraints
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

    # TODO: user should be allowed to either print one branch or print all...
    
    def print_curr_sv(self):
        assert (self.curr_branch_idx is not None)
        self.simulator.print_current_step(self.curr_branch_idx) # print all running branch info
    def print_curr_assumptions(self):
        assert (self.curr_branch_idx is not None)
        self.simulator.print_current_step_assumptions(self.curr_branch_idx) # print all running branch info

    def print_curr_sv_all_branches(self):
        self.simulator.print_current_step_all_branches() # print all running branch info
    def print_curr_assumptions_all_branches(self):
        self.simulator.print_current_step_assumptions_all_branches() # print all running branch info

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
        assert (self.dut.curr_branch_idx is not None)
        if self.dut.step_cycle(self.dut.curr_branch_idx) == 0:
            raise Exception('Combinational circuits also need initialization (free_init)')
        curr_branch_idx = self.dut.curr_branch_idx
        iv_term_dict = self.dut._get_curr_branch_iv_term_dict()

        # if you have assigned, get the one you assigned
        nf = self.dut.simulator.var(self.name) # name to SMT term
        if nf in iv_term_dict:
            print(f"Warning: expr(dut.{self.name}.value) contains current inputvars; Modifying related inputvars afterward may cause (dut.{self.name}.value) changed.")
            return iv_term_dict[nf]
        
        # get current term of signal
        try:
            signal_nr = self.dut.simulator.interpret_state_expr_on_curr_frame(nf, curr_branch_idx)   # only have state vars
            return signal_nr
        except Exception:
            signal_nr = self.dut.simulator.interpret_input_and_state_expr_on_curr_frame(nf, iv_term_dict, curr_branch_idx)  # have state vars and input vars
            print(f"Warning: expr(dut.{self.name}.value) contains current inputvars; Modifying related inputvars afterward may cause (dut.{self.name}.value) changed.")
            return signal_nr

    @value.setter
    def value(self, iv):
        # set input_signal <-> value
        iv_term_dict = self.dut._get_curr_branch_iv_term_dict()
        try:
            iv_nr = self.dut.simulator.var(self.name) # name to SMT term
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
        iv_term_dict = self.dut._get_curr_branch_iv_term_dict()
        iv_term_dict_default = self.dut._get_curr_branch_iv_term_dict_default()
        try:
            iv_nr = self.dut.simulator.var(self.name)
            if self.dut.ts.is_input_var(iv_nr):
                iv_dict = self.dut.simulator.convert({self.name : iv})
                iv_term_dict_default.update(iv_dict)
                # will only replace the value in iv_term_dict if iv_term_dict has an X there
                iv_term_dict = self.dut._merge_dict_replace_X(iv_term_dict, iv_dict)
                self.dut.branch_list[self.dut.curr_branch_idx].iv_term_dict = iv_term_dict
            else:
                raise ValueError(f"No such input variable '{self.name}'.")
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)
        
    def unset_def(self):
        iv_term_dict = self.dut._get_curr_branch_iv_term_dict()
        iv_term_dict_default = self.dut._get_curr_branch_iv_term_dict_default()
        try:
            iv_nr = self.dut.simulator.var(self.name)
        except Exception as e:
            raise ValueError(f"No such variable '{self.name}'.", e)

        if iv_nr not in iv_term_dict_default:
            raise ValueError(f"No such default assignment to variable '{self.name}'.")
        def_val = iv_term_dict_default[iv_nr]
        del iv_term_dict_default[iv_nr]
        # reset to X signal only if iv_term_dict has the same X there
        if hash(iv_term_dict[iv_nr]) == hash(def_val):
            xvar_dict = self.dut.simulator.create_input_Xvars(self.name)
            xvar = list(xvar_dict.items())[0][1]
            iv_term_dict[iv_nr] = xvar


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

    def check_assertion(self, expr, asmpts = []):    # check_valid
        assert (self._state_ptr)
        curr_branch_idx = self._state_ptr.branch_idx
        print('<solver call>')
        # we can use this func to simplify the expr first
        # simplify_expr = self.dut.expr_simplify_ite(expr, self._state_ptr.branch_cond)
        # can_sat = self.dut.check_sat(~simplify_expr, self._state_ptr.branch_cond)
        can_sat = self.dut.check_sat(~expr, asmpts)
        print('<end solver call>')
        if can_sat:
            # the behavior here should be controllable
            # it should also be debuggable
            # maybe dump waveform
            print(f"<Error branch: {curr_branch_idx}>")
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
        self.retval = None
        self.args = args
        self.kwargs = kwargs
        self.local = {}
        self.await_cond = None  # await condition could be clock(n)
        # new for branch
        self.branch_idx = 0 # initially these coroutines are just for the first branch
        self.finished = False
        
    def clone(self, branch_idx): # it returns a passthrough object
        # this does not coy the associated branch, you must copy separately and associate them with the arg
        ret = pywasim_local_state(self.coroutine, [], {}) # you don't need to clone args and kwargs because it will not branch at invocation
        ret.pc = self.pc
        ret.retval = self.retval
        ret.await_cond = None  # you don't need to deepcopy this
        # this is because when you branch, one thread will have its await set to None to let it continue
        ret.local = self.local.copy()
        # new for branch
        ret.branch_idx = branch_idx
        assert (not self.finished)
        ret.finished = False
        return ret
    
    def is_finished(self):
        return self.finished
    def set_finished(self):
        self.finished = True
        
    def return_value(self):
        return self.retval
    
    def step(self):
        """This function will handle the execution of Python code"""
        if self.is_finished():
            print ('<coroutine finished>')
            return
        if self.pc >= len(self.coroutine.funbody):
            print ('<coroutine finished>')
            self.set_finished()
            return
                    
        print(f'<coroutine.pc:{self.pc}>')
        self.local['sim']._set_stateptr(self)
        self.local['sim'].dut._set_curr_branch(self.branch_idx)

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
                            self.local['sim'].dut._set_curr_branch(None)
                            self.local['sim'].dut._do_not_interpret_var = False
                            self.pc += 1
                            return
        
        if isinstance(self.coroutine.funbody[self.pc], ast.Return):
            ret = self.coroutine.funbody[self.pc]
            self.retval = eval(ret.value, {},  self.local)
            print ('<coroutine finished>')
            self.set_finished()
        else:
            # sim.await will set await_cond
            tmp_stmt = ast.Module(body=[self.coroutine.funbody[self.pc]], type_ignores=[])
            # exec(compile(tmp_stmt, "<ast>", "exec"), {}, self.local)
            exec(compile(tmp_stmt, "<ast>", "exec"), {**globals(), **getattr(sys.modules[__name__], "_extra_globals", {})}, self.local) # allow to get global env in test file
            
        self.local['sim']._set_stateptr(None)
        self.local['sim'].dut._set_curr_branch(None)
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
        assert (self.pc < 0) #you cannot call is_finished here
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
    """ The coroutine class, when invoked, will update `_all_states` to register an invocation """
    def __init__(self, lines):
        self.lines = lines.split(sep = '\n')
        self.astnodes = ast.parse(lines)
        assert (len(self.astnodes.body) == 1)
        self.funbody = self.astnodes.body[0].body
        # print (self.funbody[3])
        
    def invoke(self, *args, **kwargs):
        _all_states.append(pywasim_local_state( coroutine = self, args = args, kwargs = kwargs))
        _all_states[-1].parse_arg() # parse the argument at the time we invoke it
        return _all_states[-1]  # you can use sim.wait_task() on this

def register_task(func):
    """register a function as a coroutine. Will update `_all_coroutine`"""
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

    while any_runnable:
        any_runnable = False
        all_finished = True
        for idx,st in enumerate(_all_states):  # list of pywasim_local_state
            # print(f'<coroutine #{idx}>')
            curr_branch_idx = st.branch_idx
            if st.is_finished():
                continue

            #else
            print(f'<coroutine #{idx}>')
            all_finished = False

            assert (st.pc >= 0)
            if st.await_cond is not None and st.await_cond.execthread is not None:
              # if the task it waits has finished
              # we can remove its blocker so that it can continue
              if st.await_cond.execthread.is_finished():
                st.await_cond = None
                
            # execute the corountine until we need to wait
            while st.await_cond is None and not st.is_finished():
                st.step() # this runs the Python code in the coroutine
                any_runnable = True

    if all_finished:
        print('<all finished>')
        sim.finish()
        return
    
    # TODO: branch before step. This maybe needed if we want 
    # if symexpr == condition
    # then we will need branch before step

    print('<dut.step>')
    stepped_branches = set()
    for idx,st in enumerate(_all_states):
        if st.is_finished():
            continue
        curr_branch_idx = st.branch_idx
        if curr_branch_idx in stepped_branches:
            continue # in case two states ---> same branch
        dut._step(curr_branch_idx)  # this eventually calls C++ to step all branches of DUT
        stepped_branches.add(curr_branch_idx)
    # this is essentially BFS
    # in the future, you may configure to try DFS etc.

    # dut.print_curr_sv()
    # next go through _all_states and decrease cycle or check condition
    for idx,st in enumerate(_all_states):
        if st.is_finished():
            continue

        print(f'<coroutine #{idx} post>')
        curr_branch_idx = st.branch_idx
        # assert (st.await_cond) # is not None
        # as we append passthrough to _all_states, its await_condition may be None 
        if st.await_cond is None:
            continue # just skip them (the newly added `passthrough` will be skipped)
        # print (st.await_cond)
        if st.await_cond.cycle:
            st.await_cond.cycle -= 1
            if st.await_cond.cycle <= 0:
                assert (st.await_cond.cycle == 0)
                st.await_cond = None # remove its blocker so it can continue
                continue
        elif st.await_cond.cond is not None:
            # check if this condition can be true
            # check if this condition can be false
            dut._set_curr_branch(curr_branch_idx)
            iv_term_dict = dut._get_curr_branch_iv_term_dict()
            cond_curr = dut.simulator.interpret_input_and_state_expr_on_curr_frame(st.await_cond.cond, iv_term_dict, curr_branch_idx)
            maybe_true = dut.check_sat(cond_curr, [] )
            maybe_false = dut.check_sat(~cond_curr, [] )
            dut._set_curr_branch(None)

            print('branch:',maybe_true, maybe_false)
            if maybe_true and not maybe_false:
                st.await_cond = None
                dut.simulator.add_assumption_interpreted(curr_branch_idx, cond_curr, "branch cond")
                # st.branch_cond.append(cond_curr) #HZ: we can get rid of this, branch_cond will be inserted into dut.simulator
            elif maybe_false and not maybe_true:
                dut.simulator.add_assumption_interpreted(curr_branch_idx, ~cond_curr, "~branch cond")
                #st.branch_cond.append(~cond_curr)  # record this as false
            else:
                assert(maybe_true and maybe_false)
                br_idx = dut.fork_branch(curr_branch_idx) # increment max_branch_idx, must before st.clone()
                # st.clone clears passthrough.await_cond
                passthrough = st.clone(branch_idx = br_idx) # link the state with the branch
                assert (passthrough.await_cond is None)
                dut.simulator.add_assumption_interpreted(br_idx, cond_curr, "branch cond")
                dut.simulator.add_assumption_interpreted(curr_branch_idx, ~cond_curr, "~branch cond")
                #passthrough.branch_cond.append(cond_curr)
                #st.branch_cond.append(~cond_curr)
                _all_states.append(passthrough)

                
                
            
    
    

    
    
