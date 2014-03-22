## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @SPEEX_CFLAGS@

codeclib_LTLIBRARIES = codec_speex.la

codec_speex_la_SOURCES = PlgSpeex.c speex_nb.c speex_uwb.c speex_wb.c
noinst_HEADERS = plgspeex.h

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

endif
