# Microsoft Developer Studio Project File - Name="sipXezPhone" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sipXezPhone - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXezPhone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXezPhone.mak" CFG="sipXezPhone - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXezPhone - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sipXezPhone - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXezPhone - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /I "..\..\lib\mswd" /I "\wxWindows-2.4.2\include" /I "..\..\..\sipXportLib\include" /I "..\..\..\sipXtackLib\include" /I "..\..\..\gips" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "DONT_USE_LONG_LONG" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 advapi32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib sipXtapi.lib sipXportLib.lib libpcre.a /nologo /subsystem:windows /machine:I386 /out:"./sipXezPhone.exe" /libpath:"..\..\Release\\" /libpath:"\wxWindows-2.4.2\lib" /libpath:"..\..\..\sipXportLib\Release" /libpath:"..\..\sipXtapi\Release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sipXezPhone - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "../../include" /I "..\..\lib\mswd" /I "\wxWindows-2.4.2\include" /I "..\..\..\sipXportLib\include" /I "..\..\..\sipXtackLib\include" /I "..\..\..\gips" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "DONT_USE_LONG_LONG" /FR /YX"stdwx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib sipXcallLib.lib sipXtapid.lib sipXportLib.lib libpcre.a comctl32.lib comdlg32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMTD.lib" /nodefaultlib:"LIBCMT.lib" /nodefaultlib:"MSVCRT" /out:"..\..\sipXtapi\debug\sipXezPhone.exe" /pdbtype:sept /libpath:"..\..\Debug" /libpath:"\\" /libpath:"..\..\sipXtapi\Debug" /libpath:"\wxWindows-2.4.2\lib" /libpath:"..\..\..\sipXportLib\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sipXezPhone - Win32 Release"
# Name "sipXezPhone - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "states"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\states\PhoneState.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateAccepted.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateCallHeldLocally.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateCallHeldRemotely.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateConnected.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateDialing.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateDisconnectRequested.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateIdle.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateLocalHoldRequested.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateMachineObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateOutgoingCallReject.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateRemoteAlerting.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateRemoteBusy.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateRinging.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateTransfering.cpp
# End Source File
# Begin Source File

SOURCE=.\states\PhoneStateTransferRequested.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ButtonPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\CallHistoryPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\CallList.cpp
# End Source File
# Begin Source File

SOURCE=.\ConferencePanel.cpp
# End Source File
# Begin Source File

SOURCE=.\ContactListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DialEntryPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\DialerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\DialPadButton.cpp
# End Source File
# Begin Source File

SOURCE=.\DialPadPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\EventLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\PlacedCallList.cpp
# End Source File
# Begin Source File

SOURCE=.\ReceivedCallList.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXAudioSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhone_wdr.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneAboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneApp.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sipXmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\stdwx.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeControlPanel.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ButtonPanel.h
# End Source File
# Begin Source File

SOURCE=.\CallHistoryPanel.h
# End Source File
# Begin Source File

SOURCE=.\CallList.h
# End Source File
# Begin Source File

SOURCE=.\ConferencePanel.h
# End Source File
# Begin Source File

SOURCE=.\ContactListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DialEntryPanel.h
# End Source File
# Begin Source File

SOURCE=.\DialPadButton.h
# End Source File
# Begin Source File

SOURCE=.\DialPadPanel.h
# End Source File
# Begin Source File

SOURCE=.\EventLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainPanel.h
# End Source File
# Begin Source File

SOURCE=.\PlacedCallList.h
# End Source File
# Begin Source File

SOURCE=.\ReceivedCallList.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sipXAudioSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\sipXezPhone_wdr.h
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneApp.h
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneFrame.h
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneSettings.h
# End Source File
# Begin Source File

SOURCE=.\sipXezPhoneSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\sipXmgr.h
# End Source File
# Begin Source File

SOURCE=.\stdwx.h
# End Source File
# Begin Source File

SOURCE=.\VolumeControlPanel.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
