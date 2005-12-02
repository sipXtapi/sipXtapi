
include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = sipXezPhone

sipXezPhone_CXXFLAGS = \
	@WXWIDGETS_CXXFLAGS@

sipXezPhone_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
    @SIPXMEDIA_MP_LIBS@ \
	@WXWIDGETS_LIBS@

sipXezPhone_SOURCES = \
    AddContactDlg.cpp \
	ButtonPanel.cpp \
	CallHistoryPanel.cpp \
	CallList.cpp \
	ContactListCtrl.cpp \
	ConferencePanel.cpp \
	DialEntryPanel.cpp \
	DialerThread.cpp \
	DialPadButton.cpp \
	DialPadPanel.cpp \
    EventLogDlg.cpp \
	MainPanel.cpp \
	PlacedCallList.cpp \
	ReceivedCallList.cpp \
	sipXAudioSettingsDlg.cpp \
	sipXezPhoneAboutDlg.cpp \
	sipXezPhoneApp.cpp \
	sipXezPhoneFrame.cpp \
	sipXezPhoneSettings.cpp \
	sipXezPhoneSettingsDlg.cpp \
	sipXezPhone_wdr.cpp \
    ContactListPanel.cpp \
	sipXmgr.cpp \
    PreviewWindow.cpp \
    VideoPanel.cpp \
    VideoWindow.cpp \
    sipXVideoSettingsDlg.cpp \
	VolumeControlPanel.cpp \
	states/PhoneStateAccepted.cpp \
	states/PhoneStateCallHeldLocally.cpp \
	states/PhoneStateCallHeldRemotely.cpp \
	states/PhoneStateConnected.cpp \
	states/PhoneState.cpp \
	states/PhoneStateDialing.cpp \
	states/PhoneStateDisconnectRequested.cpp \
	states/PhoneStateIdle.cpp \
	states/PhoneStateLocalHoldRequested.cpp \
	states/PhoneStateMachine.cpp \
	states/PhoneStateMachineObserver.cpp \
	states/PhoneStateOutgoingCallReject.cpp \
	states/PhoneStateRemoteAlerting.cpp \
	states/PhoneStateRemoteBusy.cpp \
	states/PhoneStateRinging.cpp \
	states/PhoneStateTransfering.cpp \
	states/PhoneStateTransferRequested.cpp \
	stdwx.cpp \
    sipXSrtpSettingsDlg.cpp 
