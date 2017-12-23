## Process this file with automake to produce Makefile.in
# When not building/running configure in sipX/sipXmediaLib
# Need to add relative include path not from topsrc
if G7221
SUBDIRS = ../../../../contrib/libg7221
G7221_CONFIG_INCLUDE=-I../../../../contrib/libg7221
endif

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @G7221_INCLUDE@ ${G7221_CONFIG_INCLUDE}

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_g7221.la

codec_g7221_la_SOURCES = plgg7221.c

codec_g7221_la_LIBADD = @G7221_LIB_ROOT@/src/.libs/libg722_1.la
#EXTRA_codec_g7221_la_DEPENDENCIES = @G7221_STATIC_LIB@

codec_g7221_la_LDFLAGS = -module -avoid-version 

all-local: codec_g7221.la
	echo "Installing plugin codec_g7221.so ..."
	cp -f .libs/codec_g7221.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_g7221.so

