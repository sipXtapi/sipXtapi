## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include

pkgdata_LTLIBRARIES = codec_pcmapcmu.la

codec_pcmapcmu_la_SOURCES = \
	CodecPcmaWrapper.c \
	CodecPcmuWrapper.c \
	G711.c \
	PlgPcmaPcmu.c

codec_pcmapcmu_la_LDFLAGS = -module -avoid-version

all-local: codec_pcmapcmu.la
	echo "Installing plugin codec_pcmapcmu.so ..."
	cp -f .libs/codec_pcmapcmu.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_pcmapcmu.so

