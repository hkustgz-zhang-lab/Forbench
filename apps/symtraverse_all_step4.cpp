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

smt::TermVec tag2asmpt_c3(std::string flag,
                          SymbolicSimulator & simulator,
                          smt::SmtSolver & solver)
{
  smt::TermVec ret;
  smt::Term ret_term0, ret_term1, ret_term2;
  smt::Term lhs0, lhs1, lhs2;
  smt::Term rhs_0 = solver->make_term(0, solver->make_sort(smt::BV, 1));
  smt::Term rhs_1 = solver->make_term(1, solver->make_sort(smt::BV, 1));
  if (flag == "start-id") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
  } else if (flag == "id-id") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_id_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    cout << "ret_term2:" << ret_term2 << endl;
    ret.push_back(ret_term2);
  } else if (flag == "id-ex") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_id_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_1);
    ret.push_back(ret_term2);
  } else if (flag == "ex-ex") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_ex_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    cout << "ret_term2:" << ret_term2 << endl;
    ret.push_back(ret_term2);
  } else if (flag == "ex-wb") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_ex_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_1);
    ret.push_back(ret_term2);
  } else if (flag == "wb-wb") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_wb_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    ret.push_back(ret_term2);
  } else if (flag == "wb-finish") {
    lhs0 = simulator.var("rst");
    ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
    ret.push_back(ret_term0);
    lhs1 = simulator.var("dummy_reset");
    ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
    ret.push_back(ret_term1);
    lhs2 = simulator.var("RTL_wb_go");
    ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_1);
    ret.push_back(ret_term2);
  } else {
    cout << "<ERROR> Wrong tag transition format!" << endl;
    assert(false);
  }
  return { solver->make_term(smt::And, ret) };
}

int main()
{
  auto start = system_clock::now();
  std::unordered_set<std::string> base_sv = {
    "RTL_if_id_inst",     "RTL_if_id_valid",   "RTL_id_ex_operand1",
    "RTL_id_ex_operand2", "RTL_id_ex_op",      "RTL_id_ex_rd",
    "RTL_id_ex_reg_wen",  "RTL_id_ex_valid",   "RTL_ex_wb_val",
    "RTL_ex_wb_rd",       "RTL_ex_wb_reg_wen", "RTL_ex_wb_valid",
    "RTL_registers[0]",   "RTL_registers[1]",  "RTL_registers[2]",
    "RTL_registers[3]",   "RTL_scoreboard[0]", "RTL_scoreboard[1]",
    "RTL_scoreboard[2]",  "RTL_scoreboard[3]", "__VLG_I_inst",
    "__VLG_I_inst_valid"
  };
  TraverseBranchingNode node0({ make_pair("rst", 1) }, {});
  TraverseBranchingNode node1({}, { make_pair("RTL_id_go", 1) });
  TraverseBranchingNode node2({}, { make_pair("RTL_ex_go", 1) });
  TraverseBranchingNode node3({}, { make_pair("RTL_wb_go", 1) });

  auto order = { node0, node1, node2, node3 };

  // This reference the reference path
  std::string input_file =
      PROJECT_SOURCE_DIR "/design/testcase4-four_stage_pipe2/problem_add.btor2";

  smt::SmtSolver solver = smt::BitwuzlaSolverFactory::create(false);
  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");
  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser(input_file, sts);
  auto invar = sts.inputvars();
  for (const auto & var : invar) {
    std::cout << var->to_string() << std::endl;
  }
  cout << "\n\n state var: \n" << endl;
  auto svar = sts.statevars();
  for (const auto & var : svar) {
    std::cout << var->to_string() << std::endl;
  }
  auto bvs = solver->make_sort(smt::BV, 1);
  auto t1 = solver->make_term(1, bvs);
  cout << t1->to_string() << endl;
  // auto bls = solver->make_sort(smt::BOOL, 1);
  auto t2 = solver->make_term(1);
  cout << t2->to_string() << endl;

  SymbolicSimulator simulator(sts, solver);

  assignment_type init_map = {
    { "RTL_if_id_inst", "inst_id" },   { "RTL_id_ex_operand1", "oper1" },
    { "RTL_id_ex_operand2", "oper2" }, { "RTL_id_ex_op", "op" },
    { "RTL_id_ex_rd", "rd1" },         { "RTL_id_ex_reg_wen", "w1" },
    { "RTL_ex_wb_val", "ex_val" },     { "RTL_ex_wb_rd", "rd2" },
    { "RTL_ex_wb_reg_wen", "w2" },     { "RTL_if_id_valid", "v0" },
    { "RTL_id_ex_valid", "v1" },       { "RTL_ex_wb_valid", "v2" },
    { "RTL_registers[0]", "reg0" },    { "RTL_registers[1]", "reg1" },
    { "RTL_registers[2]", "reg2" },    { "RTL_registers[3]", "reg3" },
    { "RTL_scoreboard[0]", "s0" },     { "RTL_scoreboard[1]", "s1" },
    { "RTL_scoreboard[2]", "s2" },     { "RTL_scoreboard[3]", "s3" },
    { "__VLG_I_inst", "inst" },        { "__VLG_I_inst_valid", "inst_v" },
    { "__ILA_I_inst", "ila_inst" },
  };

  auto map = simulator.convert(init_map);
  simulator.init(map);
  std::vector<StateAsmpt> state_list;
  std::vector<std::vector<StateAsmpt>> branch_list;

  // step: start
  cout << "step: start" << endl;
  auto s_init = simulator.get_curr_state();
  auto is_not_start0 =
      solver->make_term(smt::Equal,
                        simulator.var("__START__"),
                        solver->make_term(0, solver->make_sort(smt::BV, 1)));
  auto is_not_start =
      simulator.interpret_state_expr_on_curr_frame(is_not_start0);
  auto init_asmpt = s_init.asmpt_;
  auto r_t = is_sat_res(init_asmpt, solver);
  assert(not r_t.is_sat());
  smt::TermVec pop_vec = {};

  for (const auto & sv : s_init.sv_) {
    auto s = sv.first;
    auto v = sv.second;
    if (base_sv.find(s->to_string()) == base_sv.end()) {
      pop_vec.push_back(s);
    }
  }
  for (const auto & s : pop_vec) {
    s_init.sv_.erase(s);
  }
  state_list.push_back(s_init);
  branch_list.push_back(state_list);
  s_init.print();
  s_init.print_assumptions();

  std::unordered_set<std::string> base_sv_start = {
    "RTL_if_id_inst",     "RTL_if_id_valid",   "RTL_id_ex_operand1",
    "RTL_id_ex_operand2", "RTL_id_ex_op",      "RTL_id_ex_rd",
    "RTL_id_ex_reg_wen",  "RTL_id_ex_valid",   "RTL_ex_wb_val",
    "RTL_ex_wb_rd",       "RTL_ex_wb_reg_wen", "RTL_ex_wb_valid",
    "RTL_registers[0]",   "RTL_registers[1]",  "RTL_registers[2]",
    "RTL_registers[3]",   "RTL_scoreboard[0]", "RTL_scoreboard[1]",
    "RTL_scoreboard[2]",  "RTL_scoreboard[3]",
  };

  // step: start-ex
  cout << "step: start --> id" << endl;
  auto asmpt_start_id = tag2asmpt_c3("start-id", simulator, solver);
  extend_branch_next_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_start,
                           "start-id",
                           asmpt_start_id,
                           { { "__START__", 1 },
                             { "stage_tracker_if_id_iuv", 0 },
                             { "stage_tracker_id_ex_iuv", 0 },
                             { "stage_tracker_ex_wb_iuv", 0 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  cout << "step: id --> id" << endl;
  auto asmpt_id_id = tag2asmpt_c3("id-id", simulator, solver);
  extend_branch_same_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_start,
                           "id-id",
                           asmpt_id_id,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 1 },
                             { "stage_tracker_id_ex_iuv", 0 },
                             { "stage_tracker_ex_wb_iuv", 0 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  std::unordered_set<std::string> base_sv_id = {
    "RTL_id_ex_operand1", "RTL_id_ex_operand2", "RTL_id_ex_op",
    "RTL_id_ex_rd",       "RTL_id_ex_reg_wen",  "RTL_id_ex_valid",
    "RTL_ex_wb_val",      "RTL_ex_wb_rd",       "RTL_ex_wb_reg_wen",
    "RTL_ex_wb_valid",    "RTL_registers[0]",   "RTL_registers[1]",
    "RTL_registers[2]",   "RTL_registers[3]",
  };

  cout << "step: id --> ex" << endl;
  auto asmpt_id_ex = tag2asmpt_c3("id-ex", simulator, solver);
  extend_branch_next_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_id,
                           "id-ex",
                           asmpt_id_ex,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 1 },
                             { "stage_tracker_id_ex_iuv", 0 },
                             { "stage_tracker_ex_wb_iuv", 0 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  cout << "step: ex --> ex" << endl;
  auto asmpt_ex_ex = tag2asmpt_c3("ex-ex", simulator, solver);
  extend_branch_same_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_id,
                           "ex-ex",
                           asmpt_ex_ex,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 0 },
                             { "stage_tracker_id_ex_iuv", 1 },
                             { "stage_tracker_ex_wb_iuv", 0 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  std::unordered_set<std::string> base_sv_wb = {
    "RTL_ex_wb_val",    "RTL_ex_wb_rd",     "RTL_ex_wb_reg_wen",
    "RTL_ex_wb_valid",  "RTL_registers[0]", "RTL_registers[1]",
    "RTL_registers[2]", "RTL_registers[3]"
  };

  cout << "step: ex --> wb" << endl;
  auto asmpt_ex_wb = tag2asmpt_c3("ex-wb", simulator, solver);
  extend_branch_next_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_wb,
                           "ex-wb",
                           asmpt_ex_wb,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 0 },
                             { "stage_tracker_id_ex_iuv", 1 },
                             { "stage_tracker_ex_wb_iuv", 0 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  cout << "step: wb --> wb" << endl;
  auto asmpt_wb_wb = tag2asmpt_c3("wb-wb", simulator, solver);
  extend_branch_same_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_wb,
                           "wb-wb",
                           asmpt_wb_wb,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 0 },
                             { "stage_tracker_id_ex_iuv", 0 },
                             { "stage_tracker_ex_wb_iuv", 1 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  std::unordered_set<std::string> base_sv_finish = { "RTL_registers[0]",
                                                     "RTL_registers[1]",
                                                     "RTL_registers[2]",
                                                     "RTL_registers[3]" };
  cout << "step: wb --> finish" << endl;
  auto asmpt_wb_finish = tag2asmpt_c3("wb-finish", simulator, solver);
  extend_branch_next_phase(branch_list,
                           simulator,
                           sts,
                           base_sv_finish,
                           "wb-finish",
                           asmpt_wb_finish,
                           { { "__START__", 0 },
                             { "stage_tracker_if_id_iuv", 0 },
                             { "stage_tracker_id_ex_iuv", 0 },
                             { "stage_tracker_ex_wb_iuv", 1 },
                             { "stage_tracker_wb_iuv", 0 } },
                           order,
                           solver);

  auto end = system_clock::now();
  auto duration = duration_cast<seconds>(end - start);

  cout << "Program running time: "
       << double(duration.count()) * seconds::period::num / seconds::period::den
       << " (s)" << endl;

  StateRW staterw(solver);
  std::string out_dir = PROJECT_SOURCE_DIR "/output/c4/";
  staterw.StateWriteTree(branch_list, out_dir);
}
