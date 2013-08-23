## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include -I$(top_srcdir)/sipXmediaLib/contrib/libspandsp/src

codeclib_LTLIBRARIES = codec_pcmapcmu.la

codec_pcmapcmu_la_SOURCES = \
	CodecPcmaWrapper.c \
	CodecPcmuWrapper.c \
	G711.c \
	PlgPcmaPcmu.c

if PCMAPCMU_STATIC
codec_pcmapcmu_la_CFLAGS  = -DCODEC_STATIC
codec_pcmapcmu_la_LDFLAGS = -module -avoid-version -static
else
codec_pcmapcmu_la_LDFLAGS = -module -avoid-version

endif
