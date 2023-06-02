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

#include <ctime>
#include <rados/librados.hpp>
#include <algorithm>
#include "../../librmb/rados-cluster-impl.h"
#include "../../librmb/rados-storage-impl.h"
#include "mock_test.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../librmb/rados-metadata-storage-default.h"
#include "../../librmb/rados-metadata-storage-ima.h"
#include "../../librmb/rados-dovecot-ceph-cfg-impl.h"
#include "../../librmb/rados-util-impl.h"
#include "../../librmb/tools/rmb/rmb-commands-impl.h"
#include "../../librmb/rados-mail-impl.h"
#include "../../storage-interface/rados-mail.h"
#include "../../storage-engine/storage-backend-factory.h"
#include "../../librmb/rados-save-log-impl.h"
#include "../../storage-interface/rados-save-log.h"
#include "../../librmb/rados-metadata-impl.h"
#include "../../storage-interface/rados-dovecot-ceph-cfg.h"

using ::testing::AtLeast;
using ::testing::Return;

/**
 * Test object split operation
 *
 */
TEST(librmb, split_write_operation) {
  librados::bufferlist bl;
  std::string test_buffer_lenght="it is a test for test buffer end";
  bl.append(test_buffer_lenght.c_str());
  std::cout<<"it is a test for test buffer end"<<test_buffer_lenght.size()<<"buffer lenght::"<<bl.to_str();
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  std::string splitable_buffer="";
  for(int i=0;i<storage.get_max_write_size_bytes();i++){
    splitable_buffer.append("Sara");
  }
  std::cout <<"splitable_buffer.size()::"<< splitable_buffer.size()<< std::endl;
  librmb::RadosMailImpl obj;
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append(splitable_buffer);
  int buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  std::cout <<"buffer_length::"<< buffer_length << std::endl;
  obj.set_mail_size(buffer_length);
  std::cout <<"obj.get_mail_size()::"<< obj.get_mail_size() << std::endl;
  std::cout <<"storage.get_max_object_size()::"<< storage.get_max_object_size() << std::endl;
  std::cout <<"get_max_write_size_bytes()::"<< storage.get_max_write_size_bytes() << std::endl;
  obj.set_oid("test_oid");
  librados::IoCtx io_ctx;
  std::string pool_name("test");
  std::string ns("t");
  std::cout << "open" << std::endl;
  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);
  bool ret_storage=storage.save_mail(&obj);

  // stat the object
  uint64_t size;
  time_t save_date;
  std::cout<<"stat_mail is invoked"<<std::endl;
  int ret_stat = storage.stat_mail(*obj.get_oid(), &size, &save_date);
  std::cout<<"stat_mailhas been invoked "<<std::endl;

  // remove it
  int ret_remove = storage.delete_mail(*obj.get_oid());

  // tear down
  cluster.deinit();

  EXPECT_EQ(buffer_length, size);
  EXPECT_EQ(true, ret_storage);
  EXPECT_EQ(0, ret_stat);
  EXPECT_EQ(0, ret_remove);
  delete obj.get_mail_buffer();
}
/**
 * Test object split operation
 *
 */
TEST(librmb1, split_write_operation_1) {
  librmb::RadosMailImpl obj;
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("HALLO_WELT_");
  size_t buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  obj.set_oid("test_oid");
  std::string pool_name("test");
  std::string ns("t");
  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  bool ret_storage = storage.save_mail(&obj);

  // stat the object
  uint64_t size;
  time_t save_date;
  int ret_stat = storage.stat_mail(*obj.get_oid(), &size, &save_date);

  // remove it
  int ret_remove = storage.delete_mail(*obj.get_oid());

  // tear down.
  cluster.deinit();

  EXPECT_EQ(buffer_length, size);
  EXPECT_EQ(true, ret_storage);
  EXPECT_EQ(0, ret_stat);
  EXPECT_EQ(0, ret_remove);
  delete obj.get_mail_buffer();
}
/**
 * Test Rados Metadata type conversion
 *
 */
TEST(librmb1, convert_types) {
  std::string value = "4441c5339f4c9d59523000009c60b9f7";
  librmb::RadosMetadataImpl attr(storage_interface::RBOX_METADATA_GUID, value);

  EXPECT_EQ(attr.key, "G");
  EXPECT_STREQ((*attr.bl).c_str(), "4441c5339f4c9d59523000009c60b9f7");
  time_t t = 1503488583;

  attr.key = "";
  (*attr.bl).clear();
  librmb::RadosMetadataImpl attr2(storage_interface::RBOX_METADATA_RECEIVED_TIME, t);

  EXPECT_EQ(attr2.key, "R");
  EXPECT_STREQ((*attr2.bl).c_str(), "1503488583");

  // time_t recv_date;
  // attr2.convert(attr2.key, recv_date);
  // EXPECT_EQ(t, recv_date);

  size_t st = 100;
  librmb::RadosMetadataImpl attr4(storage_interface::RBOX_METADATA_VIRTUAL_SIZE, st);
  EXPECT_EQ(attr4.key, "V");
  EXPECT_STREQ((*attr4.bl).c_str(), "100");

  attr4.key = "";
  (*attr4.bl).clear();
}
/**
 * Test Storage read_mail
 *
 */
TEST(librmb1, read_mail) {
  librmb::RadosMailImpl obj;
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  const char *message =
    "From: user@domain.org\n"
    "Date: Sat, 24 Mar 2017 23:00:00 +0200\n"
    "Mime-Version: 1.0\n"
    "Content-Type: text/plain; charset=us-ascii\n"
    "\n"
    "body\n";
  std::stringstream test_buff;
  test_buff<<message;
  ((librados::bufferlist*)obj.get_mail_buffer())->append(message,test_buff.str().length());
  int buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
  obj.set_oid("test_oid");
  std::string pool_name("test");
  std::string ns("t");
  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);
  bool ret_storage = storage.save_mail(&obj);

  // stat the object
  uint64_t size;
  time_t save_date;

  int ret_stat = storage.stat_mail(*obj.get_oid(), &size, &save_date);
  int copy_mail_ret = storage.read_mail(*obj.get_oid(),&obj,0);
  // remove it
  int ret_remove = storage.delete_mail(*obj.get_oid());

  // tear down
  cluster.deinit();
  EXPECT_EQ(ret_storage, true);
  EXPECT_EQ(ret_stat, 0);
  EXPECT_EQ(ret_remove, 0);
  EXPECT_EQ(copy_mail_ret, 0);
  delete obj.get_mail_buffer();
}
/**
 * Test Load Metadata
 *
 */
TEST(librmb, load_metadata) {
  librmb::RadosMailImpl obj;
  librmb::RadosClusterImpl *cluster= new librmb::RadosClusterImpl();
  int ret_cluster= cluster->init();
  if(ret_cluster >= 0){
    std::cout<<"cluster is working";
  }
  librmb::RadosStorageImpl storage(cluster);

  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  obj.set_oid("test_oid");
  std::string pool_name("test");
  std::string ns("t");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);
  bool ret_storage = storage.save_mail(&obj);
  EXPECT_EQ(true, ret_storage);

  // librmb::RadosMetadataStorageDefault ms(storage.get_io_ctx_wrapper());
  storage_interface::RadosStorageMetadataModule *ms = new librmb::RadosMetadataStorageDefault(storage.get_io_ctx_wrapper());
  std::string key="key_1";
  std::string val="val_1";
  storage_interface::RadosMetadata *rados_metadata_1=new librmb::RadosMetadataImpl(key,val);
  std::list<storage_interface::RadosMetadata*> metadata_list;
  metadata_list.push_back(rados_metadata_1);

  key="key_2";
  val="val_2";
  storage_interface::RadosMetadata *rados_metadata_2=new librmb::RadosMetadataImpl(key,val);
  metadata_list.push_back(rados_metadata_2);
  ms->update_metadata(*obj.get_oid(),metadata_list);

  std::cout << " load with null" << std::endl;
  int i = ms->load_metadata(nullptr);
  EXPECT_EQ(-1, i);
  i = ms->load_metadata(&obj);
  EXPECT_EQ(0, i);
  std::cout << "load metadata ok" << std::endl;

  // stat the object
  uint64_t size;
  time_t save_date;
  int ret_stat = storage.stat_mail(*obj.get_oid(), &size, &save_date);

  EXPECT_EQ(buffer_length, size);
  
  EXPECT_EQ(0, ret_stat);
  EXPECT_EQ(2, obj.get_metadata()->size());

  // remove it
  int ret_remove = storage.delete_mail(*obj.get_oid());
  EXPECT_EQ(0, ret_remove);

  // tear down
  cluster->deinit();
  delete cluster;
  ceph::bufferlist *obj_buff= obj.get_mail_buffer();
  delete obj_buff;
  delete rados_metadata_1;
  delete rados_metadata_2;
  delete ms;
}
/**
 * rados object version behavior
 *
 */
TEST(librmb, AttributeVersions) {
  librmb::RadosMailImpl obj;
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  obj.set_oid("test_oid2");
  std::string pool_name("test");
  std::string ns("t");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);
  bool ret_storage = storage.save_mail(&obj);
  EXPECT_EQ(ret_storage, true);

  std::list<storage_interface::RadosMetadata*> metadata_list;
  std::string key="key_1";
  std::string val="val_1";
  librmb::RadosMetadataImpl *rados_metadata_1=new librmb::RadosMetadataImpl(key,val);
  metadata_list.push_back(rados_metadata_1);
  key="key_2";
  val="val_2";
  librmb::RadosMetadataImpl *rados_metadata_2=new librmb::RadosMetadataImpl(key,val);
  metadata_list.push_back(rados_metadata_2);
  uint64_t version = storage.get_io_ctx().get_last_version();
  librmb::RadosMetadataStorageDefault ms(storage.get_io_ctx_wrapper());
  ms.update_metadata(*obj.get_oid(),metadata_list);
  ms.load_metadata(&obj);
  EXPECT_EQ(obj.get_metadata()->size(),2);
  
  // stat the object
  uint64_t size;
  time_t save_date;
  int ret_stat = storage.stat_mail(*obj.get_oid(), &size, &save_date);
  EXPECT_EQ(ret_stat, 0);
  uint64_t version_after_xattr_update = storage.get_io_ctx().get_last_version();
  EXPECT_NE(version, version_after_xattr_update);

  std::map<std::string, librados::bufferlist> map;
  librados::bufferlist omap_bl;
  omap_bl.append("xxx");
  map.insert(std::pair<std::string, librados::bufferlist>(*obj.get_oid(), omap_bl));
  storage.get_io_ctx_wrapper()->omap_set(*obj.get_oid(), map);
  uint64_t version_after_omap_set = storage.get_io_ctx().get_last_version();
  EXPECT_NE(version_after_xattr_update, version_after_omap_set);

  // remove it
  storage.delete_mail(*obj.get_oid());
  // tear down
  cluster.deinit();
  delete obj.get_mail_buffer();
  delete rados_metadata_1;
  delete rados_metadata_2;
}

// standard call order for metadata updates
// 1. save_metadata
// 2. set_metadata (update uid)
TEST(librmb, json_ima) {

  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);
 
  storage_interface::RadosMail *obj=new librmb::RadosMailImpl();
  obj->set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj->get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length =((librados::bufferlist*)obj->get_mail_buffer())->length();
  obj->set_mail_size(buffer_length);
  obj->set_oid("test_ima");

  bool ret_storage = storage.save_mail(obj);
  EXPECT_EQ(ret_storage, true);

  storage_interface::RadosDovecotCephCfg *cfg = new librmb::RadosDovecotCephCfgImpl(storage.get_io_ctx_wrapper());
  librmb::RadosMetadataStorageIma ms(storage.get_io_ctx_wrapper(),cfg);
  unsigned int flags = 0x18;
  long recv_time = 12345677;
  // all attributes are not updateable.
  storage_interface::RadosMetadata *attr = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_GUID, "guid");
  storage_interface::RadosMetadata *attr2 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_OLDV1_FLAGS, flags);
  storage_interface::RadosMetadata *attr3 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_RECEIVED_TIME, recv_time);
  storage_interface::RadosMetadata *attr4 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_VERSION, "0.1");


  obj->add_metadata(attr);
  obj->add_metadata(attr2);
  obj->add_metadata(attr3);
  obj->add_metadata(attr4);
  ms.set_metadata(obj);

  // check
  std::map<std::string, ceph::bufferlist> attr_list;
  storage.get_io_ctx().getxattrs(*obj->get_oid(), attr_list);
  EXPECT_EQ(1, attr_list.size());

  storage.delete_mail(*obj->get_oid());
  // tear down
  cluster.deinit();
  delete obj->get_mail_buffer();
  delete obj;
  delete cfg;
}
// standard call order for metadata updates
// 0. pre-condition: setting flags as updateable
// 1. save_metadata
// 2. set_metadata (update uid)
TEST(librmb, json_ima_2) {
  uint64_t max_size = 3;
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  storage_interface::RadosMail *obj = new librmb::RadosMailImpl();
  obj->set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj->get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length = ((librados::bufferlist*)obj->get_mail_buffer())->length();
  obj->set_mail_size(buffer_length);
  obj->set_oid("test_ima");
  bool ret_storage = storage.save_mail(obj);
  EXPECT_EQ(ret_storage, true);

  storage_interface::RadosDovecotCephCfg *cfg=new librmb::RadosDovecotCephCfgImpl (storage.get_io_ctx_wrapper());
  cfg->set_update_attributes("true");
  cfg->update_updatable_attributes("F");
  librmb::RadosMetadataStorageIma ms(storage.get_io_ctx_wrapper(), cfg);
  unsigned int flags = 0x18;
  long recv_time = 12345677;
  // all attributes are not updateable.
  storage_interface::RadosMetadata *attr = 
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_GUID, "guid");
  storage_interface::RadosMetadata *attr2 =
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_OLDV1_FLAGS, flags);
  storage_interface::RadosMetadata *attr3 =
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_RECEIVED_TIME, recv_time);
  storage_interface::RadosMetadata *attr4 =
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_VERSION, "0.1");

  obj->add_metadata(attr);
  obj->add_metadata(attr2);
  obj->add_metadata(attr3);
  obj->add_metadata(attr4);
  ms.set_metadata(obj);
 
  // check there should be ima and F (Flags)
  std::map<std::string, ceph::bufferlist> attr_list;
  storage.get_io_ctx().getxattrs(*obj->get_oid(), attr_list);
  EXPECT_EQ(2, attr_list.size());

  unsigned int uid = 10;
  storage_interface::RadosMetadata *attr_uid =
   new librmb::RadosMetadataImpl (storage_interface::RBOX_METADATA_MAIL_UID, uid);

  obj->add_metadata(attr_uid);
  ms.set_metadata(obj);

  // check again
  attr_list.clear();
  storage.get_io_ctx().getxattrs(*obj->get_oid(), attr_list);
  EXPECT_EQ(2, attr_list.size());

  storage.delete_mail(*obj->get_oid());
  // tear down
  cluster.deinit();
  delete obj->get_mail_buffer();
  delete cfg;
  delete obj;
}

// standard call order for metadata updates
// 0. pre-condition: setting flags as updateable
// 1. save_metadata with keywords
// 2. set_metadata (update uid)
TEST(librmb, json_ima_3) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  storage_interface::RadosMail *obj= new librmb::RadosMailImpl();
  obj->set_mail_buffer(storage.alloc_mail_buffer());
 ((librados::bufferlist*)obj->get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length = ((librados::bufferlist*)obj->get_mail_buffer())->length();
  obj->set_mail_size(buffer_length);
  obj->set_oid("test_ima");
  bool ret_storage = storage.save_mail(obj);
  EXPECT_EQ(ret_storage, true);

  unsigned int flags = 0x18;
  long recv_time = 12345677;
  // all attributes are not updateable.
  storage_interface::RadosMetadata *attr = 
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_GUID, "guid");
  storage_interface::RadosMetadata *attr2 = 
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_OLDV1_FLAGS, flags);
  storage_interface::RadosMetadata *attr3 = 
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_RECEIVED_TIME, recv_time);
  storage_interface::RadosMetadata *attr4 = 
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_VERSION, "0.1");

  obj->add_metadata(attr);
  obj->add_metadata(attr2);
  obj->add_metadata(attr3);
  obj->add_metadata(attr4);
  
  storage_interface::RadosDovecotCephCfg *cfg=new  librmb::RadosDovecotCephCfgImpl (storage.get_io_ctx_wrapper());
  cfg->set_update_attributes("true");
  cfg->update_updatable_attributes("FK");
  librmb::RadosMetadataStorageIma ms(storage.get_io_ctx_wrapper(), cfg);
  ms.set_metadata(obj);
  std::map<std::string, ceph::bufferlist> attr_list;
  storage.get_io_ctx().getxattrs(*obj->get_oid(), attr_list);
  EXPECT_EQ(2, attr_list.size());
  
  for (int i = 0; i < 10; i++) {
    std::string keyword = std::to_string(i);
    std::string ext_key = "k_" + keyword;
    storage_interface::RadosMetadata *ext_metadata = 
    new librmb::RadosMetadataImpl(ext_key, keyword);
     obj->add_extended_metadata(ext_metadata);
  }

  unsigned int uid = 10;
  storage_interface::RadosMetadata  *attr_uid =
   new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_MAIL_UID, uid);
  obj->add_metadata(attr_uid);
  ms.set_metadata(obj);

  // check again
  attr_list.clear();
  storage.get_io_ctx().getxattrs(*obj->get_oid(), attr_list);
  EXPECT_EQ(2, attr_list.size());

  obj->get_metadata()->clear();
  obj->get_extended_metadata()->clear();
  std::cout << "loading metatadata" << std::endl;
  ms.load_metadata(obj);

  EXPECT_EQ(10, obj->get_extended_metadata()->size());

  storage.delete_mail(*obj->get_oid());
  // tear down
  cluster.deinit();
  delete obj->get_mail_buffer();
  delete cfg;
  delete obj;
}
/**
 * Load metadata with default metadata reader
 */
TEST(librmb, test_default_metadata_load_attributes) {
  uint64_t max_size = 3;
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  storage_interface::RadosMail *obj=new librmb::RadosMailImpl();
  obj->set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj->get_mail_buffer())->append("abcdefghijklmn");
  size_t buffer_length = ((librados::bufferlist*)obj->get_mail_buffer())->length();
  obj->set_mail_size(buffer_length);
  obj->set_oid("test_ima");

  bool ret_storage = storage.save_mail(obj);
  EXPECT_EQ(ret_storage, true);

  unsigned int flags = 0x18;
  long recv_time = 12345677;
  // all attributes are not updateable.
  storage_interface::RadosMetadata *attr = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_GUID, "guid");
  storage_interface::RadosMetadata *attr2 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_OLDV1_FLAGS, flags);
  storage_interface::RadosMetadata *attr3 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_RECEIVED_TIME, recv_time);
  storage_interface::RadosMetadata *attr4 = new librmb::RadosMetadataImpl(storage_interface::RBOX_METADATA_VERSION, "0.1");

  obj->add_metadata(attr);
  obj->add_metadata(attr2);
  obj->add_metadata(attr3);
  obj->add_metadata(attr4);

  for (int i = 0; i < 10; i++) {
    std::string keyword = std::to_string(i);
    std::string ext_key = "k_" + keyword;
    storage_interface::RadosMetadata *ext_metadata = new librmb::RadosMetadataImpl(ext_key, keyword);
    obj->add_extended_metadata(ext_metadata);
  }
  
  storage_interface::RadosDovecotCephCfg *cfg=new librmb::RadosDovecotCephCfgImpl (storage.get_io_ctx_wrapper());
  cfg->set_update_attributes("true");
  cfg->update_updatable_attributes("FK");
  librmb::RadosMetadataStorageDefault ms(storage.get_io_ctx_wrapper());

  ms.set_metadata(obj);

  librmb::RadosMailImpl obj2;
  obj2.set_oid("test_ima");

  int a = ms.load_metadata(&obj2);
  EXPECT_EQ(true, a >= 0);

  storage.delete_mail(*obj->get_oid());
  // tear down
  cluster.deinit();
  delete obj->get_mail_buffer();
  delete cfg;
  delete obj;
}
/**
 * Test LoadMetadata default reader
 */
TEST(librmb, test_default_metadata_load_attributes_obj_no_longer_exist) {
  librados::IoCtx io_ctx;

  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  storage_interface::RadosDovecotCephCfg *cfg=new librmb::RadosDovecotCephCfgImpl (storage.get_io_ctx_wrapper());
  cfg->set_update_attributes("true");
  cfg->update_updatable_attributes("FK");
  librmb::RadosMetadataStorageDefault ms(storage.get_io_ctx_wrapper());

  librmb::RadosMailImpl obj2;
  obj2.set_oid("test_ima1");

  int a = ms.load_metadata(&obj2);
  EXPECT_EQ(-2, a);

  // tear down
  cluster.deinit();
  delete cfg;
}
/**
 * Test Metadata reader with ima reader
 */
TEST(librmb, test_default_metadata_load_attributes_obj_no_longer_exist_ima) {
  librados::IoCtx io_ctx;

  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("test");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  storage_interface::RadosDovecotCephCfg *cfg=new librmb::RadosDovecotCephCfgImpl (storage.get_io_ctx_wrapper());
  cfg->set_update_attributes("true");
  cfg->update_updatable_attributes("FK");
  librmb::RadosMetadataStorageIma ms(storage.get_io_ctx_wrapper(), cfg);

  librmb::RadosMailImpl obj2;
  obj2.set_oid("test_ima1");

  int a = ms.load_metadata(&obj2);
  EXPECT_EQ(-2, a);

  // tear down
  cluster.deinit();
  delete cfg;
}
/**
 * Test osd increment
 */
TEST(librmb, increment_add_to_non_existing_key) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("dictionary");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  std::string key = "my-key";
  librmb::RadosMailImpl obj;
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length =((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj.set_oid("test_oid");

  EXPECT_EQ(true,storage.save_mail(&obj));

  long val = 10;  // value to add
  librmb::RadosUtilsImpl rados_utils;
  int ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  // get the value!
  std::set<std::string> keys;
  std::map<std::string, ceph::bufferlist> omap;
  keys.insert(key);

  ASSERT_EQ(0, storage.get_io_ctx().omap_get_vals_by_keys(*obj.get_oid(), keys, &omap));

  std::map<std::string, ceph::bufferlist>::iterator it = omap.find(key);
  ASSERT_NE(omap.end(), it);

  ceph::bufferlist bl = (*it).second;
  EXPECT_EQ(bl.to_str(), "10");
  storage.delete_mail(*obj.get_oid());
  // tear down
  cluster.deinit();
  delete obj.get_mail_buffer();
}
/**
 * Test osd increment
 */
TEST(librmb, increment_add_to_non_existing_object) {
  librados::IoCtx io_ctx;

  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("dictionary");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  std::string key = "my-key";

  librmb::RadosMailImpl obj2;
  obj2.set_oid("myobject");

  long val = 10;  // value to add
  librmb::RadosUtilsImpl rados_utils;
  int ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  // get the value!
  std::set<std::string> keys;
  std::map<std::string, ceph::bufferlist> omap;
  keys.insert(key);

  ASSERT_EQ(0, storage.get_io_ctx().omap_get_vals_by_keys(*obj2.get_oid(), keys, &omap));

  std::map<std::string, ceph::bufferlist>::iterator it = omap.find(key);
  ASSERT_NE(omap.end(), it);

  ceph::bufferlist bl = (*it).second;

  EXPECT_EQ(bl.to_str(), "20");
  storage.delete_mail(*obj2.get_oid());
  // tear down
  cluster.deinit();
}
/**
 * Test osd increment
 */
TEST(librmb, increment_add_to_existing_key) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("dictionary");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  std::string key = "my-key";
  librmb::RadosMailImpl obj2;
  obj2.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj2.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length = ((librados::bufferlist*)obj2.get_mail_buffer())->length();
  obj2.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj2.set_oid("test_oid");
  EXPECT_EQ(true,storage.save_mail(&obj2));
  long val = 10;  // value to add
  librmb::RadosUtilsImpl rados_utils;
  int ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  // get the value!
  std::set<std::string> keys;
  std::map<std::string, ceph::bufferlist> omap;
  keys.insert(key);

  ASSERT_EQ(0, storage.get_io_ctx().omap_get_vals_by_keys(*obj2.get_oid(), keys, &omap));

  std::map<std::string, ceph::bufferlist>::iterator it = omap.find(key);
  ASSERT_NE(omap.end(), it);

  ceph::bufferlist bl = (*it).second;

  EXPECT_EQ(bl.to_str(), "20");
  storage.delete_mail(*obj2.get_oid());
  // tear down
  cluster.deinit();
  delete obj2.get_mail_buffer();
}

/**
 * Test osd decrement
 */
TEST(librmb, increment_sub_from_existing_key) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("dictionary");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  storage.set_namespace(ns);
  EXPECT_EQ(0, open_connection);

  std::string key = "my-key";
  librmb::RadosMailImpl obj2;
  obj2.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj2.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length = ((librados::bufferlist*)obj2.get_mail_buffer())->length();
  obj2.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj2.set_oid("test_oid");
  EXPECT_EQ(true,storage.save_mail(&obj2));

  long val = 10;  // value to add
  librmb::RadosUtilsImpl rados_utils;
  int ret = rados_utils.osd_add(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, val);
  ASSERT_EQ(0, ret);
  long sub_val = 5;  // value to add
  ret = rados_utils.osd_sub(storage.get_io_ctx_wrapper(), *obj2.get_oid(), key, sub_val);
  // get the value!
  ASSERT_EQ(0, ret);
  std::set<std::string> keys;
  std::map<std::string, ceph::bufferlist> omap;
  keys.insert(key);

  ASSERT_EQ(0, storage.get_io_ctx().omap_get_vals_by_keys(*obj2.get_oid(), keys, &omap));

  std::map<std::string, ceph::bufferlist>::iterator it = omap.find(key);
  ASSERT_NE(omap.end(), it);

  ceph::bufferlist bl = (*it).second;

  EXPECT_EQ(bl.to_str(), "5");
  storage.delete_mail(*obj2.get_oid());
  // tear down
  cluster.deinit();
  delete obj2.get_mail_buffer();
}
/**
 * RmbCommands load objects
 */
TEST(librmb, rmb_load_objects) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  

  EXPECT_EQ(0, open_connection);
  storage_interface::RadosCephConfig *ceph_cfg=
   storage_engine::StorageBackendFactory::create_ceph_config_io(storage_engine::CEPH,storage.get_io_ctx_wrapper());
  EXPECT_EQ(0, ceph_cfg->save_cfg());

  std::map<std::string, std::string> opts;
  opts["pool"] = pool_name;
  opts["namespace"] = ns;
  opts["print_cfg"] = "true";
  opts["cfg_obj"] = ceph_cfg->get_cfg_object_name();

  librmb::RmbCommandsImpl rmb_commands(&storage, &cluster, &opts);

  /* update config
  rmb_commands.configuration(false, ceph_cfg);
  */
  // load metadata info
  std::string uid;
  storage_interface::RadosStorageMetadataModule *ms = rmb_commands.init_metadata_storage_module(ceph_cfg, &uid);
  EXPECT_NE(nullptr, ms);

  storage.set_namespace(ns);
  librmb::RadosMailImpl obj2;
  obj2.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj2.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length = ((librados::bufferlist*)obj2.get_mail_buffer())->length();
  obj2.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj2.set_oid("test_oid");
  storage.save_mail(&obj2);
  std::list<storage_interface::RadosMail *> mail_objects;
  std::string sort_string = "uid";
  // std::set<std::string> find_mails_set =storage.find_mails(nullptr);
  EXPECT_EQ(0, mail_objects.size());
  EXPECT_EQ(0, rmb_commands.load_objects(ms, mail_objects, sort_string));
  // EXPECT_EQ(4,find_mails_set.size());
  EXPECT_EQ(1, mail_objects.size());

  for (std::list<storage_interface::RadosMail *>::iterator it = mail_objects.begin(); it != mail_objects.end(); ++it) {
    storage_interface::RadosMail *obj = *it;
    delete obj;
  }

  storage.delete_mail(*obj2.get_oid());
  std::cout<<"*obj2.get_oid()::"<<*obj2.get_oid()<<std::endl;
  storage.delete_mail(ceph_cfg->get_cfg_object_name());
  std::cout<<"ceph_cfg.get_cfg_object_name()"<<ceph_cfg->get_cfg_object_name()<<std::endl;
  delete ms;
  // tear down
  cluster.deinit();
  delete obj2.get_mail_buffer();
  delete ceph_cfg;
}
/**
 * Test RmbCommands load objects
 */
TEST(librmb, rmb_load_objects_valid_metadata) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);

  EXPECT_EQ(0, open_connection);
  storage_interface::RadosCephConfig *ceph_cfg=
   storage_engine::StorageBackendFactory::create_ceph_config_io(storage_engine::CEPH,storage.get_io_ctx_wrapper());
  EXPECT_EQ(0, ceph_cfg->save_cfg());

  std::map<std::string, std::string> opts;
  opts["pool"] = pool_name;
  opts["namespace"] = ns;
  opts["print_cfg"] = "true";
  opts["cfg_obj"] = ceph_cfg->get_cfg_object_name();

  librmb::RmbCommandsImpl rmb_commands(&storage, &cluster, &opts);

  // load metadata info
  std::string uid;
  storage_interface::RadosStorageMetadataModule *ms = rmb_commands.init_metadata_storage_module(ceph_cfg, &uid);
  EXPECT_NE(nullptr, ms);

  storage.set_namespace(ns);

  storage_interface::RadosMail *obj2 = new librmb::RadosMailImpl();
  obj2->set_mail_buffer(storage.alloc_mail_buffer());
  obj2->set_oid("myobject_valid");
  ((librados::bufferlist*)obj2->get_mail_buffer())->append("hallo_welt");  // make sure obj is not empty.
  obj2->set_mail_size(((librados::bufferlist*)obj2->get_mail_buffer())->length());
  {
    std::string key = "M";
    std::string val = "8eed840764b05359f12718004d2485ee";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "I";
    std::string val = "v0.1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "G";
    std::string val = "8eed840764b05359f12718004d2485ee";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "R";
    std::string val = "1234567";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "S";
    std::string val = "1234561117";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "P";
    std::string val = "1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "O";
    std::string val = "0";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "Z";
    std::string val = "200";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "V";
    std::string val = "250";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "U";
    std::string val = "1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "A";
    std::string val = "";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "F";
    std::string val = "01";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "B";
    std::string val = "DRAFTS";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
 librmb::RadosMetadataImpl *xattr=new librmb::RadosMetadataImpl();
  ms->set_metadata(obj2);
  
  // save complete mail.
  EXPECT_EQ(true, storage.save_mail(obj2));
  std::list<storage_interface::RadosMail *> mail_objects;
  std::string sort_string = "uid";
  EXPECT_EQ(0, rmb_commands.load_objects(ms, mail_objects, sort_string));
  // there needs to be one mail
  EXPECT_EQ(1, mail_objects.size());
  storage.delete_mail(*obj2->get_oid());
  storage.delete_mail(ceph_cfg->get_cfg_object_name());
  for (std::list<storage_interface::RadosMail *>::iterator it = mail_objects.begin(); it != mail_objects.end(); ++it) {
    storage_interface::RadosMail *obj = *it;
    storage.delete_mail(*obj->get_oid());
    delete obj;
  }
  delete ms;
  delete xattr;
  mail_objects.clear();
  // tear down
  cluster.deinit();
  delete obj2->get_mail_buffer();
  delete ceph_cfg;
  delete obj2;
}
/**
 * Test RmbCommands load objects
 */
TEST(librmb, rmb_load_objects_invalid_metadata) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);

  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);

  EXPECT_EQ(0, open_connection);
  storage_interface::RadosCephConfig *ceph_cfg=
   storage_engine::StorageBackendFactory::create_ceph_config_io(storage_engine::CEPH,storage.get_io_ctx_wrapper()); 
  EXPECT_EQ(0, ceph_cfg->save_cfg());

  std::map<std::string, std::string> opts;
  opts["pool"] = pool_name;
  opts["namespace"] = ns;
  opts["print_cfg"] = "true";
  opts["cfg_obj"] = ceph_cfg->get_cfg_object_name();

  librmb::RmbCommandsImpl rmb_commands(&storage, &cluster, &opts);

  // load metadata info
  std::string uid;
  storage_interface::RadosStorageMetadataModule *ms = rmb_commands.init_metadata_storage_module(ceph_cfg, &uid);
  EXPECT_NE(nullptr, ms);

  storage.set_namespace(ns);

  storage_interface::RadosMail *obj2 = new librmb::RadosMailImpl();
  obj2->set_mail_buffer(storage.alloc_mail_buffer());
  obj2->set_oid("myobject_invalid");
  ((librados::bufferlist*)obj2->get_mail_buffer())->append("hallo_welt");  // make sure obj is not empty.
  obj2->set_mail_size(((librados::bufferlist*)obj2->get_mail_buffer())->length());
  {
    std::string key = "M";
    std::string val = "8eed840764b05359f12718004d2485ee";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "I";
    std::string val = "v0.1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "G";
    std::string val = "8eed840764b05359f12718004d2485ee";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "R";
    std::string val = "abnahsijsksisis";  // <-- This should be numeric
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "S";
    std::string val = "1234561117";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "P";
    std::string val = "1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "O";
    std::string val = "0";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "Z";
    std::string val = "200";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "V";
    std::string val = "250";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "U";
    std::string val = "1";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "A";
    std::string val = "";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "F";
    std::string val = "01";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  {
    std::string key = "B";
    std::string val = "DRAFTS";
    storage_interface::RadosMetadata *m = new librmb::RadosMetadataImpl(key, val);
    obj2->add_metadata(m);
  }
  librmb::RadosMetadataImpl *xattr=new librmb::RadosMetadataImpl();
  ms->set_metadata(obj2);
  // save complete mail.
  EXPECT_EQ(true, storage.save_mail(obj2));

  std::list<storage_interface::RadosMail *> mail_objects;
  std::string sort_string = "uid";

  EXPECT_EQ(0, rmb_commands.load_objects(ms, mail_objects, sort_string));
  // no mail
  EXPECT_EQ(1, mail_objects.size());

  for (std::list<storage_interface::RadosMail *>::iterator it = mail_objects.begin(); it != mail_objects.end(); ++it) {
    storage_interface::RadosMail *obj = *it;
    delete obj;
  }

  storage.delete_mail(*obj2->get_oid());
  storage.delete_mail(ceph_cfg->get_cfg_object_name());
  delete ms;
  delete xattr;
  mail_objects.clear();
  // tear down
  cluster.deinit();
  delete obj2->get_mail_buffer();
  delete ceph_cfg;
  delete obj2;
}
/**
 * Test RmbCommands
 */
TEST(librmb, delete_objects_via_rmb_tool_and_save_log_file) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  storage.set_namespace(ns);

  std::string test_file_name = "test1.log";
  storage_interface::RadosSaveLog *log_file = new librmb::RadosSaveLogImpl(test_file_name);
  EXPECT_EQ(true, log_file->open());
  storage_interface::RadosSaveLogEntry *log_entry = new librmb::RadosSaveLogEntryImpl("abc", "t1", "rmb_tool_tests", "save");
  log_file->append(log_entry);
  EXPECT_EQ(true, log_file->close());

  librmb::RadosMailImpl obj;
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length =((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj.set_oid("abc");
  storage.save_mail(&obj);

  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> moved_items;
  storage_interface::RmbCommands *rmb_cmds=storage_engine::StorageBackendFactory::create_rmb_commands_default(
    storage_engine::CEPH);
  EXPECT_EQ(1, rmb_cmds->delete_with_save_log("test1.log", "ceph", "client.admin", &moved_items));
  std::remove(test_file_name.c_str());
  cluster.deinit();
  delete rmb_cmds;

  delete obj.get_mail_buffer();
  for(std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>>::iterator it=moved_items.begin(); it != moved_items.end(); ++it){ 
    for(std::list<storage_interface::RadosSaveLogEntry*>::iterator iter = it->second.begin();iter != it->second.end(); ++iter){
      delete *iter;
      *iter=nullptr;
    }
  }
  delete log_file;
  delete log_entry; 
}
// /**
//  * Test RmbCommands
//  */
TEST(librmb, delete_objects_via_rmb_tool_and_save_log_file_file_not_found) {
  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  std::string test_file_name = "test1.log";
  librmb::RadosSaveLogImpl log_file(test_file_name);
  EXPECT_EQ(true, log_file.open());
  log_file.append(&librmb::RadosSaveLogEntryImpl("abc", "t1", "rmb_tool_tests", "save"));
  EXPECT_EQ(true, log_file.close());
  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> moved_items;
  storage_interface::RmbCommands *rmb_cmds=storage_engine::StorageBackendFactory::create_rmb_commands_default(
    storage_engine::CEPH);
  EXPECT_EQ(0, rmb_cmds->delete_with_save_log("test1.log", "ceph", "client.admin", &moved_items));
  std::remove(test_file_name.c_str());
  delete rmb_cmds;
  for(std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>>::iterator it=moved_items.begin(); it != moved_items.end(); ++it){ 
    for(std::list<storage_interface::RadosSaveLogEntry*>::iterator iter = it->second.begin();iter != it->second.end(); ++iter){
      delete *iter;
      *iter=nullptr;
    }
  }
}
/**
 * Test RmbCommands
 */
TEST(librmb, delete_objects_via_rmb_tool_and_save_log_file_invalid_file) {
  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  std::string test_file_name = "test1.log";
  librmb::RadosSaveLogImpl log_file(test_file_name);
  EXPECT_EQ(true, log_file.open());
  log_file.append(&librmb::RadosSaveLogEntryImpl("abc", "t1", "rmb_tool_tests", "save"));
  EXPECT_EQ(true, log_file.close());
  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> moved_items;
  storage_interface::RmbCommands *rmb_cmds=storage_engine::StorageBackendFactory::create_rmb_commands_default(
    storage_engine::CEPH);
  EXPECT_EQ(-1,rmb_cmds->delete_with_save_log("test12.log", "ceph", "client.admin", &moved_items));
  std::remove(test_file_name.c_str());
  delete rmb_cmds;
  for(std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>>::iterator it=moved_items.begin(); it != moved_items.end(); ++it){ 
    for(std::list<storage_interface::RadosSaveLogEntry*>::iterator iter = it->second.begin();iter != it->second.end(); ++iter){
      delete *iter;
      *iter=nullptr;
    }  
  }
}
/**
 * Test RmbCommands
 */
TEST(librmb, delete_objects_via_rmb_tool_and_save_log_file_invalid_entry) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  storage.set_namespace(ns);

  std::string test_file_name = "test1.log";
  librmb::RadosSaveLogImpl log_file(test_file_name);
  EXPECT_EQ(true, log_file.open());
  log_file.append(
    &librmb::RadosSaveLogEntryImpl("abc2", "t1", "2,2,2rmb_tool_tests", "save"));  // -> stop processing (invalid entry)!
  log_file.append(
    &librmb::RadosSaveLogEntryImpl("abc2", "t1", "rmb_tool_tests", "save"));
  EXPECT_EQ(true, log_file.close());

  librmb::RadosMailImpl obj;
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj.set_oid("abc2");
  EXPECT_EQ(true,storage.save_mail(&obj));
  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> moved_items;
  storage_interface::RmbCommands *rmb_cmds=storage_engine::StorageBackendFactory::create_rmb_commands_default(
    storage_engine::CEPH);
  EXPECT_EQ(0, rmb_cmds->delete_with_save_log("test1.log", "ceph", "client.admin",
                                                         &moved_items));  // -> due to invalid entry in object list
  std::remove(test_file_name.c_str());

  open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  storage.set_namespace(ns);
  EXPECT_EQ(storage.delete_mail("abc2"), 0);  // check that save log processing does stop at invalid line!
  cluster.deinit();
  delete obj.get_mail_buffer();
  delete rmb_cmds;
  for(std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>>::iterator it=moved_items.begin(); it != moved_items.end(); ++it){ 
    for(std::list<storage_interface::RadosSaveLogEntry*>::iterator iter = it->second.begin();iter != it->second.end(); ++iter){
      delete *iter;
      *iter=nullptr;
    }
  }
}
/**
 * Test RmbCommands
 */
TEST(librmb, move_object_delete_with_save_log) {
  librmb::RadosClusterImpl cluster;
  librmb::RadosStorageImpl storage(&cluster);
  std::string pool_name("rmb_tool_tests");
  std::string ns("t1");

  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  storage.set_namespace(ns);
  
  std::string test_file_name = "test1.log";
  librmb::RadosSaveLogImpl log_file(test_file_name);
  EXPECT_EQ(true, log_file.open());
  log_file.append(&librmb::RadosSaveLogEntryImpl("abc3", "t1", "rmb_tool_tests", "mv:t1:abc3:t1;M=123:B=INBOX:U=1:G=0246da2269ac1f5b3e1700009c60b9f7"));
  EXPECT_EQ(true, log_file.close());
  
  librmb::RadosMailImpl obj;
  obj.set_mail_buffer(storage.alloc_mail_buffer());
  ((librados::bufferlist*)obj.get_mail_buffer())->append("abcdefghijklmn");
  int buffer_length = ((librados::bufferlist*)obj.get_mail_buffer())->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj.set_oid("abc3"); 
  EXPECT_EQ(true,storage.save_mail(&obj));  
  
  std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>> moved_items;
  storage_interface::RmbCommands *rmb_cmds=storage_engine::StorageBackendFactory::create_rmb_commands_default(
    storage_engine::CEPH);
  EXPECT_EQ(1, rmb_cmds->delete_with_save_log("test1.log", "ceph", "client.admin", &moved_items));
  std::remove(test_file_name.c_str());
  EXPECT_EQ(1, moved_items.size());
  std::list<storage_interface::RadosSaveLogEntry*> list = moved_items["t1"];
  EXPECT_EQ(1, list.size());
  storage_interface::RadosSaveLogEntry* entry = list.front();

  EXPECT_EQ(entry->get_src_oid(), "abc3");
  EXPECT_EQ(entry->get_src_ns(), "t1");
  EXPECT_EQ(entry->get_src_user(), "t1");
  std::string key_guid(1, static_cast<char>(storage_interface::RBOX_METADATA_GUID));
  std::list<storage_interface::RadosMetadata*>::iterator it_guid =
      std::find_if(entry->get_metadata().begin(), entry->get_metadata().end(),
                   [key_guid](storage_interface::RadosMetadata* const m) { return m->get_key() == key_guid; });
  std::cout<<  ((ceph::bufferlist*)((*it_guid)->get_buffer()))->to_str() <<std::endl;               
  EXPECT_EQ("0246da2269ac1f5b3e1700009c60b9f7", ((ceph::bufferlist*)((*it_guid)->get_buffer()))->to_str());

  open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  storage.set_namespace(ns);
  EXPECT_EQ(storage.delete_mail("abc3"), 0);
  cluster.deinit();
  delete obj.get_mail_buffer();
  delete rmb_cmds;
  for(std::map<std::string, std::list<storage_interface::RadosSaveLogEntry*>>::iterator it=moved_items.begin(); it != moved_items.end(); ++it){ 
    for(std::list<storage_interface::RadosSaveLogEntry*>::iterator iter = it->second.begin();iter != it->second.end(); ++iter){
      delete *iter;
      *iter=nullptr;
    }  
  }

}
TEST(librmb, mock_obj) {}
int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
