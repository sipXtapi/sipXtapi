## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include

codeclib_LTLIBRARIES = codec_l16.la

codec_l16_la_SOURCES = \
	CodecL16.c \
	CodecL16Wrapper11025.c \
	CodecL16Wrapper16000.c \
	CodecL16Wrapper22050.c \
	CodecL16Wrapper24000.c \
	CodecL16Wrapper32000.c \
	CodecL16Wrapper44100.c \
	CodecL16Wrapper48000.c \
	CodecL16Wrapper8000.c \
	PlgL16.c \
	CodecL16.h

if L16_STATIC
codec_l16_la_CFLAGS  = -DCODEC_STATIC
codec_l16_la_LDFLAGS = -module -avoid-version -static
else
codec_l16_la_LDFLAGS = -module -avoid-version

endif
