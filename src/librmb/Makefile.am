#
# Copyright (c) 2017-2018 Tallence AG and the authors
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1, as published by the Free Software
# Foundation.  See file COPYING.

SUBDIRS = . tools

lib_LTLIBRARIES = \
	librmb.la 

headers = \
	encoding.h \
	rados-cluster-impl.h \
	rados-storage-impl.h \
	rados-dictionary-impl.h \
	rados-mail-impl.h \
	rados-util-impl.h \
	rados-metadata-impl.h \
	rados-dovecot-config.h \
	rados-namespace-manager-impl.h\
	rados-dovecot-ceph-cfg-impl.h \
	rados-ceph-config-impl.h \
	rados-ceph-json-config-impl.h \
	rados-metadata-storage-impl.h \
	rados-metadata-storage-default.h \
	rados-metadata-storage-ima.h \
	rados-save-log-impl.h \
	rbox-io-ctx-impl.h
		
	

librmb_la_SOURCES = \
    $(headers) \
	rados-cluster-impl.cpp \
	rados-storage-impl.cpp \
	rados-dictionary-impl.cpp \
	rados-mail-impl.cpp \
	rados-util-impl.cpp \
	rados-dovecot-config.cpp \
	rados-namespace-manager-impl.cpp\
	rados-ceph-config-impl.cpp \
	rados-dovecot-ceph-cfg-impl.cpp \
	rados-ceph-json-config-impl.cpp \
	rados-metadata-storage-default.cpp \
	rados-metadata-storage-ima.cpp \
	rados-save-log-impl.cpp
	
AM_LDFLAGS = $(JANSSON_LIBS)
AM_CFLAGS = $(JANSSON_CFLAGS)
AM_CXXFLAGS = $(JANSSON_CFLAGS)

pkginc_libdir=$(pkgincludedir)
pkginc_lib_HEADERS = $(headers)

