# Microsoft Developer Studio Project File - Name="sipXtackLibTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXtackLibTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXtackLibTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXtackLibTest.mak" CFG="sipXtackLibTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXtackLibTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXtackLibTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXtackLibTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\sipXcallLib\include" /I "..\CPPUnit\include" /I "..\sipXsdpLib\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ..\sipXportLib\Release\sipXportLib.lib ..\sipXsdpLib\Release\sipXsdpLib.lib Release\sipXtackLib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib libpcre.a cppunitd_dll.lib /nologo /subsystem:console /machine:I386 /libpath:"..\CPPUnit\lib"

!ELSEIF  "$(CFG)" == "sipXtackLibTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXportLib\src\test" /I "..\sipXportLib\include" /I "include" /I "..\sipXcallLib\include" /I "..\CPPUnit\include" /I "..\sipXsdpLib\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /GZ /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\sipXportLib\Debug\sipXportLibd.lib ..\sipXsdpLib\Debug\sipXsdpLibd.lib Debug\sipXtackLibd.lib cppunitd_dll.lib winmm.lib wsock32.lib advapi32.lib pcre.lib /nologo /subsystem:console /map /debug /machine:I386 /pdbtype:sept /libpath:"..\CPPUnit\lib"

!ENDIF 

# Begin Target

# Name "sipXtackLibTest - Win32 Release"
# Name "sipXtackLibTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\test\net\HttpBodyTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\HttpMessageTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\NameValuePairInsensitiveTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\NameValuePairTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\NetAttributeTokenizerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\NetBase64CodecTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\NetMd5CodecTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SdpBodyTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\SdpHelperTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipContactDbTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipDialogEventTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipDialogMonitorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipDialogTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipMessageTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipPresenceEventTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipProxyMessageTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipPublishContentMgrTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipRefreshManagerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipServerShutdownTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipSrvLookupTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipSubscribeServerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipSubscriptionClientTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipSubscriptionMgrTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\SipUserAgentTest.cpp
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
# Begin Source File

SOURCE=.\src\test\net\UrlTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\test\net\XmlRpcTest.cpp
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
