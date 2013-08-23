## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXtackLib/include -I$(top_srcdir)/sipXsdpLib/include -I$(top_srcdir)/sipXportLib/include

bin_PROGRAMS = syslog2siptrace

syslog2siptrace_LDADD = $(top_builddir)/sipXtackLib/src/libsipXtack.la $(top_builddir)/sipXsdpLib/src/libsipXsdp.la $(top_builddir)/sipXportLib/src/libsipXport.la

syslog2siptrace_SOURCES = main.cpp

bin_SCRIPTS = siptrace-merge
