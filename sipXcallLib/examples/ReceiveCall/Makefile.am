include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = ReceiveCall

INCLUDES = -Iinclude

ReceiveCall_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
    @SIPXMEDIA_MP_LIBS@ 

ReceiveCall_SOURCES = \
    src/ReceiveCall.cpp 

noinst_HEADERS = \
	include/ReceiveCall.h 
