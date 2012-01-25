#
# Copyright (C) 2009 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2009-2012 SIPez LLC.  All rights reserved.
# Licensed to SIPfoundry under a Contributor Agreement.
#
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# This Makefile is for building sipXsdpLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXsdp
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

LOCAL_SRC_FILES := \
    src/sdp/Sdp.cpp \
    src/sdp/SdpCandidatePair.cpp \
    src/sdp/SdpCodecList.cpp \
    src/sdp/SdpHelperResip.cpp \
    src/sdp/SdpCandidate.cpp \
    src/sdp/SdpCodec.cpp \
    src/sdp/SdpDefaultCodecFactory.cpp \
    src/sdp/SdpMediaLine.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include

LOCAL_SHARED_LIBRARIES := libpcre

LOCAL_STATIC_LIBRARIES := libsipXport

LOCAL_LDLIBS += -lstdc++ -ldl -llog

LOCAL_SHARED_LIBRARIES += $(SIPX_PORT_SHARED_LIBS)
LOCAL_LDLIBS += $(SIPX_PORT_LDLIBS)
LOCAL_CFLAGS += $(SIPX_PORT_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_PORT_C_INCLUDES)

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

# Unit test framework library

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := sipxsdpunit

fails_to_compile := \


LOCAL_SRC_FILES := \
    src/test/sdp/SdpSetGetTest.cpp \
    src/test/sdp/sdpTest.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include


LOCAL_SHARED_LIBRARIES := libpcre
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXsdp libsipXport 

LOCAL_LDLIBS += -lstdc++ -ldl -llog

LOCAL_SHARED_LIBRARIES += $(SIPX_PORT_SHARED_LIBS)
LOCAL_LDLIBS += $(SIPX_PORT_LDLIBS)
LOCAL_CFLAGS += $(SIPX_PORT_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_PORT_C_INCLUDES)

include $(BUILD_EXECUTABLE)

