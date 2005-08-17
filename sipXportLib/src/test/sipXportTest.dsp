# Microsoft Developer Studio Project File - Name="sipXportTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXportTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXportTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXportTest.mak" CFG="sipXportTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXportTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXportTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXportTest - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DISABLE_MEM_POOLS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cppunit.lib sipXportLib.lib libpcre.a wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"..\..\..\sipXportLib\Release"

!ELSEIF  "$(CFG)" == "sipXportTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\include" /I "..\..\include\glib" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DISABLE_MEM_POOLS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppunitd_dll.lib sipXportLib.lib libpcre.a wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\sipXportLib\debug"

!ENDIF 

# Begin Target

# Name "sipXportTest - Win32 Release"
# Name "sipXportTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sipxunit\TestMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestOutputter.cpp
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestRunner.cpp
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestRunner.h
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\sipxunit\unitmain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\sipxunit\TestMonitor.h
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestOutputter.h
# End Source File
# Begin Source File

SOURCE=.\sipxunit\TestUtilities.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "bug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\os\OsProcessMgrTest.cpp

!IF  "$(CFG)" == "sipXportTest - Win32 Release"

!ELSEIF  "$(CFG)" == "sipXportTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Os"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\os\OsCallbackTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsConfigDbTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsDirTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsEventTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsFileIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsFileSystemTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsFileTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsLockTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsMsgQTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsMutexTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsNameDbTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsPathTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsProcessIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsProcessTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsSemTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsSocketTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsTestUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsTestUtilities.h
# End Source File
# Begin Source File

SOURCE=.\os\OsTimerTaskTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsTimerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\os\OsTimeTest.cpp
# End Source File
# End Group
# Begin Group "utl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\utl\UtlContainableTestStub.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlContainableTestStub.h
# End Source File
# Begin Source File

SOURCE=.\utl\UtlDList.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlDListIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlHashBag.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlHashBagIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlHashMap.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlHashMapIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlInt.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlRegex.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlSList.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlSListIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlSortedList.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlSortedListIteratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlStringTest.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlStringTest.h
# End Source File
# Begin Source File

SOURCE=.\utl\UtlStringTest_ConstructiveManipulators.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlStringTest_DestructiveManipulators.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlStringTest_NonMutating.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlTokenizerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\utl\UtlVoidPtr.cpp
# End Source File
# End Group
# End Target
# End Project
