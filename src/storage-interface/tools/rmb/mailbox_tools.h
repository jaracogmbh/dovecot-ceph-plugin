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

#ifndef SRC_STORAGE_INTERFACES_TOOLS_RMB_MAILBOX_TOOLS_H_
#define SRC_STORAGE_INTERFACES_TOOLS_RMB_MAILBOX_TOOLS_H_

#include <string>

#include "../../rados-mail.h"
#include "rados-mail-box.h"

namespace storage_interface {
class MailboxTools {
 public:
  virtual ~MailboxTools() {}

  virtual int init_mailbox_dir() = 0;
  virtual int save_mail(storage_interface::RadosMail* mail_obj) = 0;
  virtual int delete_mailbox_dir() = 0;
  virtual int delete_mail(storage_interface::RadosMail* mail_obj) = 0;

  virtual int build_filename(storage_interface::RadosMail* mail_obj, std::string* filename) = 0;

  virtual std::string& get_mailbox_path() = 0;
};
};  // namespace storage_interface

#endif  // SRC_STORAGE_INTERFACES_TOOLS_RMB_MAILBOX_TOOLS_H_
