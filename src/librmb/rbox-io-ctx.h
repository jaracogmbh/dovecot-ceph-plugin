  
  
#ifndef SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_
#define SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_  

#include <string>
#include <rados/librados.hpp>

namespace librmb{
class RboxIoCtx{
    public:
      virtual ~RboxIoCtx() {}
      virtual int aio_stat(const std::string& oid,librados::AioCompletion *aio_complete,uint64_t *psize,time_t *pmtime)=0;
      virtual int omap_get_vals_by_keys(const std::string& oid,const std::set<std::string>& keys,
                                          std::map<std::string, librados::bufferlist> *vals)=0;
      virtual int omap_rm_keys(const std::string& oid,const std::set<std::string>& keys)=0;
      virtual int omap_set(const std::map<std::string, librados::bufferlist>& map,const std::string& oid)=0;
      virtual void omap_set(const std::string& oid,const std::map<std::string, librados::bufferlist>& map)=0;
      virtual int getxattrs(const std::string& oid,std::map<std::string, librados::bufferlist>& attrset)=0;
      virtual int setxattr(const std::string& oid,const char *name, librados::bufferlist& bl)=0;
      virtual const librados::NObjectIterator& nobjects_end()=0;
      virtual librados::NObjectIterator nobjects_begin()=0;
      virtual librados::NObjectIterator nobjects_begin(const librados::bufferlist& filter)=0;
      virtual void set_namespace(const std::string& nspace)=0;
      virtual int stat(const std::string& oid, uint64_t *psize, time_t *pmtime)=0;
      virtual int aio_operate(const std::string& oid, librados::AioCompletion *aio_complete, librados::ObjectWriteOperation *op)=0;
      virtual int remove(const std::string& oid)=0;
      virtual int write_full(const std::string& oid, librados::bufferlist& bl)=0;
      virtual void set_Io_Ctx(librados::IoCtx& io_ctx_)=0;
      virtual librados::IoCtx& get_io_ctx()=0;
      virtual int read(const std::string& oid, librados::bufferlist& bl, size_t len, uint64_t off)=0;
      virtual int operate(const std::string& oid, librados::ObjectWriteOperation* write_op_xattr)=0;
      virtual bool append(const std::string& oid, librados::bufferlist& bufferlist, int length)=0;
      virtual int operate(const std::string& oid,librados::ObjectReadOperation* read_op,librados::bufferlist* buffer)=0;
      virtual uint64_t get_last_version()=0;
  };
}  
#endif  // SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_ 