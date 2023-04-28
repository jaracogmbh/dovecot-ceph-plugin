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

#ifndef SRC_STORAGE_INTERFACES_RADOS_STORAGE_INTERFACE_H_
#define SRC_STORAGE_INTERFACES_RADOS_STORAGE_INTERFACE_H_

#include <string>
#include <map>
#include <list>
#include <set>
#include <rados/librados.hpp>
#include "rados-cluster.h"
#include "rados-mail.h"
#include "rados-types.h"
#include "rbox-io-ctx.h"

namespace storage_interface {
/** class RadosStorage
 *  brief an abstract Rados Storage
 *  details This abstract class provides the api
 *         to create a rados cluster Storage.
 */
class RadosStorage {
 public:
  virtual ~RadosStorage() {}

  /*!
   * if connected, return the valid ioCtx
   */
  virtual storage_interface::RboxIoCtx* get_io_ctx_wrapper() = 0;
  /*! get the object size and object save date
   * @param[in] oid unique ident for the object
   * @param[out] psize size of the object
   * @param[out] pmtime last modified date**/
  virtual int stat_mail(const std::string &oid, uint64_t *psize, time_t *pmtime) = 0;
  /*! set the object namespace
   * @param[in] _nspace namespace */
  virtual void set_namespace(const std::string &_nspace) = 0;
  /*! get the object namespace
   *
   * @return copy of the namespace
   * */
  virtual std::string get_namespace() = 0;
  /*! get the pool name
   * @return copy of the current 
  pool name
   * */
  virtual std::string get_pool_name() = 0;

  /* set the wait method for async operations */
  virtual void set_ceph_wait_method(storage_interface::rbox_ceph_aio_wait_method wait_method) = 0;
    
  /*! get the max operation size in mb
   * @return the maximal number of mb to write in a single write operation*/
  virtual int get_max_write_size() = 0;
  /*! get the max operation size in bytes
   * @return max number of bytes to write in a single write operation*/
  virtual int get_max_write_size_bytes() = 0;

  /*! get the max ceph object size 
   */
  virtual int get_max_object_size() = 0;
  /*! delete object with given oid
   * @param[in] object identifier.
   *
   * @return <0 in case of failure
   */
  virtual int delete_mail(const std::string &oid) = 0;
  /*! search for mails based on given Filter
   * @param[in] attr a list of filter attributes
   *
   * @return object iterator or librados::NObjectIterator::__EndObjectIterator */
  virtual std::set<std::string> find_mails(const storage_interface::RadosMetadata *attr) = 0;
 
  virtual std::set<std::string> find_mails_async(const storage_interface::RadosMetadata *attr, 
                                                 std::string &pool_name, 
                                                 int num_threads,
                                                 void (*ptr)(std::string&)) = 0;


  /*! open the rados connections with default cluster and username
   * @param[in] poolname the poolname to connect to, in case this one does not exists, it will be created.
   * */
  virtual int open_connection(const std::string &poolname) = 0;
  /*! open the rados connection with given user and clustername
   *
   * @param[in] poolname the poolname to connect to, in case this one does not exists, it will be created.
   * @param[in] clustername custom clustername
   * @param[in] rados_username custom username (client.xxx)
   *
   * @return linux error code or 0 if successful.
   * */
  virtual int open_connection(const std::string &poolname, const std::string &clustername,
                              const std::string &rados_username) = 0;

  /*! open the rados connections with default cluster and username
   * @param[in] poolname the poolname to connect to, in case this one does not exists, it will be created.
   * @param[in] index_pool the poolname to store recovery index objects to.
   * */
  virtual int open_connection(const std::string &poolname, const std::string &index_pool) = 0;

 /*! open the rados connection with given user and clustername
   *
   * @param[in] poolname the poolname to connect to, in case this one does not exists, it will be created.
   * @param[in] index_pool the poolname to store recovery index objects to.
   * @param[in] clustername custom clustername
   * @param[in] rados_username custom username (client.xxx)
   *
   * @return linux error code or 0 if successful.
   * */
  virtual int open_connection(const std::string &poolname, const std::string &index_pool,
                      const std::string &clustername,
                      const std::string &rados_username) = 0;
  /*!
   * close the connection. (clean up structures to allow reconnect)
   */
  virtual void close_connection() = 0;
  /**
   * append oid to index object
  */
  virtual int ceph_index_append(const std::string &oid) = 0;

  /**
   * append oids to index object
  */
  virtual int ceph_index_append(const std::set<std::string> &oids) = 0;

  /**
   * overwrite ceph index object
  */
  virtual int ceph_index_overwrite(const std::set<std::string> &oids) = 0;

  /**
   * get the ceph index object as list of oids
   * 32
  */
  virtual std::set<std::string> ceph_index_read() = 0;


  /**
   * remove oids from index object
  */
  virtual int ceph_index_delete() = 0;

  /**
   * returns the ceph index size
  */
  virtual uint64_t ceph_index_size() = 0;

  /*! read the complete mail object into bufferlist
   *
   * @param[in] oid unique object identifier
   * @param[out] buffer valid ptr to bufferlist.
   * @return linux errorcode or 0 if successful
   * */
  // virtual int read_mail(const std::string &oid, librados::bufferlist *buffer) = 0;
  virtual int read_mail(const std::string &oid, storage_interface::RadosMail* mail,int try_counter)=0;

  /*! move a object from the given namespace to the other, updates the metadata given in to_update list
   *
   * @param[in] src_oid unique identifier of source object
   * @param[in] src_ns  namespace of source object
   * @param[in] dest_oid unique identifier of destination object
   * @param[in] dest_ns namespace of destination object
   * @param[in] to_update in case metadata should be updated.
   * @param[in] delete_source in case you really want to delete the source after copy.
   * @return linux errorcode or 0 if successful
   * */
  virtual int move(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
                   std::list<storage_interface::RadosMetadata*> &to_update, bool delete_source) = 0;

  /*! copy a object from the given namespace to the other, updates the metadata given in to_update list
   * @param[in] src_oid unique identifier of source object
   * @param[in] src_ns  namespace of source object
   * @param[in] dest_oid unique identifier of destination object
   * @param[in] dest_ns namespace of destination object
   * @param[in] to_update in case metadata should be updated.
   * @return linux errorcode or 0 if successful
   */
  virtual int copy(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
                   std::list<storage_interface::RadosMetadata*> &to_update) = 0;
  /*! save the mail
   * @param[in] mail valid rados mail.   
   * @return false in case of error
   * */
  virtual bool save_mail(storage_interface::RadosMail *mail) = 0;
 
  /*! create a new RadosMail
   * create new rados Mail Object.
   *  return pointer to mail object or nullptr
   * */
  virtual storage_interface::RadosMail *alloc_rados_mail() = 0;
  /*! free the Rados Mail Object
   * @param[in] mail ptr to valid mail object
   * */
  virtual void free_rados_mail(storage_interface::RadosMail *mail) = 0;

  virtual void* alloc_mail_buffer() = 0;
  virtual const char* get_mail_buffer(void *buffer,int *mail_buff_size) = 0;
  virtual void free_mail_buffer(void *mail_buffer) = 0;
  virtual void append_to_buffer(void *buff,const unsigned char *chunk, size_t size) = 0;

};

}  // namespace storage_interface

#endif  // SRC_STORAGE_INTERFACES_RADOS_STORAGE_INTERFACE_H_
