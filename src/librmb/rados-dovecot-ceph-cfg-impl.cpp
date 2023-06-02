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

#include "rados-dovecot-ceph-cfg-impl.h"
#include "../storage-interface/rados-ceph-config.h"
#include "rados-ceph-config-impl.h"

namespace librmb {

RadosDovecotCephCfgImpl::RadosDovecotCephCfgImpl(storage_interface::RboxIoCtx *io_ctx_) {
  if(rados_cfg!=nullptr){
    rados_cfg->set_io_ctx(io_ctx_);
  }else{
    rados_cfg=
      new librmb::RadosCephConfigImpl(io_ctx_);
  }
}

RadosDovecotCephCfgImpl::RadosDovecotCephCfgImpl(librmb::RadosConfig &dovecot_cfg_, storage_interface::RadosCephConfig *rados_cfg_) : dovecot_cfg(dovecot_cfg_), rados_cfg(rados_cfg_) {}


int RadosDovecotCephCfgImpl::save_default_rados_config() {
  bool valid = rados_cfg->save_cfg() == 0 ? true : false;
  rados_cfg->set_config_valid(valid);
  return valid ? 0 : -1;
}


} /* namespace librmb */
