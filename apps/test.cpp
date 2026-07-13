#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/bitwuzla_factory.h"

using namespace wasim;
using namespace smt;

int main() {


  SmtSolver solver = BitwuzlaSolverFactory::create(false);

  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");

  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("design/test/pipe-no-stall.btor2", sts);

  std::cout << sts.trans()->to_string() << std::endl;
  
  SymbolicSimulator sim(sts, solver);
  
  auto varmap = sim.convert( { {"wen_stage2","v"}, {"tag2", 1} } );

  sim.init(varmap);

  auto s = sim.get_curr_state();

  std::cout << s.print() ;
  std::cout << s.print_assumptions();

  auto inputmap = sim.convert( {{"reg_init", 0}} );
  sim.set_input(inputmap, {});
  sim.sim_one_step();

  auto s2 = sim.get_curr_state();
  std::cout << s2.print();
  std::cout << s2.print_assumptions();

  sim.backtrack();
  sim.undo_set_input();


  return 0;
}
