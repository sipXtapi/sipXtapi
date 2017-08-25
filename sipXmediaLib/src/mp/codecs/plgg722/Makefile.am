## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPANDSP_CFLAGS@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_g722.la

codec_g722_la_SOURCES = plgg722.c

if G722_STATIC
codec_g722_la_CFLAGS  = -DCODEC_STATIC
codec_g722_la_LDFLAGS = -module -avoid-version -static
else
codec_g722_la_LIBADD = @SPANDSP_LIBS@
codec_g722_la_LDFLAGS = -module -avoid-version @SPANDSP_LDFLAGS@ 

all-local: codec_g722.la
	echo "Installing plugin codec_g722.so ..."
	cp -f .libs/codec_g722.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_g722.so

endif
