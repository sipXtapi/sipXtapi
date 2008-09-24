## Process this file with automake to produce Makefile.in


EXTRA_DIST = tapi

SUBDIRS = . test

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXmediaAdapterLib/sipXmediaVoiceEngine

lib_LTLIBRARIES = libsipXcall.la libsipXtapi.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

if BUILDTLS
SECURITY_LIBADD=@SSL_LIBS@
else
SECURITY_LIBADD=
endif

libsipXcall_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}
libsipXtapi_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXcall_la_LIBADD = \
	@SIPXTACK_LIBS@ \
        @SIPXPORT_LIBS@ \
        @SIPXMEDIA_PLUGIN_LIBS@ \
        -lstdc++ 

libsipXtapi_la_LIBADD = \
	@SIPXTACK_STATIC_LIBS@ \
        @SIPXPORT_STATIC_LIBS@ \
	@SIPXMEDIA_PLUGIN_STATIC_LIBS@ \
        -lstdc++ -lpthread -lresolv -lpcre \
        @SIPXPORT_STATIC_LIBS@ \
        $(SECURITY_LIBADD)

libsipXcall_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" 

libsipXtapi_la_CXXFLAGS = \
    -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" 

libsipXcall_la_SOURCES = \
    ../config/sipxcalllib-buildstamp.h \
    ../config/sipxcalllib-buildstamp.cpp \
    cp/CallManager.cpp \
    cp/CSeqManager.cpp \
    cp/Connection.cpp \
    cp/CpCall.cpp \
    cp/CpCallManager.cpp \
    cp/CpGatewayManager.cpp \
    cp/CpGhostConnection.cpp \
    cp/CpIntMessage.cpp \
    cp/CpMultiStringMessage.cpp \
    cp/CpPeerCall.cpp \
    cp/CpStringMessage.cpp \
    cp/SipConnection.cpp \
    tao/TaoEvent.cpp \
    tao/TaoMessage.cpp \
    tao/TaoObjectMap.cpp \
    tao/TaoReference.cpp \
    tao/TaoString.cpp \
    tapi/sipXtapi.cpp \
    tapi/sipXtapiEvents.cpp \
    tapi/sipXtapiInternal.cpp \
    tapi/SipXHandleMap.cpp \
    tapi/SipXEventDispatcher.cpp \
    tapi/SipXMessageObserver.cpp

libsipXtapi_la_SOURCES = \
    ../config/sipxcalllib-buildstamp.h \
    ../config/sipxcalllib-buildstamp.cpp \
    cp/CallManager.cpp \
    cp/CSeqManager.cpp \
    cp/Connection.cpp \
    cp/CpCall.cpp \
    cp/CpCallManager.cpp \
    cp/CpGatewayManager.cpp \
    cp/CpGhostConnection.cpp \
    cp/CpIntMessage.cpp \
    cp/CpMultiStringMessage.cpp \
    cp/CpPeerCall.cpp \
    cp/CpStringMessage.cpp \
    cp/SipConnection.cpp \
    tao/TaoEvent.cpp \
    tao/TaoMessage.cpp \
    tao/TaoObjectMap.cpp \
    tao/TaoReference.cpp \
    tao/TaoString.cpp \
    tapi/sipXtapi.cpp \
    tapi/SipXEventDispatcher.cpp \
    tapi/sipXtapiEvents.cpp \
    tapi/sipXtapiInternal.cpp \
    tapi/SipXHandleMap.cpp \
    tapi/SipXMessageObserver.cpp

DISTCLEANFILES = Makefile.in
