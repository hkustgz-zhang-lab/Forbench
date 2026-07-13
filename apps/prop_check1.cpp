#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/inv_group.h"
#include "framework/symsim.h"
#include "framework/symtraverse.h"
#include "framework/term_manip.h"
#include "framework/traverse_manip.h"
#include "framework/ts.h"
#include "smt-switch/bitwuzla_factory.h"

using namespace wasim;
// using namespace smt;
using namespace std;
using namespace chrono;

int main()
{
  auto start = system_clock::now();
  std::unordered_set<std::string> base_sv = {
    "wen_stage1", "wen_stage2", "stage1", "stage2", "stage3"
  };
  TraverseBranchingNode node0({ make_pair("rst", 1) }, {});
  TraverseBranchingNode node1({}, { make_pair("stage1_go", 1) });
  TraverseBranchingNode node2({}, { make_pair("stage2_go", 1) });
  TraverseBranchingNode node3({}, { make_pair("stage3_go", 1) });

  auto order = { node0, node1, node2, node3 };

  // This reference the reference path
  std::string input_file =
      PROJECT_SOURCE_DIR "/design/testcase1-simple_pipe/simple_pipe.btor2";

  smt::SmtSolver solver = smt::BitwuzlaSolverFactory::create(false);
  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");
  // solver->set_opt("base-context-1", "true");
  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser(input_file, sts);

  SymbolicSimulator simulator(sts, solver);

  assignment_type init_map = { { "wen_stage1", "v1" },
                               { "wen_stage2", "v2" },
                               { "stage1", "a" },
                               { "stage2", "b" },
                               { "stage3", "c" } };

  auto map = simulator.convert(init_map);
  simulator.init(map);
  std::vector<StateAsmpt> state_list;
  std::vector<std::vector<StateAsmpt>> branch_list;
  StateRW staterw(solver);
  std::string out_dir = PROJECT_SOURCE_DIR "/output/c1/";
  branch_list = staterw.StateReadTree(out_dir, 345, 7);

  auto tag2 = tobool(simulator.var("tag2"), solver);
  auto tag3 = tobool(simulator.var("tag3"), solver);
  auto reg_v = simulator.var("reg_v");
  auto reg_v_comp = simulator.var("reg_v_comp");

  // InvGroup inv_group3(3, tag2, branch_list, solver);
  // inv_group3.branch2state();
  // auto inv_dedup3 = inv_group3.inv_dedup();

  InvGroup inv_group4(4, tag2, branch_list, solver);
  inv_group4.branch2state();
  auto inv_dedup4 = inv_group4.inv_dedup();
  auto inv_l4_prop = inv_group4.extract_prop("stage3");

  InvGroup inv_group5(5, tag3, branch_list, solver);
  inv_group5.branch2state();
  auto inv_dedup5 = inv_group5.inv_dedup();
  auto inv_l5_prop = inv_group5.extract_prop("stage3");

  auto pre_reg = inv_l4_prop;
  auto post_reg = inv_l5_prop;

  auto state_trans = sts.state_updates();
  auto reg_v_comp_trans = state_trans.at(reg_v_comp);

  smt::TermVec pre_vec = {};
  for (const auto & reg : pre_reg) {
    smt::Term eq_expr = {};
    eq_expr = solver->make_term(smt::Equal, reg_v, reg);
    pre_vec.push_back(eq_expr);
  }

  smt::TermVec post_vec = {};
  auto reg_v_comp_trans_not = solver->make_term(smt::Not, reg_v_comp_trans);
  for (const auto & reg : post_reg) {
    auto eq_expr = solver->make_term(smt::Equal, reg_v_comp_trans, reg);
    post_vec.push_back(eq_expr);
  }

  assert(pre_vec.size() == post_vec.size());

  smt::TermVec sat_check_vec = {};
  for (auto i = 0; i < pre_vec.size(); i++) {
    auto expr = solver->make_term(smt::And, pre_vec.at(i), post_vec.at(i));
    sat_check_vec.push_back(expr);
  }

  auto sat_check = solver->make_term(smt::Or, sat_check_vec);
  cout << sat_check->to_string() << endl;

  // solver->push();
  // solver->assert_formula(sat_check);
  // smt::Result r = solver->check_sat();

  auto r = is_sat_res(smt::TermVec{ sat_check }, solver);

  if (r.is_unsat()) {
    cout << "Formal Property Check Pass!" << endl;
  } else {
    cout << "Formal Property Check Fail!" << endl;
    cout << "Please check the following counter-example:" << endl;
    #error HERE is a problem. is_sat_res already pop. Model might be unavailable/unreliable
    auto cex = get_model(sat_check, solver);
    auto cex_str = sort_model(cex);
  }
  // solver->pop();

  auto end = system_clock::now();
  auto duration = duration_cast<seconds>(end - start);

  cout << "Program running time: "
       << double(duration.count()) * seconds::period::num / seconds::period::den
       << " (s)" << endl;
}
