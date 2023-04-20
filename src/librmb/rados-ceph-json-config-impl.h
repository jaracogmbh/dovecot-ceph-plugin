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

#ifndef SRC_LIBRMB_RADOS_CEPH_JSON_CONFIG_IMPL_H_
#define SRC_LIBRMB_RADOS_CEPH_JSON_CONFIG_IMPL_H_

#include <string>

#include <rados/librados.hpp>
#include "../storage-interface/rados-types.h"
#include "../storage-interface/rados-ceph-json-config.h"

namespace librmb {
/**
 * Rados CephJsonConfig
 *
 * Dovecot-Ceph-plugin configuration
 * Ceph Configuration is json object, class
 * provides access to json object / Serialize / Deserialize
 *
 */
class RadosCephJsonConfigImpl : public storage_interface::RadosCephJsonConfig {
 public:
  RadosCephJsonConfigImpl();
  virtual ~RadosCephJsonConfigImpl() {}

  bool from_json(librados::bufferlist* buffer) override;
  bool to_json(librados::bufferlist* buffer) override;
  std::string to_string();

  const std::string& get_cfg_object_name() const override{ return cfg_object_name; }

  void set_cfg_object_name(const std::string& cfgObjectName) override{ cfg_object_name = cfgObjectName; }

  const std::string& get_user_mapping() const override{ return user_mapping; }

  void set_user_mapping(const std::string& user_mapping_) override{ user_mapping = user_mapping_; }

  bool is_valid() const override{ return valid; }

  void set_valid(bool isValid) override{ valid = isValid; }

  std::string& get_user_ns() override{ return user_ns; }

  void set_user_ns(const std::string& user_ns_) override{ user_ns = user_ns_; }

  std::string& get_user_suffix() override{ return user_suffix; }

  void set_user_suffix(const std::string& user_suffix_) override{ user_suffix = user_suffix_; }

  const std::string& get_public_namespace() const override{ return public_namespace; }

  void set_public_namespace(const std::string& public_namespace_) override{ public_namespace = public_namespace_; }

  void set_mail_attributes(const std::string& mail_attributes_) override{ mail_attributes = mail_attributes_; }
  void set_update_attributes(const std::string& update_attributes_) override{ update_attributes = update_attributes_; }
  void set_updateable_attributes(const std::string& updateable_attributes_) override{
    updateable_attributes = updateable_attributes_;
  }

  bool is_mail_attribute(storage_interface::rbox_metadata_key key) override;
  bool is_updateable_attribute(storage_interface::rbox_metadata_key key) override;
  bool is_update_attributes() override{ return update_attributes.compare("true") == 0; }

  void set_metadata_storage_module(const std::string& metadata_storage_module_) override{
    metadata_storage_module = metadata_storage_module_;
  }
  const std::string& get_metadata_storage_module() override{ return metadata_storage_module; }

  void set_metadata_storage_attribute(const std::string& metadata_storage_attribute_) override{
    metadata_storage_attribute = metadata_storage_attribute_;
  }
  const std::string& get_metadata_storage_attribute() override{ return metadata_storage_attribute; }

  void update_mail_attribute(const char* value) override;
  void update_updateable_attribute(const char* value) override;

  const std::string& get_key_user_mapping() const override{ return key_user_mapping; }
  const std::string& get_key_ns_cfg() const override{ return key_user_ns; }
  const std::string& get_key_ns_suffix() const override{ return key_user_suffix; }
  const std::string& get_key_public_namespace() const override{ return key_public_namespace; }

  const std::string& get_mail_attribute_key() override{ return key_mail_attributes; }
  const std::string& get_updateable_attribute_key() override{ return key_updateable_attributes; }
  const std::string& get_update_attributes_key() override{ return key_update_attributes; }

  const std::string& get_metadata_storage_module_key() override{ return key_metadata_storage_module; }
  const std::string& get_metadata_storage_attribute_key() override{ return key_metadata_storage_attribute; }

 private:
  void set_default_mail_attributes();
  void set_default_updateable_attributes();

 private:
  std::string cfg_object_name;
  bool valid;
  std::string user_mapping;
  std::string user_ns;
  std::string user_suffix;
  std::string public_namespace;

  std::string mail_attributes;
  std::string update_attributes;
  std::string updateable_attributes;

  std::string metadata_storage_module;
  std::string metadata_storage_attribute;

  std::string key_user_mapping;
  std::string key_user_ns;
  std::string key_user_suffix;
  std::string key_public_namespace;

  std::string key_mail_attributes;
  std::string key_update_attributes;
  std::string key_updateable_attributes;

  std::string key_metadata_storage_module;
  std::string key_metadata_storage_attribute;
};

} /* namespace librmb */

#endif  // SRC_LIBRMB_RADOS_CEPH_JSON_CONFIG_IMPL_H_
