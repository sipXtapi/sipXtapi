# Microsoft Developer Studio Project File - Name="sipXtapiTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXtapiTest - Win32 Debug_NoVE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapiTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapiTest.mak" CFG="sipXtapiTest - Win32 Debug_NoVE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXtapiTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXtapiTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXtapiTest - Win32 Debug_NoVE" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXtapiTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "..\sipXmediaAdapterLib\interface" /I "..\sipXcallLib\include" /I "..\sipXportLib\include" /I "include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\CPPUnit\include" /D "NDEBUG" /D "SIPXTAPI_STATIC" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib sipXsdpLib.lib sipXtackLib.lib sipXportLib.lib sipXmediaLib.lib sipXmediaAdapterLib.lib sipXcallLib.lib cppunit_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib wsock32.lib /nologo /subsystem:console /machine:I386 /libpath:"sipXtapi\Release\\" /libpath:"..\sipXsdpLib\Release" /libpath:"..\sipXtackLib\Release" /libpath:"..\sipXportLib\Release" /libpath:"..\sipXcallLib\Release" /libpath:"..\sipXmediaLib\Release" /libpath:"..\sipXmediaAdapterLib\Release" /libpath:"..\CPPUnit\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sipXtapiTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXtapiTest___Win32_Debug"
# PROP BASE Intermediate_Dir "sipXtapiTest___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXmediaAdapterLib\interface" /I "..\sipXcallLib\include" /I "..\sipXportLib\include" /I "include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\CPPUnit\include" /D "_DEBUG" /D "SIPXTAPI_STATIC" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib sipXportLibd.lib sipXsdpLibd.lib sipXtackLibd.lib sipXmediaAdapterLibd.lib sipXmediaLibd.lib sipXcallLibd.lib wsock32.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\sipXmediaLib\Debug\\" /libpath:"..\sipXmediaAdapterLib\Debug" /libpath:"..\sipXsdpLib\Debug" /libpath:"..\sipXtackLib\Debug" /libpath:"..\sipXportLib\debug" /libpath:"debug" /libpath:"..\CPPUnit\lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "sipXtapiTest - Win32 Debug_NoVE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXtapiTest___Win32_Debug_NoVE"
# PROP BASE Intermediate_Dir "sipXtapiTest___Win32_Debug_NoVE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sipXtapiTest___Win32_Debug_NoVE"
# PROP Intermediate_Dir "sipXtapiTest___Win32_Debug_NoVE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\..\sipXportLib\include" /I "..\..\sipXcallLib\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DISABLE_MEM_POOLS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXportLib\include\test" /I "..\sipXcallLib\include" /I "..\sipXportLib\include" /I "include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\CPPUnit\include" /D "_DEBUG" /D "DISABLE_MEM_POOLS" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Advapi32.lib msvcrtd.lib MSVCPRTD.lib wsock32.lib sipXtackLib.lib sipXportLib.lib sipXtapid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib libpcre.a /nologo /subsystem:console /debug /machine:I386 /out:".\sipXtapi\Debug/sipXtapiTest.exe" /pdbtype:sept /libpath:"sipXtapi\Debug\\" /libpath:"..\sipXtackLib\debug" /libpath:"..\sipXportLib\debug"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 msvcrtd.lib MSVCPRTD.lib sipXsdpLibd.lib sipXtackLib.lib sipXportLib.lib sipXtapid.lib cppunitd_dll.lib Advapi32.lib wsock32.lib pcre.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /out:".\sipXtapi\Debug_NoVE/sipXtapiTest.exe" /pdbtype:sept /libpath:"sipXtapi\Debug_NoVE\\" /libpath:"..\sipXsdpLib\debug" /libpath:"..\sipXtackLib\debug" /libpath:"..\sipXportLib\debug" /libpath:"..\CPPUnit\lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "sipXtapiTest - Win32 Release"
# Name "sipXtapiTest - Win32 Debug"
# Name "sipXtapiTest - Win32 Debug_NoVE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\test\tapi\callbacks.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\EventRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\EventValidator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestConference.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestConferenceAdv.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestLine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestSubscribe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTestTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestExternalTransport.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistrar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistryCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistryData.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestStunServerTask.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\src\test\tapi\callbacks.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\EventRecorder.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\EventValidator.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\sipXtapiTest.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistrar.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistryCollection.h
# End Source File
# Begin Source File

SOURCE=.\src\test\tapi\TestRegistryData.h
# End Source File
# End Group
# End Target
# End Project
