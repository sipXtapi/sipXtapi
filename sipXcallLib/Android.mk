#
# Copyright (C) 2009 SIPfoundry Inc.
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
# This Makefile is for building sipXcallLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXcall
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \


LOCAL_SRC_FILES := \
    src/cp/CallManager.cpp \
    src/cp/CSeqManager.cpp \
    src/cp/Connection.cpp \
    src/cp/CpCall.cpp \
    src/cp/CpCallManager.cpp \
    src/cp/CpGatewayManager.cpp \
    src/cp/CpGhostConnection.cpp \
    src/cp/CpIntMessage.cpp \
    src/cp/CpMultiStringMessage.cpp \
    src/cp/CpPeerCall.cpp \
    src/cp/CpStringMessage.cpp \
    src/cp/SipConnection.cpp \
    src/tao/TaoEvent.cpp \
    src/tao/TaoEventListener.cpp \
    src/tao/TaoListenerEventMessage.cpp \
    src/tao/TaoMessage.cpp \
    src/tao/TaoObjectMap.cpp \
    src/tao/TaoReference.cpp \
    src/tao/TaoString.cpp \


# Not immediately needed on Android
FOO_DONT_BUILD := \

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXcallLib/include

LOCAL_SHARED_LIBRARIES := libpcre 

LOCAL_STATIC_LIBRARIES := libsipXmediaAdapter libsipXmedia libsipXtack libsipXsdp libsipXport 

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

#####################################################

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXtapi

LOCAL_SRC_FILES := \
    src/tapi/sipXtapi.cpp \
    src/tapi/SipXEventDispatcher.cpp \
    src/tapi/sipXtapiEvents.cpp \
    src/tapi/sipXtapiInternal.cpp \
    src/tapi/SipXHandleMap.cpp \
    src/tapi/SipXMessageObserver.cpp \
    src/jni/testJni.cpp \


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXcallLib/include

LOCAL_SHARED_LIBRARIES := libpcre 
LOCAL_STATIC_LIBRARIES := libsipXcall libsipXmediaAdapter libsipXmedia libsipXtack libsipXsdp libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

#####################################################

# cp unit tests

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := sipxcallunit


LOCAL_SRC_FILES := \
    src/test/cp/CallManagerTest.cpp \
    src/test/cp/CpTestSupport.cpp \

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXcallLib/include \
    $(SIPX_HOME)/sipXcallLib/src/test \

LOCAL_SHARED_LIBRARIES := libpcre

LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXcall libsipXmediaAdapter libsipXmedia libsipXtack libsipXsdp libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_EXECUTABLE)

#####################################################

# sipXtapi unit tests

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := sipxtapiunit

fails_to_compile := \
    src/test/tapi/SipSubcriptionTest.cpp \


LOCAL_SRC_FILES := \
    src/test/tapi/callbacks.cpp \
    src/test/tapi/EventRecorder.cpp \
    src/test/tapi/EventValidator.cpp \
    src/test/tapi/sipXtapiTestCall.cpp \
    src/test/tapi/sipXtapiTestConferenceAdv.cpp \
    src/test/tapi/sipXtapiTestConference.cpp \
    src/test/tapi/sipXtapiTestConfig.cpp \
    src/test/tapi/sipXtapiTest.cpp \
    src/test/tapi/sipXtapiTestLine.cpp \
    src/test/tapi/sipXtapiTestSubscribe.cpp \
    src/test/tapi/sipXtapiTestTransfer.cpp \
    src/test/tapi/TestExternalTransport.cpp \
    src/test/tapi/TestRegistrar.cpp \
    src/test/tapi/TestRegistryCollection.cpp \
    src/test/tapi/TestRegistryData.cpp \
    src/test/tapi/TestStunServerTask.cpp \


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXcallLib/include \
    $(SIPX_HOME)/sipXcallLib/src/test \

LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXcall libsipXmediaAdapter libsipXmedia libsipXtack libsipXsdp libsipXport $(SIPX_CODEC_LIBS)
LOCAL_SHARED_LIBRARIES := libsipXtapi libpcre

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_EXECUTABLE)
