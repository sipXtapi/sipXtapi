## Process this file with automake to produce Makefile.in

SUBDIRS = ../../../../contrib/libamrwb

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRWB_INCLUDE@

lib_LTLIBRARIES = libcodec_amrwb.la

libcodec_amrwb_la_LIBADD = @AMRWB_LIB_ROOT@/.libs/libamrwb.la

libcodec_amrwb_la_SOURCES = PlgAmrWB.c
libcodec_amrwb_la_LDFLAGS = \
	-avoid-version

# Currently only contrib amr library supported
# uncoment lines if you want to use standart Makefile
#
#libcodec_amrwb_la_LIBADD = @AMRWB_LIB_ROOT@@AMRWB_LIB_TARGET@
#libcodec_amrwb_la_DEPENDENCIES = libamrwb
#
#.PHONY: libamrwb
#libamrwb: 	
#	make -C @AMRWB_LIB_ROOT@ all
