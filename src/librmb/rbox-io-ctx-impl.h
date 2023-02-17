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
    public:
    virtual ~RboxIoCtxImpl(){}
    void set_Io_Ctx(librados::IoCtx* storage_io_ctx) override{
        io_ctx=storage_io_ctx;
    }
    librados::IoCtx* get_Io_Ctx() override{
        return io_ctx;
    }
    int read(std::string* oid, librados::bufferlist* bl, size_t len, uint64_t off) override{
        const std::string& oid_ref=*oid;
        return io_ctx->read(oid_ref,*bl,len,0);
    }
    int operate(std::string* oid, librados::ObjectWriteOperation *write_op_xattr) override{
        const std::string& oid_ref=*oid;
        return io_ctx->operate(oid,write_op_xattr);
    }
    bool append(std::string* oid, librados::bufferlist &bufferlist, int length) override{
        const std::string& oid_ref=*oid;
        return io_ctx->append(oid, bufferlist, length);
    }
    int operate(std::string* oid,librados::ObjectReadOperation *read_op,librados::bufferlist* buffer) override{
        const std::string& oid_ref=*oid;
        return io_ctx->operate(oid,read_op,buffer);
    }
    private:
    librados::IoCtx* io_ctx;
};    
}
#endif  // SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_