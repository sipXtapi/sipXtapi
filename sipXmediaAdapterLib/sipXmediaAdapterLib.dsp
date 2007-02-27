# Microsoft Developer Studio Project File - Name="sipXmediaAdapterLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXmediaAdapterLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaAdapterLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXmediaAdapterLib.mak" CFG="sipXmediaAdapterLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXmediaAdapterLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXmediaAdapterLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXmediaAdapterLib - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "include" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "sipXmediaMediaProcessing" /I "interface" /I "..\sipXmediaLib\include" /D "NDEBUG" /D "HAVE_SPEEX" /D "HAVE_GSM" /D "_LIB" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /D "DDDDISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY" /D "DDDENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXmediaAdapterLib - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXsdpLib\include" /I "sipXmediaMediaProcessing" /I "interface" /I "..\sipXmediaLib\include" /D "_DEBUG" /D "HAVE_SPEEX" /D "HAVE_GSM" /D "_LIB" /D "WIN32" /D "_MBCS" /D "SIPXTAPI_STATIC" /D "DISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY" /D "ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sipXmediaAdapterLibd.lib"

!ENDIF 

# Begin Target

# Name "sipXmediaAdapterLib - Win32 Release"
# Name "sipXmediaAdapterLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\interface\CpMediaInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\interface\CpMediaInterfaceFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\interface\CpMediaInterfaceFactoryImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\src\CpPhoneMediaInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\src\CpTopologyGraphFactoryImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\src\CpTopologyGraphInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\src\sipXmediaFactoryImpl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\interface\mi\CpMediaInterface.h
# End Source File
# Begin Source File

SOURCE=.\interface\mi\CpMediaInterfaceFactory.h
# End Source File
# Begin Source File

SOURCE=.\interface\mi\CpMediaInterfaceFactoryFactory.h
# End Source File
# Begin Source File

SOURCE=.\interface\mi\CpMediaInterfaceFactoryImpl.h
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\include\CpPhoneMediaInterface.h
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\include\CpTopologyGraphFactoryImpl.h
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\include\CpTopologyGraphInterface.h
# End Source File
# Begin Source File

SOURCE=.\sipXmediaMediaProcessing\include\sipXmediaFactoryImpl.h
# End Source File
# End Group
# End Target
# End Project
