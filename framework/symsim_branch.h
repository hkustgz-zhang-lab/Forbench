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
 private:
  /// a class only used in symbolic simulator
  class ChoiceItem
  {
   public:
    ChoiceItem(const smt::TermVec & assumptions,
               const smt::UnorderedTermMap & var_assign)
        : assumptions_(assumptions), var_assign_(var_assign), UsedInSim_(false)
    {
    }

    void setSim()
    {
      assert(!UsedInSim_);
      UsedInSim_ = true;
    }
    void CheckSimed() const { assert(UsedInSim_); }
    void record_prev_assumption_len(unsigned l) { assmpt_len_ = l; }
    unsigned get_prev_assumption_len() const { return assmpt_len_; }

    smt::TermVec assumptions_;
    smt::UnorderedTermMap var_assign_;
    bool UsedInSim_;
    unsigned assmpt_len_;
  };  // end of class ChoiceItem

  // symsim branch
  class Branch{
    public:
    Branch(const std::vector<smt::UnorderedTermMap> & trace_,
           const std::vector<ChoiceItem> & history_choice_,
           const std::vector<smt::TermVec> & history_assumptions_,
           const std::vector<std::vector<std::string>> & history_assumptions_interp_)
        : trace_(trace_), history_choice_(history_choice_), history_assumptions_(history_assumptions_), history_assumptions_interp_(history_assumptions_interp_), finished(false)
        {}

    std::vector<smt::UnorderedTermMap> trace_;
    std::vector<ChoiceItem> history_choice_;
    std::vector<smt::TermVec> history_assumptions_;
    std::vector<std::vector<std::string>> history_assumptions_interp_;

    bool finished;
  };  // end of symsim branch

 public:
  // we will keep a reference to ts
  // and a copy of the pointer to the solver
  SymbolicSimulatorBranch(TransitionSystem & ts, const smt::SmtSolver & s)
      : ts_(ts), solver_(s), invar_(ts.inputvars()), svar_(ts.statevars())
  {
  }
  
 protected:
  TransitionSystem & ts_;
  smt::SmtSolver solver_;  // smt::SmtSolver is a smart pointer
  const smt::UnorderedTermSet & invar_;
  const smt::UnorderedTermSet & svar_;
  std::vector<smt::UnorderedTermMap> trace_;
  std::vector<ChoiceItem> history_choice_;
  std::vector<smt::TermVec> history_assumptions_;
  std::vector<std::vector<std::string>> history_assumptions_interp_;
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
  smt::Term new_var(int bitwdth,
                    const std::string & vname = "var",
                    bool x = true);

 public:
  // branch func
  void create_origin_branch();
  void create_branch(const size_t branch_idx);
  void finish_branch(const size_t branch_idx);

  /// get the length of the trace
  unsigned tracelen() const;  // branch
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
  void print_current_step() const;  // branch
  /// get the assumptions (collected from all previous steps)
  void print_current_step_assumptions() const;  // branch

  /// a shortcut to create symbolic variables/concrete values in a map
  smt::UnorderedTermMap convert(const assignment_type & vdict) const; // ok

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

  /// get (a copy of) the current state
  StateAsmpt get_curr_state(const smt::TermVec & assumptions = {}, size_t branch_idx = 0);  // branch, Not supported yet
  /// a shortcut to create a variable
  smt::Term set_var(int bitwdth, std::string vname = "var");  // ok, no used in pywasim

  /// get solver
  smt::SmtSolver get_solver() const { return solver_; } // ok

};
}  // namespace wasim
