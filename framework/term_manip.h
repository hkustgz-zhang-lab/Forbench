#pragma once

#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/smt.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <variant>
#include <vector>
#include "math.h"

namespace wasim {

// will try to create a variable starting with name n and will add number suffix
// to avoid naming conflicts
smt::Term free_make_symbol(const std::string & n,
                           smt::Sort symb_sort,
                           std::unordered_map<std::string, int> & name_cnt,
                           smt::SmtSolver & solver);

/**
 * Create one-hot expression for one_hot_vec, this allows 0 or 1 to be true
 */
smt::TermVec one_hot0(const smt::TermVec & one_hot_vec, smt::SmtSolver & solver);

// check satisfiability, if out != NULL, will update it with the sat model
smt::Result is_sat_res(const smt::TermVec & expr_vec, const smt::SmtSolver & solver, smt::UnorderedTermMap * out = NULL);

bool is_sat_bool(const smt::TermVec & expr_vec, const smt::SmtSolver & solver);
bool is_valid_bool(const smt::Term & expr, const smt::SmtSolver & solver);

std::vector<std::string> sort_model(const smt::UnorderedTermMap & cex);


smt::TermVec args(const smt::Term & term);

}  // namespace wasim
