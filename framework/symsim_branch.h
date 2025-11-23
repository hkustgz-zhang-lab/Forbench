#pragma once
#include <iomanip>
#include <string>
#include <unordered_map>
#include "assert.h"
#include "time.h"

#include "smt-switch/generic_sort.h"
#include "smt-switch/smt.h"

#include "utils/exceptions.h"

#include "term_manip.h"
#include "ts.h"
#include "choice_item.h"

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace wasim {

/// std::string : a symbolic value, int : a concrete value
typedef std::variant<int, std::string, smt::Term> value_type;
/// from string (variable name) to value
typedef std::map<std::string, value_type> assignment_type;

class SymbolicSimulatorBranch
{
  // symsim branch
  class Branch{
    public:
    Branch(): finished(false) {}
    Branch(const Branch & other) :
      trace_(other.trace_),
      history_choice_(other.history_choice_),
      history_assumptions_(other.history_assumptions_),
      history_assumptions_interp_(other.history_assumptions_interp_),
      finished(false) { }

    std::vector<smt::UnorderedTermMap> trace_; // the state variable map of each step
    std::vector<ChoiceItem> history_choice_;   // the choice of each step (input var map & assumptions)
    std::vector<smt::TermVec> history_assumptions_; // the assumptions of each step
    std::vector<std::vector<std::string>> history_assumptions_interp_; // the text descriptions of these assumptions

    bool finished; // whether this branch has finished or not // HZ: maybe currently this is not needed
    // maybe we can retarget this for other purpose in the future... (branch merge etc.)
  };  // end of symsim branch

 public:
  // we will keep a reference to ts
  // and a copy of the pointer to the solver
  SymbolicSimulatorBranch(TransitionSystem & ts, const smt::SmtSolver & s)
      : ts_(ts), solver_(s), invar_(ts.inputvars()), svar_(ts.statevars())
  { 
    create_origin_branch();
  }
  
 protected:
  TransitionSystem & ts_;
  smt::SmtSolver solver_;  // smt::SmtSolver is a smart pointer
  const smt::UnorderedTermSet & invar_;
  const smt::UnorderedTermSet & svar_;

  std::unordered_map<std::string, int> name_cnt_;
  smt::UnorderedTermSet Xvar_;

  // branch
  std::vector<Branch> symsim_branches;

  void _check_only_invar(const smt::UnorderedTermMap & vdict) const;
  bool _expr_only_sv(const smt::Term & expr) const;

  /**
   * @brief
   *
   * @param bitwdth
   * @param vname
   * @param x
   * @return smt::Term
   */
  smt::Term new_var(smt::Sort sort,
                    const std::string & vname = "var",
                    bool x = true);

  void create_origin_branch(); // should not be accessible from the outside

 public:
  // branch func
  size_t fork_branch(const size_t branch_idx); // argument: the branch idx to be copied, return: the new branch id
  void finish_branch(const size_t branch_idx);

  /// get the length of the trace
  unsigned tracelen(size_t idx) const;  // branch
  
  /// @brief add assumption to a branch
  void add_assumption_interpreted(size_t branch_idx, const smt::Term & asmpt, const std::string & interp);
  /// collect all assumptions on each frame
  smt::TermVec all_assumptions(size_t branch_idx) const;  // branch
  /// collect all interpretations of assumptions on each frame
  std::vector<std::string> all_assumption_interp(size_t branch_idx) const;  // branch
  /// get the term for a variable
  smt::Term var(const std::string & n) const; // ok
  /// get the term for name n, then use the current symbolic
  /// variable assignment to substitute all variables in it
  /// if it contains input variable, use the most recent input
  /// variable assignment as well
  smt::Term cur(const std::string & n, size_t branch_idx) const;  // branch, but no used in pywasim
  /// print the current state variable assignment
  void print_current_step(size_t branch_idx) const;  // one branch
  /// get the assumptions (collected from all previous steps)
  void print_current_step_assumptions(size_t branch_idx) const;  // one branch
  /// print the current state variable assignment
  void print_current_step_all_branches() const;  // for all branches
  /// get the assumptions (collected from all previous steps)
  void print_current_step_assumptions_all_branches() const;  // for all branches

  /// a shortcut to create symbolic variables/concrete values in a map
  smt::UnorderedTermMap convert(const assignment_type & vdict) const; // ok
  /// a shortcut to create X values for inputs
  smt::UnorderedTermMap create_input_Xvars(const std::string & inputvar_name = "");

  /// goto the previous simulation step
  void backtrack(size_t branch_idx); // branch, Not supported yet
  /// use the given variable assignment to initialize
  void init(const smt::UnorderedTermMap & var_assignment = {}); // branch
  /// use the given variable assignment, but does not use init condition
  void free_init(const smt::UnorderedTermMap & var_assignment = {});  // branch
  /// re-assign the current state
  void set_current_state(const StateAsmpt & s, size_t branch_idx); // branch, Not supported yet
  /// set the input variable values before simulating next step
  ///  (and also set some assumptions before the next step)
  void set_input(const smt::UnorderedTermMap & invar_assign,
                 const smt::TermVec & pre_assumptions, size_t branch_idx);  // branch
  /// undo the input setting
  /// usage: set_input -> sim_one_step --> (a new state) -> backtrack ->
  /// undo_set_input
  void undo_set_input(size_t branch_idx);  // branch, Not supported yet

  /// similar to cur(), but will check no reference to the input variables
  smt::Term interpret_state_expr_on_curr_frame(const smt::Term & expr, size_t branch_idx) const;  // branch
  smt::Term interpret_input_and_state_expr_on_curr_frame(const smt::Term & expr, const smt::UnorderedTermMap & iv_map, size_t branch_idx) const;  // branch
  /// similar to cur(), but will check no reference to the input variables
  smt::TermVec interpret_state_expr_on_curr_frame(
      const smt::TermVec & expr, size_t branch_idx) const;  // branch

  /// do simulation
  void sim_one_step(size_t branch_idx); // branch

  /// get the set of all X variables
  const smt::UnorderedTermSet & get_Xs() const { return Xvar_; }  // ok, no used in pywasim
  bool is_Xvar(const smt::Term & t) { return (Xvar_.find(t) != Xvar_.end()); }

  /// get (a copy of) the current state
  StateAsmpt get_curr_state(const smt::TermVec & assumptions = {}, size_t branch_idx = 0);  // branch, Not supported yet
  /// a shortcut to create a variable
  smt::Term set_var(int bitwdth, std::string vname = "var");  // ok, no used in pywasim

  /// get solver
  smt::SmtSolver get_solver() const { return solver_; } // ok

};
}  // namespace wasim
