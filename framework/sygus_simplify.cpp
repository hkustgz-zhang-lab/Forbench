

#include "term_manip.h"
#include "state_simplify.h"
#include "sygus_simplify.h"
#include "config/testpath.h"
#include "utils/exceptions.h"
#include "frontend/smt_in.h"

#include "smt-switch/smt.h"
#include "smt-switch/utils.h"
#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/cvc5_factory.h"

#include <time.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/process.hpp>
#include <boost/process/async.hpp>


namespace wasim {

using parsed_info = std::tuple<smt::UnorderedTermSet, // vars in expression
                               smt::UnorderedTermSet, // vars in assumption
                               smt::Term,             // conjunction of all assumptions
                               smt::Term,             // the expression to simplify
                               std::string>;          // sort string of the expression


namespace bp = boost::process;
class Process
{
 public:
  Process(const std::string & cmd, int timeout);
  void run();

 private:
  void timeout_handler(boost::system::error_code ec);

  const std::string command;
  const int timeout;

  bool killed = false;
  bool stopped = false;

  std::string stdOut;
  std::string stdErr;
  int returnStatus = 0;

  boost::asio::io_service ios;
  boost::process::group group;
  boost::asio::deadline_timer deadline_timer;
};

Process::Process(const std::string & cmd, const int timeout)
    : command(cmd), timeout(timeout), deadline_timer(ios)
{
}

void Process::timeout_handler(boost::system::error_code ec)
{
  if (stopped) return;

  if (ec == boost::asio::error::operation_aborted) return;

  if (deadline_timer.expires_at()
      <= boost::asio::deadline_timer::traits_type::now()) {
    // std::cout << "Time Up!" << std::endl;
    group.terminate();  // NOTE: anticipate errors
    // std::cout << "Killed the process and all its decendants" << std::endl;
    killed = true;
    stopped = true;
    deadline_timer.expires_at(boost::posix_time::pos_infin);
  }
  // NOTE: don't make it a loop
  // deadline_timer.async_wait(boost::bind(&Process::timeout_handler, this,
  // boost::asio::placeholders::error));
}

void Process::run()
{
  std::future<std::string> dataOut;
  std::future<std::string> dataErr;

  deadline_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
  deadline_timer.async_wait(boost::bind(
      &Process::timeout_handler, this, boost::asio::placeholders::error));

  bp::child c(command,
              bp::std_in.close(),
              bp::std_out > dataOut,
              bp::std_err > dataErr,
              ios,
              group,
              bp::on_exit([=](int e, std::error_code ec) {
                // TODO handle errors
                // std::cout << "on_exit: " << ec.message() << " -> " << e <<
                // std::endl;
                deadline_timer.cancel();
                returnStatus = e;
              }));

  ios.run();

  stdOut = dataOut.get();
  stdErr = dataErr.get();

  c.wait();

  returnStatus = c.exit_code();
}

void run_cmd(const std::string & cmd_string, int timeout)
{
  Process p(cmd_string, timeout);
  p.run();
}


static std::string GetTimeStamp()
{
  auto timeNow = chrono::duration_cast<chrono::milliseconds>(
      chrono::system_clock::now().time_since_epoch());
  long long timestamp = timeNow.count();
  return std::to_string(timestamp);
}

static parsed_info parse_state(const smt::TermVec & asmpt, const smt::Term & v, const smt::SmtSolver & solver)
{
  auto asmpt_and = solver->make_term(smt::And, asmpt);
  smt::UnorderedTermSet free_var_asmpt;
  smt::get_free_symbols(asmpt_and, free_var_asmpt);
  smt::UnorderedTermSet free_var;
  smt::get_free_symbols(v, free_var);
  auto Fun_type = v->get_sort()->to_string();
  return std::make_tuple(free_var, free_var_asmpt, asmpt_and, v, Fun_type);
}

smt::Term run_sygus(const parsed_info & info,
                    const smt::UnorderedTermSet & set_of_xvar,
                    const smt::SmtSolver & solver)
{
  const smt::UnorderedTermSet & free_var = std::get<0>(info);
  const smt::UnorderedTermSet & free_var_asmpt = std::get<1>(info);
  const smt::Term & asmpt_and = std::get<2>(info);
  const smt::Term & Fun = std::get<3>(info);
  const std::string & Fun_type = std::get<4>(info);
  auto time_stamp = GetTimeStamp();
  auto template_file = "sygus_template_" + time_stamp + ".sygus";
  auto result_file = "sygus_result_" + time_stamp + ".sygus";
  auto result_temp_file = "sygus_result_temp_" + time_stamp + ".sygus";
  auto bash_file = "sygus_" + time_stamp + ".sh";

  std::ofstream f;

  f.open(template_file.c_str(), ios::out | ios::app);
  auto line1 = "(set-logic BV)\n\n\n(synth-fun FunNew \n   (";
  f << line1 << endl;

  for (const auto & var : free_var) {
    if (set_of_xvar.find(var) == set_of_xvar.end()) {
      auto line2 = "    (" + var->to_string() + " "
                   + var->get_sort()->to_string() + " )";
      f << line2 << endl;
    }
  }

  auto line3 = "   )\n   " + Fun_type + "\n  )\n\n\n";
  f << line3 << endl;

  for (const auto & var : free_var_asmpt) {
    auto line4 = "(declare-var " + var->to_string() + " "
                 + var->get_sort()->to_string() + ")";
    f << line4 << endl;
  }

  for (const auto & var : free_var) {
    if (free_var_asmpt.find(var) == free_var_asmpt.end()) {
      auto line4 = "(declare-var " + var->to_string() + " "
                   + var->get_sort()->to_string() + ")";
      f << line4 << endl;
    }
  }

  auto line5 = "\n(constraint (=> ";
  f << line5 << endl;

  auto line6 = "    " + asmpt_and->to_string() + " ;\n\n    (=";
  f << line6 << endl;

  auto line7 = "        " + Fun->to_string() + " ;\n        (FunNew ";
  f << line7;

  for (const auto & var : free_var) {
    if (set_of_xvar.find(var) == set_of_xvar.end()) {
      auto line8 = var->to_string() + " ";
      f << line8;
    }
  }

  auto line9 = ") ;\n    )))\n\n\n;\n\n(check-synth)";
  f << line9 << endl;
  f.close();

  std::ofstream bash_f;

  bash_f.open(bash_file.c_str(), ios::out | ios::app);
  auto bash_line1 = "#!/bin/bash";
  bash_f << bash_line1 << endl;
  auto bash_line2 = (PROJECT_SOURCE_DIR "/deps/smt-switch/deps/cvc5/build/bin/cvc5 --lang=sygus2 ") 
                    + template_file
                    + " > " + result_temp_file;
  bash_f << bash_line2 << endl;
  bash_f.close();

  auto cmd = "chmod 755 " + bash_file;
  system(cmd.c_str());
  auto cmd_string = "./" + bash_file;
  run_cmd(cmd_string, 1000);  // 500 milliseconds -> 0.5s
  // timeout table
  // c1 ->- 100ms
  // c2 ->- 100ms
  // c3 ->- 1s

  // only move the second line of sygus output file to a new file
  std::ifstream infile(result_temp_file.c_str());
  std::string linedata = "";

  getline(infile, linedata);  // get first line, and do nothing
  getline(infile, linedata);  // get second line
  infile.close();

  std::ofstream outfile;
  outfile.open(result_file.c_str(), ios::out | ios::app);
  outfile << linedata << endl;
  outfile.close();

  smt::Term new_expr;
  // determine whether the smtlib_result is empty
  if (linedata.empty()) {
    try {
      new_expr = solver->make_symbol("no_file", solver->make_sort(smt::BV, 1));
    }
    catch (const std::exception & e) {
      new_expr = solver->get_symbol("no_file");
    }
  } else {
    auto solver_copy = solver; // TODO: in the future, change SmtLibReader to use const ref.
    WasimSmtLib2Parser pi(result_file, solver_copy);
    new_expr = pi.return_defs();
  }
  int rm;
  // rm = remove(template_file.c_str());
  rm = remove(result_file.c_str());
  rm = remove(result_temp_file.c_str());
  rm = remove(bash_file.c_str());

  return new_expr;
} // end of run_sygus


static smt::Term structure_simplify(
                             const smt::Term & v,
                             const smt::TermVec & assumptions,
                             const smt::UnorderedTermSet & set_of_xvar,
                             smt::TermTranslator & translator);

static smt::TermVec child_vec_simplify(
                                const smt::TermVec & child_vec,
                                const smt::TermVec & asmpt,
                                const smt::UnorderedTermSet & set_of_xvar,
                                smt::TermTranslator & translator)
{
  smt::TermVec child_new_vec;
  for (const auto & child : child_vec) {
    if (expr_contains_X(child, set_of_xvar))
      child_new_vec.push_back(
          structure_simplify(child, asmpt, set_of_xvar, translator));
    else 
      child_new_vec.push_back(child);
    
  }
  return child_new_vec;
}

// attempt to hierarchically simplify
static smt::Term structure_simplify(
                             const smt::Term & v,
                             const smt::TermVec & assumptions,
                             const smt::UnorderedTermSet & set_of_xvar,
                             smt::TermTranslator & translator)
{
  smt::Term new_expr;
  auto expr_info = parse_state(assumptions, v, translator.get_solver());
  auto new_expr_direct = run_sygus(expr_info, set_of_xvar, translator.get_solver());
  if (new_expr_direct->to_string() == "no_file") {
    auto child_vec = args(v);
    auto child_new_vec =
        child_vec_simplify(child_vec, assumptions, set_of_xvar, translator);

    const auto & solver_cvc5 = translator.get_solver();
    if ((v->get_op() == smt::Ite) && (child_vec.size() == 3)) {
      new_expr = solver_cvc5->make_term(smt::Ite, child_new_vec);
    } else if ((v->get_op() == smt::BVNot) && (child_vec.size() == 1)) {
      new_expr = solver_cvc5->make_term(smt::BVNot, child_new_vec);
    } else if ((v->get_op() == smt::Not) && (child_vec.size() == 1)) {
      new_expr = solver_cvc5->make_term(smt::Not, child_new_vec);
    } else if ((v->get_op() == smt::BVAnd) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::BVAnd, child_new_vec);
    } else if ((v->get_op() == smt::And) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::And, child_new_vec);
    } else if ((v->get_op() == smt::BVMul) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::BVMul, child_new_vec);
    } else if ((v->get_op() == smt::BVAdd) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::BVAdd, child_new_vec);
    } else if ((v->get_op() == smt::Concat) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::Concat, child_new_vec);
    } else if ((v->get_op() == smt::Equal) && (child_vec.size() == 2)) {
      new_expr = solver_cvc5->make_term(smt::Equal, child_new_vec);
    }
    // else if
    else {
      throw SimulatorException("new structure of " + v->to_string() + " is not handled");
    }
  } else {
    new_expr = new_expr_direct;
  }

  return new_expr;
} // end of structure_simplify

void sygus_simplify(StateAsmpt & state_btor,
                    const smt::UnorderedTermSet & set_of_xvar_btor,
                    smt::SmtSolver & solver)
{
  smt::SmtSolver solver_cvc5 = smt::Cvc5SolverFactory::create(false);
  solver_cvc5->set_logic("QF_BV");
  solver_cvc5->set_opt("produce-models", "true");
  solver_cvc5->set_opt("incremental", "true");
  smt::TermTranslator btor2cvc(solver_cvc5);
  smt::TermTranslator cvc2btor(solver);

  smt::UnorderedTermSet set_of_xvar_in_cvc;
  smt::TermVec assmpt_in_cvc;

  for (const auto & x : set_of_xvar_btor)
    set_of_xvar_in_cvc.emplace(btor2cvc.transfer_term(x));
  for (const auto & a : state_btor.get_assumptions())
    assmpt_in_cvc.push_back(btor2cvc.transfer_term(a));


  for (const auto & sv : state_btor.get_sv()) {
    const auto & s_btor = sv.first;
    const auto & v_btor = sv.second;

    if (expr_contains_X(v_btor, set_of_xvar_btor)) {
      auto v_cvc = btor2cvc.transfer_term(v_btor);
      auto new_expr =
          structure_simplify(v_cvc, assmpt_in_cvc, set_of_xvar_in_cvc, btor2cvc);
      // cout << "new_expr: " << new_expr->to_string() << endl;
      auto new_expr_btor = cvc2btor.transfer_term(new_expr);
      state_btor.update_sv().insert_or_assign(s_btor, new_expr_btor);
    } // end of if contains X
  } // end of for each sv
} // end of sygus_simplify

}  // namespace wasim
