#include "timed_assertion_checker.h"

namespace tac {

verilog_expr::VExprAst::VExprAstPtr update_target_width(verilog_expr::VExprAst::VExprAstPtr& ast, wasim::SymbolicSimulator& sim)
{
    if (!ast) {
        throw std::invalid_argument("Null AST node");
    }

    std::stack<verilog_expr::VExprAst::VExprAstPtr> node_stack;
    node_stack.push(ast);

    while (!node_stack.empty()) {
        auto current_node = node_stack.top();
        node_stack.pop();
            switch (current_node->get_op()) {
                // root or node
                case verilog_expr::voperator::L_EQ: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                case verilog_expr::voperator::TERNARY: {
                  break;
                }
                // node
                case verilog_expr::voperator::PLUS: {
                  auto current_node_target_width = current_node -> get_target_width();
                  current_node -> get_child().at(0) -> set_target_width(current_node_target_width);
                  current_node -> get_child().at(1) -> set_target_width(current_node_target_width);
                  break;
                }
                case verilog_expr::voperator::MINUS: {
                  auto current_node_target_width = current_node -> get_target_width();
                  current_node -> get_child().at(0) -> set_target_width(current_node_target_width);
                  current_node -> get_child().at(1) -> set_target_width(current_node_target_width);
                  break;
                }
                case verilog_expr::voperator::STAR: {
                  auto current_node_target_width = current_node -> get_target_width();
                  current_node -> get_child().at(0) -> set_target_width(current_node_target_width);
                  current_node -> get_child().at(1) -> set_target_width(current_node_target_width);
                  break;
                }
                case verilog_expr::voperator::DIV: {
                  auto current_node_target_width = current_node -> get_target_width();
                  current_node -> get_child().at(0) -> set_target_width(current_node_target_width);
                  current_node -> get_child().at(1) -> set_target_width(current_node_target_width);
                  break;
                }
                case verilog_expr::voperator::LSR:
                case verilog_expr::voperator::LSL: {
                  break;
                }
                case verilog_expr::voperator::MOD: {
                  auto current_node_target_width = current_node -> get_target_width();
                  current_node -> get_child().at(0) -> set_target_width(current_node_target_width);
                  current_node -> get_child().at(1) -> set_target_width(current_node_target_width);
                  break;
                }
                case verilog_expr::voperator::GTE: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                case verilog_expr::voperator::LTE: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                case verilog_expr::voperator::GT: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                case verilog_expr::voperator::LT: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                // !
                case verilog_expr::voperator::L_NEG: {
                  break;
                }
                // &&
                case verilog_expr::voperator::L_AND: {
                  break;
                }
                // ||
                case verilog_expr::voperator::L_OR: {
                  break;
                }
                // !=
                case verilog_expr::voperator::L_NEQ: {
                  auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                  current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                  current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  break;
                }
                // ~
                case verilog_expr::voperator::B_NEG: {
                  break;
                }
                // &
                case verilog_expr::voperator::B_AND: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                    current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                    current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  }
                  break;
                }
                // |
                case verilog_expr::voperator::B_OR: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                    current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                    current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  }
                  break;
                }
                // ^
                case verilog_expr::voperator::B_XOR: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node_max_width = std::max(current_node -> get_child().at(0) -> get_node_width(), current_node -> get_child().at(1) -> get_node_width());
                    current_node -> get_child().at(0) -> set_target_width(child_node_max_width);
                    current_node -> get_child().at(1) -> set_target_width(child_node_max_width);
                  }
                  break;
                }
                
                // variable leaf
                case verilog_expr::voperator::MK_VAR: {
                  break;
                }
                case verilog_expr::voperator::MK_CONST: {
                  break;
                }
                case verilog_expr::voperator::AT: {
                  break;
                }
                case verilog_expr::voperator::INDEX: {
                  break;
                }
                case verilog_expr::voperator::RANGE_INDEX: {
                  break;
                }
                case verilog_expr::voperator::IDX_PRT_SEL_PLUS: {
                  break;
                }
                case verilog_expr::voperator::IDX_PRT_SEL_MINUS: {
                  break;
                }
                case verilog_expr::voperator::CONCAT: {
                  break;
                }
                default: {
                  std::cout << "Undefined voperator error!" << current_node << std::endl;
                  break;
                }
            } //switch

            for (size_t i = 0; i < current_node -> get_child_cnt(); ++i) {
                node_stack.push(current_node -> get_child().at(i));
            }

    } //while
    std::cout << "AST update target width done" << std::endl;
    return ast;
}

verilog_expr::VExprAst::VExprAstPtr set_node_width(verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicSimulator& sim)
{
    if (!node) {
        throw std::invalid_argument("Null AST node");
    }

    std::stack<std::pair<verilog_expr::VExprAst::VExprAstPtr, int>> node_stack;
    
    node_stack.push({node, 0}); 

    // post-order traversal
    while (!node_stack.empty()) {
        auto [current_node, child_index] = node_stack.top();

        if (child_index < current_node->get_child_cnt()) {
            node_stack.top().second++;
            node_stack.push({current_node->get_child().at(child_index), 0});
        } 
        else {
            node_stack.pop(); 
            switch (current_node->get_op()) {
                case verilog_expr::voperator::MK_VAR: {
                  auto var_sort = sim.var(current_node -> to_verilog()) -> get_sort();
                  auto var_sort_width = var_sort -> get_width();
                  current_node -> set_node_width(var_sort_width);
                  current_node -> set_target_width(var_sort_width);
                  break;
                }
                case verilog_expr::voperator::MK_CONST: {
                  auto current_const_node = verilog_expr::VExprAstConstant::cast_ptr(current_node);
                  auto const_tuple = current_const_node -> get_constant();
                  auto const_width = std::get<1>(const_tuple);
                  current_node -> set_node_width(const_width);
                  current_node -> set_target_width(const_width);
                  break;
                }
                // *
                case verilog_expr::voperator::STAR: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                  current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  break;
                }
                // +
                case verilog_expr::voperator::PLUS: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                  current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  break;
                }
                // -
                case verilog_expr::voperator::MINUS: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                  current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  break;
                }
                // /
                case verilog_expr::voperator::DIV: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                  current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  break;
                }
                // LSL
                case verilog_expr::voperator::LSL: {
                  auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                  current_node -> set_node_width(child_node_width);
                  current_node -> set_target_width(child_node_width);
                  break;
                }
                // LSR
                case verilog_expr::voperator::LSR: {
                  auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                  current_node -> set_node_width(child_node_width);
                  current_node -> set_target_width(child_node_width);
                  break;
                }
                // %
                case verilog_expr::voperator::MOD: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                  current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  break;
                }
                // >=
                case verilog_expr::voperator::GTE: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // <=
                case verilog_expr::voperator::LTE: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // >
                case verilog_expr::voperator::GT: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // <
                case verilog_expr::voperator::LT: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // !
                case verilog_expr::voperator::L_NEG: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // &&
                case verilog_expr::voperator::L_AND: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // ||
                case verilog_expr::voperator::L_OR: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // ==
                case verilog_expr::voperator::L_EQ: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // !=
                case verilog_expr::voperator::L_NEQ: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // ~
                case verilog_expr::voperator::B_NEG: {
                  auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                  current_node -> set_node_width(child_node_width);
                  current_node -> set_target_width(child_node_width);
                  break;
                }
                // &
                case verilog_expr::voperator::B_AND: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                    auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                    current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                    current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  }
                  else{
                    auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                    current_node -> set_node_width(child_node_width);
                    current_node -> set_target_width(child_node_width);
                  }
                  break;
                }
                // |
                case verilog_expr::voperator::B_OR: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                    auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                    current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                    current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  }
                  else{
                    auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                    current_node -> set_node_width(child_node_width);
                    current_node -> set_target_width(child_node_width);
                  }
                  break;
                }
                // ^
                case verilog_expr::voperator::B_XOR: {
                  if ((current_node->get_child_cnt()) == 2){
                    auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                    auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                    current_node -> set_node_width(std::max(child_node1_width, child_node2_width));
                    current_node -> set_target_width(std::max(child_node1_width, child_node2_width));
                  }
                  else{
                    auto child_node_width = current_node -> get_child().at(0) -> get_node_width();
                    current_node -> set_node_width(child_node_width);
                    current_node -> set_target_width(child_node_width);
                  }
                  break;
                }
                // [x]
                case verilog_expr::voperator::INDEX: {
                  current_node -> set_node_width(1);
                  current_node -> set_target_width(1);
                  break;
                }
                // [x+n:x]
                case verilog_expr::voperator::RANGE_INDEX: {
                  auto msb = std::stoi(current_node -> get_child().at(1) -> to_verilog());
                  auto lsb = std::stoi(current_node -> get_child().at(2) -> to_verilog());
                  current_node -> set_node_width(msb - lsb + 1);
                  current_node -> set_target_width(msb - lsb + 1);
                  break;
                }
                // [x+:n] <-> [x+n-1:x]
                case verilog_expr::voperator::IDX_PRT_SEL_PLUS: {
                  auto width = std::stoi(current_node -> get_child().at(2) -> to_verilog());
                  current_node -> set_node_width(width);
                  current_node -> set_target_width(width);
                  break;
                }
                // [x-:n] <-> [x:x-n+1]
                case verilog_expr::voperator::IDX_PRT_SEL_MINUS: {
                  auto width = std::stoi(current_node -> get_child().at(2) -> to_verilog());
                  current_node -> set_node_width(width);
                  current_node -> set_target_width(width);
                  break;
                }
                // @
                case verilog_expr::voperator::AT: {
                  auto width = current_node -> get_child().at(0) -> get_node_width();
                  current_node -> set_node_width(width);
                  current_node -> set_target_width(width);
                  break;
                }
                // : ?
                case verilog_expr::voperator::TERNARY: {
                  auto width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(width);
                  current_node -> set_target_width(width);
                  break;
                }
                // {}
                case verilog_expr::voperator::CONCAT: {
                  auto child_node1_width = current_node -> get_child().at(0) -> get_node_width();
                  auto child_node2_width = current_node -> get_child().at(1) -> get_node_width();
                  current_node -> set_node_width(child_node1_width + child_node2_width);
                  current_node -> set_target_width(child_node1_width + child_node2_width);
                  break;
                }
                // POW
                // ASL
                // ASR
                // C_EQ
                // C_NEQ
                // B_EQU
                // B_NAND
                // B_NOR
                // STORE_OP
                // FUNCTION_APP
                // REPEAT
                // DELAY
                // FORALL
                // EXIST
                default: {
                  std::cout << "Undefined voperator error!" << current_node << std::endl;
                  break;
                }
            }
            
        }
    }
    std::cout << "AST set node width done" << std::endl;
    return node;
}

smt::Term ast2term(smt::SmtSolver& solver, const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicSimulator& sim) {
    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    std::stack<std::pair<const verilog_expr::VExprAst::VExprAstPtr, int>> stack; 
    stack.push({node, 0});
    std::unordered_map<verilog_expr::VExprAst::VExprAstPtr, smt::Term> terms;

    while (!stack.empty()) {
        auto [current, child_index] = stack.top();
        
        if (child_index < current->get_child_cnt()) {
            stack.top().second++;
            stack.push({current->get_child().at(child_index), 0});
        } 
        else {
            stack.pop(); 

            switch (current->get_op()) {
                // == 
                case verilog_expr::voperator::L_EQ: {
                    if (current->get_child_cnt() != 2) {
                        throw std::runtime_error("Equal node requires two childs");
                    }
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto equal_term = solver -> make_term(smt::Equal, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), equal_term);
                    }
                    else {
                      terms[current] = solver->make_term(smt::Equal, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // != 
                case verilog_expr::voperator::L_NEQ: {
                    if (current->get_child_cnt() != 2) {
                        throw std::runtime_error("Equal node requires two childs");
                    }
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto equal_term = solver -> make_term(smt::Distinct, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), equal_term);
                    }
                    else {
                      terms[current] = solver->make_term(smt::Distinct, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // ? :
                case verilog_expr::voperator::TERNARY: {
                    if (current->get_child_cnt() != 3) {
                        throw std::runtime_error("TERNARY node requires three childs");
                    }
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto ternary_term = solver -> make_term(smt::Ite, terms[current->get_child().at(0)], terms[current->get_child().at(1)], terms[current->get_child().at(2)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), ternary_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::Ite, terms[current->get_child().at(0)], terms[current->get_child().at(1)], terms[current->get_child().at(2)]);
                    }
                    break;
                }
                // +
                case verilog_expr::voperator::PLUS: {
                    terms[current] = solver->make_term(smt::BVAdd, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // -
                case verilog_expr::voperator::MINUS: {
                    terms[current] = solver->make_term(smt::BVSub, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // *
                case verilog_expr::voperator::STAR: {
                    terms[current] = solver->make_term(smt::BVMul, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // /
                case verilog_expr::voperator::DIV: {
                    terms[current] = solver->make_term(smt::BVUdiv, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // %
                case verilog_expr::voperator::MOD: {
                    terms[current] = solver->make_term(smt::BVSmod, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // <<
                case verilog_expr::voperator::LSL: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto extarct_width = log2(current -> get_child().at(0) -> get_target_width());
                      smt::Op extract_op = smt::Op(smt::Extract,  extarct_width - 1,  0);
                      auto lsl_bit = solver -> make_term(extract_op, terms[current->get_child().at(1)]);
                      auto curr_term = solver -> make_term(smt::BVShl, terms[current->get_child().at(0)], lsl_bit);

                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      auto extarct_width = log2(current -> get_child().at(0) -> get_target_width());
                      smt::Op extract_op = smt::Op(smt::Extract,  extarct_width - 1,  0);
                      auto lsl_bit = solver -> make_term(extract_op, terms[current->get_child().at(1)]);
                      terms[current] = solver->make_term(smt::BVShl, terms[current->get_child().at(0)], lsl_bit);
                    }
                    break;
                }
                // >>
                case verilog_expr::voperator::LSR: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto extarct_width = log2(current -> get_child().at(0) -> get_target_width());
                      smt::Op extract_op = smt::Op(smt::Extract,  extarct_width - 1,  0);
                      auto lsr_bit = solver -> make_term(extract_op, terms[current->get_child().at(1)]);
                      auto curr_term = solver -> make_term(smt::BVLshr, terms[current->get_child().at(0)], lsr_bit);

                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      auto extarct_width = log2(current -> get_child().at(0) -> get_target_width());
                      smt::Op extract_op = smt::Op(smt::Extract,  extarct_width - 1,  0);
                      auto lsr_bit = solver -> make_term(extract_op, terms[current->get_child().at(1)]);
                      terms[current] = solver->make_term(smt::BVLshr, terms[current->get_child().at(0)], lsr_bit);
                    }
                    break;
                }
                // >=
                case verilog_expr::voperator::GTE: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::BVUge, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::BVUge, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // <=
                case verilog_expr::voperator::LTE: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::BVUle, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::BVUle, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // >
                case verilog_expr::voperator::GT: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::BVUgt, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::BVUgt, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // <
                case verilog_expr::voperator::LT: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::BVUlt, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::BVUlt, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // !
                case verilog_expr::voperator::L_NEG: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::Not, terms[current->get_child().at(0)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::Not, terms[current->get_child().at(0)]);
                    }
                    break;
                }
                // &&
                case verilog_expr::voperator::L_AND: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver -> make_term(smt::And, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::And, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // ||
                case verilog_expr::voperator::L_OR: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver->make_term(smt::Or, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::Or, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    }
                    break;
                }
                // &
                case verilog_expr::voperator::B_AND: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      smt::Term curr_term;
                      if ( (current -> get_child_cnt()) == 2){
                        curr_term = solver->make_term(smt::BVAnd, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        curr_term = solver->make_term(smt::BVAnd, terms[current->get_child().at(0)]);
                      }
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      if ( (current -> get_child_cnt()) == 2){
                        terms[current] = solver->make_term(smt::BVAnd, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        terms[current] = solver->make_term(smt::BVAnd, terms[current->get_child().at(0)]);
                      }
                    }
                    break;
                }
                // |
                case verilog_expr::voperator::B_OR: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      smt::Term curr_term;
                      if ( (current -> get_child_cnt()) == 2){
                        curr_term = solver->make_term(smt::BVOr, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        curr_term = solver->make_term(smt::BVOr, terms[current->get_child().at(0)]);
                      }
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      if ( (current -> get_child_cnt()) == 2){
                        terms[current] = solver->make_term(smt::BVOr, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        terms[current] = solver->make_term(smt::BVOr, terms[current->get_child().at(0)]);
                      }
                    }
                    break;
                }
                // ^
                case verilog_expr::voperator::B_XOR: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      smt::Term curr_term;
                      if ( (current -> get_child_cnt()) == 2){
                        curr_term = solver->make_term(smt::BVXor, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        curr_term = solver->make_term(smt::BVXor, terms[current->get_child().at(0)]);
                      }
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      if ( (current -> get_child_cnt()) == 2){
                        terms[current] = solver->make_term(smt::BVXor, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                      }
                      else{
                        terms[current] = solver->make_term(smt::BVXor, terms[current->get_child().at(0)]);
                      }
                    }
                    break;
                }
                // ~
                case verilog_expr::voperator::B_NEG: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      auto curr_term = solver->make_term(smt::BVNot, terms[current->get_child().at(0)]);
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), curr_term);
                    }
                    else{
                      terms[current] = solver->make_term(smt::BVNot, terms[current->get_child().at(0)]);
                    }
                    break;
                }
                // @
                case verilog_expr::voperator::AT: {
                    auto string_name = current->get_child().at(0)->to_verilog() + "@" + current->get_child().at(1)->to_verilog();
                    try{
                      solver -> get_symbol(string_name);
                    }
                    catch(const IncorrectUsageException &e){
                      terms[current] = solver->make_symbol(string_name, terms[current->get_child().at(0)] -> get_sort());
                    }

                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> get_symbol(string_name));
                    }
                    else{
                      terms[current] = solver -> get_symbol(string_name);
                    }
                    break;
                }
                // concat, extend width
                case verilog_expr::voperator::CONCAT: {
                    if( (current -> get_target_width()) != (current -> get_node_width()) ){
                      smt::Term left = terms[current->get_child().at(0)];
                      smt::Term right = terms[current->get_child().at(1)];
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> make_term(smt::Concat, left, right));
                    }
                    else{
                      smt::Term left = terms[current->get_child().at(0)];
                      smt::Term right = terms[current->get_child().at(1)];
                      terms[current] = solver -> make_term(smt::Concat, left, right);
                    }
                    break;
                }
                // index []
                case verilog_expr::voperator::INDEX: {
                    if( (current -> get_target_width()) != (current -> get_node_width()) ){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term right = terms[current->get_child().at(1)];
                        smt::Op extract_op = smt::Op(smt::Extract,  right -> to_int(),  right -> to_int());

                        auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                        terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> make_term(extract_op, left));
                        break;
                    }
                    else {
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term right = terms[current->get_child().at(1)];
                        smt::Op extract_op = smt::Op(smt::Extract,  right -> to_int(),  right -> to_int());
                        terms[current] = solver -> make_term(extract_op, left);
                    }
                    break;
                }
                // range index [msb:lsb]
                case verilog_expr::voperator::RANGE_INDEX: {
                    if( (current -> get_target_width()) != (current -> get_node_width()) ){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(),  right -> to_int());

                        auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                        terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> make_term(extract_op, left));
                        break;
                    }
                    else{
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(),  right -> to_int());
                        terms[current] = solver -> make_term(extract_op, left);
                    }
                    break;
                }
                // [x+:n]
                case verilog_expr::voperator::IDX_PRT_SEL_PLUS: {
                    if( (current -> get_target_width()) != (current -> get_node_width()) ){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  (mid -> to_int()) + (right -> to_int()) - 1,  mid -> to_int());

                        auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                        terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> make_term(extract_op, left));
                        break;
                    }
                    else{
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  (mid -> to_int()) + (right -> to_int()) - 1,  mid -> to_int());
                        terms[current] = solver -> make_term(extract_op, left);
                    }
                    break;
                }
                // [x-:n]
                case verilog_expr::voperator::IDX_PRT_SEL_MINUS: {
                    if( (current -> get_target_width()) != (current -> get_node_width()) ){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(), (mid -> to_int()) - (right -> to_int()) + 1);

                        auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                        terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), solver -> make_term(extract_op, left));
                        break;
                    }
                    else{
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(), (mid -> to_int()) - (right -> to_int()) + 1);
                        terms[current] = solver -> make_term(extract_op, left);
                    }
                    break;
                }
                // make var
                case verilog_expr::voperator::MK_VAR: {
                    if ( (current -> get_target_width()) != (current -> get_node_width()) ){
                      auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                      terms[current] = solver -> make_term(smt::Op(smt::Zero_Extend, concat_width), sim.var(current->to_verilog()));
                    }
                    else {
                      terms[current] = sim.var(current->to_verilog());
                    }
                    break;
                }
                // make const
                case verilog_expr::voperator::MK_CONST: {
                    auto current_const = verilog_expr::VExprAstConstant::cast_ptr(current);
                    auto const_value_tuple = current_const -> get_constant();

                    auto base = std::get<0>(const_value_tuple);
                    auto width = std::get<1>(const_value_tuple);
                    auto value = std::get<2>(const_value_tuple);

                    int const_value;

                    switch(base){
                      case 0:{
                        width = 32;
                        const_value = std::stoi(value);
                        break;
                      }
                      case 2:{
                        const_value = std::stoi(value, nullptr, 2);
                        break;
                      }
                      case 10:{
                        const_value = std::stoi(value, nullptr, 10);
                        break;
                      }
                      case 16:{
                        const_value = std::stoi(value, nullptr, 16);
                        break;
                      }
                      default:{
                        const_value = 0;
                        break;
                      }
                    }

                    auto concat_width = (current -> get_target_width()) - (current -> get_node_width());
                    terms[current] = solver->make_term(const_value, solver->make_sort(smt::BV, width + concat_width));
                    break;
                }
                // undefined operation
                default:
                    throw std::runtime_error("Unknown operation in AST");
            }
        }
    }
    return terms[node];
}

//class TimedAssertionChecker
void TimedAssertionChecker::parse_verilog(const std::string& verilog_assertion){
  verilog_assertion_ = verilog_assertion;
  //deps from vexpparser
  Vexp::Interpreter intp;
  std::stringstream ss;
  ss << verilog_assertion;

  intp.switchInputStream(&ss);
  try{
    intp.parse();
  } catch (verilog_expr::VexpException &e) {
    std::cout << "AST constructor error:" << e.msg_ << std::endl;
  }

  ast = intp.GetAstRoot();
  std::cout << "vexpparser ast : " << ast << std::endl;
}

void TimedAssertionChecker::parse_ast(const verilog_expr::VExprAst::VExprAstPtr& ast){
    
    Variable var_struct;

    if (!ast) {
      throw std::invalid_argument("Null AST node");
    }

    std::stack<verilog_expr::VExprAst::VExprAstPtr> node_stack;
    node_stack.push(ast);

    while (!node_stack.empty()) {
        auto node = node_stack.top();
        node_stack.pop();

        switch (node->get_op()) {
            case verilog_expr::voperator::AT: {
                std::string fullname = {node -> get_child().at(0) -> to_verilog() + "@" + node -> get_child().at(1) -> to_verilog()};
                std::string name = node -> get_child().at(0) -> to_verilog();
                int cycle = std::stoi(node -> get_child().at(1) -> to_verilog());
                auto var_sort = sim_.var(node -> get_child().at(0) -> to_verilog()) -> get_sort();

                var_struct = {fullname, name, cycle};
                var_struct_vec.push_back(var_struct);
                break;
            }
            default: {
                for (size_t i = 0; i < node -> get_child_cnt(); ++i) {
                    node_stack.push(node -> get_child().at(i));
                }
                break;
            }
        }
    }
}

void TimedAssertionChecker::parse_max_cycle()
{
    max_cycle = 0;
    for(auto &var_struct : var_struct_vec)
    {
      if(max_cycle < var_struct.cycle)
      {
        max_cycle = var_struct.cycle;
      }
    }
}

void TimedAssertionChecker::sim_max_step(const std::string &rst_sig_name, bool set_rst_to_0, const std::string & clk_sig_name)
{
    int sim_cycle;
    const auto& vars = get_var_vec();
    
    //get input var for make vdict
    smt::UnorderedTermSet input_term_set = ts_.inputvars();
    if (clk_sig_name != "") {
      smt::Term clk_term = sim_.var(clk_sig_name);
      input_term_set.erase(clk_term);
    }
    
    for (sim_cycle = 0; sim_cycle <= max_cycle; sim_cycle ++)
    {
        std::cout << "--------------------------------------@" << sim_cycle << std::endl;
        if(sim_cycle != 0)
        {
          sim_.sim_one_step();
        }
        
        
        //make vdict for each input sim.convert
        wasim::assignment_type input_vdict;
        for(auto &input_var : input_term_set){
          input_vdict[input_var -> to_string()] = input_var -> to_string() + std::to_string(sim_cycle);
        }

        if (rst_sig_name != "") {
          input_vdict[rst_sig_name] = set_rst_to_0 ? 0 : 1;
        }

        auto inputmap = sim_.convert(input_vdict); //input_vdict = {{"a", "a" + std::to_string(sim_cycle)}, {"b", "b" + std::to_string(sim_cycle)}}
        sim_.set_input(inputmap, {}); 

        auto s = sim_.get_curr_state();
        std::cout << s.print();  //print state value
        std::cout << s.print_assumptions(); //print state assumption

        int j;
        bool skip = false;
        for(j = 0; j < vars.size(); j++)
        { 
          if(vars[j].cycle == sim_cycle)
          { 
            //get term form input var
            for(const auto &input : inputmap) //  get the unod_map<term, term>, <term_name, term_symbolic_expression>
            {
              if( (input.first -> to_string()) == vars[j].name)
              {
                ass_termmap[vars[j].fullname] = input.second;
                skip = true;
                break;
              }
            }

            if(skip){
              skip = false;
              continue;
            }

            //get term from sim var
            auto out_term = sim_.var(vars[j].name);  //get var term name
            ass_termmap[vars[j].fullname] = sim_.interpret_state_expr_on_curr_frame(out_term);
          }
        }
    }
}

void TimedAssertionChecker::print_term_map(){
  std::cout << "-----------------we got the symbolic term----------------" << std::endl;
  std::cout << "---------------------------------------------------------" << std::endl;
  std::cout << "term <-> symbolic term" << std::endl;
  for(const auto &var_term : ass_termmap)
  {
    std::cout << var_term.first << " <-> " << var_term.second << std::endl;
  }
  std::cout << "---------------------------------------------------------" << std::endl;
}

smt::UnorderedTermMap TimedAssertionChecker::create_substitution_map(){
  smt::UnorderedTermMap substitution_map;
  for(auto &var_struct : var_struct_vec){
    smt::Term origin_term = solver_ -> get_symbol(var_struct.fullname);
    smt::Term sub_term = ass_termmap[var_struct.fullname];
    substitution_map[origin_term] = sub_term;
  }
  return substitution_map;
}

void TimedAssertionChecker::make_assertion_term(){
  std::cout << "vexpparser AST : " << ast << std::endl;
  auto my_ast = get_ast();

    //mark the node need to be extended
  set_node_width(my_ast, sim_);
  update_target_width(my_ast, sim_);
  
    //ast -> term
  assertion_term = ast2term(solver_, my_ast, sim_); //my_ast(set width extension)
  std::cout << "my assertion term (original term) :" << assertion_term << std::endl;

    //create substitution map
  auto substitution_map = create_substitution_map();

    //substitude term
  assertion_term_sub =  solver_ -> substitute(assertion_term, substitution_map);
  std::cout << "my assertion term (substitude symbolic term) :" << assertion_term_sub << std::endl;
}

void TimedAssertionChecker::assert_formula(){
    //assert
  smt::Term not_assertion_term = solver_ -> make_term(smt::PrimOp::Not, assertion_term_sub);
  solver_ -> assert_formula(not_assertion_term);

  std::cout << "assert formula (not my assertion term) done" << std::endl;
}

void TimedAssertionChecker::check_sat(){
    smt::Result res = solver_ -> check_sat();
    std::cout << "term check sat result : " << res << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    if( res.result == smt::SAT )
    {
      std::cout << "X |"<< " assert(" + verilog_assertion_ + ") | the equation isn't always correct and has counter-example" << std::endl;
    }
    else if(res.result == smt::UNSAT )
    {
      std::cout << "√ |" << " assert(" + verilog_assertion_ + ") | the equation is always correct" << std::endl;
    }
}



}   //namespace tac