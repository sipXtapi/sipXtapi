#
# Copyright (C) 2009-2010 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2009-2010 SIPez LLC.  All rights reserved.
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

# For a few includes we need to hack into the path
ANDROID_CORE_PATH := /Users/dpetrie/dev/android_1.6

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXtack
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \

# Not immediately needed on Android
notNeededOnAndroid := \
src/net/MailAttachment.cpp \
src/net/MailMessage.cpp \
src/net/ProvisioningAgent.cpp \
src/net/ProvisioningAgentXmlRpcAdapter.cpp \
src/net/ProvisioningAttrList.cpp \
src/net/ProvisioningClass.cpp \
src/net/SipConfigServerAgent.cpp \
src/net/SipPimClient.cpp \
src/net/SipUserAgentStateless.cpp \
src/net/XmlRpcBody.cpp \
src/net/XmlRpcDispatch.cpp \
src/net/XmlRpcMethod.cpp \
src/net/XmlRpcRequest.cpp \
src/net/XmlRpcResponse.cpp \
  src/resparse/bzero.c \
  src/resparse/memset.c \
  src/resparse/ns_name.c \
  src/resparse/ns_netint.c \
  src/resparse/res_comp.c \
  src/resparse/res_copy.c \
  src/resparse/res_mkquery.c \
  src/resparse/res_send.c \
  src/resparse/res_query.c \
  src/resparse/res_init.c \


LOCAL_SRC_FILES := \
  src/resparse/android_res_utils.c \
  src/resparse/res_data.c \
  src/resparse/res_free.c \
  src/net/SipSrvLookup.cpp \
  src/resparse/res_info.c \
  src/resparse/res_naptr.c \
  src/resparse/res_parse.c \
  src/resparse/res_print.c \
  src/net/HttpBody.cpp \
  src/net/HttpConnection.cpp \
  src/net/HttpConnectionMap.cpp \
  src/net/HttpMessage.cpp \
  src/net/HttpRequestContext.cpp \
  src/net/HttpServer.cpp \
  src/net/HttpService.cpp \
  src/net/MimeBodyPart.cpp \
  src/net/NameValuePair.cpp \
  src/net/NameValuePairInsensitive.cpp \
  src/net/NetAttributeTokenizer.cpp \
  src/net/NetBase64Codec.cpp \
  src/net/NetMd5Codec.cpp \
  src/net/PidfBody.cpp \
  src/net/SdpBody.cpp \
  src/net/SdpHelper.cpp \
  src/net/SipClient.cpp \
  src/net/SipContactDb.cpp \
  src/net/SipDialog.cpp \
  src/net/SipDialogEvent.cpp \
  src/net/SipDialogMgr.cpp \
  src/net/SipDialogMonitor.cpp \
  src/net/SipLine.cpp \
  src/net/SipLineCredentials.cpp \
  src/net/SipLineEvent.cpp \
  src/net/SipLineList.cpp \
  src/net/SipLineMgr.cpp \
  src/net/SipMessage.cpp \
  src/net/SipMessageEvent.cpp \
  src/net/SipMessageList.cpp \
  src/net/SipNonceDb.cpp \
  src/net/SipNotifyStateTask.cpp \
  src/net/SipObserverCriteria.cpp \
  src/net/SipPresenceEvent.cpp \
  src/net/SipProtocolServerBase.cpp \
  src/net/SipPublishContentMgr.cpp \
  src/net/SipPublishServer.cpp \
  src/net/SipPublishServerEventStateCompositor.cpp \
  src/net/SipPublishServerEventStateMgr.cpp \
  src/net/SipRefreshManager.cpp \
  src/net/SipRefreshMgr.cpp \
  src/net/SipRequestContext.cpp \
  src/net/SipResourceList.cpp \
  src/net/SipServerBase.cpp \
  src/net/SipServerBroker.cpp \
  src/net/SipSession.cpp \
  src/net/SipSubscribeClient.cpp \
  src/net/SipSubscribeServer.cpp \
  src/net/SipSubscribeServerEventHandler.cpp \
  src/net/SipSubscriptionMgr.cpp \
  src/net/SipTcpServer.cpp \
  src/net/SipTlsServer.cpp \
  src/net/SipTransaction.cpp \
  src/net/SipTransactionList.cpp \
  src/net/SipUdpServer.cpp \
  src/net/SipUserAgent.cpp \
  src/net/SipUserAgentBase.cpp \
  src/net/SmimeBody.cpp \
  src/net/StateChangeNotifier.cpp \
  src/net/TapiMgr.cpp \
  src/net/Url.cpp \
  src/net/pk12wrapper.cpp 

LOCAL_CFLAGS += \
                  -D__pingtel_on_posix__ \
                  -DANDROID \
                  -DDEFINE_S_IREAD_IWRITE \

bionic_compile_flags := \
                -DWITH_ERRLIST                  \
                -DANDROID_CHANGES               \
                -DUSE_LOCKS                     \
                -DREALLOC_ZERO_BYTES_FREES      \
                -D_LIBC=1                       \
                -DSOFTFLOAT                     \
                -DFLOATING_POINT                \
                -DNEED_PSELECT=1                \
                -DINET6 \
                -I$(LOCAL_PATH)/private \
                -DUSE_DL_PREFIX



LOCAL_CXXFLAGS += \
                  -DSIPX_TMPDIR=\"/usr/var/tmp\" -DSIPX_CONFDIR=\"/etc/sipxpbx\"

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include

# Need to hack things a bit as resolv code is packaged in Bionic (Android clib)
LOCAL_C_INCLUDES += \
    $(ANDROID_CORE_PATH)/bionic/libc/private

#LOCAL_SHARED_LIBRARIES := libpcre libsipXport libsipXsdp

LOCAL_STATIC_LIBRARIES := libsipXsdp libsipXport libpcre 

LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

# Unit test framework library

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := sipxtackunit

fails_to_compile := \

# Not immediately needed
notNeededSource := \
    src/test/net/XmlRpcTest.cpp \

LOCAL_SRC_FILES := \
    src/test/net/HttpBodyTest.cpp \
    src/test/net/HttpMessageTest.cpp \
    src/test/net/NameValuePairInsensitiveTest.cpp \
    src/test/net/NameValuePairTest.cpp \
    src/test/net/NetAttributeTokenizerTest.cpp \
    src/test/net/NetBase64CodecTest.cpp \
    src/test/net/NetMd5CodecTest.cpp \
    src/test/net/SdpBodyTest.cpp \
    src/test/net/SipContactDbTest.cpp \
    src/test/net/SipDialogEventTest.cpp \
    src/test/net/SipDialogMonitorTest.cpp \
    src/test/net/SipDialogTest.cpp \
    src/test/net/SipMessageTest.cpp \
    src/test/net/SipPresenceEventTest.cpp \
    src/test/net/SipProxyMessageTest.cpp \
    src/test/net/SipPublishContentMgrTest.cpp \
    src/test/net/SipRefreshManagerTest.cpp \
    src/test/net/SipServerShutdownTest.cpp \
    src/test/net/SipSrvLookupTest.cpp \
    src/test/net/SipSubscribeServerTest.cpp \
    src/test/net/SipSubscriptionClientTest.cpp \
    src/test/net/SipSubscriptionMgrTest.cpp \
    src/test/net/SipUserAgentTest.cpp \
    src/test/net/UrlTest.cpp \
    src/test/SdpHelperTest.cpp \


LOCAL_CFLAGS += \
                  -D__pingtel_on_posix__ \
                  -DANDROID \
                  -DDEFINE_S_IREAD_IWRITE \


LOCAL_CXXFLAGS += \
                  -DSIPX_TMPDIR=\"/usr/var/tmp\" \
                  -DSIPX_CONFDIR=\"/etc/sipx\" \
                  -DTEST_DIR=\"/tmp\"

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include

# Need to hack things a bit as resolv code is packaged in Bionic (Android clib)
LOCAL_C_INCLUDES += \
    $(ANDROID_CORE_PATH)/bionic/libc/private

#LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXsdp libsipXtack libsipXport libpcre 

LOCAL_LDLIBS += -lstdc++ -ldl

include $(BUILD_EXECUTABLE)

