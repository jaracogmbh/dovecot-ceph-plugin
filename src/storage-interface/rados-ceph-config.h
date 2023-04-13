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
#ifndef SRC_STORAGE_INTERFACE_RADOS_CEPH_CONFIG_H_
#define SRC_STORAGE_INTERFACE_RADOS_CEPH_CONFIG_H_

#include <string>
#include "../librmb/rados-ceph-json-config.h"
#include "../librmb/rados-types.h"
#include <rados/librados.hpp>
#include "rados-storage.h"

namespace storage_interface {
/**
 * RadosCephConfig
 *
 * read Plugin configuration from rados storage
 */
class RadosCephConfig {
 public:
  virtual ~RadosCephConfig() {}

  // load settings from rados cfg_object
  virtual int load_cfg() = 0;
  virtual int save_cfg() = 0;

  virtual void set_io_ctx(librados::IoCtx *io_ctx_) = 0; 
  virtual bool is_config_valid() = 0;
  virtual void set_config_valid(bool valid_) = 0;
  virtual bool is_user_mapping() = 0;
  virtual void set_user_mapping(bool value_) = 0;
  virtual void set_user_ns(const std::string &ns_) = 0;
  virtual std::string &get_user_ns() = 0;
  virtual void set_user_suffix(const std::string &ns_suffix_) = 0;
  virtual std::string &get_user_suffix() = 0;
  virtual const std::string &get_public_namespace() const = 0;
  virtual void set_public_namespace(const std::string &public_namespace_) = 0;
  virtual void set_cfg_object_name(const std::string &cfg_object_name_) = 0;
  virtual std::string get_cfg_object_name() = 0;
  virtual librmb::RadosCephJsonConfig *get_config() = 0;

  virtual bool is_valid_key_value(const std::string &key, const std::string &value)  = 0;
  virtual bool update_valid_key_value(const std::string &key, const std::string &value) = 0;
  // virtual bool is_ceph_posix_bugfix_enabled() = 0;
  virtual bool is_mail_attribute(librmb::rbox_metadata_key key) = 0;
  virtual bool is_updateable_attribute(librmb::rbox_metadata_key key) = 0;
  virtual bool is_update_attributes() = 0;
  virtual void set_update_attributes(const std::string &update_attributes_) = 0;
  
  virtual void update_mail_attribute(const char *value) = 0;
  virtual void update_updateable_attribute(const char *value) = 0;

  virtual const std::string &get_metadata_storage_module() = 0;
  virtual const std::string &get_metadata_storage_attribute() = 0;

  virtual const std::string &get_mail_attribute_key() = 0;
  virtual const std::string &get_updateable_attribute_key() = 0;
  virtual const std::string &get_update_attributes_key()  = 0;
  virtual int save_object(const std::string &oid, librados::bufferlist &buffer) = 0;
  virtual int read_object(const std::string &oid, librados::bufferlist *buffer) = 0;
  virtual void set_io_ctx_namespace(const std::string &namespace_) = 0;
};

} /* namespace storage_interface */

#endif /*SRC_STORAGE_INTERFACE_RADOS_CEPH_CONFIG_H_*/
