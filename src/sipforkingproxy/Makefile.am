## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = $(GLIB_CFLAGS) -DSIPX_HOME=\"@SIPX_HOME@\"
sipproxy_LDFLAGS = $(GLIB_LIBS)
sipproxy_SOURCES =  \
    SipRouter.cpp \
    ForwardRules.cpp \
    SipForkingProxyMain.cpp

