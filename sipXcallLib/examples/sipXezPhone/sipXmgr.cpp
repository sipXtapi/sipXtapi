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

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneApp.h"
#include "os/OsSocket.h"
#include "states/PhoneStateMachine.h"

extern sipXezPhoneApp* thePhoneApp;



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
sipXmgr* sipXmgr::spSipXmgr = NULL;

// Constructor - private, the class is a singleton
sipXmgr::sipXmgr() :
   m_hCall(NULL),
   m_hInst(NULL),
   m_hLine(NULL)
{
}

// Destructor.
sipXmgr::~sipXmgr()
{
   if (m_hLine)
   {
      sipxLineRemove(m_hLine);
      m_hLine = NULL;
   }
}

// static accessor for the singleton instance
sipXmgr& sipXmgr::getInstance()
{
   if (spSipXmgr == NULL)
   {
      sipXmgr::spSipXmgr = new sipXmgr();
   }
   return *spSipXmgr;
}

// Initialization of sipXtapi
UtlBoolean sipXmgr::Initialize(const int iSipPort, const int iRtpPort, const bool bUseRport)
{
   sipXezPhoneSettings::getInstance().loadSettings();


   UtlString hostIP;
   OsSocket::getHostIp(&hostIP);
   if (SIPX_RESULT_SUCCESS != sipxInitialize(&m_hInst,
                                             iSipPort,
                                             iSipPort,
                                             iRtpPort,
                                             DEFAULT_CONNECTIONS,
                                             sipXezPhoneSettings::getInstance().getIdentity(),
                                             hostIP))
   {
      return false;
   }
   
   if (SIPX_RESULT_SUCCESS != sipxConfigSetOutboundProxy(m_hInst, sipXezPhoneSettings::getInstance().getProxyServer()))
   {
      return false;
   }
   if (SIPX_RESULT_SUCCESS != sipxConfigEnableRport(m_hInst, sipXezPhoneSettings::getInstance().getUseRport()))
   {
      return false;
   } 

   if (SIPX_RESULT_SUCCESS != sipxListenerAdd(m_hInst, sipXmgr::SipCallbackProc, NULL))
   {
      return false;
   } 
   
   if (SIPX_RESULT_SUCCESS != sipxLineListenerAdd(m_hInst, sipXmgr::SipLineCallbackProc, NULL))
   {
      return false;
   }

   if (SIPX_RESULT_SUCCESS != sipxLineAdd(m_hInst, sipXezPhoneSettings::getInstance().getIdentity(), true, &m_hLine))
   {
      return false;
   }

   if (SIPX_RESULT_SUCCESS != sipxLineAddCredential(m_hLine, 
                                                    sipXezPhoneSettings::getInstance().getUsername(),
                                                    sipXezPhoneSettings::getInstance().getPassword(),
                                                    sipXezPhoneSettings::getInstance().getRealm()) )
   {
      return false;
   }
   

   sipXmgr::getInstance().setSpeakerVolume(sipXezPhoneSettings::getInstance().getSpeakerVolume());
   sipXmgr::getInstance().setMicGain(sipXezPhoneSettings::getInstance().getMicGain());
   return true;
}


void sipXmgr::UnInitialize()
{
    //sipxUnInitialize(m_hInst);
}

// Callback for handling line events
void sipXmgr::SipLineCallbackProc(SIPX_LINE hLine,
                                  SIPX_LINE_EVENT_TYPE_MAJOR eMajor,
                                  void* pUserData)
{
   char szLineURI[1024];
   char szEventDesc[128] ;
   size_t theSize;
   UtlString logMsg;

   sipxLineGetURI(hLine, szLineURI, 1024, theSize);   
   sipxLineEventToString(eMajor, szEventDesc, sizeof(szEventDesc));
   logMsg = UtlString(szEventDesc) + UtlString(":") + UtlString(szLineURI) + UtlString("\n");
   thePhoneApp->addLogMessage(logMsg);
}

// Callback for handling Sip Events
void sipXmgr::SipCallbackProc( SIPX_CALL hCall,
                               SIPX_LINE hLine,
                               SIPX_CALLSTATE_MAJOR eMajor,
                               SIPX_CALLSTATE_MINOR eMinor,
                               void* pUser)
{
    char szEventDesc[128] ;

    sipxEventToString(eMajor, eMinor, szEventDesc, sizeof(szEventDesc)) ;
    printf("<-> Received Event: %s\n", szEventDesc) ;
    thePhoneApp->addLogMessage(szEventDesc);    

   char szCallID[1024];
   
   sipxCallGetID(hCall, szCallID, 1024);
   thePhoneApp->addLogMessage("CallID = " + UtlString(szCallID) + "\n");
   
   char szLineURI[1024];
   size_t theSize;
   sipxLineGetURI(hLine, szLineURI, 1024, theSize);
   thePhoneApp->addLogMessage("LineURI = " + UtlString(szLineURI) + "\n");
   
    if (0 != hCall) // if we have a valid call Id
    {
        if (getInstance().mTransferInProgress)
        {
        }
    }
   
    switch (eMajor)
    {
      case CONNECTED:
         PhoneStateMachine::getInstance().OnConnected();
         break;
      case REMOTE_ALERTING:
         PhoneStateMachine::getInstance().OnRemoteAlerting();
         break;
      case DISCONNECTED:
         if (DISCONNECTED_BUSY == eMinor)
         {
            PhoneStateMachine::getInstance().OnRemoteBusy();
         }
         else
         {
            PhoneStateMachine::getInstance().OnDisconnected(hCall);
         }
         break;
      case OFFERING: 
        PhoneStateMachine::getInstance().OnOffer(hCall);
        break;
      case ALERTING:
         PhoneStateMachine::getInstance().OnRinging(hCall);
         break;
      default:
         break;      
    }
}

void sipXmgr::placeCall(wxString szNumber)
{
   placeCall(szNumber.c_str(),
    sipXezPhoneSettings::getInstance().getIdentity(),
    sipXezPhoneSettings::getInstance().getUsername(),
    sipXezPhoneSettings::getInstance().getPassword(),
    sipXezPhoneSettings::getInstance().getRealm());
}


void sipXmgr::disconnect()
{
   if (m_hCall != NULL)
   {
      sipxCallDestroy(m_hCall);
   }
}

// Place a call to szSipUrl as szFromIdentity
bool sipXmgr::placeCall(const char* szSipUrl, const char* szFromIdentity, const char* szUsername, const char* szPassword, const char *szRealm)
{
    bool bRC = false ;

    sipxCallCreate(m_hInst, m_hLine, &m_hCall) ;    
    sipxCallConnect(m_hCall, szSipUrl) ;
   
    return bRC ;
}

// release the singleton instance of this class
void sipXmgr::release()
{
   if (sipXmgr::spSipXmgr)
   {
      sipxLineRemove(sipXmgr::spSipXmgr->m_hLine);
      delete sipXmgr::spSipXmgr;
      sipXmgr::spSipXmgr = NULL;
      sipXezPhoneSettings::getInstance().saveSettings();
   }
}

// get the "Speaker" volume - can be 1 - 10
const int sipXmgr::getSpeakerVolume() const
{
   int volume;
   
   sipxAudioGetVolume(m_hInst, SPEAKER, volume);
   
   return volume;
}

// get the "Ringer" volume - can be 1 - 10
const int sipXmgr::getRingerVolume() const
{
   int volume;
   
   sipxAudioGetVolume(m_hInst, RINGER, volume);
   
   return volume;
}

// get the Mic gain - can be 1 - 10
const int sipXmgr::getMicGain() const
{
   int gain;
   
   sipxAudioGetGain(m_hInst, gain);
   
   return gain;
}

// sets the Speaker volume
void sipXmgr::setSpeakerVolume(const int volume)
{
   sipxAudioSetVolume(m_hInst, SPEAKER, volume);
}

// sets the Ringer volume
void sipXmgr::setRingerVolume(const int volume)
{
   sipxAudioSetVolume(m_hInst, RINGER, volume);
}

// sets the Mic gain
void sipXmgr::setMicGain(const int gain)
{
   sipxAudioSetGain(m_hInst, gain);
}

// sets the current call handle
void sipXmgr::setCurrentCall(const SIPX_CALL hCall)
{
    m_hCall = hCall;
}

// gets the current line handle
const SIPX_LINE sipXmgr::getCurrentLine() const
{
    return m_hLine;
}

const SIPX_CALL sipXmgr::getCurrentCall() const
{
    return m_hCall;
}

void sipXmgr::holdCurrentCall()
{
    sipxCallHold(m_hCall);
}
   
void sipXmgr::unholdCurrentCall()
{
    sipxCallUnhold(m_hCall);
}

void sipXmgr::setInTransfer(const bool bXfer)
{
    mTransferInProgress = bXfer;
    return;
}

const bool sipXmgr::getInTransfer() const
{
    return mTransferInProgress;
}
      
      
