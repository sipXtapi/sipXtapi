## Process this file with automake to produce Makefile.in

INSTALL=@INSTALL@

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = watchdog

watchdog_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"
watchdog_LDFLAGS = $(GLIB_LIBS)
watchdog_SOURCES =  \
    EmailReporter.cpp \
    FailureReporterBase.cpp \
    WatchDogMain.cpp \
    MonitoredProcess.cpp \
    WatchDog.cpp

watchdog_LDADD = \
    ../processcgi/libprocesscgi.la \
    @CGICC_DIR@/lib/libcgicc.la

noinst_HEADERS = \
	EmailReporter.h \
	FailureReporterBase.h \
	MonitoredProcess.h \
	WatchDog.h

edit = sed \
        -e 's,@prefix\@,$(prefix),g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g'

EXTRA_DIST = watchdog.sh.in

install-exec-hook :
	$(edit) $(srcdir)/watchdog.sh.in > watchdog.sh
	$(INSTALL) -D -m 755 watchdog.sh $(DESTDIR)$(bindir)/watchdog.sh
