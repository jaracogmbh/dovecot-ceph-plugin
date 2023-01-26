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

#ifndef SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_
#define SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_


#include <string>
#include <rados/librados.hpp>

#include "rbox-io-ctx.h"

namespace librmb{
class RboxIoCtxImpl:public RboxIoCtx{
    virtual ~RboxIoCtxImpl(){}
    int operate(std::string &oid, librados::ObjectWriteOperation *write_op_xattr) override{
        return -1;
    }
    bool append(std::string &oid, librados::bufferlist &bufferlist, int length) override{
        return false;
    }
    int operate(const std::string &oid,librados::ObjectReadOperation *read_op,librados::bufferlist* buffer) override{
        return -1;
    }
};    
}
#endif  // SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_