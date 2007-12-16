## Process this file with automake to produce Makefile.in

SUBDIRS=../../../../contrib

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPEEX_CFLAGS@

lib_LTLIBRARIES = libcodec_speex.la

libcodec_speex_la_LDFLAGS = \
	-avoid-version

libcodec_speex_la_LIBADD = \
	@SPEEX_LIBS@

libcodec_speex_la_SOURCES = PlgSpeex.c speex_nb.c speex_uwb.c speex_wb.c

