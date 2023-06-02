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

#ifndef SRC_LIBRMB_RADOS_SAVE_LOG_IMPL_H_
#define SRC_LIBRMB_RADOS_SAVE_LOG_IMPL_H_


#include "../storage-interface/rados-save-log.h"
#include "../librmb/rados-metadata-impl.h"

namespace librmb {
/**
 * RadosSaveLogEntryImpl
 *
 * Class provides access to the savelog.format.
 *
 */
class RadosSaveLogEntryImpl : public  storage_interface::RadosSaveLogEntry{
 public:
  RadosSaveLogEntryImpl() {}
  RadosSaveLogEntryImpl(const std::string &oid_, const std::string &ns_, const std::string &pool_, const std::string &op_)
      : oid(oid_), ns(ns_), pool(pool_), op(op_), metadata(0) {}
  ~RadosSaveLogEntryImpl(){
    std::list<storage_interface::RadosMetadata*>::iterator it;
    for(it = metadata.begin(); it != metadata.end(); ++it){
        delete (*it);
        (*it)=nullptr;
    }
  };

  // format: mv|cp|save:src_ns,src_oid;metadata_key=metadata_value:metadata_key=metadata_value:....
  //        e.g.: // mv:ns_src:src_oid;M=ABCDEFG:B=INBOX:U=1
  bool parse_mv_op() override{
    int pos = op.find(";");
    if (pos <= 0) {
      return false;
    }

    std::stringstream left(op.substr(0, pos));
    std::vector<std::string> left_tokens;
    std::string item;
    while (std::getline(left, item, ':')) {
      left_tokens.push_back(item);
    }
    if (left_tokens.size() < 4) {
      return false;
    }

    if (left_tokens[0].compare("mv") != 0) {
      return true;  // not a move cmd.
    }
    // mv specific
    src_ns = left_tokens[1];
    src_oid = left_tokens[2];
    src_user = left_tokens[3];
    // parsing metadata.
    std::stringstream right(op.substr(pos + 1, op.size()));
    std::vector<std::string> right_tokens;
    while (std::getline(right, item, ':')) {
      storage_interface::RadosMetadata *m= new librmb::RadosMetadataImpl();
      if (!m->from_string(item)) {
        delete m;
        m=nullptr;
        return false;
      }
      metadata.push_back(m);
    }
    return true;
  }
  std::string op_save() override{ return "save"; }
  std::string op_cpy() override{ return "cpy"; }
  std::string op_mv(const std::string &src_ns, const std::string &src_oid, const std::string &src_user,
                           std::list<storage_interface::RadosMetadata *> &metadata) override{
    std::stringstream mv;
    mv << "mv:" << src_ns << ":" << src_oid << ":" << src_user << ";" << convert_metadata(metadata, ":");
    return mv.str();
  }

  friend std::ostream &operator<<(std::ostream &os, const librmb::RadosSaveLogEntryImpl &obj) {
    os << obj.get_op() << "," << obj.get_pool() << "," << obj.get_ns() << "," << obj.get_oid()<< std::endl;
    return os;
  }

  friend std::istream &operator>>(std::istream &is, librmb::RadosSaveLogEntryImpl &obj) {
    std::string line;
    std::string item;
    std::vector<std::string> csv_items;
    std::getline(is, line);

    std::stringstream line_to_parse(line);
    while (std::getline(line_to_parse, item, ',')) {
      csv_items.push_back(item);
    }

    // read obj from stream
    if (csv_items.size() == 4) {
      obj.get_op() = csv_items[0];
      obj.get_pool() = csv_items[1];
      obj.get_ns() = csv_items[2];
      obj.get_oid() = csv_items[3];
      obj.parse_mv_op();

    } else {
      is.setstate(std::ios::failbit);
    }
    return is;
  }

  static std::string convert_metadata(std::list<storage_interface::RadosMetadata *> &metadata, const std::string &separator) {
    std::stringstream metadata_str;
    std::list<storage_interface::RadosMetadata *>::iterator list_it;
    list_it = metadata.begin();
    if (list_it != metadata.end()) {
      metadata_str << (*list_it)->to_string();
      list_it++;
    }

    for (; list_it != metadata.end(); list_it++) {
      metadata_str << separator;
      metadata_str << (*list_it)->to_string();
    }
    return metadata_str.str();
  }
  
  std::string& get_oid() override{
    return oid;
  }

  std::string& get_ns() override{
    return ns;
  }

  std::string& get_pool() override{
    return pool;
  }

  std::string& get_op() override{
    return op;
  }
  
  std::string& get_src_oid() override{
    return src_oid;
  }
  std::string& get_src_ns() override{
    return src_ns;
  }

  std::string& get_src_user() override{
     return src_user;
  }

  std::list<storage_interface::RadosMetadata*>& get_metadata() override{
    return metadata;
  }

 private:
  std::string oid;   // oid
  std::string ns;    // namespace
  std::string pool;  // storage pool
  std::string op;    // operation: save, cp (copy), mv (move)
  std::string src_oid;
  std::string src_ns;
  std::string src_user;
  std::list<storage_interface::RadosMetadata*> metadata;
};

class RadosSaveLogImpl : public storage_interface::RadosSaveLog{
 public:
  explicit RadosSaveLogImpl(const std::string &logfile_) : logfile(logfile_) { log_active = !logfile.empty(); }
  RadosSaveLogImpl() { log_active = false; }
  virtual ~RadosSaveLogImpl(){};
  void set_save_log_file (const std::string &logfile_) override{
    this->logfile = logfile_;
    this->log_active = !logfile.empty();
  }
  bool open() override;
  void append(const storage_interface::RadosSaveLogEntry *entry) override;
  bool close() override;
  bool is_open() override{ return ofs.is_open(); }

 private:
  std::string logfile;
  bool log_active;
  std::ofstream ofs;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_RADOS_SAVE_LOG_IMPL_H_ */
