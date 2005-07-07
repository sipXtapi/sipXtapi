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

#ifndef _sipxmgr_h
#define _sipxmgr_h

#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"


#define MAX_RECORD_EVENTS 1024

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
   void disconnect();

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
    * Callback proc for Sip events.
    */ 
   static void  SipCallbackProc( SIPX_CALL hCall,
                            SIPX_LINE hLine,
                            SIPX_CALLSTATE_MAJOR eMajor,
                            SIPX_CALLSTATE_MINOR eMinor,
                            void* pUser); 
                            
   /**
    * Callback proc for line events.
    */ 
   static void  SipLineCallbackProc(SIPX_LINE hLine,
                            SIPX_LINE_EVENT_TYPE_MAJOR eMajor,
                            void* pUser); 
                            
    bool mTransferInProgress;


};

#endif

