#
# Copyright (c) 2017-2018 Tallence AG and the authors
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1, as published by the Free Software
# Foundation.  See file COPYING.

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/storage-interface

shlibs = \
	$(top_builddir)/src/storage-interface/libinterface.la

bin_PROGRAMS = rmb

rmb_SOURCES = \
	mailbox_tools.h \
	ls_cmd_parser.h \
	rmb-commands.h \
	rados-mail-box.h

rmb_LDADD = $(shlibs)

#man1_MANS = rmb.man

#EXTRA_DIST = \
#	$(man1_MANS)

