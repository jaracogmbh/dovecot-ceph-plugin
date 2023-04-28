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

#ifndef SRC_STORAGE_INTERFACES_TOOLS_RMB_RADOS_MAIL_BOX_H_
#define SRC_STORAGE_INTERFACES_TOOLS_RMB_RADOS_MAIL_BOX_H_

#include <string>
#include <sstream>
#include <list>
#include <map>

#include "../../rados-mail.h"
#include "ls_cmd_parser.h"
#include "../../rados-util.h"
namespace storage_interface {

class RadosMailBox {
 public:
  virtual ~RadosMailBox() {}

  virtual void add_mail(storage_interface::RadosMail *mail) = 0;
  virtual void add_to_mailbox_size(const uint64_t &_mailbox_size) = 0;
  virtual void set_mails(const std::list<storage_interface::RadosMail *> &_mails) = 0;

  virtual storage_interface::CmdLineParser *get_xattr_filter() = 0;
  virtual void set_xattr_filter(storage_interface::CmdLineParser *_parser) = 0;
  virtual std::list<storage_interface::RadosMail *> &get_mails() = 0;
  virtual std::string &get_mailbox_guid() = 0;
  virtual void set_mailbox_guid(const std::string &_mailbox_guid) = 0;
  virtual void set_mailbox_orig_name(const std::string &_mbox_orig_name) = 0;
  virtual int &get_mail_count() = 0;
};
}  // namespace storage_interface

#endif  // SRC_STORAGE_INTERFACES_TOOLS_RMB_RADOS_MAIL_BOX_H_
