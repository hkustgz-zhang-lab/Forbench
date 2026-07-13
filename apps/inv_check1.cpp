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

  auto tag0 = tobool(simulator.var("tag0"), solver);
  auto tag1 = tobool(simulator.var("tag1"), solver);
  auto tag2 = tobool(simulator.var("tag2"), solver);
  auto tag3 = tobool(simulator.var("tag3"), solver);
  auto wen_stage1 = simulator.var("wen_stage1");
  auto wen_stage2 = simulator.var("wen_stage2");
  auto stage1 = simulator.var("stage1");
  auto stage2 = simulator.var("stage2");
  auto stage3 = simulator.var("stage3");
  auto reg_v = simulator.var("reg_v");
  auto reg_v_comp = simulator.var("reg_v_comp");
  auto rst = simulator.var("rst");

  auto state_init = branch_list.at(0).at(0);
  // auto asmpt_init = solver->make_term(smt::And, state_init.asmpt_);
  smt::TermVec state_expr_vec = {};
  for (const auto & sv : state_init.sv_) {
    state_expr_vec.push_back(
        solver->make_term(smt::Equal, sv.first, sv.second));
  }
  auto state_expr = solver->make_term(smt::And, state_expr_vec);
  auto free_var = get_free_variables(state_expr);

  smt::Term v1, v2, a, b, c;
  for (const auto & var : free_var) {
    if (var->to_string() == "v1")
      v1 = var;
    else if (var->to_string() == "v2")
      v2 = var;
    else if (var->to_string() == "a")
      a = var;
    else if (var->to_string() == "b")
      b = var;
    else if (var->to_string() == "c")
      c = var;
  }

  // tag0 - tag0
  InvGroup inv_group0(0, tag0, branch_list, solver);
  inv_group0.branch2state();
  // tag0 - tag1
  InvGroup inv_group1(1, tag1, branch_list, solver);
  inv_group1.branch2state();
  // tag1 - tag1
  InvGroup inv_group2(2, tag1, branch_list, solver);
  inv_group2.branch2state();
  // tag1 - tag2
  InvGroup inv_group3(3, tag2, branch_list, solver);
  inv_group3.branch2state();
  // tag2 - tag2
  InvGroup inv_group4(4, tag2, branch_list, solver);
  inv_group4.branch2state();
  // tag2 - tag3
  InvGroup inv_group5(5, tag3, branch_list, solver);
  inv_group5.branch2state();
  // tag3 - tag3
  InvGroup inv_group6(6, tag3, branch_list, solver);
  inv_group6.branch2state();

  // begin inv check
  auto trans_sts = sts.trans();
  smt::TermVec assume_vec;
  for (const auto & cons : sts.constraints()) {
    assume_vec.push_back(solver->make_term(
        smt::Equal, cons.first, solver->make_term(cons.second)));
  }
  assume_vec.push_back(trans_sts);
  auto trans = solver->make_term(smt::And, assume_vec);

  smt::UnorderedTermMap var_to_next = {};
  for (const auto & sv : map) {
    auto var = sv.second;
    auto st = var->get_sort();
    var_to_next[var] = solver->make_symbol(var->to_string() + "_next", st);
  }

  smt::TermVec next_expr_vec = {};
  for (const auto & var : var_to_next) {
    next_expr_vec.push_back(
        solver->make_term(smt::Equal, var.second, var.first));
  }

  // define trans
  auto trans_nxt = solver->make_term(smt::And, next_expr_vec);
  auto trans_update = solver->make_term(smt::And, { trans, trans_nxt });

  // define assumptions
  auto asmpt_rst1 = solver->make_term(smt::Equal, rst, solver->make_term(0));
  auto asmpt_rst2 =
      solver->make_term(smt::Equal,
                        solver->substitute(rst, sts.state_updates()),
                        solver->make_term(0));
  auto one_hot_tag = one_hot({ tag0, tag1, tag2, tag3 }, solver);
  auto asmpt_tag = solver->make_term(smt::And, one_hot_tag);
  auto asmpt_prop = solver->make_term(
      smt::Implies,
      tag3,
      solver->make_term(
          smt::Equal,
          reg_v_comp,
          solver->make_term(
              smt::BVAdd,
              solver->make_term(1, reg_v->get_sort()),
              solver->make_term(smt::BVMul,
                                reg_v,
                                solver->make_term(2, reg_v->get_sort())))));
  auto asmpt = solver->make_term(
      smt::And, { asmpt_rst1, asmpt_rst2, asmpt_tag, asmpt_prop });

  // define inv
  auto inv_vec0 = inv_group0.inv_group();
  auto inv_l0 = solver->make_term(smt::Or, inv_vec0);
  auto inv_vec1 = inv_group1.inv_group();
  auto inv_l1 = solver->make_term(smt::Or, inv_vec1);
  auto inv_vec2 = inv_group2.inv_group();
  auto inv_l2 = solver->make_term(smt::Or, inv_vec2);
  auto inv_vec3 = inv_group3.inv_group();
  auto inv_l3 = solver->make_term(smt::Or, inv_vec3);
  auto inv_vec4 = inv_group4.inv_group();
  auto inv_l4_prop = inv_group4.extract_prop("stage3");
  auto inv_vec5 = inv_group5.inv_group();
  auto inv_l5 = solver->make_term(smt::Or, inv_vec5);
  auto inv_vec6 = inv_group6.inv_group();
  auto inv_l6 = solver->make_term(smt::Or, inv_vec6);
  smt::TermVec inv_prop1 = {};
  for (const auto & prop : inv_l4_prop) {
    inv_prop1.push_back(solver->make_term(
        smt::Implies, tag3, solver->make_term(smt::Equal, reg_v, prop)));
  }
  auto inv_tag0 = inv_l0;
  auto inv_tag1 = solver->make_term(smt::Or, inv_l1, inv_l2);
  auto inv_tag2_p1 = inv_l3;
  auto inv_tag3_p2 = inv_l6;
  smt::TermVec inv_prop2 = {};
  assert(inv_vec4.size() == inv_vec5.size());
  for (int i = 0; i < inv_vec4.size(); i++) {
    inv_prop2.push_back(solver->make_term(
        smt::And,
        smt::TermVec{ inv_vec4.at(i), inv_vec5.at(i), inv_prop1.at(i) }));
  }
  auto inv_prop_dedup = deduplicate(inv_prop2);
  auto inv_prop_expr = solver->make_term(smt::Or, inv_prop_dedup);
  smt::Term inv = {};
  inv = solver->make_term(
      smt::And,
      smt::TermVec{
          inv_tag0, inv_tag1, inv_tag2_p1, inv_tag3_p2, inv_prop_expr });
  // define inv end

  check_res res_inv = {};
  res_inv = inv_check(inv, trans_update, asmpt, sts, solver);

  /// CEGAR
  std::unordered_set<std::string> tag_set = { "tag0", "tag1", "tag2", "tag3" };
  std::unordered_set<std::string> ctrl_set = { "v1", "v2" };
  smt::TermVec tag_record = {};
  smt::TermVec unsat_expr_record = {};
  while (res_inv.first == false) {
    // int i = 1;
    // while (i!=0)
    // {
    //     i--;
    auto cex = res_inv.second;
    auto cex_res = cex_parser(cex, tag_set, ctrl_set, solver);
    auto cex_tag = cex_res.first;
    auto cex_expr = solver->make_term(smt::And, cex_res.second);

    tag_record.push_back(cex_tag);
    auto unsat_expr = solver->make_term(smt::Not, cex_expr);
    unsat_expr_record.push_back(unsat_expr);

    std::vector<int> truevec = {};
    if (cex_tag->to_string() == "tag0") {
      auto truevec0 = test_cex(inv_vec0, cex, solver);
      auto truevec0_us = inv_group0.check_unsat_expr(truevec0, cex_expr);
      auto truevec1 = test_cex(inv_vec1, cex, solver);
      auto truevec1_us = inv_group1.check_unsat_expr(truevec1, cex_expr);
      truevec = comb_unq(truevec0_us, truevec1_us);
    } else if (cex_tag->to_string() == "tag1") {
      auto truevec2 = test_cex(inv_vec2, cex, solver);
      auto truevec2_us = inv_group2.check_unsat_expr(truevec2, cex_expr);
      auto truevec3 = test_cex(inv_vec3, cex, solver);
      auto truevec3_us = inv_group3.check_unsat_expr(truevec3, cex_expr);
      truevec = comb_unq(truevec3_us, truevec3_us);
    } else if (cex_tag->to_string() == "tag2") {
      auto truevec4 = test_cex(inv_vec4, cex, solver);
      auto truevec4_us = inv_group4.check_unsat_expr(truevec4, cex_expr);
      auto truevec5 = test_cex(inv_vec5, cex, solver);
      auto truevec5_us = inv_group5.check_unsat_expr(truevec5, cex_expr);
      truevec = comb_unq(truevec4_us, truevec5_us);
    } else if (cex_tag->to_string() == "tag3") {
      auto truevec6 = test_cex(inv_vec6, cex, solver);
      auto truevec6_us = inv_group6.check_unsat_expr(truevec6, cex_expr);
      truevec = comb_unq(truevec6_us, std::vector<int>{});
    }
    for (const auto & idx : truevec) {
      inv_group0.add_unsat_asmpt(unsat_expr, idx);
      inv_group1.add_unsat_asmpt(unsat_expr, idx);
      inv_group2.add_unsat_asmpt(unsat_expr, idx);
      inv_group3.add_unsat_asmpt(unsat_expr, idx);
      inv_group4.add_unsat_asmpt(unsat_expr, idx);
      inv_group5.add_unsat_asmpt(unsat_expr, idx);
      inv_group6.add_unsat_asmpt(unsat_expr, idx);
    }
    auto inv_vec0 = inv_group0.inv_group();
    auto inv_l0 = solver->make_term(smt::Or, inv_vec0);
    auto inv_vec1 = inv_group1.inv_group();
    auto inv_l1 = solver->make_term(smt::Or, inv_vec1);
    auto inv_vec2 = inv_group2.inv_group();
    auto inv_l2 = solver->make_term(smt::Or, inv_vec2);
    auto inv_vec3 = inv_group3.inv_group();
    auto inv_l3 = solver->make_term(smt::Or, inv_vec3);
    auto inv_vec4 = inv_group4.inv_group();
    auto inv_l4_prop = inv_group4.extract_prop("stage3");
    auto inv_vec5 = inv_group5.inv_group();
    auto inv_l5 = solver->make_term(smt::Or, inv_vec5);
    auto inv_vec6 = inv_group6.inv_group();
    auto inv_l6 = solver->make_term(smt::Or, inv_vec6);

    smt::TermVec inv_prop1 = {};
    for (const auto & prop : inv_l4_prop) {
      inv_prop1.push_back(solver->make_term(
          smt::Implies, tag3, solver->make_term(smt::Equal, reg_v, prop)));
    }

    auto inv_tag0 = inv_l0;
    auto inv_tag1 = solver->make_term(smt::Or, inv_l1, inv_l2);
    auto inv_tag2_p1 = inv_l3;
    auto inv_tag3_p2 = inv_l6;
    smt::TermVec inv_prop2 = {};
    assert(inv_vec4.size() == inv_vec5.size());
    for (int i = 0; i < inv_vec4.size(); i++) {
      inv_prop2.push_back(solver->make_term(
          smt::And,
          smt::TermVec{ inv_vec4.at(i), inv_vec5.at(i), inv_prop1.at(i) }));
    }
    auto inv_prop_dedup = deduplicate(inv_prop2);
    auto inv_prop_expr = solver->make_term(smt::Or, inv_prop_dedup);
    inv = solver->make_term(
        smt::And,
        smt::TermVec{
            inv_tag0, inv_tag1, inv_tag2_p1, inv_tag3_p2, inv_prop_expr });

    res_inv = inv_check(inv, trans_update, asmpt, sts, solver);
  }

  /// init check
  auto init_sts = sts.init();
  auto init = solver->make_term(
      smt::And,
      smt::TermVec{
          init_sts,
          solver->make_term(smt::Equal, wen_stage1, v1),
          solver->make_term(smt::Equal, wen_stage2, v2),
          solver->make_term(smt::Equal, stage1, a),
          solver->make_term(smt::Equal, stage2, b),
          solver->make_term(smt::Equal, stage3, c),
          solver->make_term(smt::Equal, rst, solver->make_term(0)),
          solver->make_term(smt::Equal,
                            solver->substitute(rst, sts.state_updates()),
                            solver->make_term(0)) });

  auto res_init = init_check(init, inv, solver->make_term(1), solver);

  // prop check
  smt::Term asert = {};
  if (btor_parser.propvec().size() == 1) {
    asert = btor_parser.propvec().at(0);
  } else {
    asert = solver->make_term(smt::And, btor_parser.propvec());
  }
  auto res_prop = prop_check(asert, inv, asmpt, solver);

  auto end = system_clock::now();
  auto duration = duration_cast<seconds>(end - start);

  cout << "Program running time: "
       << double(duration.count()) * seconds::period::num / seconds::period::den
       << " (s)" << endl;
}
