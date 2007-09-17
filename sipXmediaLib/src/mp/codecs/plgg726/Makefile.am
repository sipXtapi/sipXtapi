## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @SPANDSP_CFLAGS@

lib_LTLIBRARIES = libcodec_g726.la

libcodec_g726_la_LIBADD = @SPANDSP_LIBS@

libcodec_g726_la_SOURCES = g726-16.c  g726-24.c  g726-32.c  g726-40.c  plgg726.c
libcodec_g726_la_LDFLAGS = \
	@SPANDSP_LDFLAGS@ \
	-avoid-version

