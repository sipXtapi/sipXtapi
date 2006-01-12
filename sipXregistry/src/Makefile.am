## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipregistrar

sipregistrar_LDADD = \
	@SIPXCOMMSERVER_LIBS@

sipregistrar_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipregistrar_SOURCES =  \
    ../config/sipxregistry-buildstamp.h \
    ../config/sipxregistry-buildstamp.cpp \
    main.cpp \
    RedirectResumeMsg.cpp \
    RedirectSuspend.cpp \
	 RegistrarInitialSync.h \
    RegistrarTest.h \
    RegistrarSync.h \
    RegistrarPeer.h \
    RegistrarPeer.cpp \
    SipRedirectorAliasDB.cpp \
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
