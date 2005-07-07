// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

// SYSTEM INCLUDES
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#endif

// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "mp/MpMediaTask.h"
#include "mp/NetInTask.h"
#include "mp/MpCodec.h"
#include "mp/dmaTask.h"
#include "mp/MprFromMic.h"
#include "mp/MprToSpkr.h"
#include "rtcp/RTCManager.h"
#include "net/SipUserAgent.h"
#include "net/SdpCodecFactory.h"
#include "cp/CallManager.h"
#include "ptapi/PtProvider.h"
#include "net/Url.h"
#include "net/NameValueTokenizer.h"
#include "os/OsConfigDb.h"
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "os/OsLock.h"
#include "net/TapiMgr.h"

// DEFINES
#define MP_SAMPLE_RATE          8000    // Sample rate (don't change)
#define MP_SAMPLES_PER_FRAME    80      // Frames per second (don't change)

// GLOBAL VARIABLES
// EXTERNAL VARIABLES
extern SipXHandleMap gCallHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap gLineHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap gConfHandleMap ;   // sipXtapiInternal.cpp
extern UtlDList      gSessionList ;     // sipXtapiInternal.cpp

// EXTERNAL FUNCTIONS


// STRUCTURES

/* ============================ FUNCTIONS ================================= */

#if defined(_WIN32)

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#endif /* defined(_WIN32) */

static void initLogger()
{
    OsSysLog::initialize(0, // do not cache any log messages in memory
			"sipXtapi"); // name for messages from this program
}


static bool validConnection(const SIPX_CALL_DATA* pData)
{
    assert(pData != NULL) ;
    assert(pData->callId != NULL) ;
    assert(pData->pInst != NULL) ;
    assert(pData->pInst->pCallManager != NULL) ;
    assert(pData->pInst->pRefreshManager != NULL) ;
    assert(pData->pInst->pLineManager != NULL) ;
    assert(pData->lineURI != NULL) ;
    assert(pData->remoteAddress != NULL) ;
    
    return (pData && pData->callId && 
            pData->remoteAddress &&
            pData->lineURI  && 
            pData->pInst->pCallManager && 
            pData->pInst->pRefreshManager &&
            pData->pInst->pLineManager ) ;
}



void destroyCallData(SIPX_CALL_DATA* pData) 
{
    if (pData != NULL)
    {
        if (pData->callId != NULL)
        {
            delete pData->callId ;
        }

        if (pData->lineURI != NULL)
        {
            delete pData->lineURI ;
        }

        if (pData->remoteAddress != NULL)
        {
            delete pData->remoteAddress ;
        }

        if (pData->pMutex != NULL)
        {
            delete pData->pMutex ;
        }

        delete pData ;
    }
}

static void destroyLineData(SIPX_LINE_DATA* pData) 
{
    if (pData != NULL)
    {
        if (pData->lineURI != NULL)
        {
            delete pData->pMutex ;
            delete pData->lineURI ;            
        }

        delete pData ;
    }
}

static SIPX_LINE_DATA* createLineData(SIPX_INSTANCE_DATA* pInst, const Url& uri)
{
    SIPX_LINE_DATA* pData = new SIPX_LINE_DATA ;
	memset ((void*) pData, 0, sizeof(SIPX_LINE_DATA));
    if (pData != NULL)
    {
        pData->lineURI = new Url(uri) ;
        pData->pInst = pInst ;
        pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;
        if ((pData->lineURI == NULL) || (pData->pMutex == NULL))
        {
            destroyLineData(pData) ;
            pData = NULL ;
        }
    }

    return pData ;
}



/****************************************************************************
 * Initialization
 ***************************************************************************/

#ifdef _WIN32

static void initAudioDevices(SIPX_INSTANCE_DATA* pInst) 
{
    WAVEOUTCAPS outcaps ;
    WAVEINCAPS  incaps ;
    int numDevices ;
    int i ;

    numDevices = waveInGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS)) ;
        pInst->inputAudioDevices[i] = strdup(incaps.szPname) ;
    }

    numDevices = waveOutGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS)) ;
        pInst->outputAudioDevices[i] = strdup(outcaps.szPname) ;
    }
}

#endif /* _WIN32 */



SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST* phInst,
                                        const int tcpPort, 
                                        const int udpPort,
                                        const int tlsPort,
                                        const int rtpPortStart,
                                        const int maxConnections,
                                        const char* szIdentity,
                                        const char* szBindToAddr)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    // Disable console output
    enableConsoleOutput(false) ;

    // set the sipXtackLib's tapi callback function pointers
    TapiMgr::getInstance().setTapiCallCallback(&sipxFireEvent);
    TapiMgr::getInstance().setTapiLineCallback(&sipxFireLineEvent);
    
    // Setup bindTo addr
    unsigned long bindTo = inet_addr(szBindToAddr) ;
    if (bindTo != INADDR_NONE)
    {
        OsSocket::setDefaultBindAddress(bindTo) ;

        SIPX_INSTANCE_DATA* pInst = new SIPX_INSTANCE_DATA; 
        memset(pInst, 0, sizeof(SIPX_INSTANCE_DATA)) ;

        // Create Line and Refersh Manager
        pInst->pLineManager = new SipLineMgr() ;
        pInst->pRefreshManager = new SipRefreshMgr() ;

        // Bind the SIP user agent to a port and start it up
        pInst->pSipUserAgent = new SipUserAgent(tcpPort, udpPort, tlsPort,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0,
                "PING",                
                pInst->pLineManager);

        pInst->pSipUserAgent->start();            

        // Startup Line Manager  Refresh Manager
        pInst->pLineManager->initializeRefreshMgr(pInst->pRefreshManager) ;
        pInst->pRefreshManager->init(pInst->pSipUserAgent, pInst->pSipUserAgent->getTcpPort(), pInst->pSipUserAgent->getUdpPort()) ;
        pInst->pRefreshManager->StartRefreshMgr();

        // Enable PCMU, PCMA, Tones/RFC2833 codecs
        pInst->pCodecFactory = new SdpCodecFactory() ;
        SdpCodec::SdpCodecTypes codecs[3];
    
        codecs[0] = SdpCodecFactory::getCodecType(CODEC_G711_PCMU) ;
        codecs[1] = SdpCodecFactory::getCodecType(CODEC_G711_PCMA) ;
        codecs[2] = SdpCodecFactory::getCodecType(CODEC_DTMF_RFC2833) ;

        pInst->pCodecFactory->buildSdpCodecFactory(3, codecs);

        // Initialize and start up the media subsystem
        OsConfigDb dummyConfigDb;
        mpStartUp(MP_SAMPLE_RATE, MP_SAMPLES_PER_FRAME, 6 * maxConnections, &dummyConfigDb);
        MpMediaTask::getMediaTask(maxConnections);
        CRTCManager::getRTCPControl();
        mpStartTasks();
        
        // Instantiate the call processing subsystem
        UtlString localAddress;
        UtlString utlIdentity(szIdentity);
        if (!utlIdentity.contains("@"))
        {
           OsSocket::getHostIp(&localAddress);
           char *szBuf = (char*) calloc(64 + strlen(szIdentity), 1) ;    
           sprintf(szBuf, "sip:%s@%s:%d", szIdentity, localAddress.data(), pInst->pSipUserAgent->getUdpPort()) ;
           localAddress = szBuf ;
           free(szBuf) ;
        }
        else
        {
           localAddress = utlIdentity;
        }

    
        pInst->pCallManager = new CallManager(FALSE,
                               pInst->pLineManager,
                               TRUE, // early media in 180 ringing
                               pInst->pCodecFactory,
                               rtpPortStart, // rtp start
                               rtpPortStart + (2*maxConnections), // rtp end
                               localAddress.data(),
                               localAddress.data(),
                               pInst->pSipUserAgent, 
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000); // offeringDelay

        // Start up the call processing system
        pInst->pCallManager->setOutboundLine(localAddress) ;
        pInst->pCallManager->start();

#ifdef _WIN32
        initAudioDevices(pInst) ;
#else
        // TBD
        for (int i=0; i<MAX_AUDIO_DEVICES; i++)
        {
            pInst->inputAudioDevices[i] = NULL ;
            pInst->outputAudioDevices[i] = NULL ;
        }
#endif
        *phInst = pInst ;
        gSessionList.insert(new UtlVoidPtr(pInst)) ;

    #ifdef _WIN32
        Sleep(500) ;    // Need to wait for UA and MP to startup
                        // TODO: Need to synchronize startup
    #endif 
        rc = SIPX_RESULT_SUCCESS ;
    }
    
    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst)
{
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
    /*
    if (pInst)
    {   
        mpStopTasks();
        mpShutdown();
        
        pInst->pCodecFactory->clearCodecs();
        pInst->pLineManager->requestShutdown();
        pInst->pCallManager->requestShutdown();
        pInst->pRefreshManager->requestShutdown(); 
        pInst->pSipUserAgent->requestShutdown(); 
       

        delete pInst->pCallManager;
        pInst->pCallManager = NULL;
        
        delete pInst;
        pInst = NULL;

    }    
    */
    return SIPX_RESULT_SUCCESS;
}                          
/****************************************************************************
 * Call Related Functions
 ***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL   hCall,
                                        SIPX_CONTACT_TYPE contactType) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;            
            pInst->pCallManager->acceptConnection(callId.data(), 
                    remoteAddress.data(), (CONTACT_TYPE) contactType) ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            pInst->pCallManager->rejectConnection(callId.data(), remoteAddress.data()) ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall, const char* szForwardURL) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length() && szForwardURL)
        {
            pInst->pCallManager->redirectConnection(callId.data(), remoteAddress.data(), szForwardURL) ;

        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;            
            pInst->pCallManager->answerTerminalConnection(callId.data(), remoteAddress.data(), "unused") ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


static SIPX_RESULT sipxCallCreateHelper(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        SIPX_CALL*  phCall)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pLine = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(phCall != NULL) ;
    assert(pLine != NULL) ;
    assert(pLine->lineURI != NULL) ;    

    if (pInst)
    {
        if (phCall && pLine)
        {        
            SIPX_CALL_DATA* pData = new SIPX_CALL_DATA ;
            memset((void*)pData, 0, sizeof(SIPX_CALL_DATA));            
			   
            if (pData)
            {
                UtlString callId ;
                                
                pInst->pCallManager->createCall(&callId) ;
                
                // Set Call ID
                pData->callId = new UtlString(callId) ;    
                assert(pData->callId != NULL) ;

                // Set Line URI
                pData->hLine = hLine ;
                pData->lineURI = new UtlString(pLine->lineURI->toString().data()) ;
                assert(pData->lineURI != NULL) ;

                // Remote Address
                pData->remoteAddress = NULL ;

                // Store Instance
                pData->pInst = pInst ;

                // Create Mutex
                pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;

                if ((pData->callId == NULL) || (pData->lineURI == NULL))
                {
                    *phCall = NULL ;
                    destroyCallData(pData) ;
                    sr = SIPX_RESULT_OUT_OF_MEMORY ;
                }
                else
                {
                    *phCall = gCallHandleMap.allocHandle(pData) ;
                    assert(*phCall != 0) ;

                    sr = SIPX_RESULT_SUCCESS ;
                }
            } 
            else 
            {
                sr = SIPX_RESULT_OUT_OF_MEMORY ;
                destroyCallData(pData) ;
                *phCall = NULL ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    sipxLineReleaseLock(pLine, SIPX_LOCK_READ) ;

    return sr ;
}
                                        


SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        SIPX_CALL*  phCall)
{    
    SIPX_RESULT rc = sipxCallCreateHelper(hInst, hLine, phCall) ;
    if (rc == SIPX_RESULT_SUCCESS)
    {
        SIPX_CALL_DATA* pData = sipxCallLookup(*phCall, SIPX_LOCK_READ) ;
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
        UtlString callId = *pData->callId ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
       
        // Notify Listeners
        SipSession session ;
        sipxFireEvent(pInst->pCallManager, callId.data(), &session, NULL, DIALTONE, DIALTONE_UNKNOWN) ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetLocalContacts(const SIPX_CALL hCall,
                                                  SIPX_CONTACT_ADDRESS addresses[],
                                                  size_t nMaxAddresses,
                                                  size_t& nActualAddress)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
                      
    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL) && 
            (nMaxAddresses > 0))
    {
        CONTACT_ADDRESS* pAddress = new CONTACT_ADDRESS[nMaxAddresses] ;
        if (pAddress)
        {
            memset(pAddress, 0, sizeof(CONTACT_ADDRESS) * nMaxAddresses) ;

            if (pInst->pCallManager->getLocalContactAddresses(callId.data(),
                    pAddress, nMaxAddresses, nActualAddress) == OS_SUCCESS)
            {
                for (size_t i=0; i<nActualAddress; i++)
                {
                    addresses[i].eContactType = 
                            (SIPX_CONTACT_TYPE) pAddress[i].eContactType ;
                    addresses[i].iPort = pAddress[i].iPort ;
                    strcpy((char*) addresses[i].cInterface, (char*) pAddress[i].cInterface) ;
                    strcpy((char*) addresses[i].cIpAddress, (char*) pAddress[i].cIpAddress) ;
                }
        
                sr = SIPX_RESULT_SUCCESS ;
            }
            delete pAddress ;
        }
    }
    
    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallConnect(SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_CONTACT_TYPE contactType)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    UtlString lineId ;
                       
    assert(szAddress != NULL) ;    

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        if (contactType == CONTACT_AUTO)
        {
            contactType = sipxCallGetLineContactType(hCall) ;
        }

        if (szAddress)
        {    
            PtStatus status ;
            assert(remoteAddress.length() == 0) ;    // should be null

            pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;            
            pInst->pCallManager->setOutboundLineForCall(callId.data(), 
                    lineId.data()) ;

            status = pInst->pCallManager->connect(callId.data(), szAddress, NULL, NULL, (CONTACT_TYPE) contactType) ;
            if (status == PT_SUCCESS)
            {
                int numAddresses = 0 ;
                UtlString address ; 
                OsStatus rc = pInst->pCallManager->getCalledAddresses(
                        callId.data(), 1, numAddresses, &address) ;
                assert(rc == OS_SUCCESS) ;
                assert(numAddresses == 1) ;

                // Set Remote Connection
                SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE);
                if (pData)
                {
                    pData->remoteAddress = new UtlString(address) ;
                    assert(pData->remoteAddress != NULL) ;
                    if (pData->remoteAddress == NULL) 
                    {                    
                        sr = SIPX_RESULT_OUT_OF_MEMORY ;
                    }
                    else
                    {
                        sr = SIPX_RESULT_SUCCESS ;
                    }
                    sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
                }
            }
            else
            {
                // TODO: Should this be fired outside the lock?
                SipSession session ;
                sipxFireEvent(pInst->pCallManager, callId.data(), &session, szAddress, DISCONNECTED, DISCONNECTED_BADADDRESS) ;   
                sr = SIPX_RESULT_BAD_ADDRESS ;
            }
        } 
        else 
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    return sr ;

}


SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->holdAllTerminalConnections(callId.data()) ;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->unholdAllTerminalConnections(callId.data()) ;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL& hCall) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->drop(callId.data()) ;        
        sr = SIPX_RESULT_SUCCESS ;
    }
    else
    {
        // Not finding the call is ok (torn down) providing
        // that the handle is valid.
        if (hCall != NULL)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }

    hCall = NULL ;
    
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetID(const SIPX_CALL hCall,
                                       char* szId, 
                                       const size_t iMaxLength) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, NULL, &callId, NULL, NULL))
    {
        if (iMaxLength)
        {
            strncpy(szId, callId.data(), iMaxLength) ;
            szId[iMaxLength-1] = 0 ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;    
}

SIPXTAPI_API SIPX_RESULT sipxCallGetLocalID(const SIPX_CALL hCall, 
                                            char* szId, 
                                            const size_t iMaxLength)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    UtlString callId ;
    UtlString lineId ;

    if (sipxCallGetCommonData(hCall, NULL, &callId, NULL, &lineId))
    {
        if (iMaxLength)
        {
            strncpy(szId, lineId.data(), iMaxLength) ;
            szId[iMaxLength-1] = 0 ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;    
}


SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteID(const SIPX_CALL hCall, 
                                             char* szId, 
                                             const size_t iMaxLength)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    UtlString callId ;
    UtlString remoteId ;

    if (sipxCallGetCommonData(hCall, NULL, &callId, &remoteId, NULL))
    {
        if (iMaxLength)
        {
            strncpy(szId, remoteId.data(), iMaxLength) ;
            szId[iMaxLength-1] = 0 ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;        
}


SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall, 
                                           const TONE_ID toneId,
                                           const bool bLocal,                                        
                                           const bool bRemote)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->toneStart(callId.data(), toneId, bLocal, bRemote) ;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->toneStop(callId.data()) ;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallPlayFile(const SIPX_CALL hCall,
                                          const char* szFile,
                                          const bool bLocal,
                                          const bool bRemote)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        if (szFile)
        {
            pInst->pCallManager->audioPlay(callId.data(), szFile, false, bLocal, bRemote) ;
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;

        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall, 
                                               const char* pszAddress) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        if (pszAddress)
        {
            UtlString ghostCallId ;
            pInst->pCallManager->transfer(callId.data(), pszAddress, &ghostCallId) ;
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    return sr ;
}



/****************************************************************************
 * Conference Related Functions
 ***************************************************************************/


SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF *phConference)
{
    SIPX_RESULT rc = SIPX_RESULT_OUT_OF_MEMORY ;

    assert(phConference) ;

    if (phConference)
    {
        *phConference = NULL ;

        SIPX_CONF_DATA* pData = new SIPX_CONF_DATA ;
        assert(pData != NULL) ;
        if (pData)
        {
            memset(pData, 0, sizeof(SIPX_CONF_DATA)) ;
            pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;
            pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;
            *phConference = gConfHandleMap.allocHandle(pData) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    if (hConf && hCall)
    {        
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE) ;
        if (pData)  
        {
            if (pData->nCalls == 0)
            {
                UtlString callId ;
                if (sipxCallGetCommonData(hCall, NULL, &callId, NULL, NULL))
                {
                    pData->strCallId = new UtlString(callId) ;
                    pData->hCalls[pData->nCalls++] = hCall ;

                    rc = SIPX_RESULT_SUCCESS ;
                }                
            }
            else
            {
                rc = SIPX_RESULT_OUT_OF_RESOURCES ;
            }

            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConference,
                                            const SIPX_CALL hCall)
{
    return SIPX_RESULT_NOT_IMPLEMENTED ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_CONTACT_TYPE contactType)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;    

    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE) ;
        if (pData)
        {
            if (pData->nCalls == 0) 
            {
                /*
                 * No existings legs, create one and connect
                 */

                // Create new call
                SIPX_CALL hNewCall ;                
                rc = sipxCallCreateHelper(pData->pInst, hLine, &hNewCall) ;
                if (rc == SIPX_RESULT_SUCCESS)
                {
                    // Get data struct for call and store callId as conf Id
                    assert(hNewCall != NULL) ; 
                    SIPX_CALL_DATA* pCallData = sipxCallLookup(hNewCall, SIPX_LOCK_WRITE) ;
                    pData->strCallId = new UtlString(pCallData->callId->data()) ;
                    
                    // Add the call handle to the conference handle
                    pData->hCalls[pData->nCalls++] = hNewCall ;
                    *phNewCall = hNewCall ;

                    // Allocate a new session id (to replace call id for connection)
                    UtlString sessionId ;
                    pData->pInst->pCallManager->getNewSessionId(&sessionId) ;
                    *pCallData->callId = sessionId.data() ;
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE) ;              
                                                            
                    // Notify Listeners of new call
                    SipSession session ;
                    UtlString callId ;
                    SIPX_INSTANCE_DATA* pInst ;
                    sipxCallGetCommonData(hNewCall, &pInst, &callId, NULL, NULL) ;

                    sipxFireEvent(pInst->pCallManager, sessionId.data(), &session, NULL, DIALTONE, DIALTONE_CONFERENCE) ;   

                    // Issue connect
                    PtStatus status = pData->pInst->pCallManager->connect(pData->strCallId->data(), 
                            szAddress, NULL, sessionId.data(), (CONTACT_TYPE) contactType) ;
                    if (status == PT_SUCCESS)
                    {
                        rc = SIPX_RESULT_SUCCESS ;
                    }
                    else
                    {
                        sipxFireEvent(pData->pInst->pCallManager, sessionId.data(), &session, szAddress, DISCONNECTED, DISCONNECTED_BADADDRESS) ;
                        rc = SIPX_RESULT_BAD_ADDRESS ;                        
                    }                    
                }               
            }
            else if (pData->nCalls < CONF_MAX_CONNECTIONS)
            {               
                /*
                 * Use existing call id to find call
                 */
                SIPX_INSTANCE_DATA* pInst ;
                UtlString callId ;
                UtlString lineId ;

                if (sipxCallGetCommonData(pData->hCalls[0], &pInst, &callId, NULL, &lineId))
                {
                    SIPX_CALL_DATA* pNewCallData = new SIPX_CALL_DATA ;
                    memset((void*) pNewCallData, 0, sizeof(SIPX_CALL_DATA));

                    UtlString sessionId ;
                    pData->pInst->pCallManager->getNewSessionId(&sessionId) ;

                    pNewCallData->pInst = pInst ;
                    pNewCallData->callId = new UtlString(sessionId) ;
                    pNewCallData->remoteAddress = NULL ;
                    pNewCallData->hLine = hLine ;
                    pNewCallData->lineURI = new UtlString(lineId.data()) ;
                    pNewCallData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;

                    SIPX_CALL hNewCall = gCallHandleMap.allocHandle(pNewCallData) ;
                    pData->hCalls[pData->nCalls++] = hNewCall ;
                    *phNewCall = hNewCall ;

                    // Notify Listeners
                    SipSession session ;
                    sipxFireEvent(pData->pInst->pCallManager, sessionId.data(), &session, NULL, DIALTONE, DIALTONE_CONFERENCE) ;   

                    PtStatus status = pData->pInst->pCallManager->connect(pData->strCallId->data(), szAddress, NULL, sessionId.data()) ;
                    if (status == PT_SUCCESS)
                    {
                        rc = SIPX_RESULT_SUCCESS ;
                    }
                    else
                    {
                        sipxFireEvent(pData->pInst->pCallManager, sessionId.data(), &session, szAddress, DISCONNECTED, DISCONNECTED_BADADDRESS) ;
                        rc = SIPX_RESULT_BAD_ADDRESS ;                        
                    }                                        
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
            }
            else
            {
                /*
                 * Hit max connections
                 */
                rc = SIPX_RESULT_OUT_OF_RESOURCES ;
            }

            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
        else
        {
            rc = SIPX_RESULT_FAILURE ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceRemove(const SIPX_CONF hConf,
                                              const SIPX_CALL hCall) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ; 

    if (hConf && hCall)
    {
        SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE) ;
        UtlString callId ;
        UtlString remoteAddress ;
        SIPX_INSTANCE_DATA* pInst ;

        if (pConfData && sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
        {
            sipxRemoveCallHandleFromConf(hConf, hCall) ;
            pInst->pCallManager->disconnectConnection(callId.data(), remoteAddress.data()) ;

            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            // Either the call or conf doesn't exist
            rc = SIPX_RESULT_FAILURE ;
        }

        sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE) ;
    } 
           
    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL hCalls[], 
                                                const size_t iMax, 
                                                size_t& nActual)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    if (hConf && iMax)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ) ;
        if (pData)
        {
            OsReadLock(*pData->pMutex) ;

            size_t idx ;
            for (idx=0; (idx<pData->nCalls) && (idx < iMax); idx++)
            {
                hCalls[idx] = pData->hCalls[idx] ;
            }
            nActual = idx ;

            rc = SIPX_RESULT_SUCCESS ;

            sipxConfReleaseLock(pData, SIPX_LOCK_READ) ;
        }
        else
        {
            rc = SIPX_RESULT_FAILURE ;
        }
    }
    

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf) 
{
    SIPX_CALL hCalls[CONF_MAX_CONNECTIONS] ;
    size_t nCalls ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    if (hConf)
    {
        // Get a snapshot of the calls, drop the connections, remove the conf handle,
        // and THEN whack the call -- otherwise whacking the calls will force updates
        // into SIPX_CONF_DATA structure (work that isn't needed).
        sipxConferenceGetCalls(hConf, hCalls, CONF_MAX_CONNECTIONS, nCalls) ;
        for (size_t idx=0; idx<nCalls; idx++)
        {
            sipxConferenceRemove(hConf, hCalls[idx]) ;
        }

        sipxConfFree(hConf) ;

        rc = SIPX_RESULT_SUCCESS ;            
    }

    return rc ;
}


/****************************************************************************
 * Audio Related Functions
 ***************************************************************************/


static void initMicSettings(MIC_SETTING* pMicSetting)
{
    pMicSetting->bInitialized = TRUE ;
    pMicSetting->bMuted = FALSE ;
    pMicSetting->iGain = GAIN_DEFAULT ;
}

static void initSpeakerSettings(SPEAKER_SETTING* pSpeakerSetting)
{
    pSpeakerSetting->bInitialized = TRUE ;
    pSpeakerSetting->iVol = VOLUME_DEFAULT ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetGain(const SIPX_INST hInst,
                                          const int iLevel) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        // Validate gain is within range
        assert(iLevel >= GAIN_MIN) ;
        assert(iLevel <= GAIN_MAX) ;
        if ((iLevel >= GAIN_MIN) && (iLevel <= GAIN_MAX))
        {
            // Only process if uninitialized (first call) or the state has changed
            if (!pInst->micSetting.bInitialized || (iLevel != pInst->micSetting.iGain))
            {
                OsStatus rc = OS_SUCCESS ;

                // Lazy Init
                if (!pInst->micSetting.bInitialized)
                {
                    initMicSettings(&pInst->micSetting) ;
                    assert(pInst->micSetting.bInitialized) ;
                }

                // Record Gain
                pInst->micSetting.iGain = iLevel ;
                
                // Set Gain if not muted
                if (!pInst->micSetting.bMuted)
                {
                    rc = MpCodec_setGain(pInst->micSetting.iGain) ;
                }
                assert(rc == OS_SUCCESS) ;                
                assert(pInst->micSetting.bMuted ? (MpCodec_getGain() == 0) :
                        (MpCodec_getGain() == pInst->micSetting.iGain)) ;
                if (rc == OS_SUCCESS)
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }                
            } 
            else if (iLevel == pInst->micSetting.iGain)
            {
                sr = SIPX_RESULT_SUCCESS ;
            }            
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetGain(const SIPX_INST hInst,
                                          int& iLevel) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        // Lazy init
        if (!pInst->micSetting.bInitialized)
        {
            initMicSettings(&pInst->micSetting) ;
            assert(pInst->micSetting.bInitialized) ;
        }

        iLevel = pInst->micSetting.iGain ;
        assert(pInst->micSetting.bMuted ? (MpCodec_getGain() == 0) :
                (MpCodec_getGain() == pInst->micSetting.iGain)) ;

        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioMute(const SIPX_INST hInst,
                                       const bool bMute) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        // Only process if uninitialized (first call) or the state has changed
        if (!pInst->micSetting.bInitialized || bMute != pInst->micSetting.bMuted)
        {
            // Lazy Init
            if (!pInst->micSetting.bInitialized)
            {
                initMicSettings(&pInst->micSetting) ;
                assert(pInst->micSetting.bInitialized) ;
            }

            // Store setting
            pInst->micSetting.bMuted = bMute ;
            if (bMute)
            {
                // Mute gain
                OsStatus rc = MpCodec_setGain(0) ;
                assert(rc == OS_SUCCESS);
                assert(MpCodec_getGain() == 0) ;
                if (rc == OS_SUCCESS)
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }
            }
            else
            {
                // Restore gain
                OsStatus rc = MpCodec_setGain(pInst->micSetting.iGain);
                assert(MpCodec_getGain() == pInst->micSetting.iGain);
                assert(rc == OS_SUCCESS);
                if (rc == OS_SUCCESS)
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }
            }
        }
        else if (bMute == pInst->micSetting.bMuted)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }        
    }

    return sr ;        
}


SIPXTAPI_API SIPX_RESULT sipxAudioIsMuted(const SIPX_INST hInst,
                                          bool &bMuted) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        if (!pInst->micSetting.bInitialized)
        {
            initMicSettings(&pInst->micSetting) ;
            assert(pInst->micSetting.bInitialized) ;
        }

        bMuted = pInst->micSetting.bMuted ;        
        assert(bMuted ? (MpCodec_getGain() == 0) : (MpCodec_getGain() > 0)) ;        
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;    
}



SIPXTAPI_API SIPX_RESULT sipxAudioEnableSpeaker(const SIPX_INST hInst,
                                                const SPEAKER_TYPE type) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    OsStatus status ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        if (!pInst->speakerSettings[type].bInitialized || (pInst->enabledSpeaker != type))
        {
            pInst->enabledSpeaker = type ;

            // Lazy Init
            if (!pInst->speakerSettings[type].bInitialized)
            {
                initSpeakerSettings(&pInst->speakerSettings[type]) ;
                assert(pInst->speakerSettings[type].bInitialized) ;
            }

            // Lower Volume
            status = MpCodec_setVolume(0) ;
            assert(status == OS_SUCCESS) ;
                        
            if (status == OS_SUCCESS)
            {
                // Enable Speaker
                switch (type)
                {
                    case SPEAKER:
                        DmaTask::setRingerEnabled(false) ;    
                        assert(DmaTask::isRingerEnabled() == false) ;
                        break ;
                    case RINGER:
                        DmaTask::setRingerEnabled(true) ;
                        assert(DmaTask::isRingerEnabled() == true) ;
                        break ;
                    default:
                        assert(FALSE) ;
                        break ;
                }
            }

            if (status == OS_SUCCESS)
            {
                // Reset Volume
                status = MpCodec_setVolume(pInst->speakerSettings[type].iVol) ;
                assert(status == OS_SUCCESS) ;
                assert(MpCodec_getVolume() == pInst->speakerSettings[type].iVol) ;
                if (status == OS_SUCCESS)
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }
            }
        }
        else if (pInst->enabledSpeaker == type)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetEnabledSpeaker(const SIPX_INST hInst,
                                                    SPEAKER_TYPE& type)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        type = pInst->enabledSpeaker ;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type, 
                                            const int iLevel) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(type == SPEAKER || type == RINGER) ;
    assert(iLevel >= VOLUME_MIN) ;
    assert(iLevel <= VOLUME_MAX) ;


    if (pInst)
    {
        // Validate Params
        if ((type == SPEAKER || type == RINGER) && (iLevel >= VOLUME_MIN) &&
                (iLevel <= VOLUME_MAX))
        {
            // Only process if uninitialized (first call) or the state has changed
            if (!pInst->speakerSettings[type].bInitialized || 
                    pInst->speakerSettings[type].iVol != iLevel)
            {
                // Lazy Init
                if (!pInst->speakerSettings[type].bInitialized)
                {
                    initSpeakerSettings(&pInst->speakerSettings[type]) ;
                    assert(pInst->speakerSettings[type].bInitialized) ;
                }

                // Store value
                pInst->speakerSettings[type].iVol = iLevel ;
                sr = SIPX_RESULT_SUCCESS ;

                // Set value if this type is enabled
                if (pInst->enabledSpeaker == type)
                {
                    OsStatus rc = MpCodec_setVolume(iLevel) ;
                    assert(rc == OS_SUCCESS) ;
                    assert(MpCodec_getVolume() == iLevel) ;
                    if (rc != OS_SUCCESS)
                    {
                        sr = SIPX_RESULT_FAILURE ;
                    }
                }
            }
            else if (pInst->speakerSettings[type].iVol == iLevel)
            {
                sr = SIPX_RESULT_SUCCESS ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type, 
                                            int& iLevel) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(type == SPEAKER || type == RINGER) ;

    if (pInst)
    {
        // Validate Params
        if (type == SPEAKER || type == RINGER)
        {
            // Lazy Init
            if (!pInst->speakerSettings[type].bInitialized)
            {
                initSpeakerSettings(&pInst->speakerSettings[type]) ;
                assert(pInst->speakerSettings[type].bInitialized) ;
            }

            iLevel = pInst->speakerSettings[type].iVol ;
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }
    return sr ;    
}    


SIPXTAPI_API SIPX_RESULT sipxAudioEnableAEC(const SIPX_INST hInst,
                                            const bool enable) 
{
    return SIPX_RESULT_NOT_IMPLEMENTED ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioIsAECEnabled(const SIPX_INST hInst,
                                               bool& enabled) 
{
    return SIPX_RESULT_NOT_IMPLEMENTED ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     size_t& numDevices) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        numDevices = 0 ;

        while ( (numDevices < MAX_AUDIO_DEVICES) && 
                (pInst->inputAudioDevices[numDevices] != NULL) )
        {
            numDevices++ ;
        }

        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDevice(const SIPX_INST hInst,
                                                 const int index,
                                                 const char*& szDevice) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if ((pInst) && (index >= 0) && (index < MAX_AUDIO_DEVICES))
    {
        szDevice = pInst->inputAudioDevices[index] ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNumOutputDevices(const SIPX_INST hInst,
                                                      size_t& numDevices) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        numDevices = 0 ;

        while ( (numDevices < MAX_AUDIO_DEVICES) && 
                (pInst->outputAudioDevices[numDevices] != NULL) )
        {
            numDevices++ ;
        }

        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDevice(const SIPX_INST hInst,
                                                  const int index,
                                                  const char*& szDevice) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if ((pInst) && (index >= 0) && (index < MAX_AUDIO_DEVICES))
    {
        szDevice = pInst->outputAudioDevices[index] ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetCallInputDevice(const SIPX_INST hInst,
                                                     const char* szDevice) 
{
    UtlString oldDevice = DmaTask::getMicDevice() ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
#ifdef _WIN32
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (stricmp(szDevice, "NONE") == 0)
    {
        DmaTask::setInputDevice("NONE") ;
        rc = SIPX_RESULT_SUCCESS ;
    }
    else
    {
        for (int i=0; i<MAX_AUDIO_DEVICES; i++)
        {
            if (pInst->inputAudioDevices[i])
            {
                if (strcmp(szDevice, pInst->inputAudioDevices[i]) == 0)
                {
                    // Match
                    if (strcmp(szDevice, oldDevice) != 0)
                    {
                        DmaTask::setInputDevice(szDevice) ;
                    }
                    rc = SIPX_RESULT_SUCCESS ;
                    break ;
                }
            }
            else
            {
                // Hit end of list, might was well jump
                break ;
            }
        }
    }
#endif
    return rc ;    
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetRingerOutputDevice(const SIPX_INST hInst,
                                                        const char* szDevice) 
{
    UtlString oldDevice = DmaTask::getRingDevice() ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
#ifdef _WIN32
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (stricmp(szDevice, "NONE") == 0)
    {
        DmaTask::setRingDevice("NONE") ;
        rc = SIPX_RESULT_SUCCESS ;
    }
    else
    {
        for (int i=0; i<MAX_AUDIO_DEVICES; i++)
        {
            if (pInst->outputAudioDevices[i])
            {
                if (strcmp(szDevice, pInst->outputAudioDevices[i]) == 0)
                {
                    // Match
                    if (strcmp(szDevice, oldDevice) != 0)
                    {
                        DmaTask::setRingDevice(szDevice) ;
                    }
                    rc = SIPX_RESULT_SUCCESS ;
                    break ;
                }
            }
            else
            {
                // Hit end of list, might was well jump
                break ;
            }
        }
    }
#endif
    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetCallOutputDevice(const SIPX_INST hInst,
                                                      const char* szDevice) 
{
    UtlString oldDevice = DmaTask::getCallDevice() ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
#ifdef _WIN32
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (stricmp(szDevice, "NONE") == 0)
    {
        DmaTask::setCallDevice("NONE") ;
        rc = SIPX_RESULT_SUCCESS ;
    }
    else
    {
        for (int i=0; i<MAX_AUDIO_DEVICES; i++)
        {
            if (pInst->outputAudioDevices[i])
            {
                if (strcmp(szDevice, pInst->outputAudioDevices[i]) == 0)
                {
                    // Match
                    if (strcmp(szDevice, oldDevice) != 0)
                    {
                        DmaTask::setCallDevice(szDevice) ;
                    }
                    rc = SIPX_RESULT_SUCCESS ;
                    break ;
                }
            }
            else
            {
                // Hit end of list, might was well jump
                break ;
            }
        }
    }
#endif
    return rc ;
}


/****************************************************************************
 * Line Related Functions
 ***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineUrl,
                                     const bool bRegister,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_TYPE contactType) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(szLineUrl != NULL) ;
    assert(phLine != NULL) ;

    if (pInst)
    {
        if (szLineUrl && phLine)
        {
            Url url(szLineUrl) ;
            UtlString strURI;
            url.getUri(strURI) ;
            Url uri(strURI) ;
            UtlString userId ;
            url.getUserId(userId) ;

            SipLine line(url, uri, userId) ;

            // Set the preferred contact
            Url uriPreferredContact ;
            sipxGetContactHostPort(pInst, contactType, uriPreferredContact) ;
            line.setPreferredContactUri(uriPreferredContact) ;

            UtlBoolean bRC = pInst->pLineManager->addLine(line, false) ;
            if (bRC)
            {
                SIPX_LINE_DATA* pData = createLineData(pInst, uri) ;
                if (pData != NULL)
                {
                    pData->contactType = contactType ;
                    *phLine = gLineHandleMap.allocHandle(pData) ;
                    sr = SIPX_RESULT_SUCCESS ;    
                    if (bRegister)
                    {
                        pInst->pLineManager->enableLine(*(pData->lineURI));
                    }
                    else
                    {
                        pInst->pLineManager->setStateForLine(uri, SipLine::LINE_STATE_PROVISIONED) ;
                        sipxFireLineEvent(pInst->pRefreshManager, szLineUrl, SIPX_LINE_EVENT_PROVISIONED); 
                    }
                }
                else
                {
                    sr = SIPX_RESULT_OUT_OF_MEMORY ;
                }
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;    
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;    

    if (hLine)
    {
        SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
        if (pData)
        {
            pData->pInst->pLineManager->deleteLine(*pData->lineURI) ;
            sr = SIPX_RESULT_SUCCESS ;

            gLineHandleMap.lock() ;
            gLineHandleMap.removeHandle(hLine) ;                
            gLineHandleMap.unlock() ;            
        }
        else
        {
            sr = SIPX_RESULT_FAILURE ;
        }        
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;    
}


SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(SIPX_LINE hLine,                                                 
                                                const char* szUserID,
                                                const char* szPasswd,
                                                const char* szRealm) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
    if (pData)         
    {
        if (szUserID && szPasswd && szRealm)
        {
            UtlString passwordToken;
            HttpMessage::buildMd5UserPasswordDigest(szUserID, szRealm, szPasswd, passwordToken);

            UtlBoolean rc = pData->pInst->pLineManager->addCredentialForLine(*pData->lineURI,                
                    szRealm,
                    szUserID,
                    passwordToken,                
                    HTTP_DIGEST_AUTHENTICATION) ;

            assert(rc) ;
            if (rc)
            {
                sr = SIPX_RESULT_SUCCESS ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
        sipxLineReleaseLock(pData, SIPX_LOCK_READ) ;
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }    

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxLineGet(const SIPX_INST hInst,
                                     SIPX_LINE lines[],
                                     const size_t max,
                                     size_t& actual)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
    actual = 0 ;
    
    if (pInst) 
    {
        SipLine* pLines = new SipLine[max] ;
        int iActual ;

        assert(pLines) ;
        if (pLines)
        {
            pInst->pLineManager->getLines(max, iActual, pLines) ;    // rc is > 0 lines (useless)
            if (iActual > 0)
            {
                actual = (size_t) iActual ;
                for (size_t i=0; i<actual; i++)
                {
                    lines[i] = sipxLineLookupHandle(pLines[i].getIdentity().toString()) ;
                }
            }
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }    

    return sr ;    
}


SIPXTAPI_API SIPX_RESULT sipxLineGetURI(const SIPX_LINE hLine,
                                        char*  szBuffer,
                                        const size_t nBuffer,
                                        size_t& nActual) 
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
    if (pData)
    {
        assert(pData != NULL) ;
        assert(pData->lineURI != NULL) ;

        if (pData)
        {
            if (pData->lineURI)
            {
                if (szBuffer)
                {
                    strncpy(szBuffer, pData->lineURI->toString().data(), nBuffer) ;                

                    // Make sure it is null terminated
                    szBuffer[nBuffer-1] = 0 ;
                    nActual = strlen(szBuffer) + 1 ;
                }
                else
                {
                    nActual = strlen(pData->lineURI->toString().data()) + 1;
                }

                sr = SIPX_RESULT_SUCCESS ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }    

        sipxLineReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return sr ;    
}


SIPXTAPI_API SIPX_RESULT sipxConfigEnableLog(const char* szFileName,
										     SIPX_LOG_LEVEL logLevel) 
{	
	// Start up logger thread
	initLogger() ;

	// Configure -- if passing a null szFilename, effectively turn off logging.
    OsSysLog::setOutputFile(0, szFileName) ;
    if (szFileName)
    {
        OsSysLog::setLoggingPriority((const enum tagOsSysLogPriority) logLevel) ;  
    }
    else
    {
        OsSysLog::setLoggingPriority((const enum tagOsSysLogPriority) LOG_LEVEL_EMERG) ;  
    }

    return SIPX_RESULT_SUCCESS ;
}



SIPXTAPI_API SIPX_RESULT sipxConfigSetMicAudioHook(fnMicAudioHook hookProc) 
{
    MprFromMic::s_fnMicDataHook = hookProc ;
    
    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetSpkrAudioHook(fnSpkrAudioHook hookProc) 
{
    MprToSpkr::s_fnToSpeakerHook = hookProc ;

    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundProxy(const SIPX_INST hInst,
                                                    const char* szProxy) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent) 
        {
            pInst->pSipUserAgent->setProxyServers(szProxy) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvTimeouts(const int initialTimeoutInSecs, 
                                                     const int retries) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    assert(initialTimeoutInSecs > 0) ;
    assert(retries > 0) ;

    if ((initialTimeoutInSecs > 0) && (retries > 0))
    {
        SipSrvLookup::setDnsSrvTimeouts(initialTimeoutInSecs, retries) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigEnableRport(const SIPX_INST hInst,
                                               const bool bEnable) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent) 
        {
            pInst->pSipUserAgent->setUseRport(bEnable) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterExpiration(const SIPX_INST hInst,
                                                      const int nRegisterExpirationSecs)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        assert(pInst->pRefreshManager);
        if (pInst->pRefreshManager) 
        {
            pInst->pRefreshManager->setRegistryPeriod(nRegisterExpirationSecs);
            rc = SIPX_RESULT_SUCCESS;
        }
    }

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetSubscribeExpiration(const SIPX_INST hInst,
                                                      const int nSubscribeExpirationSecs)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        assert(pInst->pRefreshManager);
        if (pInst->pRefreshManager) 
        {
            pInst->pRefreshManager->setSubscribeTimeout(nSubscribeExpirationSecs);
            rc = SIPX_RESULT_SUCCESS;
        }
    }

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvFailoverTimeout(const SIPX_INST hInst, const int failoverTimeoutInSecs)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent) 
        {
            pInst->pSipUserAgent->setDnsSrvTimeout(failoverTimeoutInSecs);
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigVoicemailSubscribe(const SIPX_INST hInst, 
													  const char* szSubscribeURL)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        assert(pInst->pRefreshManager);
        assert(pInst->pLineManager);
        assert(pInst->pSipUserAgent);
        if (pInst->pRefreshManager && pInst->pLineManager && pInst->pSipUserAgent) 
        {
            if (szSubscribeURL != NULL)
            {
                UtlString userId;
                UtlString contactUri;
                UtlString outboundLine;
                Url subscribeUrl(szSubscribeURL);
                subscribeUrl.getUserId(userId);

                pInst->pLineManager->getDefaultOutboundLine(outboundLine);

                // If the PHONESET_MSG_WAITING_SUBSCRIBE setting does
                // not have a userid field specified, get the one
                // from the default outbound line and use that
                if( userId.isNull() )
                {
                    // get default outbound line from the line manager
                    Url outbound(outboundLine);
                    outbound.getUserId(userId);
                    subscribeUrl.setUserId(userId);                    
                }

                // make sure we associate a user with the line
                pInst->pSipUserAgent->getContactUri( &contactUri );
                Url contactForLine ( contactUri );
                contactForLine.setUserId( userId );

                UtlString contactforLineStr = contactForLine.toString();

                SipMessage* mwiSubscribeRequest = new SipMessage();

                int sipSubscribePeriodSeconds;

                // get the Subscribe Period from the RefreshManager
                sipSubscribePeriodSeconds = pInst->pRefreshManager->getSubscribeTimeout();
                
                mwiSubscribeRequest->setVoicemailData (
                    outboundLine.data(),           // fromField
                    outboundLine.data(),           // toField
                    subscribeUrl.toString().data(),// uri
                    contactforLineStr.data(),      // contactUrl
                    NULL,                          // callId, do not set here
                    1,                             // cseq
                    sipSubscribePeriodSeconds);    // expiresInSeconds

                // send the subscribe request to the refresh manager
                pInst->pRefreshManager->newSubscribeMsg( *mwiSubscribeRequest );
                rc = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer, 
                                              int iKeepAliveInSec) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {
        pInst->pCallManager->enableStun(szServer, iKeepAliveInSec) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigDisableStun(const SIPX_INST hInst) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        pInst->pCallManager->enableStun(NULL, 0) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}



#endif /* ] SIPXTAPI_EXCLUDE */
