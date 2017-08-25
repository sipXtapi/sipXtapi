## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @OPUS_INCLUDE@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_opus.la

codec_opus_la_SOURCES = plgopus.c
codec_opus_la_LIBADD = @OPUS_STATIC_LIB@
codec_opus_la_LDFLAGS = -module -avoid-version

all-local: codec_opus.la
	echo "Installing plugin codec_opus.so ..."
	cp -f .libs/codec_opus.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_opus.so

