## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @GSM_CFLAGS@

codeclib_LTLIBRARIES = codec_gsm.la

codec_gsm_la_SOURCES = PlgGsm.c
codec_gsm_la_LIBADD = @GSM_LIBS@
codec_gsm_la_LDFLAGS = -module -avoid-version @GSM_LDFLAGS@

