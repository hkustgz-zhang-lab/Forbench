#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include <string>
#include <chrono>
#include "sweeper/config.hpp"
#include "sweeper/node_data.hpp"
#include "sweeper/simulation_engine.hpp"

namespace sweeper {

struct SweeperOptions {
    int find_unsat;
    int find_sat;
    std::vector<wasim::TransitionSystem*> systems;
};

struct SweeperStats {
    int solver_queries = 0;
    int unsat_count = 0;
    int sat_count = 0;
    std::chrono::milliseconds total_sat_time{0};
    std::chrono::milliseconds total_unsat_time{0};
    std::chrono::milliseconds ordering_time{0};
    size_t hash_map_size = 0;
    size_t substitution_map_size = 0;
    size_t node_data_map_size = 0;
};

// pre-collect constants
void pre_collect_constants(const std::vector<smt::Term>& traversal_roots,
                           std::unordered_map<smt::Term, NodeData>& node_data_map,
                           std::unordered_map<uint32_t, smt::TermVec>& hash_term_map,
                           std::unordered_map<smt::Term, smt::Term>& substitution_map,
                           const int & num_iterations);

// post-order sweeping 主过程
void post_order(const smt::Term & root,
                std::unordered_map<smt::Term, NodeData> & node_data_map,
                std::unordered_map<uint32_t, smt::TermVec> & hash_term_map,
                std::unordered_map<smt::Term, smt::Term> & substitution_map,
                const std::unordered_map<smt::Term, std::unordered_map<std::string, std::string>> & all_luts,
                int & count, int & unsat_count, int & sat_count,
                smt::SmtSolver & solver, int & num_iterations, bool dump_enable,
                int timeout_ms, bool debug,
                std::string & dump_file_path, std::string & load_file_path,
                std::chrono::milliseconds & total_sat_time,
                 std::chrono::milliseconds & total_unsat_time,
                 std::chrono::milliseconds & ordering_time,
                 int required_unsat,
                 int required_sat);

SweeperStats sweeper(smt::Term & root,
                     smt::SmtSolver & solver,
                     const Config & config,
                     const SweeperOptions & options,
                     const smt::TermVec & constraints = smt::TermVec());

} // namespace sweeper
