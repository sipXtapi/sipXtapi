## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../sipXcallLib/include

bin_PROGRAMS = sipxconference

sipxconference_LDADD = \
        @SIPXCALL_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@

sipxconference_CXXFLAGS = \
        -I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/sipXconference/include \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxconference_SOURCES =  \
	../../config/sipxpbx-buildstamp.h \
	../../config/sipxpbx-buildstamp.cpp \
	main.cpp \
	Conference.cpp \
	ConferenceController.cpp \
	ConferenceControllerMsg.cpp \
	Leg.cpp

install-exec-hook :

install-data-hook :
