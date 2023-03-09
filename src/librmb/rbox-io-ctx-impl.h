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
    int aio_stat(const std::string& oid,librados::AioCompletion *aio_complete,uint64_t *psize,time_t *pmtime) override{
        return get_io_ctx().aio_stat(oid,aio_complete,psize,pmtime);
    }
    int omap_get_vals_by_keys(const std::string& oid,const std::set<std::string>& keys,std::map<std::string, librados::bufferlist> *vals)override{
        return get_io_ctx().omap_get_vals_by_keys(oid,keys,vals);
    }
    int omap_rm_keys(const std::string& oid,const std::set<std::string>& keys)override{
        return get_io_ctx().omap_rm_keys(oid,keys);
    }
    int  omap_set(const std::map<std::string, librados::bufferlist>& map,const std::string& oid)override{
        get_io_ctx().omap_set(oid,map);
    }
    void omap_set(const std::string& oid,const std::map<std::string, librados::bufferlist>& map)override{
        get_io_ctx().omap_set(oid,map);
    }
    int setxattr(const std::string& oid,const char *name, librados::bufferlist& bl)override{
        return get_io_ctx().setxattr(oid,name,bl);
    }
    int getxattrs(const std::string& oid,std::map<std::string, librados::bufferlist>& attrset)override{
        return get_io_ctx().getxattrs(oid,attrset);
    }
    const librados::NObjectIterator& nobjects_end() override{
        return get_io_ctx().nobjects_end();
    }
    librados::NObjectIterator nobjects_begin() override{
        return get_io_ctx().nobjects_begin();
    }
    librados::NObjectIterator nobjects_begin(const librados::bufferlist& filter) override{
        return get_io_ctx().nobjects_begin(filter);
    }
    void set_namespace(const std::string& nspace) override{
        get_io_ctx().set_namespace(nspace);
    }
    int stat(const std::string& oid, uint64_t *psize, time_t *pmtime)override{
        return get_io_ctx().stat(oid,psize,pmtime);
    }
    int aio_operate(const std::string& oid, librados::AioCompletion *aio_complete, librados::ObjectWriteOperation *op)override{
        return get_io_ctx().aio_operate(oid,aio_complete,op);
    }
    int remove(const std::string& oid){
        return get_io_ctx().remove(oid);
    }
    int  write_full(const std::string& oid, librados::bufferlist& bl) override{
        return get_io_ctx().write_full(oid,bl);
    }
    void set_Io_Ctx(librados::IoCtx& io_ctx_) override{
        io_ctx=io_ctx_;
    }
    librados::IoCtx& get_io_ctx() override{
        return io_ctx;
    }
    int read(const std::string& oid, librados::bufferlist& bl, size_t len, uint64_t off) override{
        return get_io_ctx().read(oid,bl,len,0);
    }
    int operate(const std::string& oid, librados::ObjectWriteOperation* write_op_xattr) override{
        return get_io_ctx().operate(oid,write_op_xattr);
    }
    bool append(const std::string& oid, librados::bufferlist& bufferlist, int length) override{
        return get_io_ctx().append(oid,bufferlist, length);
    }
    int operate(
        const std::string& oid,librados::ObjectReadOperation* read_op,librados::bufferlist* buffer) override{
        return get_io_ctx().operate(oid,read_op,buffer);
    }
    uint64_t get_last_version(){
        return get_io_ctx().get_last_version();
    }
    private:
    librados::IoCtx io_ctx;
};    
}
#endif  // SRC_LIBRMB_RBOX_IO_CTX_IMPL_H_