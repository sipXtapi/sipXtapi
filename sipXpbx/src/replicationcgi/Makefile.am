## Process this file with automake to produce Makefile.in

cgidir=@wwwdir@/cgi-bin

replicationcgidir = $(cgidir)/replication
replicationcgi_PROGRAMS = replication.cgi
replication_cgi_LDADD= @CGICC_DIR@/lib/libcgicc.la
replication_cgi_CXXFLAGS = \
	$(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

replication_cgi_LDFLAGS = $(GLIB_LIBS)
replication_cgi_SOURCES = main.cpp

