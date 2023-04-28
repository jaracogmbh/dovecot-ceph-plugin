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
#include <rados/librados.hpp>
#include "../../storage-interface/rados-util.h"
#include"../../librmb/rbox-io-ctx-impl.h"
#include "../../librmb/rados-cluster-impl.h"
#include "../../librmb/rados-mail-impl.h"
#include "../../librmb/rados-storage-impl.h"
#include "../../storage-interface/tools/rmb/rmb-commands.h"
#include "../../storage-interface/tools/rmb/ls_cmd_parser.h"
#include "../../librmb/tools/rmb/rmb-commands-impl.h"
#include "../../librmb/tools/rmb/ls_cmd_parser_impl.h"
#include "../../librmb/tools/rmb/mailbox_tools_impl.h"
#include "../../librmb/tools/rmb/rados-mail-box-impl.h"
#include "mock_test.h"
#include "../../storage-interface/rados-types.h"
#include "../../storage-interface/rados-mail.h"
#include "../../librmb/rados-metadata-impl.h"

using ::testing::Return;
using ::testing::_;
using ::testing::ReturnRef;

/**
 * test ls string parser
 */
TEST(rmb, test_cmd_parser) {
  std::string key = "M";
  std::string key2 = "U";
  std::string key3 = "R";
  //                             917378644
  std::string ls = "M=abc;U=1;R<2013-12-04 15:03";
  librmb::CmdLineParserImpl parser(ls);
  EXPECT_TRUE(parser.parse_ls_string());

  EXPECT_EQ(3, (int)parser.get_predicates().size());

  EXPECT_TRUE(parser.contains_key(key));
  EXPECT_TRUE(parser.contains_key(key2));
  EXPECT_TRUE(parser.contains_key(key3));

  storage_interface::rbox_metadata_key k = static_cast<storage_interface::rbox_metadata_key>('M');
  EXPECT_EQ(k, storage_interface::RBOX_METADATA_MAILBOX_GUID);
  storage_interface::Predicate *p = parser.get_predicate(key);
  std::string value = "abc";
  EXPECT_TRUE(p->eval(value));

  storage_interface::Predicate *p2 = parser.get_predicate(key2);
  value = "1";
  EXPECT_TRUE(p2->eval(value));

  storage_interface::Predicate *p3 = parser.get_predicate(key3);

  value = "1503393219";
  EXPECT_FALSE(p3->eval(value));
}

/**
 * Test date predicate
 */
TEST(rmb1, date_arg) {
  storage_interface::Predicate *p = new librmb::PredicateImpl();

  std::string date = "2013-12-04 15:03:00";
  time_t t = -1;
  p->convert_str_to_time_t(date, &t);
  std::cout << "time t " << t << std::endl;
  EXPECT_GT(t, -1);
  time_t t2 = 1503393219;
  std::string val;
  p->convert_time_t_to_str(t, &val);
  std::cout << val << std::endl;
  p->convert_time_t_to_str(t2, &val);
  std::cout << val << std::endl;

  time_t t3 = 1086165760;
  p->convert_time_t_to_str(t3, &val);
  std::cout << val << std::endl;
  delete p;
}

/**
 * Test mailboxTools.savemail
 */
TEST(rmb1, save_mail) {
  std::string mbox_guid = "abc";
  librmb::RadosMailBoxImpl mbox(mbox_guid, 1, mbox_guid);

  std::string base_path = "test";
  librmb::MailboxToolsImpl tools(&mbox, base_path);

  int init = tools.init_mailbox_dir();
  EXPECT_EQ(0, init);

  librmb::RadosMailImpl mail;
  librados::bufferlist bl;
  std::string attr_val = "1";
  bl.append(attr_val.c_str(), attr_val.length() + 1);
  (*mail.get_metadata())["U"] = bl;
  std::string mail_guid = "defg";
  void *buffer=(void*)new librados::bufferlist();
  mail.set_mail_buffer(buffer);
  ((librados::bufferlist*)mail.get_mail_buffer())->append("hallo welt\nbababababa\n");
  mail.set_oid(mail_guid);
  mail.set_mail_size(((librados::bufferlist*)mail.get_mail_buffer())->length() - 1);
  int save = tools.save_mail(&mail);
  EXPECT_EQ(0, save);

  int ret = tools.delete_mail(&mail);
  int ret_rm_dir = tools.delete_mailbox_dir();
  EXPECT_EQ(0, ret);
  EXPECT_EQ(0, ret_rm_dir);
  delete buffer;
}
/**
 * Test mailbox Tools constructor
 */
TEST(rmb1, path_tests) {
  std::string mbox_guid = "abc";
  librmb::RadosMailBoxImpl mbox(mbox_guid, 1, mbox_guid);

  std::string base_path = "test";
  librmb::MailboxToolsImpl tools(&mbox, base_path);
  EXPECT_EQ("test/abc", tools.get_mailbox_path());
  std::string test_path = "test/";
  librmb::MailboxToolsImpl tools2(&mbox, test_path);
  EXPECT_EQ("test/abc", tools2.get_mailbox_path());

  std::string test_path2 = "";
  librmb::MailboxToolsImpl tools3(&mbox, test_path2);
  EXPECT_EQ("abc", tools3.get_mailbox_path());
}
/**
 * Test rmb commands
 * - search
 */
TEST(rmb1, rmb_commands_no_objects_found) {
  librmbtest::RadosStorageMock storage_mock;
  librmbtest::RadosClusterMock cluster_mock;
  librmbtest::RadosStorageMetadataMock ms_module_mock;

  std::map<std::string, std::string> opts;
  librmb::RmbCommandsImpl rmb_cmd(&storage_mock, &cluster_mock, &opts);
  std::list<storage_interface::RadosMail *> mails;
  std::string search_string = "uid";
  std::set<std::string> mail_list;
  librmb::RboxIoCtxImpl test_ioctx;

  EXPECT_CALL(storage_mock, find_mails(nullptr)).WillRepeatedly(Return(mail_list));
  EXPECT_CALL(storage_mock,get_io_ctx_wrapper()).WillRepeatedly(Return(&test_ioctx));
  EXPECT_CALL(storage_mock, stat_mail(_, _, _)).WillRepeatedly(Return(0));
  int ret = rmb_cmd.load_objects(&ms_module_mock, mails, search_string);
  EXPECT_EQ(ret, 0);
}
/**
 * Test rmb commands
 * - search filter
 */
TEST(rmb1, rmb_command_filter_result) {
  librmbtest::RadosStorageMock storage_mock;
  librmbtest::RadosClusterMock cluster_mock;
  std::map<std::string, std::string> opts;
  opts["ls"] = "-";
  librmb::CmdLineParserImpl parser(opts["ls"]);
  librmb::RmbCommandsImpl rmb_cmd(&storage_mock, &cluster_mock, &opts);
  std::list<storage_interface::RadosMail *> mails;
  librmb::RadosMailImpl obj1;
  obj1.set_oid("oid_1");
  obj1.set_mail_size(200);
  obj1.set_rados_save_date(time(NULL));
  {
    std::string key = "M";
    std::string val = "8eed840764b05359f12718004d2485ee";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "I";
    std::string val = "v0.1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "G";
    std::string val = "8eed840764b05359f12718004d2485ee";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "R";
    std::string val = "1234567";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "S";
    std::string val = "1234561117";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "P";
    std::string val = "1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "O";
    std::string val = "0";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "Z";
    std::string val = "200";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "V";
    std::string val = "250";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "U";
    std::string val = "1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "A";
    std::string val = "";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "F";
    std::string val = "01";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "B";
    std::string val = "DRAFTS";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  mails.push_back(&obj1);

  EXPECT_EQ(0, rmb_cmd.query_mail_storage(&mails, &parser, false, false));
}
/**
 * Test rmb commands
 * - ls filter
 */
TEST(rmb1, rmb_command_filter_result2) {
  librmbtest::RadosStorageMock storage_mock;
  librmbtest::RadosClusterMock cluster_mock;
  std::map<std::string, std::string> opts;
  opts["ls"] = "-";
  librmb::CmdLineParserImpl parser(opts["ls"]);
  librmb::RmbCommandsImpl rmb_cmd(&storage_mock, &cluster_mock, &opts);
  std::list<storage_interface::RadosMail*> mails;
  librmb::RadosMailImpl obj1;
  obj1.set_oid("oid_1");
  obj1.set_mail_size(200);
  obj1.set_rados_save_date(time(NULL));
  {
    std::string key = "M";
    std::string val = "8eed840764b05359f12718004d2485ee";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "I";
    std::string val = "v0.1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "G";
    std::string val = "8eed840764b05359f12718004d2485ee";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "R";
    std::string val = "aafsadfasdfasdf";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "S";
    std::string val = "adfhasdfhsfdkahsdf";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "P";
    std::string val = "1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "O";
    std::string val = "0";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "Z";
    std::string val = "";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "V";
    std::string val = "250";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "U";
    std::string val = "1";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "A";
    std::string val = "";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "F";
    std::string val = "01";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  {
    std::string key = "B";
    std::string val = "DRAFTS";
    librmb::RadosMetadataImpl m(key, val);
    obj1.add_metadata(&m);
  }
  mails.push_back(&obj1);

  EXPECT_EQ(0, rmb_cmd.query_mail_storage(&mails, &parser, false, false));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
