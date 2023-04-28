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

#ifndef SRC_STORAGE_INTERFACES_TOOLS_RMB_RMB_COMMANDS_H_
#define SRC_STORAGE_INTERFACES_TOOLS_RMB_RMB_COMMANDS_H_
#include <stdlib.h>

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <iterator>
#include <list>

#include "../../rados-storage.h"
#include "../../rados-cluster.h"
#include "../../rados-metadata-storage.h"
#include "../../rados-dovecot-ceph-cfg.h"
#include "../../rados-ceph-config.h"
#include "ls_cmd_parser.h"
#include "mailbox_tools.h"
#include "../../rados-metadata-storage-module.h"
#include "../../rados-save-log.h"

namespace storage_interface {

class RmbCommands {
 public:
  virtual ~RmbCommands(){}

  virtual int delete_with_save_log(const std::string &save_log, const std::string &rados_cluster,
                                  const std::string &rados_user,
                                  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> *moved_items) = 0;
  virtual void print_debug(const std::string &msg) = 0;
  virtual int lspools() = 0;
  virtual int delete_mail(bool confirmed) = 0;
  virtual int delete_namespace(storage_interface::RadosStorageMetadataModule *ms, std::list<storage_interface::RadosMail *> &mail_objects,
                       storage_interface::RadosCephConfig *cfg, bool confirmed) = 0;

  virtual int rename_user(storage_interface::RadosCephConfig *cfg, bool confirmed, const std::string &uid) = 0;

  virtual int configuration(bool confirmed, storage_interface::RadosCephConfig *ceph_cfg) = 0;

  virtual int load_objects(storage_interface::RadosStorageMetadataModule *ms, std::list<storage_interface::RadosMail *> &mail_objects,
                   std::string &sort_string, bool load_metadata = true) = 0;
  virtual int update_attributes(storage_interface::RadosStorageMetadataModule *ms, std::map<std::string, std::string> *metadata) = 0;
  virtual int print_mail(std::map<std::string, storage_interface::RadosMailBox *> *mailbox, std::string &output_dir, bool download) = 0;
  virtual int query_mail_storage(std::list<storage_interface::RadosMail *> *mail_objects, storage_interface::CmdLineParser *parser, bool download,
                         bool silent) = 0;
  virtual storage_interface::RadosStorageMetadataModule *init_metadata_storage_module(storage_interface::RadosCephConfig *ceph_cfg, std::string *uid) = 0;
  virtual void set_output_path(storage_interface::CmdLineParser *parser) = 0;
  virtual int overwrite_ceph_object_index(std::set<std::string> &mail_oids) = 0;
  virtual std::set<std::string> load_objects(storage_interface::RadosStorageMetadataModule *ms) = 0;
  virtual int remove_ceph_object_index() = 0;
  virtual int append_ceph_object_index(const std::set<std::string> &mail_oids) = 0;
};

} /* namespace storage_interface */

#endif /* SRC_STORAGE_INTERFACES_TOOLS_RMB_RMB_COMMANDS_H_ */
