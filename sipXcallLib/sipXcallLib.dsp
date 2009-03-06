# Microsoft Developer Studio Project File - Name="sipXcallLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXcallLib - Win32 Debug_NoVE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXcallLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXcallLib.mak" CFG="sipXcallLib - Win32 Debug_NoVE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXcallLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXcallLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXcallLib - Win32 Debug_NoVE" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXcallLib - Win32 Release"

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
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "..\gips\include" /I "include" /I "..\sipXportLib\include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\sipXmediaAdapterLib\interface" /D "NDEBUG" /D "SIPXMEDIA_EXCLUDE" /D "SIPXTAPI_STATIC" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXcallLib - Win32 Debug"

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
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\gips\include" /I "include" /I "..\sipXportLib\include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\sipXmediaAdapterLib\interface" /D "_DEBUG" /D "SIPXMEDIA_EXCLUDE" /D "SIPXTAPI_STATIC" /D "_LIB" /D "WIN32" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sipXcallLibd.lib"

!ELSEIF  "$(CFG)" == "sipXcallLib - Win32 Debug_NoVE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXcallLib___Win32_Debug_NoVE"
# PROP BASE Intermediate_Dir "sipXcallLib___Win32_Debug_NoVE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_NoVE"
# PROP Intermediate_Dir "Debug_NoVE"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\gips\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "DISABLE_MEM_POOLS" /D "HAVE_GIPS" /D "SIPXMEDIA_EXCLUDE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\sipXmediaAdapterLib\interface" /D "_DEBUG" /D "SIPXTAPI_EXPORTS" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug_NoVE\sipXcallLibd.lib"

!ENDIF 

# Begin Target

# Name "sipXcallLib - Win32 Release"
# Name "sipXcallLib - Win32 Debug"
# Name "sipXcallLib - Win32 Debug_NoVE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\cp\CallManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\Connection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpCallManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpGatewayManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpGhostConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpIntMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpMultiStringMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpPeerCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CpStringMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\CSeqManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\SipConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoEventListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoListenerEventMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoObjectMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoReference.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoString.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\cp\CallManager.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\Connection.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpCall.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpCallManager.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpGatewayManager.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpGhostConnection.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpIntMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpMediaInterface.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpMediaInterfaceFactory.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpMediaInterfaceFactoryFactory.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpMediaInterfaceFactoryImpl.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpMultiStringMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpPeerCall.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpPhoneMediaInterface.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CpStringMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\CSeqManager.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\DialogEventPublisher.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\SipConnection.h
# End Source File
# End Group
# End Target
# End Project
