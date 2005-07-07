## Process this file with automake to produce Makefile.in

SUBDIRS = . test

lib_LTLIBRARIES = libsipXport.la

INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXport_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}


libsipXport_la_CXXFLAGS = $(GLIB_CFLAGS)
libsipXport_la_SOURCES =  \
    utl/UtlVoidPtr.cpp \
    utl/UtlInt.cpp \
    utl/UtlString.cpp \
    utl/UtlContainable.cpp \
    utl/UtlContainer.cpp \
    utl/UtlIterator.cpp \
    utl/UtlList.cpp \
    utl/UtlListIterator.cpp \
    utl/UtlSList.cpp \
    utl/UtlSListIterator.cpp \
    utl/UtlDList.cpp \
    utl/UtlDListIterator.cpp \
    utl/UtlSortedList.cpp \
    utl/UtlSortedListIterator.cpp \
    utl/UtlHashMap.cpp \
    utl/UtlHashMapIterator.cpp \
    utl/UtlHashBag.cpp \
    utl/UtlHashBagIterator.cpp \
    utl/UtlRscStore.cpp \
    utl/UtlRegex.cpp \
    utl/UtlTokenizer.cpp \
    os/OsAssert.c \
    os/OsCallback.cpp \
    os/OsConfigDb.cpp \
    os/OsConnectionSocket.cpp \
    os/OsCSemBase.cpp \
    os/OsDatagramSocket.cpp \
    os/OsDateTime.cpp \
    os/OsDirBase.cpp \
    os/OsEncryption.cpp \
    os/OsEvent.cpp \
    os/OsEventMsg.cpp \
    os/OsExcept.cpp \
    os/OsFileBase.cpp \
    os/OsFileInfoBase.cpp \
    os/OsFileIteratorBase.cpp \
    os/OsFileSystem.cpp \
    os/OsLockingList.cpp \
    os/OsMsg.cpp \
    os/OsMsgPool.cpp \
    os/OsMsgQ.cpp \
    os/OsMulticastSocket.cpp \
    os/OsNameDb.cpp \
    os/OsPathBase.cpp \
    os/osPrintf.cpp \
    os/OsProcessBase.cpp \
    os/OsProcessIteratorBase.cpp \
    os/OsProcessMgr.cpp \
    os/OsProtectEvent.cpp \
    os/OsProtectEventMgr.cpp \
    os/OsQueuedEvent.cpp \
    os/OsRpcMsg.cpp \
    os/OsRWMutex.cpp \
    os/OsServerSocket.cpp \
    os/OsServerTask.cpp \
    os/OsSharedLibMgr.cpp \
    os/OsSocket.cpp \
    os/OsSSLConnectionSocket.cpp \
    os/OsSSLServerSocket.cpp \
    os/OsSysLog.cpp \
    os/OsSysLogFacilities.cpp \
    os/OsSysLogMsg.cpp \
    os/OsSysLogTask.cpp \
    os/OsSysTimer.cpp \
    os/OsTask.cpp \
    os/OsTime.cpp \
    os/OsTimeLog.cpp \
    os/OsTimer.cpp \
    os/OsTimerMsg.cpp \
    os/OsTimerTask.cpp \
    os/OsTokenizer.cpp \
    os/OsUtil.cpp \
    os/shared/OsMsgQShared.cpp \
    os/shared/OsSysSoftTimer.cpp \
    os/shared/OsTimerMessage.cpp \
    os/linux/clock_gettime.c \
    os/linux/host_address.c \
    os/linux/OsBSemLinux.cpp \
    os/linux/OsCSemLinux.cpp \
    os/linux/OsDateTimeLinux.cpp \
    os/linux/OsDirLinux.cpp \
    os/linux/OsFileInfoLinux.cpp \
    os/linux/OsFileIteratorLinux.cpp \
    os/linux/OsFileLinux.cpp \
    os/linux/OsMutexLinux.cpp \
    os/linux/OsPathLinux.cpp \
    os/linux/OsProcessIteratorLinux.cpp \
    os/linux/OsProcessLinux.cpp \
    os/linux/OsSharedLibMgrLinux.cpp \
    os/linux/OsTaskLinux.cpp \
    os/linux/OsUtilLinux.cpp \
    os/linux/pt_csem.c \
    os/linux/pt_mutex.c \
    xmlparser/tinystr.cpp \
    xmlparser/tinyxml.cpp \
    xmlparser/tinyxmlerror.cpp \
    xmlparser/tinyxmlparser.cpp \
    xmlparser/TiXmlIterator.cpp

EXTRA_DIST= \
    os/Wnt/getWindowsDNSServers.c \
    os/Wnt/OsBSemWnt.cpp \
    os/Wnt/OsCSemWnt.cpp \
    os/Wnt/OsDateTimeWnt.cpp \
    os/Wnt/OsDirWnt.cpp \
    os/Wnt/OsFileInfoWnt.cpp \
    os/Wnt/OsFileIteratorWnt.cpp \
    os/Wnt/OsFileSystemWnt.cpp \
    os/Wnt/OsFileWnt.cpp \
    os/Wnt/OsMutexWnt.cpp \
    os/Wnt/OsPathWnt.cpp \
    os/Wnt/OsProcessIteratorWnt.cpp \
    os/Wnt/OsProcessWnt.cpp \
    os/Wnt/OsSharedLibMgrWnt.cpp \
    os/Wnt/OsSysSoftTimer.cpp \
    os/Wnt/OsTaskWnt.cpp \
    os/Wnt/OsTimerMessage.cpp \
    os/Wnt/OsUtilWnt.cpp \
    os/Wnt/WindowsAdapterInfo.cpp 

