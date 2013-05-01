## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include

codeclib_LTLIBRARIES = codec_tones.la

codec_tones_la_SOURCES = PlgTones.c

if TONES_STATIC
codec_tones_la_CFLAGS  = -DCODEC_STATIC
codec_tones_la_LDFLAGS = -module -avoid-version -static
else
codec_tones_la_LDFLAGS = -module -avoid-version


endif
