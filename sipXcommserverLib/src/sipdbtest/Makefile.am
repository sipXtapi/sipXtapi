## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb

INSTALL = @INSTALL@

bin_PROGRAMS = sipdbtest

sipdbtest_LDFLAGS = $(top_builddir)/src/libsipXcommserver.la @PCRE_LDFLAGS@ $(GLIB_LIBS)
sipdbtest_LDADD = $(top_builddir)/src/libsipXcommserver.la @PCRE_LIBS@

sipdbtest_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
   -DSIPXCHANGE_USERNAME=\"@SIPXPBXUSER@\"

sipdbtest_SOURCES = \
    DisplayTask.cpp \
    HeartBeatMonitor.cpp \
    IMDBTaskMonitor.cpp \
    IMDBWorkerTask.cpp \
    ImportTask.cpp \
    KeepAliveTask.cpp \
    main.cpp \
    DisplayTask.h \
    HeartBeatMonitor.h \
    IMDBTaskMonitor.h \
    IMDBWorkerTask.h \
    ImportTask.h \
    KeepAliveTask.h

EXTRA_DIST = \
    sipdbtest.sh.in

edit = sed \
    -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
    -e 's,@localstatedir\@,$(localstatedir),g' \
    -e 's,@bindir\@,$(bindir),g'

install-exec-hook :
	$(edit) $(srcdir)/sipdbtest.sh.in > sipdbtest.sh
	$(INSTALL) -D -m 755 sipdbtest.sh $(DESTDIR)$(bindir)/sipdbtest.sh

