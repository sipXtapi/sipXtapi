INCLUDES = \
    -I$(top_srcdir)/include \
    @CGICC_CFLAGS@

noinst_LTLIBRARIES = libprocesscgi.la

# do not do versioning for noinst libraries

libprocesscgi_la_CXXFLAGS = 

libprocesscgi_la_SOURCES = \
    processCommon.cpp \
    DependencyList.cpp

libprocesscgi_la_LIBADD = \
	../sipxcgi/libsipxcgi.la \
	@SIPXPORT_LIBS@

cgidir=@wwwdir@/cgi-bin
processmonitordir = $(cgidir)/processmonitor

processmonitor_PROGRAMS = process.cgi

process_cgi_CXXFLAGS = \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

process_cgi_CFLAGS = $(process_cgi_CXXFLAGS)

process_cgi_SOURCES = \
	../../config/sipxpbx-buildstamp.h \
	../../config/sipxpbx-buildstamp.cpp \
	main.cpp

process_cgi_LDADD = \
    libprocesscgi.la
