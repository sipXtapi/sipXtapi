## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @GSM_CFLAGS@

pkglib_LTLIBRARIES = codec_gsm.la

codec_gsm_la_SOURCES = PlgGsm.c
codec_gsm_la_LIBADD = @GSM_LIBS@
codec_gsm_la_LDFLAGS = -module -avoid-version @GSM_LDFLAGS@

all-local: codec_gsm.la
	echo "Installing plugin codec_gsm.so ..."
	cp -f .libs/codec_gsm.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_gsm.so

