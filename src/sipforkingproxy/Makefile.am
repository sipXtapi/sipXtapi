## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = $(GLIB_CFLAGS) \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipproxy_LDADD = \
	$(GLIB_LIBS) \
	../cse/libcse.la

sipproxy_SOURCES =  \
   SipRouter.cpp \
   ForwardRules.cpp \
   SipForkingProxyMain.cpp \
	SipRouter.h \
	ForwardRules.h \
	ForkingProxyCseObserver.h \
	ForkingProxyCseObserver.cpp

