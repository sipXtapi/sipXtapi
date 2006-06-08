//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "stdwx.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
//#include "tapi/sipXtapiInternal.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneApp.h"
#include "os/OsSocket.h"
#include "os/OsFS.h"
#include "states/PhoneStateMachine.h"
#include "utl/UtlInt.h"
#include "utl/UtlString.h"
#ifdef HAVE_NSS
#include <nspr.h>
#endif

//#include "tapi/sipXtapiInternal.h"

#ifdef DIRECT_SHOW_RENDER
    #import "..\SampleGrabberCallback\Debug\SampleGrabberCallback.dll"
    #include "debug\samplegrabbercallback.tlh"
#endif

extern sipXezPhoneApp* thePhoneApp;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_STUN_KEEPALIVE 24

// STATIC VARIABLE INITIALIZATIONS
sipXmgr* sipXmgr::spSipXmgr = NULL;

// Constructor - private, the class is a singleton
sipXmgr::sipXmgr() :
   m_hInst(NULL),
   m_hLine(0),
   m_hCall(0),
   m_hConf(0),
   m_hTransferInProgress(0),
   m_hCallController(0),
   mpVideoDisplay(NULL),
   mpPreviewDisplay(NULL),
   mSrtpKey(""),
   mbLocationHeaderEnabled(false),
   mTransferInProgress(false)
{
    mpVideoDisplay = new SIPX_VIDEO_DISPLAY;
    memset(mpVideoDisplay, 0, sizeof(SIPX_VIDEO_DISPLAY));
    
    mpPreviewDisplay = new SIPX_VIDEO_DISPLAY;
    memset(mpPreviewDisplay, 0, sizeof(SIPX_VIDEO_DISPLAY));
    
    clearEventLog() ;
}

// Destructor.
sipXmgr::~sipXmgr()
{
   if (m_hLine)
   {
      sipxLineRemove(m_hLine);
      m_hLine = 0;
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
    sipxConfigSetLogLevel(LOG_LEVEL_DEBUG);
    sipxConfigSetLogFile("ezphone.log"); 

//    UtlString hostIP;
    
//    const char* addresses[SIPX_MAX_IP_ADDRESSES];
//    const char* addressAdapterNames[SIPX_MAX_IP_ADDRESSES];
//    int numAddresses = 0;
//    char szIP[32];
//    
//    sipxConfigGetAllLocalNetworkIps(addresses, addressAdapterNames, numAddresses);
//    for (int i = 0; i < numAddresses; i++)
//    {
//        // TODO - perform a connection check on the IP
//        // if (SUCCESS == connectionCheck())
//        {
//            strcpy(szIP, addresses[i]);
//            break;
//        }
//    }
  
    UtlString dbLocation;
    UtlString certNickname;
    UtlString certDbPassword;
    int iSipTlsPort = 5061;
    sipXezPhoneSettings::getInstance().getSmimeParameters(dbLocation, certNickname, certDbPassword);

    if (certNickname.length() == 0)
    {
        iSipTlsPort = -1 ;
    }
    
    if (SIPX_RESULT_SUCCESS != sipxInitialize(&m_hInst,
                                                iSipPort,
                                                iSipPort,
                                                iSipTlsPort,
                                                iRtpPort,
                                                DEFAULT_CONNECTIONS,
                                                sipXezPhoneSettings::getInstance().getIdentity(),
                                                NULL,
                                                false, 
                                                certNickname,
                                                certDbPassword,
                                                "."
                                                ))
    {
        wxMessageBox("sipXezPhone failed to initialize sipXtapi (expiration or port in use?)", "Unable to start") ;
        return false;
    }

    // set dns srv timeout to something reasonable
    sipxConfigSetDnsSrvTimeouts(2, 2);

    // Enable GIPS tracing by default
    sipxConfigEnableGIPSTracing(m_hInst, true) ;

    if (sipXezPhoneSettings::getInstance().getAudioInputDevice().length())
    {
        sipxAudioSetCallInputDevice(m_hInst, sipXezPhoneSettings::getInstance().getAudioInputDevice()) ;
    }
    if (sipXezPhoneSettings::getInstance().getAudioOutputDevice().length())
    {
        sipxAudioSetRingerOutputDevice(m_hInst, sipXezPhoneSettings::getInstance().getAudioOutputDevice()) ;
        sipxAudioSetCallOutputDevice(m_hInst, sipXezPhoneSettings::getInstance().getAudioOutputDevice()) ;
    }
#ifdef VIDEO
    if (sipXezPhoneSettings::getInstance().getVideoCaptureDevice().length())
    {
        sipxConfigSetVideoCaptureDevice(m_hInst, sipXezPhoneSettings::getInstance().getVideoCaptureDevice()) ;
    }
#endif

    sipxConfigSetUserAgentName(m_hInst, "sipXezPhone", false);
    sipxConfigEnableSipDateHeader(m_hInst, false) ;    
    // sipxConfigEnableSipAllowHeader(m_hInst, false) ;

    if (UtlString(sipXezPhoneSettings::getInstance().getStunServer()).strip(UtlString::both, ' ').length() > 0)
    {    
        sipxConfigEnableStun(m_hInst,
                             sipXezPhoneSettings::getInstance().getStunServer().data(), 
                             sipXezPhoneSettings::getInstance().getStunServerPort(), 
                             DEFAULT_STUN_KEEPALIVE); 
        //Sleep(2000); // kludge alert - wait for the stun response
    }

    if (UtlString(sipXezPhoneSettings::getInstance().getTurnServer()).strip(UtlString::both, ' ').length() > 0)
    {    
        sipxConfigEnableTurn(m_hInst,
                             sipXezPhoneSettings::getInstance().getTurnServer().data(), 
                             sipXezPhoneSettings::getInstance().getTurnServerPort(), 
                             sipXezPhoneSettings::getInstance().getTurnUsername(),
                             sipXezPhoneSettings::getInstance().getTurnPassword(),
                             DEFAULT_STUN_KEEPALIVE); 
    }

    if (sipXezPhoneSettings::getInstance().getIceEnabled())
    {
        sipxConfigEnableIce(m_hInst) ;
    }
    else
    {
        sipxConfigDisableIce(m_hInst) ;
    }

    if (sipXezPhoneSettings::getInstance().getProxyServer().length() > 0)
    {
        if (SIPX_RESULT_SUCCESS != sipxConfigSetOutboundProxy(m_hInst, sipXezPhoneSettings::getInstance().getProxyServer()))
        {
            return false;
        }
    }
    if (SIPX_RESULT_SUCCESS != sipxConfigEnableRport(m_hInst, sipXezPhoneSettings::getInstance().getUseRport()))
    {
        return false;
    } 

    if (SIPX_RESULT_SUCCESS != sipxEventListenerAdd(m_hInst, SipCallbackProc, NULL))
    {
        return false;
    } 

    // TODO - softcode this value
    if (SIPX_RESULT_SUCCESS != sipxConfigSetRegisterExpiration(m_hInst, 300))
    {
        return false;
    }

    // Select Contact Id
    SIPX_CONTACT_ID contactId = 0 ;
    size_t numAddresses = 0;
    SIPX_CONTACT_ADDRESS addresses[32];
    sipxConfigGetLocalContacts(m_hInst, addresses, 32, numAddresses);    
    for (unsigned int i = 0; i < numAddresses; i++)
    {
        if (addresses[i].eContactType == sipXezPhoneSettings::getInstance().getContactType())
        {
            contactId = addresses[i].id ;
            break ;            
        }
    }
        
    if (SIPX_RESULT_SUCCESS != sipxLineAdd(m_hInst, sipXezPhoneSettings::getInstance().getIdentity(), &m_hLine, contactId))
    {
        return false;
    }

    if (    (sipXezPhoneSettings::getInstance().getUsername().length() > 0) &&
            (sipXezPhoneSettings::getInstance().getPassword().length() > 0))
    {
        if (SIPX_RESULT_SUCCESS != sipxLineAddCredential(m_hLine, 
                                                        sipXezPhoneSettings::getInstance().getUsername(),
                                                        sipXezPhoneSettings::getInstance().getPassword(),
                                                        sipXezPhoneSettings::getInstance().getRealm()) )
        {
            return false;
        }

        if (SIPX_RESULT_SUCCESS != sipxLineRegister(m_hLine, true))
        {
            return false;
        }
    }
    
    
    sipXmgr::getInstance().setSpeakerVolume(sipXezPhoneSettings::getInstance().getSpeakerVolume());
    sipXmgr::getInstance().setMicGain(sipXezPhoneSettings::getInstance().getMicGain());

    sipXmgr::getInstance().enableAEC(sipXezPhoneSettings::getInstance().getEnableAEC());
    sipXmgr::getInstance().enableOutOfBandDTMF(sipXezPhoneSettings::getInstance().getEnableOOBDTMF());

    int audioPref = sipXezPhoneSettings::getInstance().getCodecPref();
    if (audioPref == 4)
    {
        UtlString codecName = sipXezPhoneSettings::getInstance().getSelectedAudioCodecs();
        sipXmgr::getInstance().setAudioCodecByName(codecName.data());
    }
    else
    {
        sipXmgr::getInstance().setCodecPreferences(sipXezPhoneSettings::getInstance().getCodecPref()); 
    }


#ifdef VIDEO
    int iQuality, iBitRate, iFrameRate ;
    sipXezPhoneSettings::getInstance().getVideoParameters(iQuality, iBitRate, iFrameRate) ;

    sipxConfigSetVideoParameters(m_hInst, iBitRate, iFrameRate) ;
    sipxConfigSetVideoCpuUsage(m_hInst, sipXezPhoneSettings::getInstance().getVideoCPU()) ;
#endif
    
    sipxConfigEnableSipShortNames(m_hInst, sipXezPhoneSettings::getInstance().getShortNamesEnabled());
    UtlString locationHeader = sipXezPhoneSettings::getInstance().getLocationHeader().data();
    if (locationHeader.length() != 0)
    {
        sipxConfigSetLocationHeader(m_hInst, locationHeader.data());
        enableLocationHeader(true);
    }
    sipxConfigSetDnsSrvTimeouts(2, 2);
    return true;
}


void sipXmgr::UnInitialize()
{
    if (sipXmgr::spSipXmgr)
    {
        sipxEventListenerRemove(sipXmgr::spSipXmgr->m_hInst, SipCallbackProc, NULL);
        sipxLineRemove(sipXmgr::spSipXmgr->m_hLine);
        sipXmgr::spSipXmgr->m_hLine = 0;
          
        // We need to wait for lines to unregister / calls to drop
        if (sipxUnInitialize(sipXmgr::spSipXmgr->m_hInst) != SIPX_RESULT_SUCCESS)
        {
            sipxUnInitialize(sipXmgr::spSipXmgr->m_hInst, true) ;
        }

        sipXmgr::spSipXmgr->m_hInst = NULL;
    }
}


bool sipXmgr::SipCallbackProc(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData)
{
    // Add to Event Log
    sipXmgr::getInstance().addToEventLog(category, pInfo, pUserData) ;

    // Delegate to handlers
    switch (category)
    {
        case EVENT_CATEGORY_CALLSTATE:
            return sipXmgr::getInstance().handleCallstateEvent(pInfo, pUserData);
        case EVENT_CATEGORY_LINESTATE:
            return sipXmgr::getInstance().handleLinestateEvent(pInfo, pUserData);
        case EVENT_CATEGORY_INFO_STATUS:
            return sipXmgr::getInstance().handleInfoStatusEvent(pInfo, pUserData);
        case EVENT_CATEGORY_INFO:
            return sipXmgr::getInstance().handleInfoEvent(pInfo, pUserData);
        case EVENT_CATEGORY_SECURITY:
            return sipXmgr::getInstance().handleSecurityEvent(pInfo, pUserData);
        case EVENT_CATEGORY_MEDIA:
            return sipXmgr::getInstance().handleMediaEvent(pInfo, pUserData);
        default:
            return false;
    }
    return false;
}

bool sipXmgr::handleCallstateEvent(void* pInfo, void* pUserData)
{
    assert (pInfo != NULL);
    SIPX_CALLSTATE_INFO* pCallInfo = static_cast<SIPX_CALLSTATE_INFO*>(pInfo);   

    char szEventDesc[128] ;

    sipxEventToString(EVENT_CATEGORY_CALLSTATE, pInfo, szEventDesc, sizeof(szEventDesc)) ;
    printf("<-> Received Event: %s \n", szEventDesc) ;
    thePhoneApp->addLogMessage(szEventDesc);    

    char szCallID[1024];
    memset(szCallID, 0, sizeof(szCallID));
    
    sipxCallGetID(pCallInfo->hCall, szCallID, 1024);
    thePhoneApp->addLogMessage("CallID = " + UtlString(szCallID) + "\n");

    mCallId = szCallID;
    
    char szLineURI[1024];
    memset(szLineURI, 0, sizeof(szLineURI));
    size_t theSize;
    sipxLineGetURI(pCallInfo->hLine, szLineURI, 1024, theSize);
    thePhoneApp->addLogMessage("LineURI = " + UtlString(szLineURI) + "\n");

    SIPX_CALL hCall = pCallInfo->hCall;
    if (0 != hCall) // if we have a valid call Id
    {
        if (getInstance().mTransferInProgress)
        {
        }
    }

    switch (pCallInfo->event)
    {
        case CALLSTATE_REMOTE_OFFERING:
        {
            break;
        }
        case CALLSTATE_NEWCALL:
            if (pCallInfo->cause == CALLSTATE_CAUSE_TRANSFERRED)
            {
                mTransferInProgress = true;
                m_hTransferInProgress = pCallInfo->hCall;
                m_hCallController = m_hCall;
                m_hCall = m_hTransferInProgress;
            }
            else if (pCallInfo->cause == CALLSTATE_CAUSE_TRANSFER)
            {
                m_hCall = pCallInfo->hCall;
            }
            break;
        case CALLSTATE_CONNECTED:
            PhoneStateMachine::getInstance().OnConnected();            
            break;
        case CALLSTATE_BRIDGED:
            thePhoneApp->addLogMessage("Local HOLD (Bridging)\n");
            break ;
        case CALLSTATE_REMOTE_HELD:
            PhoneStateMachine::getInstance().OnConnectedInactive();
            //thePhoneApp->addLogMessage("Remote HOLD\n");
            break ;
        case CALLSTATE_HELD:
            PhoneStateMachine::getInstance().OnConnectedInactive();
            break ;
        case CALLSTATE_REMOTE_ALERTING:
            PhoneStateMachine::getInstance().OnRemoteAlerting();
            break;
        case CALLSTATE_DISCONNECTED:
            if (CALLSTATE_CAUSE_BUSY == pCallInfo->cause)
            {
                PhoneStateMachine::getInstance().OnRemoteBusy();
                sipXmgr::getInstance().disconnect(pCallInfo->hCall, false);
            }
            else if (mTransferInProgress && m_hTransferInProgress && pCallInfo->hCall == m_hCallController)
            {
                m_hTransferInProgress = 0;
                mTransferInProgress = false;
                m_hCallController = 0;
            }
            else
            {
                PhoneStateMachine::getInstance().OnDisconnected(pCallInfo->hCall);
            }
            break;
        case CALLSTATE_OFFERING: 
            PhoneStateMachine::getInstance().OnOffer(hCall);
            break;
        case CALLSTATE_ALERTING:
            PhoneStateMachine::getInstance().OnRinging(hCall);
            break;
        case CALLSTATE_DESTROYED:
            break;
            break;
// ::TODO:: FIXME with new media events
//        case CALLSTATE_CONNECTION_IDLE:
//            sprintf(szEventDesc, "Idle for %d seconds\n", pCallInfo->idleTime);
//            thePhoneApp->addLogMessage(szEventDesc);
//            break;
//        case CALLSTATE_AUDIO_EVENT:
//            if (pCallInfo->cause == CALLSTATE_CAUSE_AUDIO_START)
//            {
//                sprintf(szPayloadType, "%d", pCallInfo->codecs.audioCodec.iPayloadType);
//                if (pCallInfo->codecs.bIsEncrypted)
//                {
//                    sprintf(szSrtpStatus, "encrypted");
//                }
//                else
//                {
//                    sprintf(szSrtpStatus, "unencrypted");
//                }
//                thePhoneApp->addLogMessage("Acodec: " + 
//                                            UtlString(pCallInfo->codecs.audioCodec.cName) +
//                                            " (" +
//                                            UtlString(szPayloadType) +
//                                            "," +
//                                            UtlString(szSrtpStatus) +
//                                            ")\n");
//                if (pCallInfo->codecs.videoCodec.iPayloadType != -1)
//                {
//                    sprintf(szPayloadType, "%d", pCallInfo->codecs.videoCodec.iPayloadType);
//                    thePhoneApp->addLogMessage("Vcodec: " + 
//                                                UtlString(pCallInfo->codecs.videoCodec.cName) +
//                                                " (" +
//                                                UtlString(szPayloadType) +
//                                                ")\n");
//            }
//            }
//            break;
        default:
            break;      
    }
    return true;
}

bool sipXmgr::handleLinestateEvent(void* pInfo, void* pUserData)
{
    assert (pInfo != NULL);
    SIPX_LINESTATE_INFO* pLineInfo = static_cast<SIPX_LINESTATE_INFO*>(pInfo);   

    char szLineURI[1024];
    char szEventDesc[128] ;
    size_t theSize;
    UtlString logMsg;

    sipxLineGetURI(pLineInfo->hLine, szLineURI, 1024, theSize);   
    
    sipxEventToString(EVENT_CATEGORY_LINESTATE, pInfo, szEventDesc, sizeof(szEventDesc));
    logMsg = UtlString(szEventDesc) + UtlString(":") + UtlString(szLineURI) + UtlString("\n");
    thePhoneApp->addLogMessage(logMsg);    
    return true;
}

bool sipXmgr::handleInfoStatusEvent(void* pInfo, void* pUserData)
{
    assert (pInfo != NULL);
    SIPX_INFOSTATUS_INFO* pInfoStatus = static_cast<SIPX_INFOSTATUS_INFO*>(pInfo);  
    
    char szCode[256];
    sprintf(szCode, "%d", pInfoStatus->responseCode); 
    
    thePhoneApp->addLogMessage(UtlString("INFO msg response:") + 
                               UtlString(" ") +
                               UtlString(szCode) +
                               UtlString(" - ") +
                               UtlString(pInfoStatus->szResponseText) +
                               UtlString("\n"));
    
    return true;
}

bool sipXmgr::handleInfoEvent(void* pInfo, void* pUserData)
{
    assert (pInfo != NULL);
    SIPX_INFO_INFO* pInfoMsg = static_cast<SIPX_INFO_INFO*>(pInfo);  
    
    char* pMsg = NULL;
    
    pMsg = (char*)malloc(pInfoMsg->nContentLength+256);
    memset((void*)pMsg, 0, pInfoMsg->nContentLength + 256);
    UtlString body(pInfoMsg->pContent, pInfoMsg->nContentLength);
    
    
    sprintf(pMsg, "type=%s, body=%s", pInfoMsg->szContentType, body.data()); 
    
    thePhoneApp->addLogMessage(UtlString("incoming INFO msg : ") + 
                               UtlString(pMsg) +
                               UtlString("\n"));
    free((void*)pMsg);
    return true;
}

bool sipXmgr::handleSecurityEvent(void* pInfo, void* pUserData)
{
    SIPX_SECURITY_INFO* pSecurityMsg = static_cast<SIPX_SECURITY_INFO*>(pInfo);  
    bool bRet = true;
    char szMessage[768];
    char szEvent[256];
    char szCause[256];
    
    sipxSecurityEventToString(pSecurityMsg->event, szEvent, sizeof(szEvent));
    sipxSecurityCauseToString(pSecurityMsg->cause, szCause, sizeof(szCause));

    sprintf(szMessage, "%s - %s\n", szEvent, szCause);
    
    thePhoneApp->addLogMessage(szMessage);

    // if this is a signature callback,
    // or a TLS server certificate callback, 
    // we can choose to
    // reject the signature or certificate here
    if   (pSecurityMsg->cause == SECURITY_CAUSE_SIGNATURE_NOTIFY ||
         (pSecurityMsg->event == SECURITY_TLS &&
         pSecurityMsg->cause == SECURITY_CAUSE_TLS_SERVER_CERTIFICATE ) ) 
    {
        if (pSecurityMsg->pCertificate)
        {
            bRet = true;
        }
        else
        {
            bRet = false;
        }

        sprintf(szMessage, "CERTIFICATE SIGNATURE: %s\n", pSecurityMsg->szSubjAltName);
        thePhoneApp->addLogMessage(szMessage);

        // do some check here with pSecurityMsg->szSubjAltName
        // set bRet accordingly
    }
    else
    {
        bRet = false;
    }
    return bRet;
}

bool sipXmgr::handleMediaEvent(void* pInfo, void* pUserData)
{
    SIPX_MEDIA_INFO* pMediaInfo = static_cast<SIPX_MEDIA_INFO*>(pInfo);  
    char szMessage[768];
    char szEvent[256];
    char szCause[256];
    
    sipxMediaEventToString(pMediaInfo->event, szEvent, sizeof(szEvent));
    sipxMediaCauseToString(pMediaInfo->cause, szCause, sizeof(szCause));

    sprintf(szMessage, "%s - %s\n", szEvent, szCause);
    
    thePhoneApp->addLogMessage(szMessage);
    return true;
}
// Callback for handling line events
/*
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
*/
// Callback for handling Sip Events
/*
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
*/

void sipXmgr::placeCall(wxString szNumber)
{
   placeCall(szNumber.c_str(),
    sipXezPhoneSettings::getInstance().getIdentity(),
    sipXezPhoneSettings::getInstance().getUsername(),
    sipXezPhoneSettings::getInstance().getPassword(),
    sipXezPhoneSettings::getInstance().getRealm());
}


bool sipXmgr::disconnect(SIPX_CALL hCall, bool bDisconnectAll)
{
    SIPX_CALL aCall[10];
    bool bDisconnected = true;
    size_t numConnections;

    if (m_hConf != 0)
    {
        if (!bDisconnectAll)
        {
            sipxConferenceRemove(m_hConf, hCall);
            sipxConferenceGetCalls(m_hConf, aCall, 10, numConnections);
            if (numConnections == 0)
            {
                sipxConferenceDestroy(m_hConf);
                m_hConf = 0;
                mConfCallHandleMap.destroyAll();
            }
            else
            {
                bDisconnected = false;
            }
        }
        else
        {
            sipxConferenceGetCalls(m_hConf, aCall, 10, numConnections);
            for (int i=0; i<numConnections && i<10; i++)
            {
                sipxConferenceRemove(m_hConf, aCall[i]);
            }
            sipxConferenceDestroy(m_hConf);
            m_hConf = 0;
            mConfCallHandleMap.destroyAll();
        }
    }
    if (hCall != 0)
    {
        sipxCallDestroy(hCall);
    }
    else if (m_hCall != 0)
    {
        sipxCallDestroy(m_hCall);
    }
    return bDisconnected;
}

// Place a call to szSipUrl as szFromIdentity
bool sipXmgr::placeCall(const char* szSipUrl,
                        const char* szFromIdentity,
                        const char* szUsername,
                        const char* szPassword, 
                        const char *szRealm)
{
    bool bRC = false ;

    sipxCallCreate(m_hInst, m_hLine, &m_hCall) ;
   
    SIPX_VIDEO_DISPLAY display;
    SIPX_SECURITY_ATTRIBUTES security;
    
    display.handle = sipXmgr::getInstance().getVideoWindow();
#ifdef DIRECT_SHOW_RENDER    
    display.type = DIRECT_SHOW_FILTER;
#else
    display.type = SIPX_WINDOW_HANDLE_TYPE;
#endif

    // Select Contact Id
    SIPX_CONTACT_ID contactId = 0 ;
    size_t numAddresses = 0;
    SIPX_CONTACT_ADDRESS addresses[32];
    sipxConfigGetLocalContacts(m_hInst, addresses, 32, numAddresses);    
    for (unsigned int i = 0; i < numAddresses; i++)
    {
        if (addresses[i].eContactType == sipXezPhoneSettings::getInstance().getContactType())
        {
            contactId = addresses[i].id ;
            break ;            
        }
    }
        
    // do a certficate lookup based on szSipUrl    
    const UtlString* pCertFile = sipXezPhoneSettings::getInstance().lookupCertificate(szSipUrl);
    int iSecurity;
    UtlString srtpKey;
    UtlString dbLocation;
    UtlString certNickname;
    UtlString certDbPassword;

    sipXezPhoneSettings::getInstance().getSrtpParameters(iSecurity);
    bool bSecurity;
    sipXezPhoneSettings::getInstance().getSecurityEnabled(bSecurity);

    SIPX_CALL_OPTIONS options;
    memset((void*)&options, 0, sizeof(SIPX_CALL_OPTIONS));
    options.cbSize = sizeof(SIPX_CALL_OPTIONS);
    options.sendLocation = sipXmgr::getInstance().isLocationHeaderEnabled();
    options.bandwidthId =  AUDIO_CODEC_BW_DEFAULT;

    if (pCertFile && bSecurity && iSecurity > 0)
    {
        sipXezPhoneSettings::getInstance().getSmimeParameters(dbLocation, certNickname, certDbPassword);

        security.setSecurityLevel((SIPX_SRTP_LEVEL)iSecurity);

        char szPublicKey[4096];
        unsigned long actualRead = 0;
    
        OsFile publicKeyFile(pCertFile->data());

        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
        publicKeyFile.close();
    
        UtlString der(szPublicKey, actualRead);

        security.setSmimeKey(szPublicKey, actualRead);
        
        security.setSrtpKey(mSrtpKey.data(), 30);
        security.setSecurityLevel((SIPX_SRTP_LEVEL)iSecurity);
		if (thePhoneApp->getFrame().getVideoVisible())
		{
			sipxCallConnect(m_hCall, szSipUrl, contactId, &display, &security, true, &options) ;
		}
		else
		{
			sipxCallConnect(m_hCall, szSipUrl, contactId, NULL, &security, true, &options) ;
		}
    }
    else
    {
		if (thePhoneApp->getFrame().getVideoVisible())
		{
	        sipxCallConnect(m_hCall, szSipUrl, contactId, &display, NULL, true, &options);
		}
		else
		{
	        sipxCallConnect(m_hCall, szSipUrl, contactId, NULL, NULL, true, &options);
		}
    }
   
    return bRC ;
}

// release the singleton instance of this class
void sipXmgr::release()
{
   if (sipXmgr::spSipXmgr)
   {
     
      delete sipXmgr::spSipXmgr;
      sipXmgr::spSipXmgr = NULL;
      //sipXezPhoneSettings::getInstance().saveSettings();
   }
}

void sipXmgr::readPublicKeyFile(UtlString& der, UtlString filename)
{
    // read the remote party's public key for s/mime encryption
    OsFile publicKeyFile(filename);
    
    char szPublicKey[4096];
    unsigned long actualRead = 0;

    publicKeyFile.open();
    publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
    publicKeyFile.close();

    der.append(szPublicKey, actualRead);

}
/*
void sipXmgr::readPkcs12File(UtlString& pkcs12, UtlString filename)
{
    // now decrypt
    OsFile privateKeyFile(filename);
    // build a key
    
    char szPkcs12[4096];
    unsigned long actualRead = 0;

    privateKeyFile.open();
    privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
    privateKeyFile.close();

    UtlString myPkcs12(szPkcs12, actualRead);
    pkcs12 = myPkcs12;
}
*/

void sipXmgr::getLocalContacts(size_t nMaxAddresses,
                               SIPX_CONTACT_ADDRESS addresses[],
                               size_t& nActualAddresses) 
{
    sipxConfigGetLocalContacts(m_hInst, addresses, nMaxAddresses, nActualAddresses) ;
}


void sipXmgr::removeCurrentLine()
{
    if (sipXmgr::spSipXmgr)
    {
        sipxLineRemove(sipXmgr::spSipXmgr->m_hLine);
    }    
}

const SIPX_CONF sipXmgr::getCurrentConference() const
{
    return m_hConf;
}

void sipXmgr::setCurrentConference(const SIPX_CONF hConf)
{
    m_hConf = hConf;
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
   int myVol = volume;
   
   if (volume > VOLUME_MAX)
   {
      myVol = VOLUME_MAX;
   }
   // not yet implemented in the Linux
   #ifdef _WIN32
       sipxAudioSetVolume(m_hInst, SPEAKER, myVol);
   #endif
}

// sets the Ringer volume
void sipXmgr::setRingerVolume(const int volume)
{
   int myVol = volume;
   
   if (volume > VOLUME_MAX)
   {
      myVol = VOLUME_MAX;
   }
   sipxAudioSetVolume(m_hInst, RINGER, myVol);
}

// sets the Mic gain
void sipXmgr::setMicGain(const int gain)
{
   int myGain = gain;
   
   if (gain > GAIN_MAX)
   {
      myGain = GAIN_MAX;
   }
   
   // not yet implemented in the Linux
   #ifdef _WIN32
//       sipxAudioSetGain(m_hInst, myGain);
   #endif
   
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

    // if a conference in progress hold all
    // conference members.
    if (m_hConf)
    {
        sipxConferenceHold(m_hConf, true) ;
    }
    else
    {
        // hold the 'main' call
        sipxCallHold(m_hCall) ;
    }
    
}
   
void sipXmgr::unholdCurrentCall()
{
    // if a conference in progress unhold all
    // conference members.
    if (m_hConf)
    {
        sipxConferenceUnhold(m_hConf) ;
    }
    else
    {
        sipxCallUnhold(m_hCall);
    }
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

void sipXmgr::stopTone()
{
    sipxCallStopTone(this->m_hCall);
}      

void sipXmgr::UnRegister()
{
    sipxLineRegister(m_hLine, false);
}

bool sipXmgr::addConfParty(const char* const szParty)
{
    bool bRet = false;

    // first, check to see whether or not a conference already exists

    if (0 == m_hConf)  // if there is no current conference
    {
        SIPX_CALL hCurrentCall = sipXmgr::getInstance().getCurrentCall();
        // create a conference
        if (SIPX_RESULT_SUCCESS !=
            sipxConferenceCreate(m_hInst, &m_hConf) )
        {
            // TODO - log a failure here & alert user
        }
        else
        {
            setCurrentConference(m_hConf);
            if (hCurrentCall)   // if there was a 'current call', join it in with the
                                // conference
            {
                sipxConferenceJoin(m_hConf, hCurrentCall);
            }
        }
    }
    
    if (m_hConf)  // if we have a conference
    {
        SIPX_CALL hNewCall = 0;
        
        SIPX_VIDEO_DISPLAY* pDisplay = new SIPX_VIDEO_DISPLAY;
        // TODO - clean up the memory leak introduced above   
        
        pDisplay->cbSize = sizeof(SIPX_VIDEO_DISPLAY);
        pDisplay->handle = sipXmgr::getInstance().getVideoWindow();
#ifdef DIRECT_SHOW_RENDER        
        pDisplay->type = DIRECT_SHOW_FILTER;
#else
        pDisplay->type = SIPX_WINDOW_HANDLE_TYPE;
#endif         

        // Select Contact Id
        SIPX_CONTACT_ID contactId = 0 ;
        size_t numAddresses = 0;
        SIPX_CONTACT_ADDRESS addresses[32];
        sipxConfigGetLocalContacts(m_hInst, addresses, 32, numAddresses);    
        for (unsigned int i = 0; i < numAddresses; i++)
        {
            if (addresses[i].eContactType == sipXezPhoneSettings::getInstance().getContactType())
            {
                contactId = addresses[i].id ;
                break ;            
            }
        }
        
        SIPX_SECURITY_ATTRIBUTES* pSecurity = NULL;
        SIPX_SECURITY_ATTRIBUTES security;
        bool bSecurityEnabled = false;
        sipXezPhoneSettings::getInstance().getSecurityEnabled(bSecurityEnabled);
        const UtlString* pCertFile = sipXezPhoneSettings::getInstance().lookupCertificate(szParty);

        if (pCertFile && bSecurityEnabled)
        {
            // do a certficate lookup based on szSipUrl    
            int iSecurity;
            UtlString srtpKey;
            UtlString dbLocation;
            UtlString certNickname;
            UtlString certDbPassword;

            sipXezPhoneSettings::getInstance().getSrtpParameters(iSecurity);
            sipXezPhoneSettings::getInstance().getSmimeParameters(dbLocation, certNickname, certDbPassword);

            char szPublicKey[4096];
            unsigned long actualRead = 0;
        
            OsFile publicKeyFile(pCertFile->data());

            publicKeyFile.open();
            publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
            publicKeyFile.close();
        
            UtlString der(szPublicKey, actualRead);

            security.setSmimeKey(szPublicKey, actualRead);
            
            security.setSrtpKey(mSrtpKey.data(), 30);
            security.setSecurityLevel((SIPX_SRTP_LEVEL)iSecurity);

            if (pCertFile && iSecurity > 0)
            {
                pSecurity = &security;
            }
        }

        SIPX_CALL_OPTIONS options;
        memset((void*)&options, 0, sizeof(SIPX_CALL_OPTIONS));
        options.cbSize = sizeof(SIPX_CALL_OPTIONS);
        options.sendLocation = sipXmgr::getInstance().isLocationHeaderEnabled();
        options.bandwidthId =  AUDIO_CODEC_BW_DEFAULT;

        if (SIPX_RESULT_SUCCESS == sipxConferenceAdd(m_hConf, getCurrentLine(), szParty, &hNewCall, contactId,
                                                     pDisplay, pSecurity, true, &options))
        {
            mConfCallHandleMap.insertKeyAndValue(new UtlString(szParty), new UtlInt(hNewCall));
            
            bRet = true;
        }
    }
    return bRet;
}

bool sipXmgr::removeConfParty(const char* const szParty)
{
    bool bRet = false;
    UtlInt* pHandle = NULL;
    UtlString key(szParty);
    pHandle = (UtlInt*)mConfCallHandleMap.findValue(&key);
    
    if (pHandle && SIPX_RESULT_SUCCESS == sipxConferenceRemove(m_hConf, pHandle->getValue()))
    {
        bRet = true;
    }   
    
    if (pHandle)
    {
        mConfCallHandleMap.destroy(&key);
        bRet = true;
    }
    
    if (mConfCallHandleMap.entries() == 0)
    {
        sipxConferenceDestroy(m_hConf);
        m_hConf = 0;
    }
    
    return bRet;
}

bool sipXmgr::getCodecPreferences(int* pCodecPref)
{
    bool rc = false;

    if (pCodecPref)
    {
        if (sipxConfigGetAudioCodecPreferences(m_hInst, (SIPX_AUDIO_BANDWIDTH_ID*)pCodecPref) == SIPX_RESULT_SUCCESS)
        {
            rc = true;
        }
    }
    return rc;
}

bool sipXmgr::getCodecList(UtlString& codecList)
{
    bool rc = false;

    int numCodecs;
    SIPX_AUDIO_CODEC codec;
    UtlString sBandWidth;

    codecList = "";
    
    if (sipxConfigGetNumAudioCodecs(m_hInst, &numCodecs) == SIPX_RESULT_SUCCESS)
    {
        for (int i=0; i<numCodecs; i++)
        {
            if (sipxConfigGetAudioCodec(m_hInst, i, &codec) == SIPX_RESULT_SUCCESS)
            {
                switch (codec.iBandWidth)
                {
                case AUDIO_CODEC_BW_VARIABLE:
                    sBandWidth = " (Variable)";
                    break;
                case AUDIO_CODEC_BW_LOW:
                    sBandWidth = " (Low)";
                    break;
                case AUDIO_CODEC_BW_NORMAL:
                    sBandWidth = " (Normal)";
                    break;
                case AUDIO_CODEC_BW_HIGH:
                    sBandWidth = " (High)";
                    break;
                 }
                codecList = codecList + codec.cName + sBandWidth + "\n"; 
            }
        }
        rc = true;
    }
    return rc;
}

bool sipXmgr::getVideoCodecPreferences(int* pCodecPref)
{
    bool rc = false;
#ifdef VIDEO
    if (pCodecPref)
    {
        if (sipxConfigGetVideoCodecPreferences(m_hInst, (SIPX_VIDEO_BANDWIDTH_ID*)pCodecPref) == SIPX_RESULT_SUCCESS)
        {
            rc = true;
        }
    }
#endif    
    return rc;
}

bool sipXmgr::getVideoCodecList(UtlString& codecList)
{
    bool rc = false;
#ifdef VIDEO
    int numCodecs;
    SIPX_VIDEO_CODEC codec;
    UtlString sBandWidth;

    codecList = "";
    
    if (sipxConfigGetNumVideoCodecs(m_hInst, &numCodecs) == SIPX_RESULT_SUCCESS)
    {
        for (int i=0; i<numCodecs; i++)
        {
            if (sipxConfigGetVideoCodec(m_hInst, i, &codec) == SIPX_RESULT_SUCCESS)
            {
                switch (codec.iBandWidth)
                {
                case AUDIO_CODEC_BW_VARIABLE:
                    sBandWidth = " (Variable)";
                    break;
                case AUDIO_CODEC_BW_LOW:
                    sBandWidth = " (Low)";
                    break;
                case AUDIO_CODEC_BW_NORMAL:
                    sBandWidth = " (Normal)";
                    break;
                case AUDIO_CODEC_BW_HIGH:
                    sBandWidth = " (High)";
                    break;
                 }
                codecList = codecList + codec.cName + sBandWidth + "\n"; 
            }
        }
        rc = true;
    }
#endif    
    return rc;
}

bool sipXmgr::setAudioCodecByName(const char* name)
{
    bool rc = false;

    if (sipxConfigSetAudioCodecByName(m_hInst, name) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
    return rc;
}

bool sipXmgr::setVideoCodecByName(const char* name)
{
    bool rc = false;
#ifdef VIDEO
    if (sipxConfigSetVideoCodecByName(m_hInst, name) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
#endif    
    return rc;
}

void sipXmgr::setPreviewWindow(void* pWnd)
{
    mpPreviewDisplay->type = SIPX_WINDOW_HANDLE_TYPE;
    mpPreviewDisplay->handle = pWnd;
#ifdef VIDEO
    sipxConfigSetVideoPreviewDisplay(sipXmgr::getInstance().getSipxInstance(), mpPreviewDisplay);
#endif
}

void* sipXmgr::getPreviewWindow()
{
    return mpPreviewDisplay->handle;
}
#ifdef DIRECT_SHOW_RENDER
    #include "Qedit.h"
    #include "../samplegrabbercallback/_SampleGrabberCallback_i.c"
#endif
void sipXmgr::setVideoWindow(void* pWnd)
{
#ifdef DIRECT_SHOW_RENDER
    IBaseFilter* pBase = NULL;
    ISampleGrabber* pGrabber = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, (void**)&pBase);
        
    pBase->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);        
    pGrabber->AddRef();
    //hr = pBase->Run(0);
    
    // create our own sample grabber callback obj
    SampleGrabberCallback::ISampGrabCB* pCallback = NULL;
    hr = CoCreateInstance(CLSID_CSampGrabCB, NULL, CLSCTX_INPROC_SERVER,
        IID_ISampGrabCB, (void**)&pCallback);
    pCallback->AddRef();
    pCallback->SetWindow((LONG)pWnd);
    ISampleGrabberCB* pCB = NULL;
    pCallback->QueryInterface(IID_ISampleGrabberCB, (void**) &pCB);
    pCB->AddRef();
    
    if (pGrabber)
    {
        hr = pGrabber->SetCallback(pCB, 1);
    }
        
    if (FAILED(hr))
    {
        // Return an error.
    }

    mpVideoDisplay->type = DIRECT_SHOW_FILTER;
    mpVideoDisplay->handle = (void*)pBase;
    pBase->AddRef();
#else
    mpVideoDisplay->type = SIPX_WINDOW_HANDLE_TYPE;
    mpVideoDisplay->handle = (void*)pWnd;
#endif
}

void* sipXmgr::getVideoWindow()
{
    return mpVideoDisplay->handle;
}

bool sipXmgr::setCodecPreferences(int codecPref)
{
    bool rc = false;

    if (sipxConfigSetAudioCodecPreferences(m_hInst, (SIPX_AUDIO_BANDWIDTH_ID)codecPref) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
    return rc;
}

bool sipXmgr::setVideoCodecPreferences(int codecPref)
{
    bool rc = false;
#ifdef VIDEO
    if (sipxConfigSetVideoBandwidth(m_hInst, (SIPX_VIDEO_BANDWIDTH_ID)codecPref) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
#endif    
    return rc;
}

bool sipXmgr::setVideoParameters(int iQuality, int iBitRate, int iFrameRate)
{
    bool rc = false;
#ifdef VIDEO
    if (sipxConfigSetVideoQuality(m_hInst, (SIPX_VIDEO_QUALITY_ID)iQuality) == SIPX_RESULT_SUCCESS)
    {
        if (sipxConfigSetVideoParameters(m_hInst, iBitRate, iFrameRate) == SIPX_RESULT_SUCCESS)
        {
            rc = true;
        }
    }
#endif
    return rc;
}

bool sipXmgr::isAECEnabled()
{
    bool rc = false;

    SIPX_AEC_MODE mode ;
    if ((sipxAudioGetAECMode(m_hInst, mode) == SIPX_RESULT_SUCCESS) && (mode != SIPX_AEC_DISABLED))
    {
        rc = true ;
    }

    return rc;
}

void sipXmgr::enableAEC(bool bEnable)
{
    sipxAudioSetAECMode(m_hInst, SIPX_AEC_CANCEL_AUTO) ;
}

bool sipXmgr::isOutOfBandDTMFEnabled()
{
    bool rc = false;

    sipxConfigIsOutOfBandDTMFEnabled(m_hInst, rc);

    return rc;
}

void sipXmgr::enableOutOfBandDTMF(bool bEnable)
{
    sipxConfigEnableOutOfBandDTMF(m_hInst, bEnable);
}

bool sipXmgr::isSRTPEnabled()
{
    bool rc = false;

    return rc;
}

void sipXmgr::enableSRTP(bool bEnable)
{
}

void sipXmgr::toggleMute()
{
    bool rc = false;

    sipxAudioIsMuted(m_hInst, rc);
    sipxAudioMute(m_hInst, !rc);
    return;
}


void sipXmgr::addToEventLog(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData)
{
    char cLogLine[1024] ;

    memset(cLogLine, 0, sizeof(cLogLine)) ;

    wxDateTime now = wxDateTime::Now() ;   

    sipxEventToString(category, pInfo, cLogLine, sizeof(cLogLine)-1) ;

    mEventLog[mEventLogIndex] = now.FormatISODate() ;
    mEventLog[mEventLogIndex].append(" ") ;
    mEventLog[mEventLogIndex].append(now.FormatISOTime()) ;
    mEventLog[mEventLogIndex].append("  ") ;
    mEventLog[mEventLogIndex].append(cLogLine) ;

    mEventLogIndex = (mEventLogIndex + 1) % MAX_EVENT_LOG_ENTRIES ;        
}


void sipXmgr::getEventLog(wxString& contents) const
{
    contents.Clear() ;

    int index = (abs(mEventLogIndex -1) % MAX_EVENT_LOG_ENTRIES) ;
    do
    {   
        index = (index + 1) % MAX_EVENT_LOG_ENTRIES ;
        if (mEventLog[index].Length())
        {
            contents.append(mEventLog[index]) ;
            contents.append("\r\n") ;
        }
    }
    while (index != abs((mEventLogIndex -1) % MAX_EVENT_LOG_ENTRIES)) ;
}


void sipXmgr::clearEventLog() 
{
    // Clearing log
    for (int i=0; i<MAX_EVENT_LOG_ENTRIES; i++)
    {
        mEventLog[i] = wxEmptyString ;
    }
    mEventLogIndex = 0 ;  
}

SIPX_INST sipXmgr::getSipxInstance()
{
    return m_hInst;
}

void sipXmgr::prepareToHibernate()
{
    SIPX_INST hInst = getSipxInstance();
    if (hInst)
    {
        sipxConfigPrepareToHibernate(hInst);
    }
}

void sipXmgr::unHibernate()
{
    SIPX_INST hInst = getSipxInstance();
    if (hInst)
    {
        sipxConfigUnHibernate(hInst);
    }
}