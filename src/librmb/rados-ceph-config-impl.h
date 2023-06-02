// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c) override2017-2018 Tallence AG and the authors
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */
#ifndef SRC_LIBRMB_RADOS_CEPH_CONFIG_H_
#define SRC_LIBRMB_RADOS_CEPH_CONFIG_H_

#include <string>
#include "../storage-interface/rados-ceph-json-config.h"
#include "rados-ceph-json-config-impl.h"
#include "../storage-interface/rados-types.h"
#include <rados/librados.hpp>
#include "../storage-interface/rados-storage.h"
#include "../storage-interface/rados-ceph-config.h"
namespace librmb {
/**
 * RadosCephConfig
 *
 * read Plugin configuration from rados storage
 */
class RadosCephConfigImpl : public storage_interface::RadosCephConfig{
 public:
  explicit RadosCephConfigImpl(storage_interface::RboxIoCtx *io_ctx_);
  RadosCephConfigImpl() { 
    io_ctx = nullptr;
    if(config==nullptr){
      config=new librmb::RadosCephJsonConfigImpl();
    }
  }
  virtual ~RadosCephConfigImpl() {
    if(config!=nullptr){
      delete config;
      config=nullptr;
    }
  }

  // load settings from rados cfg_object
  int load_cfg() override;
  int save_cfg() override;

  void set_io_ctx(storage_interface::RboxIoCtx *io_ctx_) override{ io_ctx = io_ctx_; }
  bool is_config_valid() override{ return config->is_valid(); }
  void set_config_valid(bool valid_) override{ config->set_valid(valid_); }
  bool is_user_mapping() override{ return !config->get_user_mapping().compare("true"); }
  void set_user_mapping(bool value_) override{ config->set_user_mapping(value_ ? "true" : "false"); }
  void set_user_ns(const std::string &ns_) override{ config->set_user_ns(ns_); }
  std::string &get_user_ns() override{ return config->get_user_ns(); }
  void set_user_suffix(const std::string &ns_suffix_) override{ config->set_user_suffix(ns_suffix_); }
  std::string &get_user_suffix() override{ return config->get_user_suffix(); }
  const std::string &get_public_namespace()const override { return config->get_public_namespace(); }
  void set_public_namespace(const std::string &public_namespace_) override{ config->set_public_namespace(public_namespace_); }

  void set_cfg_object_name(const std::string &cfg_object_name_) override{ config->set_cfg_object_name(cfg_object_name_); }
  std::string get_cfg_object_name() override{ return config->get_cfg_object_name(); }
  storage_interface::RadosCephJsonConfig *get_config() override{ return config; }

  bool is_valid_key_value(const std::string &key, const std::string &value) override;
  bool update_valid_key_value(const std::string &key, const std::string &value) override;
  // bool is_ceph_posix_bugfix_enabled() override;
  bool is_mail_attribute(storage_interface::rbox_metadata_key key) override{ return config->is_mail_attribute(key); }
  bool is_updateable_attribute(storage_interface::rbox_metadata_key key) override{ return config->is_updateable_attribute(key); }
  bool is_update_attributes() override{ return config->is_update_attributes(); }
  void set_update_attributes(const std::string &update_attributes_) override{
    config->set_update_attributes(update_attributes_);
  }
  
  void update_mail_attribute(const char *value) override{ config->update_mail_attribute(value); }
  void update_updateable_attribute(const char *value) override{ config->update_updateable_attribute(value); }

  const std::string &get_metadata_storage_module() override{ return config->get_metadata_storage_module(); }
  const std::string &get_metadata_storage_attribute() override{ return config->get_metadata_storage_attribute(); }

  const std::string &get_mail_attribute_key() override{ return config->get_mail_attribute_key(); }
  const std::string &get_updateable_attribute_key() override{ return config->get_updateable_attribute_key(); }
  const std::string &get_update_attributes_key() override{ return config->get_update_attributes_key(); }

  int save_object(const std::string &oid, void* buffer) override;
  int read_object(const std::string &oid, void* buffer) override;
  void set_io_ctx_namespace(const std::string &namespace_) override;

 private:
  storage_interface::RadosCephJsonConfig *config = nullptr;
  storage_interface::RboxIoCtx *io_ctx = nullptr;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_RADOS_CEPH_CONFIG_H_ */
