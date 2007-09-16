## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @GSM_CFLAGS@

lib_LTLIBRARIES = libcodec_gsm.la

libcodec_gsm_la_LDFLAGS = \
	-avoid-version \
	@GSM_LDFLAGS@

libcodec_gsm_la_LIBADD = \
	@GSM_LIBS@

libcodec_gsm_la_SOURCES = PlgGsm.c

