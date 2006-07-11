## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/config

INSTALL = @INSTALL@

bin_PROGRAMS = xsdvalid

xsdvalid_LDADD = \
	@SIPXPORT_LIBS@ \
	@XERCES_LIBS@

xsdvalid_SOURCES = \
	 ../../config/sipxcommserverlib-buildstamp.h \
	 ../../config/sipxcommserverlib-buildstamp.cpp \
	xsdvalid.cpp
