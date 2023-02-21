  
  
#ifndef SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_
#define SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_  

#include <string>
#include <rados/librados.hpp>


namespace librmb{
class RboxIoCtx{
    public:
      virtual ~RboxIoCtx() {}
      virtual librados::NObjectIterator nobjects_begin()=0;
      virtual librados::NObjectIterator nobjects_begin(const ceph::bufferlist& filter)=0;
      virtual void set_namespace(const std::string& nspace)=0;
      virtual int stat(const std::string& oid, uint64_t *psize, time_t *pmtime)=0;
      virtual int aio_operate(const std::string& oid, librados::AioCompletion *c, librados::ObjectWriteOperation *op)=0;
      virtual int remove(const std::string& oid)=0;
      virtual int write_full(const std::string& oid, librados::bufferlist& bl)=0;
      virtual void set_Io_Ctx(librados::IoCtx& io_ctx_)=0;
      virtual librados::IoCtx& get_Io_Ctx()=0;
      virtual int read(const std::string& oid, librados::bufferlist& bl, size_t len, uint64_t off)=0;
      virtual int operate(const std::string& oid, librados::ObjectWriteOperation* write_op_xattr)=0;
      virtual bool append(const std::string& oid, librados::bufferlist& bufferlist, int length)=0;
      virtual int operate(const std::string& oid,librados::ObjectReadOperation* read_op,librados::bufferlist* buffer)=0;
  };
}  
#endif  // SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_ 