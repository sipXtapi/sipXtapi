SUBDIRS =

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = PlaceCall

PlaceCall_LDADD = \
	@SIPXMEDIAADAPTER_LIBS@ \
	$(top_builddir)/src/libsipXcall.la

PlaceCall_SOURCES = PlaceCall.cpp
