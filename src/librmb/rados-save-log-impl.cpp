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

#include "rados-save-log-impl.h"

namespace librmb {

bool RadosSaveLogImpl::open() {
  if (this->log_active && !ofs.is_open()) {
    ofs.open(this->logfile, std::ofstream::out | std::ofstream::app);
    this->log_active = ofs.is_open();
  }
  return this->log_active;
}
void RadosSaveLogImpl::append(const storage_interface::RadosSaveLogEntry *entry) {
  if (this->log_active && ofs.is_open()) {
    const RadosSaveLogEntryImpl &entry_impl=
      *(dynamic_cast<const librmb::RadosSaveLogEntryImpl*>(entry));
    ofs << entry_impl;
  }
}
bool RadosSaveLogImpl::close() {
  if (this->log_active && ofs.is_open()) {
    ofs.close();
    return !ofs.is_open();
  }
  return true;
}

} /* namespace librmb */
