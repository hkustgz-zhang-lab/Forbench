
#pragma once
#include "smt-switch/smt.h"

#include <cassert>


// This class will be used by both symsim.h and symsim_branch.h
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

  