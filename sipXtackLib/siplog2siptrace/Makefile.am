## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = siplog2siptrace
siplog2siptrace_LDFLAGS = $(top_builddir)/src/libsipXtack.la ${GLIB_LIBS}
siplog2siptrace_CXXFLAGS = $(GLIB_CFLAGS)
siplog2siptrace_SOURCES =  main.cpp
