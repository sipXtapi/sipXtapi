# Microsoft Developer Studio Project File - Name="sipXmediaLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXmediaLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaLibTest.mak" CFG="sipXmediaLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXmediaLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXmediaLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXmediaLibTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\sipXtackLib\include" /I "src\test" /I "..\CPPUnit\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ..\sipXportLib\Release\sipXportLib.lib ..\sipXsdpLib\Release\sipXsdpLib.lib Release\sipXtackLib.lib Debug\sipXmediaLib.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib pcre.lib winmm.lib /nologo /subsystem:console /machine:I386 /libpath:"..\sipXmediaLib\contrib\libgsm\lib" /libpath:"..\CPPUnit\lib" /libpath:"..\sipXmediaLib\contrib\libspeex\lib"

!ELSEIF  "$(CFG)" == "sipXmediaLibTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\sipXtackLib\include" /I "src\test" /I "..\CPPUnit\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /GZ /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\sipXportLib\Debug\sipXportLibd.lib ..\sipXsdpLib\Debug\sipXsdpLibd.lib ..\sipXtackLib\Debug\sipXtackLibd.lib Debug\sipXmediaLibd.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib pcre.lib winmm.lib /nologo /subsystem:console /map /debug /machine:I386 /pdbtype:sept /libpath:"..\sipXmediaLib\contrib\libgsm\lib" /libpath:"..\CPPUnit\lib" /libpath:"..\sipXmediaLib\contrib\libspeex\lib"

!ENDIF 

# Begin Target

# Name "sipXmediaLibTest - Win32 Release"
# Name "sipXmediaLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\test\mp\MpAudioBufTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpBufTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpDspUtilsTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpFlowGraphTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpGenericResourceTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpInputDeviceManagerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpMediaTaskTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpMMTimerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpOutputDriverTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpOutputManagerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprBridgeTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpResourceTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpResourceTopologyTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprFromFileTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprFromMicTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprMixerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprSplitterTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprToSpkrTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MprToneGenTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpTestResource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpWBInputOutputDeviceTest.cpp
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

SOURCE=.\src\test\mp\dtmf5_48khz_16b_signed.h
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpGenericResourceTest.h
# End Source File
# Begin Source File

SOURCE=.\src\test\mp\MpTestResource.h
# End Source File
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
