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
#include "rbox-io-ctx.h"
#include <rados/librados.hpp>
#include "rados-cluster.h"
#include "rados-dictionary.h"
#include "rados-dovecot-ceph-cfg.h"
#include "rados-dovecot-ceph-cfg-impl.h"
#include "rados-namespace-manager.h"
#include "rados-guid-generator.h"


namespace librmb {

class RadosDictionaryImpl : public RadosDictionary {
 public:
  RadosDictionaryImpl(RadosCluster* cluster, const std::string& poolname, const std::string& username,
                      const std::string& oid, librmb::RadosGuidGenerator* guid_generator_,
                      const std::string& cfg_object_name_);
  virtual ~RadosDictionaryImpl();

  const std::string get_full_oid(const std::string& key) override;
  const std::string get_shared_oid() override;
  const std::string get_private_oid() override;

  const std::string& get_oid() override { return oid; }
  const std::string& get_username() override { return username; }
  const std::string& get_poolname() override { return poolname; }

  librmb::RboxIoCtx& get_io_ctx_wrapper(const std::string& key) override;
  librmb::RboxIoCtx& get_shared_io_ctx_wrapper() override;
  librmb::RboxIoCtx& get_private_io_ctx_wrapper() override;

  void remove_completion(librmb::RboxIoCtx &remove_completion_wrapper) override;
  void push_back_completion(librmb::RboxIoCtx &push_back_completion_wrapper) override;
  void wait_for_completions() override;

  int get(const std::string& key, std::string* value_r) override;

 private:
  bool load_configuration(librados::IoCtx* io_ctx);

  bool lookup_namespace(std::string& username_, librmb::RadosDovecotCephCfg* cfg_, std::string* ns);

 private:
  RadosCluster* cluster;
  std::string poolname;
  std::string username;
  std::string oid;

  std::string shared_oid;
  librados::IoCtx shared_io_ctx;
  librmb::RboxIoCtx* shared_io_ctx_wrapper;
  bool shared_io_ctx_created;

  std::string private_oid;
  librados::IoCtx private_io_ctx;
  librmb::RboxIoCtx* private_io_ctx_wrapper;
  bool private_io_ctx_created;

  std::list<librados::AioCompletion*> completions;
  std::mutex completions_mutex;

  librmb::RadosDovecotCephCfg* cfg;
  librmb::RadosNamespaceManager* namespace_mgr;

  RadosGuidGenerator* guid_generator;
  std::string cfg_object_name;
};

}  // namespace librmb

#endif  // SRC_LIBRMB_RADOS_DICTIONARY_IMPL_H_
