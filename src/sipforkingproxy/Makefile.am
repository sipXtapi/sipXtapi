## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipproxy_LDFLAGS = $(GLIB_LIBS)
sipproxy_SOURCES =  \
    SipRouter.cpp \
    ForwardRules.cpp \
    SipForkingProxyMain.cpp \
	 SipRouter.h \
	 ForwardRules.h

