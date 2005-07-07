// $Id$
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
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
    * Accessor for mRealm
    */
    const UtlString& getProxyServer() {return mProxyServer;}

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


    wxColor getBackgroundColor();

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
    *
    */
   bool mUseRport;

};

#endif 