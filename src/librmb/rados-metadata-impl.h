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

#ifndef SRC_LIBRMB_RADOS_METADATA_H_
#define SRC_LIBRMB_RADOS_METADATA_H_

#include "../storage-interface/rados-types.h"
#include <string>
#include "time.h"
#include <stdlib.h>
#include <sstream>
#include <rados/librados.hpp>
#include "../storage-interface/rados-metadata.h"

namespace librmb {

class RadosMetadataImpl : public storage_interface::RadosMetadata{
 public:
  RadosMetadataImpl() {}
  RadosMetadataImpl(std::string& key_, std::string& value_){
    this->key = key_;
    (*bl).clear();
    (*bl).append(value_.c_str(), value_.length() + 1);
  }
  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const std::string& val) { convert(_key, val); }

  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const time_t& val) { convert(_key, val); }

  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const char* val) { convert(_key, val); }

  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const uint& val) { convert(_key, val); }

  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const size_t& val) { convert(_key, val); }
  RadosMetadataImpl(storage_interface::rbox_metadata_key _key, const int val) { convert(_key, val); }
  ~RadosMetadataImpl() {
    if(bl != nullptr){
      delete bl;
      bl = nullptr;
    }
  }

  void convert(const char* value, time_t* t) override {
    if (t != NULL) {
      std::istringstream stream(value);
      stream >> *t;
    }
  }
  
  bool from_string(const std::string& str) override {
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> token;
    while (std::getline(ss, item, '=')) {
      token.push_back(item);
    }
    if (token.size() != 2 || this == nullptr) {
      return false;
    }
    this->key = token[0];
    this->bl->append(token[1]);
    return true;
  }

  std::string to_string() override {
    std::stringstream str;
    str << key << "=" << (*bl).to_str().substr(0, (*bl).length() - 1);
    return str.str();
  }

 public:
  ceph::bufferlist *bl= new ceph::bufferlist();
  std::string key;
  
  void* get_buffer() override{
    return (void*)bl;
  };
  
  std::string& get_key() override{
    return key;
  };

  void set_key(std::string& key_) override{
    this->key=key_;
  };

  void convert(storage_interface::rbox_metadata_key _key, const std::string& val) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    (*bl).append(val.c_str(), val.length() + 1);
  }

  void convert(storage_interface::rbox_metadata_key _key, const time_t& time) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    std::string time_ = std::to_string(time);
    (*bl).append(time_.c_str(), time_.length() + 1);
  }

  void convert(storage_interface::rbox_metadata_key _key, const char* value) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    std::string str = value;
    (*bl).append(str.c_str(), str.length() + 1);
  }

  void convert(storage_interface::rbox_metadata_key _key, const uint& value) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    std::string val = std::to_string(value);
    (*bl).append(val.c_str(), val.length() + 1);
  }

  void convert(storage_interface::rbox_metadata_key _key, const size_t& value) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    std::string val = std::to_string(static_cast<int>(value));
    (*bl).append(val.c_str(), val.length() + 1);
  }

  void convert(storage_interface::rbox_metadata_key _key, const int value) override {
    (*bl).clear();
    key = storage_interface::rbox_metadata_key_to_char(_key);
    std::string val = std::to_string(value);
    (*bl).append(val.c_str(), val.length() + 1);
  }
};
}  // end namespace
#endif /* SRC_LIBRMB_RADOS_METADATA_H_ */
