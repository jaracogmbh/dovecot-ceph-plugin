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
#ifndef SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_
#define SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_

#include <map>
#include <string>
#include "../storage-interface/rados-storage.h"
#include "../storage-interface/rados-dovecot-ceph-cfg.h"
#include "../storage-interface/rados-guid-generator.h"
#include"../storage-interface/rados-namespace-manager.h"

namespace librmb {

/**
 * Rados Namespace Manager
 *
 * Get Users Namespace based on configuration
 *
 */
class RadosNamespaceManagerImpl :public storage_interface::RadosNamespaceManager {
 public:
  /*!
   * @param[in] config_ valid radosDovecotCephCfg.
   */
  explicit RadosNamespaceManagerImpl(storage_interface::RadosDovecotCephCfg *config_) : oid_suffix("_namespace"), config(config_) {}
  virtual ~RadosNamespaceManagerImpl();
  void set_config(storage_interface::RadosDovecotCephCfg *config_) override { config = config_; }
  storage_interface::RadosDovecotCephCfg *get_config() override { return config; }

  void set_namespace_oid(std::string &namespace_oid_) override{ this->oid_suffix = namespace_oid_; }
  bool lookup_key(const std::string &uid, std::string *value) override;
  bool add_namespace_entry(const std::string &uid, std::string *value, storage_interface::RadosGuidGenerator *guid_generator_) override;

 private:
  std::map<std::string, std::string> cache;
  std::string oid_suffix;
  storage_interface::RadosDovecotCephCfg *config;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_ */
