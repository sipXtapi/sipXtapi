## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPANDSP_CFLAGS@

lib_LTLIBRARIES = libcodec_g722.la

libcodec_g722_la_LIBADD = @SPANDSP_LIBS@

libcodec_g722_la_SOURCES = plgg722.c
libcodec_g722_la_LDFLAGS = \
	@SPANDSP_LDFLAGS@ \
	-avoid-version

