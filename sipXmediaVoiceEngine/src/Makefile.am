## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/sipXmediaVoiceEngine \
	-I$(top_srcdir)/../../contrib/gips/VoiceEngine/interface

lib_LTLIBRARIES = libsipXvoiceEngine.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXvoiceEngine_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age} \
	--whole-archive \
        @SIPXMEDIA_PLUGIN_STATIC_LIBS@ \
        @SIPXSDP_STATIC_LIBS@ \
        @SIPXTACK_STATIC_LIBS@ \
        @SIPXPORT_STATIC_LIBS@ \
	--no-whole-archive

libsipXvoiceEngine_la_CXXFLAGS = \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DUSE_GIPS \
	-Wl,-no_implicit_dylibs \
	-Wl,-read_only_relocs,suppress 

libsipXvoiceEngine_la_LIBADD = \
	-lresolv -lpthread -lresolv -lpcre \
        @GIPS_VEMM_LIBS@ \
        @CA_LIBS@ \
        @CARBON_LIBS@

libsipXvoiceEngine_la_SOURCES =  \
    VoiceEngine.cpp \
    VideoEngine.cpp \
    VoiceEngineBufferInStream.cpp \
    VoiceEngineConnectionSocket.cpp \
    VoiceEngineDatagramSocket.cpp \
    VoiceEngineFactoryImpl.cpp \
    VoiceEngineMediaInterface.cpp \
    VoiceEngineMediaInterfaceBase.cpp \
    VoiceEnginePacketPusher.cpp \
    VoiceEngineSocketAdapter.cpp \
    VoiceEngineSocketFactory.cpp \
    ArsVoiceEngineReceiveAdapter.cpp
