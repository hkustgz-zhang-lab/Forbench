#pragma once

#include "smt-switch/bitwuzla_factory.h"
#include "smt-switch/smt.h"
#include "smt-switch/smtlib_reader.h"

namespace wasim {

class WasimSmtLib2Parser : public smt::SmtLibReader
{
 public:
  WasimSmtLib2Parser(const std::string & filename, smt::SmtSolver & solver);

  typedef SmtLibReader super;

  smt::Term return_defs();

 protected:
  // overloaded function, used when arg list of function is parsed
  // NOTE: | |  pipe quotes are removed.
  virtual smt::Term register_arg(const std::string & name,
                                 const smt::Sort & sort) override;

  std::string filename_;
};

} // end of namespace wasim
