# Microsoft Developer Studio Project File - Name="sipXcallLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXcallLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXcallLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXcallLibTest.mak" CFG="sipXcallLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXcallLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXcallLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXcallLibTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sipXcallLibTest___Win32_Release"
# PROP BASE Intermediate_Dir "sipXcallLibTest___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sipXcallLibTest___Win32_Release"
# PROP Intermediate_Dir "sipXcallLibTest___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\sipXmediaAdapterLib\interface" /I "include" /I "src\test" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "..\sipXportLib\src\test" /I "..\CPPUnit\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib sipXsdpLib.lib sipXtackLib.lib sipXportLib.lib sipXmediaLib.lib sipXmediaAdapterLib.lib sipXcallLib.lib cppunit_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/sipXcallLibTest.exe" /libpath:"..\sipXportLib\Release" /libpath:"..\sipXsdpLib\Release" /libpath:"..\sipXtackLib\Release" /libpath:"..\sipXmediaLib\Release" /libpath:"..\sipXmediaAdapterLib\Release" /libpath:"..\sipXcallLib\Release" /libpath:"..\CPPUnit\lib"

!ELSEIF  "$(CFG)" == "sipXcallLibTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXcallLibTest___Win32_Debug"
# PROP BASE Intermediate_Dir "sipXcallLibTest___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sipXcallLibTest___Win32_Debug"
# PROP Intermediate_Dir "sipXcallLibTest___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXmediaAdapterLib\interface" /I "include" /I "src\test" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "..\sipXportLib\src\test" /I "..\CPPUnit\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib sipXportLibd.lib sipXsdpLibd.lib sipXtackLibd.lib sipXmediaAdapterLibd.lib sipXmediaLibd.lib sipXcallLibd.lib wsock32.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/sipXcallLibTest.exe" /pdbtype:sept /libpath:"..\sipXportLib\Debug" /libpath:"..\sipXsdpLib\Debug" /libpath:"..\sipXtackLib\Debug" /libpath:"..\sipXmediaLib\Debug" /libpath:"..\sipXmediaAdapterLib\Debug" /libpath:"..\sipXcallLib\Debug" /libpath:"..\CPPUnit\lib"

!ENDIF 

# Begin Target

# Name "sipXcallLibTest - Win32 Release"
# Name "sipXcallLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=..\sipXportLib\src\test\sipxunit\TestTemplate.cpp
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
# Begin Group "cp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\test\cp\CallManagerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\cp\CpTestSupport.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\cp\CpTestSupport.h
# End Source File
# End Group
# End Target
# End Project
