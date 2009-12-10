# This Makefile is for building sipXportLib as a part of Android core.
# It fails when you try to build with it for NDK, so we provide a separate
# Makefile for NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXport
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

LOCAL_SRC_FILES := \
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
    src/utl/UtlIterator.cpp \
    src/utl/UtlList.cpp \
    src/utl/UtlListIterator.cpp \
    src/utl/UtlSList.cpp \
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
    src/os/linux/OsRWMutexLinux.cpp \
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

LOCAL_CFLAGS += -D__pingtel_on_posix__ \
                -DSIPX_TMPDIR=\"/usr/var/tmp\" -DSIPX_CONFDIR=\"/etc/sipxpbx\"

#ifeq ($(TARGET_ARCH),arm)
#	LOCAL_CFLAGS += -DARMv5_ASM
#endif

#ifeq ($(TARGET_BUILD_TYPE),debug)
#	LOCAL_CFLAGS += -DDEBUG
#endif
    
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include

#LOCAL_SHARED_LIBRARIES := \

LOCAL_STATIC_LIBRARIES := \
	libpcre

LOCAL_LDLIBS += -lpthread -lrt -lstdc++

ifeq ($(TARGET_OS),linux)
LOCAL_LDLIBS += -ldl
endif

ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

include $(BUILD_SHARED_LIBRARY)
