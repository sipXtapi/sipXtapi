## Process this file with automake to produce Makefile.in

SUBDIRS = ../../../../contrib/libilbc

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @ILBC_INCLUDE@

pkgdata_LTLIBRARIES = codec_ilbc.la

codec_ilbc_la_SOURCES = PlgIlbc.c
codec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@/.libs/libilbc.la
codec_ilbc_la_LDFLAGS = -module -avoid-version

# Currently only contrib ilbc library supported
# uncoment lines if you want to use standard Makefile
#
#codec_ilbc_la_LIBADD = @ILBC_LIB_ROOT@@ILBC_LIB_TARGET@
#codec_ilbc_la_DEPENDENCIES = libilbc
#
#.PHONY: libilbc
#libilbc: 	
#	make -C @ILBC_LIB_ROOT@ all

all-local: codec_ilbc.la
	echo "Installing plugin codec_ilbc.so ..."
	cp -f .libs/codec_ilbc.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_ilbc.so

