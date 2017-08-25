## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_tones.la

codec_tones_la_SOURCES = PlgTones.c

if TONES_STATIC
codec_tones_la_CFLAGS  = -DCODEC_STATIC
codec_tones_la_LDFLAGS = -module -avoid-version -static
else
codec_tones_la_LDFLAGS = -module -avoid-version


all-local: codec_tones.la
	echo "Installing plugin codec_tones.so ..."
	cp -f .libs/codec_tones.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_tones.so

endif
