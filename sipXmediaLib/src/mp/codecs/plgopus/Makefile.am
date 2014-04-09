## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @OPUS_CFLAGS@

codeclib_LTLIBRARIES = codec_opus.la

codec_opus_la_SOURCES = plgopus.c

if OPUS_STATIC
codec_opus_la_CFLAGS  = -DCODEC_STATIC
codec_opus_la_LDFLAGS = -module -avoid-version -static
# Below LIBADD line will pull in a statically built libopus library.
# Since this library is a shared library, the static opus library needs
# to be built with -fPIC -DPIC
codec_opus_la_LIBADD = @OPUS_STATIC_LIB@
else
# Below LIBADD line will pull in a statically built libopus library.
# Since this library is a shared library, the static opus library needs
# to be built with -fPIC -DPIC
#codec_opus_la_LIBADD = @OPUS_STATIC_LIB@

# This will pull in shared lib, which automatically has -fPIC
codec_opus_la_LIBADD = @OPUS_LIBS@

codec_opus_la_LDFLAGS = -module -avoid-version

endif
