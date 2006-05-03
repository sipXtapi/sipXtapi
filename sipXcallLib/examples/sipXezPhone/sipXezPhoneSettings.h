//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _sipXezPhoneSettings_h
#define _sipXezPhoneSettings_h

// SYSTEM INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "os/OsConfigDb.h"
#include "os/OsConfigEncryption.h"
#include "states/PhoneStateMachineObserver.h"
#include "utl/UtlDList.h"
#include "sipXmgr.h"

// APPLICATION INCLUDES
// DEFINES
#define SETTINGS_FILENAME "sipxezphone-config"

// Bitmap values for state information
#define VIDEO_VISIBLE         0x0001
#define CONFERENCE_VISIBLE    0x0002
#define HISTORY_VISIBLE       0x0004
#define MINIMAL_VIEW          0x0008
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * Container for the phone settings.  This is a singleton class, only one
 * class instance.
 */
class sipXezPhoneSettings : public PhoneStateMachineObserver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /**
    * Accessor for the single class instance.
    */
    static sipXezPhoneSettings& getInstance();

    /**
    * Store the settings using ConfigDb
    */
    UtlBoolean saveSettings(void);

    /**
    * Load the settings using ConfigDb
    */
    UtlBoolean loadSettings(void);

    /** 
     * Adds a contact name and url to the contact collection.
     */
    bool addContact(const UtlString& name, const UtlString& url, const UtlString& cert);

    /** 
     * Removes a contact name and url to the contact collection.
     */
    void removeContact(const UtlString& name, const UtlString& url, const UtlString& cert);
    
    /**
    * Mutator for mIdentity.
    */
    void setIdentity(const UtlString& identity) {mIdentity = identity;}

    /**
    * Mutator for mUsername,
    */
    void setUsername(const UtlString& username) {mUsername = username;}

    /**
    * Mutator for mPassword.
    */
    void setPassword(const UtlString& password) {mPassword = password;}

    /**
    * Mutator for mRealm.
    */
    void setRealm(const UtlString& realm) {mRealm = realm;}

    /**
    * Mutator for mProxyServer.
    */
    void setProxyServer(const UtlString& proxyServer) {mProxyServer = proxyServer;}

    /**
    * Mutator for mStunServer.
    */
    void setStunServer(const UtlString& stunServer) {mStunServer = stunServer;}

    /**
    * Mutator for mStunPort.
    */
    void setStunServerPort(const int port) {mStunPort = port;}

    /**
     * Mutator for mTurnServer.
     */
    void setTurnServer(const UtlString& turnServer) {mTurnServer = turnServer;}

    /**
     * Mutator for mTurnPort.
     */
    void setTurnServerPort(const int port) {mTurnPort = port;}

    /**
     * Mutator for mTurnUsername.
     */
    void setTurnUsername(const UtlString& turnUsername) {mTurnUsername = turnUsername;}

    /**
     * Mutator for mTurnPassword.
     */
    void setTurnPassword(const UtlString& turnPassword) {mTurnPassword = turnPassword;}

    /**
     * Mutator for mbEnableIce
     */
    void setEnableIce(const bool bEnable) {mbEnableIce = bEnable;}

    /**
     * Accessor for mIdentity.
     */
    const UtlString& getIdentity() {return mIdentity;}

    /**
     * Accessor for mUsername.
     */
    const UtlString& getUsername() {return mUsername;}

    /**
     * Accessor for mPassword
     */
    const UtlString& getPassword() {return mPassword;}

    /**
     * Accessor for mRealm
     */
    const UtlString& getRealm() {return mRealm;}

    /**
     * Accessor for mProxyServer
     */
    const UtlString& getProxyServer() {return mProxyServer;}

    /**
     * Accessor for mStunServer
     */
    const UtlString& getStunServer() {return mStunServer;}

    /**
     * Accessor for mStunPort
     */
    const int getStunServerPort() {return mStunPort;}

    /**
     * Accessor for mTurnServer
     */
    const UtlString& getTurnServer() {return mTurnServer;}

    /**
     * Accessor for mTurnPort
     */
    const int getTurnServerPort() {return mTurnPort;}

    /**
     * Accessor for mTurnUsername
     */
    const UtlString& getTurnUsername() {return mTurnUsername;}

    /**
     * Accessor for mTurnPassword
     */
    const UtlString& getTurnPassword() {return mTurnPassword;}

    /**
     * Acessor for mbEnableIce
     */
    const bool getIceEnabled() {return mbEnableIce;}

    /**
     * get the "Speaker" volume - can be 1 - 10
     */
    const int getSpeakerVolume() const {return mSpeakerVolume;}

    /**
     * get the "Ringer" volume - can be 1 - 10
     */
    const int getRingerVolume() const {return mRingerVolume;}

    /**
     * get the Mic gain - can be 1 - 10
     */
    const int getMicGain() const {return mMicGain;}

    /**
     * sets the Speaker volume
     */
    void setSpeakerVolume(const int volume) {mSpeakerVolume = volume;}

    /**
     * sets the Ringer volume
     */
    void setRingerVolume(const int volume) {mRingerVolume = volume;}

    /**
     * sets the Mic gain
     */
    void setMicGain(const int gain) {mMicGain = gain;}    

    /**
     * Gets a boolean value indicating whether or not the rport parameter should be used.
     */
    const bool& getUseRport();

    /**
     * Sets a boolean value indicating whether or not the rport parameter should be used.
     */
    void setUseRport(const bool useRport);

    /**
     * Sets a list of audio codecs selected by the user
     */
    void setSelectedAudioCodecs(const UtlDList& listOfCodecs);
    
    /**
     * Gets a boolean value indicating whether or not the auto answer mode should be enalbed.
     */
    const bool& getAutoAnswer() { return mAutoAnswer; }

    /**
     * Sets a boolean value indicating whether or not the auto answer mode should be enabled.
     */
    void setAutoAnswer(const bool bAutoAnswer ) { mAutoAnswer = bAutoAnswer; }


    /**
     * Sets a boolean value indicating whether or not to enable AEC
     */
    void setEnableAEC(const bool bEnable) {mbEnableAEC = bEnable;}

    /**
     * Gets a boolean value indicating whether or not AEC is enable
     */
    bool getEnableAEC() {return mbEnableAEC;}

    /**
     * Sets a boolean value indicating whether or not to enable 
     * out-of-band DTMF tones
    */
    void setEnableOOBDTMF(const bool bEnable) {mbEnableOutOfBandDTMF = bEnable;}

    /**
     * Gets a boolean value indicating whether or not OOB DTMF is enabled
     */
    bool getEnableOOBDTMF() {return mbEnableOutOfBandDTMF;}

    /**
     * Sets a boolean value indicating whether or not to enable 
     * SRTP
     */
    void setEnableSRTP(const bool bEnable) {mbEnableSRTP = bEnable;}

    /**
     * Gets a boolean value indicating whether or not SRTP
     */
    bool getEnableSRTP() {return mbEnableSRTP;}

    /**
     * Sets the codec preference
     */
    void setCodecPref(const int pref) {mCodecPreference = pref;}

    /**
     * Returns the codec preference
     */
    int getCodecPref() {return mCodecPreference;}

    /**
     * Sets the video codec preference
     */
    void setVideoCodecPref(const int pref) {mVideoCodecPreference = pref;}

    /**
     * Returns the video codec preference
     */
    int getVideoCodecPref() {return mVideoCodecPreference;}

    /**
     * Implementation of the IStateTransitions::OnDial method.
     */
    PhoneState* OnDial(const wxString phoneNumber);

    /**
     * Implementation of the IStateTransitions::OnRinging method.
     */
    PhoneState* OnRinging(const SIPX_CALL hCall);

    /**
     * Accessor for the list of recent numbers.
     */
    UtlDList& getRecentNumberList();
    
    /**
     * Setting the test mode enables the "Test" menu.
     * Test mode can be activated by adding "-test" to the command arguements.
     */
    void setTestMode(bool bOn);
    
    /**
     *
     */
    bool getTestMode();

    /*
     * Set video parameters
     */
    void setVideoParameters(int iQuality, int iBitrate, int iFramerate)
        {mVideoQuality=iQuality; mVideoBitrate=iBitrate; mVideoFramerate=iFramerate;}

    /*
     * Get video parameters
     */
    void getVideoParameters(int& iQuality, int& iBitrate, int& iFramerate)
        {iQuality=mVideoQuality; iBitrate=mVideoBitrate; iFramerate=mVideoFramerate;}

    void setVideoCPU(int iCPU)
        {mVideoCPU = iCPU;}

    int getVideoCPU()
        {return mVideoCPU;}

    /*
     * Set SRTP parameters
     */
    void setSrtpParameters(int iSecurityLevel)
        {mSecurityLevel=iSecurityLevel;}

    /*
     * Get SRTP parameters
     */
    void getSrtpParameters(int& iSecurityLevel)
        {iSecurityLevel=mSecurityLevel;}

    /*
     * Set security enable
     */
    void setSecurityEnabled(bool bEnabled)
        {mbSecurityEnable=bEnabled;}

    /*
     * Get security enable
     */
    void getSecurityEnabled(bool& bEnabled)
        {bEnabled=mbSecurityEnable;}

    /*
     * Set default ports
     */
    void setDefaultPorts(int iSipPort, int iRtpPort)
        {mSipPort=iSipPort; mRtpPort=iRtpPort;}

    /*
     * Get default ports
     */
    void getDefaultPorts(int& iSipPort, int& iRtpPort)
        {iSipPort=mSipPort; iRtpPort=mRtpPort;}

    /*
     * Set SMIME parameters
     */
    void setSmimeParameters(UtlString& dbLocation,
                            UtlString& certNickname,
                            UtlString& certDbPassword)
        {
         mDbLocation=dbLocation;
         mCertNickname=certNickname;
         mCertDbPassword=certDbPassword;
         sipxConfigSetSecurityParameters(sipXmgr::getInstance().getSipxInstance(), dbLocation.data(), certNickname.data(), certDbPassword.data());
         } 

    /*
     * Get SMIME parameters
     */
    void getSmimeParameters(UtlString& dbLocation,
                            UtlString& certNickname,
                            UtlString& certDbPassword)
        {
         dbLocation=mDbLocation;
         certNickname=mCertNickname;
         certDbPassword=mCertDbPassword;
         sipxConfigSetSecurityParameters(sipXmgr::getInstance().getSipxInstance(), dbLocation.data(), certNickname.data(), certDbPassword.data());
        }  

    wxColor getBackgroundColor() const;

    /* 
     * EzPhone parameters
     */
    void setPhoneParameters(int xpos, int ypos, int state)
       {mXPos=xpos; mYPos= ypos; mPhoneState=state;}
    void getPhoneParameters(int& xpos, int& ypos, int& state)
       {xpos=mXPos; ypos=mYPos; state=mPhoneState;}
    void setPhoneBackground(int red, int green, int blue)
       {mRed=red; mGreen=green; mBlue=blue;}

    SIPX_CONTACT_TYPE getContactType() const
    {
        return mContactType ;
    }

    void setContactType(SIPX_CONTACT_TYPE contactType) 
    {
        mContactType = contactType ;
    }

    const UtlDList& getContactNames() { return mContactNames; }
    const UtlDList& getContactUrls()  { return mContactUrls; }
    const UtlDList& getContactCerts() { return mContactCerts; }
    
    const UtlString* lookupCertificate(UtlString sipUrl);

    void setLocationHeader(const UtlString& header)
        {mLocationHeader=header;}
    UtlString& getLocationHeader()
        {return mLocationHeader;}

    void limitBandwidth(bool bLimit)
       {mbLimitBandwidth=bLimit;}
    bool isBandwidthLimited()
       {return mbLimitBandwidth;}

    void setSelectedAudioCodecs(const UtlString& codecs)
       {mSelectedAudioCodecs=codecs;}
    UtlString& getSelectedAudioCodecs()
       {return mSelectedAudioCodecs;}
    void setSelectedVideoCodecs(const UtlString& codecs)
       {mSelectedVideoCodecs=codecs;}
    UtlString& getSelectedVideoCodecs()
       {return mSelectedVideoCodecs;}

    void padCodecString(UtlString& in, UtlString& result);
    void unpadCodecString(UtlString& in, UtlString& result);

    int getConnectionIdleTimeout()
        {return mIdleTimeout;}
    void setConnectionIdleTimeout(int idleTimeout)
        {mIdleTimeout=idleTimeout;}

    bool getShortNamesEnabled()
        {return mbShortNames;}
    void setShortNamesEnabled(bool bEnabled)
        {mbShortNames=bEnabled;}

    UtlString& getAudioInputDevice()
        {return mAudioInputDevice;}
    void setAudioInputDevice(const UtlString& device) 
        {mAudioInputDevice = device;}

    UtlString& getAudioOutputDevice()
        {return mAudioOutputDevice;}
    void setAudioOutputDevice(const UtlString& device) 
        {mAudioOutputDevice = device;}

    UtlString& getVideoCaptureDevice()
        {return mVideoCaptureDevice;}
    void setVideoCaptureDevice(const UtlString& device) 
        {mVideoCaptureDevice = device;}


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ CREATORS ================================== */
   /**
    * sipXezPhoneSettings contructor. singleton class.
    */
   sipXezPhoneSettings();
   
   /**
    * Copy constructor - should never be used.
    */
   sipXezPhoneSettings::sipXezPhoneSettings(const sipXezPhoneSettings& src);
   

   /**
    * sipXezPhoneSettings destructor.
    */
   virtual ~sipXezPhoneSettings();

   static sipXezPhoneSettings* spSipXezPhoneSettings;


   /**
    * user's identity string.
    */ 
   UtlString mIdentity;

   /**
    * username string.
    */ 
   UtlString mUsername;

   /**
    * password string.
    */ 
   UtlString mPassword;

   /**
    * user's realm string.
    */ 
   UtlString mRealm;

   /**
    * The proxy server.
    */ 
   UtlString mProxyServer;

   /**
    * The STUN server.
    */ 
   UtlString mStunServer;

   /**
    * The STUN server port
    */
   int mStunPort;

   /**
    * The TURN server
    */ 
   UtlString mTurnServer;

   /** 
    * The TURN server port
    */ 
   int mTurnPort;

   /**
    * The TURN server username
    */
   UtlString mTurnUsername;

   /**
    * The TURN server password
    */
   UtlString mTurnPassword;


   /**
    * The ICE enable/disable flag.
    */
   bool mbEnableIce ;
   
   /**
    * A config db object for storing / loading settings.
    */
   OsConfigDb configDb;
   
   /**
    * Speaker volume value (1 - 10)
    */
   int mSpeakerVolume;

   /**
    * Ringer volume value (1 - 10)
    */
   int mRingerVolume;
   
   /**
    * Microphone gain value (1 - 10)
    */
   int mMicGain;
   
   const unsigned RecentNumbersListSize;
   
   /**
    * Collection of recently placed and received numbers.
    */
   UtlDList mRecentNumbers;
   
   /**
    * Collection of Contact Names
    */
   UtlDList mContactNames;
   
   /**
    * Collection of Contact Urls
    */
   UtlDList mContactUrls;

   UtlDList mContactCerts;
   /** 
    * Enable the Rport setting of the SipUserAgent.
    */
   bool mUseRport;
   
   /**
    * Enable the AutoAnswer mode
    */ 
   bool mAutoAnswer;
   
   /**
    * Turning on test mode enables the "Test" menu
    */
   bool mTestMode;

   /**
    * Enable echo cancellation
    */
   bool mbEnableAEC;

   /**
    * Enable out-of-band DTMF tones
    */
   bool mbEnableOutOfBandDTMF;

   /**
    * Enable out-of-band DTMF tones
    */
   bool mbEnableSRTP;

   /**
    * Codec preference
    */
   int mCodecPreference;

   /**
    * Codec preference
    */
   int mVideoCodecPreference;

   /**
    * Video quality
    */
   int mVideoQuality;

   /**
    * Video bit rate
    */
   int mVideoBitrate;

   /**
    * Video frame rate
    */
   int mVideoFramerate;

   int mVideoCPU;
   
   /**
    * Selected Audio codecs
    */
   //UtlDList mSelectedAudioCodecs;

   /**
    * Enable security
    */
   bool mbSecurityEnable;

   /**
    * SRTP settings
    */
   int mSecurityLevel;

   /** 
    * Default ports
    */
   int mSipPort;
   int mRtpPort;

   /** 
    * SMIME keys and paths
    */
   UtlString mDbLocation;
   UtlString mCertNickname;
   UtlString mCertDbPassword;

   /**
    * Default Contact Type
    */
   SIPX_CONTACT_TYPE mContactType ;

   /**
    * EzPhone attributes
    */
   int mPhoneState;
   int mXPos;
   int mYPos;
   /**
    * Background color
    */
   int mRed;
   int mGreen;
   int mBlue;

   UtlString mLocationHeader;

   /**
    * Boolean for testing limit bandwidth
    */
   bool mbLimitBandwidth;

   /**
    * Codec strings for selected codecs
    */
   UtlString mSelectedAudioCodecs;
   UtlString mSelectedVideoCodecs;

   /** 
    * Idle timeout
    */
   int mIdleTimeout;

   /**
    * Use short headers
    */
   bool mbShortNames;


   UtlString mAudioInputDevice ;

   UtlString mAudioOutputDevice ;

   UtlString mVideoCaptureDevice ;
};

#endif

