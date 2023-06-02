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

#include "rados-mail-impl.h"

#include <stdlib.h>

#include <cstring>
#include <sstream>
#include "rados-util-impl.h"

using std::endl;
using std::ostringstream;

using librmb::RadosMailImpl;

RadosMailImpl::RadosMailImpl()
    : object_size(-1),
      mail_buffer(nullptr),
      save_date_rados(-1),
      valid(true),
      index_ref(false),
      deprecated_uid(false),
      restored(false),
      lost_object(false) {}

RadosMailImpl::~RadosMailImpl() {
  /*NOTE: This means all metadata objects must be created with new!*/
 for(std::list<storage_interface::RadosMetadata*>::iterator
      it=metadata_list.begin(); it!=metadata_list.end(); ++it){
    delete *it;
    *it=nullptr; 
  }
  for(std::list<storage_interface::RadosMetadata*>::iterator
      it=extended_metadata_list.begin(); it!=extended_metadata_list.end(); ++it){
    delete *it;
    *it=nullptr; 
  }
}

std::string RadosMailImpl::to_string(const string& padding) {
  librmb::RadosUtilsImpl rados_utils;
  char* uid = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_MAIL_UID, this->get_metadata(), &uid);
  char* recv_time_str = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_RECEIVED_TIME, this->get_metadata(), &recv_time_str);
  char* p_size = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_PHYSICAL_SIZE, this->get_metadata(), &p_size);
  char* v_size = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_VIRTUAL_SIZE, this->get_metadata(), &v_size);

  char* rbox_version = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_VERSION, this->get_metadata(), &rbox_version);
  char* mailbox_guid = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_MAILBOX_GUID, this->get_metadata(), &mailbox_guid);
  char* mail_guid = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_GUID, this->get_metadata(), &mail_guid);
  char* mb_orig_name = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_ORIG_MAILBOX, this->get_metadata(), &mb_orig_name);

  // string keywords = get_metadata(RBOX_METADATA_OLDV1_KEYWORDS);
  char* flags = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_OLDV1_FLAGS, this->get_metadata(), &flags);
  char* pvt_flags = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_PVT_FLAGS, this->get_metadata(), &pvt_flags);
  char* from_envelope = NULL;
  rados_utils.get_metadata(storage_interface::RBOX_METADATA_FROM_ENVELOPE, this->get_metadata(), &from_envelope);

  time_t ts = -1;
  if (recv_time_str != NULL) {
    try {
      std::string recv(recv_time_str);
      ts = static_cast<time_t>(stol(recv));
    } catch (std::exception& ex) {
      ts = -1;
    }
  }
  ostringstream ss;
  ss << endl;
  if (!valid) {
    ss << padding << "<<<   MAIL OBJECT IS NOT VALID <<<<" << endl;
  }
  if (!index_ref) {
    ss << padding << "<<<   MAIL OBJECT HAS NO INDEX REFERENCE <<<< oid: " << oid << endl;
  }
  ss << padding << "MAIL:   ";
  if (uid != NULL) {
    ss << static_cast<char>(storage_interface::RBOX_METADATA_MAIL_UID) << "(uid)=" << uid << endl;
    ss << padding << "        ";
  }
  ss << "oid = " << oid << endl;
  string recv_time;
  if (rados_utils.convert_time_t_to_str(ts, &recv_time) >= 0) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_RECEIVED_TIME) << "(receive_time)=" << recv_time
       << "\n";
  } else {
    if (recv_time_str != NULL) {
      ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_RECEIVED_TIME)
         << "(receive_time)= INVALID DATE : '" << recv_time_str << "'"
         << "\n";
    }
  }
  string save_time;
  if (rados_utils.convert_time_t_to_str(save_date_rados, &save_time) >= 0) {
    ss << padding << "        "
       << "save_time=" << save_time << "\n";
  } else {
    ss << padding << "        "
       << "save_time= UNKNOWN '" << save_date_rados << "'\n";
  }
  if (p_size != NULL && v_size != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_PHYSICAL_SIZE) << "(phy_size)=" << p_size << " "
       << static_cast<char>(storage_interface::RBOX_METADATA_VIRTUAL_SIZE) << "(v_size) = " << v_size << " stat_size=" << object_size
       << endl;
  }
  if (mailbox_guid != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_MAILBOX_GUID) << "(mailbox_guid)=" << mailbox_guid
       << endl;
  }
  if (mb_orig_name != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_ORIG_MAILBOX)
       << "(mailbox_orig_name)=" << mb_orig_name << endl;
  }
  if (mail_guid != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_GUID) << "(mail_guid)=" << mail_guid << endl;
  }
  if (rbox_version != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_VERSION) << "(rbox_version): " << rbox_version
       << endl;
  }

  if (extended_attrset.size() > 0) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_OLDV1_KEYWORDS) << "(keywords): " << std::endl;
    for (std::map<string, void*>::iterator iter = extended_attrset.begin(); iter != extended_attrset.end();
         ++iter) {
      ss << "                             " << iter->first << " : " << ((ceph::bufferlist*)iter->second)->to_str() << endl;
    }
  }

  if (flags != NULL) {
    uint8_t flags_;
    if (rados_utils.string_to_flags(flags, &flags_)) {
      std::string resolved_flags;
      rados_utils.resolve_flags(flags_, &resolved_flags);
      ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_OLDV1_FLAGS) << "(flags): " << resolved_flags
         << std::endl;
    }
  }

  if (pvt_flags != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_PVT_FLAGS) << "(private flags): " << pvt_flags
       << endl;
  }

  if (from_envelope != NULL) {
    ss << padding << "        " << static_cast<char>(storage_interface::RBOX_METADATA_FROM_ENVELOPE)
       << "(from envelope): " << from_envelope << endl;
  }

  return ss.str();
}
