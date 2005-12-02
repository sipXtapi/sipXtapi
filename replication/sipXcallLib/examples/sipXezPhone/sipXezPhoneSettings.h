//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _sipXezPhoneSettings_h
#define _sipXezPhoneSettings_h

// SYSTEM INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "os/OsConfigDb.h"
#include "states/PhoneStateMachineObserver.h"
#include "utl/UtlDList.h"

// APPLICATION INCLUDES
// DEFINES
#define SETTINGS_FILENAME "sipxezphone-config"
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
    bool addContact(const UtlString& name, const UtlString& url);

    /** 
     * Removes a contact name and url to the contact collection.
     */
    void removeContact(const UtlString& name, const UtlString& url);
    
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


    wxColor getBackgroundColor() const;

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
};

#endif

