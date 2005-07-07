## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipregistrar

sipregistrar_LDFLAGS = $(GLIB_LIBS)
sipregistrar_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipregistrar_SOURCES =  \
    main.cpp \
    SipHuntGroupFilter.cpp \
    SipImpliedSubscriptions.cpp \
    SipRedirectServer.cpp \
    SipRegistrar.cpp \
    SipRegistrarServer.cpp \
    SipHuntGroupFilter.h \
    SipImpliedSubscriptions.h \
    SipRedirectServer.h \
    SipRegistrar.h \
    SipRegistrarServer.h \
	version.h

