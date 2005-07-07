## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = syslog2siptrace
syslog2siptrace_LDFLAGS = $(top_builddir)/src/libsipXtack.la ${GLIB_LIBS}
syslog2siptrace_CXXFLAGS = $(GLIB_CFLAGS)
syslog2siptrace_SOURCES =  main.cpp
