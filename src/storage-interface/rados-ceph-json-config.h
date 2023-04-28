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

#ifndef SRC_STORAGE_INTERFACES_RADOS_CEPH_JSON_CONFIG_H_
#define SRC_STORAGE_INTERFACES_RADOS_CEPH_JSON_CONFIG_H_

#include <string>

#include <rados/librados.hpp>
#include "rados-types.h"

namespace storage_interface {
/**
 * Rados CephJsonConfig
 *
 * Dovecot-Ceph-plugin configuration
 * Ceph Configuration is json object, class
 * provides access to json object / Serialize / Deserialize
 *
 */
class RadosCephJsonConfig {
 public:
  virtual ~RadosCephJsonConfig() {}

  virtual bool from_json(librados::bufferlist* buffer) = 0;
  virtual bool to_json(librados::bufferlist* buffer) = 0;
  virtual std::string to_string() = 0;

  virtual const std::string& get_cfg_object_name() const = 0;

  virtual void set_cfg_object_name(const std::string& cfgObjectName) = 0;

  virtual const std::string& get_user_mapping() const = 0;

  virtual void set_user_mapping(const std::string& user_mapping_) = 0; 

  virtual bool is_valid() const = 0; 

  virtual void set_valid(bool isValid) = 0; 

  virtual std::string& get_user_ns() = 0; 

  virtual void set_user_ns(const std::string& user_ns_) = 0; 

  virtual std::string& get_user_suffix() = 0; 

  virtual void set_user_suffix(const std::string& user_suffix_) = 0; 

  virtual const std::string& get_public_namespace() const = 0; 

  virtual void set_public_namespace(const std::string& public_namespace_) = 0; 

  virtual void set_mail_attributes(const std::string& mail_attributes_) = 0; 
  virtual void set_update_attributes(const std::string& update_attributes_) = 0; 
  virtual void set_updateable_attributes(const std::string& updateable_attributes_) = 0; 

  virtual bool is_mail_attribute(enum rbox_metadata_key key) = 0;
  virtual bool is_updateable_attribute(enum rbox_metadata_key key) = 0;
  virtual bool is_update_attributes() = 0; 

  virtual void set_metadata_storage_module(const std::string& metadata_storage_module_) = 0; 
  virtual const std::string& get_metadata_storage_module() = 0;
  virtual void set_metadata_storage_attribute(const std::string& metadata_storage_attribute_) = 0; 
  virtual const std::string& get_metadata_storage_attribute() = 0; 

  virtual void update_mail_attribute(const char* value) = 0;
  virtual void update_updateable_attribute(const char* value) = 0;

  virtual const std::string& get_key_user_mapping() const = 0; 
  virtual const std::string& get_key_ns_cfg() const = 0;
  virtual const std::string& get_key_ns_suffix() const = 0; 
  virtual const std::string& get_key_public_namespace() const = 0;

  virtual const std::string& get_mail_attribute_key() = 0; 
  virtual const std::string& get_updateable_attribute_key() = 0; 
  virtual const std::string& get_update_attributes_key() = 0; 

  virtual const std::string& get_metadata_storage_module_key() = 0; 
  virtual const std::string& get_metadata_storage_attribute_key() = 0;
};

} /* namespace storage_interface */

#endif  // SRC_STORAGE_INTERFACES_RADOS_CEPH_JSON_CONFIG_H_
