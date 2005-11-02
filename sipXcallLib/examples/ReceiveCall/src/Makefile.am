SUBDIRS =

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = ReceiveCall

ReceiveCall_LDADD = \
	@SIPXMEDIA_MP_LIBS@ \
	$(top_builddir)/src/libsipXcall.la

ReceiveCall_SOURCES = ReceiveCall.cpp
