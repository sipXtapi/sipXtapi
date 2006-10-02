## Process this file with automake to produce Makefile.in

SUBDIRS = test

bin_PROGRAMS = sipregistrar

sipregistrar_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipregistrar_SOURCES =  \
    main.cpp

sipregistrar_LDADD = \
	libsipXregistry.la

noinst_LTLIBRARIES = libsipXregistry.la

libsipXregistry_la_CXXFLAGS = $(sipregistrar_CXXFLAGS)

libsipXregistry_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libsipXregistry_la_SOURCES = \
    $(top_builddir)/config/sipxregistry-buildstamp.h \
    $(top_builddir)/config/sipxregistry-buildstamp.cpp \
    RedirectResumeMsg.cpp \
    RedirectSuspend.cpp \
    RegistrarInitialSync.h \
    RegistrarInitialSync.cpp \
    RegistrarPersist.h \
    RegistrarPersist.cpp \
    RegistrarTest.h \
    RegistrarTest.cpp \
    RegistrarSync.h \
    RegistrarSync.cpp \
    RegistrarPeer.h \
    RegistrarPeer.cpp \
    SipRedirectorAliasDB.cpp \
    SipRedirectorCallerAlias.cpp \
    SipRedirectorHunt.cpp \
    SipRedirectorMapping.cpp \
    SipRedirectorMPT.cpp \
    SipRedirectorPickUp.cpp \
    SipRedirectorRegDB.cpp \
    SipRedirectorSubscribe.cpp \
    SipRedirectServer.cpp \
    SipRedirector.cpp \
    SipRegistrar.cpp \
    SipRegistrarServer.cpp \
    RedirectResumeMsg.h \
    RedirectSuspend.h \
    SipRedirectorAliasDB.h \
    SipRedirectorCallerAlias.h \
    SipRedirectorHunt.h \
    SipRedirectorMapping.h \
    SipRedirectorMPT.h \
    SipRedirectorPickUp.h \
    SipRedirectorRegDB.h \
    SipRedirectorSubscribe.h \
    SipRedirectServer.h \
    SipRegistrar.h \
    SipRegistrarServer.h \
    SipRedirector.h \
    SyncRpc.h \
    SyncRpc.cpp
