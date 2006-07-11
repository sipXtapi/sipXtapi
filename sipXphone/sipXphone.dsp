# Microsoft Developer Studio Project File - Name="sipXphone" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sipXphone - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXphone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXphone.mak" CFG="sipXphone - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXphone - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sipXphone - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXphone - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIPXPHONE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "include" /I "rwToolsppLib\include" /I "sipXcallLib\include" /I "sipXmediaLib\include" /I "sipXportLib\include" /I "sipXportLib\include\glib" /I "sipXtackLib\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D SIPX_HOME=\".\" /D "DISABLE_MEM_POOLS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sipXcallLib.lib sipXmediaLib.lib sipXportLib.lib sipXtackLib.lib jvm.lib libpcre.a /nologo /dll /machine:I386 /libpath:"rwToolsppLib\release" /libpath:"sipXcallLib\release" /libpath:"sipXmediaLib\release" /libpath:"sipXportLib\release" /libpath:"sipXtackLib\release" /libpath:"sipXmediaLib\lib"

!ELSEIF  "$(CFG)" == "sipXphone - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIPXPHONE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "rwToolsppLib\include" /I "sipXcallLib\include" /I "sipXmediaLib\include" /I "sipXportLib\include" /I "sipXportLib\include\glib" /I "sipXtackLib\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D SIPX_HOME=\".\" /D "DISABLE_MEM_POOLS" /FR /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sipXcallLib.lib sipXmediaLib.lib sipXportLib.lib sipXtackLib.lib jvm.lib libpcre.a /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"rwToolsppLib\debug" /libpath:"sipXcallLib\debug" /libpath:"sipXmediaLib\debug" /libpath:"sipXportLib\debug" /libpath:"sipXtackLib\debug" /libpath:"sipXmediaLib\lib"

!ENDIF 

# Begin Target

# Name "sipXphone - Win32 Release"
# Name "sipXphone - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\pingerjni\ApplicationRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\AppsList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\licensemanager\BaseLicenseManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\CallListenerHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\wnt\Clarisys.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\debug_jni.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\DialerCallListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\javaJNIArgInit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\JavaSUAListenerDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\JNIHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\JXAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\LightLamp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_DialerHack.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_pds_cds_SipConfigServerAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sip_SipLineManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sip_SipRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sipxphone_sys_startup_PingerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sipxphone_sys_user_PUserManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sipxphone_sys_util_NetUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_sipxphone_sys_util_PingerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_stapi_PCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_stapi_PMediaPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_stapi_PMediaPlaylistPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_AbstractStateChangePoller.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_callcontrol_PtCallControlAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_callcontrol_PtCallControlCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_callcontrol_PtCallControlConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtComponentGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneButton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneHookSwitch.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneLamp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneMicrophone.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_phone_PtPhoneSpeaker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtTerminal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_telephony_PtTerminalConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\wnt\org_sipfoundry_teleping_testbed_TestbedButtonEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\wnt\org_sipfoundry_teleping_testbed_TestbedFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_util_PingerConfigFileParser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_util_SipParser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\org_sipfoundry_util_SysLog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\licensemanager\opensource\OSLicenseManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\Pinger.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\PingerConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\PingerConfigEncryption.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\PingerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\PingerInfoX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\PingerMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\PtCallListenerWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\PtConnectionListenerWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\PtTerminalComponentListenerContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\PtTerminalConnectionListenerWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\Reboot.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\SipConfigDeviceAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pinger\sipXtapi_stub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\Speeddial.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\SpeedDialWebDSP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pingerjni\VoicemailNotifyStateListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\Webui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\WebuiLineMgrUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\web\Webutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\licensemanager\opensource\OSLicenseManager.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
