## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

lib_LTLIBRARIES = libsipXmediaPluginMgr.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXmediaPluginMgr_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXmediaPluginMgr_la_CXXFLAGS = \
	-D_REALTIME_LINUX_AUDIO_THREADS \
	$(LOCAL_AUDIO_CXXFLAGS) 

libsipXmediaPluginMgr_la_LIBADD = \
	@SIPXTACK_LIBS@ \
	@SIPXPORT_LIBS@ \
	-lstdc++

libsipXmediaPluginMgr_la_SOURCES =  \
    ../config/sipxmedialibpluginmgr-buildstamp.h \
    ../config/sipxmediapluginmgr-buildstamp.cpp \
    mediaPlugin/MediaPluginFactory.cpp \
    mediaBaseImpl/CpMediaConnection.cpp \
    mediaBaseImpl/CpMediaConnectionSocket.cpp \
    mediaBaseImpl/CpMediaDeviceMgr.cpp \
    mediaBaseImpl/CpMediaInStream.cpp \
    mediaBaseImpl/CpMediaInterface.cpp \
    mediaBaseImpl/CpMediaNetTask.cpp \
    mediaBaseImpl/CpMediaServer.cpp \
    mediaBaseImpl/CpMediaSocketAdapter.cpp \
    mediaBaseImpl/CpMediaSocketMonitorTask.cpp \
    mediaBaseImpl/MediaDeviceInfo.cpp \
    mediaBaseImpl/MediaConnectivityInfo.cpp \
    ARS/SnacCodec.cpp \
    ARS/ArsConnectionSocket.cpp \
    ARS/ArsReceiveAdapter.cpp
