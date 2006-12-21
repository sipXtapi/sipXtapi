## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxrls

EXTRA_DIST =

sipxrls_LDADD = \
        @SIPXCALL_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@ \
	@SIPXCOMMSERVER_LIBS@

sipxrls_CXXFLAGS = \
        -I$(top_builddir)/config \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxrls_SOURCES =  \
	../../config/sipxpbx-buildstamp.h \
	../../config/sipxpbx-buildstamp.cpp \
	ResourceList.cpp \
	RlsSubscribePolicy.cpp \
	main.cpp

noinst_HEADERS =

install-exec-hook :
