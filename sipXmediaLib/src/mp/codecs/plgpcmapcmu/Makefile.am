## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include

lib_LTLIBRARIES = libcodec_pcmapcmu.la

libcodec_pcmapcmu_la_SOURCES = \
	CodecPcmaWrapper.c \
	CodecPcmuWrapper.c \
	G711.c \
	PlgPcmaPcmu.c

libcodec_pcmapcmu_la_LDFLAGS = \
	-avoid-version

