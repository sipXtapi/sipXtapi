## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include -I$(top_srcdir)/contrib/libspandsp/src

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_pcmapcmu.la

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

all-local: codec_pcmapcmu.la
	echo "Installing plugin codec_pcmapcmu.so ..."
	cp -f .libs/codec_pcmapcmu.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_pcmapcmu.so
endif
