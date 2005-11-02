SUBDIRS =

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = PlaceCall

PlaceCall_LDADD = \
	@SIPXMEDIA_MP_LIBS@ \
	$(top_builddir)/src/libsipXcall.la

PlaceCall_SOURCES = PlaceCall.cpp
