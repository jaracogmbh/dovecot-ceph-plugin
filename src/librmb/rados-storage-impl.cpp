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

#include "rados-storage-impl.h"

#include <algorithm>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <thread>
#include <mutex>
#include <iostream>
#include "rados-util.h"

#include <rados/librados.hpp>

#include <sstream>
#include "encoding.h"
#include "limits.h"
#include "rados-metadata-storage-impl.h"
#include "rbox-io-ctx.h"
#include "rbox-io-ctx-impl.h"
using std::pair;
using std::string;

using ceph::bufferlist;

using librmb::RadosStorageImpl;

#define DICT_USERNAME_SEPARATOR '/'
const char *RadosStorageImpl::CFG_OSD_MAX_WRITE_SIZE = "osd_max_write_size";
const char *RadosStorageImpl::CFG_OSD_MAX_OBJECT_SIZE= "osd_max_object_size";

RadosStorageImpl::RadosStorageImpl(RadosCluster *_cluster) {
  cluster = _cluster;
  max_write_size = 10;
  max_object_size = 134217728; //ceph default 128MB
  io_ctx_created = false;
  wait_method = WAIT_FOR_COMPLETE_AND_CB;
  io_ctx_wrapper=new librmb::RboxIoCtxImpl();
}

RadosStorageImpl::~RadosStorageImpl() {
  delete io_ctx_wrapper;
  io_ctx_wrapper=nullptr;
}
librmb::RboxIoCtx& RadosStorageImpl::get_io_ctx_wrapper(){
  return *io_ctx_wrapper;
}
//DEPRECATED!!!!! -> moved to rbox-save.cpp
int RadosStorageImpl::split_buffer_and_exec_op(RadosMail *current_object,
                                               librados::ObjectWriteOperation *write_op_xattr,
                                               const uint64_t &max_write) {
  
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }

  int ret_val = 0;
  uint64_t write_buffer_size = current_object->get_mail_size();

  assert(max_write > 0);

  if (write_buffer_size == 0 || 
      max_write <= 0) {      
    ret_val = -1;
    return ret_val;
  }

  ret_val = io_ctx_wrapper->operate(*current_object->get_oid(), write_op_xattr);

  if(ret_val< 0){
    ret_val = -1;
    return ret_val;
  }

  uint64_t rest = write_buffer_size % max_write;
  int div = write_buffer_size / max_write + (rest > 0 ? 1 : 0);
  for (int i = 0; i < div; ++i) {

    // split the buffer.
    librados::bufferlist tmp_buffer;
    
    librados::ObjectWriteOperation write_op;

    int offset = i * max_write;

    uint64_t length = max_write;
    if (write_buffer_size < ((i + 1) * length)) {
      length = rest;
    }
#ifdef HAVE_ALLOC_HINT_2
    write_op.set_alloc_hint2(write_buffer_size, length, librados::ALLOC_HINT_FLAG_COMPRESSIBLE);
#else
    write_op.set_alloc_hint(write_buffer_size, length);
#endif
    if (div == 1) {
      write_op.write(0, *current_object->get_mail_buffer());
    } else {
      tmp_buffer.clear();
      tmp_buffer.substr_of(*current_object->get_mail_buffer(), offset, length);
      write_op.write(offset, tmp_buffer);
    }
    
    ret_val = io_ctx_wrapper->operate(*current_object->get_oid(), &write_op);
    if(ret_val < 0){
      ret_val = -1;
      break;
    }
  }
  // deprecated unused
  current_object->set_write_operation(nullptr);
  current_object->set_completion(nullptr);
  current_object->set_active_op(0);
    
  // free mail's buffer cause we don't need it anymore
  librados::bufferlist *mail_buffer = current_object->get_mail_buffer();
  delete mail_buffer;

  return ret_val;
}

int RadosStorageImpl::save_mail(const std::string &oid, librados::bufferlist &buffer) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }
  librados::bufferlist librados_buffer;
  librados_buffer.append(buffer);
  return io_ctx_wrapper->write_full(oid,librados_buffer);
}
int RadosStorageImpl::read_mail(const std::string &oid, librmb::RadosMail* mail,int try_counter){
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }
  
  int ret=0;
  int stat_err = 0;
  int read_err = 0;
  uint64_t psize;
  time_t save_date;
  
  librados::ObjectReadOperation read_op;
  read_op.read(0, INT_MAX, mail->get_mail_buffer(), &read_err);
  read_op.stat(&psize, &save_date, &stat_err);
  ret=io_ctx_wrapper->operate(oid, &read_op, mail->get_mail_buffer());
  if(ret == -ETIMEDOUT) {
    int max_retry = 10; //TODO FIX 
    if(try_counter < max_retry){
      usleep(((rand() % 5) + 1) * 10000);
      try_counter++;
      ret=read_mail(oid,mail,try_counter);
    }
  }
  if(ret < 0 || ret == -ETIMEDOUT){
    return ret;
  }
  mail->set_mail_size((const int)psize);
  mail->set_rados_save_date(&save_date);
  return ret;
}
int RadosStorageImpl::delete_mail(const std::string &oid) {
  if (!cluster->is_connected() || oid.empty() || !io_ctx_created) {
    return -1;
  }
  return io_ctx_wrapper->remove(oid);
}

bool RadosStorageImpl::execute_operation(std::string &oid, librados::ObjectWriteOperation *write_op_xattr) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return false;
  }
  return io_ctx_wrapper->operate(oid, write_op_xattr) >=0 ? true : false;
}

bool RadosStorageImpl::append_to_object(std::string &oid, librados::bufferlist &bufferlist, int length) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return false;
  }
  return io_ctx_wrapper->append(oid, bufferlist, length) >=0 ? true : false;
}
int RadosStorageImpl::aio_operate(librados::IoCtx *io_ctx_, const std::string &oid, librados::AioCompletion *c,
                                  librados::ObjectWriteOperation *op) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }

  if (io_ctx_ != nullptr) {
    return io_ctx_->aio_operate(oid, c, op);
  } else {
    return io_ctx_wrapper->aio_operate(oid, c, op);
  }
}

int RadosStorageImpl::stat_mail(const std::string &oid, uint64_t *psize, time_t *pmtime) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }
  return io_ctx_wrapper->stat(oid, psize, pmtime);
}

void RadosStorageImpl::set_namespace(const std::string &_nspace) {
  io_ctx_wrapper->set_namespace(_nspace);
  this->nspace = _nspace;
}

std::set<std::string> RadosStorageImpl::find_mails(const RadosMetadata *attr) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return ;
  }
  std::set<std::string> mail_list;
  librados::NObjectIterator iter_guid;
  if (attr != nullptr) {
    std::string filter_name = PLAIN_FILTER_NAME;
    ceph::bufferlist filter_bl;

    encode(filter_name, filter_bl);
    encode("_" + attr->key, filter_bl);
    encode(attr->bl.to_str(), filter_bl);

    iter_guid=io_ctx_wrapper->nobjects_begin(filter_bl);
  } else {
    iter_guid=io_ctx_wrapper->nobjects_begin();
  }
  while (iter_guid != librados::NObjectIterator::__EndObjectIterator) {
    mail_list.insert((*iter_guid).get_oid());
    iter_guid++;
  }
  return mail_list;
}
/**
 * POC Implementation: 
 * 
 * see in prod how it behaves. 
 * 
 **/
std::set<std::string> RadosStorageImpl::find_mails_async(const RadosMetadata *attr, 
                                                         std::string &pool_name,
                                                         int num_threads,
                                                         void (*ptr)(std::string&)){

    std::set<std::string> oid_list;
    std::mutex oid_list_mutex;
    
    // Thread!!
    auto f = [](const std::vector<std::string> &list, 
                std::mutex &oid_mutex, 
                std::set<std::string> &oids, 
                librados::IoCtx *io_ctx,
                void (*ptr)(std::string&),
                std::string osd,
                ceph::bufferlist &filter) {

        int total_count = 0;

        for (auto const &pg: list) {

          uint64_t ppool;
          uint32_t pseed;
          int r = sscanf(pg.c_str(), "%llu.%x", (long long unsigned *)&ppool, &pseed);
          
          librados::NObjectIterator iter= io_ctx->nobjects_begin(pseed);
          int count = 0;
          while (iter != librados::NObjectIterator::__EndObjectIterator) {
            std::string oid = iter->get_oid();
            {
              std::lock_guard<std::mutex> guard(oid_mutex);          
              oids.insert(oid);  
              count++;
            }          
            iter++;
          }       
          total_count+=count;
          std::string t = "osd "+ osd +" pg done " + pg + " objects " + std::to_string(count);
          (*ptr)(t);    
        } 
        std::string t = "done with osd "+ osd + " total: " + std::to_string(total_count);
        (*ptr)(t);             
    }; 


    //std::string pool_mame = "mail_storage";
    std::map<std::string, std::vector<std::string>> osd_pg_map = cluster->list_pgs_osd_for_pool(pool_name);
    std::vector<std::thread> threads;

    std::string filter_name = PLAIN_FILTER_NAME;
    ceph::bufferlist filter_bl;

    encode(filter_name, filter_bl);
    encode("_" + attr->key, filter_bl);
    encode(attr->bl.to_str(), filter_bl);
    std::string msg_1 = "buffer set";
    (*ptr)(msg_1); 

    for (const auto& x : osd_pg_map){
      if(threads.size() == num_threads){        
        threads[0].join();
        threads.erase(threads.begin());            
      }
      //TODO: update parser that this will not end here filter out invalid osd
      if(x.first == "oon") {
        std::string create_msg = "skipping thread for osd: "+ x.first;
        (*ptr)(create_msg);         
        continue;
      }
      threads.push_back(std::thread(f, std::ref(x.second),std::ref(oid_list_mutex),std::ref(oid_list), &get_io_ctx(), ptr, x.first, std::ref(filter_bl)));
      std::string create_msg = "creating thread for osd: "+ x.first;
      (*ptr)(create_msg);       
    }


    for (auto const &thread: threads) {      
        thread.join();
    }   
    return oid_list;
}
librados::IoCtx& RadosStorageImpl::get_io_ctx() { return io_ctx_wrapper->get_io_ctx(); }
librados::IoCtx& RadosStorageImpl::get_recovery_io_ctx() { return io_ctx_wrapper->get_recovery_io_ctx(); }

int RadosStorageImpl::open_connection(const std::string &poolname, const std::string &index_pool,
                                      const std::string &clustername,
                                      const std::string &rados_username) {
  if (cluster->is_connected() && io_ctx_created) {
    // cluster is already connected!
    return 1;
  }

  if (cluster->init(clustername, rados_username) < 0) {
    return -1;
  }
  return create_connection(poolname, index_pool);
}
int RadosStorageImpl::open_connection(const std::string &poolname,
                                      const std::string &clustername,
                                      const std::string &rados_username) {
  if (cluster->is_connected() && io_ctx_created) {
    // cluster is already connected!
    return 1;
  }

  if (cluster->init(clustername, rados_username) < 0) {
    return -1;
  }
  return create_connection(poolname, poolname);
}
int RadosStorageImpl::open_connection(const string &poolname, const string &index_pool) {
  if (cluster->init() < 0) {
    return -1;
  }
  return create_connection(poolname, index_pool);
}
int RadosStorageImpl::open_connection(const string &poolname) {
  if (cluster->init() < 0) {
    return -1;
  }
  return create_connection(poolname, poolname);
}

int RadosStorageImpl::create_connection(const std::string &poolname, const std::string &index_pool){
  // pool exists? else create
  int err = cluster->io_ctx_create(poolname,*io_ctx_wrapper);
  if (err < 0) {
    return err;
  }

  err = cluster->recovery_index_io_ctx(index_pool,*io_ctx_wrapper);
  if (err < 0) {
    return err;
  }
  string max_write_size_str;
  err = cluster->get_config_option(RadosStorageImpl::CFG_OSD_MAX_WRITE_SIZE, &max_write_size_str);
  if (err < 0) {
    return err;
  }

  std::stringstream ss_write;
  ss_write << max_write_size_str;
  ss_write >> max_write_size;
 
  string max_object_size_str;
  err = cluster->get_config_option(RadosStorageImpl::CFG_OSD_MAX_OBJECT_SIZE, &max_object_size_str);
  if (err < 0) {
    return err;
  }
  std::stringstream ss;
  ss << max_object_size_str;
  ss >> max_object_size;
  
  if (err == 0) {
    io_ctx_created = true;
  }
  
  // set the poolname
  pool_name = poolname;
  return 0;
}
void RadosStorageImpl::close_connection() {
  if (cluster != nullptr && io_ctx_created) {
    cluster->deinit();
  }
}
// assumes that destination io ctx is current io_ctx;
int RadosStorageImpl::move(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
                           std::list<RadosMetadata> &to_update, bool delete_source) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }

  int ret = 0;
  librados::ObjectWriteOperation write_op;
  librados::IoCtx src_io_ctx, dest_io_ctx;

  librados::AioCompletion *completion = librados::Rados::aio_create_completion();

  // destination io_ctx is current io_ctx
  dest_io_ctx = get_io_ctx();

  if (strcmp(src_ns, dest_ns) != 0) {
    src_io_ctx.dup(dest_io_ctx);
    src_io_ctx.set_namespace(src_ns);
    dest_io_ctx.set_namespace(dest_ns);

#if LIBRADOS_VERSION_CODE >= 30000
    write_op.copy_from(src_oid, src_io_ctx, 0, 0);
#else
    write_op.copy_from(src_oid, src_io_ctx, 0);
#endif
  } else {
    src_io_ctx = dest_io_ctx;
    time_t t;
    uint64_t size;
    ret = src_io_ctx.stat(src_oid, &size, &t);
    if (ret < 0) {
      return ret;
    }
  }

  // because we create a copy, save date needs to be updated
  // as an alternative we could use &ctx->data.save_date here if we save it to xattribute in write_metadata
  // and restore it in read_metadata function. => save_date of copy/move will be same as source.
  // write_op.mtime(&ctx->data.save_date);
  time_t save_time = time(NULL);
  write_op.mtime(&save_time);

  // update metadata
  for (std::list<RadosMetadata>::iterator it = to_update.begin(); it != to_update.end(); ++it) {
    write_op.setxattr((*it).key.c_str(), (*it).bl);
  }
  ret = aio_operate(&dest_io_ctx, dest_oid, completion, &write_op);
  if (ret >= 0) {
    completion->wait_for_complete();
    ret = completion->get_return_value();
    if (delete_source && strcmp(src_ns, dest_ns) != 0 && ret == 0) {
      ret = src_io_ctx.remove(src_oid);
    }
  }
  completion->release();
  // reset io_ctx
  dest_io_ctx.set_namespace(dest_ns);
  return ret;
}

// assumes that destination io ctx is current io_ctx;
int RadosStorageImpl::copy(std::string &src_oid, const char *src_ns, std::string &dest_oid, const char *dest_ns,
                           std::list<RadosMetadata> &to_update) {
  if (!cluster->is_connected() || !io_ctx_created) {
    return -1;
  }

  librados::ObjectWriteOperation write_op;
  librados::IoCtx src_io_ctx, dest_io_ctx;

  // destination io_ctx is current io_ctx
  dest_io_ctx = get_io_ctx();

  if (strcmp(src_ns, dest_ns) != 0) {
    src_io_ctx.dup(dest_io_ctx);
    src_io_ctx.set_namespace(src_ns);
    dest_io_ctx.set_namespace(dest_ns);
  } else {
    src_io_ctx = dest_io_ctx;
  }

#if LIBRADOS_VERSION_CODE >= 30000
  write_op.copy_from(src_oid, src_io_ctx, 0, 0);
#else
  write_op.copy_from(src_oid, src_io_ctx, 0);
#endif

  // because we create a copy, save date needs to be updated
  // as an alternative we could use &ctx->data.save_date here if we save it to xattribute in write_metadata
  // and restore it in read_metadata function. => save_date of copy/move will be same as source.
  // write_op.mtime(&ctx->data.save_date);
  time_t save_time = time(NULL);
  write_op.mtime(&save_time);

  // update metadata
  for (std::list<RadosMetadata>::iterator it = to_update.begin(); it != to_update.end(); ++it) {
    write_op.setxattr((*it).key.c_str(), (*it).bl);
  }
  int ret = 0;
  librados::AioCompletion *completion = librados::Rados::aio_create_completion();
  ret = aio_operate(&dest_io_ctx, dest_oid, completion, &write_op);
  if (ret >= 0) {
    ret = completion->wait_for_complete();
    // cppcheck-suppress redundantAssignment
    ret = completion->get_return_value();
  }
  completion->release();
  // reset io_ctx
  dest_io_ctx.set_namespace(dest_ns);
  return ret;
}

bool  RadosStorageImpl::save_mail(RadosMail *current_object){
  
  if (!cluster->is_connected() || !io_ctx_created) {
    return false;
  }
  bool ret_val=false;
  int object_size = current_object->get_mail_size();
  int max_object_size = this->get_max_object_size();
  if( max_object_size < object_size ||object_size<0||max_object_size==0){
    return false;
  }

  librados::ObjectWriteOperation write_metadata;
  librados::IoCtx *io_ctx_=&(this->get_io_ctx());
  librmb::RadosMetadataStorageDefault rados_metadata_storage (io_ctx_);
  rados_metadata_storage.save_metadata(&write_metadata,current_object);
  ret_val=execute_operation(*current_object->get_oid(), &write_metadata);
  if(!ret_val){
    return ret_val;
  }
  
  int max_write=get_max_write_size_bytes();
  uint64_t rest = object_size % max_write;
  int div = object_size / max_write + (rest > 0 ? 1 : 0);
  for (int i = 0; i < div; ++i) {

    librados::bufferlist tmp_buffer;
    int offset = i * max_write;

    uint64_t length = max_write;
    if (object_size < ((i + 1) * length)) {
      length = rest;
    }

    if (div == 1) {
      librados::ObjectWriteOperation write_op;      
      write_op.write(0,*current_object->get_mail_buffer());
      ret_val=execute_operation(*current_object->get_oid(), &write_op);
    }
    else {
      if(offset + length >object_size){
        return false;
      }else{
        if(offset + length > current_object->get_mail_buffer()->length() ){
          tmp_buffer.substr_of(*current_object->get_mail_buffer(), offset,current_object->get_mail_buffer()->length() - offset );
        }else{  
          tmp_buffer.substr_of(*current_object->get_mail_buffer(), offset, length);
        }
      }      
      ret_val = append_to_object(*current_object->get_oid(), tmp_buffer, length); 
    }
    if(!ret_val){
      return ret_val;
    }
  }

  return ret_val;
}
  
librmb::RadosMail *RadosStorageImpl::alloc_rados_mail() {
  librmb::RadosMail * mail=new librmb::RadosMail(); 
  mail->set_mail_buffer(new librados::bufferlist());
  return mail;
}

void RadosStorageImpl::free_rados_mail(librmb::RadosMail *mail) {
  if (mail != nullptr) {
    delete mail;
    mail = nullptr;
  }
}

uint64_t RadosStorageImpl::ceph_index_size(){
  uint64_t psize;
  time_t pmtime;
  get_recovery_io_ctx().stat(get_namespace(), &psize, &pmtime);
  return psize;
}

int RadosStorageImpl::ceph_index_append(const std::string &oid) {  
  librados::bufferlist bl;
  bl.append(RadosUtils::convert_to_ceph_index(oid));
  return get_recovery_io_ctx().append( get_namespace(),bl, bl.length());
}

int RadosStorageImpl::ceph_index_append(const std::set<std::string> &oids) {
  librados::bufferlist bl;
  bl.append(RadosUtils::convert_to_ceph_index(oids));
  return get_recovery_io_ctx().append( get_namespace(),bl, bl.length());
}
int RadosStorageImpl::ceph_index_overwrite(const std::set<std::string> &oids) {
  librados::bufferlist bl;
  bl.append(RadosUtils::convert_to_ceph_index(oids));
  return get_recovery_io_ctx().write_full( get_namespace(),bl);
}
std::set<std::string> RadosStorageImpl::ceph_index_read() {
  std::set<std::string> index;
  librados::bufferlist bl;
  size_t max = INT_MAX;
  int64_t psize;
  time_t pmtime;
  get_recovery_io_ctx().stat(get_namespace(), &psize, &pmtime);
  if(psize <=0){
    return index;
  }
  int ret = get_recovery_io_ctx().read(get_namespace(),bl, max,0);


  if(ret < 0){
    return index;
  }
  index = RadosUtils::ceph_index_to_set(bl.c_str());
  return index;
}
int RadosStorageImpl::ceph_index_delete() {
  return get_recovery_io_ctx().remove(get_namespace());
}

