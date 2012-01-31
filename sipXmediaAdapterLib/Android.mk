#
# Copyright (C) 2009-2010 SIPfoundry Inc.  All rights reserved
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2009-2010 SIPez LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
#
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# This Makefile is for building sipXmediaAdapterLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXmediaAdapter
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \

notNeededOnAndroid := \
  ./sipXmediaMediaProcessing/src/CpPhoneMediaInterface.cpp \


LOCAL_SRC_FILES := \
    interface/CpMediaInterface.cpp \
    interface/CpMediaInterfaceFactory.cpp \
    interface/CpMediaInterfaceFactoryImpl.cpp \
    interface/MiDtmfNotf.cpp \
    interface/MiIntNotf.cpp \
    interface/MiNotification.cpp \
    interface/MiProgressNotf.cpp \
    interface/MiRtpStreamActivityNotf.cpp \
    interface/MiStringNotf.cpp \
    sipXmediaMediaProcessing/src/CpTopologyGraphFactoryImpl.cpp \
    sipXmediaMediaProcessing/src/CpTopologyGraphInterface.cpp \
    sipXmediaMediaProcessing/src/MaNotfTranslatorDispatcher.cpp \
    sipXmediaMediaProcessing/src/sipXmediaFactoryImpl.cpp \


LOCAL_CFLAGS += -DDISABLE_STREAM_PLAYER \
                -DENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY \
                -DDISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY

LOCAL_CFLAGS += -DHAVE_SPEEX

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include

LOCAL_SHARED_LIBRARIES := libpcre

LOCAL_STATIC_LIBRARIES := libsipXmedia libsipXtack libsipXsdp libsipXport

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)


# =======================

# Unit test framework library

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := sipxmediaadapterunit

LOCAL_SRC_FILES := \
    sipXmediaMediaProcessing/src/test/CpCryptoTest.cpp \
    sipXmediaMediaProcessing/src/test/CpPhoneMediaInterfaceTest.cpp \


LOCAL_CFLAGS += -DDISABLE_STREAM_PLAYER \
                -DENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY \
                -DDISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/src/test \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \


LOCAL_SHARED_LIBRARIES := libpcre 
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXsdp libsipXtack libsipXmediaAdapter libsipXmedia libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_EXECUTABLE)

