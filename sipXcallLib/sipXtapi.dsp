# Microsoft Developer Studio Project File - Name="sipXtapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sipXtapi - Win32 Debug_NoVE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapi.mak" CFG="sipXtapi - Win32 Debug_NoVE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXtapi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sipXtapi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sipXtapi - Win32 Debug_NoVE" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXtapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\sipXtapi\Release"
# PROP Intermediate_Dir ".\sipXtapi\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\sipXportLib\include" /I "include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\gips" /I "..\gips\sipXmediaVoiceEngine\include" /I "..\sipXmediaAdapterLib\interface" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /D "DISABLE_MEM_POOLS" /D "HAVE_GIPS" /D "VOICE_ENGINE" /D "DONT_USE_LONG_LONG" /D "WIN32" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 sipXportLib.lib sipXtackLib.lib sipXmediaVoiceEngine.lib sipXcallLib.lib VideoEngineWindowsLib.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib libpcre.a libeay32.lib ssleay32.lib sipXmediaAdapterLibd.lib /nologo /dll /map /debug /machine:I386 /libpath:"..\sipXcallLib\Release" /libpath:"..\sipXmediaLib\release" /libpath:"..\sipXportLib\release" /libpath:"..\sipXtackLib\Release" /libpath:"..\gips\sipXmediaVoiceEngine\Release" /libpath:"..\gips\VoiceEngine\libraries"

!ELSEIF  "$(CFG)" == "sipXtapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXtapi___Win32_Debug"
# PROP BASE Intermediate_Dir "sipXtapi___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\sipXtapi\Debug"
# PROP Intermediate_Dir ".\sipXtapi\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXcallLib\include" /I "..\sipXportLib\include" /I "include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\gips" /I "..\gips\sipXmediaVoiceEngine\include" /I "..\sipXmediaAdapterLib\interface" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /D "DISABLE_MEM_POOLS" /D "HAVE_GIPS" /D "VOICE_ENGINE" /D "DONT_USE_LONG_LONG" /D "WIN32" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 sipXportLib.lib sipXtackLib.lib sipXmediaVoiceEngine.lib sipXcallLib.lib VideoEngineWindowsLib.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib libpcre.a libeay32.lib ssleay32.lib sipXmediaAdapterLibd.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib:"libcmt.lib" /nodefaultlib:"LIBC" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"LIBCD" /nodefaultlib:"msvcrt.lib" /out:".\sipXtapi\Debug/sipXtapid.dll" /pdbtype:sept /libpath:"..\sipXcallLib\Debug" /libpath:"..\gips\sipXmediaVoiceEngine\debug" /libpath:"..\sipXportLib\debug" /libpath:"..\sipXtackLib\Debug" /libpath:"..\gips" /libpath:"..\gips\VoiceEngine\Libraries" /libpath:"..\gips\sipXmediaVoiceEngine\Debug"

!ELSEIF  "$(CFG)" == "sipXtapi - Win32 Debug_NoVE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXtapi___Win32_Debug_NoVE"
# PROP BASE Intermediate_Dir "sipXtapi___Win32_Debug_NoVE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sipXtapi\Debug_NoVE"
# PROP Intermediate_Dir "sipXtapi\Debug_NoVE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\..\sipXportLib\include" /I "..\..\sipXcallLib\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /D "DISABLE_MEM_POOLS" /D "HAVE_GIPS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\sipXcallLib\include" /I "..\sipXportLib\include" /I "include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\gips" /I "..\gips\sipXmediaVoiceEngine\include" /I "..\sipXmediaAdapterLib\interface" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SIPXTAPI_EXPORTS" /D "DISABLE_MEM_POOLS" /D "DONT_USE_LONG_LONG" /D "WIN32" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib sipXportLib.lib sipXtackLib.lib sipXmediaVoiceEngine.lib sipXcallLib.lib libpcre.a GIPSVoiceEngineLib.lib /nologo /dll /debug /machine:I386 /out:".\sipXtapi\Debug/sipXtapid.dll" /pdbtype:sept /libpath:"..\sipXcallLib\Debug" /libpath:"..\gips\sipXmediaVoiceEngine\debug" /libpath:"..\sipXportLib\debug" /libpath:"..\sipXtackLib\Debug" /libpath:"..\gips" /libpath:"..\gips\VoiceEngine\Libraries" /libpath:"..\gips\sipXmediaVoiceEngine\Debug"
# ADD LINK32 sipXportLibd.lib sipXtackLibd.lib sipXmediaLibd.lib sipXcallLibd.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib libpcre.a libeay32.lib ssleay32.lib sipXmediaAdapterLibd.lib /nologo /dll /map /debug /machine:I386 /out:".\sipXtapi\Debug_NoVE/sipXtapid.dll" /pdbtype:sept /libpath:"..\sipXcallLib\Debug_NoVE" /libpath:"..\sipXmediaLib\Debug" /libpath:"..\sipXportLib\debug" /libpath:"..\sipXtackLib\Debug" /libpath:"..\sipXmediaAdapterLib\Debug"

!ENDIF 

# Begin Target

# Name "sipXtapi - Win32 Release"
# Name "sipXtapi - Win32 Debug"
# Name "sipXtapi - Win32 Debug_NoVE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\tapi\SipXHandleMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tapi\SipXMessageObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tapi\sipXtapi.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tapi\sipXtapiEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tapi\sipXtapiInternal.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\sipXtapi.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\tapi\SipXHandleMap.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\SipXMessageObserver.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapi.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapiEvents.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapiInternal.h
# End Source File
# End Group
# End Target
# End Project
