# Microsoft Developer Studio Project File - Name="sipXmediaAdapterLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXmediaAdapterLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaAdapterLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaAdapterLibTest.mak" CFG="sipXmediaAdapterLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXmediaAdapterLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXmediaAdapterLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXmediaAdapterLibTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "interface" /I "include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "..\CPPUnit\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SIPXTAPI_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ..\sipXportLib\Release\sipXportLib.lib ..\sipXsdpLib\Release\sipXsdpLib.lib ..\sipXtackLib\Release\sipXtackLib.lib ..\sipXmediaLib\Release\sipXmediaLib.lib Release\sipXmediaAdapterLib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib libpcre.a cppunit.lib winmm.lib /nologo /subsystem:console /machine:I386 /libpath:"..\CPPUnit\include"

!ELSEIF  "$(CFG)" == "sipXmediaAdapterLibTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "interface" /I "include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "..\CPPUnit\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SIPXTAPI_STATIC" /YX /FD /GZ /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\sipXtackLib\Debug\sipXtackLibd.lib ..\sipXsdpLib\Debug\sipXsdpLibd.lib ..\sipXmediaLib\Debug\sipXmediaLibd.lib Debug\sipXmediaAdapterLibd.lib ..\sipXportLib\Debug\sipXportLibd.lib cppunitd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib pcre.lib winmm.lib /nologo /subsystem:console /map /debug /machine:I386 /pdbtype:sept /libpath:"..\CPPUnit\lib"

!ENDIF 

# Begin Target

# Name "sipXmediaAdapterLibTest - Win32 Release"
# Name "sipXmediaAdapterLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\src\test\CpPhoneMediaInterfaceTest.cpp
# End Source File
# Begin Source File

SOURCE=..\sipXportLib\src\test\sipxunit\TestMonitor.cpp
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
