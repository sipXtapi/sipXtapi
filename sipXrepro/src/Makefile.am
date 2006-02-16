## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = sipxrepro

sipxrepro_LDADD = \
	@SIPXCOMMSERVER_LIBS@ \
	@REPRO_LIBS@ \
	@RESIPROCATE_LIBS@

sipxrepro_LDFLAGS = \
	@RESIPROCATE_LDFLAGS@ \
	@REPRO_LDFLAGS@

sipxrepro_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/lib \
	-UHAVE_CONFIG_H \
	@RESIPROCATE_CXXFLAGS@ \
	-DSIPX_REPRO \
	-UUSE_MYSQL \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxrepro_SOURCES =  \
   ../config/sipxrepro-buildstamp.h \
   ../config/sipxrepro-buildstamp.cpp \
   sipxrepro.cpp \
	RedirectRouter.h \
	RedirectRouter.cpp \
	RedirectServiceRecurse.h \
	RedirectServiceRecurse.cpp

