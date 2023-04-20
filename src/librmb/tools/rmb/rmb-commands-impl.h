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

#ifndef SRC_LIBRMB_TOOLS_RMB_RMB_COMMANDS_IMPL_H_
#define SRC_LIBRMB_TOOLS_RMB_RMB_COMMANDS_IMPL_H_
#include <stdlib.h>

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <iterator>
#include <list>

#include "../../../storage-interface/rados-storage.h"
#include "../../../storage-interface/rados-cluster.h"
#include "../../../storage-interface/rados-metadata-storage.h"
#include "../../../storage-interface/rados-dovecot-ceph-cfg.h"
#include "../../../storage-interface/rados-ceph-config.h"
#include "../../../storage-interface/tools/rmb/ls_cmd_parser.h"
#include "../../../storage-interface/tools/rmb/mailbox_tools.h"
#include "../../../storage-interface/rados-metadata-storage-module.h"
#include "../../../storage-interface/tools/rmb/rmb-commands.h"
#include "../../../storage-interface/rados-save-log.h"
using storage_interface::RmbCommands;
namespace librmb {

class RmbCommandsImpl : public storage_interface::RmbCommands {
 public:
  RmbCommandsImpl() {}
  RmbCommandsImpl(storage_interface::RadosStorage *storage_, storage_interface::RadosCluster *cluster_,
              std::map<std::string, std::string> *opts_);
  virtual ~RmbCommandsImpl();

  int delete_with_save_log(const std::string &save_log, const std::string &rados_cluster,
                                  const std::string &rados_user,
                                  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> *moved_items) override;
  void print_debug(const std::string &msg) override;
  int lspools() override;
  int delete_mail(bool confirmed) override;
  int delete_namespace(storage_interface::RadosStorageMetadataModule *ms, std::list<storage_interface::RadosMail *> &mail_objects,
                       storage_interface::RadosCephConfig *cfg, bool confirmed) override;

  int rename_user(storage_interface::RadosCephConfig *cfg, bool confirmed, const std::string &uid) override;
  int configuration(bool confirmed, storage_interface::RadosCephConfig *ceph_cfg) override;
  int load_objects(storage_interface::RadosStorageMetadataModule *ms, std::list<storage_interface::RadosMail *> &mail_objects,
                   std::string &sort_string, bool load_metadata = true)override ;

  int update_attributes(storage_interface::RadosStorageMetadataModule *ms, std::map<std::string, std::string> *metadata) override;
  int print_mail(std::map<std::string, storage_interface::RadosMailBox *> *mailbox, std::string &output_dir, bool download) override;
  int query_mail_storage(std::list<storage_interface::RadosMail *> *mail_objects, storage_interface::CmdLineParser *parser, bool download,
                         bool silent) override;

  storage_interface::RadosStorageMetadataModule *init_metadata_storage_module(storage_interface::RadosCephConfig *ceph_cfg, std::string *uid) override;

  static bool sort_uid(storage_interface::RadosMail *i, storage_interface::RadosMail *j);
  static bool sort_recv_date(storage_interface::RadosMail *i, storage_interface::RadosMail *j);
  static bool sort_phy_size(storage_interface::RadosMail *i, storage_interface::RadosMail *j);
  static bool sort_save_date(storage_interface::RadosMail *i, storage_interface::RadosMail *j);

  void set_output_path(storage_interface::CmdLineParser *parser) override;

  int overwrite_ceph_object_index(std::set<std::string> &mail_oids) override;
  std::set<std::string> load_objects(storage_interface::RadosStorageMetadataModule *ms) override;
  int remove_ceph_object_index() override;
  int append_ceph_object_index(const std::set<std::string> &mail_oids) override;

 private:
  std::map<std::string, std::string> *opts;
  storage_interface::RadosStorage *storage;
  storage_interface::RadosCluster *cluster;
  bool is_debug;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_TOOLS_RMB_RMB_COMMANDS_IMPL_H_ */
