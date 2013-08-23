## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @SPANDSP_CFLAGS@

codeclib_LTLIBRARIES = codec_g726.la

codec_g726_la_SOURCES = g726-16.c  g726-24.c  g726-32.c  g726-40.c  plgg726.c

if G726_STATIC
codec_g726_la_CFLAGS  = -DCODEC_STATIC
codec_g726_la_LDFLAGS = -module -avoid-version -static
else
codec_g726_la_LIBADD = @SPANDSP_LIBS@
codec_g726_la_LDFLAGS = -module -avoid-version @SPANDSP_LDFLAGS@ 

endif
