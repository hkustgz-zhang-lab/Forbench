#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
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
smt::TermVec tag2asmpt_c1(std::string flag,
                          SymbolicSimulator & simulator,
                          smt::SmtSolver & solver)
{
  smt::TermVec ret;
  smt::Term ret_term, ret_term2;
  smt::Term lhs, lhs2;
  smt::Term rhs_0 = solver->make_term(0, solver->make_sort(smt::BV, 1));
  smt::Term rhs_1 = solver->make_term(1, solver->make_sort(smt::BV, 1));
  if (flag == "tag0_0") {
    lhs = simulator.var("stage1_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
    ret.push_back(ret_term);
  } else if (flag == "tag0_1") {
    lhs = simulator.var("stage1_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    ret.push_back(ret_term);
  } else if (flag == "tag1_1") {
    lhs = simulator.var("stage1_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
    lhs2 = simulator.var("stage2_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    ret.push_back(ret_term);
    ret.push_back(ret_term2);
  } else if (flag == "tag1_2") {
    lhs = simulator.var("stage2_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    ret.push_back(ret_term);
  } else if (flag == "tag2_2") {
    lhs = simulator.var("stage2_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
    lhs2 = simulator.var("stage3_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    ret.push_back(ret_term);
    ret.push_back(ret_term2);
  } else if (flag == "tag2_3") {
    lhs = simulator.var("stage3_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    ret.push_back(ret_term);
  } else if (flag == "tag3_3") {
    lhs = simulator.var("stage3_go");
    ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
    ret.push_back(ret_term);
  } else {
    cout << "<ERROR> Wrong tag transition format!" << endl;
    assert(false);
  }
  return ret;
}

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

  // step: tag0 --> tag0
  cout << "step: tag0 --> tag0" << endl;
  auto asmpt_tag0_0 = tag2asmpt_c1("tag0_0", simulator, solver);
  extend_branch_init(branch_list,
                     simulator,
                     sts,
                     base_sv,
                     "tag0_0",
                     asmpt_tag0_0,
                     order,
                     solver);

  // step: tag0 --> tag1
  cout << "\n\n\nstep: tag0 --> tag1" << endl;
  auto asmpt_tag0_1 = tag2asmpt_c1("tag0_1", simulator, solver);
  extend_branch_next_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag0_1",
      asmpt_tag0_1,
      { { "tag0", 1 }, { "tag1", 0 }, { "tag2", 0 }, { "tag3", 0 } },
      order,
      solver);

  // step: tag1 --> tag1
  cout << "\n\n\nstep: tag1 --> tag1" << endl;
  auto asmpt_tag1_1 = tag2asmpt_c1("tag1_1", simulator, solver);
  extend_branch_same_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag1_1",
      asmpt_tag1_1,
      { { "tag0", 0 }, { "tag1", 1 }, { "tag2", 0 }, { "tag3", 0 } },
      order,
      solver);

  // step: tag1 --> tag2
  cout << "\n\n\nstep: tag1 --> tag2" << endl;
  auto asmpt_tag1_2 = tag2asmpt_c1("tag1_2", simulator, solver);
  extend_branch_next_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag1_2",
      asmpt_tag1_2,
      { { "tag0", 0 }, { "tag1", 1 }, { "tag2", 0 }, { "tag3", 0 } },
      order,
      solver);

  // step: tag2 --> tag2
  cout << "\n\n\nstep: tag2 --> tag2" << endl;
  auto asmpt_tag2_2 = tag2asmpt_c1("tag2_2", simulator, solver);
  extend_branch_same_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag2_2",
      asmpt_tag2_2,
      { { "tag0", 0 }, { "tag1", 0 }, { "tag2", 1 }, { "tag3", 0 } },
      order,
      solver);

  // step: tag2 --> tag3
  cout << "\n\n\nstep: tag2 --> tag3" << endl;
  auto asmpt_tag2_3 = tag2asmpt_c1("tag2_3", simulator, solver);
  extend_branch_next_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag2_3",
      asmpt_tag2_3,
      { { "tag0", 0 }, { "tag1", 0 }, { "tag2", 1 }, { "tag3", 0 } },
      order,
      solver);

  // step: tag3 --> tag3
  cout << "\n\n\nstep: tag3 --> tag3" << endl;
  auto asmpt_tag3_3 = tag2asmpt_c1("tag3_3", simulator, solver);
  extend_branch_same_phase(
      branch_list,
      simulator,
      sts,
      base_sv,
      "tag3_3",
      asmpt_tag3_3,
      { { "tag0", 0 }, { "tag1", 0 }, { "tag2", 0 }, { "tag3", 1 } },
      order,
      solver);

  auto end = system_clock::now();
  auto duration = duration_cast<seconds>(end - start);

  cout << "Program running time: "
       << double(duration.count()) * seconds::period::num / seconds::period::den
       << " (s)" << endl;

  StateRW staterw(solver);
  std::string out_dir = PROJECT_SOURCE_DIR "/output/c1/";
  staterw.StateWriteTree(branch_list, out_dir);
}
