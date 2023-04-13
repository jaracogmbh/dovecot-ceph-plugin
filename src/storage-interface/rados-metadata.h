// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c)  = 0;2017-2018 Tallence AG and the authors
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */

#ifndef SRC_STORAGE_INTERFACE_RADOS_METADATA_H_
#define SRC_STORAGE_INTERFACE_RADOS_METADATA_H_

#include "../librmb/rados-types.h"
#include <string>
#include "time.h"
#include <stdlib.h>
#include <sstream>
#include <rados/librados.hpp>

namespace storage_interface {

class RadosMetadata {
 public:
  virtual ~RadosMetadata(){}

  virtual void convert(const char* value, time_t* t) = 0;
  virtual bool from_string(const std::string& str) = 0;
  virtual std::string to_string() = 0;
  virtual void convert(librmb::rbox_metadata_key _key, const std::string& val) = 0;
  virtual void convert(librmb::rbox_metadata_key _key, const time_t& time) = 0;
  virtual void convert(librmb::rbox_metadata_key _key, char* value) = 0;
  virtual void convert(librmb::rbox_metadata_key _key, const uint& value) = 0;
  virtual void convert(librmb::rbox_metadata_key _key, const size_t& value) = 0;
  virtual void convert(librmb::rbox_metadata_key _key, const int value) = 0;
  virtual ceph::bufferlist& get_buffer() = 0;
  virtual std::string& get_key() = 0;
  virtual void set_buffer(ceph::bufferlist& bl_) = 0;
  virtual void set_key(std::string& key_) = 0;
};
}  // end namespace
#endif /* SRC_STORAGE_INTERFACE_RADOS_METADATA_H_ */
