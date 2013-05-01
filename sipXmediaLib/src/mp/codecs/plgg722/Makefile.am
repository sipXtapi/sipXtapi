## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @SPANDSP_CFLAGS@

codeclib_LTLIBRARIES = codec_g722.la

codec_g722_la_SOURCES = plgg722.c

if G722_STATIC
codec_g722_la_CFLAGS  = -DCODEC_STATIC
codec_g722_la_LDFLAGS = -module -avoid-version -static
else
codec_g722_la_LIBADD = @SPANDSP_LIBS@
codec_g722_la_LDFLAGS = -module -avoid-version @SPANDSP_LDFLAGS@ 

endif
