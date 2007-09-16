## Process this file with automake to produce Makefile.in

SUBDIRS = ../../../../contrib/libamrnb

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRNB_INCLUDE@

lib_LTLIBRARIES = libcodec_amr.la

libcodec_amr_la_LIBADD = @AMRNB_LIB_ROOT@/.libs/libamrnb.la

libcodec_amr_la_SOURCES = PlgAmr.c
libcodec_amr_la_LDFLAGS = \
	-avoid-version

# Currently only contrib amr library supported
# uncoment lines if you want to use standart Makefile
#
#libcodec_amr_la_LIBADD = @AMRNB_LIB_ROOT@@AMRNB_LIB_TARGET@
#libcodec_amr_la_DEPENDENCIES = libamrnb
#
#.PHONY: libamrnb
#libamrnb: 	
#	make -C @AMRNB_LIB_ROOT@ all
