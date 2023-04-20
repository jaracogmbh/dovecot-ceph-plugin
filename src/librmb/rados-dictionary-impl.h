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

#ifndef SRC_LIBRMB_RADOS_DICTIONARY_IMPL_H_
#define SRC_LIBRMB_RADOS_DICTIONARY_IMPL_H_

#include <list>
#include <string>
#include <cstdint>
#include <mutex>  // NOLINT
#include "../storage-interface/rbox-io-ctx.h"
#include <rados/librados.hpp>
#include "../storage-interface/rados-cluster.h"
#include "../storage-interface/rados-dictionary.h"
#include "../storage-interface/rados-dovecot-ceph-cfg.h"
#include "rados-dovecot-ceph-cfg-impl.h"
#include "../storage-interface/rados-namespace-manager.h"
#include "../storage-interface/rados-guid-generator.h"


namespace librmb {

class RadosDictionaryImpl : public storage_interface::RadosDictionary {
 public:
  RadosDictionaryImpl(storage_interface::RadosCluster* cluster, const std::string& poolname, const std::string& username,
                      const std::string& oid, storage_interface::RadosGuidGenerator* guid_generator_,
                      const std::string& cfg_object_name_);
  virtual ~RadosDictionaryImpl();

  const std::string get_full_oid(const std::string& key) override;
  const std::string get_shared_oid() override;
  const std::string get_private_oid() override;

  const std::string& get_oid() override { return oid; }
  const std::string& get_username() override { return username; }
  const std::string& get_poolname() override { return poolname; }

  storage_interface::RboxIoCtx* get_io_ctx_wrapper(const std::string& key) override;
  storage_interface::RboxIoCtx* get_shared_io_ctx_wrapper() override;
  storage_interface::RboxIoCtx* get_private_io_ctx_wrapper() override;

  void remove_completion(storage_interface::RboxIoCtx* remove_completion_wrapper) override;
  void push_back_completion(storage_interface::RboxIoCtx* push_back_completion_wrapper) override;
  void wait_for_completions() override;

  int get(const std::string& key, std::string* value_r) override;
    
  storage_interface::RboxIoCtx* RadosDictionary::remove_completion_wrapper;
  storage_interface::RboxIoCtx* RadosDictionary::push_back_completion_wrapper;

 private:
  bool load_configuration(librados::IoCtx* io_ctx);
  bool lookup_namespace(std::string& username_, storage_interface::RadosDovecotCephCfg* cfg_, std::string* ns);

 private:
  storage_interface::RadosCluster* cluster;
  std::string poolname;
  std::string username;
  std::string oid;

  std::string shared_oid;
  librados::IoCtx shared_io_ctx;
  storage_interface::RboxIoCtx* shared_io_ctx_wrapper;
  bool shared_io_ctx_created;

  std::string private_oid;
  librados::IoCtx private_io_ctx;
  storage_interface::RboxIoCtx* private_io_ctx_wrapper;
  bool private_io_ctx_created;

  std::list<librados::AioCompletion*> completions;
  std::mutex completions_mutex;

  storage_interface::RadosDovecotCephCfg* cfg;
  storage_interface::RadosNamespaceManager* namespace_mgr;

  storage_interface::RadosGuidGenerator* guid_generator;
  std::string cfg_object_name;
};

}  // namespace librmb

#endif  // SRC_LIBRMB_RADOS_DICTIONARY_IMPL_H_
