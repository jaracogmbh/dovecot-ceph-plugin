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
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::DoAll;
using ::testing::Assign;
using ::testing::Lt;
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

<<<<<<< HEAD
TEST_F(StorageTest,true_io_ctx_created){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);

  /*if io_ctx_created is false the expected value of save_mail is false,
   io_ctx_created gets value on create_connection method 
   and will be true if all following methods return 0 otherwise it will be false */ 
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0)); 
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));
=======
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
}
/*
 //make sure is deprecated uid is always false initialy   
TEST_F(StorageTest, split_buffer) {

//simple test for save method:: the buffer must be splited 
 librmbtest::RadosClusterMock* cluster_mock;
librmb::RadosStorageImpl under_test(cluster_mock);

std::string pool_name("test");
std::string ns("t1");

int open_connection = under_test.open_connection(pool_name);
under_test.set_namespace(ns);
EXPECT_EQ(0, open_connection);

std::string buffer_text="";
for(int i=0;i < under_test.get_max_write_size_bytes();i++){
    buffer_text.append("Sara");
}

librmb::RadosMail rados_mail;
librados::bufferlist buffer;
rados_mail.set_mail_buffer(&buffer);
rados_mail.get_mail_buffer()->append(buffer_text);
int buffer_length = rados_mail.get_mail_buffer()->length();
rados_mail.set_mail_size(buffer_length);
rados_mail.set_oid("test_mail_id");  

// bool ret_storage = under_test.save_mail(&rados_mail);
// EXPECT_EQ(true,ret_storage);
// under_test.delete_mail(&rados_mail);
//test save metada
librmb::RadosDovecotCephCfgImpl cfg(&under_test.get_io_ctx());
librmb::RadosMetadataStorageIma ms(&under_test.get_io_ctx(), &cfg);
unsigned int flags = 0x18;
long recv_time = 12345677;
librmb::RadosMetadata attr(librmb::RBOX_METADATA_GUID, "guid");
librmb::RadosMetadata attr2(librmb::RBOX_METADATA_OLDV1_FLAGS, flags);
librmb::RadosMetadata attr3(librmb::RBOX_METADATA_RECEIVED_TIME, recv_time);
librmb::RadosMetadata attr4(librmb::RBOX_METADATA_VERSION, "0.1");

rados_mail.add_metadata(attr);
rados_mail.add_metadata(attr2);
rados_mail.add_metadata(attr3);
rados_mail.add_metadata(attr4);

bool ret_storage = under_test.save_mail(&rados_mail);
EXPECT_EQ(ret_storage, true);

std::map<std::string, ceph::bufferlist> attr_list;
under_test.get_io_ctx().getxattrs(*rados_mail.get_oid(), attr_list);
EXPECT_EQ(4, attr_list.size());

unsigned int uid = 10;
librmb::RadosMetadata attr_uid(librmb::RBOX_METADATA_MAIL_UID, uid);
ms.set_metadata(&rados_mail, attr_uid);

  // check again
attr_list.clear();
under_test.get_io_ctx().getxattrs(*rados_mail.get_oid(), attr_list);
EXPECT_EQ(5, attr_list.size());
under_test.delete_mail(&rados_mail);
}

TEST_F(StorageTest, one_chunck_buffer_save){
librmbtest::RadosCluster* cluster_mock;
librmb::RadosStorageImpl under_test(cluster_mock);

std::string pool_name("test");
std::string ns("t1");

int open_connection = under_test.open_connection(pool_name);
under_test.set_namespace(ns);
EXPECT_EQ(0, open_connection);

std::string buffer_text="simple_test_one_chunck";
librmb::RadosMail rados_mail;
librados::bufferlist buffer;
rados_mail.set_mail_buffer(&buffer);
rados_mail.get_mail_buffer()->append(buffer_text);
int buffer_length = rados_mail.get_mail_buffer()->length();
rados_mail.set_mail_size(buffer_length);
rados_mail.set_oid("test_mail_id");  

bool ret_storage = under_test.save_mail(&rados_mail);
EXPECT_EQ(true,ret_storage);
under_test.delete_mail(&rados_mail);
}

TEST_F(StorageTest,read_mail){
  librmb::RadosMail obj;
  librados::bufferlist buffer;
  obj.set_mail_buffer(&buffer);
  
  obj.get_mail_buffer()->append("abcdefghijklmn");
  int buffer_length = obj.get_mail_buffer()->length();
  obj.set_mail_size(buffer_length);
  int max_size = buffer_length;
 
  obj.set_oid("test_oid");
  librados::IoCtx io_ctx;

  librmbtest::RadosClusterMock cluster;
  librmb::RadosStorageImpl storage(&cluster);
>>>>>>> 3bc2758da16d8fb8f5197818a1286e9eabf7bfa1

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  librados::IoCtx *io_ctx=&(under_test.get_io_ctx());
  EXPECT_NE(io_ctx,nullptr);
  EXPECT_EQ(0, open_connection);
  delete cluster_mock;

}

TEST_F(StorageTest,false_io_ctx_created){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);


  /*if io_ctx_created is false the expected value of save_mail is false,
   io_ctx_created gets value on create_connection method 
   and will be true if all following methods return 0 otherwise it will be false */ 
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0)); 
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(-1));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(0);
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(0);

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(-1, open_connection);
  librados::IoCtx *io_ctx=&(under_test.get_io_ctx());
  EXPECT_NE(io_ctx,nullptr);
  delete cluster_mock;
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
  librados::IoCtx *io_ctx=&(under_test.get_io_ctx());
  EXPECT_NE(io_ctx,nullptr);
  
  EXPECT_CALL(*cluster_mock, is_connected()).Times(1).WillOnce(Return(false));

  std::string buffer_text="simple_test_one_chunck";
  librmb::RadosMail rados_mail;
  librados::bufferlist buffer;
  rados_mail.set_mail_buffer(&buffer);
  rados_mail.get_mail_buffer()->append(buffer_text);
  int buffer_length = rados_mail.get_mail_buffer()->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 

  /*div==1*/ 
  bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(false,ret_storage);
  delete cluster_mock;
}
TEST_F(StorageTest,second_cluster_connectio){
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
  librados::IoCtx *io_ctx=&(under_test.get_io_ctx());
  EXPECT_NE(io_ctx,nullptr);
  EXPECT_CALL(*cluster_mock, is_connected()).Times(2).WillOnce(Return(true))
                                                     .WillOnce(Return(false));

  std::string buffer_text="simple_test_one_chunck";
  librmb::RadosMail rados_mail;
  librados::bufferlist buffer;
  rados_mail.set_mail_buffer(&buffer);
  rados_mail.get_mail_buffer()->append(buffer_text);
  int buffer_length = rados_mail.get_mail_buffer()->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 

  /*div==1*/ 
  
  bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(false,ret_storage);
  delete cluster_mock;

}
TEST_F(StorageTest,true_cluster_connectio){

  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  under_test.set_io_ctx(io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,append(_,_,_)).Times(0);
  EXPECT_CALL(*io_ctx_mock,operate(_,_)).Times(1).WillOnce(Return(0)); 
 
  
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));   
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

 
  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection); 

  EXPECT_CALL(*cluster_mock, is_connected()).Times(3).WillOnce(Return(true))
                                                     .WillOnce(Return(true))
                                                     .WillOnce(Return(false));
 

  std::string buffer_text="simple_test_one_chunck";
  librmb::RadosMail rados_mail;
  librados::bufferlist buffer;
  rados_mail.set_mail_buffer(&buffer);
  rados_mail.get_mail_buffer()->append(buffer_text);
  int buffer_length = rados_mail.get_mail_buffer()->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 
 /*div==1*/ 
 bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(false,ret_storage);
  delete cluster_mock;
  delete io_ctx_mock;
}
TEST_F(StorageTest,split_buffer){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl under_test(cluster_mock);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  under_test.set_io_ctx(io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,append(_,_,_)).Times(4).WillRepeatedly(Return(true));
  EXPECT_CALL(*io_ctx_mock,operate(_,_)).Times(1).WillOnce(Return(0));
  
  EXPECT_CALL(*cluster_mock, init()).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, io_ctx_create(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, recovery_index_io_ctx(_ , _)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*cluster_mock, get_config_option(_ , _)).Times(2).WillRepeatedly(Return(0));

  std::string pool_name("test");
  /*create_connection is invoked by open_connection method*/
  int open_connection = under_test.open_connection(pool_name);
  EXPECT_EQ(0, open_connection);

  
  EXPECT_CALL(*cluster_mock, is_connected()).Times(6).WillRepeatedly(Return(true));

  std::string buffer_text="";
  for(int i=0;i < under_test.get_max_write_size_bytes();i++){
      buffer_text.append("Sara");
  }
  librmb::RadosMail rados_mail;
  librados::bufferlist buffer;
  rados_mail.set_mail_buffer(&buffer);
  rados_mail.get_mail_buffer()->append(buffer_text);
  int buffer_length = rados_mail.get_mail_buffer()->length();
  rados_mail.set_mail_size(buffer_length);  
  rados_mail.set_oid("test_mail_id"); 

  /*div==4*/ 
 
  bool ret_storage = under_test.save_mail(&rados_mail);
  EXPECT_EQ(true,ret_storage);
  delete cluster_mock;
  delete io_ctx_mock;
}
TEST_F(StorageTest,true_first_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  librmb::RadosMail* mail=new librmb::RadosMail();
  const std::string& oid="read_test";
  mail->set_oid(oid);
  mail->set_mail_buffer(new librados::bufferlist);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx(io_ctx_mock);
  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(1).WillOnce(Return(0));
  int ret=storage.read_mail(oid,mail,0);
  EXPECT_EQ(0,ret);
  delete io_ctx_mock;
}
TEST_F(StorageTest,true_repeated_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  librmb::RadosMail* mail=new librmb::RadosMail();
  const std::string& oid="read_test";
  mail->set_oid(oid);
  mail->set_mail_buffer(new librados::bufferlist);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx(io_ctx_mock);

  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(4).WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(-ETIMEDOUT))
                                                   .WillOnce(Return(0));
  int ret=storage.read_mail(oid,mail,0);
  EXPECT_EQ(0,ret);
  delete io_ctx_mock;
}
TEST_F(StorageTest,false_read){
  librmbtest::RadosClusterMock *cluster_mock = new librmbtest::RadosClusterMock();
  librmb::RadosStorageImpl storage(cluster_mock);
  librmb::RadosMail* mail=new librmb::RadosMail();
  const std::string& oid="read_test";
  mail->set_oid(oid);
  mail->set_mail_buffer(new librados::bufferlist);

  librmbtest::RboxIoCtxMock* io_ctx_mock=new librmbtest::RboxIoCtxMock();
  storage.set_io_ctx(io_ctx_mock);

  EXPECT_CALL(*io_ctx_mock,operate(_,_,_)).Times(11).WillRepeatedly(Return(-ETIMEDOUT));
  int ret=storage.read_mail(oid,mail,0);
  std::cout<<"ret_value"<<ret<<std::endl;
  EXPECT_NE(0,ret);
  delete io_ctx_mock;
}
*/
TEST_F(StorageTest, deinit) {}

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}