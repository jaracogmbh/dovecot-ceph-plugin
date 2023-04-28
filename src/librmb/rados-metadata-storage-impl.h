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
#ifndef SRC_LIBRMB_RADOS_METADATA_STORAGE_IMPL_H_
#define SRC_LIBRMB_RADOS_METADATA_STORAGE_IMPL_H_

#include <assert.h> /* assert */
#include <string>
#include "../storage-interface/rados-metadata-storage-module.h"
#include "rados-metadata-storage-default.h"
#include "rados-metadata-storage-ima.h"
#include "../storage-interface/rados-metadata-storage.h"

namespace librmb {
class RadosMetadataStorageImpl : public storage_interface::RadosMetadataStorage {
 public:
  RadosMetadataStorageImpl() {
    storage = nullptr;
    io_ctx_wrapper = nullptr;
    cfg = nullptr;
  }
  virtual ~RadosMetadataStorageImpl() {
    if (storage != nullptr) {
      delete storage;
      storage = nullptr;
    }
  }

  storage_interface::RadosStorageMetadataModule *create_metadata_storage(storage_interface::RboxIoCtx *io_ctx_wrapper_, storage_interface::RadosDovecotCephCfg *cfg_) override {
    this->io_ctx_wrapper = io_ctx_wrapper_;
    this->cfg = cfg_;
    if (storage == nullptr) {
      // decide metadata storage!
      std::string storage_module_name = cfg_->get_metadata_storage_module();
      if (storage_module_name.compare(librmb::RadosMetadataStorageIma::module_name) == 0) {
        storage = new librmb::RadosMetadataStorageIma(io_ctx_wrapper, cfg_);
      } else {
        storage = new librmb::RadosMetadataStorageDefault(io_ctx_wrapper);
      }
    }
    return storage;
  }

  storage_interface::RadosStorageMetadataModule *get_storage() override {
    assert(storage != nullptr);
    return storage;
  }

 private:
  storage_interface::RboxIoCtx *io_ctx_wrapper;
  storage_interface::RadosDovecotCephCfg *cfg;
  storage_interface::RadosStorageMetadataModule *storage;
};
}  // namespace librmb

#endif  // SRC_LIBRMB_RADOS_METADATA_STORAGE_IMPL_H_
