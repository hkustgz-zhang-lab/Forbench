/*********************                                                        */
/*! \file
 ** \verbatim
 ** Top contributors (to current version):
 **   Makai Mann
 ** This file is part of the pono project.
 ** Copyright (c) 2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file LICENSE in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief
 **
 **
 **/

#include "logger.h"

std::string remove_curly_brackets(std::string s)
{
  std::size_t pos;
  while ((pos = s.find("{")) != std::string::npos) {
    s.replace(pos, 1, "");
  }
  while ((pos = s.find("}")) != std::string::npos) {
    s.replace(pos, 1, "");
  }
  return s;
}

// declare a global logger
namespace wasim {
Log logger;

void set_global_logger_verbosity(size_t v) { logger.set_verbosity(v); }

// -------------------------------------------------------------------

void SetLogLevel(const int& lvl) { FLAGS_minloglevel = lvl; }

void SetLogPath(const std::string& path) { FLAGS_log_dir = path; }

void SetToStdErr(const int& to_err) { FLAGS_logtostderr = to_err; }

// DebugLog

LogInitter::LogInitter() {
  google::InitGoogleLogging("forbench");
  FLAGS_minloglevel = 0; // log all message above level 0
#ifndef NDEBUG
  FLAGS_logtostderr = 1; // log INFO and WARNING to stderr
#else                    // NDEBUG
  FLAGS_logtostderr = 0; // not logging INFO and WARNING to stderr
#endif                   // NDEBUG
}

std::set<std::string> DebugLog::debug_tags_;

LogInitter DebugLog::init_;

void DebugLog::Enable(const std::string& tag) { debug_tags_.insert(tag); }

void DebugLog::Disable(const std::string& tag) { debug_tags_.erase(tag); }

void DebugLog::Clear() { debug_tags_.clear(); }

bool DebugLog::Find(const std::string& tag) {
  return (debug_tags_.find(tag) != debug_tags_.end());
}


}  // namespace wasim
