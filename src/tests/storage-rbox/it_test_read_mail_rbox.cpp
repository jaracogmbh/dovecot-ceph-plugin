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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestCase.h"
#include "../../storage-engine/storage-backend-factory.h"
#include "../../librmb/rados-util-impl.h"

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

#include "libdict-rados-plugin.h"
#include "mail-search-parser-private.h"
#include "mail-search.h"
}
#include "rbox-storage.hpp"
#include "../mocks/mock_test.h"
#include "dovecot-ceph-plugin-config.h"
#include "../test-utils/it_utils.h"

using ::testing::AtLeast;
using ::testing::Return;

TEST_F(StorageTest, init) {}

TEST_F(StorageTest, mailbox_open_inbox) {
  struct mail_namespace *ns = mail_namespace_find_inbox(s_test_mail_user->namespaces);
  struct mailbox *box = mailbox_alloc(ns->list, "INBOX", MAILBOX_FLAG_READONLY);
  ASSERT_GE(mailbox_open(box), 0);
  mailbox_free(&box);
}


TEST_F(StorageTest, load_update_metadata){
  librados::ObjectWriteOperation write_op;
  ceph::bufferlist bl;
  std::string val;

  val = "123";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("U",bl);
  bl.clear();

  val = "4567";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("R",bl);
  bl.clear();

  val = "8910111213";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("Z",bl);
  bl.clear();

  val = "111213";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("V",bl);
  bl.clear();
  
  val = "it is a test to put it not NULL";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("M",bl);
  bl.clear();

  val = "it is not NULL";
  bl.append(val.c_str(), val.length() + 1);
  write_op.setxattr("G",bl);
  bl.clear();

  storage_interface::RadosCluster *cluster=storage_engine::StorageBackendFactory::create_cluster(storage_engine::CEPH);
  storage_interface::RadosStorage *storage=storage_engine::StorageBackendFactory::create_storage(storage_engine::CEPH,cluster);
  std::string pool_name="test_pool";
  storage->open_connection(pool_name);
  std::string oid = "test_oid";
  
  /**save metadata**/
  storage->get_io_ctx_wrapper()->operate(oid,&write_op);

  /**load metadata**/
  storage_interface::RadosMail *mail_read = storage->alloc_rados_mail();
  mail_read->set_oid("test_oid"); 
  std::map<std::string, ceph::bufferlist> metadata_map;
  int ret = storage->get_io_ctx_wrapper()->getxattrs(*mail_read->get_oid(), metadata_map);
  std::cout<<"it is the metadata size"<<metadata_map.size()<<std::endl<<std::endl;
  if(true){  
    for(std::map<std::string,ceph::bufferlist>::iterator it=metadata_map.begin(); it!=metadata_map.end(); ++it){   
        std::string key = (*it).first;
        std::string value = std::string((*it).second.c_str());
        storage_interface::RadosMetadata *xattr= new librmb::RadosMetadataImpl(key,value);
        mail_read->add_metadata(xattr);     
    }
    librmb::RadosUtilsImpl rados_utils;
    ret = rados_utils.get_all_keys_and_values(storage->get_io_ctx_wrapper(), *mail_read->get_oid(), mail_read);
  }

  /**validate_metadata**/
  librmb::RadosUtilsImpl utils;
  std::string buf_str;
  int nummeric = 0;
  std::map<std::string,void*>* metadata=mail_read->get_metadata();

  if(metadata->find("Z") != metadata->end()){
    buf_str = std::string(((ceph::bufferlist*)(*metadata)["Z"])->c_str());
    std::cout<<"key is Z, value is::"<<buf_str<<"value length is"<<buf_str.size()<<std::endl;
    nummeric=utils.is_numeric(((ceph::bufferlist*)(*metadata)["Z"])->c_str()) ? 0 : 1;
    std::cout<<"key is equal to PSIZE::"<<nummeric<<std::endl<<std::endl;
  }

  if(metadata->find("R") != metadata->end()){
    buf_str = std::string(((ceph::bufferlist*)(*metadata)["R"])->c_str());
    std::cout<<"key is R, value is::"<<buf_str<<"value length is"<<buf_str.size()<<std::endl;
    nummeric=utils.is_numeric(((ceph::bufferlist*)(*metadata)["R"])->c_str()) ? 0 : 1;
    std::cout<<"key is equal to TIME::"<<nummeric<<std::endl<<std::endl;
  }

  if(metadata->find("U") != metadata->end()){
    buf_str = std::string(((ceph::bufferlist*)(*metadata)["U"])->c_str());
    std::cout<<"key is U, value is::"<<buf_str<<"value length is"<<buf_str.size()<<std::endl;
    nummeric=utils.is_numeric(((ceph::bufferlist*)(*metadata)["U"])->c_str()) ? 0 : 1;
    std::cout<<"key is equal to UID::"<<nummeric<<std::endl<<std::endl;
  }

  if(metadata->find("V") != metadata->end()){
    buf_str = std::string(((ceph::bufferlist*)(*metadata)["V"])->c_str());
    std::cout<<"key is V, value is::"<<buf_str<<"value length is"<<buf_str.size()<<std::endl;
    nummeric=utils.is_numeric(((ceph::bufferlist*)(*metadata)["V"])->c_str()) ? 0 : 1;
    std::cout<<"key is equal to VSIZE::"<<nummeric<<std::endl<<std::endl;
  }

  if(metadata->find("M") != metadata->end()){
    nummeric= ((ceph::bufferlist*)(*metadata)["M"])->c_str() == NULL ? 1 : 0;
    std::cout<<"key is equal to mailbox_guid::"<<nummeric<<std::endl<<std::endl;
  }

  if(metadata->find("G") != metadata->end()){
    nummeric= ((ceph::bufferlist*)(*metadata)["G"])->c_str() == NULL ? 1 : 0;
    std::cout<<"key is equal to metadata_guid::"<<nummeric<<std::endl<<std::endl;
  }

  delete mail_read;
  mail_read = nullptr;
  delete storage;
  storage = nullptr;
  delete cluster;
  cluster = nullptr;  
}
/**
 * Adds a mail via the regular alloc, save, commit plugin cycle and
 * afterwards calls the dovecot read_mail api calls to read the mail
 * via the plugin read cycle.
 *
 * Additionally tests the input and output stream classes,
 * by comparing the resulting streams.
 */
TEST_F(StorageTest, read_mail_test) {
  struct mailbox_transaction_context *desttrans;
  struct mail *mail;
  struct mail_search_context *search_ctx;
  struct mail_search_args *search_args;
  struct mail_search_arg *sarg;

  const char *message =
      "From: user@domain.org\n"
      "Date: Sat, 24 Mar 2017 23:00:00 +0200\n"
      "Mime-Version: 1.0\n"
      "Content-Type: text/plain; charset=us-ascii\n"
      "\n"
      "body\n";

  const char *mailbox = "INBOX";
  std::cout<<"it correctly works 79"<<std::endl;
  // testdata
  testutils::ItUtils::add_mail(message, mailbox, StorageTest::s_test_mail_user->namespaces);
  std::cout<<"it correctly works 82"<<std::endl;
  search_args = mail_search_build_init();
  std::cout<<"it correctly works 84"<<std::endl;
  sarg = mail_search_build_add(search_args, SEARCH_ALL);
  std::cout<<"it correctly works 86"<<std::endl;
  ASSERT_NE(sarg, nullptr);

  struct mail_namespace *ns = mail_namespace_find_inbox(s_test_mail_user->namespaces);
  ASSERT_NE(ns, nullptr);

  struct mailbox *box = mailbox_alloc(ns->list, mailbox, MAILBOX_FLAG_SAVEONLY);

  if (mailbox_open(box) < 0) {
    i_error("Opening mailbox %s failed: %s", mailbox, mailbox_get_last_internal_error(box, NULL));
    FAIL() << " Forcing a resync on mailbox INBOX Failed";
  }
#ifdef DOVECOT_CEPH_PLUGIN_HAVE_MAIL_STORAGE_TRANSACTION_OLD_SIGNATURE
  desttrans = mailbox_transaction_begin(box, MAILBOX_TRANSACTION_FLAG_EXTERNAL);
#else
  char reason[256];
  memset(reason, '\0', sizeof(reason));
  desttrans = mailbox_transaction_begin(box, MAILBOX_TRANSACTION_FLAG_EXTERNAL, reason);
#endif

  search_ctx = mailbox_search_init(desttrans, search_args, NULL, static_cast<mail_fetch_field>(0), NULL);
  mail_search_args_unref(&search_args);
  std::cout<<"it correctly works 106"<<std::endl;
  struct message_size hdr_size, body_size;
  struct istream *input = NULL;
  while (mailbox_search_next(search_ctx, &mail)) {
    int ret2 = mail_get_stream(mail, &hdr_size, &body_size, &input);
    EXPECT_EQ(ret2, 0);
    EXPECT_NE(input, nullptr);
    EXPECT_NE(body_size.physical_size, (uoff_t)0);
    EXPECT_NE(hdr_size.physical_size, (uoff_t)0);

    size_t size = -1;
    int ret_size = i_stream_get_size(input, true, &size);
    EXPECT_EQ(ret_size, 1);
    uoff_t phy_size;
    index_mail_get_physical_size(mail, &phy_size);

    std::string msg3(
        "From: user@domain.org\nDate: Sat, 24 Mar 2017 23:00:00 +0200\nMime-Version: 1.0\nContent-Type: "
        "text/plain; charset=us-ascii\n\nbody\n");

    EXPECT_EQ(phy_size, msg3.length());  // i_stream ads a \r before every \n

    // read the input stream and evaluate content.
    struct const_iovec iov;
    const unsigned char *data = NULL;
    ssize_t ret = 0;
    std::string buff;
    do {
      (void)i_stream_read_data(input, &data, &iov.iov_len, 0);
      std::cout<<"it correctly works 135"<<std::endl;
      if (iov.iov_len == 0) {
        if (input->stream_errno != 0)
          FAIL() << "stream errno";
        break;
      }
      const char *data_t = reinterpret_cast<const char *>(data);
      std::string tmp(data_t, phy_size);
      buff += tmp;
      // make sure mail is \0 terminated!
      EXPECT_EQ(*(data_t + phy_size + 1), '\0');
    } while ((size_t)ret == iov.iov_len);

    //    i_debug("data: %s", buff.c_str());
    std::string msg(
        "From: user@domain.org\nDate: Sat, 24 Mar 2017 23:00:00 +0200\nMime-Version: 1.0\nContent-Type: "
        "text/plain; charset=us-ascii\n\nbody\n");

    // validate !
    EXPECT_EQ(buff, msg);

    break;
  }

  if (mailbox_search_deinit(&search_ctx) < 0) {
    FAIL() << "search deinit failed";
  }

  if (mailbox_transaction_commit(&desttrans) < 0) {
    FAIL() << "tnx commit failed";
  }

  if (mailbox_sync(box, static_cast<mailbox_sync_flags>(0)) < 0) {
    FAIL() << "sync failed";
  }

  ASSERT_EQ(1, (int)box->index->map->hdr.messages_count);
  mailbox_free(&box);
}

TEST_F(StorageTest, deinit) {}

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
