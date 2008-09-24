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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\gips\include" /I "include" /I "..\sipXportLib\include" /I "..\sipXsdpLib\include" /I "..\sipXtackLib\include" /I "..\sipXmediaLib\include" /I "..\sipXmediaAdapterLib\interface" /D "_DEBUG" /D "SIPXMEDIA_EXCLUDE" /D "SIPXTAPI_STATIC" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /GZ /c
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

SOURCE=.\src\ps\PsButtonInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsButtonTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsHookswDev.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\wnt\PsHookswDevWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsHookswTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsKeybdDev.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsLampDev.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\wnt\PsLampDevWnt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsLampInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsLampTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsPhoneTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoButton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoComponentGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoHookswitch.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoLamp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoMicrophone.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoRinger.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ps\PsTaoSpeaker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtAddessCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtAddressForwarding.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtAudioCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtCallCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtCallEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtCallListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtComponentGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtComponentGroupCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtConfigDb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtConnectionCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtConnectionEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtConnectionListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtEventListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtMediaCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtMetaEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtMultiCallMetaEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneButton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneExtSpeaker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneGraphicDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneHookswitch.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneLamp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneMicrophone.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneRinger.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneSpeaker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneTerminal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtPhoneTextDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtProviderCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtProviderListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtSessionDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtSingleCallMetaEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtStubs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalComponentEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalComponentListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalConnectionCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalConnectionEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalConnectionListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptapi\PtTerminalListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cp\SipConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tapi\SipXEventDispatcher.cpp
# End Source File
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
# Begin Source File

SOURCE=.\src\tao\TaoAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoAddressAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoCallAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoClientTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoConnectionAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoEventDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoEventListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoListenerClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoListenerEventMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoListenerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoListeningTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoObjectMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoPhoneComponentAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoProviderAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoProviderListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoReference.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoServerTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoString.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoTerminalAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoTerminalConnectionAdaptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoTerminalConnectionListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoTransportAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tao\TaoTransportTask.cpp
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

SOURCE=.\include\ps\PsButtonId.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsButtonInfo.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsButtonTask.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsHookswDev.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\wnt\PsHookswDevWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsHookswTask.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsKeybdDev.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\wnt\PsKeybdDevWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsLampDev.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\wnt\PsLampDevWnt.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsLampId.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsLampInfo.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsLampTask.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsMsg.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsPhoneTask.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoButton.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoComponent.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoComponentGroup.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoDisplay.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoHookswitch.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoLamp.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoMicrophone.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoRinger.h
# End Source File
# Begin Source File

SOURCE=.\include\ps\PsTaoSpeaker.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAddessCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAddress.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAddressEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAddressForwarding.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAddressListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtAudioCodec.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtCall.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtCallCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtCallEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtCallListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtComponent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtComponentGroup.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtComponentGroupCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtComponentIntChangeEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtComponentStringChangeEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtConfigDb.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtConnection.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtConnectionCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtConnectionEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtConnectionListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtDefs.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtEventListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtEventMask.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtGatewayInterface.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtMediaCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtMetaEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtMultiCallEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtMultiCallMetaEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneButton.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneDisplay.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneExtSpeaker.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneGraphicDisplay.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneHookswitch.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneLamp.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneMicrophone.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneRinger.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneSpeaker.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneTerminal.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtPhoneTextDisplay.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtProvider.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtProviderCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtProviderEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtProviderListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtSessionDesc.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtSingleCallMetaEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminal.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalComponentEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalComponentListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalConnection.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalConnectionCapabilities.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalConnectionEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalConnectionListener.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\ptapi\PtTerminalListener.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\SipConnection.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\SipXHandleMap.h
# End Source File
# Begin Source File

SOURCE=.\include\cp\sipXmediaFactoryImpl.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\SipXMessageObserver.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapi.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\SipXtapiEventDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapiEvents.h
# End Source File
# Begin Source File

SOURCE=.\include\tapi\sipXtapiInternal.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoAddressAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoCallAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoClientTask.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoConnectionAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoDefs.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoEventDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoEventListener.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoListenerClient.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoListenerEventMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoListenerManager.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoListeningTask.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoObject.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoObjectMap.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoPhoneComponentAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoProviderAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoProviderListener.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoReference.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoServerTask.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoString.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoTerminalAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoTerminalConnectionAdaptor.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoTerminalConnectionListener.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoTransportAgent.h
# End Source File
# Begin Source File

SOURCE=.\include\tao\TaoTransportTask.h
# End Source File
# End Group
# End Target
# End Project
