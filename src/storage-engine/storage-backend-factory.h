#ifndef SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_
#define SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_
#include<string>
#include<map>

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
#include "../storage-interface/rados-metadata.h"
#include "../librmb/rados-metadata-impl.h"
#include "../librmb/rados-types.h"
#include "../librmb/rados-dovecot-config.h"
#include "../storage-interface/tools/rmb/rmb-commands.h"
#include "../librmb/tools/rmb/rmb-commands-impl.h"
#include "../storage-interface/rados-save-log.h"
#include "../librmb/rados-save-log-impl.h"
#include "../storage-interface/tools/rmb/ls_cmd_parser.h"
#include "../librmb/tools/rmb/ls_cmd_parser_impl.h"
#include "../storage-interface/rados-util.h"
#include "../librmb/rados-util-impl.h"


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

  static storage_interface::RadosDovecotCephCfg *create_dovecot_ceph_cfg_io(StorageType storage_type,librados::IoCtx *io_ctx_) {
    storage_interface::RadosDovecotCephCfg *dovecot_ceph_cfg;
    if (storage_type == CEPH) {
      dovecot_ceph_cfg = new librmb::RadosDovecotCephCfgImpl(io_ctx_);
    }
    return dovecot_ceph_cfg;
  }
  
  static storage_interface::RadosDovecotCephCfg *create_dovecot_ceph_cfg(
    StorageType storage_type, librmb::RadosConfig &dovecot_cfg_, storage_interface::RadosCephConfig *rados_cfg_){
      storage_interface::RadosDovecotCephCfg *dovecot_ceph_cfg;
    if(storage_type == CEPH){
      dovecot_ceph_cfg=new librmb::RadosDovecotCephCfgImpl(dovecot_cfg_, rados_cfg_);
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

  static storage_interface::RadosCephConfig *create_ceph_config_io(StorageType storage_type,librados::IoCtx *io_ctx_){
    storage_interface::RadosCephConfig *ceph_config;
    if(storage_type==CEPH){
      ceph_config=new librmb::RadosCephConfigImpl(io_ctx_);
    }
    return ceph_config;
  }
  
  static storage_interface::RadosMetadata *create_metadata_default(StorageType storage_type){
    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl();
    }
    return metadata;
  }

  static storage_interface::RadosMetadata *create_metadata_str_key_val(
    StorageType storage_type,std::string& key_, std::string& val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(key_,val);
    }
    return metadata;
  }

  static storage_interface::RadosMetadata *create_metadata_string(
    StorageType storage_type,librmb::rbox_metadata_key _key,const std::string& val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }

  static storage_interface::RadosMetadata *create_metadata_time(
    StorageType storage_type,librmb::rbox_metadata_key _key,const time_t& val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }

  static storage_interface::RadosMetadata *create_metadata_char(
    StorageType storage_type,librmb::rbox_metadata_key _key,const char* val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }
  
  static storage_interface::RadosMetadata *create_metadata_uint(
    StorageType storage_type,librmb::rbox_metadata_key _key,const uint& val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }

  static storage_interface::RadosMetadata *create_metadata_size_t(
    StorageType storage_type,librmb::rbox_metadata_key _key,const size_t& val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }
  
  static storage_interface::RadosMetadata *create_metadata_int(
    StorageType storage_type,librmb::rbox_metadata_key _key,const int val){

    storage_interface::RadosMetadata *metadata;
    if(storage_type==CEPH){
      metadata=new librmb::RadosMetadataImpl(_key,val);
    }
    return metadata;
  }
  static storage_interface::RmbCommands *create_rmb_commands(
    StorageType storage_type, storage_interface::RadosStorage *storage_, storage_interface::RadosCluster *cluster_,
      std::map<std::string, std::string> *opts_){

    storage_interface::RmbCommands *rmb_commands;
    if(storage_type==CEPH){
      rmb_commands=new librmb::RmbCommandsImpl(storage_, cluster_, opts_);
    }
    return rmb_commands;    
  }
  static storage_interface::RmbCommands *create_rmb_commands_default(StorageType storage_type){
    storage_interface::RmbCommands *rmb_commands;
    if(storage_type==CEPH){
      rmb_commands=new librmb::RmbCommandsImpl();
    }
    return rmb_commands;
  }

  static storage_interface::RadosSaveLogEntry *create_save_log_entry(
    StorageType storage_type,const std::string &oid_, const std::string &ns_, const std::string &pool_, const std::string &op_){

    storage_interface::RadosSaveLogEntry *save_log_entry;
    if(storage_type==CEPH){
      save_log_entry=new librmb::RadosSaveLogEntryImpl(oid_, ns_, pool_, op_);
    }
    return save_log_entry;
  }

  static storage_interface::RadosSaveLogEntry *create_save_log_entry_default(StorageType storage_type){
    storage_interface::RadosSaveLogEntry *save_log_entry;
    if(storage_type==CEPH){
      save_log_entry=new librmb::RadosSaveLogEntryImpl();
    }
    return save_log_entry;
  }
  
  static storage_interface::RadosSaveLog *create_save_log(StorageType storage_type,const std::string& log_file){
    storage_interface::RadosSaveLog *save_log;
    if(storage_type==CEPH){
      save_log=new librmb::RadosSaveLogImpl(log_file);
    }
    return save_log;
  }
  static storage_interface::RadosSaveLog *create_save_log_default(StorageType storage_type){
    storage_interface::RadosSaveLog *save_log;
    if(storage_type==CEPH){
      save_log=new librmb::RadosSaveLogImpl();
    }
    return save_log;
  }

  static storage_interface::CmdLineParser *create_cmd_line_parser(StorageType storage_type,const std::string &_ls_value){
    storage_interface::CmdLineParser *cmd_line_parser;
    if(storage_type==CEPH){
      cmd_line_parser=new librmb::CmdLineParserImpl(_ls_value);
    }
    return cmd_line_parser;
  }

  static storage_interface::RadosUtils *create_rados_utils(StorageType storage_type){
    storage_interface::RadosUtils *rados_utils;
    if(storage_type==CEPH){
      rados_utils=new librmb::RadosUtilsImpl();
    }
    return rados_utils;
  }
};
}  // namespace interface_engine
#endif  // SRC_STORAGE_ENGINE_STORAGE_BACKEND_FACTORY_H_