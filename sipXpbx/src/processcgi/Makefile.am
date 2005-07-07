
INCLUDES = \
    -I$(top_srcdir)/include \
    -I@CGICC_DIR@/include

noinst_LTLIBRARIES = libprocesscgi.la
libprocesscgi_la_CXXFLAGS = $(GLIB_CFLAGS)
libprocesscgi_la_SOURCES = \
    processCommon.cpp \
    DependencyList.cpp

cgidir=@wwwdir@/cgi-bin
processmonitordir = $(cgidir)/processmonitor
processmonitor_PROGRAMS = process.cgi
process_cgi_CXXFLAGS = \
	$(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"
process_cgi_CFLAGS = $(process_cgi_CXXFLAGS)

process_cgi_LDFLAGS = $(GLIB_LIBS)
process_cgi_SOURCES = main.cpp
process_cgi_LDADD = \
    libprocesscgi.la \
    @CGICC_DIR@/lib/libcgicc.la


