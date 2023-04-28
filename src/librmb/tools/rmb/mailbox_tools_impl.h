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

#ifndef SRC_LIBRMB_TOOLS_RMB_MAILBOX_TOOLS_IMPL_H_
#define SRC_LIBRMB_TOOLS_RMB_MAILBOX_TOOLS_IMPL_H_

#include <string>

#include "../../../storage-interface/rados-mail.h"
#include "../../../storage-interface/tools/rmb/mailbox_tools.h"
#include "../../../storage-interface/tools/rmb/rados-mail-box.h"

namespace librmb {
class MailboxToolsImpl : public storage_interface::MailboxTools {
 public:
  MailboxToolsImpl(storage_interface::RadosMailBox* mailbox, const std::string& base);
  ~MailboxToolsImpl() {}

  int init_mailbox_dir() override;
  int save_mail(storage_interface::RadosMail* mail_obj) override;
  int delete_mailbox_dir();
  int delete_mail(storage_interface::RadosMail* mail_obj) override;

  int build_filename(storage_interface::RadosMail* mail_obj, std::string* filename) override;

  std::string& get_mailbox_path()  override{ return this->mailbox_path; }

 private:
  storage_interface::RadosMailBox* mbox;
  std::string base_path;
  std::string mailbox_path;
};
};  // namespace librmb

#endif  // SRC_LIBRMB_TOOLS_RMB_MAILBOX_TOOLS_IMPL_H_
