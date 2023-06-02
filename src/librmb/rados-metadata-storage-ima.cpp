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

#include "rados-metadata-storage-ima.h"
#include "rados-util-impl.h"
#include <string.h>
#include <utility>
#include <unistd.h>
#include "../storage-interface/rados-mail.h"
#include "rados-mail-impl.h"
#include"rados-metadata-impl.h"

std::string librmb::RadosMetadataStorageIma::module_name = "ima";
std::string librmb::RadosMetadataStorageIma::keyword_key = "K";
namespace librmb {

RadosMetadataStorageIma::RadosMetadataStorageIma(storage_interface::RboxIoCtx *io_ctx_wrapper_, storage_interface::RadosDovecotCephCfg *cfg_) {
  this->io_ctx_wrapper = io_ctx_wrapper_;
  this->cfg = cfg_;
}

RadosMetadataStorageIma::~RadosMetadataStorageIma() {}

int RadosMetadataStorageIma::parse_attribute(storage_interface::RadosMail *mail, json_t *root) {
  std::string key;
  void *iter = json_object_iter(root);

  while (iter) {
    key = json_object_iter_key(iter);
    json_t *value = json_object_iter_value(iter);

    if (key.compare(RadosMetadataStorageIma::keyword_key) == 0) {
      std::string _keyword_key;
      void *keyword_iter = json_object_iter(value);
      while (keyword_iter) {
        librados::bufferlist bl;
        _keyword_key = json_object_iter_key(keyword_iter);
        json_t *keyword_value = json_object_iter_value(keyword_iter);
        bl.append(json_string_value(keyword_value));

        (*mail->get_extended_metadata())[_keyword_key] = (void*)&bl;

        keyword_iter = json_object_iter_next(value, keyword_iter);
      }
    } else {
      librados::bufferlist bl;
      bl.append(json_string_value(value));
      (*mail->get_metadata())[key] = (void*)&bl;
    }
    iter = json_object_iter_next(root, iter);
  }
  return 0;
}

int RadosMetadataStorageIma::load_metadata(storage_interface::RadosMail *mail) {
  if (mail == nullptr) {
    return -1;
  }
  if (mail->get_metadata()->size() > 0) {
    return 0;
  }

  std::map<std::string, ceph::bufferlist> attr;
  // retry mechanism ..
  int max_retry = 10;
  int ret = -1;
  for(int i=0;i<max_retry;i++){
    ret = io_ctx_wrapper->getxattrs(*mail->get_oid(), attr);
    if(ret >= 0){      
      break;
    }
    // wait random time before try again!!
    usleep(((rand() % 5) + 1) * 10000);
  }
  
  if (ret < 0) {
    return ret;
  }

  if (attr.find(cfg->get_metadata_storage_attribute()) != attr.end()) {
    // json object for immutable attributes.
    json_t *root;
    json_error_t error;
    root = json_loads(attr[cfg->get_metadata_storage_attribute()].to_str().c_str(), 0, &error);
    parse_attribute(mail, root);

    json_decref(root);
  }

  // load other attributes
  for (std::map<std::string, ceph::bufferlist>::iterator it = attr.begin(); it != attr.end(); ++it) {
    if ((*it).first.compare(cfg->get_metadata_storage_attribute()) != 0) {
      std::string key = (*it).first;
      std::string value = std::string((*it).second.c_str());
      storage_interface::RadosMetadata *xattr= new librmb::RadosMetadataImpl(key,value);
      mail->add_metadata(xattr);
    }
  }

  // load other omap values.
  if (cfg->is_updateable_attribute(storage_interface::RBOX_METADATA_OLDV1_KEYWORDS)) {
    librmb::RadosUtilsImpl rados_utils;
    ret = rados_utils.get_all_keys_and_values(io_ctx_wrapper, *mail->get_oid(), mail);
  }
  return ret;
}

int RadosMetadataStorageIma::set_metadata(storage_interface::RadosMail *mail) {
  librados::ObjectWriteOperation write_op_xattr;
  save_metadata(&write_op_xattr, mail);
  return io_ctx_wrapper->operate(*mail->get_oid(), &write_op_xattr);
}

void RadosMetadataStorageIma::save_metadata(librados::ObjectWriteOperation *write_op, storage_interface::RadosMail *mail) {
  char *s = NULL;
  json_t *root = json_object();
  librados::bufferlist bl;
  std::string buff_str;
  ceph::bufferlist temp_bl;

  if (mail->get_metadata()->size() > 0) {
    for (std::map<std::string, void*>::iterator it = mail->get_metadata()->begin();
         it != mail->get_metadata()->end(); ++it) {
      storage_interface::rbox_metadata_key k = static_cast<storage_interface::rbox_metadata_key>(*(*it).first.c_str());
      buff_str = std::string(((ceph::bufferlist*)(*it).second)->c_str());
      temp_bl.append(buff_str.c_str(), buff_str.size() + 1);
      if (!cfg->is_updateable_attribute(k) || !cfg->is_update_attributes()) {
        json_object_set_new(root, (*it).first.c_str(), json_string(temp_bl.to_str().c_str()));
        temp_bl.clear();
      } else {
        write_op->setxattr((*it).first.c_str(), temp_bl);
        temp_bl.clear();
      }
    }
  }
  json_t *keyword = json_object();
  // build extended Metadata object
  if (mail->get_extended_metadata()->size() > 0) {
    bool json = (!cfg->is_updateable_attribute(storage_interface::RBOX_METADATA_OLDV1_KEYWORDS) || !cfg->is_update_attributes());
    std::map<std::string, ceph::bufferlist> ceph_metadata;
    for (std::map<std::string, void*>::iterator it = mail->get_extended_metadata()->begin();
          it != mail->get_extended_metadata()->end(); ++it) {
      buff_str = std::string(((ceph::bufferlist*)(*it).second)->c_str());
      temp_bl.append(buff_str.c_str(), buff_str.size() +1);  
      if(json){
        json_object_set_new(keyword, (*it).first.c_str(), json_string(temp_bl.to_str().c_str()));
        temp_bl.clear();
      }else{
        ceph_metadata[(*it).first]=temp_bl;
        temp_bl.clear();
      }    
    }
    if(json){
      json_object_set_new(root, RadosMetadataStorageIma::keyword_key.c_str(), keyword);
    }else{
      write_op->omap_set(ceph_metadata);
    }   
  } 

  s = json_dumps(root, 0);
  bl.append(s);
  free(s);
  json_decref(keyword);
  json_decref(root);

  write_op->setxattr(cfg->get_metadata_storage_attribute().c_str(), bl);
}

bool RadosMetadataStorageIma::update_metadata(const std::string &oid, std::list<storage_interface::RadosMetadata*> &to_update) {
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
int RadosMetadataStorageIma::update_keyword_metadata(const std::string &oid, storage_interface::RadosMetadata *metadata) {
  int ret = -1;
  ceph::bufferlist bl;
  std::string buff_str;
  if (metadata != nullptr) {
    if (!cfg->is_updateable_attribute(storage_interface::RBOX_METADATA_OLDV1_KEYWORDS) || !cfg->is_update_attributes()) {
    } else {
    std::map<std::string, librados::bufferlist> map;
    buff_str = std::string(((ceph::bufferlist*)metadata->get_buffer())->c_str());
    bl.append(buff_str.c_str(), buff_str.size() +1);
    map.insert(std::pair<std::string, librados::bufferlist>(metadata->get_key(),bl));
    ret = io_ctx_wrapper->omap_set(map,oid);
    bl.clear();
    }
  }
  return ret;
}
int RadosMetadataStorageIma::remove_keyword_metadata(const std::string &oid, std::string &key) {
  std::set<std::string> keys;
  keys.insert(key);
  return io_ctx_wrapper->omap_rm_keys(oid, keys);
}
} /* namespace librmb */
