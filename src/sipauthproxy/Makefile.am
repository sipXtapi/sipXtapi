## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = sipauthproxy

sipauthproxy_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipauthproxy_LDFLAGS = $(GLIB_LIBS)
sipauthproxy_SOURCES =  \
    sipauthproxymain.cpp \
    SipAaa.cpp

