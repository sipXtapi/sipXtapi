## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = $(GLIB_CFLAGS)
sipproxy_LDFLAGS = $(GLIB_LIBS)
sipproxy_SOURCES =  \
    SipRouter.cpp \
    ForwardRules.cpp \
    SipForkingProxyMain.cpp

