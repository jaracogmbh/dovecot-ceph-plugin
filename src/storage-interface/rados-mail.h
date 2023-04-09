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

#ifndef SRC_STORAGE_INTERFACE_RADOS_MAIL_INTERFACE_H_
#define SRC_STORAGE_INTERFACE_RADOS_MAIL_INTERFACE_H_

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include "../librmb/rados-metadata.h"
#include "rados-types.h"
#include <rados/librados.hpp>

namespace storage_interface {

using std::map;
using std::string;

class RadosMail{
 public:
  virtual ~RadosMail(){}
  virtual void set_oid(const char* _oid) = 0; 
  virtual void set_oid(const string& _oid) = 0; 
  virtual void set_mail_size(const int _size) = 0;
  virtual void set_rados_save_date(const time_t& _save_date) = 0;
  virtual string* get_oid() = 0;
  virtual int get_mail_size() = 0;
  virtual time_t get_rados_save_date() = 0; 
  virtual uint8_t get_guid_ref() = 0; 
  
  /*!
   * @return ptr to internal buffer .
   */
  virtual void set_mail_buffer(void* buffer) = 0; 
  virtual void* get_mail_buffer() = 0; 
  virtual map<string, ceph::bufferlist>* get_metadata() = 0; 
  virtual bool is_index_ref() = 0;
  virtual void set_index_ref(bool ref) = 0; 
  virtual bool is_valid() = 0; 
  virtual void set_valid(bool valid_) = 0; 
  
  virtual bool is_restored() = 0; 
  virtual void set_restored(bool restored_) = 0; 
  
  virtual bool is_lost_object() = 0; 
  virtual void set_lost_object(bool is_lost_object) = 0; 
  virtual string to_string(const string& padding) = 0;
  virtual void add_metadata(const librmb::RadosMetadata& metadata) = 0; 
  virtual bool is_deprecated_uid() = 0; 
  virtual void set_deprecated_uid(bool deprecated_uid_) = 0; 
  /*!
   * Some metadata isn't saved as xattribute (default). To access those, get_extended_metadata can
   * be used.
   */
  virtual map<string, ceph::bufferlist>* get_extended_metadata() = 0; 
  /*!
   * Save metadata to extended metadata store currently omap
   * @param[in] metadata valid radosMetadata.
   */
  virtual void add_extended_metadata(const librmb::RadosMetadata& metadata) = 0;

  virtual const string get_extended_metadata(const string& key) = 0;
};

}  // namespace storage_interface

#endif  // SRC_STORAGE_INTERFACE_RADOS_MAIL_INTERFACE_H_
