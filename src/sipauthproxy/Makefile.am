## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipauthproxy

sipauthproxy_CXXFLAGS = $(GLIB_CFLAGS) -I. \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipauthproxy_LDADD = $(GLIB_LIBS) \
	../cse/libcse.la

sipauthproxy_SOURCES =  \
	../../config/sipxproxy-buildstamp.h \
	../../config/sipxproxy-buildstamp.cpp \
   sipauthproxymain.cpp \
   SipAaa.cpp \
	SipAaa.h \
	AuthProxyCseObserver.h \
	AuthProxyCseObserver.cpp


