//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "stdwx.h"
#include "tapi/sipXtapi.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneApp.h"
#include "os/OsSocket.h"
#include "states/PhoneStateMachine.h"
#include "utl/UtlInt.h"
#include "utl/UtlString.h"

#include "tapi/sipXtapiInternal.h"


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
   mpPreviewDisplay(NULL)
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
    sipXezPhoneSettings::getInstance().loadSettings();

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
    if (SIPX_RESULT_SUCCESS != sipxInitialize(&m_hInst,
                                                iSipPort,
                                                iSipPort,
                                                DEFAULT_TLS_PORT,
                                                DEFAULT_RTP_START_PORT,
                                                DEFAULT_CONNECTIONS,
                                                sipXezPhoneSettings::getInstance().getIdentity(),
                                                NULL
                                                ))
    {
        return false;
    }

    sipxConfigSetUserAgentName(m_hInst, "sipXezPhone", false);

    if (UtlString(sipXezPhoneSettings::getInstance().getStunServer()).strip(UtlString::both, ' ').length() > 0)
    {    
        sipxConfigEnableStun(m_hInst,
                             sipXezPhoneSettings::getInstance().getStunServer().data(), 
                             DEFAULT_STUN_KEEPALIVE, SIPX_STUN_CHANGE_PORT); 
        //sleep(2000); // kludge alert - wait for the stun response
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
    OsTask::delay(2000);
    if (SIPX_RESULT_SUCCESS != sipxLineAdd(m_hInst, sipXezPhoneSettings::getInstance().getIdentity(), &m_hLine))
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

    OsTask::delay(2000);
    if (SIPX_RESULT_SUCCESS != sipxLineRegister(m_hLine, true))
    {
        return false;
    }
    
    sipXmgr::getInstance().setSpeakerVolume(sipXezPhoneSettings::getInstance().getSpeakerVolume());
    sipXmgr::getInstance().setMicGain(sipXezPhoneSettings::getInstance().getMicGain());

    sipXmgr::getInstance().enableAEC(sipXezPhoneSettings::getInstance().getEnableAEC());
    sipXmgr::getInstance().enableOutOfBandDTMF(sipXezPhoneSettings::getInstance().getEnableOOBDTMF());
    sipXmgr::getInstance().setCodecPreferences(sipXezPhoneSettings::getInstance().getCodecPref());

#ifdef TEST_GSM
    sipxConfigSetAudioCodecByName(m_hInst, "GSM");
#endif

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
        for (int i=0; i<10; i++)
        {
            if (sipxUnInitialize(sipXmgr::spSipXmgr->m_hInst) != SIPX_RESULT_SUCCESS)
            {
                OsTask::delay(1000) ;
            }
            else
            {
                break ;
            }
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
    char szPayloadType[5];

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
            if (pCallInfo->cause == CALLSTATE_NEW_CALL_TRANSFERRED)
            {
                mTransferInProgress = true;
                m_hTransferInProgress = pCallInfo->hCall;
                m_hCallController = m_hCall;
                m_hCall = m_hTransferInProgress;
            }
            break;
        case CALLSTATE_CONNECTED:
            if (pCallInfo->cause == CALLSTATE_CONNECTED_ACTIVE)
            {
                /*  FOR TESTING THE sipxCallGetVoiceEnginePtr 
                class fred : public wxThread
                {
                    public:
                        fred(SIPX_CALL hCall) { mhCall = hCall; }
                        void* Entry()
                        {   
                            GipsVoiceEngineLib* pLib = sipxCallGetVoiceEnginePtr(mhCall);
                            return NULL;
                        }

                        void OnExit() { }
                    private:
                        SIPX_CALL mhCall;
                };
                fred* f = new fred(pCallInfo->hCall);
                f->Create();
                f->Run();                
                */
                PhoneStateMachine::getInstance().OnConnected();
            }
            else if (pCallInfo->cause == CALLSTATE_CONNECTED_ACTIVE_HELD)
            {
                thePhoneApp->addLogMessage("Remote HOLD\n");
            }
            else if (pCallInfo->cause == CALLSTATE_CONNECTED_INACTIVE)
            {
                PhoneStateMachine::getInstance().OnConnectedInactive();
            }
            break;
        case CALLSTATE_REMOTE_ALERTING:
            PhoneStateMachine::getInstance().OnRemoteAlerting();
            break;
        case CALLSTATE_DISCONNECTED:
            if (CALLSTATE_DISCONNECTED_BUSY == pCallInfo->cause)
            {
                PhoneStateMachine::getInstance().OnRemoteBusy();
            }
            else if (mTransferInProgress && m_hTransferInProgress && pCallInfo->hCall == m_hCallController)
            {
                m_hTransferInProgress = 0;
                mTransferInProgress = false;
                m_hCallController = 0;
            }
            else
            {
                PhoneStateMachine::getInstance().OnDisconnected(hCall);
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
        case CALLSTATE_AUDIO_EVENT:
            if (pCallInfo->cause == CALLSTATE_AUDIO_START)
            {
                sprintf(szPayloadType, "%d", pCallInfo->codecs.audioCodec.iPayloadType);
                thePhoneApp->addLogMessage("Audio codec: " + 
                                            UtlString(pCallInfo->codecs.audioCodec.cName) +
                                            ", Pl type: " +
                                            UtlString(szPayloadType) +
                                            "\n");
                if (pCallInfo->codecs.videoCodec.iPayloadType != -1)
                {
                    sprintf(szPayloadType, "%d", pCallInfo->codecs.videoCodec.iPayloadType);
                    thePhoneApp->addLogMessage("Video codec: " + 
                                                UtlString(pCallInfo->codecs.videoCodec.cName) +
                                                ", Pl type: " +
                                                UtlString(szPayloadType) +
                                                "\n");
                }
            }
            break;
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


void sipXmgr::disconnect()
{
    if (m_hConf != 0)
    {
        sipxConferenceDestroy(m_hConf);
        m_hConf = 0;
        mConfCallHandleMap.destroyAll();
    }
    if (m_hCall != 0)
    {
        sipxCallDestroy(m_hCall);
    }
}

// Place a call to szSipUrl as szFromIdentity
bool sipXmgr::placeCall(const char* szSipUrl, const char* szFromIdentity, const char* szUsername, const char* szPassword, const char *szRealm)
{
    bool bRC = false ;

    sipxCallCreate(m_hInst, m_hLine, &m_hCall) ;    
    
    SIPX_VIDEO_DISPLAY display;
    // TODO - clean up the memory leak introduced above   
    
    display.handle = sipXmgr::getInstance().getVideoWindow();
    display.type = SIPX_WINDOW_HANDLE_TYPE;
        
    /* for testing of the LOCAL address type
    size_t numAddresses = 0;
    SIPX_CONTACT_ADDRESS addresses[32];
    sipxConfigGetLocalContacts(m_hInst, addresses, 32, numAddresses);
    
    for (int i = 0; i < numAddresses; i++)
    {
        if (addresses[i].eContactType == LOCAL)
        {
            break;
        }
    }
    
    sipxCallConnect(m_hCall, szSipUrl, addresses[i].id, pDisplay) ;
    */

    sipxCallConnect(m_hCall, szSipUrl, 0, &display) ;
   
    return bRC ;
}

// release the singleton instance of this class
void sipXmgr::release()
{
   if (sipXmgr::spSipXmgr)
   {
     
      delete sipXmgr::spSipXmgr;
      sipXmgr::spSipXmgr = NULL;
      sipXezPhoneSettings::getInstance().saveSettings();
   }
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
       sipxAudioSetGain(m_hInst, myGain);
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
        sipxCallHold(m_hCall);
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
        pDisplay->type = SIPX_WINDOW_HANDLE_TYPE;
        
        if (SIPX_RESULT_SUCCESS == sipxConferenceAdd(m_hConf, getCurrentLine(), szParty, &hNewCall, 0,
                                                     pDisplay ))
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

    if (pCodecPref)
    {
        if (sipxConfigGetVideoCodecPreferences(m_hInst, (SIPX_VIDEO_BANDWIDTH_ID*)pCodecPref) == SIPX_RESULT_SUCCESS)
        {
            rc = true;
        }
    }
    return rc;
}

bool sipXmgr::getVideoCodecList(UtlString& codecList)
{
    bool rc = false;

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

    if (sipxConfigSetVideoCodecByName(m_hInst, name) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
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

void sipXmgr::setVideoWindow(void* pWnd)
{
    mpVideoDisplay->handle = pWnd;
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

    if (sipxConfigSetVideoCodecPreferences(m_hInst, (SIPX_VIDEO_BANDWIDTH_ID)codecPref) == SIPX_RESULT_SUCCESS)
    {
        rc = true;
    }
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

    sipxAudioIsAECEnabled(m_hInst, rc);

    return rc;
}

void sipXmgr::enableAEC(bool bEnable)
{
    sipxAudioEnableAEC(m_hInst, bEnable);
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

    int index = mEventLogIndex ;
    while (index != abs((mEventLogIndex -1)) % MAX_EVENT_LOG_ENTRIES)
    {
        if (mEventLog[index].Length())
        {
            contents.append(mEventLog[index]) ;
            contents.append("\r\n") ;
        }
        index = (index + 1) % MAX_EVENT_LOG_ENTRIES ;
    }
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
