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
#include "rados-util-impl.h"
#include <utility>
#include "../storage-interface/rados-mail.h"
#include "rados-metadata-impl.h"
#include "rados-mail-impl.h"
namespace librmb {

std::string RadosMetadataStorageDefault::module_name = "default";

RadosMetadataStorageDefault::RadosMetadataStorageDefault(storage_interface::RboxIoCtx *io_ctx_wrapper) { this->io_ctx_wrapper = io_ctx_wrapper; }

RadosMetadataStorageDefault::~RadosMetadataStorageDefault() {}

int RadosMetadataStorageDefault::load_metadata(storage_interface::RadosMail *mail) {
  int ret = -1;
  if (mail == nullptr) {
    return ret;
  }
  if (mail->get_metadata()->size() > 0) {
    mail->get_metadata()->clear();
  }
  std::map<std::string, ceph::bufferlist> ceph_metadata;

  ret = io_ctx_wrapper->getxattrs(*mail->get_oid(),ceph_metadata);
  if(ret >= 0){  
    for(std::map<std::string,ceph::bufferlist>::iterator it=ceph_metadata.begin(); it!=ceph_metadata.end(); ++it){   
        std::string key = (*it).first;
        std::string value = std::string((*it).second.c_str());
        storage_interface::RadosMetadata *xattr= new RadosMetadataImpl(key,value);
        
        mail->add_metadata(xattr);          
    }
    librmb::RadosUtilsImpl rados_utils;
    ret = rados_utils.get_all_keys_and_values(io_ctx_wrapper, *mail->get_oid(), mail);
  }

  return ret;
}
// xattr is deprecated
int RadosMetadataStorageDefault::set_metadata(storage_interface::RadosMail *mail) {
  librados::ObjectWriteOperation write_op_xattr;
  save_metadata(&write_op_xattr, mail);
  return io_ctx_wrapper->operate(*mail->get_oid(), &write_op_xattr);    

}

void RadosMetadataStorageDefault::save_metadata(librados::ObjectWriteOperation *write_op, storage_interface::RadosMail *mail) {
  // update metadata
  ceph::bufferlist bl;
  std::string buff_str;

  for (std::map<std::string, void*>::iterator it = mail->get_metadata()->begin();
        it != mail->get_metadata()->end(); ++it) {
    buff_str = std::string(((ceph::bufferlist*)(*it).second)->c_str());
    bl.append(buff_str.c_str(), buff_str.size() + 1);
    const char* key =  (*it).first.c_str();
    std::cout << bl.c_str() <<key  << std::endl;

    write_op->setxattr(key,bl);
    bl.clear();
  }
  
  if (mail->get_extended_metadata()->size() > 0) {
    std::map<std::string, ceph::bufferlist> ceph_metadata;    
    for(std::map<std::string,void*>::iterator it=mail->get_extended_metadata()->begin(); 
         it!=mail->get_extended_metadata()->end(); ++it){
      buff_str = std::string(((ceph::bufferlist*)(*it).second)->c_str());
      bl.append(buff_str.c_str(), buff_str.size() +1);     
      ceph_metadata[(*it).first]=bl;
      bl.clear();    
    }
    write_op->omap_set(ceph_metadata);
  }
}

bool RadosMetadataStorageDefault::update_metadata(const std::string &oid, std::list<storage_interface::RadosMetadata*> &to_update) {
  librados::ObjectWriteOperation write_op;
  ceph::bufferlist bl;
  std::string buff_str;
  // update metadata
  for (std::list<storage_interface::RadosMetadata*>::iterator it = to_update.begin(); it != to_update.end(); ++it) {
    buff_str = std::string(((ceph::bufferlist*)(*it)->get_buffer())->c_str());
    bl.append(buff_str.c_str(), buff_str.size() + 1);
    write_op.setxattr((*it)->get_key().c_str(), bl);
    bl.clear();
  }

  int ret = io_ctx_wrapper->operate(oid, &write_op);
  return ret == 0;
}
int RadosMetadataStorageDefault::update_keyword_metadata(const std::string &oid, storage_interface::RadosMetadata *metadata) {
  int ret = -1;
  ceph::bufferlist bl;
  std::string buff_str;
  if (metadata != nullptr) {
    std::map<std::string, librados::bufferlist> map;
    buff_str = std::string(((ceph::bufferlist*)metadata->get_buffer())->c_str());
    bl.append(buff_str.c_str(), buff_str.size() +1);
    map.insert(std::pair<std::string, librados::bufferlist>(metadata->get_key(),bl));
    ret = io_ctx_wrapper->omap_set(map,oid);
    bl.clear();
  }
  return ret;
}
int RadosMetadataStorageDefault::remove_keyword_metadata(const std::string &oid, std::string &key) {
  std::set<std::string> keys;
  keys.insert(key);
  return io_ctx_wrapper->omap_rm_keys(oid, keys);
}
} /* namespace librmb */
