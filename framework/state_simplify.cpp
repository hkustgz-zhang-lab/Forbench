#include "state_simplify.h"
#include "term_manip.h"
#include "independence_check.h"

#include "smt-switch/utils.h"
#include <queue>

namespace wasim {

using namespace smt;

void get_xvar_sub(const smt::TermVec & assumptions,
                  const smt::UnorderedTermSet & set_of_xvar,
                  const smt::UnorderedTermSet & free_var,
                  const smt::SmtSolver & solver,
                  smt::UnorderedTermMap & xvar_sub)
{
  auto bv1_sort = solver->make_sort(smt::BV, 1);
  for (const auto & xvar : set_of_xvar) {
    if (free_var.find(xvar) == free_var.end())
      continue;
    if (xvar->get_sort()->get_sort_kind() == smt::SortKind::BOOL) {
      auto reducible = is_reducible_bool(xvar, assumptions, solver);
      if (reducible == 0) {
        xvar_sub[xvar] = solver->make_term(0);
      } else if (reducible == 1) {
        xvar_sub[xvar] = solver->make_term(1);
      }
    // end of BOOL kind
    } else if (xvar->get_sort()->get_sort_kind() == smt::SortKind::BV) {
      if (xvar->get_sort()->get_width() == 1) {
        auto reducible = is_reducible_bv_width1(xvar, assumptions, solver);
        if (reducible == 0) {
          xvar_sub[xvar] = solver->make_term(0, bv1_sort);
        } else if (reducible == 1) {
          xvar_sub[xvar] = solver->make_term(1, bv1_sort);
        }
      }
    } // end if BV kind
    // will not try simplify in the other cases
  } // end of for each xvar
} // end of get_xvar_sub


void get_xvar_independent(const smt::TermVec & assumptions,
                  const smt::UnorderedTermSet & set_of_xvar,
                  const smt::Term & expr,
                  const smt::SmtSolver & solver,
                  smt::UnorderedTermSet & xvar_that_can_be_removed) {

  smt::UnorderedTermSet free_var;
  smt::get_free_symbols(expr, free_var);
  for (const auto & v : free_var) {
    if (set_of_xvar.find(v) == set_of_xvar.end()) continue;
    // keep only the intersection of free_var and set_of_xvar
    if (e_is_independent_of_v(expr, v, assumptions))
      xvar_that_can_be_removed.emplace(v);
  }
} // get_xvar_independent


bool expr_contains_X(const smt::Term & expr, const smt::UnorderedTermSet & set_of_xvar)
{
  smt::UnorderedTermSet vars_in_expr;
  smt::get_free_symbols(expr, vars_in_expr);
  for (const auto & var : vars_in_expr)
    if (set_of_xvar.find(var) != set_of_xvar.end())
      return true;
  return false;
}

// check if expr is a constant under assumptions
smt::Term check_if_constant(
    const smt::Term & expr,
    const smt::TermVec & assumptions, 
    const smt::SmtSolver & solver) {
  
  auto r = solver->check_sat_assuming(assumptions);
  if (!r.is_sat()) {
    return nullptr;
  }
  TermVec assumptions_all(assumptions);
  auto retval = solver->get_value(expr);
  assumptions_all.push_back(solver->make_term(
    smt::Not, solver->make_term(smt::Equal, expr, retval) ));
  r = solver->check_sat_assuming(assumptions_all);
  if (r.is_unsat()) {
    return retval; // expr == retval is required
  }
  // not a constant
  return nullptr;
}
    

int is_reducible_bool(const smt::Term & expr,
                      const smt::TermVec & assumptions,
                      const smt::SmtSolver & solver)
{
  bool is_bool_sort = expr->get_sort()->get_sort_kind() == smt::SortKind::BOOL;
  smt::TermVec check_vec_true(assumptions);
  smt::Term eq_expr_true =
      solver->make_term(smt::Equal, expr,
      is_bool_sort ? solver->make_term(true) : 
                     solver->make_term(1, solver->make_sort(smt::SortKind::BV, 1)));
  check_vec_true.push_back(eq_expr_true);

  auto r_t = is_sat_res(check_vec_true, solver);

  smt::TermVec check_vec_false(assumptions);
  smt::Term eq_expr_false =
      solver->make_term(smt::Equal, expr, 
      is_bool_sort ? solver->make_term(false) : 
                     solver->make_term(0, solver->make_sort(smt::SortKind::BV, 1)));
  check_vec_false.push_back(eq_expr_false);
  // auto r_f = solver->check_sat_assuming(check_vec_false);
  auto r_f = is_sat_res(check_vec_false, solver);

  if (! r_t.is_sat())
    return 0;
  if (! r_f.is_sat())
    return 1;
  return 2;
}

int is_reducible_bv_width1(const smt::Term & expr,
                           const smt::TermVec & assumptions,
                           const smt::SmtSolver & solver)
{
  smt::TermVec check_vec_true(assumptions);
  auto bv_sort = solver->make_sort(smt::BV, 1);
  smt::Term eq_expr_true =
      solver->make_term(smt::Equal, expr, solver->make_term(1, bv_sort));
  check_vec_true.push_back(eq_expr_true);
  // auto r_t = solver->check_sat_assuming(check_vec_true);
  auto r_t = is_sat_res(check_vec_true, solver);

  smt::TermVec check_vec_false(assumptions);
  smt::Term eq_expr_false =
      solver->make_term(smt::Equal, expr, solver->make_term(0, bv_sort));
  check_vec_false.push_back(eq_expr_false);
  // auto r_f = solver->check_sat_assuming(check_vec_false);
  auto r_f = is_sat_res(check_vec_false, solver);
  if (! r_t.is_sat())
    return 0;
  if (! r_f.is_sat())
    return 1;
  return 2;
} // end of is_reducible_bv_width1

smt::Term expr_simplify_ite(const smt::Term & expr,
                            const smt::TermVec & assumptions,
                            const smt::SmtSolver & solver)
{
  std::unordered_map<Term, int> cond_set; // deduplicate (make sure we visit the same condition only once)
  std::queue<smt::Term> que;
  smt::UnorderedTermSet visited;
  que.push(expr);
  auto T = solver->make_term(1);
  auto F = solver->make_term(0);
  auto bv1 = solver->make_term(1,solver->make_sort(smt::SortKind::BV, 1));
  auto bv0 = solver->make_term(0,solver->make_sort(smt::SortKind::BV, 1));

  smt::UnorderedTermMap subst_map;
  while (que.size() != 0) {
    auto node = que.front();
    que.pop();
    auto res = visited.emplace(node);
    if (!res.second) // if we have visited this node before, then skip
      continue;

    if (node->get_op() == smt::Ite) {
      auto childern = args(node);
      auto cond = childern.at(0);
      auto cond_set_pos = cond_set.find(cond);
      if (cond_set_pos == cond_set.end()) {
        auto reducible = is_reducible_bool(cond, assumptions, solver);
        cond_set.emplace(cond, reducible);
        if (reducible == 0) {
          subst_map[cond] = F;
          que.push(childern.at(2));
        } else if (reducible == 1) {
          subst_map[cond] = T;
          que.push(childern.at(1));
        } else {
          for (const auto & c : childern)
            que.push(c);
        } // end else not reducible
      } else { // end if not cached in cond_set
        // below is based on we have cached
        auto reducible = cond_set_pos->second;
        if(reducible == 0) {
          que.push(childern.at(2));
        } else if (reducible == 1) {
          que.push(childern.at(1));
        } else { // if we know it is not reducible, check its child
          assert(reducible == 2);
          for (const auto & c : childern)
            que.push(c);
        }
      } // end if not cached in cond_set
    } else if (node->get_sort()->get_sort_kind() == SortKind::BOOL || 
            (node->get_sort()->get_sort_kind() == SortKind::BV && 
             node->get_sort()->get_width() == 1 )) {
      auto reducible = is_reducible_bool(node, assumptions, solver);
      bool is_bool = node->get_sort()->get_sort_kind() == SortKind::BOOL;
      if (reducible == 0) {
        subst_map[node] = is_bool ? F : bv0;
      } else if (reducible == 1) {
        subst_map[node] = is_bool ? T : bv1;
      } else {
        for (const auto & c : node)
          que.push(c);
      }
    } else {
      auto children =  args(node);
      for (const auto & c : children)
        que.push(c);
    }
  } // end of traversal of AST
  return replacement_and_constant_propagation(expr, subst_map, solver);
} // end of expr_simplify_ite


void state_simplify_xvar(StateAsmpt & s,
                         const smt::UnorderedTermSet & set_of_xvar,
                         const smt::SmtSolver & solver)
{
  smt::UnorderedTermSet free_vars;
  for (const auto & sv : s.get_sv()) {
    const auto & expr = sv.second;
    smt::get_free_symbols(expr, free_vars);
  }

  smt::UnorderedTermMap xvar_sub;
  get_xvar_sub(s.get_assumptions(), set_of_xvar, free_vars, solver, xvar_sub);
  smt::UnorderedTermMap sv_to_replace; // try not to change s.sv_ while traversing

  for (const auto & sv : s.get_sv()) {
    const auto & var = sv.first;
    const auto & expr = sv.second;
    auto expr_new = solver->substitute(expr, xvar_sub);
    auto expr_final = expr_simplify_ite(expr_new, s.get_assumptions(), solver);
    sv_to_replace.emplace(var, expr_final);
  }
  (s.update_sv()).swap(sv_to_replace); // constant time operation
}


smt::Term replacement_and_constant_propagation(const smt::Term & expr,
                                               const smt::UnorderedTermMap submap,
                                               const smt::SmtSolver & solver) {
  
  // std::cout << "[DEBUG] replacement_and_constant_propagation\n";
  // for (const auto & p : submap)
  //   std::cout << "[DEBUG] " << p.first->to_string() << " --> " << p.second->to_string() << "\n";
  // std::cout << "[DEBUG] in: " << expr->to_string() << "\n";

  auto T = solver->make_term(1);
  auto F = solver->make_term(0);
  auto bv1 = solver->make_term(1,solver->make_sort(smt::SortKind::BV, 1));
  auto bv0 = solver->make_term(0,solver->make_sort(smt::SortKind::BV, 1));

  // cache starts with the substitutions
  UnorderedTermMap cache(submap);
  std::vector<std::pair<smt::Term, bool> > to_visit;
  to_visit.push_back({ expr, false });
  
  while (to_visit.size())
  {
    auto & [t, visited] = to_visit.back();
    if (cache.find(t) != cache.end()) {
      to_visit.pop_back();
      continue;
    } // else if not found
    if (!visited) {
      if (t->is_value()) {
        auto pos = submap.find(t);
        if (pos == submap.end())
          cache[t] = t;
        else
          cache[t] = pos->second;
        visited = true;
        continue;
      }

      for (auto c : t)
        to_visit.push_back({c,false});
      visited = true;
      continue;
    } // else if found
    
    TermVec cached_children;
    for (auto c : t)
      cached_children.push_back(cache.at(c));

    if (cached_children.size() && !t->is_value())
    {
      auto op = t->get_op();
      if (op.prim_op == smt::Implies) {
        assert(cached_children.size() == 2);
        auto ante  = cached_children.at(0);
        auto consq = cached_children.at(1);
        if (ante->is_value()) {
          // true -> x : x
          // false -> x : true
          auto val = ante->to_string();
          if (val == "#b0" || val == "false" || val == "(_ bv0 1)")
            cache[t] = T;
          else
            cache[t] = consq;
          continue;
        } else if (consq->is_value()) {
          // x -> false : not(x) // we know that x is not a value at this point
          // x -> true : true
          auto val = consq->to_string();
          if (val == "#b0" || val == "false" || val == "(_ bv0 1)")
            cache[t] = solver->make_term(smt::Not, ante);
          else // true
            cache[t] = T;
          continue;
        }
      } else if (op.prim_op == smt::And)  {
        // check if any child of And is 0 or 1
        assert(cached_children.size() >= 2);

        bool is_zero = false;
        for (auto it = cached_children.begin(); it != cached_children.end(); ) {
          auto c = *it;
          if (c->is_value()) {
            auto val = c->to_string();
            if (val == "#b0" || val == "false" || val == "(_ bv0 1)") {
              cache[t] = c; // And (xxx , 0, xx) -> 0
              is_zero = true;
              break;
            } else if (val == "#b1" || val == "true" || val == "(_ bv1 1)") {
              it = cached_children.erase(it);
            } else { // if AND can also be used for multibit (maybe in some solver?)
              ++it;
            }
          } else {
            // try to find if there are any duplications...  e.g.  And(a,a) --> a
            if (std::find(cached_children.begin(), it, *it) != it)
              it = cached_children.erase(it); // if found, remove this one
            else
              ++it;
          }
        } // end of for each child
        if (is_zero) // no need to set cache[t] again
          continue;
        if (cached_children.size() == 0) {
          cache[t] = T;
          continue;
        } else if (cached_children.size() == 1) {
          cache[t] = cached_children.at(0);
          continue;
        } // else
        // if its size is >= 2 then invoke  make_term and set cache[t] again
      } else if (op.prim_op == smt::Or) {
        // check if any child of And is 0 or 1
        assert(cached_children.size() >= 2);

        bool is_one = false;
        for (auto it = cached_children.begin(); it != cached_children.end(); ) {
          auto c = *it;
          if (c->is_value()) {
            auto val = c->to_string();
            if (val == "#b1" || val == "true" || val == "(_ bv1 1)") {
              cache[t] = c; // Or (xxx , 1, xx) -> 1
              is_one = true;
              break;
            } else if ( val == "#b0" || val == "false" || val == "(_ bv0 1)" ) {
              it = cached_children.erase(it);
            } else { // if OR can also be used for multibit (maybe in some solver?)
              ++it;
            }
          } else {
            // try to find if there are any duplications...  e.g.  Or(a,a) --> a
            if (std::find(cached_children.begin(), it, *it) != it)
              it = cached_children.erase(it); // if found, remove this one
            else
              ++it;
          }
        } // end of for each child
        if (is_one) // no need to set cache[t] again
          continue;
        if (cached_children.size() == 0) {
          cache[t] = F;
          continue;
        } else if (cached_children.size() == 1) {
          cache[t] = cached_children.at(0);
          continue;
        } // else
        // if its size is >= 2 then invoke  make_term and set cache[t] again
      } else if (op.prim_op == smt::Not) {
        if (cached_children.at(0)->is_value()) {
          auto val = cached_children.at(0)->to_string();
          if (val == "#b1" || val == "true" || val == "(_ bv1 1)") // T -> F
            cache[t] = F;
          else     // F -> T
            cache[t] = T;
          continue;
        }
      } else if (op.prim_op == smt::BVAnd || op.prim_op == smt::BVNand) {
        bool neg = op.prim_op == smt::BVNand;
        if (t->get_sort()->get_width() == 1) {
          // check if any child of And is 0 or 1
          assert(cached_children.size() >= 2);

          bool is_zero = false;
          for (auto it = cached_children.begin(); it != cached_children.end(); ) {
            auto c = *it;
            if (c->is_value()) {
              auto val = c->to_string();
              if (val == "#b0" || val == "false" || val == "(_ bv0 1)") {
                is_zero = true;
                break;
              } else if ( val == "#b1" || val == "true" || val == "(_ bv1 1)" ) {
                it = cached_children.erase(it);
              } else { // if AND can also be used for multibit (maybe in some solver?)
                ++it;
              }
            } else {
              // try to find if there are any duplications...  e.g.  And(a,a) --> a
              if (std::find(cached_children.begin(), it, *it) != it)
                it = cached_children.erase(it); // if found, remove this one
              else
                ++it;
            }
          } // end of for each child
          if (is_zero) {// no need to set cache[t] again
            cache[t] = neg ? bv1 : bv0;
            continue;
          }
          if (cached_children.size() == 0) {
            cache[t] = neg ? bv0 : bv1;
            continue;
          } else if (cached_children.size() == 1) {
            cache[t] = neg ? (solver->make_term(smt::BVNot, cached_children.at(0)))
                        : cached_children.at(0);
            continue;
          } // else you need to do the rest
        } // end of singlebit
        // TODO: multi-bit
      } else if (op.prim_op == smt::BVOr || op.prim_op == smt::BVNor) {
        bool neg = op.prim_op == smt::BVNor;
        if (t->get_sort()->get_width() == 1) {
          // check if any child of And is 0 or 1
          assert(cached_children.size() >= 2);

          bool is_one = false;
          for (auto it = cached_children.begin(); it != cached_children.end(); ) {
            auto c = *it;
            if (c->is_value()) {
              auto val = c->to_string();
              if (val == "#b1" || val == "true" || val == "(_ bv1 1)") {
                is_one = true;
                break;
              } else if ( val == "#b0" || val == "false" || val == "(_ bv0 1)" ) {
                it = cached_children.erase(it);
              } else { // if AND can also be used for multibit (maybe in some solver?)
                ++it;
              }
            } else {
              // try to find if there are any duplications...  e.g.  And(a,a) --> a
              if (std::find(cached_children.begin(), it, *it) != it)
                it = cached_children.erase(it); // if found, remove this one
              else
                ++it;
            }
          } // end of for each child
          if (is_one) {// no need to set cache[t] again
            cache[t] = neg ? bv0 : bv1;
            continue;
          }
          if (cached_children.size() == 0) {
            cache[t] = neg ? bv1 : bv0;
            continue;
          } else if (cached_children.size() == 1) {
            cache[t] = neg ? (solver->make_term(smt::BVNot, cached_children.at(0)))
                        : cached_children.at(0);
            continue;
          } // else you need to do the rest
        } // end of singlebit
        // TODO: multi-bit            
      } else if (op.prim_op == smt::BVNot && cached_children.at(0)->is_value()) {
        if (t->get_sort()->get_width() == 1) {
          auto val = cached_children.at(0)->to_string();
          if (val == "#b1" || val == "true" || val == "(_ bv1 1)") // T -> F
            cache[t] = bv0;
          else     // F -> T
            cache[t] = bv1;
          continue;
        } // TODO: multibits
      } else if (op.prim_op == smt::Ite) {
        if (cached_children.at(0)->is_value()) {
          auto cond = cached_children.at(0)->to_string(); 
          if (cond == "#b1" || cond == "true" || cond == "(_ bv1 1)")
            cache[t] = cached_children.at(1);
          else
            cache[t] = cached_children.at(2);
          continue;
        } else if (cached_children.at(1) == cached_children.at(2)) {
          cache[t] = cached_children.at(1);
          continue;
        } // for the rest will make_term as usual
      } 
      cache[t] = solver->make_term(op, cached_children);
    } else {
      cache[t] = t;
    }
  } // end of traversal while


  // std::cout << "[DEBUG] results in: " << cache.at(expr)->to_string() << "\n";

  return cache.at(expr);
} // end of replacement_and_constant_propagation


}  // namespace wasim