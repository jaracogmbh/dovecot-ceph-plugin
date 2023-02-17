  
  
#ifndef SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_
#define SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_  

#include <string>
#include <rados/librados.hpp>


namespace librmb{
class RboxIoCtx{
    public:
      virtual ~RboxIoCtx() {}
      virtual void set_Io_Ctx(librados::IoCtx* storage_io_ctx)=0;
      virtual librados::IoCtx* get_Io_Ctx()=0;
      virtual int read(std::string* oid, librados::bufferlist* bl, size_t len, uint64_t off)=0;
      virtual int operate(std::string* oid, librados::ObjectWriteOperation *write_op_xattr)=0;
      virtual bool append(std::string* oid, librados::bufferlist &bufferlist, int length)=0;
      virtual int operate(std::string* oid,librados::ObjectReadOperation *read_op,librados::bufferlist* buffer)=0;
  };
}  
#endif  // SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_ 