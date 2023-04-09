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

#include "rados-metadata-storage-default.h"
#include "rados-util.h"
#include <utility>
#include "../storage-interface/rados-mail.h"
namespace librmb {

std::string RadosMetadataStorageDefault::module_name = "default";

RadosMetadataStorageDefault::RadosMetadataStorageDefault(librmb::RboxIoCtx &io_ctx_wrapper) { this->io_ctx_wrapper = &io_ctx_wrapper; }

RadosMetadataStorageDefault::~RadosMetadataStorageDefault() {}

int RadosMetadataStorageDefault::load_metadata(storage_interface::RadosMail *mail) {
  int ret = -1;
  if (mail == nullptr) {
    return ret;
  }
  if (mail->get_metadata()->size() > 0) {
    mail->get_metadata()->clear();
  }
  ret = io_ctx_wrapper->getxattrs(*mail->get_oid(), *mail->get_metadata());

  if (ret >= 0) {
    ret = RadosUtils::get_all_keys_and_values(&io_ctx_wrapper->get_io_ctx(), *mail->get_oid(), mail->get_extended_metadata());
  }
  return ret;
}
int RadosMetadataStorageDefault::set_metadata(storage_interface::RadosMail *mail, RadosMetadata &xattr) {
  if(mail->get_metadata()->size()==0){
    mail->add_metadata(xattr);
    return io_ctx_wrapper->setxattr(*mail->get_oid(), xattr.key.c_str(), xattr.bl);
  }else{
    librados::ObjectWriteOperation write_op_xattr;
    save_metadata(&write_op_xattr,mail);
    return io_ctx_wrapper->operate(*mail->get_oid(), &write_op_xattr);
  }
}
void RadosMetadataStorageDefault::save_metadata(librados::ObjectWriteOperation *write_op, storage_interface::RadosMail *mail) {
  // update metadata
  for (std::map<std::string, ceph::bufferlist>::iterator it = mail->get_metadata()->begin();
       it != mail->get_metadata()->end(); ++it) {
    write_op->setxattr((*it).first.c_str(), (*it).second);
  }
  if (mail->get_extended_metadata()->size() > 0) {
    write_op->omap_set(*mail->get_extended_metadata());
  }
}
bool RadosMetadataStorageDefault::update_metadata(const std::string &oid, std::list<RadosMetadata> &to_update) {
  librados::ObjectWriteOperation write_op;
  librados::AioCompletion *completion = librados::Rados::aio_create_completion();

  // update metadata
  for (std::list<RadosMetadata>::iterator it = to_update.begin(); it != to_update.end(); ++it) {
    write_op.setxattr((*it).key.c_str(), (*it).bl);
  }

  int ret = io_ctx_wrapper->aio_operate(oid, completion, &write_op);
  completion->wait_for_complete();
  completion->release();
  return ret == 0;
}
int RadosMetadataStorageDefault::update_keyword_metadata(const std::string &oid, RadosMetadata *metadata) {
  int ret = -1;
  if (metadata != nullptr) {
    std::map<std::string, librados::bufferlist> map;
    map.insert(std::pair<std::string, librados::bufferlist>(metadata->key, metadata->bl));
    ret = io_ctx_wrapper->omap_set(map,oid);
  }
  return ret;
}
int RadosMetadataStorageDefault::remove_keyword_metadata(const std::string &oid, std::string &key) {
  std::set<std::string> keys;
  keys.insert(key);
  return io_ctx_wrapper->omap_rm_keys(oid, keys);
}
int RadosMetadataStorageDefault::load_keyword_metadata(const std::string &oid, std::set<std::string> &keys,
                                                       std::map<std::string, ceph::bufferlist> *metadata) {
  return io_ctx_wrapper->omap_get_vals_by_keys(oid, keys, metadata);
}

} /* namespace librmb */
