## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = sipauthproxy

sipauthproxy_CXXFLAGS = $(GLIB_CFLAGS)
sipauthproxy_LDFLAGS = $(GLIB_LIBS)
sipauthproxy_SOURCES =  \
    sipauthproxymain.cpp \
    SipAaa.cpp

