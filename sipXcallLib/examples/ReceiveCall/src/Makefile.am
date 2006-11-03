SUBDIRS =

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = ReceiveCall

ReceiveCall_LDADD = \
	@SIPXMEDIAADAPTER_LIBS@ \
	$(top_builddir)/src/libsipXcall.la

ReceiveCall_SOURCES = ReceiveCall.cpp
