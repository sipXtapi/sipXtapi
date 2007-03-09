# Microsoft Developer Studio Project File - Name="sipXsdpLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXsdpLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXsdpLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXsdpLib.mak" CFG="sipXsdpLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXsdpLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXsdpLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXsdpLib - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "include" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /D "NDEBUG" /D "_LIB" /D SIP_STACK_VERSION=\"2.5.2\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXsdpLib - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /D "_DEBUG" /D "_LIB" /D SIP_STACK_VERSION=\"2.5.2\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sipXsdpLibd.lib"

!ENDIF 

# Begin Target

# Name "sipXsdpLib - Win32 Release"
# Name "sipXsdpLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\sdp\Sdp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdp\SdpCandidate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdp\SdpCandidatePair.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdp\SdpCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdp\SdpHelperResip.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdp\SdpMediaLine.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\sdp\Sdp.h
# End Source File
# Begin Source File

SOURCE=.\include\sdp\SdpCandidate.h
# End Source File
# Begin Source File

SOURCE=.\include\sdp\SdpCandidatePair.h
# End Source File
# Begin Source File

SOURCE=.\include\sdp\SdpCodec.h
# End Source File
# Begin Source File

SOURCE=.\include\sdp\SdpHelperResip.h
# End Source File
# Begin Source File

SOURCE=.\include\sdp\SdpMediaLine.h
# End Source File
# End Group
# End Target
# End Project
