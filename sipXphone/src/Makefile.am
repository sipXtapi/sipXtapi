## Process this file with automake to produce Makefile.in

SUBDIRS = . test

INCLUDES = -I$(top_srcdir)/include

lib_LTLIBRARIES = libsipXphone.la

libsipXphone_la_LIBADD = \
	@GLIB_LIBS@ \
	@PCRE_LIBS@  @CPPUNIT_LIBS@\
    @SIPXCALL_LIBS@ 


##libsipXphone_la_DEPENDENCIES = sipXportLib sipXtackLib sipXmediaLib sipXcallLib

libsipXphone_la_SOURCES =  \
    pinger/Pinger.cpp  \
    pinger/PingerConfig.cpp  \
    pinger/PingerConfigEncryption.cpp  \
    pinger/PingerInfo.cpp  \
    pinger/PingerInfoX.cpp  \
    pinger/PingerMsg.cpp  \
    pinger/sipXtapi_stub.cpp \
    pinger/SipConfigDeviceAgent.cpp  \
    pingerjni/ApplicationRegistry.cpp  \
    pingerjni/CallListenerHelpers.cpp  \
    pingerjni/org_sipfoundry_DialerHack.cpp  \
    pingerjni/org_sipfoundry_pds_cds_SipConfigServerAgent.cpp  \
    pingerjni/org_sipfoundry_sip_SipLineManager.cpp  \
    pingerjni/org_sipfoundry_sip_SipRequest.cpp  \
    pingerjni/org_sipfoundry_stapi_PCall.cpp  \
    pingerjni/org_sipfoundry_stapi_PMediaPlayer.cpp  \
    pingerjni/org_sipfoundry_stapi_PMediaPlaylistPlayer.cpp  \
    pingerjni/org_sipfoundry_telephony_AbstractStateChangePoller.cpp  \
    pingerjni/org_sipfoundry_telephony_callcontrol_PtCallControlAddress.cpp  \
    pingerjni/org_sipfoundry_telephony_callcontrol_PtCallControlCall.cpp  \
    pingerjni/org_sipfoundry_telephony_callcontrol_PtCallControlConnection.cpp  \
    pingerjni/org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter.cpp  \
    pingerjni/org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtComponent.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtComponentGroup.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneButton.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneDisplay.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneHookSwitch.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneLamp.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneMicrophone.cpp  \
    pingerjni/org_sipfoundry_telephony_phone_PtPhoneSpeaker.cpp  \
    pingerjni/org_sipfoundry_telephony_PtAddress.cpp  \
    pingerjni/org_sipfoundry_telephony_PtCall.cpp  \
    pingerjni/org_sipfoundry_telephony_PtConnection.cpp  \
    pingerjni/org_sipfoundry_telephony_PtProvider.cpp  \
    pingerjni/org_sipfoundry_telephony_PtTerminal.cpp  \
    pingerjni/org_sipfoundry_telephony_PtTerminalConnection.cpp  \
    pingerjni/org_sipfoundry_util_PingerConfigFileParser.cpp  \
    pingerjni/org_sipfoundry_util_SipParser.cpp  \
    pingerjni/org_sipfoundry_util_SysLog.cpp  \
    pingerjni/org_sipfoundry_sipxphone_sys_startup_PingerApp.cpp  \
    pingerjni/org_sipfoundry_sipxphone_sys_user_PUserManager.cpp  \
    pingerjni/org_sipfoundry_sipxphone_sys_util_NetUtilities.cpp  \
    pingerjni/org_sipfoundry_sipxphone_sys_util_PingerInfo.cpp  \
    pingerjni/debug_jni.cpp  \
    pingerjni/javaJNIArgInit.cpp  \
    pingerjni/JavaSUAListenerDispatcher.cpp  \
    pingerjni/JNIHelpers.cpp  \
    pingerjni/JXAPI.cpp  \
    pingerjni/LightLamp.cpp  \
    pingerjni/linux/org_sipfoundry_teleping_testbed_TestbedButtonEvent.cpp \
    pingerjni/linux/org_sipfoundry_teleping_testbed_TestbedFrame.cpp \
    pingerjni/PtCallListenerWrap.cpp  \
    pingerjni/PtConnectionListenerWrap.cpp  \
    pingerjni/PtTerminalComponentListenerContainer.cpp  \
    pingerjni/PtTerminalConnectionListenerWrap.cpp  \
    pingerjni/Reboot.cpp  \
    pingerjni/SpeedDialWebDSP.cpp  \
    pingerjni/VoicemailNotifyStateListener.cpp \
    web/AppsList.cpp \
    web/DialerCallListener.cpp \
    web/Speeddial.cpp \
    web/Webui.cpp \
    web/WebuiLineMgrUtil.cpp \
    web/Webutil.cpp \
    licensemanager/BaseLicenseManager.cpp \
    licensemanager/opensource/OSLicenseManager.cpp 

##makefiles = ../sipXportLib/Makefile ../sipXtackLib/Makefile \
##            ../sipXmediaLib/Makefile ../sipXcallLib/Makefile

##sipXportLib: ../sipXportLib/Makefile
##	$(MAKE) -C ../$(@F) all check
##
##sipXtackLib: ../sipXtackLib/Makefile
##	$(MAKE) -C ../$(@F) all check
##
##sipXmediaLib: ../sipXmediaLib/Makefile
##	$(MAKE) -C ../$(@F) all check
##
##sipXcallLib: ../sipXcallLib/Makefile
##	$(MAKE) -C ../$(@F) all check

##$(makefiles): %/Makefile:       
##	cd $(top_srcdir)/src && autoreconf --install $(@D)      
##	mkdir -p $(@D)
##	cd $(@D) && $(top_srcdir)/src/$(@D)/configure

#.PHONY: sipXportLib sipXtackLib sipXmediaLib sipXcallLib
