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

#include "ls_cmd_parser_impl.h"

#include <iostream>
#include <string>

namespace librmb {

CmdLineParserImpl::~CmdLineParserImpl() {
  for (auto &it : predicates) {
    delete it.second;
  }
}

storage_interface::Predicate *CmdLineParserImpl::create_predicate(const std::string &_ls_value) {
  storage_interface::Predicate *p = new librmb::PredicateImpl();

  size_t pos = _ls_value.find("=");
  pos = (pos == std::string::npos) ? _ls_value.find(">") : pos;
  pos = (pos == std::string::npos) ? _ls_value.find("<") : pos;

  p->get_key() = _ls_value.substr(0, pos);
  p->get_op() = _ls_value[pos];
  p->get_value() = _ls_value.substr(pos + 1, _ls_value.length());
  p->get_valid() = true;

  this->keys += p->get_key() + " ";
  // std::cout << " predicate: key " << p->key << " op " << p->op << " value " << p->value << std::endl;
  return p;
}

void CmdLineParserImpl::set_output_dir(const std::string& out) {
  if (out.length() > 0 && out.at(0) == '~') {
    // Convert tilde to $HOME path (if exists)
    char *home = getenv("HOME");
    if (home != NULL) {
      char outpath[PATH_MAX];
      snprintf(outpath, sizeof(outpath), "%s", home);
      out_dir.clear();
      out_dir.append(outpath);
      if (out.length() > 1 && out.at(1) != '/') {
        out_dir.append(1, '/');
      }
      out_dir.append(out, 1, std::string::npos);
    } else {
      this->out_dir = out;
    }

  } else {
    this->out_dir = out;
  }
}

bool CmdLineParserImpl::parse_ls_string() {
  std::string pred_sep = ";";

  size_t pos = ls_value.find(pred_sep);
  if (pos == std::string::npos) {
    // single condition.
    storage_interface::Predicate *p = create_predicate(ls_value);
    if (p->get_valid()) {
      predicates[p->get_key()] = p;
    }
    return p->get_valid();

  } else {
    int offset = 0;
    std::string tmp = ls_value;
    while (pos != std::string::npos) {
      tmp = tmp.substr(0, pos);

      storage_interface::Predicate *p = create_predicate(tmp);
      if (p->get_valid()) {
        predicates[p->get_key()] = p;
      }

      tmp = ls_value.substr(offset + pos + 1, ls_value.length());
      offset += pos + 1;
      pos = tmp.find(pred_sep);
    }
    storage_interface::Predicate *p = create_predicate(tmp);
    if (p->get_valid()) {
      predicates[p->get_key()] = p;
    }
    return p->get_valid();
  }
}
}  // namespace librmb
