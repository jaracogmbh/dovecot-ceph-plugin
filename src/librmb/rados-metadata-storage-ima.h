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

#ifndef SRC_LIBRMB_RADOS_METADATA_STORAGE_IMA_H_
#define SRC_LIBRMB_RADOS_METADATA_STORAGE_IMA_H_

#include <jansson.h>
#include <list>
#include <set>
#include <string>
#include <map>

#include "../storage-interface/rados-ceph-config.h"
#include "../storage-interface/rados-dovecot-ceph-cfg.h"
#include "../storage-interface/rados-metadata-storage-module.h"
#include "../storage-interface/rados-mail.h"
#include "../storage-interface/rbox-io-ctx.h"
namespace librmb {
/**
 *  All immutable mail attributes are saved in one rados
 *  attribute. The value of the attribute is a json format
 *
 * If a attribute changes from immutable to mutable, a
 * new attribute is added to the mail object, which overrides the
 * immutable value.
 *
 */
class RadosMetadataStorageIma : public storage_interface::RadosStorageMetadataModule {
 private:
  int parse_attribute(storage_interface::RadosMail *mail, json_t *root);

 public:
  RadosMetadataStorageIma(storage_interface::RboxIoCtx *io_ctx_wrapper, storage_interface::RadosDovecotCephCfg *cfg_);
  virtual ~RadosMetadataStorageIma();
  void set_io_ctx(storage_interface::RboxIoCtx *io_ctx_wrapper) override { this->io_ctx_wrapper = io_ctx_wrapper; }
  int load_metadata(storage_interface::RadosMail *mail) override;
  int set_metadata(storage_interface::RadosMail *mail, storage_interface::RadosMetadata *xattr) override;
  bool update_metadata(const std::string &oid, std::list<storage_interface::RadosMetadata*> &to_update) override;
  void save_metadata(librados::ObjectWriteOperation *write_op, storage_interface::RadosMail *mail);

  int update_keyword_metadata(const std::string &oid, storage_interface::RadosMetadata *metadata) override;
  int remove_keyword_metadata(const std::string &oid, std::string &key) override;
  int load_keyword_metadata(const std::string &oid, std::set<std::string> &keys,
                            std::map<std::string, ceph::bufferlist> *metadata) override;

 public:
  static std::string module_name;
  static std::string keyword_key;

 private:
  storage_interface::RboxIoCtx *io_ctx_wrapper;
  storage_interface::RadosDovecotCephCfg *cfg;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_RADOS_METADATA_STORAGE_IMA_H_ */
