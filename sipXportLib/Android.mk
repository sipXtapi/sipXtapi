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
# This Makefile is for building sipXportLib, unit test framwork and unit test
# as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXport
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

not_used_on_android := \
    src/os/linux/OsRWMutexLinux.cpp \


LOCAL_SRC_FILES := \
    src/os/shared/OsRWMutexShared.cpp \
    config/sipxportlib-buildstamp.h \
    config/sipxportlib-buildstamp.cpp \
    src/utl/UtlLongLongInt.cpp \
    src/utl/UtlNameValueTokenizer.cpp \
    src/utl/UtlBool.cpp \
    src/utl/UtlCrc32.cpp \
    src/utl/UtlCryptoData.cpp \
    src/utl/UtlCryptoKey.cpp \
    src/utl/UtlCryptoKeyRsa.cpp \
    src/utl/UtlCryptoKeySym.cpp \
    src/utl/UtlDateTime.cpp \
    src/utl/UtlLink.cpp \
    src/utl/UtlVoidPtr.cpp \
    src/utl/UtlHistogram.cpp \
    src/utl/UtlInit.cpp \
    src/utl/UtlInt.cpp \
    src/utl/UtlIntPtr.cpp \
    src/utl/UtlString.cpp \
    src/utl/UtlChainPool.cpp \
    src/utl/UtlContainable.cpp \
    src/utl/UtlContainer.cpp \
    src/utl/UtlContainableAtomic.cpp \
    src/utl/UtlContainablePair.cpp \
    src/utl/UtlIterator.cpp \
    src/utl/UtlList.cpp \
    src/utl/UtlListIterator.cpp \
    src/utl/UtlSList.cpp \
    src/utl/UtlCopyableContainable.cpp \
    src/utl/UtlCopyableSList.cpp \
    src/utl/UtlSListIterator.cpp \
    src/utl/UtlDList.cpp \
    src/utl/UtlDListIterator.cpp \
    src/utl/UtlCopyableSortedList.cpp \
    src/utl/UtlSortedList.cpp \
    src/utl/UtlSortedListIterator.cpp \
    src/utl/UtlHashMap.cpp \
    src/utl/UtlHashMapIterator.cpp \
    src/utl/UtlHashBag.cpp \
    src/utl/UtlHashBagIterator.cpp \
    src/utl/UtlRscStore.cpp \
    src/utl/UtlRegex.cpp \
    src/utl/UtlTokenizer.cpp \
    src/utl/XmlContent.cpp \
    src/utl/UtlRandom.cpp \
    src/utl/PluginHooks.cpp \
    src/os/NatMsg.cpp \
    src/os/OsAssert.c \
    src/os/OsCallback.cpp \
    src/os/OsConfigDb.cpp \
    src/os/OsContact.cpp \
    src/os/OsContactList.cpp \
    src/os/OsConnectionSocket.cpp \
    src/os/OsCSemBase.cpp \
    src/os/OsDatagramSocket.cpp \
    src/os/OsDateTime.cpp \
    src/os/OsDirBase.cpp \
    src/os/OsEncryption.cpp \
    src/os/OsEvent.cpp \
    src/os/OsEventMsg.cpp \
    src/os/OsExcept.cpp \
    src/os/OsFileBase.cpp \
    src/os/OsFileInfoBase.cpp \
    src/os/OsFileIteratorBase.cpp \
    src/os/OsFileSystem.cpp \
    src/os/OsLockingList.cpp \
    src/os/OsIntPtrMsg.cpp \
    src/os/OsMsg.cpp \
    src/os/OsMsgPool.cpp \
    src/os/OsMsgQ.cpp \
    src/os/OsMsgDispatcher.cpp \
    src/os/OsMulticastSocket.cpp \
    src/os/OsNameDb.cpp \
    src/os/OsNameDbInit.cpp \
    src/os/OsNatAgentTask.cpp \
    src/os/OsNatDatagramSocket.cpp \
    src/os/OsNatConnectionSocket.cpp \
    src/os/OsNatSocketBaseImpl.cpp \
    src/os/OsPathBase.cpp \
    src/os/osPrintf.cpp \
    src/os/OsProcessBase.cpp \
    src/os/OsProcessIteratorBase.cpp \
    src/os/OsProcessMgr.cpp \
    src/os/OsProtectEvent.cpp \
    src/os/OsProtectEventMgr.cpp \
    src/os/OsProtectEventMgrInit.cpp \
    src/os/OsPtrMsg.cpp \
    src/os/OsQueuedEvent.cpp \
    src/os/OsRpcMsg.cpp \
    src/os/OsServerSocket.cpp \
    src/os/OsServerTask.cpp \
    src/os/OsSharedLibMgr.cpp \
    src/os/OsSocket.cpp \
    src/os/OsSocketCrypto.cpp \
    src/os/OsSSL.cpp \
    src/os/OsSSLConnectionSocket.cpp \
    src/os/OsSSLServerSocket.cpp \
    src/os/OsSysLog.cpp \
    src/os/OsSysLogFacilities.cpp \
    src/os/OsSysLogMsg.cpp \
    src/os/OsSysLogTask.cpp \
    src/os/OsTask.cpp \
    src/os/OsTime.cpp \
    src/os/OsTimeLog.cpp \
    src/os/OsTimer.cpp \
    src/os/OsTimerMsg.cpp \
    src/os/OsTimerTask.cpp \
    src/os/OsTLSServerSocket.cpp \
    src/os/OsTLSConnectionSocket.cpp \
    src/os/OsTLSClientConnectionSocket.cpp \
    src/os/OsTokenizer.cpp \
    src/os/OsUtil.cpp \
    src/os/StunMessage.cpp \
    src/os/StunUtils.cpp \
    src/os/TurnMessage.cpp \
    src/os/shared/OsMsgQShared.cpp \
    src/os/shared/OsTimerMessage.cpp \
    src/os/linux/clock_gettime.c \
    src/os/linux/host_address.c \
    src/os/linux/AdapterInfo.cpp \
    src/os/linux/OsBSemLinux.cpp \
    src/os/linux/OsCSemLinux.cpp \
    src/os/linux/OsDateTimeLinux.cpp \
    src/os/linux/OsDirLinux.cpp \
    src/os/linux/OsFileInfoLinux.cpp \
    src/os/linux/OsFileIteratorLinux.cpp \
    src/os/linux/OsFileLinux.cpp \
    src/os/linux/OsMutexLinux.cpp \
    src/os/linux/OsPathLinux.cpp \
    src/os/linux/OsProcessIteratorLinux.cpp \
    src/os/linux/OsProcessLinux.cpp \
    src/os/linux/OsSharedLibMgrLinux.cpp \
    src/os/linux/OsTaskLinux.cpp \
    src/os/linux/OsUtilLinux.cpp \
    src/os/linux/pt_csem.c \
    src/os/linux/pt_mutex.c \
    src/xmlparser/tinystr.cpp \
    src/xmlparser/tinyxml.cpp \
    src/xmlparser/tinyxmlerror.cpp \
    src/xmlparser/tinyxmlparser.cpp \
    src/xmlparser/TiXmlIterator.cpp \
    src/xmlparser/ExtractContent.cpp \
    src/hmac-sha1/hmac-sha1.c \
    src/hmac-sha1/sha1.c 

# Would like to use native threads instead of sipX mutex and sem, but pthread_mutex_timedlock is not
# in the bionic lib for 1.6 though it is in the header file
#LOCAL_CFLAGS += -DSIPX_USE_NATIVE_PTHREADS


sipXportLib/config/sipxportlib-buildstamp.cpp:
	( cd $SIPX_HOME; scripts/makeBuildTimestamps.sh )

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include

# Hack for NDK
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre

LOCAL_SHARED_LIBRARIES := libpcre

#LOCAL_STATIC_LIBRARIES := libpcre

LOCAL_LDLIBS += -lstdc++ -ldl -llog

SIPX_PORT_SHARED_LIBS += libcutils
SIPX_PORT_LDLIBS += -Wl,--allow-shlib-undefined
SIPX_PORT_CFLAGS += -include AndroidConfig.h -DANDROID_2_0
SIPX_PORT_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/system/core/include/arch/linux-arm

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
LOCAL_MODULE := libsipxUnit

LOCAL_SRC_FILES := \
  src/test/sipxportunit/SipxPortUnitTestClass.cpp \
  src/test/sipxportunit/SipxPortUnitTestConstructor.cpp \
  src/test/sipxportunit/SipxPortUnitTestEnvironment.cpp \
  src/test/sipxportunit/SipxPortUnitTestPointFailure.cpp \
  src/test/sipxunit/TestUtilities.cpp \
  src/test/sipxportunit/main.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \


#LOCAL_SHARED_LIBRARIES :=

LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS += -lstdc++ -ldl -llog

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)


# =======================

# Executable for testing the unit test framework
# this can also be used as a sandbox test.  Just delete the
# foo and bar tests from the LOCAL_SRC_FILES and add your test(s)

include $(CLEAR_VARS)

LOCAL_MODULE := sipxsandbox

LOCAL_SRC_FILES := \
  src/test/os/OsLockTest.cpp \
  src/test/os/OsMutexTest.cpp

#  src/test/sipxportunit/bar.cpp \
#  src/test/sipxportunit/foo.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \


LOCAL_SHARED_LIBRARIES := libpcre
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXport 

LOCAL_LDLIBS += -lstdc++ -ldl -llog

LOCAL_SHARED_LIBRARIES += $(SIPX_PORT_SHARED_LIBS)
LOCAL_LDLIBS += $(SIPX_PORT_LDLIBS)
LOCAL_CFLAGS += $(SIPX_PORT_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_PORT_C_INCLUDES)

include $(BUILD_EXECUTABLE)


# =======================

# shared lib for JNI run of unit tests

include $(CLEAR_VARS)

LOCAL_MODULE := libsipxportjnisandbox

LOCAL_SRC_FILES := \
  src/test/sipxportunit/unitJni.cpp \
  src/test/os/OsMutexTest.cpp \
  src/test/os/OsLockTest.cpp \
  src/test/os/OsAdapterInfoTest.cpp \
  src/test/os/OsCallbackTest.cpp \
  src/test/os/OsConfigDbTest.cpp \
  src/test/os/OsContactListTest.cpp \
  src/test/os/OsContactTest.cpp \
  src/test/os/OsDirTest.cpp \
  src/test/os/OsEventTest.cpp \
  src/test/os/OsFileIteratorTest.cpp \
  src/test/os/OsFileSystemTest.cpp \
  src/test/os/OsMsgDispatcherTest.cpp \
  src/test/os/OsMsgQTest.cpp \
  src/test/os/OsNameDbTest.cpp \
  src/test/os/OsPathTest.cpp \
  src/test/os/OsProcessIteratorTest.cpp \
  src/test/os/OsProcessMgrTest.cpp \
  src/test/os/OsProcessTest.cpp \
  src/test/os/OsSemTest.cpp \
  src/test/os/OsServerTaskTest.cpp \
  src/test/os/OsSharedLibMgrTest.cpp \
  src/test/os/OsSocketTest.cpp \
  src/test/os/OsTestUtilities.cpp \
  src/test/os/OsTimeTest.cpp \
  src/test/os/OsTimerTaskTest.cpp \
  src/test/os/OsTimerTest.cpp \
  src/test/utl/UtlChainTest.cpp \
  src/test/utl/UtlContainableTestStub.cpp \
  src/test/utl/UtlCrc32Test.cpp \
  src/test/utl/UtlCryptoTest.cpp \
  src/test/utl/UtlDList.cpp \
  src/test/utl/UtlDListIterator.cpp \
  src/test/utl/UtlHashBag.cpp \
  src/test/utl/UtlHashBagIterator.cpp \
  src/test/utl/UtlHashMap.cpp \
  src/test/utl/UtlHashMapIterator.cpp \
  src/test/utl/UtlInt.cpp \
  src/test/utl/UtlLinkTest.cpp \
  src/test/utl/UtlLongLongInt.cpp \
  src/test/utl/UtlRegex.cpp \
  src/test/utl/UtlSList.cpp \
  src/test/utl/UtlSListIteratorTest.cpp \
  src/test/utl/UtlSortedList.cpp \
  src/test/utl/UtlSortedListIteratorTest.cpp \
  src/test/utl/UtlStringTest.cpp \
  src/test/utl/UtlStringTest_ConstructiveManipulators.cpp \
  src/test/utl/UtlStringTest_DestructiveManipulators.cpp \
  src/test/utl/UtlStringTest_NonMutating.cpp \
  src/test/utl/UtlTokenizerTest.cpp \
  src/test/utl/UtlVoidPtr.cpp \
  src/test/utl/XmlContentTest.cpp \

Unit_tests_not_ready =: \
  src/test/os/OsFileTest.cpp \


#  src/test/sipxportunit/bar.cpp \
#  src/test/sipxportunit/foo.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \


LOCAL_SHARED_LIBRARIES := libpcre
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXport

LOCAL_LDLIBS += -lstdc++ -ldl -llog

LOCAL_SHARED_LIBRARIES += $(SIPX_PORT_SHARED_LIBS)
LOCAL_LDLIBS += $(SIPX_PORT_LDLIBS)
LOCAL_CFLAGS += $(SIPX_PORT_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_PORT_C_INCLUDES)

#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)


# =======================
# Executable for testing the sipXportLib utilities

include $(CLEAR_VARS)

LOCAL_MODULE := sipxportunit

#TODO:
# UtlHashMapPerformance.cpp & UtlListPerformance.cpp needs executables
hangs_unittest := \
  src/test/utl/PluginHooksTest.cpp \
  src/test/os/StunMessageTest.cpp \
  src/test/os/TurnMessageTest.cpp \


LOCAL_SRC_FILES := \
  src/test/os/OsMutexTest.cpp \
  src/test/os/OsLockTest.cpp \
  src/test/os/OsAdapterInfoTest.cpp \
  src/test/os/OsCallbackTest.cpp \
  src/test/os/OsConfigDbTest.cpp \
  src/test/os/OsContactListTest.cpp \
  src/test/os/OsContactTest.cpp \
  src/test/os/OsDirTest.cpp \
  src/test/os/OsEventTest.cpp \
  src/test/os/OsFileIteratorTest.cpp \
  src/test/os/OsFileSystemTest.cpp \
  src/test/os/OsFileTest.cpp \
  src/test/os/OsMsgDispatcherTest.cpp \
  src/test/os/OsMsgQTest.cpp \
  src/test/os/OsNameDbTest.cpp \
  src/test/os/OsPathTest.cpp \
  src/test/os/OsProcessIteratorTest.cpp \
  src/test/os/OsProcessMgrTest.cpp \
  src/test/os/OsProcessTest.cpp \
  src/test/os/OsSemTest.cpp \
  src/test/os/OsServerTaskTest.cpp \
  src/test/os/OsSharedLibMgrTest.cpp \
  src/test/os/OsSocketTest.cpp \
  src/test/os/OsTestUtilities.cpp \
  src/test/os/OsTimeTest.cpp \
  src/test/os/OsTimerTaskTest.cpp \
  src/test/os/OsTimerTest.cpp \
  src/test/utl/UtlChainTest.cpp \
  src/test/utl/UtlContainableTestStub.cpp \
  src/test/utl/UtlCrc32Test.cpp \
  src/test/utl/UtlCryptoTest.cpp \
  src/test/utl/UtlDList.cpp \
  src/test/utl/UtlDListIterator.cpp \
  src/test/utl/UtlHashBag.cpp \
  src/test/utl/UtlHashBagIterator.cpp \
  src/test/utl/UtlHashMap.cpp \
  src/test/utl/UtlHashMapIterator.cpp \
  src/test/utl/UtlInt.cpp \
  src/test/utl/UtlLinkTest.cpp \
  src/test/utl/UtlLongLongInt.cpp \
  src/test/utl/UtlRegex.cpp \
  src/test/utl/UtlSList.cpp \
  src/test/utl/UtlSListIteratorTest.cpp \
  src/test/utl/UtlSortedList.cpp \
  src/test/utl/UtlSortedListIteratorTest.cpp \
  src/test/utl/UtlStringTest.cpp \
  src/test/utl/UtlStringTest_ConstructiveManipulators.cpp \
  src/test/utl/UtlStringTest_DestructiveManipulators.cpp \
  src/test/utl/UtlStringTest_NonMutating.cpp \
  src/test/utl/UtlTokenizerTest.cpp \
  src/test/utl/UtlVoidPtr.cpp \
  src/test/utl/XmlContentTest.cpp \


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \


LOCAL_SHARED_LIBRARIES := libpcre
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXport

LOCAL_LDLIBS += -lstdc++ -ldl -llog

LOCAL_SHARED_LIBRARIES += $(SIPX_PORT_SHARED_LIBS)
LOCAL_LDLIBS += $(SIPX_PORT_LDLIBS)
LOCAL_CFLAGS += $(SIPX_PORT_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_PORT_C_INCLUDES)

include $(BUILD_EXECUTABLE)

