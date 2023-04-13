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

#ifndef SRC_LIBRMB_RADOS_STORAGE_IMPL_H_
#define SRC_LIBRMB_RADOS_STORAGE_IMPL_H_

#include <stddef.h>

#include <map>
#include <string>
#include <cstdint>
#include <list>
#include <set>

#include <algorithm>
#include <rados/librados.hpp>

#include "../storage-interface/rados-mail.h"
#include "rbox-io-ctx.h"
#include "../storage-interface/rados-storage.h"
#include "../storage-interface/rados-metadata.h"
#include "rbox-io-ctx.h"

namespace librmb {
class RadosStorageImpl : public storage_interface::RadosStorage {
 public:
  explicit RadosStorageImpl(storage_interface::RadosCluster *cluster);
  virtual ~RadosStorageImpl();
  librados::IoCtx &get_io_ctx();
  librmb::RboxIoCtx& get_io_ctx_wrapper()override;
  void set_io_ctx_wrapper(librmb::RboxIoCtx* io_ctx_){
    if(io_ctx_wrapper!=nullptr){
      delete io_ctx_wrapper;
      io_ctx_wrapper=io_ctx_;
    }
  }
  
  int stat_mail(const std::string &oid, uint64_t *psize, time_t *pmtime) override;
  void set_namespace(const std::string &_nspace) override;
  std::string get_namespace() override { return nspace; }
  std::string get_pool_name() override { return pool_name; }

  void set_ceph_wait_method(enum rbox_ceph_aio_wait_method wait_method_) override { this->wait_method = wait_method_; }
  int get_max_write_size() override { return max_write_size; }
  int get_max_write_size_bytes() override { return max_write_size * 1024 * 1024; }
  int get_max_object_size() override {return max_object_size;}

  int delete_mail(const std::string &oid) override;


  std::set<std::string> find_mails(const storage_interface::RadosMetadata *attr) override;
  
  std::set<std::string> find_mails_async(const storage_interface::RadosMetadata *attr, std::string &pool_name, int num_threads, void (*ptr)(std::string&)) override;

  int open_connection(const std::string &poolname) override;
  int open_connection(const std::string &poolname, const std::string &index_pool) override;


  int open_connection(const std::string &poolname, const std::string &index_pool,
                      const std::string &clustername,
                      const std::string &rados_username) override;
  int open_connection(const std::string &poolname, const std::string &clustername,
                      const std::string &rados_username) override;
  void close_connection() override;
  // int read_mail(const std::string &oid, librados::bufferlist *buffer) override;
  int read_mail(const std::string &oid, storage_interface::RadosMail* mail,int try_counter) override;
  int move(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
           std::list<storage_interface::RadosMetadata*> &to_update, bool delete_source) override;
  int copy(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
           std::list<storage_interface::RadosMetadata*> &to_update) override;
  bool save_mail(storage_interface::RadosMail *mail) override;
  storage_interface::RadosMail *alloc_rados_mail() override;

  void free_rados_mail(storage_interface::RadosMail *mail) override;

  uint64_t ceph_index_size() override;
  int ceph_index_append(const std::string &oid)  override;
  int ceph_index_append(const std::set<std::string> &oids)  override;
  int ceph_index_overwrite(const std::set<std::string> &oids)  override;
  std::set<std::string> ceph_index_read() override;
  int ceph_index_delete() override;
  void* alloc_mail_buffer() override;
  const char* get_mail_buffer(void *buffer,int *mail_buff_size) override;
  void free_mail_buffer(void* mail_buffer) override;
  void append_to_buffer(void *buff,const unsigned char * chunk, size_t size) override;

  bool execute_operation(std::string &oid, librados::ObjectWriteOperation *write_op_xattr);
  bool append_to_object(std::string &oid, librados::bufferlist &bufferlist, int length);

 private:
  int create_connection(const std::string &poolname,const std::string &index_pool);
  librados::IoCtx &get_recovery_io_ctx();
  int split_buffer_and_exec_op(storage_interface::RadosMail *current_object, librados::ObjectWriteOperation *write_op_xattr,
                               const uint64_t &max_write);
  int aio_operate(librados::IoCtx *io_ctx_, const std::string &oid, librados::AioCompletion *c,
                  librados::ObjectWriteOperation *op);                             
 
 private:
  storage_interface::RadosCluster *cluster;
  int max_write_size;
  int max_object_size;
  std::string nspace;
  librmb::RboxIoCtx* io_ctx_wrapper;
 
  bool io_ctx_created;
  std::string pool_name;
  enum rbox_ceph_aio_wait_method wait_method;

  static const char *CFG_OSD_MAX_WRITE_SIZE;
  static const char *CFG_OSD_MAX_OBJECT_SIZE;
};
}  // namespace librmb

#endif  // SRC_LIBRMB_RADOS_MAIL_STORAGE_IMPL_H_
