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
#include "rados-namespace-manager-impl.h"


#include <rados/librados.hpp>

namespace librmb {

RadosNamespaceManagerImpl::~RadosNamespaceManagerImpl() {}

bool RadosNamespaceManagerImpl::lookup_key(const std::string &uid, std::string *value) {
  if (uid.empty()) {
    *value = uid;
    return true;
  }

  if (config == nullptr) {
    return false;
  }

  if (!config->is_config_valid()) {
    return false;
  }

  if (!config->is_user_mapping()) {
    *value = uid;
    return true;
  }

  if (cache.find(uid) != cache.end()) {
    *value = cache[uid];
    return true;
  }

 
  std::ostringstream stream_buffer;
  bool retval = false;

  // temporarily set storage namespace to config namespace
  config->set_io_ctx_namespace(config->get_user_ns());
  // storage->set_namespace(config->get_user_ns());
  int err = config->read_object(uid,stream_buffer);
  if (err >= 0 && !stream_buffer.str().empty()) {
    *value = stream_buffer.str();
    cache[uid] = *value;
    retval = true;
  }
  // reset namespace to empty
  config->set_io_ctx_namespace("");
  return retval;
}

bool RadosNamespaceManagerImpl::add_namespace_entry(const std::string &uid, std::string *value,
                                                storage_interface::RadosGuidGenerator *guid_generator_) {
  if (config == nullptr) {
    return false;
  }

  if (!config->is_config_valid()) {
    return false;
  }
  if (guid_generator_ == nullptr) {
    return false;
  }

  guid_generator_->generate_guid(value);
  // temporarily set storage namespace to config namespace
  config->set_io_ctx_namespace(config->get_user_ns());
  bool retval = false;
  
  std::istringstream i_value(*value);
  if (config->save_object(uid,i_value) >= 0) {
    cache[uid] = i_value.str();
    retval = true;
  }
  // reset namespace
  config->set_io_ctx_namespace("");
  delete guid_generator_;
  guid_generator_ = nullptr;
  return retval;
}

} /* namespace librmb */
