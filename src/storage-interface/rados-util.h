// -*- mode:C++ ; tab-width:8 ; c-basic-offset:2 ; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c) 2017-2018 Tallence AG and the authors
 *
 * This is free software ; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */

#ifndef SRC_STORAGE_INTERFACES_RADOS_UTIL_H_
#define SRC_STORAGE_INTERFACES_RADOS_UTIL_H_

#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <regex>

#include <string>
#include <map>
#include "rados-storage.h"
#include "rados-metadata-storage.h"
#include "rados-types.h"
#include "rbox-io-ctx.h"

namespace storage_interface {

/**
 * Rados Utils
 *
 * Utility class with usefull helper functions.
 *
 */
class RadosUtils {
 public:
  virtual ~RadosUtils(){}
  /*!
   * convert given date string to time_t
   *
   * @param[in] date Date format: %Y-%m-%d %H:%M:%S
   * @param[out] val ptr to time_t.
   * @return true if sucessfull.
   */
  virtual bool convert_str_to_time_t(const std::string &date, time_t *val) = 0;
  /*!
   * check if given string is a numeric value.
   * @param[in] s string if s is empty => false
   * @return true if given string is numeric.
   */
  virtual bool is_numeric(const char *s) = 0;
  /*!
   * check if given string is a numeric value.
   * @param[in] text string, if string is empty => true
   * @return true if given string is numeric.
   */
  virtual bool is_numeric_optional(const char *text) = 0;
  /*!
   * checks if key is a data attribute
   */
  virtual bool is_date_attribute(const storage_interface::rbox_metadata_key &key) = 0;
  /*!
   * converts given data_string to numeric string
   * @param[in] date_string Date format: %Y-%m-%d %H:%M:%S
   * @param[out] date : unix timestamp.
   */
  virtual bool convert_string_to_date(const std::string &date_string, std::string *date) = 0;
  /*!
   * converts given time_to to string %Y-%m-%d %H:%M:%S
   * @param[in] t time_t
   * @param[out] ret_val : ptr to valid string buffer.
   * @return <0 error
   */
  virtual int convert_time_t_to_str(const time_t &t, std::string *ret_val) = 0;
  /*!
   * converts flags to hex string
   * @param[in] flags flags
   * @param[out] ptr to string buffer:
   * @return false if not sucessful
   */
  virtual bool flags_to_string(const uint8_t &flags, std::string *flags_str) = 0;

  /*!
   * converts hex string to uint8_t
   * @param[in] flags_str flags (e.g. 0x03
   * @param[out] flags to uint8_t
   * @return false if not sucessful
   */
  virtual bool string_to_flags(const std::string &flags_str, uint8_t *flags) = 0;

  /*!
   * replace string in text.
   * @param[in,out] source: source string
   * @param[in] find : text to find.
   * @param[in] replace: text to replace.
   */
  virtual void find_and_replace(std::string *source, std::string const &find, std::string const &replace) = 0;

  /*!
   * get a list of key value pairs
   * @param[in] io_ctx valid io_ctx
   * @param[in] oid: unique identifier
   * @param[out] kv_map valid ptr to key value map.
   */
  virtual int get_all_keys_and_values(storage_interface::RboxIoCtx *io_ctx, const std::string &oid,
                                     storage_interface::RadosMail* mail) = 0;
  /*!
   * get the text representation of uint flags.
   * @param[in] flags
   * @param[out] flat : string representation
   */
  virtual void resolve_flags(const uint8_t &flags, std::string *flat) = 0;
  /*!
   * copy object to alternative storage
   * @param[in] src_oid
   * @param[in] dest_oid
   * @param[in] primary rados primary storage
   * @param[in] alt_storage rados alternative storage
   * @param[in] metadata storage
   * @param[in] bool inverse if true, copy from alt to primary.
   * @return linux error code or 0 if sucessful
   */
  virtual int copy_to_alt(std::string &src_oid, std::string &dest_oid, storage_interface::RadosStorage *primary, storage_interface::RadosStorage *alt_storage,
                         storage_interface::RadosMetadataStorage *metadata, bool inverse) = 0;
  /*!
   * move object to alternative storage
   * @param[in] src_oid
   * @param[in] dest_oid
   * @param[in] primary rados primary storage
   * @param[in] alt_storage rados alternative storage
   * @param[in] metadata storage
   * @param[in] bool inverse if true, move from alt to primary.
   * @return linux error code or 0 if sucessful
   */
  virtual int move_to_alt(std::string &oid, storage_interface::RadosStorage *primary, storage_interface::RadosStorage *alt_storage,
                         storage_interface::RadosMetadataStorage *metadata, bool inverse) = 0;
  /*!
   * increment (add) value directly on osd
   * @param[in] ioctx
   * @param[in] oid
   * @param[in] key
   * @param[in] value_to_add
   *
   * @return linux error code or 0 if sucessful
   */
  virtual int osd_add(storage_interface::RboxIoCtx *ioctx, const std::string &oid, const std::string &key, long long value_to_add) = 0;
  /*!
   * decrement (sub) value directly on osd
   * @param[in] ioctx
   * @param[in] oid
   * @param[in] key
   * @param[in] value_to_subtract
   *
   * @return linux error code or 0 if sucessful
   */
  virtual int osd_sub(storage_interface::RboxIoCtx *ioctx, const std::string &oid, const std::string &key,
                     long long value_to_subtract) = 0;

  /*!
   * check all given metadata key is valid
   *
   * @param[in] metadata
   * @return true if all keys and value are correct. (type, name, value)
   */
  virtual bool validate_metadata(std::map<std::string, void*> *metadata) = 0;
  /*!
   * get metadata
   *
   * @param[in] key
   * @param[int] valid pointer to metadata map
   * @return the metadata value
   */
  virtual void get_metadata(const std::string &key, std::map<std::string, void*> *metadata, char **value) = 0;

  /*!
   * get metadata
   *
   * @param[in] key
   * @param[int] valid pointer to metadata map
   * @return the metadata value
   */
  virtual void get_metadata(storage_interface::rbox_metadata_key key, std::map<std::string, void*> *metadata, char **value) = 0;


  /**
   * POC Implemnentation to extract pgs and primary osds from mon_command output!
   **/
  virtual std::vector<std::string> extractPgs(const std::string& str) = 0;

  virtual std::map<std::string, std::vector<std::string>> extractPgAndPrimaryOsd(const std::string& str) = 0;

  virtual std::vector<std::string> split(std::string str_to_split, char delimiter) = 0;


  virtual std::string convert_to_ceph_index(const std::set<std::string> &list) = 0;
  virtual std::string convert_to_ceph_index(const std::string &str) = 0;

  virtual std::set<std::string> ceph_index_to_set(const std::string &str) = 0;
};

}  // namespace storage_interface

#endif  // SRC_STORAGE_INTERFACES_RADOS_UTIL_H_
