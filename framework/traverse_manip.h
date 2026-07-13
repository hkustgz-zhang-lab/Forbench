#pragma once
// #include "assert.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>

#include "smt-switch/bitwuzla_factory.h"
// #include "../deps/smt-switch/local/include/smt-switch/boolector_extensions.h"

#include "smt-switch/smt.h"
// #include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

// #include "../utils/exceptions.h"

#include "symsim.h"
#include "symtraverse.h"
#include "ts.h"

// #include <iostream>
// #include <unordered_map>
// #include <utility>
// #include <functional>
// #include <vector>
// #include <set>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <variant>
#include <vector>

using namespace std;

namespace wasim {
void extend_branch_init(std::vector<std::vector<StateAsmpt>> & branch_list,
                        SymbolicSimulator & simulator,
                        TransitionSystem & sts,
                        std::unordered_set<std::string> base_sv,
                        std::string flag,
                        smt::TermVec flag_asmpt,
                        std::vector<TraverseBranchingNode> order,
                        smt::SmtSolver & solver);

void extend_branch_next_phase(
    std::vector<std::vector<StateAsmpt>> & branch_list,
    SymbolicSimulator & simulator,
    TransitionSystem & sts,
    std::unordered_set<std::string> base_sv,
    std::string flag,
    smt::TermVec flag_asmpt,
    assignment_type phase_marker,
    std::vector<TraverseBranchingNode> order,
    smt::SmtSolver & solver);

void extend_branch_same_phase(
    std::vector<std::vector<StateAsmpt>> & branch_list,
    SymbolicSimulator & simulator,
    TransitionSystem & sts,
    std::unordered_set<std::string> base_sv,
    std::string flag,
    smt::TermVec flag_asmpt,
    assignment_type phase_marker,
    std::vector<TraverseBranchingNode> order,
    smt::SmtSolver & solver);
}  // namespace wasim
