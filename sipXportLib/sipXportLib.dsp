# Microsoft Developer Studio Project File - Name="sipXportLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXportLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXportLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXportLib.mak" CFG="sipXportLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXportLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXportLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXportLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "include" /I "..\..\sipXportLib\include" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /D "HAVE_SSL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXportLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\..\sipXportLib\include" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /D "HAVE_SSL" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sipXportLibd.lib"

!ENDIF 

# Begin Target

# Name "sipXportLib - Win32 Release"
# Name "sipXportLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\os\Wnt\getWindowsDNSServers.cpp
# End Source File
# Begin Source File

SOURCE=".\src\hmac-sha1\hmac-sha1.c"
# End Source File
# Begin Source File

SOURCE=.\src\os\NatMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsAssert.c
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsBSemWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsCallback.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsConfigDb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsConnectionSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsContact.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsContactList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsCSemBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsCSemWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsDatagramSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsDateTime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsDateTimeWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsDirBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsDirWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsEncryption.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsEventMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsExcept.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsFileBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsFileInfoBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsFileInfoWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsFileIteratorBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsFileIteratorWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsFileSystemWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsFileWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsLockingList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsMsgPool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsMsgQ.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\shared\OsMsgQShared.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsMulticastSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsMutexWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsNameDb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsNatAgentTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsNatDatagramSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsNatSocketBaseImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsPathBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsPathWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\osPrintf.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsProcessBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsProcessIteratorBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsProcessIteratorWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsProcessMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsProcessWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsProtectEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsProtectEventMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsPtrMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsQueuedEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsRpcMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsRWMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsServerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsServerTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSharedLibMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsSharedLibMgrWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSSLConnectionSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSSLServerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSysLog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSysLogFacilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSysLogMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsSysLogTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsTaskWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTimeLog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsTimerMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTimerMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTimerTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\OsUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\OsUtilWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\PluginHooks.cpp
# End Source File
# Begin Source File

SOURCE=".\src\hmac-sha1\sha1.c"
# End Source File
# Begin Source File

SOURCE=.\src\os\StunMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\StunUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xmlparser\tinystr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xmlparser\tinyxml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xmlparser\tinyxmlerror.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xmlparser\tinyxmlparser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xmlparser\TiXmlIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\TurnMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlBool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlContainable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlContainableAtomic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlCrc32.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlDateTime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlDList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlDListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlHashBag.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlHashBagIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlHashMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlHashMapIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlInt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlLink.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlLongLongInt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlRandom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlRegex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlRscStore.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlRscTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlSList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlSListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlSortedList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlSortedListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlString.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\UtlVoidPtr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\os\Wnt\WindowsAdapterInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\utl\XmlContent.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\os\wnt\getWindowsDNSServers.h
# End Source File
# Begin Source File

SOURCE=.\include\os\HostAdapterAddress.h
# End Source File
# Begin Source File

SOURCE=.\include\os\IStunSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\NatMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsAssert.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsBSem.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsBSemWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsCallback.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsConfigDb.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsConfigEncryption.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsContact.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsContactList.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsCSem.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsCSemWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsDatagramSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsDateTime.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsDateTimeWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsDefs.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsDirBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsDirWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsEncryption.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsEventMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsExcept.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsFileBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsFileInfoBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsFileInfoWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsFileIteratorBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsFileIteratorWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsFileSystemWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsFileWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsFS.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsLock.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsLockingList.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsMsgPool.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsMsgQ.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsMulticastSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsMutex.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsMutexWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNameDb.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNatAgentTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNatConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNatDatagramSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNatKeepaliveListener.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNatSocketBaseImpl.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsNotification.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsPathBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsPathWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsProcess.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsProcessIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsProcessIteratorWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsProcessMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsProcessWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsProtectEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsProtectEventMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsPtrMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsQueuedEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsReadLock.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsRpcMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsRWMutex.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsServerTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSharedLibMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsSharedLibMgrWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSSL.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSSLConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSSLServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsStatus.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsStunAgentTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsStunDatagramSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsStunQueryAgent.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSyncBase.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSysLog.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSysLogFacilities.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSysLogMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSysLogTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsSysTimer.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsTaskWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTime.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTimeLog.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTimer.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTimerDb.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTimerMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTimerTask.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTLS.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTLSClientConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTLSConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTLSServerConnectionSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTLSServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsUtil.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\OsUtilWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\os\OsWriteLock.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\PluginHooks.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\PtTest.h
# End Source File
# Begin Source File

SOURCE=.\include\os\qsTypes.h
# End Source File
# Begin Source File

SOURCE=".\src\hmac-sha1\sha1.h"
# End Source File
# Begin Source File

SOURCE=.\include\os\StunMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\os\StunUtils.h
# End Source File
# Begin Source File

SOURCE=.\include\xmlparser\tinystr.h
# End Source File
# Begin Source File

SOURCE=.\include\xmlparser\tinyxml.h
# End Source File
# Begin Source File

SOURCE=.\include\xmlparser\TiXmlIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\os\TurnMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\aes\uitypes.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlBool.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlContainable.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlContainableAtomic.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlContainer.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlCrc32.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlDateTime.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlDefs.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlDList.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlDListIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlHashBag.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlHashBagIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlHashMap.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlHashMapIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlHistogram.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlInt.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlLink.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlList.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlListIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlLongLongInt.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlObservable.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlObservableImpl.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlRandom.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlRegex.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlRscStore.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlRscTrace.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlSList.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlSListIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlSortedList.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlSortedListIterator.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlString.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\UtlVoidPtr.h
# End Source File
# Begin Source File

SOURCE=.\include\os\wnt\WIndowsAdapterInfo.h
# End Source File
# Begin Source File

SOURCE=.\include\utl\XmlContent.h
# End Source File
# End Group
# End Target
# End Project
