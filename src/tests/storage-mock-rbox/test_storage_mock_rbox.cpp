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

#include "../storage-mock-rbox/TestCase.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"           // turn off warnings for Dovecot :-(
#pragma GCC diagnostic ignored "-Wundef"            // turn off warnings for Dovecot :-(
#pragma GCC diagnostic ignored "-Wredundant-decls"  // turn off warnings for Dovecot :-(
#ifndef __cplusplus
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"  // turn off warnings for Dovecot :-(
#endif

extern "C" {
#include "lib.h"
#include "mail-user.h"
#include "mail-storage.h"
#include "mail-storage-service.h"
#include "mail-namespace.h"
#include "mailbox-list.h"
#include "ioloop.h"
#include "istream.h"
#include "mail-search-build.h"
#include "ostream.h"
#include "libdict-rados-plugin.h"
}

#include <iostream>
#include "dovecot-ceph-plugin-config.h"
#include "../test-utils/it_utils.h"

#include "rbox-storage.hpp"
#include "rbox-save.h"
#include "rbox-mail.h"
#include "../mocks/mock_test.h"
#include "rados-dovecot-ceph-cfg-impl.h"
#include "../../storage-rbox/istream-bufferlist.h"
#include "../../storage-rbox/ostream-bufferlist.h"
#include "../../storage-engine/storage-backend-factory.h"
#include "../../storage-interface/rados-mail.h"
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::DoAll;
using ::testing::Assign;
using ::testing::Lt;
using ::testing::NiceMock;
#pragma GCC diagnostic pop

#if DOVECOT_PREREQ(2, 3)
#define mailbox_get_last_internal_error(box, error_r) mailbox_get_last_internal_error(box, error_r)
#else
#define mailbox_get_last_internal_error(box, error_r) mailbox_get_last_error(box, error_r)
#endif

#ifndef i_zero
#define i_zero(p) memset(p, 0, sizeof(*(p)))
#endif

TEST_F(StorageTest, init) {}

// test storage class, cluster not initialized
TEST_F(StorageTest, connect_failed_test) {

  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  /* configure behavior of the mock */
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(-1));

  librmb::RadosStorageImpl under_test(cluster_mock);

  std::string pool_name("test");

  int open_connection = under_test.open_connection(pool_name);
  
  EXPECT_EQ(-1, open_connection);

  delete cluster_mock;
  cluster_mock=nullptr;
}

// test storage class, cluster not initialized
TEST_F(StorageTest, connect_io_ctx_cant_be_created_test) {

  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  /* configure behavior of the mock */
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(-1));
  librmb::RadosStorageImpl under_test(cluster_mock);

  std::string pool_name("test");

  int open_connection = under_test.open_connection(pool_name);
  
  EXPECT_EQ(-1, open_connection);

  delete cluster_mock;
  cluster_mock=nullptr;
}

// test storage class, cluster not initialized
TEST_F(StorageTest, connect_io_ctx_recovery_index_io_ctx_failed) {

  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  /* configure behavior of the mock */
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(-1));
  librmb::RadosStorageImpl under_test(cluster_mock);

  std::string pool_name("test");
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(-1, open_connection);

  delete cluster_mock;
  cluster_mock=nullptr;
}
TEST_F(StorageTest,false_io_ctx_created){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);
  
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0)); 
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(-1));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(0);
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(0);

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(-1, open_connection);
  delete cluster_mock;
  cluster_mock=nullptr;
}
TEST_F(StorageTest,first_cluster_connectio){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);


  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  
  EXPECT_CALL(*cluster_mock, is_connected()).Times(1).WillOnce(Return(false));

  std::string buffer_text="simple_test_one_chunck";
  storage_interface::RadosMail *rados_mail=
    storage_engine::StorageBackendFactory::create_mail(storage_engine::StorageBackendFactory::CEPH);
  void *buffer=(void*)new librados::bufferlist();
  rados_mail->set_mail_buffer(buffer);
  ((librados::bufferlist*)rados_mail->get_mail_buffer())->append(buffer_text);
  int buffer_length = ((librados::bufferlist*)rados_mail->get_mail_buffer())->length();
  rados_mail->set_mail_size(buffer_length);  
  rados_mail->set_oid("test_mail_id"); 

  /*div==1*/ 
  bool ret_storage = under_test.save_mail(rados_mail);
  EXPECT_EQ(false,ret_storage);
  delete cluster_mock;
  cluster_mock=nullptr;
  delete buffer;
  delete rados_mail;
  rados_mail=nullptr;
}
TEST_F(StorageTest,second_cluster_connectio){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);

  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

  librmbtest::RboxIoCtxMock *io_ctx_mock=new librmbtest::RboxIoCtxMock();
  under_test.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  librmb::RadosMetadataStorageDefault rados_metadata_storage (*io_ctx_mock);
  rados_metadata_storage.set_io_ctx(*io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,append(_,_,_)).Times(0);
  EXPECT_CALL(*io_ctx_mock,operate(_,_)).Times(0).WillOnce(Return(0));
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx)); 

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  EXPECT_CALL(*cluster_mock, is_connected()).Times(2).WillOnce(Return(true))
                                                     .WillOnce(Return(false));

  std::string buffer_text="simple_test_one_chunck";
  librmb::RadosMailImpl rados_mail;
  void *buffer=(void*)new librados::bufferlist();
  rados_mail.set_mail_buffer(buffer);
  ((librados::bufferlist*)rados_mail.get_mail_buffer())->append(buffer_text);
  int buffer_length =((librados::bufferlist*)rados_mail.get_mail_buffer())->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 

  /*div==1*/ 
  
  bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(false,ret_storage);
  delete cluster_mock;
  cluster_mock=nullptr;
  delete buffer;
}
TEST_F(StorageTest,true_cluster_connection){

  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));   
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

  librmbtest::RboxIoCtxMock *io_ctx_mock=new librmbtest::RboxIoCtxMock();
  under_test.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  librmb::RadosMetadataStorageDefault rados_metadata_storage (*io_ctx_mock);
  rados_metadata_storage.set_io_ctx(*io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,append(_,_,_)).Times(0);
  EXPECT_CALL(*io_ctx_mock,operate(_,_)).Times(1).WillOnce(Return(0));
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx)); 

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection); 

  EXPECT_CALL(*cluster_mock, is_connected()).Times(2).WillOnce(Return(true))
                                                     .WillOnce(Return(true));
                                                     
 

  std::string buffer_text="simple_test_one_chunck";
  storage_interface::RadosMail *rados_mail=
    storage_engine::StorageBackendFactory::create_mail(storage_engine::StorageBackendFactory::CEPH);
  void *buffer=(void*)new librados::bufferlist();
  rados_mail->set_mail_buffer(buffer);
  ((librados::bufferlist*)rados_mail->get_mail_buffer())->append(buffer_text);
  int buffer_length = ((librados::bufferlist*)rados_mail->get_mail_buffer())->length();
  rados_mail->set_mail_size(buffer_length);  
  rados_mail->set_oid("test_mail_id"); 
 /*div==1*/ 
  bool ret_storage = under_test.save_mail(rados_mail);
  EXPECT_EQ(true,ret_storage);
  delete cluster_mock;
  cluster_mock=nullptr;
  delete buffer;
  delete rados_mail;
  rados_mail=nullptr;
}
TEST_F(StorageTest,split_buffer){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);

  NiceMock<librmbtest::RboxIoCtxMock>* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  under_test.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  librmb::RadosMetadataStorageDefault rados_metadata_storage (*io_ctx_mock);
  rados_metadata_storage.set_io_ctx(*io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,append(_,_,_)).Times(4).WillRepeatedly(Return(true));
  EXPECT_CALL(*io_ctx_mock,operate(_,_)).Times(0).WillOnce(Return(0));
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx)); 
  
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

  std::string pool_name("test");
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);
  EXPECT_CALL(*cluster_mock, is_connected()).Times(5).WillRepeatedly(Return(true));

  std::string buffer_text="";
  for(int i=0;i < under_test.get_max_write_size_bytes();i++){
      buffer_text.append("Sara");
  }
  librmb::RadosMailImpl rados_mail;
  void *buffer=(void*)new librados::bufferlist();
  rados_mail.set_mail_buffer(buffer);
  ((librados::bufferlist*)rados_mail.get_mail_buffer())->append(buffer_text);
  int buffer_length =((librados::bufferlist*)rados_mail.get_mail_buffer())->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 

  /*div==4*/ 
  bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(true,ret_storage);
  delete cluster_mock;
  cluster_mock=nullptr;
  delete buffer;
}
TEST_F(StorageTest,true_first_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));
  EXPECT_CALL(*cluster_mock, is_connected()).Times(1).WillOnce(Return(true));

  std::string pool_name("test");
  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);

  librmb::RadosMailImpl mail;
  const std::string& oid="read_test";
  mail.set_oid(oid);
  mail.set_mail_buffer(new std::stringstream);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx));
  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(1).WillOnce(Return(0));
  int ret=storage.read_mail(oid,&mail,0);
  EXPECT_EQ(0,ret);
  delete cluster_mock;
  cluster_mock=nullptr;
}
TEST_F(StorageTest,true_repeated_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));
  EXPECT_CALL(*cluster_mock, is_connected()).Times(4).WillRepeatedly(Return(true));

  std::string pool_name("test");
  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);

  librmb::RadosMailImpl mail;
  const std::string& oid="read_test";
  mail.set_oid(oid);
  mail.set_mail_buffer(new std::stringstream);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx));
  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(4).WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(0));
  int ret=storage.read_mail(oid,&mail,0);
  EXPECT_EQ(0,ret);
  delete cluster_mock;
  cluster_mock=nullptr;
}
TEST_F(StorageTest,false_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));
  EXPECT_CALL(*cluster_mock, is_connected()).Times(11).WillRepeatedly(Return(true));

  std::string pool_name("test");
  int open_connection = storage.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);

  librmb::RadosMailImpl mail;
  const std::string& oid="read_test";
  mail.set_oid(oid);
  mail.set_mail_buffer(new std::stringstream);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx_wrapper(io_ctx_mock);
  librados::IoCtx io_ctx;
  ON_CALL(*io_ctx_mock,get_io_ctx()).WillByDefault(ReturnRef(io_ctx));
  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(11).WillRepeatedly(Return(-ETIMEDOUT));
  int ret=storage.read_mail(oid,&mail,0);
  EXPECT_NE(0,ret);
  delete cluster_mock;
  cluster_mock=nullptr;
}

TEST_F(StorageTest, deinit) {}

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}