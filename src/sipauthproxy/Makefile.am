## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipauthproxy

sipauthproxy_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipauthproxy_LDADD = \
	$(top_builddir)/src/cse/libcse.la \
	@SIPXCOMMSERVER_LIBS@

sipauthproxy_SOURCES =  \
	../../config/sipxproxy-buildstamp.h \
	../../config/sipxproxy-buildstamp.cpp \
   sipauthproxymain.cpp \
   SipAaa.cpp \
	SipAaa.h \
	AuthProxyCseObserver.h \
	AuthProxyCseObserver.cpp


