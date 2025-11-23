#include "symsim_branch.h"

#include "smt-switch/utils.h"

using namespace std;

namespace wasim {

void SymbolicSimulatorBranch::create_origin_branch() {
  symsim_branches.push_back(Branch());
}

size_t SymbolicSimulatorBranch::fork_branch(const size_t branch_idx) {
  symsim_branches.push_back(Branch(symsim_branches[branch_idx]));
  return symsim_branches.size()-1;
}

void SymbolicSimulatorBranch::finish_branch(const size_t branch_idx) {
  symsim_branches[branch_idx].finished = true;
}

unsigned SymbolicSimulatorBranch::tracelen(size_t idx) const { 
  assert(idx < symsim_branches.size());
  return symsim_branches[idx].trace_.size();
}

void SymbolicSimulatorBranch::add_assumption_interpreted(size_t branch_idx, const smt::Term & asmpt, const std::string & interp) {
  auto & br = symsim_branches[branch_idx];
  br.history_assumptions_.back().push_back(asmpt);
  br.history_assumptions_interp_.back().push_back(interp);
}

smt::TermVec SymbolicSimulatorBranch::all_assumptions(size_t branch_idx) const
{
  smt::TermVec ret_vec;
  for (const auto & l : symsim_branches[branch_idx].history_assumptions_) {
    for (const auto & c : l) {
      ret_vec.push_back(c);
    }
  }
  return ret_vec;
}

std::vector<std::string> SymbolicSimulatorBranch::all_assumption_interp(size_t branch_idx) const
{
  std::vector<std::string> ret_vec;
  for (const auto & l : symsim_branches[branch_idx].history_assumptions_interp_) {
    for (const auto & c : l) {
      ret_vec.push_back(c);
    }
  }
  return ret_vec;
}

smt::Term SymbolicSimulatorBranch::var(const std::string & n) const
{
  return ts_.named_terms().at(n);
}

smt::Term SymbolicSimulatorBranch::cur(const std::string & n, size_t branch_idx) const
{
  auto & br = symsim_branches[branch_idx];
  const auto & sv_mapping = br.trace_.back();
  smt::Term expr = var(n);
  if (!_expr_only_sv(expr)) {
    assert(br.history_choice_.size() != 0);
    assert(!br.history_choice_.back().UsedInSim_);
    const smt::UnorderedTermMap & iv_mapping = br.history_choice_.back().var_assign_;
    auto subs_mapping = sv_mapping;  // make a copy
    // another way of doing this is to substitute twice
    // instead of merging the maps
    subs_mapping.insert(iv_mapping.begin(), iv_mapping.end());
    expr = solver_->substitute(expr, subs_mapping);
  } else {
    expr = solver_->substitute(expr, sv_mapping);
  }
  return expr;
}

void SymbolicSimulatorBranch::_check_only_invar(
    const smt::UnorderedTermMap & vdict) const
{
  for (const auto & v : vdict) {
    if (invar_.find(v.first) == invar_.end())
      throw SimulatorException("Expecting " + v.first->to_string()
                          + " to be input var");
  }
}

bool SymbolicSimulatorBranch::_expr_only_sv(const smt::Term & expr) const
{
  smt::UnorderedTermSet var_set;
  smt::get_free_symbols(expr, var_set);

  for (const auto & v : var_set) {
    if (svar_.find(v) == svar_.end())
      return false;
  }
  // if there is no variable, then it is also only sv
  return true;
}

smt::UnorderedTermMap SymbolicSimulatorBranch::create_input_Xvars(const std::string & inputvar_name) {
  smt::UnorderedTermMap retdict;
  if (inputvar_name.empty()) {
    for (const auto & v : invar_) 
      retdict.emplace(v, new_var(v->get_sort(), v->to_string(), true));
  } else {
    auto v = ts_.lookup(inputvar_name);
    if (!ts_.is_input_var(v) )
      throw SimulatorException(inputvar_name + " is not an input variable");
    retdict.emplace(v, new_var(v->get_sort(), v->to_string(), true));
  }
  return retdict;
}

smt::UnorderedTermMap SymbolicSimulatorBranch::convert(
    const assignment_type & vdict) const
{
  smt::UnorderedTermMap retdict;
  for (const auto & v : vdict) {  // check key only
    const auto & key = v.first;
    const auto & value = v.second;

    smt::Term key_new = var(key);
    // if(svar_.find(key_new) == svar_.end() && invar_.find(key_new) ==
    // invar_.end())
    //    throw SimulatorException("var name " + key + " is not a state/input
    //    variable");

    if (std::holds_alternative<std::string>(value)) {
      auto value_str = std::get<std::string>(value);
      try{
        // allow assigning the same symbol value repeatedly
        auto value_old = solver_->get_symbol(value_str);
        assert(key_new->get_sort() == value_old->get_sort());
        retdict.emplace(key_new, value_old);
      } catch (const std::exception & e) {
        auto key_sort = key_new->get_sort();
        auto value_new = solver_->make_symbol(value_str, key_sort);
        retdict.emplace(key_new, value_new);
      }
    } else if (std::holds_alternative<int>(value)) {
      auto value_int = std::get<int>(value);
      auto key_sort = key_new->get_sort();

      auto value_new = solver_->make_term(value_int, key_sort);
      retdict.emplace(key_new, value_new);
    } else if (std::holds_alternative<smt::Term>(value)) {
      auto value_term = std::get<smt::Term>(value);
      retdict.emplace(key_new, value_term);
    } else
      throw SimulatorException("Unhandled case in assignment_type");
  }
  return retdict;
}

void SymbolicSimulatorBranch::backtrack(size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  assert(br.history_choice_.size() != 0);
  br.trace_.pop_back();
  br.history_assumptions_.pop_back();
  br.history_assumptions_interp_.pop_back();
  br.history_choice_.back().UsedInSim_ = false;
}

void SymbolicSimulatorBranch::free_init(const smt::UnorderedTermMap & var_assignment) {
  assert(symsim_branches.size() == 1);
  auto & br = symsim_branches.at(0);
  br.trace_.push_back(var_assignment);
  auto & var_assignment_ref = br.trace_.back();
  for (const auto & v : svar_) {
    if (var_assignment_ref.find(v) == var_assignment_ref.end()) {
      var_assignment_ref[v] =
          new_var(v->get_sort(), v->to_string(), false);
    }
  }
  if (!_expr_only_sv(ts_.init()))
    throw SimulatorException("init condition contains non state var");
  
  br.history_assumptions_.push_back({ });
  br.history_assumptions_interp_.push_back({ });
  // this will not assumptions
} // end free_init

void SymbolicSimulatorBranch::init(
    const smt::UnorderedTermMap & var_assignment /*={}*/)
{
  assert(symsim_branches.size() == 1);
  auto & br = symsim_branches.at(0);
  // check if all vars are state vars
  for (const auto & v_val_map : var_assignment) {
    if ( ts_.statevars().find(v_val_map.first) == ts_.statevars().end() )
      throw SimulatorException( "[init] " + v_val_map.first->to_string() + " is not a state var.");
  }

  br.trace_.push_back(var_assignment);

  auto & var_assignment_ref = br.trace_.back();
  for (const auto & v : svar_) {
    if (var_assignment_ref.find(v) == var_assignment_ref.end()) {
      var_assignment_ref[v] =
          new_var(v->get_sort(), v->to_string(), false);
    }
  }

  // make sure the init constraint only contains state variables
  // you cannot constrain input variables in the initial state
  // btorparser will help convert the TS to avoid this
  if (!_expr_only_sv(ts_.init()))
    throw SimulatorException("init condition contains non state var");

  auto init_constr = solver_->substitute(ts_.init(), var_assignment_ref);
  br.history_assumptions_.push_back({ init_constr });
  br.history_assumptions_interp_.push_back({ "init" });
}

void SymbolicSimulatorBranch::set_current_state(const StateAsmpt & s, size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  br.trace_.clear();
  br.trace_.push_back(s.get_sv());

  // smt::TermVec asmpt_copy(s.asmpt_.begin(), s.asmpt_.end());
  // std::vector<std::string> asmpt_interp_copy(s.assumption_interp_.begin(),
  // s.assumption_interp_.end());

  br.history_assumptions_.clear();
  br.history_assumptions_.push_back(s.get_assumptions());
  br.history_assumptions_interp_.clear();
  br.history_assumptions_interp_.push_back(s.get_assumption_interpretations());
  br.history_choice_.clear();
}

/// print the current state variable assignment
void SymbolicSimulatorBranch::print_current_step(size_t branch_idx) const  // one branch
{
  assert(branch_idx < symsim_branches.size());
  const auto & branch = symsim_branches.at(branch_idx);
  if (branch.finished)
    cout << "<branch #" << branch_idx << " finished in cycle " << branch.trace_.size() << ">" << endl;

  cout << "<branch #" << branch_idx << ">" << endl;
  const auto & prev_sv = branch.trace_.back();
  cout << "--------------------------------" << endl;
  cout << "| " << setiosflags(ios::left) << setw(20) << "sv"
       << "| " << setw(20) << "value" << endl;
  cout << "--------------------------------" << endl;
  for (const auto & sv : prev_sv) {
    cout << "| " << setiosflags(ios::left) << setw(20) << sv.first->to_string()
         << "| " << setw(20) << sv.second->to_string() << endl;
  }
}

void SymbolicSimulatorBranch::print_current_step_all_branches() const
{ 
  for (size_t idx = 0; idx < symsim_branches.size(); ++idx)
    print_current_step(idx);
}

/// get the assumptions (collected from all previous steps)
void SymbolicSimulatorBranch::print_current_step_assumptions(size_t branch_idx) const  // one branch
{
  assert(branch_idx < symsim_branches.size());
  const auto & branch = symsim_branches.at(branch_idx);
  if (branch.finished)
    cout << "<branch #" << branch_idx << " finished in cycle " << branch.trace_.size() << ">" << endl;
  
  cout << "<branch #" << branch_idx << ">" << endl;
  int i = 0;
  for (const auto & l : branch.history_assumptions_) {
    int j = 0;
    for (const auto & a : l) {
      const auto & interp = branch.history_assumptions_interp_.at(i).at(j);
      cout << "A" << i << ", " << j << " " << interp << endl;
      cout << "A" << i << ", " << j << " " << a << endl;
      j++;
    }
    i++;
  }
}


void SymbolicSimulatorBranch::print_current_step_assumptions_all_branches() const
{ 
  for (size_t idx = 0; idx < symsim_branches.size(); ++idx)
    print_current_step_assumptions(idx);
}

void SymbolicSimulatorBranch::set_input(const smt::UnorderedTermMap & invar_assign,
                                 const smt::TermVec & pre_assumptions, size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  if (br.trace_.empty())
    throw SimulatorException("Simulator.init should be called before set_input");

  if (br.history_choice_.size() != 0) {
    br.history_choice_.back().CheckSimed();
  }

  _check_only_invar(invar_assign);
  const auto & prev_sv(br.trace_.back());

  br.history_choice_.push_back(ChoiceItem(
      pre_assumptions, invar_assign));  // construct by r-value reference
  auto & c = br.history_choice_.back();

  for (const auto & v : invar_) {
    if (prev_sv.find(v) != prev_sv.end()) {
      cout << "WARNING: shadowing input assignment as assigned by prev-state "
           << v->to_string() << endl;
    }
    if (c.var_assign_.find(v) == c.var_assign_.end()) {
      c.var_assign_[v] =
          new_var(v->get_sort(), v->to_string(), true);
    }
  }
  const auto & invar_assign_all = c.var_assign_;

  unsigned len = br.history_assumptions_.back().size();
  c.record_prev_assumption_len(len);

  assert(br.history_assumptions_.size() == br.history_assumptions_interp_.size());
  assert(br.history_assumptions_.back().size()
         == br.history_assumptions_interp_.back().size());

  auto submap(prev_sv);  // copy here is needed anyway
  submap.insert(invar_assign_all.begin(), invar_assign_all.end());

  // TODO: constraints here are different from those in COSA btor parser!
  // the second Boolean has no use in our case
  smt::TermVec assmpt_vec;
  for (const auto & vect : ts_.constraints()) {
    auto assumption = solver_->substitute(vect.first, submap);
    assmpt_vec.push_back(assumption);
  }
  if (!assmpt_vec.empty()) {
    smt::Term assmpt;
    if (assmpt_vec.size() == 1) {
      assmpt = assmpt_vec.back();
    } else {
      assmpt = solver_->make_term(smt::And, assmpt_vec);
    }

    br.history_assumptions_.back().push_back(assmpt);
    br.history_assumptions_interp_.back().push_back(
      "ts.asmpt @" + (std::to_string(br.trace_.size() - 1)));
  }

  for (const auto & vect : pre_assumptions) {
    auto assmpt_temp = solver_->substitute(vect, submap);
    br.history_assumptions_.back().push_back(assmpt_temp);
    br.history_assumptions_interp_.back().push_back(
        vect->to_string() + "@" + std::to_string(br.trace_.size() - 1));
  }
}  // end of set_input

void SymbolicSimulatorBranch::undo_set_input(size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  assert(br.history_choice_.size() != 0);
  const auto & c = br.history_choice_.back();  // avoid copy
  assert(!c.UsedInSim_);
  auto l = c.get_prev_assumption_len();
  br.history_assumptions_.back().resize(l);
  br.history_assumptions_interp_.back().resize(l);
  br.history_choice_.pop_back();  // you can only pop in the end, o.w. reference will fail
}

/// similar to cur(), but will check no reference to the input variables
smt::Term SymbolicSimulatorBranch::interpret_state_expr_on_curr_frame(
    const smt::Term & expr, size_t branch_idx) const
{
  auto & br = symsim_branches[branch_idx];
  if (!_expr_only_sv(expr))
    throw SimulatorException("expr should only contain only state variables");
  const auto & prev_sv = br.trace_.back();
  return solver_->substitute(expr, prev_sv);
}

smt::Term SymbolicSimulatorBranch::interpret_input_and_state_expr_on_curr_frame(
    const smt::Term & expr, const smt::UnorderedTermMap & iv_map, size_t branch_idx) const
{
  auto & br = symsim_branches[branch_idx];
  if (!_expr_only_sv(expr)){
    const auto & sv_mapping = br.trace_.back();
    assert(br.history_choice_.size() != 0);
    auto subs_mapping = sv_mapping;  // make a copy
    subs_mapping.insert(iv_map.begin(), iv_map.end());
    return solver_->substitute(expr, subs_mapping);
  }
  const auto & prev_sv = br.trace_.back();
  return solver_->substitute(expr, prev_sv);
}

/// similar to cur(), but will check no reference to the input variables
smt::TermVec SymbolicSimulatorBranch::interpret_state_expr_on_curr_frame(
    const smt::TermVec & expr_list, size_t branch_idx) const
{
  smt::TermVec ret;
  for (const auto & e : expr_list) {
    smt::Term e_term = interpret_state_expr_on_curr_frame(e, branch_idx);
    ret.push_back(std::move(e_term));
  }
  return ret;
}

void SymbolicSimulatorBranch::sim_one_step(size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  assert(br.history_choice_.size() != 0);

  auto & c = br.history_choice_.back();
  c.setSim();

  const auto & invar_assign = c.var_assign_;
  const auto & prev_sv = br.trace_.back();
  smt::UnorderedTermMap svmap;
  auto submap = prev_sv;
  submap.insert(invar_assign.begin(), invar_assign.end());
  for (const auto & sv : ts_.state_updates()) {
    svmap[sv.first] = solver_->substitute(sv.second, submap);
  }
  br.trace_.push_back(
      std::move(svmap));  // svmap will not be used afterwards, avoid copy
  br.history_assumptions_.push_back({});
  br.history_assumptions_interp_.push_back({});
}

// void SymbolicSimulatorBranch::sim_one_step_direct()
// {
//   const auto & prev_sv = trace_.back();
//   smt::UnorderedTermMap svmap;
//   for (const auto & sv : ts_.state_updates()) {
//     svmap[sv.first] = solver_->substitute(sv.second, prev_sv);
//   }
//   trace_.push_back(std::move(svmap));
//   history_assumptions_.push_back({});
//   history_assumptions_interp_.push_back({});
// }

smt::Term SymbolicSimulatorBranch::new_var(smt::Sort sort,
                                    const std::string & vname /*"=var"*/,
                                    bool x /*=true*/)
{
  std::string n = x ? vname + "X" : vname;
  smt::Term symb = free_make_symbol(n, sort, name_cnt_, solver_);

  if (x) Xvar_.insert(symb);
  return symb;
}

StateAsmpt SymbolicSimulatorBranch::get_curr_state(const smt::TermVec & assumptions, size_t branch_idx)
{
  auto & br = symsim_branches[branch_idx];
  auto need_to_push_input = false;
  if ((br.history_choice_.size() == 0) || (br.history_choice_.back().UsedInSim_))
    need_to_push_input = true;

  if (need_to_push_input)
    set_input({}, assumptions, branch_idx);
  else if (assumptions.size() != 0)
    cout << "WARNING: assumptions are not used in get_curr_state" << endl;

  StateAsmpt ret(br.trace_.back(), all_assumptions(branch_idx), all_assumption_interp(branch_idx));
  if (need_to_push_input) {
    undo_set_input(branch_idx);
  }
  return ret;
}

smt::Term SymbolicSimulatorBranch::set_var(int bitwdth, std::string vname /*= "var"*/)
{
  if (bitwdth < 0)
    throw SimulatorException("bitwidth cannot be negative");
  auto symb_sort = bitwdth > 0 ? solver_->make_sort(smt::BV, bitwdth) : solver_->make_sort(smt::BOOL);
  auto symb = solver_->make_symbol(vname, symb_sort);
  return symb;  
}

}  // namespace wasim
