#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include "simulation.h"
#include "sweeper/config.hpp"

#include "framework/ts.h"
#include "frontend/btor2_encoder.h"
#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/smtlib_reader.h"
#include "smt-switch/substitution_walker.h"
#include "smt-switch/utils.h"

namespace sweeper {

using smt::Term;
using smt::TermVec;
using smt::UnorderedTermSet;
using smt::SmtSolver;

class NodeData {
private:
    Term term_{nullptr};
    size_t bit_width_{0};
    std::vector<BtorBitVectorPtr> simulation_data_;
public:
    NodeData() = default;
    explicit NodeData(const Term & t) : term_(t), bit_width_(0) {}
    NodeData(const Term & t, const size_t & bw) : term_(t), bit_width_(bw) {}

    NodeData(const NodeData&) = delete;
    NodeData& operator=(const NodeData&) = delete;
    NodeData(NodeData&&) noexcept = default;
    NodeData& operator=(NodeData&&) noexcept = default;

    Term get_term() const noexcept { return term_; }
    size_t get_bit_width() const noexcept { return bit_width_; }

    std::vector<BtorBitVectorPtr>& get_simulation_data() noexcept { return simulation_data_; }
    const std::vector<BtorBitVectorPtr>& get_simulation_data() const noexcept { return simulation_data_; }

    size_t hash() const { return hash(simulation_data_); }
    static size_t hash(const std::vector<BtorBitVectorPtr>& data);
};

// ---- 工具/流程函数声明（实现见 node_data.cpp） ----
void create_lut(Term current, std::unordered_map<std::string, std::string>& lut);

void btor_bv_operation_1child(const smt::Op& op,
                              const BtorBitVector& btor_child_1,
                              NodeData &nd);

void btor_bv_operation_2children(const smt::Op& op,
                                 const BtorBitVector& btor_child_1,
                                 const BtorBitVector& btor_child_2,
                                 NodeData &nd);

void btor_bv_operation_3children(const smt::Op& op,
                                 const BtorBitVector& btor_child_1,
                                 const BtorBitVector& btor_child_2,
                                 const BtorBitVector& btor_child_3,
                                 NodeData &nd);

void process_single_child_simulation(const Term & child,
                                     int & num_iterations,
                                     const smt::Op& op_type,
                                     const std::unordered_map<Term, NodeData> & node_data_map,
                                     NodeData & out,
                                     bool debug=false);

void process_two_children_simulation(const TermVec & children,
                                     int & num_iterations,
                                     const smt::Op& op_type,
                                     const std::unordered_map<Term, NodeData>& node_data_map,
                                     const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                                     NodeData& nd, bool debug=false);

void process_three_children_simulation(const TermVec& children,
                                       int & num_iterations,
                                       const smt::Op& op_type,
                                       const std::unordered_map<Term, NodeData>& node_data_map,
                                       const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                                       NodeData& nd, bool debug=false);

void compute_simulation(const TermVec & children,
                        int& num_iterations,
                        const smt::Op& op_type,
                        const std::unordered_map<Term, NodeData>& node_data_map,
                        const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                        NodeData& nd);

void children_substitution(const TermVec& children,
                           TermVec& out,
                           const std::unordered_map<Term, Term>& substitution_map);

void initialize_arrays(const std::vector<wasim::TransitionSystem*>& systems,
                       std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                       std::unordered_map<Term, Term>& substitution_map,
                       bool & debug);

void match_term_constraint_pattern(const smt::TermVec & constraints,
                                   std::unordered_map<Term, std::string> & constraint_input_map);

inline bool has_simulation_data(const Term& t, const std::unordered_map<Term, NodeData>& node_data_map);

void simulate_constant_node(const Term& current,
                            int & num_iterations,
                            std::unordered_map<Term, NodeData>& node_data_map);

void simulate_leaf_node(const Term & current,
                        int & num_iterations,
                        std::unordered_map<Term, NodeData> & node_data_map,
                        std::string & dump_file_path,
                        std::string & load_file_path);

void count_total_nodes(const Term& root, int& total_nodes);

bool is_t1_deeper_than_t2(const Term & t1, const Term & t2);

// 其他在 sweeping 阶段会调用的工具
struct TryFindResult {
    bool found;
    Term term_eq;
    TermVec terms_for_solving;
    int hit_rank = -1;
};

TryFindResult try_find_equiv_term(const Term & cnode,
                                  const uint32_t & current_hash,
                                  const NodeData & sim_data,
                                  int & num_iterations,
                                  const std::unordered_map<uint32_t, TermVec> & hash_term_map,
                                  const std::unordered_map<Term, NodeData> & node_data_map,
                                  const std::unordered_map<Term, Term> & substitution_map,
                                  bool & debug);

TryFindResult try_find_equiv_term_heur(const Term & cnode,
                                  const uint32_t & current_hash,
                                  const NodeData & sim_data,
                                  int & num_iterations,
                                  const std::unordered_map<uint32_t, TermVec> & hash_term_map,
                                  const std::unordered_map<Term, NodeData> & node_data_map,
                                  const std::unordered_map<Term, Term> & substitution_map,
                                  bool & debug);

smt::Term get_first_unreducible_term (smt::TermList & assumption_list,
                                      const smt::SmtSolver & reducer,
                                      smt::Result & r);

void fill_simulation_data_for_all_nodes(std::unordered_map<Term, NodeData>& node_data_map,
                                        SmtSolver& solver,
                                        int num_iterations,
                                        std::unordered_map<Term, Term>& substitution_map,
                                        const std::unordered_map<Term, std::unordered_map<std::string, std::string>> & all_luts);

// and-折叠工具
smt::Term and_vec(const TermVec & v, SmtSolver & solver);

bool all_substituted(const TermVec& children,
                     const std::unordered_map<Term, Term>& subst_map);

} // namespace sweeper
