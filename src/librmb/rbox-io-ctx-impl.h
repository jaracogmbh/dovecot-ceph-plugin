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
    RboxIoCtxImpl(){}
    librados::NObjectIterator nobjects_begin() override{
        return get_Io_Ctx().nobjects_begin();
    }
    librados::NObjectIterator nobjects_begin(const ceph::bufferlist& filter) override{
        return get_Io_Ctx().nobjects_begin(filter);
    }
    void set_namespace(const std::string& nspace) override{
        get_Io_Ctx().set_namespace(nspace);
    }
    int stat(const std::string& oid, uint64_t *psize, time_t *pmtime)override{
        int ret=get_Io_Ctx().stat(oid,psize,pmtime);
        psize=nullptr;pmtime=nullptr;
        delete psize;delete pmtime;
        return ret;
    }
    int aio_operate(const std::string& oid, librados::AioCompletion *c, librados::ObjectWriteOperation *op)override{
        int ret=get_Io_Ctx().aio_operate(oid,c,op);
        c=nullptr;op=nullptr;
        delete c;delete op;
        return ret;
    }
    int remove(const std::string& oid){
        return get_Io_Ctx().remove(oid);
    }
    int  write_full(const std::string& oid, librados::bufferlist& bl) override{
        return get_Io_Ctx().write_full(oid,bl);
    }
    void set_Io_Ctx(librados::IoCtx& io_ctx_) override{
        io_ctx=io_ctx_;
    }
    librados::IoCtx& get_Io_Ctx() override{
        return io_ctx;
    }
    int read(const std::string& oid, librados::bufferlist& bl, size_t len, uint64_t off) override{
        return get_Io_Ctx().read(oid,bl,len,0);
    }
    int operate(const std::string& oid, librados::ObjectWriteOperation* write_op_xattr) override{
        int ret=get_Io_Ctx().operate(oid,write_op_xattr);
        write_op_xattr=nullptr;
        delete write_op_xattr;
        return ret;
    }
    bool append(const std::string& oid, librados::bufferlist& bufferlist, int length) override{
        return get_Io_Ctx().append(oid,bufferlist, length);
    }
    int operate(
        const std::string& oid,librados::ObjectReadOperation* read_op,librados::bufferlist* buffer) override{
        int ret=get_Io_Ctx().operate(oid,read_op,buffer);
        read_op=nullptr;buffer=nullptr;
        delete read_op;delete buffer;
        return ret;
    }
    private:
    librados::IoCtx io_ctx;
};    
}
#endif  // SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_