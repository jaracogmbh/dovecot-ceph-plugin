// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c) 2017-2018 Tallence AG and the authors
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */

#ifndef SRC_INTERFACES_TOOLS_RMB_LS_CMD_PARSER_H_
#define SRC_INTERFACES_TOOLS_RMB_LS_CMD_PARSER_H_

#include <string>
#include <list>
#include <iostream>
#include <ctime>
#include <map>

#include "../../rados-mail.h"
#include "../../rados-util.h"

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

namespace storage_interface {

class Predicate{
 public:
  virtual ~Predicate(){}
  virtual std::string& get_key() = 0;
  virtual std::string& get_op() = 0;
  virtual std::string& get_value() = 0;  
  virtual bool& get_valid() = 0;

  virtual bool eval(const std::string &_p_value) = 0; 
  virtual bool convert_str_to_time_t(const std::string &date, time_t *val) = 0;
  virtual int convert_time_t_to_str(const time_t &t, std::string *ret_val) = 0;
};

class CmdLineParser {
 public:
  virtual ~CmdLineParser(){}
  virtual bool parse_ls_string() = 0;
  virtual std::map<std::string, Predicate *> &get_predicates() = 0;

  virtual bool contains_key(const std::string &key) = 0;
  virtual Predicate *get_predicate(const std::string &key) = 0;
  virtual Predicate *create_predicate(const std::string &ls_value) = 0;

  virtual void set_output_dir(const std::string &out) = 0;
  virtual std::string &get_output_dir() = 0;
};
};      // namespace storage_interface
#endif  // SRC_INTERFACES_TOOLS_RMB_LS_CMD_PARSER_H_
