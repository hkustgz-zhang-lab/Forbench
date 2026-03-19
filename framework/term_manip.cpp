#include "term_manip.h"

#include "smt-switch/utils.h"

using namespace std;

namespace wasim {

smt::Term free_make_symbol(const std::string & n,
                           smt::Sort symb_sort,
                           std::unordered_map<std::string, int> & name_cnt,
                           smt::SmtSolver & solver)
{
  int cnt;
  if (name_cnt.find(n) == name_cnt.end())
    cnt = 0;
  else
    cnt = name_cnt[n];

  do {
    ++cnt;
    name_cnt[n] = cnt;
    smt::Term symb;
    try {
      symb = solver->make_symbol(n + "#" + std::to_string(cnt), symb_sort);
      return symb;
    }
    catch (const std::exception & e) {  // maybe name conflict
      // std::cout << "New symbol: " << n + std::to_string(cnt) << " failed." <<
      // std::endl;
    }
  } while (true);
}


smt::TermVec one_hot0(const smt::TermVec & one_hot_vec, smt::SmtSolver & solver)
{
  smt::TermVec ret;
  auto ll = one_hot_vec.size();
  for (int i = 0; i < ll; i++) {
    for (int j = i + 1; j < ll; j++) {
      ret.push_back(solver->make_term(
          smt::Not,
          solver->make_term(smt::And,
                            { one_hot_vec.at(i), one_hot_vec.at(j) })));
    }
  }

  return ret;
}


smt::Result is_sat_res(const smt::TermVec & expr_vec, const smt::SmtSolver & solver, smt::UnorderedTermMap * out)
{
  // solver->push();
  auto r = solver->check_sat_assuming(expr_vec);

  if (r.is_sat() && out != NULL) {
    smt::UnorderedTermSet free_var_set;
    for (const auto & a : expr_vec)
      smt::get_free_symbols(a, free_var_set);
    for (const auto & t : free_var_set) {
      out->emplace(t, solver->get_value(t));
    }
  } // end of is_sat and requires model

  // solver->pop();
  return r;
}

bool is_sat_bool(const smt::TermVec & expr_vec, const smt::SmtSolver & solver)
{
  return is_sat_res(expr_vec, solver, NULL).is_sat();
}

bool is_valid_bool(const smt::Term & expr, const smt::SmtSolver & solver)
{
  auto expr_not = solver->make_term(smt::Not, expr);
  return (! is_sat_bool(smt::TermVec{ expr_not }, solver));
}

std::vector<std::string> sort_model(const smt::UnorderedTermMap & cex)
{
  std::vector<std::string> cex_vec;
  for (const auto & sv : cex) {
    auto var = sv.first;
    auto value = sv.second;
    std::string cex_expr = var->to_string() + " := " + value->to_string();
    cex_vec.push_back(cex_expr);
  }
  std::sort(cex_vec.begin(), cex_vec.end());
  for (const auto & cex_str : cex_vec) {
    cout << cex_str << endl;
  }
  return cex_vec;
}

smt::TermVec args(const smt::Term & term)
{
  smt::TermVec arg_vec;
  for (auto pos = term->begin(); pos != term->end(); ++pos)
    arg_vec.push_back(*pos);

  return arg_vec;
}

}  // namespace wasim
