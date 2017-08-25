## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPANDSP_CFLAGS@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_g726.la

codec_g726_la_SOURCES = g726-16.c  g726-24.c  g726-32.c  g726-40.c  plgg726.c

if G726_STATIC
codec_g726_la_CFLAGS  = -DCODEC_STATIC
codec_g726_la_LDFLAGS = -module -avoid-version -static
else
codec_g726_la_LIBADD = @SPANDSP_LIBS@
codec_g726_la_LDFLAGS = -module -avoid-version @SPANDSP_LDFLAGS@ 

all-local: codec_g726.la
	echo "Installing plugin codec_g726.so ..."
	cp -f .libs/codec_g726.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_g726.so

endif
