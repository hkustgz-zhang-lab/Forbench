#pragma once
#include <set>
#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/cvc5_factory.h"
#include "smt-switch/smt.h"
#include "term_manip.h"
#include "ts.h"

using namespace std;

namespace wasim {

class TraceManager
{
 public:
  TraceManager(const TransitionSystem & ts, smt::SmtSolver & s)
      : ts_(ts), solver_(s), invar_(ts.inputvars()), svar_(ts.statevars())
  {
  }
 
 protected:
  const TransitionSystem & ts_;
  smt::SmtSolver solver_;
  const smt::UnorderedTermSet & invar_;
  const smt::UnorderedTermSet & svar_;
  smt::UnorderedTermSet Xvar_;
  smt::UnorderedTermSet base_var_;

  std::vector<StateAsmpt> abs_state_;

 public:
  const std::vector<StateAsmpt> & get_abs_state() const { return abs_state_; }

  std::vector<StateAsmpt> & update_abs_state() { return abs_state_; }

  void record_x_var(const smt::Term & var) { Xvar_.insert(var); }
  void record_x_var(const smt::TermVec & var) { Xvar_.insert(var.begin(), var.end()); }
  void record_x_var(const smt::UnorderedTermSet & var) { Xvar_.insert(var.begin(), var.end()); }

  void record_base_var(const smt::Term & var) { base_var_.insert(var); }
  void record_base_var(const smt::TermVec & var) { base_var_.insert(var.begin(), var.end()); }
  void record_base_var(const smt::UnorderedTermSet & var) { base_var_.insert(var.begin(), var.end()); }

  void remove_base_var(const smt::Term & var) { base_var_.erase(var); };

  // record a state into abs_state_, return true if it is not abstractly
  // equivalent to any state in abs_state_
  bool record_state(const StateAsmpt & state, 
                    const smt::UnorderedTermSet & Xvar);

  // record a state into abs_state_, return true if it is not abstractly
  // equivalent to any state in new_state_vec
  // new_state_vec is used a branch in the branching search
  bool record_state_nonexisted_in_vec(
                             const std::vector<StateAsmpt> & new_state_vec,
                             const StateAsmpt & state,
                             const smt::UnorderedTermSet & Xvar);

  // void _debug_abs_check(smt::Term expr, smt::TermVec assumptions);
  bool abs_eq(const StateAsmpt & s_abs, const StateAsmpt & s2);
  bool check_reachable(const StateAsmpt & s_in) const;
  // check if all base_var in s_in are free from Xs
  bool check_concrete_enough(const StateAsmpt & s_in, const smt::UnorderedTermSet & Xs);
  StateAsmpt abstract(const StateAsmpt & s);
};

}  // namespace wasim
