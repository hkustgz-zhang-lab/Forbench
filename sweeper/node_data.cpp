#include "sweeper/node_data.hpp"
#include "sweeper/simulation_engine.hpp"
#include <filesystem>
#include <sstream>
#include <optional>

using namespace smt;
using namespace std;

namespace sweeper {

size_t NodeData::hash(const std::vector<BtorBitVectorPtr>& data) {
    if (data.empty()) return 0;
    size_t hash_val = 0;
    for (const auto & v : data) {
        auto clean_val = btor_bv_to_string(*v);
        sweeper::hashCombine(hash_val, clean_val);
    }
    return hash_val;
}

void create_lut(Term current, std::unordered_map<std::string, std::string>& lut) {
    while (current->get_op().prim_op == PrimOp::Store) {
        TermVec ch(current->begin(), current->end());
        auto array = ch[0];
        auto index = ch[1];
        auto value = ch[2];
        lut[index->to_string().substr(2)] = value->to_string().substr(2);
        current = array;
    }
}

void btor_bv_operation_1child(const smt::Op& op, const BtorBitVector& c1, NodeData &nd) {
    if(op.prim_op == PrimOp::Not || op.prim_op == PrimOp::BVNot)
        nd.get_simulation_data().push_back(btor_bv_not(c1));
    else if(op.prim_op == PrimOp::Extract)
        nd.get_simulation_data().push_back(btor_bv_slice(c1, op.idx0, op.idx1));
    else if(op.prim_op == PrimOp::Zero_Extend)
        nd.get_simulation_data().push_back(btor_bv_uext(c1, op.idx0));
    else if(op.prim_op == PrimOp::Sign_Extend)
        nd.get_simulation_data().push_back(btor_bv_sext(c1, op.idx0));
    else if(op.prim_op == PrimOp::BVNeg)
        nd.get_simulation_data().push_back(btor_bv_neg(c1));
    else
        throw std::runtime_error("Unsupported 1-child op: " + op.to_string());
}

void btor_bv_operation_2children(const smt::Op& op,
                                 const BtorBitVector& a,
                                 const BtorBitVector& b,
                                 NodeData &nd) {
    auto &sim = nd.get_simulation_data();
    switch (op.prim_op) {
    case PrimOp::BVAdd:  sim.push_back(btor_bv_add(a,b)); break;
    case PrimOp::BVAnd:
    case PrimOp::And:    sim.push_back(btor_bv_and(a,b)); break;
    case PrimOp::Concat: sim.push_back(btor_bv_concat(a,b)); break;
    case PrimOp::Equal:  sim.push_back(btor_bv_eq(a,b)); break;
    case PrimOp::Xor:
    case PrimOp::BVXor:  sim.push_back(btor_bv_xor(a,b)); break;
    case PrimOp::Or:
    case PrimOp::BVOr:   sim.push_back(btor_bv_or(a,b)); break;
    case PrimOp::BVMul:  sim.push_back(btor_bv_mul(a,b)); break;
    case PrimOp::BVComp: sim.push_back(btor_bv_int64_to_bv(btor_bv_compare(a,b), 1)); break;
    case PrimOp::Distinct: sim.push_back(btor_bv_ne(a,b)); break;
    case PrimOp::BVUdiv: sim.push_back(btor_bv_udiv(a,b)); break;
    case PrimOp::BVSub:  sim.push_back(btor_bv_sub(a,b)); break;
    case PrimOp::BVUlt:  sim.push_back(btor_bv_ult(a,b)); break;
    case PrimOp::BVUle:  sim.push_back(btor_bv_ulte(a,b)); break;
    case PrimOp::BVUgt:  sim.push_back(btor_bv_ugt(a,b)); break;
    case PrimOp::BVUge:  sim.push_back(btor_bv_ugte(a,b)); break;
    case PrimOp::BVSlt:  sim.push_back(btor_bv_slt(a,b)); break;
    case PrimOp::BVSle:  sim.push_back(btor_bv_slte(a,b)); break;
    case PrimOp::BVSgt:  sim.push_back(btor_bv_sgt(a,b)); break;
    case PrimOp::BVSge:  sim.push_back(btor_bv_sgte(a,b)); break;
    case PrimOp::BVNand: sim.push_back(btor_bv_nand(a,b)); break;
    case PrimOp::BVNor:  sim.push_back(btor_bv_nor(a,b)); break;
    case PrimOp::BVXnor: sim.push_back(btor_bv_xnor(a,b)); break;
    case PrimOp::BVUrem: sim.push_back(btor_bv_urem(a,b)); break;
    case PrimOp::BVSdiv: sim.push_back(btor_bv_sdiv(a,b)); break;
    case PrimOp::BVSrem: sim.push_back(btor_bv_srem(a,b)); break;
    case PrimOp::BVLshr: sim.push_back(btor_bv_srl(a,b)); break;
    case PrimOp::BVAshr: sim.push_back(btor_bv_sra(a,b)); break;
    case PrimOp::BVShl:  sim.push_back(btor_bv_sll(a,b)); break;
    case PrimOp::Implies: sim.push_back(btor_bv_implies(a,b)); break;
    default:
        throw std::runtime_error("Unsupported 2-children op: " + op.to_string());
    }
}

void btor_bv_operation_3children(const smt::Op& op,
                                 const BtorBitVector& c, const BtorBitVector& t,
                                 const BtorBitVector& e, NodeData &nd) {
    if (op.prim_op != PrimOp::Ite)
        throw std::runtime_error("Unsupported 3-children op: " + op.to_string());
    auto result = btor_bv_ite(c, t, e);
    if (!result) throw std::runtime_error("Null result in btor_bv_operation_3children");
    nd.get_simulation_data().push_back(std::move(result));
}

void process_single_child_simulation(const Term & child,
                                     int & num_iterations,
                                     const smt::Op& op_type,
                                     const std::unordered_map<Term, NodeData> & node_data_map,
                                     NodeData & out,
                                     bool debug) {
    const auto & sim_data = node_data_map.at(child).get_simulation_data();
    if (debug && sim_data.size() != static_cast<size_t>(num_iterations))
        throw std::runtime_error("[Simulation Error] single child size mismatch");
    for (size_t i = 0; i < (size_t)num_iterations; ++i)
        btor_bv_operation_1child(op_type, *sim_data[i], out);
}

void process_two_children_simulation(const TermVec & children,
                                     int & num_iterations,
                                     const smt::Op& op_type,
                                     const std::unordered_map<Term, NodeData>& node_data_map,
                                     const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                                     NodeData& nd, bool debug) {
    if (op_type.prim_op == PrimOp::Select) {
        const auto& array_var  = children[0];
        const auto& index_term = children[1];
        const auto& sim_data_index = node_data_map.at(index_term).get_simulation_data();
        if (debug && sim_data_index.size() != static_cast<size_t>(num_iterations))
            throw std::runtime_error("[Simulation Error] index size mismatch");
        for (size_t i = 0; i < (size_t)num_iterations; ++i) {
            auto index_str = std::string(btor_bv_to_string(*sim_data_index[i]));
            const auto & val_str = all_luts.at(array_var).at(index_str);
            nd.get_simulation_data().push_back(btor_bv_char_to_bv(val_str.data()));
        }
        return;
    }

    const auto& child_1 = children[0];
    const auto& child_2 = children[1];
    
    // 检查子节点是否在 node_data_map 中
    if (node_data_map.find(child_1) == node_data_map.end()) {
        throw std::runtime_error("[Simulation Error] Child 1 not found in node_data_map: " + child_1->to_string());
    }
    if (node_data_map.find(child_2) == node_data_map.end()) {
        throw std::runtime_error("[Simulation Error] Child 2 not found in node_data_map: " + child_2->to_string());
    }
    
    const auto& sim_data_1 = node_data_map.at(child_1).get_simulation_data();
    const auto& sim_data_2 = node_data_map.at(child_2).get_simulation_data();
    if (sim_data_1.size() != static_cast<size_t>(num_iterations)
     || sim_data_2.size() != static_cast<size_t>(num_iterations))
        throw std::runtime_error("[Simulation Error] two children size mismatch");

    for (size_t i = 0; i < (size_t)num_iterations; ++i)
        btor_bv_operation_2children(op_type, *sim_data_1[i], *sim_data_2[i], nd);
}

void process_three_children_simulation(const TermVec& children,
                                       int & num_iterations,
                                       const smt::Op& op_type,
                                       const std::unordered_map<Term, NodeData>& node_data_map,
                                       const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                                       NodeData& nd, bool debug) {
    const auto& s1 = node_data_map.at(children[0]).get_simulation_data();
    const auto& s2 = node_data_map.at(children[1]).get_simulation_data();
    const auto& s3 = node_data_map.at(children[2]).get_simulation_data();
    if (s1.size() != static_cast<size_t>(num_iterations)
     || s2.size() != static_cast<size_t>(num_iterations)
     || s3.size() != static_cast<size_t>(num_iterations))
        throw std::runtime_error("[Simulation Error] three children size mismatch");

    for (size_t i = 0; i < (size_t)num_iterations; ++i)
        btor_bv_operation_3children(op_type, *s1[i], *s2[i], *s3[i], nd);
}

void compute_simulation(const TermVec & children,
                        int& num_iterations,
                        const smt::Op& op_type,
                        const std::unordered_map<Term, NodeData>& node_data_map,
                        const std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                        NodeData& nd) {
    if (children.size() == 1) process_single_child_simulation(children[0], num_iterations, op_type, node_data_map, nd);
    else if (children.size() == 2) process_two_children_simulation(children, num_iterations, op_type, node_data_map, all_luts, nd);
    else if (children.size() == 3) process_three_children_simulation(children, num_iterations, op_type, node_data_map, all_luts, nd);
    else throw std::runtime_error("[Simulation Error] Unsupported number of children: " + std::to_string(children.size()));
}

void children_substitution(const TermVec& children, TermVec& out,
                           const std::unordered_map<Term, Term>& substitution_map) {
    out.reserve(children.size());
    for (const auto & c : children) {
        if (c->get_sort()->get_sort_kind() == smt::ARRAY) { out.push_back(c); continue; }
        auto it = substitution_map.find(c);
        if (it == substitution_map.end())
            std::cout << "[Warning] No substitution found for term: " << c->to_string() << std::endl;
        out.push_back(it == substitution_map.end() ? c : it->second);
    }
}

void initialize_arrays(const std::vector<wasim::TransitionSystem*>& systems,
                       std::unordered_map<Term, std::unordered_map<std::string, std::string>>& all_luts,
                       std::unordered_map<Term, Term>& substitution_map,
                       bool & debug) {
    for (auto* sts : systems) {
        for (const auto& var_val_pair : sts->init_constants()) {
            if (var_val_pair.first->get_sort()->get_sort_kind() != smt::ARRAY) continue;
            Term var = var_val_pair.first;
            Term val = var_val_pair.second;
            if (all_luts.find(var) == all_luts.end()) {
                create_lut(val, all_luts[var]);
                if (debug) std::cout << "[array create] " << var->to_string()
                                     << " of size " << all_luts[var].size() << std::endl;
            }
        }
    }
    // 等价数组替换（相同 LUT）
    for (auto pos = all_luts.begin(); pos != all_luts.end(); ++pos) {
        const auto & array_var_i = pos->first;
        const auto & idx_val_i   = pos->second;
        bool another_array_found = false;
        for (auto pos_j = all_luts.begin(); pos_j != pos; ++pos_j ) {
            if (pos_j->second.size() != idx_val_i.size()) continue;
            bool all_equal = true;
            for (const auto & kv : idx_val_i) {
                auto it = pos_j->second.find(kv.first);
                if (it == pos_j->second.end() || it->second != kv.second) { all_equal = false; break; }
            }
            if (!all_equal) continue;
            substitution_map.insert({array_var_i, pos_j->first});
            another_array_found = true; break;
        }
        if (!another_array_found)
            substitution_map.insert({array_var_i, array_var_i});
    }
}

void match_term_constraint_pattern(const smt::TermVec & constraints,
                                   std::unordered_map<Term, std::string> & constraint_input_map) {
    auto extract_val = [](const Term & term) -> std::string { return term->to_string().substr(2); };
    auto try_add_entry = [&](const Term & sym_term, const Term & const_term) {
        if (!sym_term || !const_term || !sym_term->is_symbolic_const() || !const_term->is_value()) return;
        constraint_input_map[sym_term] = extract_val(const_term);
    };
    std::function<void(const Term&)> recursive_match;
    recursive_match = [&](const Term & t) {
        if (t->get_op().prim_op == smt::Equal) {
            TermVec ch(t->begin(), t->end());
            if (ch.size() != 2) return;
            Term lhs = ch[0], rhs = ch[1];
            try_add_entry(lhs, rhs);
            try_add_entry(rhs, lhs);
            if (lhs->get_op().prim_op == smt::BVComp && rhs->is_value() && extract_val(rhs) == "1") {
                TermVec cc(lhs->begin(), lhs->end());
                if (cc.size() != 2) return;
                try_add_entry(cc[0], cc[1]);
                try_add_entry(cc[1], cc[0]);
            }
            if (lhs->get_op().prim_op == smt::Extract && rhs->is_value()) {
                TermVec ec(lhs->begin(), lhs->end());
                if (ec.size() != 1 || !ec[0]->is_symbolic_const()) return;
                uint64_t high = lhs->get_op().idx0;
                uint64_t low  = lhs->get_op().idx1;
                std::string tag = "EXTRACT_" + std::to_string(high) + "_" + std::to_string(low);
                constraint_input_map[ec[0]] = tag + "=" + extract_val(rhs);
            }
        }
        for (auto it = t->begin(); it != t->end(); ++it) recursive_match(*it);
    };
    for (const auto & c : constraints) recursive_match(c);
}

inline bool has_simulation_data(const Term& t, const std::unordered_map<Term, NodeData>& node_data_map) {
    auto it = node_data_map.find(t);
    return it != node_data_map.end() && !it->second.get_simulation_data().empty();
}

void simulate_constant_node(const Term& current, int & num_iterations,
                            std::unordered_map<Term, NodeData>& node_data_map) {
    if (has_simulation_data(current, node_data_map)) return;
    NodeData nd(current);
    auto & sim_vec = nd.get_simulation_data();
    if (current->get_sort()->get_sort_kind() == smt::BOOL) {
        std::string bitval = (current->to_string() == "true") ? "1" : "0";
        for (int i = 0; i < num_iterations; ++i)
            sim_vec.push_back(btor_bv_char_to_bv(bitval.c_str()));
    } else {
        std::string val = current->to_string().substr(2);
        for (int i = 0; i < num_iterations; ++i)
            sim_vec.push_back(btor_bv_char_to_bv(val.c_str()));
    }
    node_data_map[current] = std::move(nd);
}

void simulate_leaf_node(const Term & current, int & num_iterations,
                        std::unordered_map<Term, NodeData> & node_data_map,
                        std::string & dump_file_path,
                        std::string & load_file_path) {
    // 输入变量的仿真值已在 simulation() 批量生成；若遗漏则抛错
    if (!has_simulation_data(current, node_data_map)) {
        throw std::runtime_error("[Simulation Error] missing input assignment for " + current->to_string());
    }
}

void count_total_nodes(const Term& root, int& total_nodes) {
    std::stack<Term> st; std::unordered_set<Term> vis;
    st.push(root);
    while (!st.empty()) {
        Term cur = st.top(); st.pop();
        if (vis.count(cur)) continue;
        vis.insert(cur);
        ++total_nodes;
        for (auto c : cur) {
            auto k = c->get_sort()->get_sort_kind();
            if (k == smt::BV || k == smt::BOOL) st.push(c);
        }
    }
}

// 等价候选查找：哈希一致 + 仿真逐条比对
TryFindResult try_find_equiv_term(const Term & cnode,
                                  const uint32_t & current_hash,
                                  const NodeData & sim_data,
                                  int & num_iterations,
                                  const std::unordered_map<uint32_t, TermVec> & hash_term_map,
                                  const std::unordered_map<Term, NodeData> & node_data_map,
                                  const std::unordered_map<Term, Term> & substitution_map,
                                  bool & debug) {
    TryFindResult result{false, nullptr, {}};
    auto ht = hash_term_map.find(current_hash);
    if (ht == hash_term_map.end()) return result;

    const auto & terms_to_check = ht->second;
    const auto & simv = sim_data.get_simulation_data();

    for (const auto & t : terms_to_check) {
        if (t == cnode) { result.found = true; result.term_eq = t; return result; }
        if (t->get_sort() != cnode->get_sort()) continue;
        auto it = node_data_map.find(t);
        if (it == node_data_map.end()) continue;

        const auto & es = it->second.get_simulation_data();
        bool match = true;
        for (int i = 0; i < num_iterations; ++i) {
            if (btor_bv_compare(*es[i], *simv[i]) != 0) { match = false; break; }
        }
        if (match) result.terms_for_solving.push_back(t);
    }
    return result;
}

//=================================================================
//                     Modern Heuristic Algorithms
//=================================================================

std::size_t set_intersection_size(const std::vector<int>& a,
                                  const std::vector<int>& b
) {
     /* 1. 选较小序列建哈希表 */
    const std::vector<int>* small = &a;
    const std::vector<int>* large = &b;
    if (a.size() > b.size()) std::swap(small, large);

    /* 2. 以 (元素数 ×2) 预留桶，负载因子≈0.5，避免 rehash */
    std::unordered_set<int> lookup(small->begin(), small->end(),
                                   small->size() * 2);

    /* 3. 扫描较大序列统计命中 */
    std::size_t cnt = 0;
    for (int elem : *large)
        if (lookup.count(elem)) ++cnt;

    return cnt;
}

struct PairHash {
    std::size_t operator()(const std::pair<int,int>& p) const noexcept
    {
        // 经典 64-bit splitmix64 组合
        std::size_t h1 = static_cast<std::size_t>(p.first);
        std::size_t h2 = static_cast<std::size_t>(p.second);

        // mix h1
        h1 ^= h1 >> 33;  h1 *= 0xff51afd7ed558ccdULL;
        h1 ^= h1 >> 33;  h1 *= 0xc4ceb9fe1a85ec53ULL;
        h1 ^= h1 >> 33;

        // mix h2
        h2 ^= h2 >> 33;  h2 *= 0xff51afd7ed558ccdULL;
        h2 ^= h2 >> 33;  h2 *= 0xc4ceb9fe1a85ec53ULL;
        h2 ^= h2 >> 33;

        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1<<6) + (h1>>2));
    }
};

std::size_t set_intersection_size(const std::vector<std::pair<int,int>>& a,
                                  const std::vector<std::pair<int,int>>& b)
{
    using Edge = std::pair<int,int>;

    /* 1. 选较小序列建表 */
    const std::vector<Edge>* small = &a;
    const std::vector<Edge>* large = &b;
    if (a.size() > b.size()) std::swap(small, large);

    /* 2. 建立哈希表，桶数≈元素数×2，负载因子≈0.5 */
    std::unordered_set<Edge, PairHash> lookup(
        small->begin(), small->end(), small->size() * 2);

    /* 3. 扫描大序列统计命中 */
    std::size_t cnt = 0;
    for (const Edge& e : *large)
        if (lookup.count(e)) ++cnt;

    return cnt;
}

struct GraphEntry {
    std::vector<int>                 vertices;
    std::vector<std::pair<int,int>>  edges;
};

static std::unordered_map<int, GraphEntry> cache_each_node;
static const GraphEntry& collect_nodes_edges(const Term& term
){

    auto it = cache_each_node.find(term->get_id());
    if (it != cache_each_node.end()) return it->second;

    GraphEntry entry;
    std::stack<Term> st;
    st.push(term);
    std::unordered_set<int> visited;
    while (!st.empty()) {
        Term cur = st.top(); 
        st.pop();
        auto id_cur = static_cast<int>(cur->get_id());
        if (!visited.insert(id_cur).second) continue;  // 已访问
        entry.vertices.push_back(id_cur);
        for (const Term& child : *cur) {
            int id_ch = static_cast<int>(child->get_id());
            entry.edges.emplace_back(id_cur, id_ch);
            st.push(child);
        }
    }
    auto [iter, _] = cache_each_node.emplace(term->get_id(), std::move(entry));
    return iter->second;
}


double computeVEO(const Term& t1, 
                  const Term& t2, 
                  double alpha = 0.5){
    const GraphEntry& g1 = collect_nodes_edges(t1);
    const GraphEntry& g2 = collect_nodes_edges(t2);
    auto& v1 = g1.vertices;
    auto& v2 = g2.vertices;
    auto& e1 = g1.edges;
    auto& e2 = g2.edges;

    size_t v_inter = set_intersection_size(v1, v2);
    size_t e_inter = set_intersection_size(e1, e2);

    double v_overlap = static_cast<double>(v_inter) / (v1.size() + v2.size() - v_inter + 1e-12);
    double e_overlap = static_cast<double>(e_inter) / (e1.size() + e2.size() - e_inter + 1e-12);
    return alpha * v_overlap + (1.0 - alpha) * e_overlap;
}



struct WLBagEntry {
    std::unordered_map<std::size_t,int> bag;   // 颜色计数
};

static std::unordered_map<int, WLBagEntry> wl_cache; 
using Color = std::size_t;
static std::unordered_map<Color,int>
WL_feature_bag_raw(const Term& root, int h)
{
    std::unordered_map<int, std::vector<int>> adj;
    std::vector<int> nodes;
    nodes.reserve(128);

    std::stack<Term> st; st.push(root);
    while (!st.empty()) {
        Term cur = st.top(); st.pop();
        int id = static_cast<int>(cur->get_id());
        if (!adj.emplace(id, std::vector<int>{}).second) continue; // 已插入
        nodes.push_back(id);
        for (const Term& ch : *cur) {
            int cid = static_cast<int>(ch->get_id());
            adj[id].push_back(cid);
            st.push(ch);
        }
    }

    std::unordered_map<int,Color> color;
    color.reserve(nodes.size());
    for (int n : nodes) color[n] = adj[n].size();

    for (int iter = 0; iter < h; ++iter) {
        bool changed = false;
        std::unordered_map<int,Color> new_color;
        new_color.reserve(nodes.size());

        for (int n : nodes) {
            std::vector<Color> multiset;
            multiset.reserve(adj[n].size());
            for (int nb : adj[n]) multiset.push_back(color[nb]);
            std::sort(multiset.begin(), multiset.end());
            Color hash = 17;
            for (Color c : multiset) hash = hash * 31u + c;
            new_color[n] = hash;
            if (hash != color[n]) changed = true;
        }
        color.swap(new_color);
        if (!changed) break;                 // ★ 已收敛，提前退出
    }

    std::unordered_map<Color,int> bag;
    bag.reserve(color.size());
    for (auto& kv : color) ++bag[kv.second];
    return bag;
}


/* ---------- 获取词袋（带缓存） ---------- */
static const std::unordered_map<Color,int>&
get_WL_bag_cached(const Term& root, int depth = 2)
{
    int id = static_cast<int>(root->get_id());
    auto it = wl_cache.find(id);
    if (it != wl_cache.end()) return it->second.bag;

    WLBagEntry e;
    e.bag = WL_feature_bag_raw(root, depth);
    auto [iter, _] = wl_cache.emplace(id, std::move(e));
    return iter->second.bag;
}

static double compute_WL_kernel(const Term& t1,
                                const Term& t2,
                                int depth = 2)
{
    const auto& b1 = get_WL_bag_cached(t1, depth);
    const auto& b2 = get_WL_bag_cached(t2, depth);

    double dot = 0, n1 = 0, n2 = 0;
    for (auto& kv : b1) {
        int c1 = kv.second;
        n1 += c1 * c1;
        auto it = b2.find(kv.first);
        if (it != b2.end()) dot += c1 * it->second;
    }
    for (auto& kv : b2) n2 += kv.second * kv.second;

    double norm = std::sqrt(n1 * n2) + 1e-12;
    return dot / norm;
}

static double compute_beta(const std::vector<double>& vec_veo,
                           const std::vector<double>& vec_wl)
{
    auto variance = [](const std::vector<double>& v){
        if (v.empty()) return 0.0;
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double sum  = 0.0;
        for (double x : v) sum += (x-mean)*(x-mean);
        return sum / v.size();
    };
    double sv  = std::sqrt(variance(vec_veo));
    double swl = std::sqrt(variance(vec_wl));
    double denom = sv + swl + 1e-12;
    return (denom < 1e-9) ? 0.5 : (1.0 - sv / denom);
}


// using some heuristic algorithm
TryFindResult try_find_equiv_term_heur(const Term & cnode,
                                       const uint32_t & current_hash,
                                       const NodeData & sim_data,
                                       int & num_iterations,
                                       const std::unordered_map<uint32_t, TermVec> & hash_term_map,
                                       const std::unordered_map<Term, NodeData> & node_data_map,
                                       const std::unordered_map<Term, Term> & substitution_map,
                                       bool & debug) {
    TryFindResult result{false, nullptr, {}};
    auto ht = hash_term_map.find(current_hash);
    if (ht == hash_term_map.end()) return result; // 没有哈希匹配的项

    const auto & terms_to_check = ht->second;
    const auto & simv = sim_data.get_simulation_data();

    for (const auto & t : terms_to_check) {
        if (t == cnode) { 
            result.found = true; 
            result.term_eq = t;
            return result; 
        }
        
        if (t->get_sort() != cnode->get_sort()) continue;  // 排除类型不匹配的项
        auto it = node_data_map.find(t);
        if (it == node_data_map.end()) continue; // 排除未仿真的项

        const auto & es = it->second.get_simulation_data();
        bool match = true;
        for (int i = 0; i < num_iterations; ++i) {
            if (btor_bv_compare(*es[i], *simv[i]) != 0) { 
                match = false; // 仿真数据不匹配 
                break; 
            }
        }
        if (match) result.terms_for_solving.push_back(t);
    }

    //here we get a terms_for_solving vector
    //then we need to reorder this vector using heuristic algorithm

    if (result.terms_for_solving.empty()) return result; // 如果没有找到匹配的项，直接返回
    
    //Vertex Edge Overlap, Weisfeiler–Lehman Kernel
    struct Metric {
        Term   t;
        double veo;
        double wl;
    };
    std::vector<Metric> feats;
    feats.reserve(result.terms_for_solving.size());

    constexpr double VEO_ALPHA   = 0.5;   // 顶点/边权重
    constexpr int    WL_DEPTH    = 2;     // WL 迭代层数
    constexpr double SCORE_BETA  = 0.6;   // VEO 与 WL 线性融合权重
    constexpr double VEO_PRUNE   = 0.25;

    // for (const Term & cand : result.terms_for_solving) {
    //     double veo = computeVEO(cnode, cand, VEO_ALPHA);
    //     double wl  = 0.0;
    //     if (veo >= VEO_PRUNE)
    //         wl = compute_WL_kernel(cnode, cand, WL_DEPTH);   // 只算“更像”的候选
    //     feats.push_back({cand, veo, 0.0});
    // }

    std::vector<double> vec_veo, vec_wl;
    vec_veo.reserve(result.terms_for_solving.size());
    vec_wl .reserve(result.terms_for_solving.size());

    for (const Term& cand : result.terms_for_solving) {
        double veo = computeVEO(cnode, cand, VEO_ALPHA);

        double wl  = 0.0;
        if (veo >= VEO_PRUNE)                 // ★ 低 VEO 的候选不再算 WL
            wl = compute_WL_kernel(cnode, cand, WL_DEPTH);

        feats.push_back({cand, veo, wl});
        vec_veo.push_back(veo);
        vec_wl .push_back(wl);
    }


    //------------------------------------------------------------------
    // 3. 排序：按 β·VEO + (1-β)·WL 降序
    //------------------------------------------------------------------

    double BETA = compute_beta(vec_veo, vec_wl);
    std::stable_sort(feats.begin(), feats.end(),
        [&](const Metric& a, const Metric& b) {
            double sa = BETA * a.veo + (1.0 - BETA) * a.wl;
            double sb = BETA * b.veo + (1.0 - BETA) * b.wl;
            if (sa != sb) return sa > sb;
            return a.t.get() < b.t.get();
    });

    /* ---------- Top-K 推送后续 SAT ---------- */
    constexpr int TOP_K = 8;          // 可按需要调大 / 调小
    result.terms_for_solving.clear();
    for (int i = 0; i < feats.size() && i < TOP_K; ++i)
        result.terms_for_solving.push_back(feats[i].t);


    

    return result;
}


smt::Term and_vec(const TermVec & v, SmtSolver & solver)
{
    if (v.empty()) return solver->make_term(true);
    if (v.size() == 1) return v.at(0);
    auto ret = v.at(0);
    for (size_t i = 1; i < v.size(); ++i)
        ret = solver->make_term(smt::And, ret, v.at(i));
    return ret;
}

bool all_substituted(const TermVec& children,
                     const std::unordered_map<Term, Term>& subst_map) {
    for (const auto & c : children)
        if (subst_map.find(c) == subst_map.end())
            return false;
    return true;
}


void fill_simulation_data_for_all_nodes(std::unordered_map<Term, NodeData>& node_data_map,
                                        SmtSolver& solver,
                                        int num_iterations,
                                        std::unordered_map<Term, Term>& substitution_map,
                                        const std::unordered_map<Term, std::unordered_map<std::string, std::string>> & all_luts) {
    for (auto& [term, nd] : node_data_map) {
        auto& sim_vec = nd.get_simulation_data();
        int cur = static_cast<int>(sim_vec.size());
        if (cur >= num_iterations) continue;

        if (term->is_value()) {
            std::string vs = (term->get_sort()->get_sort_kind() == smt::BOOL)
                           ? ((term->to_string() == "true") ? "1" : "0")
                           : term->to_string().substr(2);
            auto bv = btor_bv_char_to_bv(vs.c_str());
            for (int i = cur; i < num_iterations; ++i)
                sim_vec.push_back(btor_bv_copy(*bv)); // 使用copy函数创建新的unique_ptr
        } else if (term->is_symbolic_const() && term->get_op().is_null()) {
            Term val = solver->get_value(term);
            std::string vs = (term->get_sort()->get_sort_kind() == smt::BOOL)
                           ? ((val->to_string() == "true") ? "1" : "0")
                           : val->to_string().substr(2);
            sim_vec.push_back(btor_bv_char_to_bv(vs.c_str()));
        } else {
            TermVec ch(term->begin(), term->end()), sub;
            children_substitution(ch, sub, substitution_map);
            NodeData tmp; int one = 1;
            compute_simulation(sub, one, term->get_op(), node_data_map, all_luts, tmp);
            sim_vec.push_back(std::move(tmp.get_simulation_data().front()));
        }

        if (sim_vec.size() != static_cast<size_t>(num_iterations)) {
            std::cerr << "[FATAL] Post-fill sim_data size mismatch for term: "
                      << term->to_string() << " size: " << sim_vec.size()
                      << " vs expected: " << num_iterations << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

smt::Term get_first_unreducible_term (smt::TermList & assumption_list,
                                      const smt::SmtSolver & reducer,
                                      smt::Result & r) {
    r = reducer->check_sat_assuming_list(assumption_list);
    if (!r.is_unsat()) return nullptr;
    auto pos = assumption_list.begin();
    while (pos != assumption_list.end()) {
        Term t = *pos;
        auto after = assumption_list.erase(pos);
        r = reducer->check_sat_assuming_list(assumption_list);
        pos = assumption_list.insert(after, t);
        if (r.is_sat()) return t;
        // 若仍 UNSAT，可结合 core 缩减（此处略，保持接口）
        break;
    }
    return nullptr;
}


int compute_ast_level(const Term & t, std::unordered_map<Term, int> & memo) {
    // 若已缓存则直接返回
    if (memo.find(t) != memo.end()) {
        return memo[t];
    }

    // 叶子节点（常量或符号）level 为 1
    if (t->is_symbolic_const() || t->is_value()) {
        memo[t] = 1;
        return 1;
    }

    // 递归获取所有子节点的最大层级
    int max_child_level = 0;
    for (const Term & child : *t) {
        int child_level = compute_ast_level(child, memo);
        if (child_level > max_child_level) {
            max_child_level = child_level;
        }
    }

    // 当前节点的level是子节点最大值 + 1
    memo[t] = max_child_level + 1;
    return memo[t];
}

// 若 t1 的 AST 层级更深，则返回 true，否则返回 false
bool is_t1_deeper_than_t2(const Term & t1, const Term & t2) {
    std::unordered_map<Term, int> memo;

    int level1 = compute_ast_level(t1, memo);
    int level2 = compute_ast_level(t2, memo);

    return level1 > level2;
}


} // namespace sweeper
