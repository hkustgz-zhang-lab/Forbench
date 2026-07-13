#pragma once
#include "assert.h"
#include "math.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>

#include "smt-switch/bitwuzla_factory.h"

#include "smt-switch/smt.h"

#include "ts.h"
#include "term_manip.h"

#include "utils/exceptions.h"

#include "state_simplify.h"
#include "sygus_simplify.h"
#include "symsim.h"
#include "tracemgr.h"

#include <optional>

using namespace std;

namespace wasim {

using type_v = std::pair<std::string, int>;

class TraverseBranchingNode
{
 public:
  // create a branching node, based on whether it is a input or an internal signal
  TraverseBranchingNode(const std::optional<wasim::type_v> & input_v,
                        const std::optional<wasim::type_v> & signal_v) : value_(0)
  {
    // one and only one is specified.
    assert(input_v.has_value() || signal_v.has_value());
    assert(!(input_v.has_value() && signal_v.has_value()));
    branch_on_inputvar_ = input_v.has_value();
    const std::optional<wasim::type_v> & iv = branch_on_inputvar_ ? input_v : signal_v;
    v_name_ = iv.value().first;
    v_width_ = iv.value().second;
  }

  // move to the next possible value, return true if possible
  bool next() {
    value_++;
    if (value_ == pow(2, v_width_)) {
      return false;
    }
    return true;
  }

  // we may need to copy a node from a template
  TraverseBranchingNode copy_node_without_value() const {
    TraverseBranchingNode tmp = *this;
    tmp.value_ = 0;
    return tmp;
  }

  std::string repr() const {
    return (v_name_ + " == " + to_string(value_) + " "); }

  const std::string & var_name() const {return v_name_; }
  int var_value() const {return value_; }
  bool branch_on_inputvar() const {return branch_on_inputvar_;}

 protected:
  bool branch_on_inputvar_;
  std::string v_name_;
  int v_width_;
  int value_;
};

class PerStateStack
{
 public:
  PerStateStack(const std::vector<TraverseBranchingNode> & branching_point,
                const SymbolicSimulator & simulator)
      : solver_(simulator.get_solver()), simulator_(simulator), branching_point_(branching_point),
        ptr_(0), no_next_choice_(false)
  {  }

  std::string repr() const;
  bool has_valid_choice() const {return (!no_next_choice_);}

  // convert the choices so far into input assignments or assumptions
  // along witht the (optional) additional assumptions
  std::pair<smt::UnorderedTermMap, smt::TermVec> get_iv_asmpt(
      const smt::TermVec & assumptions) const;

  // try next choice allowed, return true if possible
  bool next_choice();
  // try next choice at the next level, return true if possible
  bool deeper_choice();

 protected:

  smt::SmtSolver solver_;
  const SymbolicSimulator & simulator_;
  // we only need convert (which is a const function anyway)

  std::vector<TraverseBranchingNode> stack_;
  int ptr_;
  std::vector<TraverseBranchingNode> branching_point_;
  bool no_next_choice_;
};

class SymbolicTraverse
{
 public:
  SymbolicTraverse(const TransitionSystem & ts,
                   SymbolicSimulator & simulator,
                   smt::SmtSolver & s,
                   const smt::UnorderedTermSet & base_variable)
      : sts_(ts), solver_(s), 
      simulator_(simulator), tracemgr_(ts, s),
      base_variable_(base_variable)
  {
    tracemgr_.record_base_var(base_variable);
  }

  // return the total number of states -- it is suggested these functions are called
  // from a brand-new SymbolicTraverse object
  // do not share SymbolicTraverse among different traversal
  unsigned traverse_one_step(const smt::TermVec & assumptions,
                         const std::vector<TraverseBranchingNode> & branching_point );

  // return the total number of states
  unsigned traverse(const smt::TermVec & assumptions,
                const std::vector<TraverseBranchingNode> & branching_point );

  
  const std::vector<StateAsmpt> & get_abs_state() const { return tracemgr_.get_abs_state(); }
  const std::vector<std::vector<StateAsmpt>> & get_all_branches() const { return all_branches_; }

 protected:
  const TransitionSystem & sts_;
  smt::SmtSolver solver_;
  SymbolicSimulator & simulator_;
  TraceManager tracemgr_;
  smt::UnorderedTermSet base_variable_;
  smt::UnorderedTermMap s_concrete_;

  // this is all branches that have been explored
  std::vector<std::vector<StateAsmpt>> all_branches_; 
};

}  // namespace wasim
