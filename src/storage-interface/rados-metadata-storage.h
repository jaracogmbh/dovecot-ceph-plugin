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
#ifndef SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_PRODUCER_H_
#define SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_PRODUCER_H_
#include "rados-dovecot-ceph-cfg.h"
#include "rados-metadata-storage-module.h"
#include "rbox-io-ctx.h"

namespace storage_interface {
/**
 * Rados MetadataStorage
 *
 * abstract class to access the metadata storage.
 *
 */
class RadosMetadataStorage {
 public:
  virtual ~RadosMetadataStorage() {}
  /* create the medata data class based on configuration */
  virtual RadosStorageMetadataModule *create_metadata_storage(storage_interface::RboxIoCtx *io_ctx_wrapper, RadosDovecotCephCfg *cfg_) = 0;
  virtual RadosStorageMetadataModule *get_storage() = 0;
};

}  // namespace storage_interface

#endif /* SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_PRODUCER_H_ */
