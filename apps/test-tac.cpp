#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/bitwuzla_factory.h"
#include "timed-assertion-checker/timed_assertion_checker.h"

using namespace wasim;
using namespace smt;

int main() {


  SmtSolver solver = BitwuzlaSolverFactory::create(false);

  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");

  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("../design/test/adder.btor2", sts);

  std::cout << sts.trans()->to_string() << std::endl;
  
  /*------------------------------simulation--------------------------------*/
  SymbolicSimulator sim(sts, solver);
  sim.init();

    // timed assertion
  std::string my_assertion = "out@2 == a@0 + b@1";  // var with bit width format: (out@2)[3:0], (out@2)[0+:4]

  tac::TimedAssertionChecker test_tac(my_assertion, sts, sim, solver);

  test_tac.sim_max_step("",true,"clk"); // (rst_sig_name, set_rst_to_0, clk_sig_name)
  test_tac.print_term_map();
  test_tac.make_assertion_term();
  test_tac.assert_formula();
  test_tac.check_sat();

  return 0;
}

