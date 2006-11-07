## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipproxy_LDADD = \
	../cse/libcse.la \
	@SIPXCOMMSERVER_LIBS@

sipproxy_SOURCES =  \
	$(top_builddir)/config/sipxproxy-buildstamp.h \
	$(top_builddir)/config/sipxproxy-buildstamp.cpp \
	SipRouter.cpp \
	ForwardRules.cpp \
	SipForkingProxyMain.cpp \
	SipRouter.h \
	ForwardRules.h \
	ForkingProxyCseObserver.h \
	ForkingProxyCseObserver.cpp
