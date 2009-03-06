# Microsoft Developer Studio Project File - Name="sipXsdpLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXsdpLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXsdpLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXsdpLibTest.mak" CFG="sipXsdpLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXsdpLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXsdpLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXsdpLibTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\CPPUnit\include" /I "..\sipXtackLib\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ..\sipXportLib\Release\sipXportLib.lib Release\sipXsdpLib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib pcre.lib cppunit_dll.lib /nologo /subsystem:console /machine:I386 /libpath:"..\CPPUnit\lib"

!ELSEIF  "$(CFG)" == "sipXsdpLibTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\CPPUnit\include" /I "..\sipXtackLib\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\sipXportLib\Debug\sipXportLibd.lib ..\sipXtackLib\Debug\sipXtackLibd.lib Debug\sipXsdpLibd.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib pcre.lib /nologo /subsystem:console /map /debug /machine:I386 /pdbtype:sept /libpath:"..\CPPUnit\lib"

!ENDIF 

# Begin Target

# Name "sipXsdpLibTest - Win32 Release"
# Name "sipXsdpLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\test\sdp\SdpSetGetTest.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestMonitor.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestOsSysLogListener.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestOutputter.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestRunner.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestUtilities.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\unitmain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestMonitor.h
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestOsSysLogListener.h
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestOutputter.h
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestRunner.h
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestUtilities.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
