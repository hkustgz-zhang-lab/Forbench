#include "independence_check.h"

#include "smt-switch/bitwuzla_factory.h"

namespace wasim {
// TODO: need to distinguish constant and op?
bool e_is_always_valid(const smt::Term & e,
                       smt::TermVec assumptions /*={}*/,
                       const smt::SmtSolver & s)
{
  bool is_bool = (e->get_sort()->get_sort_kind() == smt::BOOL);
  bool is_bv = (e->get_sort()->get_sort_kind() == smt::BV);

  if (is_bool) {
    auto not_e = s->make_term(smt::Not, e);
    assumptions.push_back(not_e);
  } else if (is_bv) {
    if (e->get_sort()->get_width() == 1) {
      auto sort_0 = s->make_sort(smt::BV, 1);
      auto bv_0 = s->make_term(0, sort_0);
      auto eq_e = s->make_term(smt::Equal, e, bv_0);
      assumptions.push_back(eq_e);
    } else
      throw SmtException("[e_is_always_valid] : expecting bit-width to be 1");
  }

  auto r = is_sat_res(assumptions, s);
  auto ret = r.is_unsat();

  return ret;
}

bool e_is_always_invalid(const smt::Term & e,
                         smt::TermVec assumptions /*={}*/,
                         const smt::SmtSolver & s)
{
  bool is_bool = (e->get_sort()->get_sort_kind() == smt::BOOL);
  bool is_bv = (e->get_sort()->get_sort_kind() == smt::BV);

  if (is_bool) {
    assumptions.push_back(e);
  } else if (is_bv) {
    if (e->get_sort()->get_width() == 1) {
      auto sort_1 = s->make_sort(smt::BV, 1);
      auto bv_1 = s->make_term(1, sort_1);
      auto eq_e = s->make_term(smt::Equal, e, bv_1);
      assumptions.push_back(eq_e);
    } else
      throw SmtException("[e_is_always_invalid] : expecting bit-width to be 1");
  }

  auto r = is_sat_res(assumptions, s);
  auto ret = r.is_unsat();

  return ret;
}

// will append longer and longer suffix until it becomes a fresh variable
static smt::Term try_get_fresh_variable_in_a_solver(const std::string & name,
                                                    const std::string & suffix,
                                                    const smt::Sort sort,
                                                    smt::SmtSolver & slv)
{
  bool succ;
  std::string curr_name = name + suffix;
  smt::Term ret;
  do {
    try {
      ret = slv->make_symbol(curr_name, sort);
      succ = true;
    }
    catch (const std::exception & e) {
      succ = false;
      curr_name += suffix;
    }
  } while (!succ);
  return ret;
}

bool e_is_independent_of_v(const smt::Term & e,
                           const smt::Term & v,
                           const smt::TermVec & assumptions)
{
  // HZ: the reason for making a new solver is because I want to make the
  // context of the original solver clear from the new variables we created
  // locally in this function I was hoping push/pop can be used to create
  // temporary SMT variables however, it is not the case.

  auto localSolver = smt::BitwuzlaSolverFactory::create(false);
  localSolver->set_logic("QF_UFBV");
  localSolver->set_opt("incremental", "true");
  localSolver->set_opt("produce-models", "true");
  localSolver->set_opt("produce-unsat-assumptions", "true");
  smt::TermTranslator translator(localSolver);

  auto e_local = translator.transfer_term(e);
  auto v_local = translator.transfer_term(v);

  auto w = v_local->get_sort()->get_width();
  auto sort_w = localSolver->make_sort(smt::BV, w);

  // I think we should never use get_symbol in this function
  // if it succeeds, it means there has been such a variable (maybe already in
  // use) but we want a fresh one, which does not interfere with the
  // constraints!
  smt::Term v1 = try_get_fresh_variable_in_a_solver(
      v_local->to_string(), "1", sort_w, localSolver);
  smt::Term v2 = try_get_fresh_variable_in_a_solver(
      v_local->to_string(), "2", sort_w, localSolver);

  smt::UnorderedTermMap sub_map1, sub_map2;
  sub_map1 = { { v_local, v1 } };
  sub_map2 = { { v_local, v2 } };
  auto e1 = localSolver->substitute(e_local, sub_map1);
  auto e2 = localSolver->substitute(e_local, sub_map2);

  auto e1_neq_e2 = localSolver->make_term(
      smt::Not, localSolver->make_term(smt::Equal, e1, e2));
  localSolver->push();
  localSolver->assert_formula(e1_neq_e2);

  // cout << "assump: " << assumptionSub_expr << endl;
  for (const auto & a : assumptions) {
    auto a_local = translator.transfer_term(a);
    localSolver->assert_formula(localSolver->substitute(a_local, sub_map1));
    localSolver->assert_formula(localSolver->substitute(a_local, sub_map2));
  }

  auto r = localSolver->check_sat();
  localSolver->pop();

  return r.is_unsat();
}

}  // namespace wasim
