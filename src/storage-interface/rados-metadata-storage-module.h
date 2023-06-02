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

#ifndef SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_MODULE_H_
#define SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_MODULE_H_

#include <string>
#include <list>
#include <set>
#include "rados-mail.h"
#include "rbox-io-ctx.h"
#include "rados-metadata.h"

namespace storage_interface {
class RadosStorageMetadataModule {
 public:
  virtual ~RadosStorageMetadataModule(){};
  /* update io_ctx */
  virtual void set_io_ctx(storage_interface::RboxIoCtx *io_ctx_wrapper){};
  /* load the metadta into RadosMail */
  virtual int load_metadata(storage_interface::RadosMail *mail) = 0;
  /* set a new metadata attribute to a mail object */
  virtual int set_metadata(storage_interface::RadosMail *mail) = 0;
  /* update the given metadata attributes */
  virtual bool update_metadata(const std::string &oid, std::list<storage_interface::RadosMetadata*> &to_update) = 0;
  /* manage keywords */
  virtual int update_keyword_metadata(const std::string &oid, storage_interface::RadosMetadata *metadata) = 0;
  virtual int remove_keyword_metadata(const std::string &oid, std::string &key) = 0;
};

}  // namespace storage_interface

#endif /* SRC_STORAGE_INTERFACES_RADOS_METADATA_STORAGE_MODULE_H_ */
