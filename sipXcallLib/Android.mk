#
# Copyright (C) 2009 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2009 SIPez LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
#
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# This Makefile is for building sipXtackLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXtack
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \


LOCAL_SRC_FILES := \
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
    tao/TaoEventListener.cpp \
    tao/TaoListenerEventMessage.cpp \
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

# Not immediately needed on Android
FOO_DONT_BUILD := \


LOCAL_CXXFLAGS += -D__pingtel_on_posix__ \
                  -DANDROID \
                  -DDEFINE_S_IREAD_IWRITE \
                  -DSIPX_TMPDIR=\"/usr/var/tmp\" -DSIPX_CONFDIR=\"/etc/sipxpbx\"

#ifeq ($(TARGET_ARCH),arm)
#	LOCAL_CFLAGS += -DARMv5_ASM
#endif

#ifeq ($(TARGET_BUILD_TYPE),debug)
#	LOCAL_CFLAGS += -DDEBUG
#endif

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaVoiceEngine \
    $(SIPX_HOME)/sipXcallLib/include

LOCAL_SHARED_LIBRARIES := libpcre libsipXport libsipXsdp libsipXtack libsipXmedia libsipXmediaAdapter

#LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS += -lstdc++ -ldl

include $(BUILD_SHARED_LIBRARY)
