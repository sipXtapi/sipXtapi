//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _sipxmgr_h
#define _sipxmgr_h

#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

// DEFINES
#define MAX_RECORD_EVENTS       1024

#define MAX_EVENT_LOG_ENTRIES   256     /**< Max number of events to log in 
                                             event log */

/**
 * Singleton class whose one instance encapsulates the use of the sipXtapi functions.
 */
class sipXmgr
{
public:
    /**
    * Accessor for the single class instance.
    */
    static sipXmgr& getInstance();

    /**
    * Delete's the singleton instance
    */
    static void release();

    /**
    * sipXtapi Initialization calls.
    */
    UtlBoolean Initialize(const int iSipPort = DEFAULT_UDP_PORT, const int iRtpPort = DEFAULT_RTP_START_PORT, const bool bUseRport = false);

    /**
    * Call to sipxUnInitialize
    */
    void UnInitialize();

    /**
    * place a phone call
    */
    void placeCall(wxString szNumber);

    /**
    * place a phone call
    */
    bool placeCall(const char* szSipUrl, const char* szFromIdentity, const char* szUsername, const char* szPassword, const char *szRealm);

    /**
    * Accept a phone call.
    */
    void acceptCall(SIPX_CALL hCall);

    /**
    * get the "Speaker" volume - can be 1 - 10
    */
    const int getSpeakerVolume() const;

    /**
    * get the "Ringer" volume - can be 1 - 10
    */
    const int getRingerVolume() const;

    /**
    * get the Mic gain - can be 1 - 10
    */
    const int getMicGain() const;

    /**
    * sets the Speaker volume
    */
    void setSpeakerVolume(const int volume);

    /**
    * sets the Ringer volume
    */
    void setRingerVolume(const int volume);

    /**
    * sets the Mic gain
    */
    void setMicGain(const int gain);

    /**
    * Disconnect the currently connected call, if any.
    */
    bool disconnect(SIPX_CALL hCall, bool bDisconnectAll);

    /**
    * Sets the handle of the "current" call.
    */
    void setCurrentCall(const SIPX_CALL hCall);

    /**
    * Gets the handle of the "current" call.
    */
    const SIPX_CALL getCurrentCall() const;

    /**
    * Returns the current Line handle
    */
    const SIPX_LINE getCurrentLine() const;

    /**
    * Places the current call on hold.
    */
    void holdCurrentCall();

    /**
    * Takes the current held call to a normal connected state
    */
    void unholdCurrentCall();

    /**
    * Sets a flag indicating whether or not the phone is
    * currently trying to transfer a call.
    */
    void setInTransfer(const bool bXfer);

    /**
    * Gets a flag indicating whether or not the phone is
    * currently trying to transfer a call.
    */
    const bool getInTransfer() const;

    /**
    * Gets the "current conference" handle.
    * Returns 0 if no conference exists.
    */
    const SIPX_CONF getCurrentConference() const;
    
    /**
    * Sets the "current conference" handle.
    */
    void setCurrentConference(const SIPX_CONF hConf);

    /**
    * Stops a playing tone.
    */
    void stopTone();

    /** 
    * Removes the currently active Line
    */  
    void removeCurrentLine();

    /**
    * Unregisters the current line.
    */
    void UnRegister();

    /**
    * Creates a conference if none exists.
    * Makes a new call to a party and adds that party
    * to the conference.
    */
    bool addConfParty(const char* const szParty);
    
    /**
    * Removes a party from the conference.
    */
    bool removeConfParty(const char* const szParty);

    /**
     * Retrieves current codec preference setting
     */
    bool getCodecPreferences(int* pCodecPref);

    /**
     * Sets current codec preference setting
     */
    bool setCodecPreferences(int codecPref);

    /**
     * Retrieves current codec preference setting
     */
    bool getCodecList(UtlString& codecList);

    /**
     * Sets the audio codec by name
     */
    bool setAudioCodecByName(const char* name);

    /**
     * Sets the video codec by name
     */
    bool setVideoCodecByName(const char* name);

    /**
     * Retrieves current video codec preference setting
     */
    bool getVideoCodecPreferences(int* pCodecPref);

    /**
     * Sets current video codec preference setting
     */
    bool setVideoCodecPreferences(int codecPref);

    /**
     * Retrieves current video codec preference setting
     */
    bool getVideoCodecList(UtlString& codecList);

    /**
     * Set video parameters
     */
    bool setVideoParameters(int iQuality, int iBitRate, int iFrameRate);

    /**
    * Retrieves setting of echo cancellation
    */
    bool isAECEnabled();

    /**
    * Sets echo cancellation
    */
    void enableAEC(bool bEnable);

    /**
    * Retrieves setting of out-of-band DTMF
    */
    bool isOutOfBandDTMFEnabled();

    /**
    * Sets out-of-band DTMF
    */
    void enableOutOfBandDTMF(bool bEnable);

    /**
    * Retrieves setting of SRTP
    */
    bool isSRTPEnabled();

    /**
    * Sets out-of-band DTMF
    */
    void enableSRTP(bool bEnable);
    
    /**
    * Toggles the mute setting.
    */
    void toggleMute();

    /**
     * Get the contents of the event log.
     */
    void getEventLog(wxString& contents) const;

    /**
     * Clear the contents of the event log.
     */
    void clearEventLog() ;

    SIPX_INST getSipxInstance();
    
    void* getVideoWindow();
    void* getPreviewWindow();
    void setVideoWindow(void* pWnd);
    void setPreviewWindow(void* pWnd);

    void setSrtpKey(UtlString& key)
        {mSrtpKey=key;}
    void getSrtpKey(UtlString& key)
        {key=mSrtpKey;}
        
    void readPublicKeyFile(UtlString& der, UtlString filename);
    
//    void readPkcs12File(UtlString& pkcs12, UtlString filename);

    void getLocalContacts(size_t nMaxAddresses,
                          SIPX_CONTACT_ADDRESS addresses[],
                          size_t& nActualAddresses) ;

    void enableLocationHeader(bool bEnable)
        {mbLocationHeaderEnabled=bEnable;}
    bool isLocationHeaderEnabled()
        {return mbLocationHeaderEnabled;}

    void prepareToHibernate();
    void unHibernate();
    
    /**
    * Destructor.
    */
    virtual ~sipXmgr();

private:
    sipXmgr();
    static sipXmgr* spSipXmgr;

    /**
    * Handle to the sipXtapi instanance
    */

    SIPX_INST m_hInst;       

    /**
    * Line Instance (id, auth, etc)
    */ 
    SIPX_LINE m_hLine;       

    /**
    * Handle to a call.
    */ 
    SIPX_CALL m_hCall; 

    /**
    * Handle to the current conference.
    */
    SIPX_CONF m_hConf;
    
    /**
    * Callback proc for receiving all types of sipXtapi events
    */
    static bool SipCallbackProc(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData);


    /**  (DEPRECATED)
    * Callback proc for Sip Call events.
    */
    //static void  SipCallbackProc( SIPX_CALL hCall,
    //                        SIPX_LINE hLine,
    //                        SIPX_CALLSTATE_MAJOR eMajor,
    //                        SIPX_CALLSTATE_MINOR eMinor,
    //                        void* pUser); 

    /**  (DEPRECATED)
    * Callback proc for line events.
    */
    //static void  SipLineCallbackProc(SIPX_LINE hLine,
    //                        SIPX_LINE_EVENT_TYPE_MAJOR eMajor,
    //                        void* pUser); 

    bool mTransferInProgress;

    SIPX_CALL m_hTransferInProgress;
    
    SIPX_CALL m_hCallController;
    
    /**
    * event handler for call state events fired from the sipXtapi layer.
    */
    bool handleCallstateEvent(void* pInfo, void* pUserData);

    /**
    * event handler for line state events fired from the sipXtapi layer.
    */
    bool handleLinestateEvent(void* pInfo, void* pUserData);

    /**
    * event handler for Info Status events fired from the sipXtapi layer.
    */
    bool handleInfoStatusEvent(void* pInfo, void* pUserData);

    /**
    * event handler for incoming Info events fired from the sipXtapi layer.
    */
    bool handleInfoEvent(void* pInfo, void* pUserData);

    /**
    * event handler for incoming Security events fired from the sipXtapi layer.
    */
    bool handleSecurityEvent(void* pInfo, void* pUserData);

    /**
    * event handler for incoming Media events fired from the sipXtapi layer.
    */
    bool handleMediaEvent(void* pInfo, void* pUserData);

    /**
     * Add event data to the event log.
     */
    void addToEventLog(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData);

    /**
    * The call-id string of the most recently handled call-event's call.
    */
    UtlString mCallId;
    
    /**
    * Map of call handles in the conference.
    * The key is the party's address, and 
    * the value is the call handle for that
    * party, for the conference.
    */
    UtlHashMap mConfCallHandleMap;

    /**
     * Event log for display to end user.  This is a circular buffer indexed
     * by mEventLogIndex.
     */
    wxString mEventLog[MAX_EVENT_LOG_ENTRIES] ;

    /**
     * Circular buffer index for mEventLog
     */
    int mEventLogIndex ;
    
    SIPX_VIDEO_DISPLAY* mpPreviewDisplay;
    SIPX_VIDEO_DISPLAY* mpVideoDisplay;

    /**
     * Buffer for generated SRTP key
     */
    UtlString mSrtpKey;

    /**
     * State of location header
     */
    bool mbLocationHeaderEnabled;
};

#endif
