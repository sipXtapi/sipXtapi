## Process this file with automake to produce Makefile.in
# When not building/running configure in sipX/sipXmediaLib
# Need to add relative include path not from topsrc
if G729
SUBDIRS = ../../../../contrib/libg729a
G729_CONFIG_INCLUDE=-I../../../../contrib/libg729a/src/interface
endif

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @G729_INCLUDE@ ${G729_CONFIG_INCLUDE}

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_g729.la

codec_g729_la_SOURCES = PlgItuG729.c

codec_g729_la_LIBADD = @G729_LIB_ROOT@/src/.libs/libg729a.la
#EXTRA_codec_g729_la_DEPENDENCIES = @G729_STATIC_LIB@

codec_g729_la_LDFLAGS = -module -avoid-version 

all-local: codec_g729.la
	echo "Installing plugin codec_g729.so ..."
	cp -f .libs/codec_g729.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_g729.so

