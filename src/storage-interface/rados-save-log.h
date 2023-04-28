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

#ifndef SRC_STORAGE_INTERFACES_RADOS_SAVE_LOG_H_
#define SRC_STORAGE_INTERFACES_RADOS_SAVE_LOG_H_

#include <fstream> 
#include <cstdio>
#include <vector>
#include <sstream>
#include <list>
#include <iostream>

#include "rados-metadata.h"

namespace storage_interface {
/**
 * RadosSaveLogEntry
 *
 * Class provides access to the savelog.format.
 *
 */
class RadosSaveLogEntry {
 public:
  ~RadosSaveLogEntry(){};
  virtual bool parse_mv_op() = 0;
  virtual std::string op_save() = 0;
  virtual std::string op_cpy() = 0;
  virtual std::string op_mv(const std::string &src_ns, const std::string &src_oid, const std::string &src_user,
                           std::list<storage_interface::RadosMetadata *> &metadata) = 0;
  virtual std::string& get_oid() = 0;
  virtual std::string& get_ns() = 0;
  virtual std::string& get_pool() = 0;
  virtual std::string& get_op() = 0;
  virtual std::string& get_src_oid() = 0;
  virtual std::string& get_src_ns() = 0;
  virtual std::string& get_src_user() = 0;
  virtual std::list<storage_interface::RadosMetadata*>& get_metadata() = 0;
};

class RadosSaveLog {
 public:
  virtual ~RadosSaveLog(){};
  virtual void set_save_log_file(const std::string &logfile_) = 0;
  
  virtual bool open() = 0;
  virtual void append(const RadosSaveLogEntry *entry) = 0;
  virtual bool close() = 0;
  virtual bool is_open() = 0;
};

} /* namespace storage_interface */

#endif /*SRC_STORAGE_INTERFACES_RADOS_SAVE_LOG_H_ */
