## Process this file with automake to produce Makefile.in

SUBDIRS = ../../../../contrib/libilbc

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @ILBC_INCLUDE@

lib_LTLIBRARIES = libcodec_ilbc.la

libcodec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@/.libs/libilbc.la

libcodec_ilbc_la_SOURCES = PlgIlbc.c
libcodec_ilbc_la_LDFLAGS = \
	-avoid-version

# Currently only contrib ilbc library supported
# uncoment lines if you want to use standart Makefile
#
#libcodec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@@ILBC_LIB_TARGET@
#libcodec_ilbc_la_DEPENDENCIES = libilbc
#
#.PHONY: libilbc
#libilbc: 	
#	make -C @ILBC_LIB_ROOT@ all
