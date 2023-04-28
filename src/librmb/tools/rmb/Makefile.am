#
# Copyright (c) 2017-2018 Tallence AG and the authors
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1, as published by the Free Software
# Foundation.  See file COPYING.

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/librmb

shlibs = \
	$(top_builddir)/src/librmb/librmb.la

bin_PROGRAMS = rmb

rmb_SOURCES = \
	mailbox_tools_impl.cpp \
	mailbox_tools_impl.h \
	ls_cmd_parser_impl.cpp \
	ls_cmd_parser_impl.h \
	rmb-commands-impl.cpp \
	rmb-commands-impl.h \
	rmb.cpp \
	rados-mail-box-impl.h

rmb_LDADD = $(shlibs)

#man1_MANS = rmb.man

#EXTRA_DIST = \
#	$(man1_MANS)

