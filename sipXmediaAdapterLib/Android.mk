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
# This Makefile is for building sipXmediaAdapterLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXmediaAdapter
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \
    sipXmediaMediaProcessing/src/CpTopologyGraphFactoryImpl.cpp \


LOCAL_SRC_FILES := \
    sipXmediaMediaProcessing/src/sipXmediaFactoryImpl.cpp \
    sipXmediaMediaProcessing/src/CpPhoneMediaInterface.cpp \
    sipXmediaMediaProcessing/src/CpTopologyGraphInterface.cpp \
    sipXmediaMediaProcessing/src/MaNotfTranslatorDispatcher.cpp

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
    $(SIPX_HOME)/sipXmediaAdapterLib/interface \
    $(SIPX_HOME)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include

#LOCAL_SHARED_LIBRARIES := libpcre libsipXport libsipXsdp libsipXtack libsipXmedia

LOCAL_STATIC_LIBRARIES := libsipXmedia libsipXtack libsipXsdp libsipXport libpcre

LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
