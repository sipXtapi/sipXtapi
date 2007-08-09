# Microsoft Developer Studio Project File - Name="sipXportLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXportLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXportLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXportLibTest.mak" CFG="sipXportLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXportLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXportLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXportLibTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "DONT_USE_LONG_LONG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "." /I "include" /I "src/test" /I "..\CPPUnit\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D TEST_DIR=\".\" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cppunit_dll.lib sipXportLib.lib libpcre.a wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"Release" /libpath:"..\CPPUnit\lib"

!ELSEIF  "$(CFG)" == "sipXportLibTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "include" /I "src/test" /I "..\CPPUnit\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D TEST_DIR=\".\" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppunitd_dll.lib sipXportLibd.lib pcre.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"debug" /libpath:"..\CPPUnit\lib"

!ENDIF 

# Begin Target

# Name "sipXportLibTest - Win32 Release"
# Name "sipXportLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\test\sipxunit\TestMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestOsSysLogListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestOutputter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestRunner.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\unitmain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\test\sipxunit\TestMonitor.h
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestOsSysLogListener.h
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestOutputter.h
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestRunner.h
# End Source File
# Begin Source File

SOURCE=.\src\test\sipxunit\TestUtilities.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "bug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\test\os\OsProcessMgrTest.cpp

!IF  "$(CFG)" == "sipXportLibTest - Win32 Release"

!ELSEIF  "$(CFG)" == "sipXportLibTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Os"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\test\os\OsCallbackTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsConfigDbTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsContactListTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsContactTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsDirTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsEventTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsFileIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsFileSystemTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsFileTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsLockTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsMsgQTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsMsgDispatcherTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsMutexTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsNameDbTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsPathTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsProcessIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsProcessTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsSemTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsServerTaskTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsSocketTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsTestUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsTestUtilities.h
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsTimerTaskTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsTimerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\OsTimeTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\StunMessageTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\os\TurnMessageTest.cpp
# End Source File
# End Group
# Begin Group "utl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\test\utl\UtlChainTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlContainableTestStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlContainableTestStub.h
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlDList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlDListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlHashBag.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlHashBagIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlHashMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlHashMapIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlInt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlLinkTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlLongLongInt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlRegex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlSList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlSListIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlSortedList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlSortedListIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlStringTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlStringTest.h
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlStringTest_ConstructiveManipulators.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlStringTest_DestructiveManipulators.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlStringTest_NonMutating.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlTokenizerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\UtlVoidPtr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\utl\XmlContentTest.cpp
# End Source File
# End Group
# End Target
# End Project
