## Process this file with automake to produce Makefile.in
# When not building/running configure in sipX/sipXmediaLib
# Need to add relative include path not from topsrc
if OPUS
OPUS_CONFIG_INCLUDE=-I../../../../contrib/libopus
endif

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @OPUS_INCLUDE@ ${OPUS_CONFIG_INCLUDE}

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_opus.la

codec_opus_la_SOURCES = plgopus.c

if NE10
codec_opus_la_LIBADD = @OPUS_STATIC_LIB@ -lNE10
# TODO: add NE10 as dependency
EXTRA_codec_opus_la_DEPENDENCIES = @OPUS_STATIC_LIB@
else
codec_opus_la_LIBADD = @OPUS_STATIC_LIB@ 
EXTRA_codec_opus_la_DEPENDENCIES = @OPUS_STATIC_LIB@
endif

codec_opus_la_LDFLAGS = -module -avoid-version @SIPX_NE10_LDFLAGS@

all-local: codec_opus.la
	echo "LIBS: $(LIBS) 2 ${LIBS} 3 @SIPX_NE10_STATIC_LIB@"
	echo "Installing plugin codec_opus.so ..."
	cp -f .libs/codec_opus.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_opus.so

