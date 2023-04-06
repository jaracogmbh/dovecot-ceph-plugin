#ifndef SRC_STORAGE_INTERFACES_RADOS_NAMESPACE_MANAGER_H_
#define SRC_STORAGE_INTERFACES_RADOS_NAMESPACE_MANAGER_H_
#include <map>
#include <string>
#include "rados-storage.h"
#include "rados-dovecot-ceph-cfg.h"
#include "rados-guid-generator.h"
namespace storage_interface
{
class RadosNamespaceManager {
 public:
  virtual ~RadosNamespaceManager(){}
  virtual void set_config(storage_interface::RadosDovecotCephCfg *config_) = 0;
  virtual storage_interface::RadosDovecotCephCfg *get_config() = 0;

  virtual void set_namespace_oid(std::string &namespace_oid_) = 0;
  virtual bool lookup_key(const std::string &uid, std::string *value) = 0;
  virtual bool add_namespace_entry(const std::string &uid, std::string *value, storage_interface::RadosGuidGenerator *guid_generator_) = 0;
};

} // namespace storage_interface
#endif SRC_STORAGE_INTERFACES_RADOS_NAMESPACE_MANAGER_H_