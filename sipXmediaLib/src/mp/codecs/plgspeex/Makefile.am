## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPEEX_CFLAGS@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_speex.la

if SPEEX_CONTRIB_ENABLED
speex_utils =
else
speex_utils = bits_extensions.c
endif

codec_speex_la_SOURCES= PlgSpeex.c speex_nb.c speex_uwb.c speex_wb.c $(speex_utils)

if SPEEX_STATIC
codec_speex_la_CFLAGS  = -DCODEC_STATIC
codec_speex_la_LDFLAGS = -module -avoid-version -static
# Below LIBADD line will pull in a statically built libspeex library.
# Since this library is a shared library, the static speex library needs
# to be built with -fPIC -DPIC
codec_speex_la_LIBADD = @SPEEX_STATIC_LIB@
else
# Below LIBADD line will pull in a statically built libspeex library.
# Since this library is a shared library, the static speex library needs
# to be built with -fPIC -DPIC
#codec_speex_la_LIBADD = @SPEEX_STATIC_LIB@ @SPEEXDSP_STATIC_LIB@

# This will pull in shared lib, which automatically has -fPIC
codec_speex_la_LIBADD = @SPEEX_LIBS@ @SPEEXDSP_LIBS@

codec_speex_la_LDFLAGS = -module -avoid-version

all-local: codec_speex.la
	echo "Installing plugin codec_speex.so ..."
	cp -f .libs/codec_speex.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_speex.so

endif
