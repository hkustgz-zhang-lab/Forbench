#include <iostream>
#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/smt.h"
using namespace smt;
using namespace std;
int main()
{
  // Bitwuzla aliases booleans and bitvectors of size one
  // and also performs on-the-fly rewriting
  // if you'd like to maintain the term structure, you can
  // enable logging by passing true
  SmtSolver s = BitwuzlaSolverFactory::create(false);

  s->set_logic("QF_UFBV");
  s->set_opt("incremental", "true");
  s->set_opt("produce-models", "true");
  s->set_opt("produce-unsat-assumptions", "true");

  auto s_1 = s->make_term(1);
  auto s_0 = s->make_term(0);

  auto unsat_expr = s->make_term(smt::Equal, s_0, s_1);
  auto unsat_expr2 =
      s->make_term(smt::Equal, s_0, s->make_term(smt::And, s_1, s_1));

  s->push();
  s->assert_formula(unsat_expr);
  auto res1 = s->check_sat();
  s->pop();
  cout << res1.is_sat() << endl;

  auto sat_expr = s->make_term(smt::Equal, s_0, s_0);

  s->push();
  s->assert_formula(sat_expr);
  auto res2 = s->check_sat();
  s->pop();
  cout << res2.is_sat() << endl;

  s->push();
  s->assert_formula(unsat_expr);
  s->assert_formula(sat_expr);
  s->assert_formula(unsat_expr2);
  auto res3 = s->check_sat();
  s->pop();
  cout << res3.is_sat() << endl;

  s->push();
  s->assert_formula(sat_expr);
  auto res4 = s->check_sat();
  s->pop();
  cout << res4.is_sat() << endl;
}
