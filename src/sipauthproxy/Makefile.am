## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipauthproxy

sipauthproxy_CXXFLAGS = $(GLIB_CFLAGS) -I. \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipauthproxy_LDADD = $(GLIB_LIBS)
sipauthproxy_SOURCES =  \
    sipauthproxymain.cpp \
    SipAaa.cpp \
	 SipAaa.h

