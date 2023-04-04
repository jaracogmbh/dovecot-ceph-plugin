#ifndef SRC_STORAGE_RBOX_STORAGE_BACKEND_FACTORY_H_
#define SRC_STORAGE_RBOX_STORAGE_BACKEND_FACTORY_H_
#include "../librmb/rados-cluster.h"
#include "../librmb/rados-cluster-impl.h"
#include "../librmb/rados-storage.h"
#include "../librmb/rados-storage-impl.h"
#include "../librmb/rados-dovecot-ceph-cfg.h"
#include "../librmb/rados-dovecot-ceph-cfg-impl.h"
#include "../librmb/rados-namespace-manager.h"
#include "../librmb/rados-metadata-storage.h"
#include "../librmb/rados-metadata-storage-impl.h"

namespace rbox {
class StorageBackendFactory {
 public:
  enum StorageType { CEPH, S3 };

  static StorageBackendFactory &get_instance() { return instance; }
  StorageBackendFactory(StorageBackendFactory const &) = delete;

  librmb::RadosCluster *create_cluster(StorageType storage_type) {
    librmb::RadosCluster *cluster;
    if (storage_type == CEPH) {
      cluster = new librmb::RadosClusterImpl();
    }
    return cluster;
  }

  librmb::RadosStorage *create_storage(StorageType storage_type, librmb::RadosCluster *cluster) {
    librmb::RadosStorage *storage;
    if (storage_type == CEPH) {
      storage = new librmb::RadosStorageImpl(cluster);
    }
    return storage;
  }

  librmb::RadosDovecotCephCfg *create_dovecot_ceph_cfg(StorageType storage_type, librmb::RadosStorage *storage) {
    librmb::RadosDovecotCephCfg *dovecot_ceph_cfg;
    if (storage_type == CEPH) {
      dovecot_ceph_cfg = new librmb::RadosDovecotCephCfgImpl(&storage->get_io_ctx_wrapper().get_io_ctx());
    }
    return dovecot_ceph_cfg;
  }

  librmb::RadosNamespaceManager *create_namespace_manager(StorageType storage_type,
                                                          librmb::RadosDovecotCephCfg *dovecot_ceph_cfg) {
    librmb::RadosNamespaceManager *name_space_manager;
    if (storage_type == CEPH) {
      name_space_manager = new librmb::RadosNamespaceManager(dovecot_ceph_cfg);
    }
    return name_space_manager;
  }

  librmb::RadosMetadataStorage *create_metadata_storage(StorageType storage_type) {
    librmb::RadosMetadataStorage *metadata_storage;
    if (storage_type == CEPH) {
      metadata_storage = new librmb::RadosMetadataStorageImpl();
    }
    return metadata_storage;
  }

 private:
  StorageBackendFactory() {}
  static StorageBackendFactory instance;
};

StorageBackendFactory StorageBackendFactory::instance;
}  // namespace rbox
#endif  // SRC_STORAGE_RBOX_STORAGE_BACKEND_FACTORY_H_