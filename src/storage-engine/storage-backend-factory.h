#ifndef SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_
#define SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_
#include "../storage-interface/rados-cluster.h"
#include "../librmb/rados-cluster-impl.h"
#include "../storage-interface/rados-storage.h"
#include "../librmb/rados-storage-impl.h"
#include "../storage-interface/rados-dovecot-ceph-cfg.h"
#include "../librmb/rados-dovecot-ceph-cfg-impl.h"
#include "../storage-interface/rados-namespace-manager.h"
#include "../librmb/rados-namespace-manager-impl.h"
#include "../storage-interface/rados-metadata-storage.h"
#include "../librmb/rados-metadata-storage-impl.h"
#include "../storage-interface/rados-mail.h"
#include "../librmb/rados-mail-impl.h"
#include "../storage-interface/rados-ceph-config.h"
#include "../librmb/rados-ceph-config-impl.h"

namespace storage_engine {
class StorageBackendFactory {
 public:
  enum StorageType { CEPH, S3 };

  static storage_interface::RadosCluster *create_cluster(StorageType storage_type) {
    storage_interface::RadosCluster *cluster;
    if (storage_type == CEPH) {
      cluster = new librmb::RadosClusterImpl();
    }
    return cluster;
  }

  static storage_interface::RadosStorage *create_storage(StorageType storage_type, storage_interface::RadosCluster *cluster) {
    storage_interface::RadosStorage *storage;
    if (storage_type == CEPH) {
      storage = new librmb::RadosStorageImpl(cluster);
    }
    return storage;
  }

  static storage_interface::RadosDovecotCephCfg *create_dovecot_ceph_cfg(StorageType storage_type, storage_interface::RadosStorage *storage) {
    storage_interface::RadosDovecotCephCfg *dovecot_ceph_cfg;
    if (storage_type == CEPH) {
      dovecot_ceph_cfg = new librmb::RadosDovecotCephCfgImpl(&storage->get_io_ctx_wrapper().get_io_ctx());
    }
    return dovecot_ceph_cfg;
  }

  static storage_interface::RadosNamespaceManager *create_namespace_manager(StorageType storage_type,
                                                          storage_interface::RadosDovecotCephCfg *dovecot_ceph_cfg) {
    storage_interface::RadosNamespaceManager *name_space_manager;
    if (storage_type == CEPH) {
      name_space_manager = new librmb::RadosNamespaceManagerImpl(dovecot_ceph_cfg);
    }
    return name_space_manager;
  }

  static storage_interface::RadosMetadataStorage *create_metadata_storage(StorageType storage_type) {
    storage_interface::RadosMetadataStorage *metadata_storage;
    if (storage_type == CEPH) {
      metadata_storage = new librmb::RadosMetadataStorageImpl();
    }
    return metadata_storage;
  }

  static storage_interface::RadosMail *create_mail(StorageType storage_type){
    storage_interface::RadosMail *mail;
    if (storage_type == CEPH){
      mail=new librmb::RadosMailImpl();
    }
    return mail;
  }

  static storage_interface::RadosCephConfig *create_ceph_config(StorageType storage_type){
    storage_interface::RadosCephConfig *ceph_config;
    if(storage_type==CEPH){
      ceph_config=new librmb::RadosCephConfigImpl();
    }
    return ceph_config;
  }
};
}  // namespace rbox
#endif  // SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_