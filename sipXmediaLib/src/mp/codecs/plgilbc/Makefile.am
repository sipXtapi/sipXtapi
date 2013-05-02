## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include \
 -I$(top_srcdir)/sipXportLib/include \
 @ILBC_INCLUDE@

codeclib_LTLIBRARIES = codec_ilbc.la

codec_ilbc_la_SOURCES = PlgIlbc.c

# for linking ilbc from contrib or packaged version
#codec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@/.libs/libilbc.la
codec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@@ILBC_LIB_TARGET@

codec_ilbc_la_LDFLAGS = -module -avoid-version

codec_ilbc_la_DEPENDENCIES = libilbc
#
#.PHONY: libilbc
#libilbc: 	
#	make -C @ILBC_LIB_ROOT@ all

