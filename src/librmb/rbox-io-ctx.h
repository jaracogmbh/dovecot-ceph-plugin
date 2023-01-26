  
  
#ifndef SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_
#define SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_  

#include <string>
#include <rados/librados.hpp>


namespace librmb{
class RboxIoCtx{
    public:
      virtual ~RboxIoCtx() {}
      virtual int operate(std::string &oid, librados::ObjectWriteOperation *write_op_xattr)=0;
      virtual bool append(std::string &oid, librados::bufferlist &bufferlist, int length)=0;
      virtual int operate(const std::string &oid,librados::ObjectReadOperation *read_op,librados::bufferlist* buffer)=0;
  };
}  
#endif  // SRC_LIBRMB_INTERFACE_RBOX_IO_CTX_H_ 