//
// Copyright (C) 2005-2020 SIPez LLC. All rights reserved.
// 
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <MMSystem.h>
#else /* _WIN32 */
#include <os/OsIntTypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */
#include <math.h>

// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include <tapi/SipXHandleMap.h>
#include <tapi/SipXMessageObserver.h>
#include <tapi/SipXEventDispatcher.h>
//#include <rtcp/RTCManager.h>
#include <net/SipUserAgent.h>
#include <sdp/SdpCodecList.h>
#include <sdp/SdpDefaultCodecFactory.h>
#include <cp/CallManager.h>
#include <mp/MprVoiceActivityNotifier.h>
#include <mp/MpResourceTopology.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <mi/CpMediaInterfaceFactoryImpl.h>
// This is poluting the interface indepenence a bit:
#include <CpTopologyGraphFactoryImpl.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>


#include "ptapi/PtProvider.h"
#include "net/Url.h"
#include "utl/UtlNameValueTokenizer.h"
#include "os/OsConfigDb.h"
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "os/OsLock.h"
#include "os/OsMutex.h"
#include "os/OsSysLog.h"
#include "os/OsTimerTask.h"
#include "os/OsNatAgentTask.h"
#include "net/TapiMgr.h"
#include "net/SipSrvLookup.h"
#include "net/SipSubscribeServer.h"
#include "net/SipSubscribeClient.h"
#include "net/SipDialogMgr.h"
#include "net/SipPublishContentMgr.h"
#include "os/HostAdapterAddress.h"
#include "utl/UtlSList.h"
#include "utl/UtlHashMapIterator.h"

#ifdef ANDROID
#include <mp/MpAndroidAudioBindingInterface.h>
#endif

#ifdef VOICE_ENGINE
#include "include/VoiceEngineFactoryImpl.h"
#else
#include "mp/MprFromMic.h"
#include "mp/MprToSpkr.h"
#endif

// DEFINES
// GLOBAL VARIABLES

// EXTERNAL VARIABLES
extern SipXHandleMap* gpCallHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap* gpLineHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap* gpConfHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap* gpInfoHandleMap ;   // sipXtapiInternal.cpp
extern SipXHandleMap* gpPubHandleMap ;    // sipXtapiInternal.cpp
extern SipXHandleMap* gpSubHandleMap ;    // sipXtapiInternal.cpp
extern SipXHandleMap* gpTransportHandleMap ; // sipXtapiInternal.cpp
extern UtlDList*      gpSessionList ;     // sipXtapiInternal.cpp
extern OsMutex gSubscribeAccessLock; // sipXtapiInternal.cpp
// EXTERNAL FUNCTIONS

// STRUCTURES

// GLOBALS
//static bool gbHibernated = false;

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

// jaro: CHECKED
static void initLogger()
{
   OsSysLog::initialize(0, // do not cache any log messages in memory
                        "sipXtapi"); // name for messages from this program
}

// jaro: CHECKED, external lock on mutex is assumed
void destroyCallData(SIPX_CALL_DATA* pData)
{
   if (pData)
   {
      // Increment call count
      pData->pInst->pLock->acquire();
      pData->pInst->nCalls--;
      assert(pData->pInst->nCalls >= 0);
      pData->pInst->pLock->release();

      // delete tests for NULL automatically
      delete pData->callId;
      pData->callId = NULL;
      delete pData->lineURI;
      pData->lineURI = NULL;
      delete pData->remoteAddress;
      pData->remoteAddress = NULL;
      delete pData->ghostCallId;
      pData->ghostCallId = NULL;
      delete pData->sessionCallId;
      pData->sessionCallId = NULL;
      delete pData->contactAddress;
      pData->contactAddress = NULL;
#ifdef __APPLE__ // [
      // Under Mac OS X pthread_rwlock_destroy() function will fail if
      // the system has detected an attempt to destroy the object referenced by
      // rwlock while it is locked.
      pData->pMutex->releaseRead();
      pData->pMutex->releaseWrite();
#endif // __APPLE__ ]
      // no need to release mutex, nobody should be waiting on it or its a bug
      delete pData->pMutex;
      pData->pMutex = NULL;
      delete pData;
   }
}

// jaro: CHECKED
static SIPX_LINE_DATA* createLineData(SIPX_INSTANCE_DATA* pInst, const Url& uri)
{
   SIPX_LINE_DATA* pData = new SIPX_LINE_DATA();
   // if there is allocation failure, std::bad_alloc exception is thrown
   // unless we use _set_new_handler to set new handler
   // we do not handle std::bad_alloc thus program will exit, which is ok
   // if we want to check for NULL, then we would have to use
   // new(std:::nothrow) instead of just new

   pData->lineURI = new Url(uri);
   pData->pInst = pInst;
   pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
   pData->pLineAliases = NULL;

   pInst->pLock->acquire();
   pInst->nLines++;
   pInst->pLock->release();

   return pData ;
}



/****************************************************************************
 * Initialization
 ***************************************************************************/

static void initAudioDevices(SIPX_INSTANCE_DATA* pInst)
{
    int i ;

    // Clear devices
    for (i=0; i<MAX_AUDIO_DEVICES; i++)
    {
        pInst->inputAudioDevices[i] = NULL ;
        pInst->outputAudioDevices[i] = NULL ;
    }

    UtlSList inputDeviceList;
    int inputDevicesFound = CpTopologyGraphFactoryImpl::getInputDeviceList(inputDeviceList);
    OsSysLog::add(FAC_SIPXTAPI, inputDevicesFound > 0 ? PRI_DEBUG : PRI_ERR,
                  "initAudioDevices found: %d input devices",
                  inputDevicesFound);

    UtlSListIterator inputIterator(inputDeviceList);
    UtlString* inputDeviceName = NULL;
    int numInputDevices = 0;
    while(numInputDevices < MAX_AUDIO_DEVICES && 
          (inputDeviceName = (UtlString*) inputIterator()))
    {
        pInst->inputAudioDevices[numInputDevices] = strdup(inputDeviceName->data());
        OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                      "initAudioDevices adding input device[%d]: %s",
                      numInputDevices,
                      inputDeviceName->data());
        numInputDevices++;
    }

    UtlSList outputDeviceList;
    int outputDevicesFound = CpTopologyGraphFactoryImpl::getOutputDeviceList(outputDeviceList);
    OsSysLog::add(FAC_SIPXTAPI, outputDevicesFound > 0 ? PRI_DEBUG : PRI_ERR,
                  "initAudioDevices found: %d output devices",
                  inputDevicesFound);

    UtlSListIterator outputIterator(outputDeviceList);
    UtlString* outputDeviceName = NULL;
    int numOutputDevices = 0;
    while(numOutputDevices < MAX_AUDIO_DEVICES && 
          (outputDeviceName = (UtlString*) outputIterator()))
    {
        pInst->outputAudioDevices[numOutputDevices] = strdup(outputDeviceName->data());
        OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                      "initAudioDevices adding output device[%d]: %s",
                      numOutputDevices,
                      outputDeviceName->data());
        numOutputDevices++;
    }
}


SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST*  phInst,
                                        const int   udpPort,
                                        const int   tcpPort,
                                        const int   tlsPort,
                                        const int   rtpPortStart,
                                        const int   maxConnections,
                                        const char* szIdentity,
                                        const char* szBindToAddr,
                                        bool        bUseSequentialPorts,
                                        const char* szTLSCertificateNickname,
                                        const char* szTLSCertificatePassword,
                                        const char* szDbLocation,
                                        bool        bEnableLocalAudio,
                                        const int   internalSamplerate,
                                        const int   devicesSamplerate,
                                        const int   internalFrameSizeMs,
                                        const char *callInputDeviceName,
                                        const char *callOutputDeviceName)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxInitialize");
    int iActualTLSPort = tlsPort ;

#ifdef LOG_TO_FILE
    // Start up logger thread
    initLogger() ;
    OsSysLog::setLoggingPriority((OsSysLogPriority) LOG_LEVEL_DEBUG) ;  
    OsSysLog::setOutputFile(0, "sipXtapi.log");
#endif

    char cVersion[80] ;
    sipxConfigGetVersion(cVersion, sizeof(cVersion)) ;
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "%s", cVersion);

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxInitialize tcpPort=%d udpPort=%d tlsPort=%d rtpPortStart=%d"
            " maxConnections=%d identity=%s bindTo=%s sequentialPorts=%d"
            " certNickname=%s, DBLocation=%s bEnableLocalAudio=%s internalSamplerate=%d"
            " devicesSamplerate=%d internalFrameSizeMs=%d callInputDeviceName=\"%s\" callOutputDeviceName=\"%s\"",
            tcpPort, udpPort, iActualTLSPort, rtpPortStart, maxConnections,
            ((szIdentity != NULL) ? szIdentity : ""),
            ((szBindToAddr != NULL) ? szBindToAddr : ""),
            bUseSequentialPorts,
            ((szTLSCertificateNickname != NULL) ? szTLSCertificateNickname : ""),
            ((szDbLocation != NULL) ? szDbLocation : ""),
            (bEnableLocalAudio ? "true" : "false"),
            internalSamplerate,
            devicesSamplerate,
            internalFrameSizeMs,
            callInputDeviceName,
            callOutputDeviceName);

    sipxStructureIntegrityCheck();

    // Disable Console by default
    enableConsoleOutput(false) ;

#ifdef WIN32
    // Validate Network status (need to be implemented on linux/macos)
    const char* szAddresses[1] ;
    const char* szAdapters[1] ;
    int numAdapters = 1 ;
    if (sipxConfigGetAllLocalNetworkIps(szAddresses, szAdapters, numAdapters) == SIPX_RESULT_SUCCESS)
    {
        if (numAdapters > 0)
        {
            free((void*) szAddresses[0]) ;
            free((void*) szAdapters[0]) ;
        }
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "No network interfaces found") ;
            return SIPX_RESULT_NETWORK_FAILURE ;
        }
    }
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "Unable to query for network interfaces") ;
        return SIPX_RESULT_NETWORK_FAILURE ;
    }
#else
#warning "Network availability check not implemented on non-WIN32"
#endif

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

#ifdef ANDROID
// Load and initialize correct driver for this version of Android
    OsStatus stat = MpAndroidAudioBindingInterface::spGetAndroidAudioBinding()->getStatus();
    //OsStatus stat = MpAndroidAudioTrack::setAudioTrackCreator();
    switch(stat)
    {
        // Things are good continue
        case OS_SUCCESS:
        break;

        // Could not find audio driver symbol in sipX android audio drivers shared lib
        case OS_PLATFORM_NOT_SUPPORTED:
            rc = SIPX_RESULT_OS_PLATFORM_UNSUPPORTED;
        break;

        // Could not find sipX android audio drivers shared lib specific to this version
        case OS_NOT_FOUND:
            rc = SIPX_RESULT_LIB_NOT_FOUND;
        break;

        // Unknown case
            rc = SIPX_RESULT_NOT_SUPPORTED;
        default:

        break;
    }
    if(stat != OS_SUCCESS)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
        "Failed to initialize Android media bindings, OsStatus: %d SIPX_RESULT: %d", stat, rc);

        return(rc);
    }
#endif
   
    rc = SIPX_RESULT_INVALID_ARGS ;

#ifdef SIPXTAPI_EVAL_EXPIRATION
    OsDateTime expireDate(EVAL_EXPIRE_YEAR, EVAL_EXPIRE_MONTH, EVAL_EXPIRE_DAY, 23, 59, 59, 0) ;
    OsDateTime nowDate ;         
    OsTime expireTime ;
    OsTime nowTime ;

    OsDateTime::getCurTime(nowDate) ;   
    expireDate.cvtToTimeSinceEpoch(expireTime) ;
    nowDate.cvtToTimeSinceEpoch(nowTime) ;

    if (nowTime > expireTime)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "sipXtapi has expired") ;
        return SIPX_RESULT_EVAL_TIMEOUT ;
    }
#endif

    // check for the security runtime, if it is not there,
    // and a TLS port has been specified,
    // return with failure
#ifdef HAVE_NSS
    if (iActualTLSPort > 0)
    {
        rc = sipxConfigLoadSecurityRuntime();
        if (SIPX_RESULT_SUCCESS != rc)
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "Disabling TLS: Specified TLS port %d; however, NSS runtime not found", iActualTLSPort) ;
            iActualTLSPort = -1 ;
        }
    }
#else
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipXtapi built without NSS support") ;
#endif

    // set the sipXtackLib's tapi callback function pointers
    TapiMgr::getInstance().setTapiCallCallback(&sipxFireCallEvent);
    TapiMgr::getInstance().setTapiMediaCallback(&sipxFireMediaEvent);
    TapiMgr::getInstance().setTapiLineCallback(&sipxFireLineEvent);
    TapiMgr::getInstance().setTapiCallback(&sipxFireEvent);

    SIPX_INSTANCE_DATA* pInst = new SIPX_INSTANCE_DATA;
    memset(pInst, 0, sizeof(SIPX_INSTANCE_DATA)) ;
    pInst->audioCodecSetting.pPreferences = new UtlString() ;
    pInst->videoCodecSetting.pPreferences = new UtlString() ;

    // Create Line and Refersh Manager
    pInst->pLineManager = new SipLineMgr() ;
    pInst->pRefreshManager = new SipRefreshMgr() ;
    pInst->pRefreshManager->setLineMgr(pInst->pLineManager);

    // Init counts
    pInst->pLock = new OsMutex(OsMutex::Q_FIFO) ;
    pInst->nCalls = 0 ;
    pInst->nLines = 0 ;
    pInst->nConferences = 0 ; 

    if (szBindToAddr == NULL)
    {
        szBindToAddr = "0.0.0.0" ;
    }

    // Default energy level notification period mSec.
    pInst->nEnergyLevelNotificationPeriodMs = 1000;

    // Bind the SIP user agent to a port and start it up
    pInst->pSipUserAgent = new SipUserAgent(
            tcpPort,                    // sipTcpPort
            udpPort,                    // sipUdpPort
            iActualTLSPort,             // sipTlsPort
            NULL,                       // publicAddress
            NULL,                       // defaultUser
            szBindToAddr,               // default IP Address
            NULL,                       // sipProxyServers
            NULL,                       // sipDirectoryServers
            NULL,                       // sipRegistryServers
            NULL,                       // authenticationScheme
            NULL,                       // authenicateRealm
            NULL,                       // authenticateDb
            NULL,                       // authorizeUserIds
            NULL,                       // authorizePasswords
            pInst->pLineManager,        // lineMgr
            SIP_DEFAULT_RTT,            // sipFirstResendTimeout
            TRUE,                       // defaultToUaTransactions
            -1,                         // readBufferSize
            OsServerTask::DEF_MAX_MSGS, // queueSize
            bUseSequentialPorts,        // bUseNextAvailablePort
            szTLSCertificateNickname,
            szTLSCertificatePassword,
            szDbLocation);
    pInst->pSipUserAgent->allowMethod(SIP_INFO_METHOD);

    UtlString defaultBindAddressString;
  	int unused;

    pInst->pSipUserAgent->getLocalAddress(&defaultBindAddressString, &unused, TRANSPORT_UDP);
    unsigned long defaultBindAddress = inet_addr(defaultBindAddressString.data());
    OsSocket::setDefaultBindAddress(defaultBindAddress);
    pInst->pSipUserAgent->start();    
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "Default bind address %s, udpPort=%d, tcpPort=%d, tlsPort=%d",
            defaultBindAddressString.data(),
            pInst->pSipUserAgent->getUdpPort(),
            pInst->pSipUserAgent->getTcpPort(),
            pInst->pSipUserAgent->getTlsPort()) ;

    // Startup Line Manager  Refresh Manager
    pInst->pLineManager->start() ;
    pInst->pLineManager->initializeRefreshMgr(pInst->pRefreshManager) ;    
    pInst->pRefreshManager->init(pInst->pSipUserAgent, pInst->pSipUserAgent->getTcpPort(), pInst->pSipUserAgent->getUdpPort()) ;
    pInst->pRefreshManager->StartRefreshMgr();

    // Create and start up a SIP SUBSCRIBE server
    pInst->pSubscribeServer = 
        SipSubscribeServer::buildBasicServer(*pInst->pSipUserAgent);
    pInst->pSubscribeServer->start();

    // Create and start up a SIP SUBSCRIBE client
    pInst->pDialogManager = new SipDialogMgr;
    pInst->pSipRefreshManager = 
        new SipRefreshManager(*pInst->pSipUserAgent,
                                *pInst->pDialogManager);
    pInst->pSipRefreshManager->start();
    pInst->pSubscribeClient = 
        new SipSubscribeClient(*pInst->pSipUserAgent,
                                *pInst->pDialogManager, 
                                *pInst->pSipRefreshManager);
    pInst->pSubscribeClient->start();

    // Enable all codecs codecs
    pInst->pCodecFactory = new SdpCodecList() ;

    // Instantiate the call processing subsystem
    UtlString localAddress;
    UtlString utlIdentity(szIdentity);
    if (!utlIdentity.contains("@"))
    {
        OsSocket::getHostIp(&localAddress);
        char *szBuf = (char*) calloc(64 + utlIdentity.length(), 1) ;
        sprintf(szBuf, "sip:%s@%s:%d", szIdentity, localAddress.data(), pInst->pSipUserAgent->getUdpPort()) ;
        localAddress = szBuf ;
        free(szBuf) ;
    }
    else
    {
        localAddress = utlIdentity;
    }
    
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "Default Identity: %s\n",
            localAddress.data()) ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "internalFrameSizeMs: %d devicesSamplerate: %d bEnableLocalAudio: %s callInputDeviceName: %s callOutputDeviceName: %s", 
           internalFrameSizeMs, devicesSamplerate, bEnableLocalAudio ? "true" : "false", callInputDeviceName, callOutputDeviceName);

    OsConfigDb configDb;
    configDb.set("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxConnections);

    CpMediaInterfaceFactory* interfaceFactory =
                            sipXmediaFactoryFactory(&configDb, internalFrameSizeMs,
                                                    // This should be the max of the two rates, used for buffer size
                                                    internalSamplerate > devicesSamplerate ? internalSamplerate : devicesSamplerate,
                                                    devicesSamplerate,
                                                    bEnableLocalAudio,
                                                    callInputDeviceName,
                                                    callOutputDeviceName);

    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxInitialize interfaceFactory: %p", interfaceFactory);

    pInst->pCallManager = new CallManager(FALSE,
                            pInst->pLineManager,
                            TRUE, // early media in 180 ringing
                            pInst->pCodecFactory,
                            rtpPortStart, // rtp start
#ifdef VIDEO
                            rtpPortStart + (4 * maxConnections), // rtp end
#else
                            rtpPortStart + (2 * maxConnections), // rtp end
#endif
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
                            5000, // offeringDelay
                            "",
                            CP_MAXIMUM_RINGING_EXPIRE_SECONDS,
                            QOS_LAYER3_LOW_DELAY_IP_TOS,
                            maxConnections,
                            interfaceFactory,
                            internalSamplerate);


    // Start up the call processing system
    pInst->pCallManager->setOutboundLine(localAddress) ;
    pInst->pCallManager->start();

    CpMediaInterfaceFactoryImpl* pInterface = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
    if (pInterface == NULL)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "Unable to create global media interface") ;
    }
    else if (szIdentity)
    {
        pInterface->setRTCPName(szIdentity) ;
    }

    sipxConfigSetAudioCodecPreferences(pInst, AUDIO_CODEC_BW_NORMAL);
    sipxConfigSetVideoBandwidth(pInst, VIDEO_CODEC_BW_NORMAL);

    initAudioDevices(pInst) ;

#ifdef VOICE_ENGINE
    sipxCreateLocalAudioConnection(pInst) ;
#endif

    // Setup listener delegation
    pInst->pEventDispatcher = new SipXEventDispatcher(pInst) ;
    pInst->pEventDispatcher->start() ;

    *phInst = pInst ;
    gpSessionList->insert(new UtlVoidPtr(pInst)) ;
    sipxIncSessionCount();
    
    // create the message observer
    pInst->pMessageObserver = new SipXMessageObserver(pInst);
    pInst->pMessageObserver->start();
    pInst->pSipUserAgent->addMessageObserver(*(pInst->pMessageObserver->getMessageQueue()), SIP_INFO_METHOD, 1, 0, 1, 0, 0, 0, (void*)pInst);

    // Enable ICE by default (only makes sense with STUN, TURN or with mulitple nics 
    // (multiple nic support still needs work).
    //sipxConfigEnableIce(pInst) ;

    pInst->bAllowHeader = true;
    pInst->bDateHeader = true;
    //sipxConfigEnableSipShortNames(pInst, true);

    rc = SIPX_RESULT_SUCCESS ;
    //  check for TLS initialization
#ifdef SIP_TLS
    if (pInst->pSipUserAgent->getTlsServer() && iActualTLSPort > 0 && szTLSCertificateNickname != NULL)
    {
        OsStatus initStatus = pInst->pSipUserAgent->getTlsServer()->getTlsInitCode();
        if (initStatus != OS_SUCCESS)
        {
            rc = SIPX_RESULT_FAILURE;
        }
        else
        {
            sipxConfigSetSecurityParameters((SIPX_INST)pInst,
                                            szTLSCertificateNickname,
                                            szTLSCertificatePassword,
                                            szDbLocation);
        }
    }   
#endif

    pInst->pKeepaliveDispatcher = new KeepaliveEventDispatcher(pInst->pCallManager) ;

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxReInitialize(SIPX_INST*  phInst,
                                          const int   udpPort,
                                          const int   tcpPort,
                                          const int   tlsPort,
                                          const int   rtpPortStart,
                                          const int   maxConnections,
                                          const char* szIdentity,
                                          const char* szBindToAddr,
                                          bool        bUseSequentialPorts,
                                          const char* szTLSCertificateNickname,
                                          const char* szTLSCertificatePassword,
                                          const char* szDbLocation)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxReInitialize");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxReInitialize hInst=%p",
            *phInst);
            
    if (phInst)
    {
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) *phInst ;

        if (pInst)
        {
            sipxSubscribeDestroyAll(*phInst) ;
            sipxPublisherDestroyAll(*phInst) ;        
            sipxCallDestroyAll(*phInst) ;
            sipxConferenceDestroyAll(*phInst) ;
            sipxLineRemoveAll(*phInst) ;

            sipxDisableListeners() ;
            sipxUnInitialize(*phInst, true);
            sipxEnableListeners() ;
        }

        rc = sipxInitialize(phInst,
                       udpPort,
                       tcpPort,
                       tlsPort, 
                       rtpPortStart,
                       maxConnections,
                       szIdentity,
                       szBindToAddr,
                       bUseSequentialPorts,
                       szTLSCertificateNickname,
                       szTLSCertificatePassword,
                       szDbLocation);

    }

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst,
                                          bool      bForceShutdown)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxUnInitialize");

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxUnInitialize hInst=%p",
        hInst);
        
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        // Verify that all calls are torn down and that no lines
        // are present.

        int iAttempts = 0 ;
        int nCalls ;
        int nConferences ;
        int nLines ;
        int nCallManagerCalls ;
        UtlString checkCalls[4] ;

        do 
        {
            pInst->pLock->acquire() ;
            nCalls = pInst->nCalls ;
            nConferences = pInst->nConferences ;
            nLines = pInst->nLines ;
            nCallManagerCalls = 0 ;
            sipxGetActiveCallIds(hInst, 4, nCallManagerCalls, checkCalls)  ;

            pInst->pLock->release() ;

            if ((nCalls != 0) || (nConferences != 0) || (nLines != 0) || (nCallManagerCalls != 0))
            {
                OsTask::delay(250) ;
                iAttempts++ ;

                OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
                "Busy SIPX_INST (Waiting) (%p) nCalls=%d, nLines=%d, nConferences=%d nCallManagerCalls=%d",
                        hInst, nCalls, nLines, nConferences, nCallManagerCalls) ;
            }
            else
            {
                break ;
            }
        } while (iAttempts < 10) ;

        if ( bForceShutdown || ((nCalls == 0) && (nConferences == 0) && 
                (nLines == 0) && (nCallManagerCalls == 0)) )
        {
#ifdef VOICE_ENGINE
            sipxDestroyLocalAudioConnection(pInst) ;
#endif            
            
            // First: Shutdown user agent to avoid processing during teardown
            pInst->pSipUserAgent->shutdown(TRUE) ;
            pInst->pSipUserAgent->requestShutdown();

            // get rid of pointer to the line manager in the refresh manager
            pInst->pRefreshManager->setLineMgr(NULL);
            pInst->pLineManager->requestShutdown();
            pInst->pCallManager->requestShutdown();
            pInst->pRefreshManager->requestShutdown();
            pInst->pSubscribeClient->requestShutdown();
            pInst->pSubscribeServer->requestShutdown();
            pInst->pSipRefreshManager->requestShutdown();
            pInst->pMessageObserver->requestShutdown();
            pInst->pCodecFactory->clearCodecs();
            if (pInst->pEventDispatcher)
            {
                pInst->pEventDispatcher->requestShutdown();
            }            

            delete pInst->pSubscribeClient ;
            delete pInst->pSubscribeServer ;
            delete pInst->pRefreshManager ;
            delete pInst->pSipRefreshManager ;
            delete pInst->pDialogManager ;
            delete pInst->pSipUserAgent;
            delete pInst->pLineManager;            
            delete pInst->pCallManager;
            delete pInst->pCodecFactory;            

            sipxDestroyMediaFactoryFactory() ;

            // Destroy the timer task to flush timers
            OsTimerTask::destroyTimerTask() ;
            pInst->pCallManager = NULL;

            sipxDecSessionCount();
            if (sipxGetSessionCount() == 0)
            {
                OsNatAgentTask::releaseInstance();
            }            

            int codecIndex;
            // Did we previously allocate an audio codecs array and store it in our codec settings?
            if (pInst->audioCodecSetting.bInitialized)
            {
                // Free up the previously allocated codecs and the array
                for (codecIndex = 0; codecIndex < pInst->audioCodecSetting.numCodecs; codecIndex++)
                {
                    if (pInst->audioCodecSetting.sdpCodecArray[codecIndex])
                    {
                        delete pInst->audioCodecSetting.sdpCodecArray[codecIndex];
                        pInst->audioCodecSetting.sdpCodecArray[codecIndex] = NULL;
                    }
                }
                delete[] pInst->audioCodecSetting.sdpCodecArray;
                pInst->audioCodecSetting.sdpCodecArray = NULL;
                pInst->audioCodecSetting.bInitialized = false;
            }
            // Did we previously allocate a video codecs array and store it in our codec settings?
            if (pInst->videoCodecSetting.bInitialized)
            {
                // Free up the previuosly allocated codecs and the array
                for (codecIndex = 0; codecIndex < pInst->videoCodecSetting.numCodecs; codecIndex++)
                {
                    if (pInst->videoCodecSetting.sdpCodecArray[codecIndex])
                    {
                        delete pInst->videoCodecSetting.sdpCodecArray[codecIndex];
                        pInst->videoCodecSetting.sdpCodecArray[codecIndex] = NULL;
                    }
                }
                delete[] pInst->videoCodecSetting.sdpCodecArray;
                pInst->videoCodecSetting.sdpCodecArray = NULL;
                pInst->videoCodecSetting.bInitialized = false;
            }

            for (int i=0; i<MAX_AUDIO_DEVICES; i++)
            {
                if (pInst->inputAudioDevices[i])
                {
                    free(pInst->inputAudioDevices[i]) ;
                    pInst->inputAudioDevices[i] = NULL ;
                }

                if (pInst->outputAudioDevices[i])
                {                    
                    free(pInst->outputAudioDevices[i]) ;
                    pInst->outputAudioDevices[i] = NULL ;
                }
            }
        
            UtlVoidPtr key(pInst) ;
            gpSessionList->destroy(&key) ;

            if (pInst->pStunNotification != NULL)
            {
                delete pInst->pStunNotification ;
                pInst->pStunNotification = NULL ;
            }

            if (pInst->pMessageObserver)
            {
                delete pInst->pMessageObserver ;
                pInst->pMessageObserver = NULL ;
            }

            if (pInst->pEventDispatcher)
            {
                delete pInst->pEventDispatcher;
            }

            sipxTransportDestroyAll(pInst) ;

            delete pInst->pLock ;
            delete pInst->pKeepaliveDispatcher ;

            delete pInst->audioCodecSetting.pPreferences ;
            delete pInst->videoCodecSetting.pPreferences ;

            delete pInst;
            pInst = NULL;

            OsSysLog::shutdown() ;

            // Destroy the timer task once more -- some of the destructors (SipUserAgent)
            // mistakenly re-creates them when terminating.
            OsTimerTask::destroyTimerTask() ;

            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                "Unable to shutdown busy SIPX_INST (%p) nCalls=%d, nLines=%d, nConferences=%d",
                        hInst, nCalls, nLines, nConferences) ;
            rc = SIPX_RESULT_BUSY ;
        }
    }
    
    // sipxDestroyMediaFactoryFactory is now being called regardless of the return code
    // failure to do so could cause a hang, at least it does using the VoiceEngine media adapter
    sipxDestroyMediaFactoryFactory() ;
    
    return rc ;
}
/****************************************************************************
 * Call Related Functions
 ***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL   hCall,
                                        SIPX_VIDEO_DISPLAY* const pDisplay,
                                        SIPX_SECURITY_ATTRIBUTES* const pSecurity,
                                        SIPX_CALL_OPTIONS* options,
                                        bool bSendEarlyMedia)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAccept");
    bool bEnableLocationHeader=false;
    int bandWidth=AUDIO_CODEC_BW_DEFAULT;
    SIPX_CONTACT_ID contactId = 0 ;

    if (options != NULL && options->cbSize)
    {
        bEnableLocationHeader = options->sendLocation;
        bandWidth = options->bandwidthId;
        if (options->cbSize == sizeof(SIPX_CALL_OPTIONS))
        {
            contactId = options->contactId ;
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAccept hCall=%d display=%p bEnableLocationHeader=%d bandWidth=%d, contactId=%d bSendEarlyMedia=%s",
        hCall, pDisplay, bEnableLocationHeader, bandWidth, contactId, 
        bSendEarlyMedia ? "true" : "false");

    if (pSecurity)
    {
        SIPX_RESULT rc = sipxConfigLoadSecurityRuntime();
        if (SIPX_RESULT_SUCCESS != rc)
        {
            return rc;  // return right away 
        }
    }

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    char* pLocationHeader = NULL;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {

        // Log contact ID
        if (contactId > 0)
        {
            SIPX_CONTACT_ADDRESS* pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
            OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                    "contactId(%d): type: %s, transport: %s interface: %s, ip: %s:%d",
                    contactId,
                    sipxContactTypeToString((SIPX_CONTACT_TYPE) pContact->eContactType),
                    sipxTransportTypeToString((SIPX_TRANSPORT_TYPE) pContact->eTransportType),
                    pContact->cInterface,
                    pContact->cIpAddress,
                    pContact->iPort) ;
        }

        if (pInst && pSecurity)
        {
            // augment the security attributes with the instance's security parameters
            SecurityHelper securityHelper;
            // don't generate a random key, and remove one if one it is there
            // (the caller will provide the agreed upon key)
            pSecurity->setSrtpKey("", 30);
            if (pInst->dbLocation)
            {
                securityHelper.setDbLocation(*pSecurity, pInst->dbLocation);
            }
            if (pInst->myCertNickname)
            {
                securityHelper.setMyCertNickname(*pSecurity, pInst->myCertNickname);
            }
            if (pInst->dbPassword)
            {
                securityHelper.setDbPassword(*pSecurity, pInst->dbPassword);
            }
        }
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            // set the display object
            {
                SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
                if (pCallData)
                {
                    if (pDisplay)
                    {
                        pCallData->display = *pDisplay;   
                    }
                    if (pSecurity)
                    {
                        pCallData->security = *pSecurity;
                    }
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
                }
            }
            // Only take focus if something doesn't already have it.
            if (!sipxIsCallInFocus())
            {
                SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
                if (pCallData)
                {
                    pCallData->bInFocus = true ;                    
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
                }
                pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;
            }
            if (pInst)
            {
                pLocationHeader = (bEnableLocationHeader) ? pInst->szLocationHeader : NULL;
            }
            if (pSecurity && pSecurity->getSecurityLevel() > 0)
            {
                SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
                if (pCallData)
                {
                    pInst->pCallManager->acceptConnection(callId.data(),
                            remoteAddress.data(),
                            contactId, 
                            (void*)&pCallData->display,
                            (void*)&pCallData->security,
                            pLocationHeader,
                            bandWidth,
                            bSendEarlyMedia ? TRUE : FALSE);
                           
                    // If energy level notifications are turned on
                    if(pInst->nEnergyLevelNotificationPeriodMs > 0)
                    {
                        OsMsgQ tempQueue;
                        // Create and send message to turn on Mic energy notifications
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Setting speaker energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                              tempQueue,
                                                              pInst->nEnergyLevelNotificationPeriodMs);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Setting mic energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                              tempQueue,
                                                              pInst->nEnergyLevelNotificationPeriodMs);

                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(callId.data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }
                    else
                    {
                        OsMsgQ tempQueue;
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Disabling speaker energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                            tempQueue);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Disabling mic energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                            tempQueue);

                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(callId.data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }
 
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
                }
            }
            else
            {
                SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
                if (pCallData)
                {
                    pInst->pCallManager->acceptConnection(callId.data(),
                            remoteAddress.data(),
                            contactId, 
                            (void*)&pCallData->display,
                            NULL,
                            pLocationHeader,
                            bandWidth,
                            bSendEarlyMedia ? TRUE : FALSE);

                    // If energy level notifications are turned on
                    if(pInst->nEnergyLevelNotificationPeriodMs > 0)
                    {
                        // Create and send messages to turn on Mic and Speaker energy notifications
                        OsMsgQ tempQueue;
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Setting speaker energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                                  tempQueue,
                                                                  pInst->nEnergyLevelNotificationPeriodMs);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Setting mic energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                                  tempQueue,
                                                                  pInst->nEnergyLevelNotificationPeriodMs);

                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(callId.data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }
                    else
                    {
                        OsMsgQ tempQueue;
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Disabling speaker energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                            tempQueue);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallAccept: Disabling mic energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                            tempQueue);
                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(callId.data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }

                    sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
                }
            }
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall,
                                        const int errorCode,
                                        const char* szErrorText)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallReject");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallReject hCall=%d code=%d text=\"%s\"",
        hCall, errorCode, szErrorText);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            pInst->pCallManager->rejectConnection(callId.data(), remoteAddress.data(), errorCode, szErrorText) ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall, const char* szForwardURL)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallRedirect");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallRedirect hCall=%d forwardURL=%s",
        hCall, szForwardURL);
        
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


SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall, bool bTakeFocus)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAnswer");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAnswer hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        assert(remoteAddress.length()) ;
        if (remoteAddress.length())
        {
            SIPX_VIDEO_DISPLAY display;
            SIPX_SECURITY_ATTRIBUTES security;

            if (!sipxIsCallInFocus() || bTakeFocus)
            {
                SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
                if (pCallData)
                {
                    pCallData->bInFocus = true ;                    
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
                }

                // The hold even will remove the bInFocus param from 
                // the other connection
                pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;
            }

            SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            if (pCallData)
            {
                display = pCallData->display;
                security = pCallData->security;
                
                sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
            }

            if (display.handle)
            {
                if (security.getSecurityLevel() == SRTP_LEVEL_NONE)
                {
                    pInst->pCallManager->answerTerminalConnection(callId.data(), remoteAddress.data(), "unused", &display) ;
                }
                else
                {
                    pInst->pCallManager->answerTerminalConnection(callId.data(), remoteAddress.data(), "unused", &display, &pCallData->security) ;
                }
            }
            else
            {
                if (security.getSecurityLevel() == SRTP_LEVEL_NONE)
                {
                    pInst->pCallManager->answerTerminalConnection(callId.data(), remoteAddress.data(), "unused") ;
                }
                else
                {
                    pInst->pCallManager->answerTerminalConnection(callId.data(), remoteAddress.data(), "unused", NULL, &security) ;
                }
            }
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


static SIPX_RESULT sipxCallCreateHelper(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        const SIPX_CONF hConf,
                                        SIPX_CALL*  phCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallCreateHelper");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pLine = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);
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
                // Set Call ID
                pData->callId = new UtlString() ;
                assert(pData->callId != NULL) ;
                pInst->pCallManager->createCall(pData->callId) ;

                // Set Conference handle
                pData->hConf = hConf ;

                // Set Line URI
                pData->hLine = hLine ;
                pData->lineURI = new UtlString(pLine->lineURI->toString().data()) ;
                assert(pData->lineURI != NULL) ;

                // Remote Address
                pData->remoteAddress = NULL ;

                // Connection Id, initialize to -1, cache later
                pData->connectionId = -1;

                // Store Instance
                pData->pInst = pInst ;

                // Create Mutex
                pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;

                // Increment call count
                pInst->pLock->acquire() ;
                pInst->nCalls++ ;
                pInst->pLock->release() ;

                if ((pData->callId == NULL) || (pData->lineURI == NULL))
                {
                    *phCall = SIPX_CALL_NULL ;
                    destroyCallData(pData) ;
                    sr = SIPX_RESULT_OUT_OF_MEMORY ;
                }
                else
                {
                    *phCall = gpCallHandleMap->allocHandle(pData) ;
                    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                                  "sipxCallCreateHelper new hCall: %d Call-Id: %s",
                                  *phCall,
                                  pData->callId->data());
                    assert(*phCall != 0) ;
                    sr = SIPX_RESULT_SUCCESS ;

                    // If energy level notifications are turned on
                    if(pInst->nEnergyLevelNotificationPeriodMs > 0)
                    {
                        OsMsgQ tempQueue;
                        // Create and send message to turn on Mic energy notifications
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallCreateHelper: Setting speaker energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                                          tempQueue,
                                                                          pInst->nEnergyLevelNotificationPeriodMs);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallCreateHelper: Setting mic energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                                          tempQueue,
                                                                          pInst->nEnergyLevelNotificationPeriodMs);

                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(pData->callId->data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }
                    else
                    {
                        // get flowgraph queue to post message on
                        OsMsgQ tempQueue;
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallCreateHelper: Disabling speaker energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                            tempQueue);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallCreateHelper: Disabling mic energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                            tempQueue);

                        OsMsg* flowgraphMessage = NULL;
                        tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        while(flowgraphMessage)
                        {
                            pInst->pCallManager->sendFlowgraphMessage(pData->callId->data(), *flowgraphMessage);
                            tempQueue.receive(flowgraphMessage, OsTime::NO_WAIT_TIME);
                        }
                    }
                }
            }
            else
            {
                sr = SIPX_RESULT_OUT_OF_MEMORY ;
                destroyCallData(pData) ;
                *phCall = SIPX_CALL_NULL ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    sipxLineReleaseLock(pLine, SIPX_LOCK_READ, stackLogger);

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        SIPX_CALL*  phCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallCreate");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallCreate hInst=%p hLine=%d phCall=%p",
        hInst, hLine, phCall);

    if (hLine == SIPX_LINE_NULL)
    {
       OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "sipxCallCreate is called with NULL hLine!");
       return SIPX_RESULT_INVALID_ARGS;
    }

    SIPX_RESULT rc = sipxCallCreateHelper(hInst, hLine, SIPX_CONF_NULL, phCall) ;
    if (rc == SIPX_RESULT_SUCCESS)
    {
        SIPX_CALL_DATA* pData = sipxCallLookup(*phCall, SIPX_LOCK_READ, stackLogger);
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
        UtlString callId = *pData->callId ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

        // Notify Listeners
        sipxFireCallEvent(pInst->pCallManager, 
                callId.data(), 
                NULL,
                NULL, 
                CALLSTATE_DIALTONE, 
                CALLSTATE_CAUSE_NORMAL) ;
    }
    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxCallConnect(SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_CONTACT_ID contactId,
                                         SIPX_VIDEO_DISPLAY* const pDisplay,
                                         SIPX_SECURITY_ATTRIBUTES* const pSecurity,
                                         bool bTakeFocus,
                                         SIPX_CALL_OPTIONS* options,
                                         const char* szCallId)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallConnect");
    SIPX_TRANSPORT hTransport = SIPX_TRANSPORT_NULL;
    bool bEnableLocationHeader=false;
    RTP_TRANSPORT rtpTransportOptions = RTP_TRANSPORT_UDP;
    int bandWidth=AUDIO_CODEC_BW_DEFAULT;

    if (options != NULL && options->cbSize)
    {
        bEnableLocationHeader = options->sendLocation;
        bandWidth = options->bandwidthId;

        if (options->cbSize == sizeof(SIPX_CALL_OPTIONS))
        {
            rtpTransportOptions = (RTP_TRANSPORT)options->rtpTransportFlags;
        }
        else
        {
            // try to provide some drop-in compatibility.
            rtpTransportOptions = RTP_TRANSPORT_UDP;
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallConnect hCall=%d szAddress=%s contactId=%d bEnableLocationHeader=%d bandWidth=%d",
        hCall, szAddress, contactId, bEnableLocationHeader, bandWidth);

    // check for and load security runtime .dlls
    if (pSecurity)
    {
        SIPX_RESULT rc = sipxConfigLoadSecurityRuntime();
        if (SIPX_RESULT_SUCCESS != rc)
        {
            return rc;  // return right away 
        }
    }
    if (pDisplay)
    {
        assert(pDisplay->handle);
    }
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    UtlString lineId ;
    bool bSetFocus = false ;
    char* pLocationHeader = NULL;

    assert(szAddress != NULL) ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        SIPX_CONTACT_ADDRESS* pContact = NULL;
        SIPX_CONTACT_TYPE contactType;

        if (pInst && pSecurity)
        {
            // augment the security attributes with the instance's security parameters
            SecurityHelper securityHelper;
            // generate a random key, if one isn't there
            if (strlen(pSecurity->getSrtpKey()) == 0)
            {
                securityHelper.generateSrtpKey(*pSecurity);
            }
            if (pInst->dbLocation)
            {
                securityHelper.setDbLocation(*pSecurity, pInst->dbLocation);
            }
            if (pInst->myCertNickname)
            {
                securityHelper.setMyCertNickname(*pSecurity, pInst->myCertNickname);
            }
            if (pInst->dbPassword)
            {
                securityHelper.setDbPassword(*pSecurity, pInst->dbPassword);
            }
        }        
        if (contactId > 0)
        {
            pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
            assert(pContact);
            contactType = pContact->eContactType;
            if (pContact->eTransportType > 3)
            {
                hTransport = pContact->eTransportType;
            }

            OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                    "contactId(%d): type: %s, transport: %s interface: %s, ip: %s:%d",
                    contactId,
                    sipxContactTypeToString((SIPX_CONTACT_TYPE) pContact->eContactType),
                    sipxTransportTypeToString((SIPX_TRANSPORT_TYPE) pContact->eTransportType),
                    pContact->cInterface,
                    pContact->cIpAddress,
                    pContact->iPort) ;
        }
        else
        {
            contactType = CONTACT_AUTO;
        }
        
        if (szAddress)
        {
            PtStatus status ;
            assert(remoteAddress.length() == 0) ;    // should be null

            if (!sipxIsCallInFocus() || bTakeFocus)
            {
                pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;
                bSetFocus = true ;
            }

            pInst->pCallManager->setOutboundLineForCall(callId.data(),
                    lineId.data()) ;

            UtlString sessionId(szCallId) ;
            if (sessionId.length() < 1)
            {
                pInst->pCallManager->getNewSessionId(&sessionId) ;
            }
            SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            if (pData)
            {
                pData->bInFocus = bSetFocus ;
                assert(pData->sessionCallId == NULL) ; // should be null
                if (pData->sessionCallId != NULL)
                {
                    delete pData->sessionCallId ;
                }
                pData->sessionCallId = new UtlString(sessionId.data()) ;
                
                if (pDisplay)
                {
                    pData->display = *pDisplay;
                }
                if (pSecurity)
                {
                    pData->security = *pSecurity;
                }

                pData->hTransport = hTransport;
            }

            SIPX_SECURITY_ATTRIBUTES* pTempSecurity = NULL;
            if (pSecurity)
            {
                pTempSecurity = &pData->security;
            }
            if (pInst)
            {
                pLocationHeader = (bEnableLocationHeader) ? pInst->szLocationHeader : NULL;
            }

            SIPX_TRANSPORT_DATA* pTransportDataCopy = NULL;
            if (SIPX_TRANSPORT_NULL != hTransport)
            {
                SIPX_TRANSPORT_DATA* pTransportData = NULL;
                pTransportData = sipxTransportLookup(hTransport, SIPX_LOCK_READ);
                pTransportDataCopy = new SIPX_TRANSPORT_DATA(*pTransportData);
                sipxTransportReleaseLock(pTransportData, SIPX_LOCK_READ);
            }

            if (pDisplay && pDisplay->handle)
            {
                status = pInst->pCallManager->connect(callId.data(), szAddress, NULL, sessionId, (SIPX_CONTACT_ID) contactId, 
                                                      &pData->display, pTempSecurity, pLocationHeader, bandWidth, pTransportDataCopy, rtpTransportOptions) ;
            }
            else
            {
                status = pInst->pCallManager->connect(callId.data(), szAddress, NULL, sessionId, (SIPX_CONTACT_ID) contactId, 
                                                      NULL, pTempSecurity, pLocationHeader, bandWidth, pTransportDataCopy, rtpTransportOptions) ;
            }
            delete pTransportDataCopy;
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
            if (status == PT_SUCCESS)
            {
                int numAddresses = 0 ;
                UtlString address ;
                OsStatus rc = pInst->pCallManager->getCalledAddresses(
                        callId.data(), 1, numAddresses, &address) ;
                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                              "sipxCallConnect connected hCall=%d callId=%s, numAddr = %d, addr = %s rc = %d",
                              hCall, callId.data(), numAddresses, address.data(), rc);
                assert(rc == OS_SUCCESS) ;
                assert(numAddresses == 1) ;

                // Set Remote Connection
                SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
                if (pData)
                {
                    if (pData->remoteAddress)
                    {
                        delete pData->remoteAddress ;
                    }
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
                    sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
                }
                // If energy level notifications are turned on
                if(pInst->nEnergyLevelNotificationPeriodMs > 0)
                {
                    // get flowgraph queue to post message on
                    OsMsgQ* flowgraphQueue = sipxCallGetMediaConrolQueue(hCall);
                    assert(flowgraphQueue);
                    if(flowgraphQueue)
                    {
                        // Create and send message to turn on Mic energy notifications
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallConnect: Setting speaker energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                                          *flowgraphQueue,
                                                                          pInst->nEnergyLevelNotificationPeriodMs);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallConnect: Setting mic energy notificiation: %s period: %d",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                      pInst->nEnergyLevelNotificationPeriodMs);
                        MprVoiceActivityNotifier::chageNotificationPeriod(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                                          *flowgraphQueue,
                                                                          pInst->nEnergyLevelNotificationPeriodMs);
                    }
                }
                else
                {
                    // get flowgraph queue to post message on
                    OsMsgQ* flowgraphQueue = sipxCallGetMediaConrolQueue(hCall);
                    assert(flowgraphQueue);
                    if(flowgraphQueue)
                    {
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallConnect: Disabling speaker energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX,
                                                            *flowgraphQueue);
                        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
                                      "sipxCallConnect: Disabling mic energy notificiation: %s",
                                      DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX);
                        MpResource::setNotificationsEnabled(FALSE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX,
                                                            *flowgraphQueue);
                    }
                }
            }
            else
            {
                sipxFireCallEvent(pInst->pCallManager, 
                        callId.data(), 
                        NULL,
                        szAddress, 
                        CALLSTATE_DISCONNECTED, 
                        CALLSTATE_CAUSE_BAD_ADDRESS) ;   
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


SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall,
                                      bool  bStopRemoteAudio)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallHold");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallHold hCall=%d bStopRemoteAudio=%d",
        hCall, bStopRemoteAudio);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress;
    
    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
        if (  pCallData
           && pCallData->state == SIPX_INTERNAL_CALLSTATE_HELD
           && !pCallData->bHoldAfterConnect)
        {
            sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
            sr = SIPX_RESULT_INVALID_STATE;
        } 
        else
        {
            sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
            SIPX_CONF hConf = sipxCallGetConf(hCall) ;
            if (hConf == SIPX_CONF_NULL)
            {            
                if (bStopRemoteAudio)
                {
                    pInst->pCallManager->holdTerminalConnection(callId.data(), remoteAddress.data(), 0) ;
                }
                pInst->pCallManager->holdLocalTerminalConnection(callId.data()) ;
            }
            else
            {
                pInst->pCallManager->holdTerminalConnection(callId.data(), remoteAddress.data(), 0) ;
            }
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    
    
    if (SIPX_RESULT_SUCCESS == sr)
    {
        SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, 
                stackLogger);
        if (pCallData)
        {
            pCallData->bCallHoldInvoked = true;
            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallUnhold");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallUnhold hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        SIPX_CONF hConf = sipxCallGetConf(hCall) ;
        if (hConf == SIPX_CONF_NULL)
        {
            SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            if (pCallData && pCallData->state != SIPX_INTERNAL_CALLSTATE_HELD)
            {
                sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
                sr = SIPX_RESULT_INVALID_STATE;
            }
            else if (pCallData)
            {
                pCallData->bInFocus = true ;
                sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
            }
            pInst->pCallManager->unholdTerminalConnection(callId.data(), remoteAddress, NULL);        
            pInst->pCallManager->unholdLocalTerminalConnection(callId.data()) ;
        }
        else
        {
            pInst->pCallManager->unholdTerminalConnection(callId.data(), remoteAddress, NULL);                    
        }
        sr = SIPX_RESULT_SUCCESS ;
    }
    if (SIPX_RESULT_SUCCESS == sr)
    {
        SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
        if (pCallData)
        {
            pCallData->bCallHoldInvoked = false;
            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL& hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallDestroy");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallDestroy hCall=%d",
        hCall);

    SIPX_CONF hConf = sipxCallGetConf(hCall) ;
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteUrl ;
    UtlString ghostCallId ;

    if (hConf != 0)
    {
        sr = sipxConferenceRemove(hConf, hCall) ;
    }
    else if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteUrl, NULL, &ghostCallId))
    {
        UtlBoolean bRemoveInsteadOfDrop = sipxCallIsRemoveInsteadOfDropSet(hCall);
        SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

        if (pData && pData->state != SIPX_INTERNAL_CALLSTATE_DESTROYING)
        {
            // I'm not sure why this check was here - likely it was because
            // of some GIPS VE quirks. But with sipXmediaLib we don't need
            // this, so I remove it. This check makes it much more likely to
            // have non-cleaned up calls if something goes wrong.
//            if (false == pData->bTonePlaying && pData->nFilesPlaying <= 0)
            {
                pData->state = SIPX_INTERNAL_CALLSTATE_DESTROYING;
                sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
                pData = NULL;
                if (bRemoveInsteadOfDrop)
                {
                    pInst->pCallManager->dropConnection(callId, remoteUrl) ;
                }
                else
                {
                    pInst->pCallManager->drop(callId.data()) ;
                    if (ghostCallId.length() > 0)
                    {
                        pInst->pCallManager->drop(ghostCallId.data()) ;
                    }
                    sr = SIPX_RESULT_SUCCESS ;
                }
                // If not remote url, then the call was never completed and no
                // connection object exists (and no way to send an event) -- remove
                // the call handle.
                if (remoteUrl.length() == 0)
                {
                    // TODO:: This should fire a destroy event
                    sipxCallObjectFree(hCall, stackLogger);
                }
            }
        }
        else
        {
            sr = SIPX_RESULT_BUSY;
        }
        if (pData)
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }
        
    }
    else
    {
        // Not finding the call is ok (torn down) providing
        // that the handle is valid.
        if (hCall != SIPX_CALL_NULL)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }

    if (SIPX_RESULT_SUCCESS == sr)
    {
        hCall = SIPX_CALL_NULL ;
    }
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetID(const SIPX_CALL hCall,
                                       char* szId,
                                       const size_t iMaxLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetID");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetID hCall=%d",
        hCall);

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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetLocalID");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetLocalID hCall=%d",
        hCall);

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

SIPXTAPI_API SIPX_RESULT sipxCallGetLine(const SIPX_CALL hCall,
                                        SIPX_LINE& hLine)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetLine");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxCallGetLine hCall=%d", hCall);

    SIPX_RESULT result = SIPX_RESULT_INVALID_ARGS;
    if(hCall != SIPX_CALL_NULL)
    {
        SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
        if(pData && pData->hLine != SIPX_LINE_NULL)
        {
            result = SIPX_RESULT_SUCCESS;
            hLine = pData->hLine;
        }
        sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
    }

    return(result);
}


SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteID(const SIPX_CALL hCall,
                                             char* szId,
                                             const size_t iMaxLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRemoteID");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetRemoteID hCall=%d",
        hCall);

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

SIPXTAPI_API SIPX_RESULT sipxCallGetContactID(const SIPX_CALL hCall,
                                              char* szId,
                                              const size_t iMaxLength)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetContactID hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    UtlString callId ;
    UtlString contactAddress ;

    if (sipxCallGetCommonData(hCall, NULL, &callId, NULL, NULL, NULL, &contactAddress))
    {
        if (iMaxLength)
        {
            strncpy(szId, contactAddress.data(), iMaxLength) ;
            szId[iMaxLength-1] = 0 ;
        }
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionId(const SIPX_CALL hCall,
                                                 int& connectionId)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetConnectionId");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    connectionId = -1;
    
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   
    if (pData)        
    {
        if (pData->connectionId != -1)
        {
            connectionId = pData->connectionId;
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

            sr = SIPX_RESULT_SUCCESS;
        }
        else
        {
            if (pData->pInst && pData->pInst->pCallManager && pData->callId && 
                    pData->remoteAddress)
            {
                CallManager* pCallManager = pData->pInst->pCallManager ;
                UtlString callId(*pData->callId) ;
                UtlString remoteAddress(*pData->remoteAddress) ;

                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

                connectionId = pCallManager->getMediaConnectionId(callId, remoteAddress);

                pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
                pData->connectionId = connectionId;
                sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;
                               
                if (-1 != connectionId)
                {
                    sr = SIPX_RESULT_SUCCESS;
                }           
            }
            else
            {
                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
            }    
        }
    }

    return sr;
}                                                 

SIPXTAPI_API SIPX_RESULT sipxCallGetConference(const SIPX_CALL hCall,
                                               SIPX_CONF&      hConf) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetConference");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetConference hCall=%d", hCall);

    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS ;
    if (hCall != SIPX_CALL_NULL)
    {
        hConf = sipxCallGetConf(hCall) ;
        if (hConf != SIPX_CONF_NULL)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_FAILURE ;
        }
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallGetRequestURI(const SIPX_CALL hCall,
                                               char* szUri,
                                               const size_t iMaxLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRequestURI");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetRequestURI hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
    
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId &&
            pData->remoteAddress) 
        {
            CallManager* pCallManager = pData->pInst->pCallManager ;
            UtlString callId(*pData->callId) ;
            UtlString remoteAddress(*pData->remoteAddress) ;

            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

            SipDialog sipDialog;
            pCallManager->getSipDialog(callId, remoteAddress, sipDialog);
            
            UtlString uri;
            sipDialog.getRemoteRequestUri(uri);
            if (iMaxLength)
            {
                strncpy(szUri, uri.data(), iMaxLength) ;
                szUri[iMaxLength-1] = 0 ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }    
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteContact(const SIPX_CALL hCall,
                                                  char* szContact,
                                                  const size_t iMaxLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRemoteContact");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetRemoteContact hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
        
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId && 
                pData->remoteAddress)
        {
            CallManager* pCallManager = pData->pInst->pCallManager ;
            UtlString callId(*pData->callId) ;
            UtlString remoteAddress(*pData->remoteAddress) ;

            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

            SipDialog sipDialog;
            pCallManager->getSipDialog(callId, remoteAddress, sipDialog);
            
            Url contact;
            sipDialog.getRemoteContact(contact);

            if (iMaxLength)
            {
                strncpy(szContact, contact.toString().data(), iMaxLength) ;
                szContact[iMaxLength-1] = 0 ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }    
    }


    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteUserAgent(const SIPX_CALL hCall,
                                                    char* szAgent,
                                                    const size_t iMaxLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRemoteUserAgent");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetRemoteUserAgent hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
        
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId && 
                pData->remoteAddress)
        {
            CallManager* pCallManager = pData->pInst->pCallManager ;
            UtlString callId(*pData->callId) ;
            UtlString remoteAddress(*pData->remoteAddress) ;
            UtlString userAgent;

            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
       
            pCallManager->getRemoteUserAgent(callId, remoteAddress, userAgent);
       
            if (iMaxLength)
            {
                strncpy(szAgent, userAgent.data(), iMaxLength) ;
                szAgent[iMaxLength-1] = 0 ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }    
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallGetInviteHeader(const SIPX_CALL hCall, 
                                                 const char* headerName, 
                                                 char* headerValue, 
                                                 const size_t maxValueLength,
                                                 bool* inviteFromRemote,
                                                 const size_t headerInstanceIndex)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetInviteHeader");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallGetInviteHeader hCall=%d, header=\"%s\", maxValueLenght=%d, headerInstanceIndex=%d",
        hCall,
        headerName,
        (int)maxValueLength,
        (int)headerInstanceIndex);

    SIPX_RESULT sipxReturn = SIPX_RESULT_INVALID_STATE;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
        
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId && 
                pData->remoteAddress)
        {
            CallManager* pCallManager = pData->pInst->pCallManager;
            UtlString callId(*pData->callId);
            UtlString remoteAddress(*pData->remoteAddress);
            UtlString headerValueString;
            UtlBoolean inviteIsFromRemote;

            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
       
            OsStatus cpReturn = 
                pCallManager->getInviteHeaderValue(callId, remoteAddress, 
                                                   headerName, headerValueString,
                                                   inviteIsFromRemote,
                                                   headerInstanceIndex);

            if(inviteIsFromRemote)
            {
                *inviteFromRemote = true;
            }
            else
            {
                *inviteFromRemote = false;
            }
       
            if (maxValueLength)
            {
                strncpy(headerValue, headerValueString.data(), maxValueLength);
                headerValue[maxValueLength - 1] = 0;

                switch(cpReturn)
                {
                case OS_SUCCESS:
                    sipxReturn = SIPX_RESULT_SUCCESS;
                    break;

                case OS_NOT_FOUND:
                    sipxReturn = SIPX_RESULT_FAILURE;
                    break;

                case OS_INVALID_STATE:
                    sipxReturn = SIPX_RESULT_INVALID_STATE;
                    break;

                default:
                    sipxReturn = SIPX_RESULT_FAILURE;
                    break;

                }
            }
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
            OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                "sipxCallGetInviteHeader hCall=%d inst=%p callMgr: %p callId: %s remoteAddr: %s",
                hCall,
                pData->pInst,
                pData->pInst->pCallManager,
                pData->callId ? pData->callId->data() : "",
                pData->remoteAddress ? pData->remoteAddress->data() : "");
        }    
    }
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxCallGetInviteHeader hCall=%d info not found",
            hCall);
    }

    return(sipxReturn);
}

SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall,
                                           const SIPX_TONE_ID toneId,
                                           const bool bLocal,
                                           const bool bRemote)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStartTone");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallStartTone hCall=%d ToneId=%d bLocal=%d bRemote=%d",
        hCall, toneId, bLocal, bRemote);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    SIPX_TONE_ID xlateId;

    if (sipxTranslateToneId(toneId, xlateId) == SIPX_RESULT_SUCCESS)
    {
        if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
        {
            SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            
            if (pData)
            {
               /*  we no longer need to do this, we are not allowing call teardown while a tone is playing

                gpCallHandleMap->addHandleRef(hCall);  // Add a handle reference, so that
                                                    // if the call ends before
                                                    // the tone is stopped,
                                                    // there is still a valid call handle
                                                    // to use with stopTone
               */                                                        
                pData->bTonePlaying = true;
                sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
                
                pInst->pCallManager->toneChannelStart(callId, remoteAddress, xlateId, bLocal, bRemote) ;
                sr = SIPX_RESULT_SUCCESS ;

                if (!pInst->toneStates.bInitialized)
                {
                    pInst->toneStates.bInitialized = true;
                }
            }     
        }
    }
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStopTone");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallStopTone hCall=%d",
        hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    SIPX_CALL_DATA* pData = NULL;
    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        pData =  sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
        if (pData && pData->bTonePlaying && pInst->toneStates.bInitialized )
        {
      
            pInst->pCallManager->toneChannelStop(callId, remoteAddress) ;
            /*  we no longer need to do this, we are not allowing call teardown while a file is playing
            sipxCallObjectFree(hCall);
            */        
            sr = SIPX_RESULT_SUCCESS ;
            pData->bTonePlaying = false;
        }
    }
    
    if (pData)
    {
        sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
    }
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStart(const SIPX_CALL hCall,
                                                    const char* szFile,
                                                    const bool bRepeat,
                                                    const bool bLocal,
                                                    const bool bRemote,
                                                    const bool bMixWithMicrophone,
                                                    const float fVolumeScaling) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlayFileStart");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioPlayFileStart hCall=%d File=%s bLocal=%d bRemote=%d bRepeat=%d bMixWithMic=%d, volScale=%f",
        hCall, szFile, bLocal, bRemote, bRepeat, bMixWithMicrophone, fVolumeScaling);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    // Massage volume scaling into range
    float fDownScaling = fVolumeScaling ;
    if (fDownScaling > 1.0)
    {
        fDownScaling = 1.0 ;
    }
    else if (fDownScaling < 0.0)
    {
        fDownScaling = 0.0 ;
    }

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        if (szFile)
        {
            SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
    
            if (pData)
            {    
            /*  we no longer need to do this, we are not allowing call teardown while a file is playing
            
                gpCallHandleMap->addHandleRef(hCall);  // Add a handle reference, so that
                                                    // if the call ends before
                                                    // the file play is stopped,
                                                    // there is still a valid call handle
                                                    // to use with PlayFileStop
            */                                                    
                pData->nFilesPlaying++;
                sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
            }            
            pInst->pCallManager->audioChannelPlay(callId, remoteAddress, szFile, bRepeat, bLocal, bRemote, bMixWithMicrophone, (int) (fDownScaling * 100.0)) ;
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStop(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlayFileStop");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioPlayFileStop hCall=%d", hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
    
        SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
        
        if (pData)
        {
            if (pData->nFilesPlaying)
            {
                pInst->pCallManager->audioChannelStop(callId, remoteAddress) ;
                pData->nFilesPlaying--;
            /*  we no longer need to do this, we are not allowing call teardown while a file is playing
                sipxCallObjectFree(hCall);
             */
                sr = SIPX_RESULT_SUCCESS ;
            }
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStart(const SIPX_CALL hCall,
                                                      const char* szFile,
                                                      const SIPX_AUDIO_FILE_FORMAT recordFormat,
                                                      const bool appendToFile,
                                                      const int numChannels) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStart");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioRecordFileStart hCall=%d szFile=%s recordFormat: %d append: %s channels: %d", 
        hCall, szFile, recordFormat,
        appendToFile ? "TRUE" : "FALSE",
        numChannels);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    CpMediaInterface::CpAudioFileFormat cpRecordFormat = CpMediaInterface::CP_UNKNOWN_FORMAT;
    switch(recordFormat)
    {
        case SIPX_WAVE_PCM_16:
            cpRecordFormat = CpMediaInterface::CP_WAVE_PCM_16;
            break;

        case SIPX_WAVE_GSM:
            cpRecordFormat = CpMediaInterface::CP_WAVE_GSM;
            break;

        case SIPX_OGG_OPUS:
            cpRecordFormat = CpMediaInterface::CP_OGG_OPUS;
            break;

        default:
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                    "Invalid record format: %d",
                    recordFormat);
            OsSysLog::flush();
            assert(0);
            break;
    }

    if (szFile && 
        cpRecordFormat != CpMediaInterface::CP_UNKNOWN_FORMAT &&
        sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {

        if (pInst->pCallManager->audioChannelRecordStart(callId, 
                                                         remoteAddress, 
                                                         szFile, 
                                                         cpRecordFormat,
                                                         appendToFile ? TRUE : FALSE,
                                                         numChannels))
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return(sr);
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordPause(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordPause");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioRecordPause hCall=%d", hCall);

    SIPX_RESULT status = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA *pInst;
    UtlString callId;
    UtlString remoteAddress;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        if (pInst->pCallManager->audioChannelRecordPause(callId, remoteAddress))
        {
            status = SIPX_RESULT_SUCCESS;
        }
    }
    else
    {
        status = SIPX_RESULT_INVALID_ARGS;
    }

    return(status);
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordResume(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordResume");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioRecordResume hCall=%d", hCall);

    SIPX_RESULT status = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA *pInst;
    UtlString callId;
    UtlString remoteAddress;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        if (pInst->pCallManager->audioChannelRecordResume(callId, remoteAddress))
        {
            status = SIPX_RESULT_SUCCESS;
        }
    }
    else
    {
        status = SIPX_RESULT_INVALID_ARGS;
    }

    return(status);
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStop(const SIPX_CALL hCall) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStop");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioRecordFileStop hCall=%d", hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        if (pInst->pCallManager->audioChannelRecordStop(callId, remoteAddress))
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordBufferStart(const SIPX_CALL hCall,
                                                        const char* pBuffer,
                                                        const int bufferSize,
                                                        const int bufferType,
                                                        const int maxRecordTime,
                                                        const int maxSilence)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordBufferStart");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallAudioRecordBufferStart hCall=%d pBuffer=%p bufferSize=%d "
        "bufferType=%d maxRecordTime=%d maxSilence=%d", hCall, pBuffer, bufferSize,
        bufferType, maxRecordTime, maxSilence);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (pBuffer && bufferSize && sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {        
        if (pInst->pCallManager->audioChannelRecordBufferStart(callId, remoteAddress,
                                 pBuffer, bufferSize, bufferType, maxRecordTime, maxSilence))
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordBufferStop(const SIPX_CALL hCall) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStop");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                  "sipxCallAudioRecordFileStop hCall=%d", hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        if (pInst->pCallManager->audioChannelRecordBufferStop(callId, remoteAddress))
        {
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStart(const SIPX_CALL hCall,
                                                 const char* szBuffer,
                                                 const int  bufSize,
                                                 const int  bufType,
                                                 const bool bRepeat,
                                                 const bool bLocal,
                                                 const bool bRemote)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallPlayBufferStart");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallPlayBufferStart hCall=%d Buffer=%p Size=%d Type=%d bLocal=%d bRemote=%d bRepeat=%d",
        hCall, szBuffer, bufSize, bufType, bLocal, bRemote, bRepeat);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
        
        if (pData)
        {
            pData->nFilesPlaying++;
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }
        if (szBuffer)
        {
            pInst->pCallManager->bufferPlay(callId.data(), (const void*)szBuffer, bufSize, bufType, bRepeat, bLocal, bRemote) ;
            sr = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;

        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStop(const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallPlayBufferStop");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallPlayBufferStop hCall=%d", hCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        int nFilesPlaying = 0;
        
        SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
        if (pData)
        {
            nFilesPlaying = pData->nFilesPlaying;
            pData->nFilesPlaying--;
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }
        if (nFilesPlaying > 0)
        {
            pInst->pCallManager->audioStop(callId.data()) ;
            sr = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;

    }

    return sr ;
}

CpMediaInterface::MEDIA_STREAM_TYPE convertToCpMediaInterfaceMediaType(MEDIA_TYPE mediaType)
{
    CpMediaInterface::MEDIA_STREAM_TYPE cpMediaType = CpMediaInterface::MEDIA_TYPE_UNKNOWN;
    switch(mediaType)
    {
    case AUDIO_MEDIA:
        cpMediaType = CpMediaInterface::AUDIO_STREAM;
    break;

    case VIDEO_MEDIA:
        cpMediaType = CpMediaInterface::VIDEO_STREAM;
    break;

    default:
       assert(mediaType == VIDEO_MEDIA);
    break;
    }

    return(cpMediaType);
}

SIPXTAPI_API SIPX_RESULT sipxCallSetMediaPassThrough(const SIPX_CALL hCall,
                                                     MEDIA_TYPE mediaType,
                                                     int mediaTypeStreamIndex,
                                                     const char* streamReceiveAddress,
                                                     int rtpPort,
                                                     int rtcpPort)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSetMediaPassThrough");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallSetMediaPassThrough hCall=%d mediaType=%d mediaTypeStreamIndex=%d streamReceiveAddress=\"%s\" rtpPort=%d rtcpPort=%d",
        hCall, mediaType, mediaTypeStreamIndex, streamReceiveAddress, rtpPort, rtcpPort);

    SIPX_RESULT sipxResult = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA *pInst;
    UtlString callId;
    UtlString remoteAddress;
    UtlString lineId;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        assert(mediaType == VIDEO_MEDIA);
        CpMediaInterface::MEDIA_STREAM_TYPE cpMediaType = convertToCpMediaInterfaceMediaType(mediaType);

        OsStatus osStatus = 
            pInst->pCallManager->setMediaPassThrough(callId, remoteAddress, cpMediaType, mediaTypeStreamIndex,
                                                     streamReceiveAddress, rtpPort, rtcpPort);

        if(osStatus == OS_SUCCESS)
        {
            sipxResult = SIPX_RESULT_SUCCESS;
        }
    }

    return(sipxResult);
}

SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub,
                                           bool bRemoteContactIsGruu)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSubscribe");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallSubscribe hCall=%d szEventType=\"%s\" szAcceptType=\"%s\"", 
        hCall,
        szEventType ? szEventType : "<null>",
        szAcceptType ? szAcceptType : "<null>");

    SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA *pInst;
    UtlString callId;
    UtlString remoteAddress;
    UtlString lineId;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        SIPX_SUBSCRIPTION_DATA* subscriptionData = new SIPX_SUBSCRIPTION_DATA;
        subscriptionData->pDialogHandle = new UtlString;
        subscriptionData->pMutex =  new OsRWMutex(OsRWMutex::Q_FIFO) ;
        subscriptionData->pInst = pInst;
        *phSub = gpSubHandleMap->allocHandle(subscriptionData);
        int subscriptionPeriod = 3600;

        // Need to get the resourceId, To, From and Contact from
        // the associated call
        SipSession session;
        if(pInst->pCallManager->getSession(callId, remoteAddress, session))
        {
            Url toUrl;
            session.getToUrl(toUrl);
            toUrl.removeFieldParameters();
            UtlString toField;
            toUrl.toString(toField);
            UtlString contactField;
            Url contactUrl;
            session.getLocalContact(contactUrl);
            contactUrl.toString(contactField);

            // If remoteContactIsGruu is set we assume the
            // remote contact is a globally routable unique URI (GRUU).
            // Normally one cannot assume the Contact header is a
            // publicly addressable address and we assume that the
            // To or From from the remote side has a better chance of
            // being globally routable as it typically is an address
            // of record (AOR).
            UtlString resourceId;
            Url requestUri;
            if(bRemoteContactIsGruu)
            {
                requestUri = contactUrl;
            }
            else
            {
                requestUri = toUrl;
            }

            requestUri.removeFieldParameters();
            requestUri.removeUrlParameters();
            requestUri.toString(resourceId);

            // May need to get the from field from the line manager
            // For now, use the From in the session
            UtlString fromField;
            Url fromUrl;
            session.getFromUrl(fromUrl);
            fromUrl.removeFieldParameters();
            fromUrl.toString(fromField);

            UtlBoolean sessionDataGood = TRUE;
            if(resourceId.length() <= 1 ||
                fromField.length() <= 4 ||
                toField.length() <= 4 ||
                contactField.length() <= 4)
            {
                sessionDataGood = FALSE;
                OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                    "sipxCallSubscribe bad session data: hCall=%d szEventType=\"%s\" szAcceptType=\"%s\" resourceId=\"%s\" From=\"%s\" To=\"%s\" Contact=\"%s\"", 
                    hCall,
                    szEventType ? szEventType : "<null>",
                    szAcceptType ? szAcceptType : "<null>",
                    resourceId.data(),
                    fromField.data(),
                    toField.data(),
                    contactField.data());
            }

            // Session data is big enough
            else
            {
                OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                    "sipxCallSubscribe subscribing: hCall=%d szEventType=\"%s\" szAcceptType=\"%s\" resourceId=\"%s\" From=\"%s\" To=\"%s\" Contact=\"%s\"", 
                    hCall,
                    szEventType ? szEventType : "<null>",
                    szAcceptType ? szAcceptType : "<null>",
                    resourceId.data(),
                    fromField.data(),
                    toField.data(),
                    contactField.data());
            }

            // Subscribe and keep the subscription refreshed
            if(sessionDataGood &&
               pInst->pSubscribeClient->addSubscription(resourceId, 
                                                        szEventType, 
                                                        szAcceptType,
                                                        fromField, 
                                                        toField, 
                                                        contactField,                                                         
                                                        subscriptionPeriod, 
                                                        (void*)*phSub, 
                                                        sipxSubscribeClientSubCallback,
                                                        sipxSubscribeClientNotifyCallback, 
                                                        *(subscriptionData->pDialogHandle)))
            {
                sipXresult = SIPX_RESULT_SUCCESS;
            }
        }

        // Could not find session for given call handle
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                "sipxCallSubscribe: could not get session for call handle: %d callId: %s remote address: %s",
                hCall, callId.data(), remoteAddress.data());
            sipXresult = SIPX_RESULT_INVALID_ARGS;
        }
    }
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                      "sipxCallSubscribe: could not find call data for call handle: %d",
                      hCall);
        sipXresult = SIPX_RESULT_INVALID_ARGS;

    }

    return sipXresult;
}

// CHECKED
SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallUnsubscribe");
   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallSubscribe hSub=%lx", hSub);

   gSubscribeAccessLock.acquire(); // we need global lock to delete mutex
   SIPX_SUBSCRIPTION_DATA* subscriptionData = sipxSubscribeLookup(hSub, SIPX_LOCK_WRITE, stackLogger);

   if(subscriptionData)
   {
      assert(subscriptionData->pInst); // this should never happen
      assert(subscriptionData->pDialogHandle);
      UtlString dialogHandle(*(subscriptionData->pDialogHandle));
      SIPX_INSTANCE_DATA *pInst = subscriptionData->pInst;

      // Remove and free up the subscription handle and data
      gpSubHandleMap->removeHandle(hSub);
      gSubscribeAccessLock.release(); // we can release global lock now

      delete subscriptionData->pDialogHandle;
      subscriptionData->pDialogHandle = NULL;
      delete subscriptionData->pMutex;
      subscriptionData->pMutex = NULL;
      delete subscriptionData;

      if(pInst->pSubscribeClient->endSubscription(dialogHandle))
      {
         sipXresult = SIPX_RESULT_SUCCESS;
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxCallUnsubscribe endSubscription failed for subscription handle: %ld dialog handle: \"%s\"",
            hSub,
            dialogHandle.data());
         sipXresult = SIPX_RESULT_INVALID_ARGS;
      }
   }
   else  // Invalid subscription handle, possibly already deleted
   {
      gSubscribeAccessLock.release();
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
         "sipxCallUnsubscribe: cannot find subscription data for handle: %ld",
         hSub);
      sipXresult = SIPX_RESULT_INVALID_ARGS;
      // no other lock to release, since handle wasn't found
   }

   return(sipXresult);
}

SIPXTAPI_API SIPX_RESULT sipxConfigSubscribe(const SIPX_INST hInst, 
                                             const SIPX_LINE hLine, 
                                             const char* szTargetUrl, 
                                             const char* szEventType, 
                                             const char* szAcceptType, 
                                             const SIPX_CONTACT_ID contactId, 
                                             SIPX_SUB* phSub) 
{ 
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSubscribe");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, 
        "sipxConfigSubscribe hInst=%p szTargetUrl=\"%s\" szEventType=\"%s\" szAcceptType=\"%s\"", 
        hInst, 
        szTargetUrl ?  szTargetUrl  : "<null>", 
        szEventType ? szEventType : "<null>", 
        szAcceptType ? szAcceptType : "<null>"); 
    UtlString sTargetUrl(szTargetUrl);        

    SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE; 
    if (hInst) 
    { 
        SIPX_INSTANCE_DATA *pInst = (SIPX_INSTANCE_DATA*)hInst; 

        SIPX_SUBSCRIPTION_DATA* subscriptionData = new SIPX_SUBSCRIPTION_DATA; 
        subscriptionData->pDialogHandle = new UtlString; 
        subscriptionData->pInst = pInst; 
        subscriptionData->pMutex =  new OsRWMutex(OsRWMutex::Q_FIFO);
        *phSub = gpSubHandleMap->allocHandle(subscriptionData); 
        int subscriptionPeriod = 3600; 

        // Need to get the resourceId, To, From and Contact from 
        // the associated call 
        UtlString resourceId(sTargetUrl); 
        UtlString fromField; 
        UtlString toField(sTargetUrl); 
        UtlString contactField; 
        SIPX_LINE_DATA* pLineData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger) ; 

        if(pLineData) 
        { 
            pLineData->lineURI->getIdentity(fromField); 

            // build a contact field 
            UtlString userId(""); 

            SIPX_CONTACT_ADDRESS* pContact = pInst->pSipUserAgent->getContactDb().find(contactId); 
            if (pContact) 
            { 
                contactField.append("sip:"); 
                pLineData->lineURI->getUserId(userId); 
                contactField.append(userId); 
                contactField.append("@"); 
                contactField.append(pContact->cIpAddress); 
                char szPort[32]; 
                sprintf(szPort, ":%d", pContact->iPort); 
                contactField.append(szPort); 
                if (pContact->cCustomTransportName && pContact->cCustomTransportName[0])
                {
                    Url target(szTargetUrl);
                    UtlString temp;
                    target.setUrlParameter("transport", pContact->cCustomTransportName);
                    target.toString(sTargetUrl);
                }
            } 
            else 
            { 
                contactField.append(fromField); 
            } 

            sipxLineReleaseLock(pLineData, SIPX_LOCK_READ, stackLogger) ; 

            if(resourceId.length() <= 1 || 
                fromField.length() <= 4 || 
                toField.length() <= 4 || 
                contactField.length() <= 4) 
            { 
                OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
                    "sipxConfigSubscribe bad parameters"); 
            } 
            // Session data is big enough 
            else 
            { 
                    // Subscribe and keep the subscription refreshed 
                    if(pInst->pSubscribeClient->addSubscription(resourceId, 
                                                                szEventType, 
                                                                szAcceptType,
                                                                fromField, 
                                                                toField, 
                                                                contactField, 
                                                                subscriptionPeriod, 
                                                                (void*)*phSub, 
                                                                sipxSubscribeClientSubCallback, 
                                                                sipxSubscribeClientNotifyCallback, 
                                                                *(subscriptionData->pDialogHandle))) 
                    { 
                        sipXresult = SIPX_RESULT_SUCCESS; 
                    } 

            } 
        } 

    } // if (hInst) 

    return sipXresult; 
} 
    
SIPXTAPI_API SIPX_RESULT sipxConfigUnsubscribe(const SIPX_SUB hSub) 
{ 
    return sipxCallUnsubscribe(hSub); 
} 

SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall,
                                               const char* pszAddress)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallBlindTransfer");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallBlindTransfer hCall=%d Address=%s",
        hCall, pszAddress);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        if (pszAddress)
        {
            UtlString ghostCallId ;
            
            // first, get rid of any existing ghost connection
            SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            if (pData && pData->ghostCallId)
            {
                if (pData->ghostCallId->length() > 0)
                {
                    pInst->pCallManager->drop(pData->ghostCallId->data()) ;
                }
                delete pData->ghostCallId;
                pData->ghostCallId = NULL;
            }
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;

            // call the transfer
            pInst->pCallManager->transfer_blind(callId.data(), pszAddress, &ghostCallId) ;
            
            pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
            if (pData)
            {   
                // set the new ghost call Id
                pData->ghostCallId = new UtlString(ghostCallId);
                
                sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;            
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



SIPXTAPI_API SIPX_RESULT sipxCallTransfer(const SIPX_CALL hSourceCall,
                                          const SIPX_CALL hTargetCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallTransfer");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallTransfer hSourceCall=%d hTargetCall=%d\n",
        hSourceCall, hTargetCall);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString sourceCallId;
    UtlString sourceAddress;    
    UtlString targetCallId ;
    UtlString targetAddress;
	UtlString lineId;

    if (    sipxCallGetCommonData(hSourceCall, &pInst, &sourceCallId, &sourceAddress, NULL) &&
            sipxCallGetCommonData(hTargetCall, NULL, &targetCallId, &targetAddress, NULL))
        
    {
        // call the transfer            
        if (pInst->pCallManager->transfer(sourceCallId, sourceAddress, targetCallId, targetAddress) == PT_SUCCESS)
        {
            sr = SIPX_RESULT_SUCCESS ;
        }            
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxCallSendInfo(SIPX_INFO* phInfo,
                                          const SIPX_CALL hCall,
                                          const char* szContentType,
                                          const char* szContent,
                                          const size_t nContentLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSendInfo");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallSendInfo phInfo=%p hCall=%d contentType=%s content=%p contentLength=%d",
        phInfo, hCall, szContentType, szContent, (int) nContentLength);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString lineId;
    UtlString remoteAddress ;
    
    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        if (callId.length() != 0)
        {
            SIPX_LINE hLine = sipxLineLookupHandleByURI(lineId.data()) ;
            SIPX_INFO_DATA* pInfoData = new SIPX_INFO_DATA;
            memset((void*) pInfoData, 0, sizeof(SIPX_INFO_DATA));
            SIPX_CALL_DATA* pCall = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger); 

            if(pCall)
            {
                pInfoData->pInst = pInst;
                // Create Mutex
                pInfoData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
                pInfoData->infoData.nSize = sizeof(SIPX_INFO_INFO);
                pInfoData->infoData.hCall = hCall;
                pInfoData->infoData.hLine = hLine;
                pInfoData->infoData.szFromURL = strdup(lineId.data());
                pInfoData->infoData.nContentLength = nContentLength;
                pInfoData->infoData.szContentType = strdup(szContentType);
                pInfoData->infoData.pContent = strdup(szContent);

                *phInfo = gpInfoHandleMap->allocHandle(pInfoData) ;
                assert(*phInfo != 0) ;

                SipSession* pSession = new SipSession(callId, pCall->remoteAddress->data(), pInfoData->infoData.szFromURL);
                sipxCallReleaseLock(pCall, SIPX_LOCK_READ, stackLogger);
                
                pInst->pSipUserAgent->addMessageObserver(*(pInst->pMessageObserver->getMessageQueue()), SIP_INFO_METHOD, 0, 1, 1, 0, 0, pSession, (void*)*phInfo);
                delete pSession;

                if (pInst->pCallManager->sendInfo(callId, remoteAddress, szContentType, nContentLength, szContent))
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    sr = SIPX_RESULT_INVALID_STATE ;
                }
            }
            else
            {
                // Could not find or lock call object
                sr = SIPX_RESULT_INVALID_ARGS ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
    }
    return sr;
} 


SIPXTAPI_API SIPX_RESULT sipxCallGetEnergyLevels(const SIPX_CALL hCall,
                                                 int&            iInputEnergyLevel,
                                                 int&            iOutputEnergyLevel,
                                                 const size_t    nMaxContributors,
                                                 unsigned int    CCSRCs[],
                                                 int             iEnergyLevels[],
                                                 size_t&         nActualContributors) 
{
#if VOICE_ENGINE
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetEnergyLevels");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);    
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId &&
            pData->remoteAddress) 
        {
            if (pData->state == SIPX_INTERNAL_CALLSTATE_CONNECTED)
            {
                CallManager* pCallManager = pData->pInst->pCallManager ;
                UtlString callId(*pData->callId) ;
                UtlString remoteAddress(*pData->remoteAddress) ;
                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

                int nContributors = (int) nMaxContributors ; 
                if (pCallManager->getAudioEnergyLevels(callId, 
                        remoteAddress, 
                        iInputEnergyLevel,
                        iOutputEnergyLevel,
                        nContributors,
                        CCSRCs,
                        iEnergyLevels))
                {
                    nActualContributors = (size_t) nContributors ;
                    rc = SIPX_RESULT_SUCCESS ;                    
                }
                else
                {
                    nActualContributors = 0 ; 
                }
            }
            else
            {
                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
                rc = SIPX_RESULT_INVALID_STATE ;
            }            
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }
    }

#else
    SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED ;
#endif

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtpSourceIds(const SIPX_CALL hCall,
                                                      unsigned int& iSendSSRC,
                                                      unsigned int& iReceiveSSRC) 
{
#if VOICE_ENGINE
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetAudioRtpSourceIds");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);    
    if (pData)        
    {
        if (pData->pInst && pData->pInst->pCallManager && pData->callId &&
            pData->remoteAddress) 
        {
            if (pData->state == SIPX_INTERNAL_CALLSTATE_CONNECTED)
            {
                CallManager* pCallManager = pData->pInst->pCallManager ;
                UtlString callId(*pData->callId) ;
                UtlString remoteAddress(*pData->remoteAddress) ;
                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

                if (pCallManager->getAudioRtpSourceIDs(
                        callId,
                        remoteAddress, 
                        iSendSSRC,
                        iReceiveSSRC))
                {
                    rc = SIPX_RESULT_SUCCESS ;                    
                }
            }
            else
            {
                sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
                rc = SIPX_RESULT_INVALID_STATE ;
            }            
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }
    }

#else
    SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED ;
#endif

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtcpStats(const SIPX_CALL hCall,
                                                   SIPX_RTCP_STATS* pStats) 
{
#if VOICE_ENGINE
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetAudioRtcpStats");
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    int connectionId ;

    // TODO: This should really be plumbed through the call manager

    if (pStats && pStats->cbSize == sizeof(SIPX_RTCP_STATS))
    {
        if (sipxCallGetConnectionId(hCall, connectionId) == SIPX_RESULT_SUCCESS)
        {
            rc = SIPX_RESULT_FAILURE ;
            GipsVoiceEngineLib* pVoiceEngine = sipxCallGetVoiceEnginePtr(hCall);
            if (pVoiceEngine)
            {
                GIPSVE_CallStatistics stats ;
                memset(&stats, 0, sizeof(GIPSVE_CallStatistics)) ;

                if (pVoiceEngine->GIPSVE_RTCPStat(connectionId, &stats) == 0)
                {
                    pStats->fraction_lost = stats.fraction_lost ;
                    pStats->cum_lost = stats.cum_lost ;
                    pStats->ext_max = stats.ext_max ;
                    pStats->jitter = stats.jitter ;
                    pStats->RTT = stats.RTT ;
                    pStats->bytesSent = stats.bytesSent ;
                    pStats->packetsSent = stats.packetsSent ;
                    pStats->bytesReceived = stats.bytesReceived ;
                    pStats->packetsReceived = stats.packetsReceived ;

                    rc = SIPX_RESULT_SUCCESS ;
                }
            }
        }
    }
#else
    SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED ;
#endif

    return rc ;
}
SIPXTAPI_API SIPX_RESULT sipxCallLimitCodecs(const SIPX_CALL hCall,
                                             const char* codecNames)
{
    SIPX_RESULT result = SIPX_RESULT_SUCCESS;
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallLimitCodecs");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallLimitCodecs hCall=%d codecNames=\"%s\"",
        hCall, codecNames);

    SIPX_INSTANCE_DATA *pInst;
    UtlString callId;
    UtlString remoteAddress;

    if (codecNames && *codecNames && sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        pInst->pCallManager->limitCodecs(callId, remoteAddress, codecNames);
    }
    else
    {
        result = SIPX_RESULT_INVALID_ARGS;
    }

    return(result);
}

SIPXTAPI_API SIPX_RESULT sipxCallLimitCodecPreferences(const SIPX_CALL hCall,
                                                       const SIPX_AUDIO_BANDWIDTH_ID audioBandwidth,
                                                       const SIPX_VIDEO_BANDWIDTH_ID videoBandwidth,
                                                       const char* szVideoCodecName)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallLimitCodecPreferences");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallLimitCodecPreferences hCall=%d audioBandwidth=%d videoBandwidth=%d szVideoCodecName=\"%s\"", 
        hCall,
        audioBandwidth,
        videoBandwidth,
        (szVideoCodecName != NULL) ? szVideoCodecName : "") ;

    SIPX_RESULT sr = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    // Test bandwidth for legal values
    if (((audioBandwidth >= AUDIO_CODEC_BW_LOW && audioBandwidth <= AUDIO_CODEC_BW_HIGH) || audioBandwidth == AUDIO_CODEC_BW_DEFAULT) &&
        ((videoBandwidth >= VIDEO_CODEC_BW_LOW && videoBandwidth <= VIDEO_CODEC_BW_HIGH) || videoBandwidth == VIDEO_CODEC_BW_DEFAULT))
    {
        if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
        {
            pInst->pCallManager->limitCodecPreferences(callId, remoteAddress, audioBandwidth, videoBandwidth, szVideoCodecName) ;
            pInst->pCallManager->renegotiateCodecsTerminalConnection(callId, remoteAddress, NULL) ;

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



SIPXTAPI_API SIPX_RESULT sipxCallSetMicGain(const SIPX_CALL hCall,
                                            float gain)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSetMicGain");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxCallSetMicGain() hCall=%d Gain=%f", hCall, gain);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA *pInst ;
    UtlString callId ;

    if (gain < 0.)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, "sipxCallSetMicGain() gain<0");
        sr = SIPX_RESULT_INVALID_ARGS ;
    }
    else if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                      "sipxCallSetMicGain() Setting Mic gain for \"%s\"", callId.data());
        pInst->pCallManager->setMicGain(callId.data(), gain);
        sr = SIPX_RESULT_SUCCESS ;
    }
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, "sipxCallSetMicGain() Can't get call common data");
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxCallSetMixOutputGain(const SIPX_CALL hCall,
                                                  int bridgeOutputIndex,
                                                  float gain)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSetMixOutputGain");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxCallSetMixOutputGain(hCall=%d, bridgeOutputIndex=%d, gain=%f)",
        hCall, bridgeOutputIndex, gain);

    SIPX_RESULT status = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst;
    UtlString callId;

    if(bridgeOutputIndex < 0 || gain < 0.0f)
    {
        // status already set
        assert(bridgeOutputIndex >= 0);
        assert(gain >= 0.0f);
    }
    else if(sipxCallGetCommonData(hCall, &pInst, &callId, NULL, NULL))
    {
        pInst->pCallManager->setOutputMixWeight(callId, bridgeOutputIndex, gain);
        status = SIPX_RESULT_SUCCESS;
    }
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, "sipxCallSetMixOutputGain could not get call data for callId:%d",
            hCall);
        status = SIPX_RESULT_FAILURE;
    }

    return(status);
}

/****************************************************************************
 * Publisher Related Functions
 ***************************************************************************/


SIPXTAPI_API SIPX_RESULT sipxPublisherCreate(const SIPX_INST hInst, 
                                             SIPX_PUB* phPub,
                                             const char* szResourceId,
                                             const char* szEventType,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherCreate");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCreatePublisher hInst=%p phPub=%ld szResourceId=\"%s\" szEventType=\"%s\" szContentType=\"%s\" pContent=\"%s\" nContentLength=%d", 
        hInst,
        *phPub,
        szResourceId ? szResourceId : "<null>",
        szEventType ? szEventType : "<null>",
        szContentType ? szContentType : "<null>",
        pContent ? pContent : "<null>",
        (int) nContentLength);

    SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;

    // Get the instance data
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    // Verify that no content has been previously published for this
    // resourceID and eventType
    HttpBody* oldContentPtr = NULL;
    UtlBoolean isDefaultContent;
    SipPublishContentMgr* publishMgr = NULL;

    if(szEventType && *szEventType)
    {
        if(pInst->pSubscribeServer->isEventTypeEnabled(szEventType))
        {
            publishMgr = 
                pInst->pSubscribeServer->getPublishMgr(szEventType);
            if(publishMgr)
            {
                publishMgr->getContent(szResourceId, 
                                       szEventType, 
                                       szEventType, 
                                       szContentType,
                                       oldContentPtr, 
                                       isDefaultContent);
            }
            // Default content is ok, ignore it
            if(isDefaultContent && oldContentPtr)
            {
                delete oldContentPtr;
                oldContentPtr = NULL;
            }
        }
    }

    else
    {
        sipXresult = SIPX_RESULT_INVALID_ARGS;
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxCreatePublisher: argument szEventType is NULL");
    }

    if(oldContentPtr)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxCreatePublisher: content already exists for resourceId: %s and eventType: %s",
            szResourceId ? szResourceId : "<null>",
            szEventType ? szEventType : "<null>");

        sipXresult = SIPX_RESULT_INVALID_ARGS;
        delete oldContentPtr;
        oldContentPtr = NULL;
    }

    // No prior content published
    else if(szEventType && *szEventType)
    {
        // Create a publish structure for the SIPX_PUB handle
        SIPX_PUBLISH_DATA* pData = new SIPX_PUBLISH_DATA;
        if(pData)
        {
            pData->pInst = pInst;
            pData->pResourceId = new UtlString(szResourceId);
            pData->pMutex =  new OsRWMutex(OsRWMutex::Q_FIFO);
            if(pData->pResourceId)
            {
                pData->pEventType = new UtlString(szEventType);
                if(pData->pEventType)
                {
                    // Create a new HttpBody to publish for the resourceId and eventType
                    HttpBody* content = 
                        new HttpBody(pContent, nContentLength, szContentType);
                    if(content)
                    {
                        // Register the publisher handle
                        *phPub = gpPubHandleMap->allocHandle(pData);

                        // No publisher for this event type
                        if(publishMgr == NULL)
                        {
                            // Enable the event type for the SUBSCRIBE Server
                            pInst->pSubscribeServer->enableEventType(*pData->pEventType);
                            publishMgr = 
                                pInst->pSubscribeServer->getPublishMgr(*pData->pEventType);
                        }

                        // Publish the content
                        publishMgr->publish(pData->pResourceId->data(),
                                            pData->pEventType->data(),
                                            pData->pEventType->data(),
                                            1, // one content type for event
                                            &content);
                        sipXresult = SIPX_RESULT_SUCCESS;
                    }
                    else
                    {
                        sipXresult = SIPX_RESULT_OUT_OF_MEMORY;
                        delete pData->pEventType;
                        delete pData->pResourceId;
                        delete pData;
                        pData = NULL;
                    }
                }
                else
                {
                    sipXresult = SIPX_RESULT_OUT_OF_MEMORY;
                    delete pData->pResourceId;
                    delete pData;
                    pData = NULL;
                }
            }
            else
            {
                sipXresult = SIPX_RESULT_OUT_OF_MEMORY;
                delete pData;
                pData = NULL;
            }
        }
        else
        {
            sipXresult = SIPX_RESULT_OUT_OF_MEMORY;
            *phPub = SIPX_PUB_NULL;
        }
    }
    return(sipXresult);
}

SIPXTAPI_API SIPX_RESULT sipxPublisherUpdate(const SIPX_PUB hPub,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherUpdate");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxUpdatePublisher hPub=%ld szContentType=\"%s\" pContent=\"%s\" nContentLength=%d", 
        hPub,
        szContentType ? szContentType : "<null>",
        pContent ? pContent : "<null>",
        (int) nContentLength);

    SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;

    SIPX_PUBLISH_DATA* pData = (SIPX_PUBLISH_DATA*) sipxPublishLookup(hPub, SIPX_LOCK_WRITE, stackLogger);

    if(szContentType && *szContentType &&
        nContentLength > 0 && 
        pContent && *pContent && 
        pData)
    {
        HttpBody* newContent = 
            new HttpBody(pContent, nContentLength, szContentType);

        SipPublishContentMgr* publishMgr = 
            pData->pInst->pSubscribeServer->getPublishMgr(*pData->pEventType);
        if(publishMgr)
        {
            // Publish the state change
            publishMgr->publish(pData->pResourceId->data(),
                                pData->pEventType->data(),
                                pData->pEventType->data(),
                                1, // one content type for event
                                &newContent);
            sipXresult = SIPX_RESULT_SUCCESS;
        }
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                "sipxUpdatePublisher: no publisher for event type: %s",
                pData->pEventType->data());
            sipXresult = SIPX_RESULT_FAILURE;
        }
    }

    // content size < 0 || content is null
    else
    {
        sipXresult = SIPX_RESULT_INVALID_ARGS;
    }

    sipxPublishReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

    return(sipXresult);
}

SIPXTAPI_API SIPX_RESULT sipxPublisherDestroy(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const size_t nContentLength)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherDestroy");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxDestroyPublisher hPub=%ld szContentType=\"%s\" pFinalContent=\"%s\" nContentLength=%d", 
        hPub,
        szContentType ? szContentType : "<null>",
        pFinalContent ? pFinalContent : "<null>",
        (int) nContentLength);

    SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
    UtlBoolean unPublish = FALSE;
    if(szContentType && *szContentType &&
        pFinalContent && *pFinalContent &&
        nContentLength > 0)
    {
        unPublish = TRUE;
        sipxPublisherUpdate(hPub, szContentType, pFinalContent, nContentLength);
    }

    SIPX_PUBLISH_DATA* pData = sipxPublishLookup(hPub, SIPX_LOCK_WRITE, stackLogger);

    if(pData)
    {

        if(nContentLength > 0 &&
                (szContentType == NULL || *szContentType == '\000' ||
                 pFinalContent == NULL || *pFinalContent == '\000'))
        {
            unPublish = FALSE;
            sipXresult = SIPX_RESULT_INVALID_ARGS;
        }

        if(unPublish)
        {
            SipPublishContentMgr* publishMgr = 
                pData->pInst->pSubscribeServer->getPublishMgr(*pData->pEventType);
            if(publishMgr)
            {
                // Publish the state change
                publishMgr->unpublish(*pData->pResourceId, 
                                      *pData->pEventType,
                                      *pData->pEventType);
            }


            if(pData->pEventType)
            {
                delete pData->pEventType;
                pData->pEventType = NULL;
            }
            if(pData->pResourceId)
            {
                delete pData->pResourceId;
                pData->pResourceId = NULL;
            }
            gpPubHandleMap->removeHandle(hPub);
            sipxPublishReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
            delete pData;
            pData = NULL;
            sipXresult = SIPX_RESULT_SUCCESS;
        }
    }

    // Could not find the publish data for the given handle
    else
    {
        sipXresult = SIPX_RESULT_INVALID_ARGS;
    }

    if (pData)
    {
        sipxPublishReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
    }
    return(sipXresult);
}
                                         
/****************************************************************************
 * Conference Related Functions
 ***************************************************************************/


SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF *phConference)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceCreate");
    SIPX_RESULT rc = SIPX_RESULT_OUT_OF_MEMORY ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceCreate hInst=%p phConference=%p",
        hInst, phConference);

    assert(phConference) ;

    if (phConference)
    {
        *phConference = SIPX_CONF_NULL ;

        SIPX_CONF_DATA* pData = new SIPX_CONF_DATA ;
        assert(pData != NULL) ;
        if (pData)
        {
            // Init conference data
            memset(pData, 0, sizeof(SIPX_CONF_DATA)) ;
            pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;

            // Increment conference counter
            pData->pInst->pLock->acquire() ;
            pData->pInst->nConferences++ ;
            pData->pInst->pLock->release() ;

            pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;
            *phConference = gpConfHandleMap->allocHandle(pData) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}



SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceJoin");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceJoin hConf=%ld hCall=%d",
        hConf, hCall);

    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;
    bool bDoSplit = false ;
    UtlString sourceCallId ;
    UtlString sourceAddress ;
    UtlString targetCallId ;
    SIPX_INSTANCE_DATA* pInst = NULL ;

    if (hConf && hCall)
    {
        SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pConfData)
        {
            SIPX_CALL_DATA * pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger) ;
            if (pCallData)
            {
                if (pCallData->hConf == SIPX_CALL_NULL)
                {
                    rc = SIPX_RESULT_SUCCESS ;

                    if (pConfData->strCallId.isNull())
                    {
                        assert(pConfData->nCalls == 0) ; 

                        // make this call's CpPeerCall the conference shell
                        bDoSplit = false;
                        pConfData->strCallId = *pCallData->callId;

                        pCallData->hConf = hConf ;
                        pConfData->hCalls[pConfData->nCalls++] = hCall ;
                    }
                    else
                    {
                        // we need to split connection from old CpPeerCall
                        // and join it into conference CpPeerCall
                        if ((pCallData->state == SIPX_INTERNAL_CALLSTATE_REMOTE_HELD) ||
                           (pCallData->state == SIPX_INTERNAL_CALLSTATE_HELD))
                        {
                            // Mark data for split/drop below
                            bDoSplit = true ;
                            sourceCallId = *pCallData->callId ;
                            sourceAddress = *pCallData->remoteAddress ;
                            targetCallId = pConfData->strCallId ;
                            pInst = pCallData->pInst ;

                            // Update data structures
                            *pCallData->callId = targetCallId ;
                            pCallData->hConf = hConf ;
                            pConfData->hCalls[pConfData->nCalls++] = hCall ;
                        }
                        else
                        {
                            rc = SIPX_RESULT_INVALID_STATE ;
                        }
                    }                   
                }
                else
                {
                    rc = SIPX_RESULT_INVALID_STATE ;
                }

                sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger) ;
                sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger) ;

                if (rc != SIPX_RESULT_SUCCESS)
                {
                   return rc;
                }
            }
            else
            {
               sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger) ;
               return rc;
            }
        }
        else
        {
           return rc;
        }

        if (bDoSplit)
        {
            // Do the split
            PtStatus status = pInst->pCallManager->split(sourceCallId, sourceAddress, targetCallId) ;
            if (status != PT_SUCCESS)
            {
                rc = SIPX_RESULT_FAILURE ;
            }
            else
            {
                rc = SIPX_RESULT_SUCCESS ;
            }

            // If the call fails -- hard to recover, drop the call anyways.
            pInst->pCallManager->drop(sourceCallId) ;
        }
        else
        {
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConf,
                                             const SIPX_CALL hCall)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceSplit");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceSplit hConf=%ld hCall=%d",
        hConf, hCall);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    UtlString sourceCallId ;
    UtlString sourceAddress ;
    UtlString targetCallId ;
    SIPX_INSTANCE_DATA *pInst = NULL ;
    size_t nCalls;

    if (hConf && hCall)
    {
        SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pConfData)
        {
            nCalls = pConfData->nCalls;
            SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger) ;
            if (pCallData)
            {
                if ((pCallData->state == SIPX_INTERNAL_CALLSTATE_REMOTE_HELD) || 
                    (pCallData->state == SIPX_INTERNAL_CALLSTATE_HELD))
                {
                    // Record data for split
                    pInst = pCallData->pInst ;
                    sourceCallId = *pCallData->callId ;
                    sourceAddress = *pCallData->remoteAddress ;

                    // Remove from conference handle
                    sipxRemoveCallHandleFromConf(hConf, hCall) ;                

                    // Create a CpPeerCall call to hold connection
                    pCallData->pInst->pCallManager->createCall(&targetCallId) ;

                    *pCallData->callId = targetCallId ;
                    pCallData->hConf = SIPX_CALL_NULL ;                                    

                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_INVALID_STATE ;
                }

                sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger) ;
            }
            sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger) ;
        }

        // Initiate Split
        if (rc == SIPX_RESULT_SUCCESS)
        {
            PtStatus status = pInst->pCallManager->split(sourceCallId, sourceAddress, targetCallId) ;
            if (status != PT_SUCCESS)
            {
                rc = SIPX_RESULT_FAILURE ;
            }
            else
            if (nCalls == 1)
            {
                // this was the last connection, drop the shell call
                SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
                pInst->pCallManager->drop(pConfData->strCallId);
                pConfData->strCallId = "";
                sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger) ;
            }
        }
    }


    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_CONTACT_ID contactId,
                                           SIPX_VIDEO_DISPLAY* const pDisplay,
                                           SIPX_SECURITY_ATTRIBUTES* const pSecurity,
                                           bool bTakeFocus,
                                           SIPX_CALL_OPTIONS* options)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceAdd");
    bool bEnableLocationHeader=false;
    SIPX_TRANSPORT hTransport = SIPX_TRANSPORT_NULL;    
    int bandWidth=AUDIO_CODEC_BW_DEFAULT;
    RTP_TRANSPORT rtpTransportOptions = RTP_TRANSPORT_UDP;

    if (options != NULL && options->cbSize)
    {
        bEnableLocationHeader = options->sendLocation;
        bandWidth = options->bandwidthId;

        if (options->cbSize == sizeof(SIPX_CALL_OPTIONS))
        {
            rtpTransportOptions = (RTP_TRANSPORT)options->rtpTransportFlags;
        }
        else
        {
            // try to provide some drop-in compatibility.
            rtpTransportOptions = RTP_TRANSPORT_UDP;
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceAdd hConf=%ld hLine=%d szAddress=%s contactId=%d, pDisplay=%p "
        "bEnableLocationHeader=%d bandWidth=%d",
        hConf, hLine, szAddress, contactId, pDisplay, bEnableLocationHeader, bandWidth);

    if (hLine == SIPX_LINE_NULL)
    {
       OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "sipxConferenceAdd is called with NULL hLine!");
       return SIPX_RESULT_INVALID_ARGS;
    }

    // check for and load security runtime .dlls
    if (pSecurity)
    {
        SIPX_RESULT rc = sipxConfigLoadSecurityRuntime();
        if (SIPX_RESULT_SUCCESS != rc)
        {
            return rc;  // return right away 
        }
    }

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    bool bSetFocus = false ;
    char* pLocationHeader = NULL;

    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData->pInst && pSecurity)
        {
            // augment the security attributes with the instance's security parameters
            SecurityHelper securityHelper;
            // generate a random key, if one isn't there
            if (strlen(pSecurity->getSrtpKey()) == 0)
            {
                securityHelper.generateSrtpKey(*pSecurity);
            }
            if (pData->pInst->dbLocation)
            {
                securityHelper.setDbLocation(*pSecurity, pData->pInst->dbLocation);
            }
            if (pData->pInst->myCertNickname)
            {
                securityHelper.setMyCertNickname(*pSecurity, pData->pInst->myCertNickname);
            }
            if (pData->pInst->dbPassword)
            {
                securityHelper.setDbPassword(*pSecurity, pData->pInst->dbPassword);
            }
        }
        if (pData)
        {
            SIPX_CONTACT_ADDRESS* pContactAddress = pData->pInst->pSipUserAgent->getContactDb().find(contactId) ;
            SIPX_CONTACT_TYPE contactType;
            
            if (pContactAddress)
            {
                contactType = pContactAddress->eContactType;
                if (pContactAddress->eTransportType > 3)
                {
                    hTransport = pContactAddress->eTransportType;
                    pData->hTransport = hTransport;
                }            
            

                OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                    "contactId(%d): type: %s, transport: %s, interface: %s, ip: %s:%d",
                    contactId,
                    sipxContactTypeToString((SIPX_CONTACT_TYPE) pContactAddress->eContactType),
                    sipxTransportTypeToString((SIPX_TRANSPORT_TYPE) pContactAddress->eTransportType),
                    pContactAddress->cInterface,
                    pContactAddress->cIpAddress,
                    pContactAddress->iPort) ;
            }

            if (pData->nCalls == 0)
            {
                /*
                 * No existing legs, create one and connect
                 */

                // Create new call
                SIPX_CALL hNewCall ;
                rc = sipxCallCreateHelper(pData->pInst, hLine, hConf, &hNewCall) ;
                if (rc == SIPX_RESULT_SUCCESS)
                {
                    // Only take focus if something doesn't already have it.
                    if (!sipxIsCallInFocus() || bTakeFocus)
                    {
                        SIPX_CALL_DATA* pCallData = sipxCallLookup(hNewCall, SIPX_LOCK_READ, stackLogger) ;
                        if (pCallData)
                        {
                            pCallData->pInst->pCallManager->unholdLocalTerminalConnection(pCallData->callId->data()) ;
                            bSetFocus = true ;
                            sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger) ;
                        }                        
                    }

                    // Get data struct for call and store callId as conf Id
                    assert(hNewCall) ;

                    SIPX_CALL_DATA* pCallData = sipxCallLookup(hNewCall, SIPX_LOCK_WRITE, stackLogger) ;                    
                    assert(pCallData) ;
                    if (pDisplay)
                    {
                        pCallData->display = *pDisplay;
                    }
                    if (pSecurity)
                    {
                        pCallData->security = *pSecurity;
                    }
                    pCallData->bInFocus = bSetFocus ;
                    pData->strCallId = *pCallData->callId;
                    UtlString legCallId = pData->strCallId;
                    
                    // Add the call handle to the conference handle
                    pData->hCalls[pData->nCalls++] = hNewCall ;
                    *phNewCall = hNewCall ;

                    UtlString sessionId ;
                    pData->pInst->pCallManager->getNewSessionId(&sessionId) ;
                    pCallData->sessionCallId = new UtlString(sessionId);

                    // Save some data for later use
                    SIPX_INSTANCE_DATA* pInst = pCallData->pInst;
                    UtlString lineId = *pCallData->lineURI;
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger) ;


                    // set outbound line id
                    pInst->pCallManager->setOutboundLineForCall(legCallId.data(), lineId.data()) ;

                    // THIS IS A HACK!
                    // We should pass sessionId directly to sipxFireCallEvent()
                    // instead of faking SipSession.
                    SipSession session;
                    session.setCallId(sessionId);
                    // Notify Listeners of new call
                    sipxFireCallEvent(pInst->pCallManager, 
                            legCallId.data(), 
                            &session,
                            NULL, 
                            CALLSTATE_DIALTONE, 
                            CALLSTATE_CAUSE_CONFERENCE) ;   

                    // Issue connect
                    if (pInst)
                    {
                        pLocationHeader = (bEnableLocationHeader) ? pInst->szLocationHeader : NULL;
                    }
                    
                    SIPX_TRANSPORT_DATA* pTransportDataCopy = NULL;
                    if (SIPX_TRANSPORT_NULL != hTransport)
                    {
                        SIPX_TRANSPORT_DATA* pTransportData = NULL;
                        pTransportData = sipxTransportLookup(hTransport, SIPX_LOCK_READ);
                        pTransportDataCopy = new SIPX_TRANSPORT_DATA(*pTransportData);
                        sipxTransportReleaseLock(pTransportData, SIPX_LOCK_READ);
                    }
                    
                    PtStatus status;
                    if (pDisplay && pDisplay->handle)
                    {
                        status = pData->pInst->pCallManager->connect(pData->strCallId.data(),
                                szAddress, NULL, sessionId.data(), (SIPX_CONTACT_ID) contactId, 
                                &pCallData->display, pSecurity ? &pCallData->security : NULL,
                                pLocationHeader, bandWidth, pTransportDataCopy, rtpTransportOptions) ;
                    }
                    else
                    {
                        status = pData->pInst->pCallManager->connect(pData->strCallId.data(),
                                szAddress, NULL, sessionId.data(), (SIPX_CONTACT_ID) contactId, 
                                NULL, pSecurity ? &pCallData->security : NULL,
                                pLocationHeader, bandWidth, pTransportDataCopy, rtpTransportOptions) ;
                    }
                    delete pTransportDataCopy;

                    if (status == PT_SUCCESS)
                    {
                        rc = SIPX_RESULT_SUCCESS ;
                    }
                    else
                    {
                        sipxFireCallEvent(pData->pInst->pCallManager, 
                                legCallId.data(),
                                &session,
                                szAddress, 
                                CALLSTATE_DISCONNECTED, 
                                CALLSTATE_CAUSE_BAD_ADDRESS) ;
                        rc = SIPX_RESULT_BAD_ADDRESS ;
                    }
                }
            }
            else if (pData->nCalls < CONF_MAX_CONNECTIONS && 
                    pData->pInst->pCallManager->canAddConnection(pData->strCallId.data()))
            {
                SIPX_LINE_DATA* pLine = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);
                UtlString lineId = pLine->lineURI->toString().data();
                sipxLineReleaseLock(pLine, SIPX_LOCK_READ, stackLogger);

                // Only take focus if something doesn't already have it.
                if (!sipxIsCallInFocus() || bTakeFocus)
                {
                    pData->pInst->pCallManager->unholdLocalTerminalConnection(pData->strCallId.data()) ;
                    bSetFocus = true ;
                }

                /*
                 * Use existing call id to find call
                 */
                SIPX_INSTANCE_DATA* pInst ;
                UtlString dummy ;

                if (sipxCallGetCommonData(pData->hCalls[0], &pInst, NULL, NULL, &dummy))
                {
                    SIPX_CALL_DATA* pNewCallData = new SIPX_CALL_DATA ;
                    SIPX_CONTACT_ADDRESS* pContactAddress = pInst->pSipUserAgent->getContactDb().find(contactId) ;
                    SIPX_CONTACT_TYPE contactType;
                    
                    if (pContactAddress)
                    {
                        contactType = pContactAddress->eContactType;
                        if (pContactAddress->eTransportType > 3)
                        {
                            hTransport = pContactAddress->eTransportType;
                            pData->hTransport = hTransport;
                        }            
                    }                        
                    
                    memset((void*) pNewCallData, 0, sizeof(SIPX_CALL_DATA));

                    UtlString sessionId ;
                    UtlString callId = pData->strCallId;
                    pData->pInst->pCallManager->getNewSessionId(&sessionId) ;

                    pNewCallData->pInst = pInst ;
                    pNewCallData->hConf = hConf ;
                    pNewCallData->callId = new UtlString(callId);
                    pNewCallData->sessionCallId = new UtlString(sessionId);
                    pNewCallData->remoteAddress = NULL ;
                    pNewCallData->hLine = hLine ;
                    pNewCallData->lineURI = new UtlString(lineId.data()) ;
                    pNewCallData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;
                    pNewCallData->connectionId = -1;
                    pNewCallData->bInFocus = bSetFocus;
                    pNewCallData->hTransport = hTransport;
                    if (pSecurity)
                        pNewCallData->security = *pSecurity;


                    SIPX_CALL hNewCall = gpCallHandleMap->allocHandle(pNewCallData) ;
                    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                                  "sipxConferenceAdd new hCall: %d Call-Id: %s",
                                  hNewCall,
                                  pNewCallData->callId->data());

                    pData->hCalls[pData->nCalls++] = hNewCall ;
                    *phNewCall = hNewCall ;

                    pInst->pLock->acquire() ;
                    pInst->nCalls++ ;
                    pInst->pLock->release() ;

                    // THIS IS A HACK!
                    // We should pass sessionId directly to sipxFireCallEvent()
                    // instead of faking SipSession.
                    SipSession session;
                    session.setCallId(sessionId);
                    // Notify Listeners of new call
                    sipxFireCallEvent(pData->pInst->pCallManager, 
                            callId.data(), 
                            &session,
                            NULL, 
                            CALLSTATE_DIALTONE, 
                            CALLSTATE_CAUSE_CONFERENCE) ;

                    if (pInst)
                    {
                        pLocationHeader = (bEnableLocationHeader) ? pInst->szLocationHeader : NULL;
                    }

                    // set outbound line id
                    pInst->pCallManager->setOutboundLineForCall(pData->strCallId.data(), lineId.data()) ;

                    SIPX_TRANSPORT_DATA* pTransportDataCopy = NULL;
                    if (SIPX_TRANSPORT_NULL != hTransport)
                    {
                        SIPX_TRANSPORT_DATA* pTransportData = NULL;
                        pTransportData = sipxTransportLookup(hTransport, SIPX_LOCK_READ);
                        pTransportDataCopy = new SIPX_TRANSPORT_DATA(*pTransportData);
                        sipxTransportReleaseLock(pTransportData, SIPX_LOCK_READ);
                    }
                    
                    PtStatus status = pData->pInst->pCallManager->connect(
                            pData->strCallId.data(), szAddress, NULL, 
                            sessionId.data(), (SIPX_CONTACT_ID) contactId,                                                                           
                            pDisplay, pSecurity ? &pNewCallData->security : NULL, 
                            pLocationHeader, bandWidth, pTransportDataCopy, rtpTransportOptions) ;
                    delete pTransportDataCopy;                            
                    if (status == PT_SUCCESS)
                    {
                        rc = SIPX_RESULT_SUCCESS ;
                    }
                    else
                    {
                        sipxFireCallEvent(pData->pInst->pCallManager, 
                                callId.data(), 
                                &session,
                                szAddress, 
                                CALLSTATE_DISCONNECTED, 
                                CALLSTATE_CAUSE_BAD_ADDRESS) ;
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

            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceRemove");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceRemove hConf=%ld hCall=%d",
        hConf, hCall);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    if (hConf && hCall)
    {
        SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        UtlString callId ;
        UtlString remoteAddress ;
        SIPX_INSTANCE_DATA* pInst ;

        if (pConfData && sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
        {
            sipxRemoveCallHandleFromConf(hConf, hCall) ;
            pInst->pCallManager->dropConnection(callId.data(), remoteAddress.data()) ;

            if (pConfData->nCalls == 0)
            {
                // this was the last connection, drop the shell call
                pInst->pCallManager->drop(pConfData->strCallId);
                pConfData->strCallId = "";
            }
            

            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            // Either the call or conf doesn't exist
            rc = SIPX_RESULT_FAILURE ;
        }

        sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger) ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL hCalls[],
                                                const size_t iMax,
                                                size_t& nActual)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceGetCalls");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceGetCalls hConf=%ld",
        hConf);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    nActual = 0 ;

    if (hConf && iMax)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData)
        {
            // OsReadLock(*pData->pMutex) ;

            size_t idx ;
            for (idx=0; (idx<pData->nCalls) && (idx < iMax); idx++)
            {
                hCalls[idx] = pData->hCalls[idx] ;
            }
            nActual = idx ;

            rc = SIPX_RESULT_SUCCESS ;

            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }
        else
        {
            rc = SIPX_RESULT_FAILURE ;
        }
    }
    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceHold(const SIPX_CONF hConf, bool bBridging)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceHold");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceHold hConf=%ld bBridging=%d",
        hConf,
        bBridging);


    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData)
        {
            sr = SIPX_RESULT_INVALID_STATE; 
            if (!pData->strCallId.isNull())
            {
               if (bBridging)
               {
                  if (pData->confHoldState != CONF_STATE_BRIDGING_HOLD)
                  {
                     pData->pInst->pCallManager->holdLocalTerminalConnection(pData->strCallId.data());
                     pData->confHoldState = CONF_STATE_BRIDGING_HOLD;
                     sr = SIPX_RESULT_SUCCESS;
                  }
               }
               else
               {
                  if (pData->confHoldState != CONF_STATE_NON_BRIDGING_HOLD)
                  {
                     pData->pInst->pCallManager->holdAllTerminalConnections(pData->strCallId.data());
                     pData->confHoldState = CONF_STATE_NON_BRIDGING_HOLD;
                     sr = SIPX_RESULT_SUCCESS;
                  }
               }
            }
            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;
        }
        else
        {
            sr = SIPX_RESULT_FAILURE ;
        }
    }
        
    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceUnhold(const SIPX_CONF hConf)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceUnhold");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceUnHold hConf=%ld",
        hConf);

    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData)
        {
            if (pData->confHoldState == CONF_STATE_BRIDGING_HOLD)
            {
                pData->pInst->pCallManager->unholdLocalTerminalConnection(pData->strCallId.data());
                pData->confHoldState = CONF_STATE_UNHELD;
                sr = SIPX_RESULT_SUCCESS;
            }
            else if (pData->confHoldState == CONF_STATE_NON_BRIDGING_HOLD)
            {
                pData->pInst->pCallManager->unholdAllTerminalConnections(pData->strCallId.data());
                pData->confHoldState = CONF_STATE_UNHELD;
                sr = SIPX_RESULT_SUCCESS;
            }
            else
            {
                sr = SIPX_RESULT_INVALID_STATE ;
            }
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
        }
        else
        {
            sr = SIPX_RESULT_FAILURE ;
        }
    }
    return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStart(const SIPX_CONF hConf,
                                                          const char* szFile,
                                                          const bool bRepeat,
                                                          const bool bLocal,
                                                          const bool bRemote,
                                                          const bool bMixWithMic,
                                                          const float fVolumeScaling)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferencePlayAudioFileStart");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferencePlayAudioFileStart hConf=%ld File=%s bLocal=%d bRemote=%d bRepeat=%d",
        hConf, szFile, bLocal, bRemote, bRepeat);

    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS ;

    // Massage volume scaling into range
    float fDownScaling = fVolumeScaling ;
    if (fDownScaling > 1.0)
    {
        fDownScaling = 1.0 ;
    }
    else if (fDownScaling < 0.0)
    {
        fDownScaling = 0.0 ;
    }
    
    if (hConf && szFile && (bLocal || bRemote))
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData)
        {
            pData->nNumFilesPlaying++;
            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }
        pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData && pData->pInst)
        {
            pData->pInst->pCallManager->audioPlay(pData->strCallId.data(), szFile, bRepeat, bLocal, bRemote, bMixWithMic, (int) (fDownScaling * 100.0)) ;            
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

            sr = SIPX_RESULT_SUCCESS ;
        }
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStop(const SIPX_CONF hConf)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferencePlayAudioFileStop");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferencePlayAudioFileStop hConf=%ld", hConf);

    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS ;

    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData)
        {
            pData->nNumFilesPlaying--;
            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
        }    
        pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData && pData->pInst)
        {
            pData->pInst->pCallManager->audioStop(pData->strCallId.data()) ;
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

            sr = SIPX_RESULT_SUCCESS ;
        }
    }

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceDestroy");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceDestroy hConf=%ld",
        hConf);

    SIPX_CALL hCalls[CONF_MAX_CONNECTIONS] ;
    size_t nCalls ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    if (hConf)
    {
        int nNumFilesPlaying = 0;
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
        if (pData)
        {
            nNumFilesPlaying = pData->nNumFilesPlaying;
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
        }    
        
        // I'm not sure why this check was here - likely it was because
        // of some GIPS VE quirks. But with sipXmediaLib we don't need
        // this, so I remove it. This check makes it much more likely to
        // have non-cleaned up calls if something goes wrong.
/*        if (nNumFilesPlaying > 0)
        {
           rc = SIPX_RESULT_BUSY;
        }
        else
*/
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
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceGetEnergyLevels(const SIPX_CONF hConf,
                                                       int&            iInputEnergyLevel,
                                                       int&            iOutputEnergyLevel) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceGetEnergyLevels");
    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS ;

    if (hConf)
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData)
        {
            if (pData->pInst && pData->pInst->pCallManager && !pData->strCallId.isNull())
            {
                CallManager* pCallManager = pData->pInst->pCallManager ;
                UtlString callId = pData->strCallId ;            
                sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;

                if (pCallManager->getAudioEnergyLevels(callId, iInputEnergyLevel, iOutputEnergyLevel))
                {
                    sr = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    sr = SIPX_RESULT_FAILURE ;
                }
            }
            else
            {
                sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
                sr = SIPX_RESULT_FAILURE ;                
            }            
        }
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceLimitCodecPreferences(const SIPX_CONF hConf,
                                                             const SIPX_AUDIO_BANDWIDTH_ID audioBandwidth,
                                                             const SIPX_VIDEO_BANDWIDTH_ID videoBandwidth,
                                                             const char* szVideoCodecName) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceLimitCodecPreferences");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConferenceLimitCodecPreferences hCall=%ld audioBandwidth=%d videoBandwidth=%d szVideoCodecName=\"%s\"", 
        hConf,
        audioBandwidth,
        videoBandwidth,
        (szVideoCodecName != NULL) ? szVideoCodecName : "") ;

    SIPX_RESULT sr = SIPX_RESULT_FAILURE;

    UtlString callId ;

    // Test bandwidth for legal values
    if (((audioBandwidth >= AUDIO_CODEC_BW_LOW && audioBandwidth <= AUDIO_CODEC_BW_HIGH) || audioBandwidth == AUDIO_CODEC_BW_DEFAULT) &&
        ((videoBandwidth >= VIDEO_CODEC_BW_LOW && videoBandwidth <= VIDEO_CODEC_BW_HIGH) || videoBandwidth == VIDEO_CODEC_BW_DEFAULT) &&
        (hConf != SIPX_CONF_NULL))
    {
        SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger) ;
        if (pData)
        {
            UtlString callId = pData->strCallId ;
            CallManager* callManager = pData->pInst->pCallManager;
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
            pData = NULL; // Can't touch pData after release lock

           if(callManager && !callId.isNull())
            {

                callManager->limitCodecPreferences(callId, audioBandwidth, videoBandwidth, szVideoCodecName);
                callManager->silentRemoteHold(callId);
                callManager->renegotiateCodecsAllTerminalConnections(callId);

                sr = SIPX_RESULT_SUCCESS ;
            }
            else
            {
                sr = SIPX_RESULT_INVALID_ARGS ;
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS;
        }
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxMediaConnectionCreate(const SIPX_CONF hConf,
                                                             int& connectionId)
{
    SIPX_RESULT status = SIPX_RESULT_INVALID_ARGS;
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxMediaConnectionCreate");

    SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
    if (pData)
    {
        UtlString callId = pData->strCallId;
        CallManager* callManager = pData->pInst->pCallManager;
        // We want to release this lock ASAP as createMediaConnection can block for a while
        sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
        pData = NULL; // Can't touch pData after release lock

        // I suppose theoretically the callManger could get deleted out from under us.
        if(callManager && !callId.isNull())
        {
            OsStatus osStatus = callManager->createMediaConnection(callId, connectionId);

            if(osStatus == OS_SUCCESS)
            {
                status = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return(status);
}

SIPXTAPI_API SIPX_RESULT sipxMediaConnectionRtpSetDestination(const SIPX_CONF hConf,
                                                              int connectionId,
                                                              MEDIA_TYPE mediaType,
                                                              int mediaTypeStreamIndex,
                                                              const char* mediaRecieveAddress,
                                                              int rtpPort,
                                                              int rtcpPort)
{
    SIPX_RESULT status = SIPX_RESULT_INVALID_ARGS;
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxMediaConnectionRtpSetDestination");

    CpMediaInterface::MEDIA_STREAM_TYPE cpMediaType = convertToCpMediaInterfaceMediaType(mediaType);

    SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
    if (pData)
    {
        UtlString callId = pData->strCallId;
        CallManager* callManager = pData->pInst->pCallManager;
        // We want to release this lock ASAP as createMediaConnection can block for a while
        sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
        pData = NULL; // Can't touch pData after release lock

        if(callManager && !callId.isNull())
        {
            OsStatus osStatus = callManager->setRtpDestination(callId, connectionId, cpMediaType,
                mediaTypeStreamIndex, mediaRecieveAddress, rtpPort, rtcpPort);

            if(osStatus == OS_SUCCESS)
            {
                status = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return(status);
}

SIPXTAPI_API SIPX_RESULT sipxMediaConnectionRtpStartSend(const SIPX_CONF hConf,
                                                         int connectionId,
                                                         int numTokens,
                                                         const char* codecTokens[],
                                                         int payloadIds[])
{
    SIPX_RESULT status = SIPX_RESULT_INVALID_ARGS;
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxMediaConnectionRtpStartSend");

    SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
    if (pData)
    {
        UtlString callId = pData->strCallId;
        CallManager* callManager = pData->pInst->pCallManager;
        // We want to release this lock ASAP as createMediaConnection can block for a while
        sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
        pData = NULL; // Can't touch pData after release lock

        if(callManager && !callId.isNull())
        {
            SdpCodecList codecList;
            codecList.addCodecs(numTokens, codecTokens, payloadIds);

            // Make sure the codecs are bound to payload ID.  Codecs with unbound payload IDs are set to -1
            codecList.bindPayloadTypes();

            OsStatus osStatus = callManager->startRtpSend(callId, connectionId, codecList);

            if(osStatus == OS_SUCCESS)
            {
                status = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return(status);
}

SIPXTAPI_API SIPX_RESULT sipxMediaConnectionRtpStopSend(const SIPX_CONF hConf,
                                                        int connectionId)
{
    SIPX_RESULT status = SIPX_RESULT_INVALID_ARGS;
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxMediaConnectionRtpStopSend");

    SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
    if (pData)
    {
        UtlString callId = pData->strCallId;
        CallManager* callManager = pData->pInst->pCallManager;
        // We want to release this lock ASAP as createMediaConnection can block for a while
        sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
        pData = NULL; // Can't touch pData after release lock

        if(callManager && !callId.isNull())
        {
            OsStatus osStatus = callManager->stopRtpSend(callId, connectionId);

            if(osStatus == OS_SUCCESS)
            {
                status = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return(status);
}

/****************************************************************************
 * Audio Related Functions
 ***************************************************************************/


static void initMicSettings(MIC_SETTING* pMicSetting)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "initMicSettings micSettings=%p",
        pMicSetting);

    pMicSetting->bInitialized = TRUE ;
    pMicSetting->bMuted = FALSE ;
    pMicSetting->iGain = GAIN_DEFAULT ;
    memset(pMicSetting->device, 0, sizeof(pMicSetting->device)) ;
}

static void initSpeakerSettings(SPEAKER_SETTING* pSpeakerSetting)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "initSpeakerSettings speakerSettings=%p",
        pSpeakerSetting);
        
    pSpeakerSetting->bInitialized = TRUE ;
    pSpeakerSetting->iVol = VOLUME_DEFAULT ;

    memset(pSpeakerSetting->device, 0, sizeof(pSpeakerSetting->device)) ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetGain(const SIPX_INST hInst,
                                          const int iLevel)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetGain");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioSetGain hInst=%p iLevel=%d",
        hInst, iLevel);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

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
                    int iAdjustedGain = (int) ((double)((double)iLevel / (double)GAIN_MAX) * 100.0) ;
                    rc = pInterface->setMicrophoneGain( iAdjustedGain ) ;    
                    assert(rc == OS_SUCCESS) ;                
                }
                
                sr = SIPX_RESULT_SUCCESS ;
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetGain");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetGain hInst=%p",
        hInst);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        // Lazy init
        if (!pInst->micSetting.bInitialized)
        {
            int iGain ;
            
            initMicSettings(&pInst->micSetting) ;
            assert(pInst->micSetting.bInitialized) ;
            
            // Get Live Gain
            OsStatus status = pInterface->getMicrophoneGain(iGain) ;
            assert(status == OS_SUCCESS) ;

            int iAdjustedGain = (int) (double)((((double)iGain / 100.0)) * (double)GAIN_MAX);        
            pInst->micSetting.iGain = iAdjustedGain;          
        }

        iLevel = pInst->micSetting.iGain;
        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioMute(const SIPX_INST hInst,
                                       const bool bMute)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioMute");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioMute hInst=%p bMute=%d",
        hInst, bMute);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

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
                OsStatus rc = pInterface->muteMicrophone(bMute) ;
                assert(rc == OS_SUCCESS);
                sr = SIPX_RESULT_SUCCESS ;
            }
            else
            {
                // UnMute mic
                OsStatus rc = pInterface->muteMicrophone(bMute) ;
                assert(rc == OS_SUCCESS);
                // Restore gain
                // convert from sipXtapi scale to 100 scale
                int iAdjustedGain = (int) (double)((((double)pInst->micSetting.iGain / (double)GAIN_MAX)) * 100.0); 
                rc = pInterface->setMicrophoneGain(iAdjustedGain);
                assert(rc == OS_SUCCESS) ;

                int iGain ;
                
                rc = pInterface->getMicrophoneGain(iGain);

                assert(iGain == pInst->micSetting.iGain);                                             
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioIsMuted");
    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
        "sipxAudioIsMuted hInst=%p",
        hInst);
        
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

        sr = SIPX_RESULT_SUCCESS ;
    }

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxAudioEnableSpeaker(const SIPX_INST hInst,
                                                const SPEAKER_TYPE type)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioEnableSpeaker");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioEnableSpeaker hInst=%p type=%d",
        hInst, type);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    OsStatus status ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
    UtlString checkDevice ; 

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

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
            status = pInterface->setSpeakerVolume(0) ;
            assert(status == OS_SUCCESS) ;

            if (status == OS_SUCCESS)
            {
                // Enable Speaker
                switch (type)
                {
                    case SPEAKER:
                    case RINGER:
                        pInterface->setSpeakerDevice(pInst->speakerSettings[type].device) ;                        
                        pInterface->getSpeakerDevice(checkDevice) ;
                        strncpy(pInst->speakerSettings[type].device, checkDevice, 
                                sizeof(pInst->speakerSettings[type].device)-1) ;
                        break ;                    
                    default:
                        assert(FALSE) ;
                        break ;
                }
            }

            if (status == OS_SUCCESS)
            {
                // Reset Volume
                SIPX_RESULT rc;
                rc = sipxAudioSetVolume(hInst, type, pInst->speakerSettings[type].iVol);
                assert(rc == SIPX_RESULT_SUCCESS) ;
                int iVolume ;
                rc = sipxAudioGetVolume(hInst, type, iVolume);
                assert(rc == SIPX_RESULT_SUCCESS) ;
                assert(iVolume == pInst->speakerSettings[type].iVol) ;
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetEnabledSpeaker");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetEnabledSpeaker hInst=%p",
        hInst);
        
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetVolume");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioSetVolume hInst=%p type=%d iLevel=%d",
        hInst, type, iLevel);
        
    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS;

    if(iLevel >= VOLUME_MIN && iLevel <= VOLUME_MAX && 
        (type == SPEAKER || type == RINGER))
    {
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

        if (pInst)
        {
            CpMediaInterfaceFactoryImpl* pInterface = 
                    pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

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
                        // the CpMediaInterfaceFactoryImpl always uses a scale of 0 - 100
                        OsStatus status = pInterface->setSpeakerVolume(iLevel) ;
                        assert(status == OS_SUCCESS) ;
                        int iVolume ;
                        status = pInterface->getSpeakerVolume(iVolume) ;
                        assert(status == OS_SUCCESS) ;
                        assert(iVolume == iLevel) ;
                        if (status != OS_SUCCESS)
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
    }
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type,
                                            int& iLevel)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetVolume");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetVolume hInst=%p type=%d",
        hInst, type);
        
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


SIPXTAPI_API SIPX_RESULT sipxAudioSetAECMode(const SIPX_INST hInst,
                                             const SIPX_AEC_MODE mode) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetAECMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxAudioSetAECMode hInst=%p mode=%d",
            hInst, mode);

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface)
        {
            if (pInterface->setAudioAECMode((MEDIA_AEC_MODE) mode) == OS_SUCCESS)
            {
                if (!pInst->aecSetting.bInitialized)
                {
                    pInst->aecSetting.bInitialized = true;
                }
                pInst->aecSetting.mode = mode ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
    }
    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetAECMode(const SIPX_INST hInst,
                                             SIPX_AEC_MODE&  mode)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetAECMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
            "sipxAudioGetAECMode hInst=%p",
            hInst);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
               pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface && !pInst->aecSetting.bInitialized)
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                          "sipxAudioGetAECMode !aecSetting.bInitialized");
            MEDIA_AEC_MODE aceMode ;
            OsStatus miStatus = pInterface->getAudioAECMode(aceMode);
            if (miStatus == OS_SUCCESS)
            {
                pInst->aecSetting.bInitialized = true;
                pInst->aecSetting.mode = (SIPX_AEC_MODE) aceMode ;
                mode = (SIPX_AEC_MODE) aceMode ;

                sr = SIPX_RESULT_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                              "sipxAudioGetAECMode getAudioAECMode returned: %d", miStatus);
            }
        }
        else
        {
            mode = pInst->aecSetting.mode  ;
            sr = SIPX_RESULT_SUCCESS;
        }
    }
    return sr;
}

// For backward compatibility -- remove in time.
SIPXTAPI_API SIPX_RESULT sipAudioSetAGCMode(const SIPX_INST hInst,
                                            const bool bEnable) 
{    
    return sipxAudioSetAGCMode(hInst, bEnable) ;
} 


SIPXTAPI_API SIPX_RESULT sipxAudioSetAGCMode(const SIPX_INST hInst,
                                            const bool bEnable) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetAGCMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxAudioSetAGCMode hInst=%p enable=%d",
            hInst, bEnable);

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface)
        {
            if (pInterface->enableAGC(bEnable) == OS_SUCCESS)
            {
                if (!pInst->agcSetting.bInitialized)
                {
                    pInst->agcSetting.bInitialized = true;
                }
                pInst->agcSetting.bEnabled = bEnable ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
    }
    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetAGCMode(const SIPX_INST hInst,
                                             bool& bEnabled) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetAGCMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
            "sipxAudioGetAGCMode hInst=%p",
            hInst);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
               pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface && !pInst->agcSetting.bInitialized)
        {            
            UtlBoolean bCheck ;
            if (pInterface->isAGCEnabled(bCheck) == OS_SUCCESS)
            {
                pInst->agcSetting.bInitialized = true;
                pInst->agcSetting.bEnabled = (bCheck == TRUE);
                bEnabled = (bCheck == TRUE);

                sr = SIPX_RESULT_SUCCESS;
            }
        }
        else
        {
            bEnabled = pInst->agcSetting.bEnabled ;
            sr = SIPX_RESULT_SUCCESS;
        }
    }

    return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetNoiseReductionMode(const SIPX_INST hInst,
                                                        const SIPX_NOISE_REDUCTION_MODE mode) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetNoiseReductionMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxAudioSetNoiseReductionMode hInst=%p mode=%d",
            hInst, mode);

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface)
        {
            if (pInterface->setAudioNoiseReductionMode((MEDIA_NOISE_REDUCTION_MODE) mode) == OS_SUCCESS)
            {
                if (!pInst->nrSetting.bInitialized)
                {
                    pInst->nrSetting.bInitialized = true;
                }
                pInst->nrSetting.mode = mode ;
                sr = SIPX_RESULT_SUCCESS;
            }
        }
    }
    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNoiseReductionMode(const SIPX_INST hInst,
                                                        SIPX_NOISE_REDUCTION_MODE& mode) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetNoiseReductionMode");
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
            "sipxAudioGetNoiseReductionMode hInst=%p",
            hInst);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
               pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        if (pInterface && !pInst->nrSetting.bInitialized)
        {
            MEDIA_NOISE_REDUCTION_MODE nrMode ;
            if (pInterface->getAudioNoiseReductionMode(nrMode) == OS_SUCCESS)
            {
                pInst->nrSetting.bInitialized = true;
                pInst->nrSetting.mode = (SIPX_NOISE_REDUCTION_MODE) nrMode ;
                mode = (SIPX_NOISE_REDUCTION_MODE) nrMode ;

                sr = SIPX_RESULT_SUCCESS;
            }
        }
        else
        {
            mode = pInst->nrSetting.mode  ;
            sr = SIPX_RESULT_SUCCESS;
        }
    }
    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     size_t& numDevices)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetNumInputDevices");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetNumInputDevices hInst=%p",
        hInst);

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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetInputDevice");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetInputDevice hInst=%p index=%d",
        hInst, index);
        
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetNumOutputDevices");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetNumOutputDevices hInst=%p",
        hInst);
        
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetOutputDevice");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioGetOutputDevice hInst=%p index=%d",
        hInst, index);
        
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetCallInputDevice");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioSetCallInputDevice hInst=%p device=%s",
        hInst, szDevice);
        
    UtlString oldDevice ;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;    
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        // Get existing device
        OsStatus status = pInterface->getMicrophoneDevice(oldDevice) ;
        //assert(status == OS_SUCCESS) ;

        // Lazy Init
        if (!pInst->micSetting.bInitialized)
        {
            initMicSettings(&pInst->micSetting) ;
            assert(pInst->micSetting.bInitialized) ;
        }

        if (strcasecmp(szDevice, "NONE") == 0)
        {
            strcpy(pInst->micSetting.device, "NONE") ;
            status = pInterface->setMicrophoneDevice(pInst->micSetting.device) ;            
            assert(status == OS_SUCCESS || status == OS_NOT_YET_IMPLEMENTED) ;
            rc = (status==OS_SUCCESS) ? SIPX_RESULT_SUCCESS :
                 (status==OS_NOT_YET_IMPLEMENTED) ? SIPX_RESULT_NOT_IMPLEMENTED :
                 SIPX_RESULT_FAILURE;
        }
        else
        {
            for (int i=0; i<MAX_AUDIO_DEVICES; i++)
            {
                if (pInst->inputAudioDevices[i])
                {
                    // Get rid of whitespace
                    UtlString requestedDevice(szDevice);
                    requestedDevice.strip(UtlString::both);
                    UtlString availableDevice(pInst->inputAudioDevices[i]);
                    availableDevice.strip(UtlString::both);

                    // Allow sloppy match where requested device string is a substring
                    // of the actual device name
                    if (requestedDevice.length() > 2 &&
                        availableDevice.index(requestedDevice) == 0)
                    {
                        // Match
                        if (strcmp(szDevice, oldDevice) != 0)
                        {
                            strncpy(pInst->micSetting.device, pInst->inputAudioDevices[i], sizeof(pInst->micSetting.device)-1) ;
                            status = pInterface->setMicrophoneDevice(pInst->micSetting.device) ;
                            // GIPS returns -1 on the call to set audio input device, no matter what
                            //assert(status == OS_SUCCESS) ;
                        }
                        rc = (status==OS_SUCCESS) ? SIPX_RESULT_SUCCESS :
                             (status==OS_NOT_YET_IMPLEMENTED) ? SIPX_RESULT_NOT_IMPLEMENTED :
                             SIPX_RESULT_FAILURE;
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
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetRingerOutputDevice(const SIPX_INST hInst,
                                                        const char* szDevice)
{    
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetRingerOutputDevice");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioSetRingerOutputDevice hInst=%p device=%s",
        hInst, szDevice);
        
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
    UtlString oldDevice ; 

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        // Lazy Init settings
        if (!pInst->speakerSettings[RINGER].bInitialized)
        {
            initSpeakerSettings(&pInst->speakerSettings[RINGER]) ;
            assert(pInst->speakerSettings[RINGER].bInitialized) ;
        }

        if (strcasecmp(szDevice, "NONE") == 0)
        {
            strcpy(pInst->speakerSettings[RINGER].device, "NONE") ;
            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            for (int i=0; i<MAX_AUDIO_DEVICES; i++)
            {
                if (pInst->outputAudioDevices[i])
                {
                    // Get rid of whitespace
                    UtlString requestedDevice(szDevice);
                    requestedDevice.strip(UtlString::both);
                    UtlString availableDevice(pInst->outputAudioDevices[i]);
                    availableDevice.strip(UtlString::both);

                    // Allow sloppy match where requested device string is a substring
                    // of the actual device name
                    if (requestedDevice.length() > 2 &&
                        availableDevice.index(requestedDevice) == 0)
                    {
                        oldDevice = pInst->speakerSettings[RINGER].device ;
                        strncpy(pInst->speakerSettings[RINGER].device, pInst->outputAudioDevices[i],
                            sizeof(pInst->speakerSettings[RINGER].device)-1) ;
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

        // Set the device if it changed and this is the active device group
        if ((pInst->enabledSpeaker == RINGER) && 
                (oldDevice.compareTo(pInst->speakerSettings[RINGER].device) != 0))
        {
            if (pInterface->setSpeakerDevice(pInst->speakerSettings[RINGER].device) == OS_FAILED)
            {
                rc = SIPX_RESULT_FAILURE;
            }
        }       
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetCallOutputDevice(const SIPX_INST hInst,
                                                      const char* szDevice)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetCallOutputDevice");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxAudioSetCallOutputDevice hInst=%p device=%s",
        hInst, szDevice);
        
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;
    UtlString oldDevice ; 

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;

        // Lazy Init settings
        if (!pInst->speakerSettings[SPEAKER].bInitialized)
        {
            initSpeakerSettings(&pInst->speakerSettings[SPEAKER]) ;
            assert(pInst->speakerSettings[SPEAKER].bInitialized) ;
        }

        if (strcasecmp(szDevice, "NONE") == 0)
        {
            strcpy(pInst->speakerSettings[SPEAKER].device, "NONE") ;
            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            for (int i=0; i<MAX_AUDIO_DEVICES; i++)
            {
                if (pInst->outputAudioDevices[i])
                {
                    // Get rid of whitespace
                    UtlString requestedDevice(szDevice);
                    requestedDevice.strip(UtlString::both);
                    UtlString availableDevice(pInst->outputAudioDevices[i]);
                    availableDevice.strip(UtlString::both);

                    // Allow sloppy match where requested device string is a substring
                    // of the actual device name
                    if (requestedDevice.length() > 2 &&
                        availableDevice.index(requestedDevice) == 0)
                    {
                        oldDevice = pInst->speakerSettings[SPEAKER].device;
                        strncpy(pInst->speakerSettings[SPEAKER].device, pInst->outputAudioDevices[i], 
                                sizeof(pInst->speakerSettings[SPEAKER].device)-1);

                        printf("target device \"%s\" matches[%d] \"%s\"\n",
                            szDevice, i, pInst->outputAudioDevices[i]);

                        rc = SIPX_RESULT_SUCCESS ;
                        break ;
                    }
                    else
                    {
                        printf("target device \"%s\" does not match[%d] \"%s\"\n",
                            szDevice, i, pInst->outputAudioDevices[i]);
                    }
                }
                else
                {
                    // Hit end of list, might was well jump
                    break ;
                }
            }
        }

        // Set the device if it changed and this is the active device group
        if ((pInst->enabledSpeaker == SPEAKER) && 
            (oldDevice.compareTo(pInst->speakerSettings[SPEAKER].device) != 0))
        {
            OsStatus status = pInterface->setSpeakerDevice(pInst->speakerSettings[SPEAKER].device);
            rc = (status==OS_SUCCESS) ? SIPX_RESULT_SUCCESS :
                 (status==OS_NOT_YET_IMPLEMENTED) ? SIPX_RESULT_NOT_IMPLEMENTED :
                 SIPX_RESULT_FAILURE;

        }
    }

    return rc ;
}


/****************************************************************************
 * Line Related Functions
 ***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineUrl,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_ID contactId)

{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAdd");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineAdd hInst=%p lineUrl=%s, phLine=%p contactId=%d ",
        hInst, szLineUrl, phLine, contactId);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(szLineUrl != NULL) ;
    assert(phLine != NULL) ;


    if (pInst)
    {
        if (szLineUrl && phLine)
        {
            UtlString userId ;
            Url url(szLineUrl) ;
            Url uri = url ;
            uri.removeFieldParameters() ;
            uri.removeHeaderParameters() ;
            url.getUserId(userId) ;
            
            SipLine line(url, uri, userId) ;

            // Set the preferred contact
            Url uriPreferredContact ;
            SIPX_CONTACT_ADDRESS* pContact = NULL;
            SIPX_CONTACT_TYPE contactType = CONTACT_AUTO;
            
            pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
            if (pContact)
            {
                contactType = pContact->eContactType;            
            }
            SIPX_TRANSPORT_TYPE sipx_protocol = TRANSPORT_UDP;

            UtlString lineUrl(szLineUrl);
            if (lineUrl.contains("sips:") || lineUrl.contains("transport=tls"))
            {
                sipx_protocol = TRANSPORT_TLS;
            }
            else if (lineUrl.contains("transport=tcp"))
            {
                sipx_protocol = TRANSPORT_TCP;
            }
            sipxGetContactHostPort(pInst, (SIPX_CONTACT_TYPE)contactType, uriPreferredContact, sipx_protocol) ;
            line.setPreferredContactUri(uriPreferredContact) ;

            UtlBoolean bRC = pInst->pLineManager->addLine(line, false) ;
            if (bRC)
            {
                SIPX_LINE_DATA* pData = createLineData(pInst, uri) ;
                if (pData != NULL)
                {
                    pData->contactId = contactId ;
                    pData->contactType = (SIPX_CONTACT_TYPE) contactType ;
                    *phLine = gpLineHandleMap->allocHandle(pData) ;
                    sr = SIPX_RESULT_SUCCESS ;

                    pInst->pLineManager->setStateForLine(uri, SipLine::LINE_STATE_PROVISIONED) ;
                    sipxFireLineEvent(pInst->pRefreshManager, 
                            szLineUrl, 
                            LINESTATE_PROVISIONED, 
                            LINESTATE_PROVISIONED_NORMAL);
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
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxLineAdd hLine=%d", *phLine);
    
    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxLineAddAlias(const SIPX_LINE hLine, const char* szLineURL) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAddAlias");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineAddAlias hLine=%d szLineURL=%s",
        hLine, szLineURL);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    if (hLine)
    {
        SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData)
        {
            if (pData->pLineAliases == NULL)
            {
                pData->pLineAliases = new UtlSList() ;
            }

            Url url(szLineURL) ;
            UtlString strURI;
            url.getUri(strURI) ;
            Url uri(strURI) ;
            UtlString userId ;
            url.getUserId(userId) ;
            UtlString displayName;
            url.getDisplayName(displayName);
            uri.setDisplayName(displayName);

            pData->pLineAliases->append(new UtlVoidPtr(new Url(uri))) ;
            
            sipxLineReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;

            sr = SIPX_RESULT_SUCCESS ;
        }                
    }

    return sr;
}

SIPXTAPI_API SIPX_RESULT sipxLineRegister(const SIPX_LINE hLine, const bool bRegister)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineRegister");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineRegister hLine=%d bRegister=%d",
        hLine, bRegister);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    if (hLine)
    {
        SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger) ;
        if (!pData)
            return sr;

        if (bRegister)
        {
            pData->pInst->pLineManager->enableLine(*pData->lineURI);
        }
        else
        {
            pData->pInst->pLineManager->disableLine(*pData->lineURI,
                                                    false,
                                                    pData->lineURI->toString());//  ->unRegisterUser(*pData->lineURI, false, pData->lineURI->toString());
        }
        sr = SIPX_RESULT_SUCCESS ;
        sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
    }

    return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineRemove");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineRemove hLine=%d",
        hLine);    
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;

    if (hLine)
    {
        SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE, stackLogger) ;
        if (pData)
        {
            pData->pInst->pLineManager->deleteLine(*pData->lineURI) ;
            sipxLineReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger) ;
            sipxLineObjectFree(hLine) ;
            
            sr = SIPX_RESULT_SUCCESS ;
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


SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(const SIPX_LINE hLine,                                                 
                                                const char* szUserID,
                                                const char* szPasswd,
                                                const char* szRealm)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAddCredential");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineAddCredential hLine=%d userId=%s realm=%s",
        hLine, szUserID, szRealm);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger) ;
    if (pData)
    {
        if (szUserID && szPasswd)
        {
            UtlBoolean rc = pData->pInst->pLineManager->addCredentialForLine(*pData->lineURI,
                    szRealm,
                    szUserID,
                    szPasswd,
                    HTTP_DIGEST_AUTHENTICATION) ;

            // Add credential fails for two common reasons: 1) the line handle does not exist
            // which is unlikely at this point as we have already looked up the line info, or 2)
            // a credential set exists for this realm.  So try removing the credential for this 
            // realm and try adding again
            if(!rc)
            {
                UtlString id;
                assert(pData->lineURI);
                pData->lineURI->getIdentity(id);
                OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                              "addCredentialForLine failed for line: %s, trying to delete credential and add again",
                              id.data());
                rc = pData->pInst->pLineManager->deleteCredentialForLine(*pData->lineURI,
                    szRealm);

                // If the credential was not removed, don't bother trying to add it again.
                if(rc)
                {
                    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                                  "deleteCredentialForLine deleted credential, trying to add again");
                    rc = pData->pInst->pLineManager->addCredentialForLine(*pData->lineURI,
                        szRealm,
                        szUserID,
                        szPasswd,
                        HTTP_DIGEST_AUTHENTICATION);
                }
                else
                {
                    OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                                  "deleteCredentialForLine failed to remove old credential");
                }
            }
            if (rc)
            {
                sr = SIPX_RESULT_SUCCESS;
            }
            else
            {
                UtlString id;
                assert(pData->lineURI);
                pData->lineURI->getIdentity(id);
                OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                              "addCredentialForLine failed for line: %s", id.data());
                assert(rc);
            }
        }
        else
        {
            sr = SIPX_RESULT_INVALID_ARGS ;
        }
        sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGet");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineGet hInst=%p",
        hInst);
        
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
                    lines[i] = sipxLineLookupHandleByURI(pLines[i].getIdentity().toString()) ;
                }
            }
            delete [] pLines ;            
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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGetURI");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineGetURI hLine=%d",
        hLine);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger) ;
    if (pData)
    {
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

        sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
    }

    return sr ;
}

SIPXTAPI_API SIPX_RESULT sipxLineGetContactId(const SIPX_LINE  hLine,
                                              SIPX_CONTACT_ID& contactId)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGetContactId");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, 
            "sipxLineGetContactId hLine=%d", hLine);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger) ;
    if (pData)
    {
        contactId = pData->contactId ;
        sr = SIPX_RESULT_SUCCESS ;

        sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger) ;
    }  

    return sr ;
}


SIPXTAPI_API SIPX_RESULT sipxLineFindByURI(const SIPX_INST hInst,
                                           const char* szURI,
                                           SIPX_LINE&  hLine) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineFindByURI");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxLineFindByURI hInst=%p szURI=%s", hInst, szURI);
        
    SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS ;

    if (hInst && szURI)
    {
        hLine = sipxLineLookupHandleByURI(szURI) ;
        if (hLine != SIPX_LINE_NULL)
            sr = SIPX_RESULT_SUCCESS ;
        else
            sr = SIPX_RESULT_FAILURE ;
    }

    return sr ;
}



SIPXTAPI_API SIPX_RESULT sipxConfigSetLogLevel(SIPX_LOG_LEVEL logLevel) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetLogLevel");
    
#ifndef LOG_TO_FILE
    // Start up logger thread
    initLogger() ;

    logLevel = (logLevel == LOG_LEVEL_NONE) ? LOG_LEVEL_EMERG : logLevel;
    OsSysLog::setLoggingPriority((OsSysLogPriority) logLevel) ;  
#endif    

    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetLogFile(const char* szFilename)
{
#ifndef LOG_TO_FILE
    OsSysLog::setOutputFile(0, szFilename) ;
#endif   
    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetLogCallback(sipxLogCallback pCallback)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetLogCallback");

#ifndef LOG_TO_FILE
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    if ( OsSysLog::setCallbackFunction(pCallback) == OS_SUCCESS )
    {
        rc = SIPX_RESULT_SUCCESS;
    }
    return rc;
#else
    return SIPX_RESULT_SUCCESS;
#endif        
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableGIPSTracing(SIPX_INST hInst, 
                                                     bool bEnable) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableGIPSTracing");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigEnableGIPSTracing hInst=%p bEnable=%d", hInst, bEnable);

#ifdef VOICE_ENGINE
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    if (pInst)
    {
        VoiceEngineFactoryImpl* pInterface = dynamic_cast<VoiceEngineFactoryImpl*>(
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation());
        if (pInterface)
        {
            pInterface->setGipsTracing(bEnable) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        rc = SIPX_RESULT_INVALID_ARGS ;
    }
#else
    SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED ;
#endif

    return rc ;      
}



SIPXTAPI_API SIPX_RESULT sipxConfigSetMicAudioHook(fnMicAudioHook hookProc) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetMicAudioHook");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetMicAudio hookProc=%p",
        hookProc);

#ifndef VOICE_ENGINE
    // TODO - call MediaInterface for hook data
    MprFromMic::s_fnMicDataHook = hookProc ;
#endif

    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetSpkrAudioHook(fnSpkrAudioHook hookProc)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetSpkrAudioHook");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetSpkrAudioHook hookProc=%p",
        hookProc);

#ifndef VOICE_ENGINE
    // TODO - call MediaInterface for hook data
    MprToSpkr::s_fnToSpeakerHook = hookProc ;
#endif

    return SIPX_RESULT_SUCCESS ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetEnergyLevelNotificationPeriod(const SIPX_INST hInst,
                                                                    int periodMs)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetEnergyLevelNotificationPeriod");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetEnergyLevelNotificationPeriod hInst=%p periodMs=%d", hInst, periodMs);
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    SIPX_RESULT error = SIPX_RESULT_INVALID_ARGS;
    if(periodMs >= 0 && pInst)
    {
        error = SIPX_RESULT_SUCCESS;
        pInst->nEnergyLevelNotificationPeriodMs = periodMs;
    }
    return(error);
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundProxy(const SIPX_INST hInst,
                                                    const char* szProxy)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetOutboundProxy");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetOutboundProxy hInst=%p proxy=%s",
        hInst, szProxy);

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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetDnsSrvTimeouts");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetDnsSrvTimeouts initialTimeoutInSecs=%d retries=%d",
        initialTimeoutInSecs, retries);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    assert(initialTimeoutInSecs > 0) ;
    assert(retries > 0) ;

    if ((initialTimeoutInSecs > 0) && (retries > 0))
    {
        SipSrvLookup::setDnsSrvTimeouts(initialTimeoutInSecs, retries) ;
        rc = SIPX_RESULT_SUCCESS ;
    }
    /*
    else
    {
        SipSrvLookup::setOption(OptionCodeIgnoreSRV, 1);
        SipSvrLookup::setDnsSrvTimeouts(
    }
    */

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterResponseWaitSeconds(const SIPX_INST hInst,
                                                                  const int seconds)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetRegisterResponseWaitSeconds");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetRegisterResponseWaitSeconds hInst=%p seconds=%d",
        hInst, seconds);
        
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    if (hInst)
    {
        SIPX_INSTANCE_DATA* pData = (SIPX_INSTANCE_DATA*) hInst;
        
        if (pData->pSipUserAgent)
        {
            pData->pSipUserAgent->setRegisterResponseTimeout(seconds);
            rc = SIPX_RESULT_SUCCESS;
        }
    }
    
    return rc;
}  
 
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRport(const SIPX_INST hInst,
                                               const bool bEnable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableRport");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableRport hInst=%p bEnable=%d",
        hInst, bEnable);

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

SIPXTAPI_API SIPX_RESULT sipxConfigEnableRportMapping(const SIPX_INST hInst,
                                                      const bool bEnable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableRportMapping");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableRportMapping hInst=%p bEnable=%d",
        hInst, bEnable);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setUseRportMapping(bEnable) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetUserAgentName(const SIPX_INST hInst,
                                                    const char* szName,
                                                    const bool bIncludePlatform)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetUserAgentName");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetUserAgentName hInst=%p szName=%s",
        hInst, szName);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    assert(pInst) ;
    if (pInst)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setIncludePlatformInUserAgentName(bIncludePlatform);
            pInst->pSipUserAgent->setUserAgentName(szName) ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterExpiration(const SIPX_INST hInst,
                                                      const int nRegisterExpirationSecs)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetRegisterExpiration");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetRegisterExpiration hInst=%p seconds=%d",
        hInst, nRegisterExpirationSecs);

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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetSubscribeExpiration");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetSubscribeExpiration hInst=%p seconds=%d",
        hInst, nSubscribeExpirationSecs);

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
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetDnsSrvFailoverTimeout");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetDnsSrvFailoverTimeout hInst=%p seconds=%d",
        hInst, failoverTimeoutInSecs);

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



SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer,
                                              int iServerPort,
                                              int iKeepAliveInSecs)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableStun");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableStun hInst=%p server=%s:%d keepalive=%d",
        hInst, szServer, iServerPort, iKeepAliveInSecs);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {
        // A bit of hackery; If someone calls this multiple times
        // while a STUN response is outstanding, we don't want to
        // whack the existing pNotification (cause we will likely
        // crash).  So, only create/add a new notification object
        // if the current one is NULL.  The notification object
        // is only created here and destroy on reception of the 
        // event and/or destruction of the SIPX_INST handle.
        OsNotification* pNotification = pInst->pStunNotification ;
        if (pNotification == NULL)
        {
            pNotification = new OsQueuedEvent(*pInst->pMessageObserver->getMessageQueue(), SIPXMO_NOTIFICATION_STUN) ;
            pInst->pStunNotification = pNotification ;
        }

        pInst->pCallManager->enableStun(szServer, iServerPort, iKeepAliveInSecs, pNotification) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigDisableStun(const SIPX_INST hInst)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigDisableStun");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigDisableStun hInst=%p",
        hInst);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        pInst->pCallManager->enableStun(NULL, PORT_NONE, 0) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigEnableTurn(const SIPX_INST hInst,
                                              const char*     szServer,
                                              const int       iServerPort,
                                              const char*     szUsername,
                                              const char*     szPassword,
                                              const int       iKeepAliveInSecs) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableTurn");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableTurn hInst=%p server=%s:%d keepalive=%d",
        hInst, szServer, iServerPort, iKeepAliveInSecs);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {      
        pInst->pCallManager->enableTurn(szServer, iServerPort, szUsername, 
                szPassword, iKeepAliveInSecs) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;    
}


SIPXTAPI_API SIPX_RESULT sipxConfigDisableTurn(const SIPX_INST hInst) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigDisableTurn");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigDisableTurn hInst=%p",
        hInst);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {
        pInst->pCallManager->enableTurn(NULL, PORT_NONE, NULL, NULL, 0) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigEnableIce(const SIPX_INST hInst)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableIce");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableICE hInst=%p", hInst);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {      
        pInst->pCallManager->enableIce(true) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;    
}


SIPXTAPI_API SIPX_RESULT sipxConfigDisableIce(const SIPX_INST hInst) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigDisableIce");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigDisableICE hInst=%p", hInst);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    assert(pInst);
    if (pInst)
    {
        pInst->pCallManager->enableIce(false) ;
        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc ;    

}


SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveAdd(const SIPX_INST     hInst,
                                                SIPX_CONTACT_ID     contactId,
                                                SIPX_KEEPALIVE_TYPE type,
                                                const char*         remoteIp,
                                                int                 remotePort,
                                                int                 keepAliveSecs) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigKeepAliveAdd");
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    UtlString localSocket = "0.0.0.0" ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigKeepAliveAdd hInst=%p type=%d target=%s:%d keepAlive=%d",
            hInst,
            type,
            remoteIp ? remoteIp : "<NULL>",
            remotePort,
            keepAliveSecs) ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    assert(remoteIp) ;
    assert(remotePort > 0) ;
    assert(type >= SIPX_KEEPALIVE_CRLF && type <= SIPX_KEEPALIVE_SIP_OPTIONS) ;
    assert(keepAliveSecs > -2) ;

    if (pInst)
    {
        SIPX_CONTACT_ADDRESS* pContact = NULL;        
        if (contactId > 0)
        {
            pContact = pInst->pSipUserAgent->getContactDb().getLocalContact(contactId);
            if (pContact)
            {
                localSocket = pContact->cIpAddress ;
            }
        }

        switch (type)
        {
            case SIPX_KEEPALIVE_CRLF:
                if (pInst->pSipUserAgent->addCrLfKeepAlive(localSocket, 
                        remoteIp, remotePort, keepAliveSecs, 
                        pInst->pKeepaliveDispatcher))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_STUN:
                if (pInst->pSipUserAgent->addStunKeepAlive(localSocket, 
                        remoteIp, remotePort, keepAliveSecs, 
                        pInst->pKeepaliveDispatcher))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_SIP_PING:
                if (pInst->pSipUserAgent->addSipKeepAlive(localSocket,
                        remoteIp, remotePort, "PING", keepAliveSecs, 
                        pInst->pKeepaliveDispatcher))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_SIP_OPTIONS:
                if (pInst->pSipUserAgent->addSipKeepAlive(localSocket, 
                        remoteIp, remotePort, "OPTIONS", keepAliveSecs, 
                        pInst->pKeepaliveDispatcher))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ; 
            default:
                break ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveRemove(const SIPX_INST     hInst,
                                                   SIPX_CONTACT_ID     contactId,
                                                   SIPX_KEEPALIVE_TYPE type,
                                                   const char*         remoteIp,
                                                   int                 remotePort) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigKeepAliveRemove");
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    UtlString localSocket = "0.0.0.0" ;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigKeepAliveRemove hInst=%p target=%s:%d",
            hInst,
            remoteIp ? remoteIp : "<NULL>",
            remotePort) ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst) ;
    assert(remoteIp) ;
    assert(remotePort > 0) ;
    assert(type >= SIPX_KEEPALIVE_CRLF && type <= SIPX_KEEPALIVE_SIP_OPTIONS) ;

    if (pInst && remoteIp && remotePort > 0)
    {
        SIPX_CONTACT_ADDRESS* pContact = NULL;        
        if (contactId > 0)
        {
            pContact = pInst->pSipUserAgent->getContactDb().getLocalContact(contactId);
            if (pContact)
            {
                localSocket = pContact->cIpAddress ;
            }
        }

        switch (type)
        {
            case SIPX_KEEPALIVE_CRLF:
                if (pInst->pSipUserAgent->removeCrLfKeepAlive(localSocket, 
                        remoteIp, remotePort))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_STUN:
                if (pInst->pSipUserAgent->removeStunKeepAlive(localSocket, 
                        remoteIp, remotePort))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_SIP_PING:
                if (pInst->pSipUserAgent->removeSipKeepAlive(localSocket,
                        remoteIp, remotePort, "PING"))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ;
            case SIPX_KEEPALIVE_SIP_OPTIONS:
                if (pInst->pSipUserAgent->removeSipKeepAlive(localSocket, 
                        remoteIp, remotePort, "OPTIONS"))
                {
                    rc = SIPX_RESULT_SUCCESS ;
                }
                else
                {
                    rc = SIPX_RESULT_FAILURE ;
                }
                break ; 
            default:
                break ;
        }
    }
    else
    {
        rc = SIPX_RESULT_INVALID_ARGS;
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxConfigGetVersion(char* szVersion,
											  const size_t nBuffer)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetVersion");
    SIPX_RESULT rc = SIPX_RESULT_INSUFFICIENT_BUFFER;
    size_t nLen;

    if (szVersion)
    {
        memset(szVersion, 0, nBuffer);
        // Determine length of version string 
        nLen = (strlen(SIPXTAPI_VERSION_STRING)-8) +
               strlen(SIPXTAPI_VERSION) + 
               strlen(SIPXTAPI_BUILDNUMBER) + 
               strlen(SIPXTAPI_BUILDDATE) + 4;
        if (nLen <= nBuffer)
        {
            sprintf(szVersion, SIPXTAPI_VERSION_STRING, SIPXTAPI_VERSION,
                                                        SIPXTAPI_BUILDNUMBER,
#ifdef _DEBUG
                                                        "Dbg",
#else
                                                        "Rls",
#endif
                                                        SIPXTAPI_BUILDDATE);

            rc = SIPX_RESULT_SUCCESS;
        }
    }
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipUdpPort(SIPX_INST hInst, int* pPort) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetLocalSipUdpPort");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetLocalSipUdpPort hInst=%p",
        hInst);
        
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;    
    if (pInst && pPort)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent)
        {
            *pPort = pInst->pSipUserAgent->getUdpPort() ;
            if (portIsValid(*pPort))
            {
                rc = SIPX_RESULT_SUCCESS ;
            }
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTcpPort(SIPX_INST hInst, int* pPort) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetLocalSipTcpPort");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetLocalSipTcpPort hInst=%p",
        hInst);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;    
    if (pInst && pPort)
    {
        rc = SIPX_RESULT_FAILURE ;
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent)
        {
            *pPort = pInst->pSipUserAgent->getTcpPort() ;
            if (portIsValid(*pPort))
            {
                rc = SIPX_RESULT_SUCCESS ;
            }
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTlsPort(SIPX_INST hInst, int* pPort) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetLocalSipTlsPort");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetLocalSipTlsPort hInst=%p",
        hInst);

    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;    
    if (pInst && pPort)
    {
        assert(pInst->pSipUserAgent) ;
        if (pInst->pSipUserAgent)
        {
            *pPort = pInst->pSipUserAgent->getTlsPort() ;
            if (portIsValid(*pPort))
            {
                rc = SIPX_RESULT_SUCCESS ;
            }
        }
    }
    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecPreferences(const SIPX_INST hInst, 
                                                            const SIPX_AUDIO_BANDWIDTH_ID bandWidth)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetAudioCodecPreferences");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetAudioCodecPreferences hInst=%p bandWidth=%d",
            hInst, bandWidth);

    if (pInst)
    {
        int numCodecs;
        SdpCodec** codecsArray = NULL;
        UtlString codecName;
        int iRejected;

        // Check if bandwidth is legal, do not allow variable bandwidth
        if (bandWidth >= AUDIO_CODEC_BW_LOW && bandWidth <= AUDIO_CODEC_BW_HIGH)
        {
            CpMediaInterfaceFactoryImpl* pInterface = 
                    pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

            *pInst->audioCodecSetting.pPreferences = "";

            if (pInterface)
            {
                int codecIndex;

                /* Unconditionally rebuild codec factory with all supported codecs. If we 
                * don't do this first then only the previously preferred codecs will be used to
                * build the new factory -> that doesn't work for changing from a lower bandwidth to
                * a higher bandwidth.
                */
                pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                              "", // No audio preferences
                                              *pInst->videoCodecSetting.pPreferences, // Keep video prefs
                                              -1, // Allow all formats
                                              &iRejected);

                // Now pick preferences out of all available codecs
                pInst->pCodecFactory->getCodecs(numCodecs, codecsArray, "audio");

                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                              "sipxConfigSetAudioCodecPreferences number of Codec = %d for hInst=%p",
                              numCodecs, hInst);
                              
                for (int i=0; i<numCodecs; i++)
                {
                    if (codecsArray[i]->getBWCost() <= bandWidth)
                    {
                        if (SdpDefaultCodecFactory::getCodecNameByType(codecsArray[i]->getCodecType(),
                                                                       codecName
                                                                      ) == OS_SUCCESS)
                        {
                            *pInst->audioCodecSetting.pPreferences = 
                                *pInst->audioCodecSetting.pPreferences + " " + codecName;
                        }
                    }
                }
                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                        "sipxConfigSetAudioCodecPreferences: %s", pInst->audioCodecSetting.pPreferences->data());

                if (pInst->audioCodecSetting.pPreferences->length() != 0)
                {
                    // Did we previously allocate a codecs array and store it in our settings?
                    if (pInst->audioCodecSetting.bInitialized)
                    {
                        // Free up the previuosly allocated codecs and the array
                        for (codecIndex = 0; codecIndex < pInst->audioCodecSetting.numCodecs; codecIndex++)
                        {
                            if (pInst->audioCodecSetting.sdpCodecArray[codecIndex])
                            {
                                delete pInst->audioCodecSetting.sdpCodecArray[codecIndex];
                                pInst->audioCodecSetting.sdpCodecArray[codecIndex] = NULL;
                            }
                        }
                        delete[] pInst->audioCodecSetting.sdpCodecArray;
                        pInst->audioCodecSetting.sdpCodecArray = NULL;
                    }
                    pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                                  *pInst->audioCodecSetting.pPreferences,
                                                  *pInst->videoCodecSetting.pPreferences,
                                                  -1, // Allow all formats
                                                  &iRejected);

                    // We've rebuilt the factory, so get the new count of codecs
                    pInst->pCodecFactory->getCodecs(pInst->audioCodecSetting.numCodecs,
                                                    pInst->audioCodecSetting.sdpCodecArray,
                                                    "audio");
                    pInst->audioCodecSetting.fallBack = bandWidth;
                    pInst->audioCodecSetting.codecPref = bandWidth;
                    pInst->audioCodecSetting.bInitialized = true;
                    rc = SIPX_RESULT_SUCCESS;
                }
                else
                {
                    // Resetting the codec preferences failed but we've already rebuilt the factory
                    // with all codecs - go to the fallback preferences and rebuild again but return failure
                    OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                            "sipxConfigSetAudioCodecPreferences: Setting %d failed, falling back to preference %d", 
                            bandWidth, pInst->audioCodecSetting.fallBack);
                    sipxConfigSetAudioCodecPreferences(hInst, pInst->audioCodecSetting.fallBack);
                }

                // Free up the codecs and the array
                for (codecIndex = 0; codecIndex < numCodecs; codecIndex++)
                {
                    delete codecsArray[codecIndex];
                    codecsArray[codecIndex] = NULL;
                }
                delete[] codecsArray;
                codecsArray = NULL;
            }
        }
        else
        {
            rc = SIPX_RESULT_INVALID_ARGS ;
        }
    }

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecByName(const SIPX_INST hInst, 
                                                       const char* szCodecName)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetAudioCodecByName");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetAudioCodecByName hInst=%p codec=%s",
            hInst, szCodecName);

    if (pInst)
    {
        int iRejected;

        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

        *pInst->audioCodecSetting.pPreferences = szCodecName;
        *pInst->audioCodecSetting.pPreferences += " telephone-event";

        if (pInterface)
        {
            if (pInst->audioCodecSetting.pPreferences->length() != 0)
            {
                // Did we previously allocate a codecs array and store it in our settings?
                if (pInst->audioCodecSetting.bInitialized)
                {
                    int codecIndex;

                    // Free up the previuosly allocated codecs and the array
                    for (codecIndex = 0; codecIndex < pInst->audioCodecSetting.numCodecs; codecIndex++)
                    {
                        if (pInst->audioCodecSetting.sdpCodecArray[codecIndex])
                        {
                            delete pInst->audioCodecSetting.sdpCodecArray[codecIndex];
                            pInst->audioCodecSetting.sdpCodecArray[codecIndex] = NULL;
                        }
                    }
                    delete[] pInst->audioCodecSetting.sdpCodecArray;
                    pInst->audioCodecSetting.sdpCodecArray = NULL;
                }
                pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                              *pInst->audioCodecSetting.pPreferences,
                                              *pInst->videoCodecSetting.pPreferences,
                                              -1, // Allow all formats
                                              &iRejected);

                // We've rebuilt the factory, so get the new count of codecs
                pInst->pCodecFactory->getCodecs(pInst->audioCodecSetting.numCodecs,
                                                pInst->audioCodecSetting.sdpCodecArray,
                                                "audio");
                if (pInst->audioCodecSetting.numCodecs > 1)
                {
                    pInst->audioCodecSetting.codecPref = AUDIO_CODEC_BW_CUSTOM;
                    rc = SIPX_RESULT_SUCCESS;
                }
                else
                {
                    // Codec setting by name failed - we have an empty (except for DTMF) codec factory.
                    // Fall back to previously set bandwidth Id
                    OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                            "sipxConfigSetAudioCodecByName: Setting %s failed, falling back to preference %d", 
                            szCodecName, pInst->audioCodecSetting.fallBack);
                    sipxConfigSetAudioCodecPreferences(hInst, pInst->audioCodecSetting.fallBack);
                }
                pInst->audioCodecSetting.bInitialized = true;

            }

        }
    }

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodecPreferences(const SIPX_INST hInst, 
                                                            SIPX_AUDIO_BANDWIDTH_ID *pBandWidth)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetAudioCodecPreferences");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    if (pInst && pInst->audioCodecSetting.bInitialized)
    {
        *pBandWidth = pInst->audioCodecSetting.codecPref;
        rc = SIPX_RESULT_SUCCESS;
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetAudioCodecPreferences hInst=%p bandWidth=%d",
        hInst, *pBandWidth);

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetNumAudioCodecs(const SIPX_INST hInst, 
                                                     int* pNumCodecs)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetNumAudioCodecs");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;    

    if (pInst && pNumCodecs)
    {
        if (pInst->audioCodecSetting.bInitialized)
        {
            *pNumCodecs = pInst->audioCodecSetting.numCodecs;
        }
        else
        {
            *pNumCodecs = 0;
        }
        rc = SIPX_RESULT_SUCCESS;
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetNumAudioCodecs hInst=%p numCodecs=%d",
        hInst, *pNumCodecs);

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodec(const SIPX_INST hInst, 
                                                 const int index, 
                                                 SIPX_AUDIO_CODEC* pCodec)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetAudioCodec");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    UtlString codecName;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    assert(pCodec);

    if (pInst && pCodec)
    {
        assert(pInst->audioCodecSetting.bInitialized);

        memset((void*)pCodec, 0, sizeof(SIPX_AUDIO_CODEC));
        if (index >= 0 && index < pInst->audioCodecSetting.numCodecs)
        {
            //CpMediaInterfaceFactoryImpl* pInterface = 
            //    pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

            // If a name is found for the codec type, copy name and bandwidth cost
            if (SdpDefaultCodecFactory::getCodecNameByType(pInst->audioCodecSetting.sdpCodecArray[index]->getCodecType(),
                                                           codecName))
            {
                strncpy(pCodec->cName, codecName, SIPXTAPI_CODEC_NAMELEN-1);
                pCodec->iBandWidth = 
                    (SIPX_AUDIO_BANDWIDTH_ID)pInst->audioCodecSetting.sdpCodecArray[index]->getBWCost();

                rc = SIPX_RESULT_SUCCESS;
            }                   
        }
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetAudioCodec hInst=%p index=%d, codec-%s",
        hInst, index, codecName.data());

    return rc;
}



SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodecPreferences(const SIPX_INST hInst, 
                                                            SIPX_VIDEO_BANDWIDTH_ID *pBandWidth)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetVideoCodecPreferences");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    if (pInst && pInst->videoCodecSetting.bInitialized)
    {
        *pBandWidth = pInst->videoCodecSetting.codecPref;
        rc = SIPX_RESULT_SUCCESS;
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVideoCodecPreferences hInst=%p bandWidth=%d",
        hInst, *pBandWidth);

    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoBandwidth(const SIPX_INST hInst, 
                                                     SIPX_VIDEO_BANDWIDTH_ID bandWidth)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoBandwidth");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoBandwidth hInst=%p bandWidth=%d",
            hInst, bandWidth);

    if (pInst)
    {
        if (!pInst->videoCodecSetting.bInitialized)
        {
            pInst->pCodecFactory->getCodecs(pInst->videoCodecSetting.numCodecs,
                                            pInst->videoCodecSetting.sdpCodecArray,
                                            "video");
            pInst->videoCodecSetting.bInitialized = true;
        }
        // Check if bandwidth is legal, do not allow variable bandwidth
        if (bandWidth >= VIDEO_CODEC_BW_LOW && bandWidth <= VIDEO_CODEC_BW_HIGH)
        {
    
            CpMediaInterfaceFactoryImpl* pImpl = 
                            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
            int frameRate = 0;
            pImpl->getVideoFrameRate(frameRate);

            pInst->videoCodecSetting.codecPref = bandWidth;
            switch (bandWidth)
            {
            case VIDEO_CODEC_BW_LOW:
                sipxConfigSetVideoParameters(hInst, 5, 10);
                break;
            case VIDEO_CODEC_BW_NORMAL:
                sipxConfigSetVideoParameters(hInst, 70, frameRate);
                break;
            case VIDEO_CODEC_BW_HIGH:
                sipxConfigSetVideoParameters(hInst, 400, frameRate);
                break;
            }
            rc = SIPX_RESULT_SUCCESS;
        }
    }
    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevices(const SIPX_INST hInst,
                                                          char** arrSzCaptureDevices,
                                                          int nDeviceStringLength,
                                                          int nArrayLength)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetVideoCaptureDevices");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigGetVideoCaptureDevices hInst=%p",
            hInst);
            

    char *pTemp = (char*)arrSzCaptureDevices;
    memset(arrSzCaptureDevices, 0, nDeviceStringLength * nArrayLength);          
    if (pInst && pInst->pCallManager)
    {
        CpMediaInterfaceFactoryImpl* pImpl = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
        if (pImpl)
        {
            UtlSList captureDevices; 
            
            if (OS_SUCCESS == pImpl->getVideoCaptureDevices(captureDevices))
            {
                UtlSListIterator iterator(captureDevices);
                UtlString* pDevice;
                int index = 0;
                while ((pDevice = (UtlString*)iterator()))
                {
                    if (pDevice->length())
                    {
                        strncpy(pTemp, pDevice->data(),
                        pDevice->length() > nDeviceStringLength ? 
                            nDeviceStringLength : pDevice->length()  );
                    }
                    index++;
                    pTemp += nDeviceStringLength;
                    if (index > nArrayLength)
                    {
                        break;
                    }
                }
                rc = SIPX_RESULT_SUCCESS;
            }
        }                
    }    
    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}                                                          

SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevice(const SIPX_INST hInst,
                                                         char* szCaptureDevice,
                                                         int   nLength)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetVideoCaptureDevice");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigGetVideoCaptureDevice hInst=%p",
            hInst);
            
    if (pInst && pInst->pCallManager)
    {
        CpMediaInterfaceFactoryImpl* pImpl = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
        if (pImpl)
        {
            UtlString captureDevice; 
            
            if (OS_SUCCESS == pImpl->getVideoCaptureDevice(captureDevice))
            {
                strncpy(szCaptureDevice, captureDevice.data(), 
                        captureDevice.length() > nLength ? 
                        nLength : captureDevice.length());
                rc = SIPX_RESULT_SUCCESS;
                
            }
        }                
    }    
    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}            

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCaptureDevice(const SIPX_INST hInst,
                                                         const char* szCaptureDevice)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoCaptureDevice");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoCaptureDevice hInst=%p device=%s",
            hInst, szCaptureDevice);
            
    if (pInst && pInst->pCallManager)
    {
        CpMediaInterfaceFactoryImpl* pImpl = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
        if (pImpl)
        {
            UtlString captureDevice(szCaptureDevice); 
            
            if (OS_SUCCESS == pImpl->setVideoCaptureDevice(captureDevice))
            {
                rc = SIPX_RESULT_SUCCESS;
            }
        }                
    }    
    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}                                                         

SIPXTAPI_API SIPX_RESULT sipxConfigGetNumVideoCodecs(const SIPX_INST hInst, 
                                                     int* pNumCodecs)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetNumVideoCodecs");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;    

    if (pInst && pNumCodecs)
    {
        assert(pInst->videoCodecSetting.bInitialized);

        if (pInst->videoCodecSetting.bInitialized)
        {
            *pNumCodecs = pInst->videoCodecSetting.numCodecs;
            rc = SIPX_RESULT_SUCCESS;
        }
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetNumVideoCodecs hInst=%p numCodecs=%d",
        hInst, *pNumCodecs);

    return rc;
#else
    if (pNumCodecs)
    {
        *pNumCodecs = 0 ;        
    }
    return SIPX_RESULT_SUCCESS ;
#endif
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodec(const SIPX_INST hInst, 
                                                 const int index, 
                                                 SIPX_VIDEO_CODEC* pCodec)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetVideoCodec");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    UtlString codecName;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    assert(pCodec);
    if (pInst && pCodec)
    {
        assert(pInst->videoCodecSetting.bInitialized);

        memset((void*)pCodec, 0, sizeof(SIPX_VIDEO_CODEC));
        if (index >= 0 && index < pInst->videoCodecSetting.numCodecs)
        {
            CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

            // If a name is found for the codec type, copy name and bandwidth cost
            if (SdpDefaultCodecFactory::getCodecNameByType(pInst->videoCodecSetting.sdpCodecArray[index]->getCodecType(),
                                                           codecName))
            {
                strncpy(pCodec->cName, codecName, SIPXTAPI_CODEC_NAMELEN-1);
                pCodec->iBandWidth = 
                    (SIPX_VIDEO_BANDWIDTH_ID)pInst->videoCodecSetting.sdpCodecArray[index]->getBWCost();

                rc = SIPX_RESULT_SUCCESS;
            }                   
        }
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVideoCodec hInst=%p index=%d, codec-%s",
        hInst, index, codecName.data());

    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCodecByName(const SIPX_INST hInst, 
                                                       const char* szCodecName)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoCodecByName");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoCodecByName hInst=%p codec=%s",
            hInst, szCodecName);

    if (pInst)
    {
        int iRejected;

        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

        *pInst->videoCodecSetting.pPreferences = szCodecName;

        if (pInterface)
        {
            if (pInst->videoCodecSetting.pPreferences->length() != 0)
            {
                // Did we previously allocate a codecs array and store it in our settings?
                if (pInst->videoCodecSetting.bInitialized)
                {
                    int codecIndex;

                    // Free up the previously allocated codecs and the array
                    for (codecIndex = 0; codecIndex < pInst->videoCodecSetting.numCodecs; codecIndex++)
                    {
                        if (pInst->videoCodecSetting.sdpCodecArray[codecIndex])
                        {
                            delete pInst->videoCodecSetting.sdpCodecArray[codecIndex];
                            pInst->videoCodecSetting.sdpCodecArray[codecIndex] = NULL;
                        }
                    }
                    delete[] pInst->videoCodecSetting.sdpCodecArray;
                    pInst->videoCodecSetting.sdpCodecArray = NULL;
                }
                pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                              *pInst->audioCodecSetting.pPreferences,
                                              *pInst->videoCodecSetting.pPreferences,
                                              -1, // Allow all formats
                                              &iRejected);

                // We've rebuilt the factory, so get the new count of codecs
                pInst->pCodecFactory->getCodecs(pInst->videoCodecSetting.numCodecs,
                                                pInst->videoCodecSetting.sdpCodecArray,
                                                "video");
                if (pInst->videoCodecSetting.numCodecs > 0)
                {
                    rc = SIPX_RESULT_SUCCESS;
                }
                else
                {
                    // Codec setting by name failed - we have an empty codec factory.
                    // Fall back to previously set bandwidth Id
                    OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                            "sipxConfigSetVideoCodecByName: Setting %s failed, falling back to preference %d", 
                            szCodecName, pInst->videoCodecSetting.fallBack);
                    //sipxConfigSetVideoCodecPreferences(hInst, pInst->audioCodecSetting.fallBack);
                }
                pInst->videoCodecSetting.bInitialized = true;

            }
        }
    }

    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}


SIPXTAPI_API SIPX_RESULT sipxConfigResetVideoCodecs(const SIPX_INST hInst)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigResetVideoCodecs");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigResetVideoCodecs hInst=%p", hInst);

    if (pInst)
    {
        int iRejected;

        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

        if (pInterface)
        {
            // Did we previously allocate a codecs array and store it in our settings?
            if (pInst->videoCodecSetting.bInitialized)
            {
                int codecIndex;

                // Free up the previously allocated codecs and the array
                for (codecIndex = 0; codecIndex < pInst->videoCodecSetting.numCodecs; codecIndex++)
                {
                    if (pInst->videoCodecSetting.sdpCodecArray[codecIndex])
                    {
                        delete pInst->videoCodecSetting.sdpCodecArray[codecIndex];
                        pInst->videoCodecSetting.sdpCodecArray[codecIndex] = NULL;
                    }
                }
                delete[] pInst->videoCodecSetting.sdpCodecArray;
                pInst->videoCodecSetting.sdpCodecArray = NULL;
            }
            // Rebuild with all video codecs
            pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                          *pInst->audioCodecSetting.pPreferences,
                                          "",
                                          -1, // Allow all formats
                                          &iRejected);

            // We've rebuilt the factory, so get the new count of codecs
            pInst->pCodecFactory->getCodecs(pInst->videoCodecSetting.numCodecs,
                                            pInst->videoCodecSetting.sdpCodecArray,
                                            "video");
            if (pInst->videoCodecSetting.numCodecs > 0)
            {
                rc = SIPX_RESULT_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                    "sipxConfigResetVideoCodecs: Setting failed!!");
            }
            pInst->videoCodecSetting.bInitialized = true;
        }
    }

    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFormat(const SIPX_INST hInst,
                                                  SIPX_VIDEO_FORMAT videoFormat)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoFormat");

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;   

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoFormat hInst=%p videoFormat=%d", hInst, videoFormat);

    if (pInst)
    {
        CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();
        if (pInterface)
        {
            int iRejected;
            // Did we previously allocate a codecs array and store it in our settings?
            if (pInst->videoCodecSetting.bInitialized)
            {
                int codecIndex;

                // Free up the previuosly allocated codecs and the array
                for (codecIndex = 0; codecIndex < pInst->videoCodecSetting.numCodecs; codecIndex++)
                {
                    if (pInst->videoCodecSetting.sdpCodecArray[codecIndex])
                    {
                        delete pInst->videoCodecSetting.sdpCodecArray[codecIndex];
                        pInst->videoCodecSetting.sdpCodecArray[codecIndex] = NULL;
                    }
                }
                delete[] pInst->videoCodecSetting.sdpCodecArray;
                pInst->videoCodecSetting.sdpCodecArray = NULL;
            }
            // Rebuild with limited video format
            pInterface->buildCodecFactory(pInst->pCodecFactory, 
                                          *pInst->audioCodecSetting.pPreferences,
                                          "",
                                          videoFormat,
                                          &iRejected);

            // We've rebuilt the factory, so get the new count of codecs
            pInst->pCodecFactory->getCodecs(pInst->videoCodecSetting.numCodecs,
                                            pInst->videoCodecSetting.sdpCodecArray,
                                            "video");
            if (pInst->videoCodecSetting.numCodecs > 0)
            {
                rc = SIPX_RESULT_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                    "sipxConfigResetVideoCodecs: Setting failed!!");
            }
            pInst->videoCodecSetting.bInitialized = true;
        }
    }
    return rc;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableDnsSrv(const bool enable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableDnsSrv");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigEnableDnsSrv bEnable=%d",
            enable);
            
    UtlBoolean bEnable(enable);
    // The IgnoreSRV option has the opposite sense of bEnable.
    SipSrvLookup::setOption(SipSrvLookup::OptionCodeIgnoreSRV, !bEnable);
    
    return SIPX_RESULT_SUCCESS;
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableOutOfBandDTMF(const SIPX_INST hInst,
                                                       const bool bEnable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableOutOfBandDTMF");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableOutOfBandDTMF hInst=%p bEnbale=%d",
        hInst, bEnable);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

         if (pInterface)
         {
             if (pInterface->enableOutOfBandDTMF(bEnable) == OS_SUCCESS)
             {
                 rc = SIPX_RESULT_SUCCESS;
             }

         }
    }
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableInBandDTMF(const SIPX_INST hInst,
                                                    const bool bEnable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableInBandDTMF");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableInBandDTMF hInst=%p bEnbale=%d",
        hInst, bEnable);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

         if (pInterface)
         {
             if (pInterface->enableInBandDTMF(bEnable) == OS_SUCCESS)
             {

                 rc = SIPX_RESULT_SUCCESS;
             }
         }
    }
    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigIsOutOfBandDTMFEnabled(const SIPX_INST hInst,
                                                          bool& enabled)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigIsOutOfBandDTMFEnabled");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    UtlBoolean bEnabled;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

         if (pInterface && pInterface->isOutOfBandDTMFEnabled(bEnabled) == OS_SUCCESS)
         {
             enabled = (bEnabled) ? true : false;
             rc = SIPX_RESULT_SUCCESS;
         }
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigIsOutOfBandDTMFEnabled hInst=%p enabled=%d",
        hInst, enabled);

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigIsInBandDTMFEnabled(const SIPX_INST hInst,
                                                       bool& enabled)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigIsInBandDTMFEnabled");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    UtlBoolean bEnabled;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

         if (pInterface && pInterface->isInBandDTMFEnabled(bEnabled) == OS_SUCCESS)
         {
             enabled = (bEnabled) ? true : false;
             rc = SIPX_RESULT_SUCCESS;
         }
    }
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigIsInBandDTMFEnabled hInst=%p enabled=%d",
        hInst, enabled);

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableRTCP(const SIPX_INST hInst,
                                              const bool bEnable) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableRTCP");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableRTCP hInst=%p enable=%d",
        hInst, bEnable);

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

         if (pInterface && (pInterface->enableRTCP(bEnable) == OS_SUCCESS))
         {             
             rc = SIPX_RESULT_SUCCESS;
         }
    }

    return rc;
}


// Old version of the SIPX_CONTACT_ADDRESS -- used to generate the 
// size if cbSize is 0.
typedef struct __OLD_SIPX_CONTACT_ADDRESS
{
    SIPX_CONTACT_ID     id ;
    SIPX_CONTACT_TYPE   eContactType ;  
    SIPX_TRANSPORT_TYPE eTransportType ;
    char                cInterface[32] ;
    char                cIpAddress[32] ;
    int                 iPort ;

} __OLD_SIPX_CONTACT_ADDRESS ;


SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalContacts(const SIPX_INST hInst,
                                                    SIPX_CONTACT_ADDRESS addresses[],
                                                    size_t nMaxAddresses,
                                                    size_t& nActualAddresses) 
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetLocalContacts");
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    UtlString address ;
    
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    assert(pInst->pSipUserAgent != NULL) ;
    nActualAddresses = 0 ;
    if (pInst && pInst->pSipUserAgent && nMaxAddresses > 0)
    {
        SIPX_CONTACT_ADDRESS* pOutAddress = &addresses[0] ;
        SIPX_CONTACT_ADDRESS* contacts[MAX_IP_ADDRESSES];
        int numContacts = 0;
        pInst->pSipUserAgent->getContactAddresses(contacts, numContacts);       
        
        // copy contact records
        for (unsigned int i = 0; (i < (unsigned int)numContacts) && (i < nMaxAddresses); i++)
        {
            *pOutAddress = *contacts[i];
            if (pOutAddress->eTransportType > TRANSPORT_CUSTOM)
            {
                pOutAddress->eTransportType = TRANSPORT_CUSTOM ;
            }

            if (pOutAddress->cbSize)
            {
                pOutAddress = (SIPX_CONTACT_ADDRESS*) 
                        (((char*) pOutAddress) + pOutAddress->cbSize) ;
            }
            else
            {
                pOutAddress = (SIPX_CONTACT_ADDRESS*) 
                        (((char*) pOutAddress) + sizeof(__OLD_SIPX_CONTACT_ADDRESS)) ;
            }

            OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                "sipxConfigGetLocalContacts index=%d contactId=%d contactType=%s transportType=%s port=%d address=%s adapter=%s",
                i,
                contacts[i]->id,
                sipxContactTypeToString(contacts[i]->eContactType),
                sipxTransportTypeToString(contacts[i]->eTransportType),
                contacts[i]->iPort,
                contacts[i]->cIpAddress,
                contacts[i]->cInterface);
            nActualAddresses++ ;
        }
        rc = SIPX_RESULT_SUCCESS;
    }
    else
    {
        rc = SIPX_RESULT_FAILURE ;
    }        
    return rc;
}
SIPXTAPI_API SIPX_RESULT sipxConfigAddContact(const SIPX_INST hInst,
                                              const char* szSipContactAddress,
                                              const int iSipContactPort,
                                              SIPX_CONTACT_ID& iNewContactId)
{
    iNewContactId = 0;
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
 
    if (pInst)
    {
        SipContactDb* contactDb = &(pInst->pSipUserAgent->getContactDb());

        if(contactDb && szSipContactAddress && *szSipContactAddress && (iSipContactPort > 0))
        {
            SIPX_CONTACT_ADDRESS spoofedContact;
            spoofedContact.iPort = iSipContactPort;
            strncpy(spoofedContact.cIpAddress, szSipContactAddress, 27);
            spoofedContact.cIpAddress[27] = '\0';
            spoofedContact.eContactType = CONTACT_CONFIG;
            bool contactAdded = contactDb->addContact(spoofedContact);
            OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigAddContact Spoofed address was %sadded, contactId: %d", contactAdded ? "" : "NOT ", 
                spoofedContact.id);
            iNewContactId = spoofedContact.id;
            rc = SIPX_RESULT_SUCCESS;

            CpMediaInterfaceFactoryImpl* pImpl =
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

            if(pImpl)
            {
                pImpl->setConfiguredIpAddress(szSipContactAddress);
            }
        }
    }

    return(rc);
}

SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalFeedbackAddress(const SIPX_INST hInst,
                                                           const char*     szRemoteIp,
                                                           const int       iRemotePort,
                                                           char*           szContactIp,
                                                           size_t          nContactIpLength,
                                                           int&            iContactPort,
                                                           int             iTimeoutMs) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ; 
    if (pInst && szRemoteIp && (iRemotePort > 0) && szContactIp)
    {
        memset(szContactIp, 0, nContactIpLength) ;        
        iContactPort = 0 ;

        UtlString contactAddress ;
        int       contactPort ;
        if (OsNatAgentTask::getInstance()->findContactAddress(
                szRemoteIp, iRemotePort,
                &contactAddress, &contactPort, iTimeoutMs))
        {
            strncpy(szContactIp, contactAddress.data(), nContactIpLength) ;
            iContactPort = contactPort ;
            rc = SIPX_RESULT_SUCCESS ;
        }
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigGetAllLocalNetworkIps(const char* arrAddresses[], const char* arrAddressAdapter[], int &numAddresses)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigGetAllLocalNetworkIps");
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    
    const HostAdapterAddress* utlAddresses[SIPX_MAX_IP_ADDRESSES];
    
    if (getAllLocalHostIps(utlAddresses, numAddresses))
    {
        rc = SIPX_RESULT_SUCCESS;

        for (int i = 0; i < numAddresses; i++)
        {
            char *szAddress = NULL;
            char *szAdapter = NULL;        
            szAddress = (char*)malloc(utlAddresses[i]->mAddress.length() + 1);
            szAdapter = (char*)malloc(utlAddresses[i]->mAdapter.length() + 1);
            strcpy(szAddress, utlAddresses[i]->mAddress.data());
            strcpy(szAdapter, utlAddresses[i]->mAdapter.data());
            arrAddresses[i] = szAddress;
            arrAddressAdapter[i] = szAdapter;
            
            OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                "sipxConfigGetAllLocalNetworkIps index=%d address=%s adapter=%s",
                i, arrAddresses[i], arrAddressAdapter[i]);
            delete utlAddresses[i];
        }
    }
    else
    {
        numAddresses = 0 ;
    }
    
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoPreviewDisplay(const SIPX_INST hInst, SIPX_VIDEO_DISPLAY* const pDisplay)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoPreviewDisplay");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoPreviewWindow hInst=%p, hDisplay=%p",
        hInst, pDisplay);
        
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    CpMediaInterfaceFactoryImpl* pImpl = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
           
    pImpl->setVideoPreviewDisplay(pDisplay);
    
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigUpdatePreviewWindow(const SIPX_INST hInst, const SIPX_WINDOW_HANDLE hWnd)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigUpdatePreviewWindow");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigUpdatePreviewWindow hInst=%p, hWnd=%p",
        hInst, hWnd);
        
#if defined(_WIN32) && defined(HAVE_GIPS)
    #include <windows.h>
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint((HWND)hWnd, &ps);
	GipsVideoEngineWindows* pVideoEngine = sipxConfigGetVideoEnginePtr(hInst);
	pVideoEngine->GIPSVideo_OnPaint(hdc);
	EndPaint((HWND)hWnd, &ps);
#endif
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif

}

SIPXTAPI_API SIPX_RESULT sipxCallResizeWindow(const SIPX_CALL hCall, const SIPX_WINDOW_HANDLE hWnd)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallResizeWindow");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigResizeWindow hCall=%d, hWnd=%p",
        hCall, hWnd);

    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
        
#if defined(_WIN32) && defined(HAVE_GIPS)
        #include <windows.h>

        if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
        {
    		GipsVideoEngineWindows* pVideoEngine = sipxConfigGetVideoEnginePtr(pInst);
            if (pVideoEngine)
            {
		        pVideoEngine->GIPSVideo_OnSize((HWND)hWnd);
                sr = SIPX_RESULT_SUCCESS;
            }
        }
#endif
    return sr;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif

}

SIPXTAPI_API SIPX_RESULT sipxCallUpdateVideoWindow(const SIPX_CALL hCall, const SIPX_WINDOW_HANDLE hWnd)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallUpdateVideoWindow");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxCallUpdateVideoWindow hCall=%d, hWnd=%p",
        hCall, hWnd);
        
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, NULL))
    {
        // for now, just call the sipxConfigUpdatePreviewWindow - it does the same thing
        sipxConfigUpdatePreviewWindow(pInst, hWnd);
    }
    
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoQuality(const SIPX_INST hInst, const SIPX_VIDEO_QUALITY_ID quality)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoQuality");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoQuality hInst=%p, quality=%d",
        hInst, quality);
    SIPX_RESULT sr = SIPX_RESULT_FAILURE ;

    if (quality>0 && quality<4)
    {
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
        
        CpMediaInterfaceFactoryImpl* pImpl = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
               
        pImpl->setVideoQuality(quality);
        sr = SIPX_RESULT_SUCCESS;
    }
    else
    {
        sr = SIPX_RESULT_INVALID_ARGS;
    }
    return sr;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif

}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoParameters(const SIPX_INST hInst,
                                                      const int bitRate,
                                                      const int frameRate)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoParameters");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoParameters hInst=%p, bitRate=%d, frameRate=%d",
        hInst, bitRate, frameRate);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    int localBitrate = bitRate;

    if (localBitrate < 5)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoParameters - Setting localBitrate to 5");
        localBitrate = 5;
    }
    else if (localBitrate > 400)
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigSetVideoParameters - Setting localBitrate to 400");
        localBitrate = 400;
    }
    
    CpMediaInterfaceFactoryImpl* pImpl = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
            
    pImpl->setVideoParameters(localBitrate, frameRate);
  
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCpuUsage(const SIPX_INST hInst, 
                                                    const int cpuUsage)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoCpuUsage");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoCpuUsage hInst=%p, cpuUsage=%d",
        hInst, cpuUsage);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    CpMediaInterfaceFactoryImpl* pImpl = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
            
    pImpl->setVideoCpuValue(cpuUsage);
  
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoBitrate(const SIPX_INST hInst, 
                                                   const int bitRate)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoBitrate");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoBitrate hInst=%p, bitRate=%d",
         hInst, bitRate);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    CpMediaInterfaceFactoryImpl* pImpl = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
            
    pImpl->setVideoBitrate(bitRate);
  
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFramerate(const SIPX_INST hInst, 
                                                     const int frameRate)
{
#ifdef VIDEO
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetVideoFramerate");

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVideoFramerate hInst=%p, frameRate=%d",
         hInst, frameRate);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    CpMediaInterfaceFactoryImpl* pImpl = 
            pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation() ;
            
    pImpl->setVideoFramerate(frameRate);
  
    return SIPX_RESULT_SUCCESS;
#else
    return SIPX_RESULT_NOT_SUPPORTED ;
#endif

}

SIPXTAPI_API SIPX_RESULT sipxConfigSetSecurityParameters(const SIPX_INST hInst,
                                                         const char* szDbLocation,
                                                         const char* szMyCertNickname,
                                                         const char* szDbPassword)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetSecurityParameters");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetSecurityParameters hInst=%p, dbLocation=%s",
        hInst, szDbLocation);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    if (pInst)
    {
        strncpy(pInst->dbLocation, szDbLocation, sizeof(pInst->dbLocation));
        strncpy(pInst->myCertNickname, szMyCertNickname, sizeof(pInst->myCertNickname));
        strncpy(pInst->dbPassword, szDbPassword, sizeof(pInst->dbPassword));
    }
    return SIPX_RESULT_SUCCESS;
}                                                                                                 

SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipShortNames(const SIPX_INST hInst, 
                                                       const bool bEnabled)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableSipShortNames");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableSipShortNames hInst=%p, bEnabled=%d",
        hInst, bEnabled);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ; 

    if (pInst)
    {
        pInst->bShortNames = bEnabled;
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setHeaderOptions(pInst->bAllowHeader, pInst->bDateHeader, pInst->bShortNames, pInst->szAcceptLanguage);
            rc = SIPX_RESULT_SUCCESS;
        }
    }    
    return rc;
}                                                       

SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipDateHeader(const SIPX_INST hInst, 
                                                       const bool bEnabled)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableSipDateHeader");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableSipDateHeader hInst=%p, bEnabled=%d",
        hInst, bEnabled);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
        pInst->bDateHeader = bEnabled;
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setHeaderOptions(pInst->bAllowHeader, pInst->bDateHeader, pInst->bShortNames, pInst->szAcceptLanguage);
            rc = SIPX_RESULT_SUCCESS;
        }
    }    
    return rc;
}                                                       

SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipAllowHeader(const SIPX_INST hInst, 
                                                       const bool bEnabled)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableSipAllowHeader");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableSipAllowHeader hInst=%p, bEnabled=%d",
        hInst, bEnabled);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
        pInst->bAllowHeader = bEnabled;
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setHeaderOptions(pInst->bAllowHeader, pInst->bDateHeader, pInst->bShortNames, pInst->szAcceptLanguage);
            rc = SIPX_RESULT_SUCCESS;
        }
    }    
    return rc;
}                                                       

SIPXTAPI_API SIPX_RESULT sipxConfigSetSipAcceptLanguage(const SIPX_INST hInst, 
                                                        const char* szLanguage)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetSipAcceptLanguage");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetSipAcceptLanguage hInst=%p, szLanguage=%s",
        hInst, szLanguage);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
        strncpy(pInst->szAcceptLanguage, szLanguage, 16);
        if (pInst->pSipUserAgent)
        {
            pInst->pSipUserAgent->setHeaderOptions(pInst->bAllowHeader, pInst->bDateHeader, pInst->bShortNames, pInst->szAcceptLanguage);
            rc = SIPX_RESULT_SUCCESS;
        }
    }    
    return rc;
} 

SIPXTAPI_API SIPX_RESULT sipxConfigSetLocationHeader(const SIPX_INST hInst, 
                                                     const char* szHeader)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetLocationHeader");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetLocationHeader hInst=%p, szHeader=%s",
        hInst, szHeader);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst && szHeader)
    {
        strncpy(pInst->szLocationHeader, szHeader, 255);
    }    
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigSetConnectionIdleTimeout(const SIPX_INST hInst,
                                                            const int idleTimeout)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSetConnectionIdleTimeout");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetSetConnectionIdleTimeout hInst=%p, idleTimeout==%d",
        hInst, idleTimeout);

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  

    if (pInst)
    {
         CpMediaInterfaceFactoryImpl* pInterface = 
                pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();

        if (pInterface)
        {
            pInterface->setConnectionIdleTimeout(idleTimeout);
        }
        rc = SIPX_RESULT_SUCCESS;
    }
    return rc;
}

      
SIPXTAPI_API SIPX_RESULT sipxConfigExternalTransportAdd(SIPX_INST const           hInst,
                                                        SIPX_TRANSPORT&           hTransport,
                                                        const bool                bIsReliable,
                                                        const char*               szTransport,
                                                        const char*               szLocalIp,
                                                        const int                 iLocalPort,
                                                        SIPX_TRANSPORT_WRITE_PROC writeProc,
                                                        const char*               szLocalRoutingId,
                                                        const void*               pUserData)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_TRANSPORT_DATA* pData = new SIPX_TRANSPORT_DATA;

    assert(szTransport);
    assert(szTransport[0] != '\0');
    assert(writeProc);

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
            "sipxConfigExternalTransportAdd hInst=%p, reliable=%d, transport=%s, localIp=%s, localPort=%d, routingId=%s",
            hInst, bIsReliable, 
            szTransport ? szTransport : "<NULL>",
            szLocalIp ? szLocalIp : "<NULL>",
            iLocalPort,
            szLocalRoutingId ? szLocalRoutingId : "<NULL>") ;


    if (hInst)
    {
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst;
        pData->bIsReliable = bIsReliable;
        strncpy(pData->szTransport, szTransport, MAX_TRANSPORT_NAME - 1);
        strncpy(pData->szLocalIp, szLocalIp, 31);
        pData->iLocalPort = iLocalPort;
        pData->pFnWriteProc = writeProc;
        pData->pUserData = pUserData ;
        strncpy(pData->cRoutingId, szLocalRoutingId, sizeof(pData->cRoutingId)-1);

        pData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
        hTransport = gpTransportHandleMap->allocHandle(pData);
        
        SIPX_TRANSPORT_DATA* pLockedData = sipxTransportLookup(hTransport, SIPX_LOCK_WRITE);
        if (pLockedData)
        {
            pLockedData->hTransport = hTransport;
            pData->pInst->pSipUserAgent->addExternalTransport(pLockedData->szTransport, pLockedData);
            sipxTransportReleaseLock(pLockedData, SIPX_LOCK_WRITE);
        }
        
        pData->pInst->pSipUserAgent->getContactDb().replicateForTransport(TRANSPORT_UDP,
            (SIPX_TRANSPORT_TYPE) hTransport, szTransport, szLocalRoutingId);

        rc = SIPX_RESULT_SUCCESS;
    }
    return rc;
}    
SIPXTAPI_API SIPX_RESULT sipxConfigExternalTransportRemove(const SIPX_TRANSPORT hTransport)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, 
            "sipxConfigExternalTransportRemove") ;

    assert(hTransport);

    SIPX_TRANSPORT_DATA* pData = sipxTransportLookup(hTransport, SIPX_LOCK_READ);

    if (pData && pData->pInst && pData->pInst->pSipUserAgent)
    {
        pData->pInst->pSipUserAgent->removeExternalTransport(pData->szTransport, pData);
        pData->pInst->pSipUserAgent->getContactDb().removeForTransport(
            (SIPX_TRANSPORT_TYPE) hTransport);
        rc = SIPX_RESULT_SUCCESS;;
    }
    sipxTransportReleaseLock(pData, SIPX_LOCK_READ) ;

    sipxTransportObjectFree(hTransport);

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigExternalTransportRouteByUser(const SIPX_TRANSPORT hTransport,
                                                                bool                 bRouteByUser) 
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SIPX_TRANSPORT_DATA* pTransportData = sipxTransportLookup(hTransport, SIPX_LOCK_WRITE);
    if (pTransportData)
    {                 
        pTransportData->bRouteByUser = bRouteByUser ;
        rc = SIPX_RESULT_SUCCESS;
    }
    sipxTransportReleaseLock(pTransportData, SIPX_LOCK_WRITE) ;

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxConfigExternalTransportHandleMessage(const SIPX_TRANSPORT hTransport,
                                                                  const char*  szSourceIP,
                                                                  const int    iSourcePort,
                                                                  const char*  szLocalIP,
                                                                  const int    iLocalPort,
                                                                  const void*  pData,
                                                                  const size_t nData)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    SipMessage* message = new SipMessage((const char*)pData, nData);
    
    message->setFromThisSide(false);

    long epochDate;
    if(!message->getDateField(&epochDate))
    {
        message->setDateField();
    }

    message->setSendProtocol(OsSocket::CUSTOM);
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    
    message->setTransportTime(time.seconds());

    // Keep track of where this message came from
    message->setSendAddress(szSourceIP, iSourcePort);
    
    // Keep track of the interface on which this message was
    // received.               
    message->setLocalIp(szLocalIP);

    SIPX_TRANSPORT_DATA* pTransportData = sipxTransportLookup(hTransport, SIPX_LOCK_READ);

    if (pTransportData && pTransportData->pInst && pTransportData->pInst->pSipUserAgent)
    {                       
        // have the user-agent dispatch it

        if (OsSysLog::willLog(FAC_SIP_CUSTOM, PRI_DEBUG))
        {
            UtlString data((const char*) pData, nData) ;
            OsSysLog::add(FAC_SIP_CUSTOM, PRI_DEBUG, "[Received] From: %s To: %s \r\n%s\r\n", 
                    szSourceIP, szLocalIP, data.data()) ;
        }

        pTransportData->pInst->pSipUserAgent->dispatch(message, SipMessageEvent::APPLICATION, pTransportData);
        rc = SIPX_RESULT_SUCCESS;
    }
    sipxTransportReleaseLock(pTransportData, SIPX_LOCK_READ) ;

    return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSetVoiceQualityServer(const SIPX_INST hInst,
                                                         const char*     szServer) 
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigSetVoiceQualityServer Inst=%p target=%s", hInst, szServer);

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;      
    if (pInst)
    {
        pInst->pCallManager->setVoiceQualityReportTarget(szServer) ;
        rc = SIPX_RESULT_SUCCESS ;
    }
        
    return rc;
}                                                                

SIPXTAPI_API SIPX_RESULT sipxConfigPrepareToHibernate(const SIPX_INST hInst)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigPrepareToHibernate");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigPrepareToHibernate Inst=%p", hInst);

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;


/*
    // As if 2006-10-06 -- The timer subsystem has been rewritten and no 
    // longer supports the ability to restart timers -- work is needed
    // to add this (not sure how much), but until then, disabling this
    // functionality.

    OsTimer* pTimer;
    
    if (!gbHibernated)
    {
        gbHibernated = true;
        // hibernate singleton object timers
        // log file timer
        pTimer = OsSysLog::getTimer();
        if (pTimer)
        {
            pTimer->stop();
        }
        
        pTimer = OsNatAgentTask::getInstance()->getTimer();
        if (pTimer)
        {
            pTimer->stop();
        }
    }
    
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    if (pInst)
    {   
        pTimer = pInst->pRefreshManager->getTimer();
        if (pTimer)
        {
            pTimer->stop();
        }
        pTimer = pInst->pSipUserAgent->getTimer();
        if (pTimer)
        {
            pTimer->stop();        
        }
        pInst->pSipUserAgent->stopTransactionTimers();
        
        rc = SIPX_RESULT_SUCCESS;
    }
    */
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigUnHibernate(const SIPX_INST hInst)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigUnHibernate");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigUnHibernate Inst=%p", hInst);

    SIPX_RESULT rc = SIPX_RESULT_FAILURE;

/*
    // As if 2006-10-06 -- The timer subsystem has been rewritten and no 
    // longer supports the ability to restart timers -- work is needed
    // to add this (not sure how much), but until then, disabling this
    // functionality.

    OsTimer* pTimer;       
    if (gbHibernated)
    {
        gbHibernated = false;
        
        // UnHibernate singleton objects

        
        // log file timer
        pTimer = OsSysLog::getTimer();
        if (pTimer)
        {
            pTimer->start();        
        }
        
        pTimer = OsNatAgentTask::getInstance()->getTimer();
        if (pTimer)
        {
            pTimer->start();        
        }
    }
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    
    if (pInst)
    {   
        pTimer = pInst->pRefreshManager->getTimer();
        if (pTimer)
        {
            pTimer->start();        
        }
        pTimer = pInst->pSipUserAgent->getTimer();
        if (pTimer)
        {
            pTimer->start();        
        }
        pInst->pSipUserAgent->startTransactionTimers();
        
        rc = SIPX_RESULT_SUCCESS;
    }
    
*/
    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConfigEnableRtpOverTcp(const SIPX_INST hInst,
                                                    bool bEnable)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigEnableRtpOverTcp");
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigEnableRtpOverTcp Inst=%p bEnable=%d", hInst, bEnable);
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;  
    if (pInst)
    {   
        pInst->bRtpOverTcp = bEnable;
        rc = SIPX_RESULT_SUCCESS;
    }
    
    return rc;        
}                                                   


/**
 * Simple utility function to parse the username, host, and port from
 * a URL.  All url, field, and header parameters are ignored.  You may also 
 * specify NULL for any parameters (except szUrl) which are not needed.  
 * Lastly, the caller must allocate enough storage space for each url
 * component -- if in doubt use the length of the supplied szUrl.
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlParse(const char* szUrl,
                                          char* szUsername,
                                          char* szHostname,
                                          int*  iPort) 
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;    

    if (szUrl && strlen(szUrl))
    {
        Url url(szUrl) ;
        UtlString temp ;

        if (szUsername)
        {
            url.getUserId(temp) ;
            strcpy(szUsername, temp) ;
        }
        if (szHostname)
        {
            url.getHostAddress(temp) ;
            strcpy(szHostname, temp) ;
        }
        if (iPort)
        {
            *iPort = url.getHostPort() ;
        }

        rc = SIPX_RESULT_SUCCESS ;        
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxUtilUrlGetDisplayName(const char* szUrl,
                                                   char*       szDisplayName,
                                                   size_t      nDisplayName) 
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;    

    if (szUrl && strlen(szUrl))
    {
        Url url(szUrl) ;
        UtlString temp ;

        if (szDisplayName && nDisplayName)
        {
            url.getDisplayName(temp) ;
            temp.strip(UtlString::both, '\"') ;
            strncpy(szDisplayName, temp, nDisplayName) ;
        }

        rc = SIPX_RESULT_SUCCESS ;        
    }

    return rc ;
}



/**
 * Simple utility function to update a URL.  If the szUrl isn't large enough,
 * this function will fail.  Specify a NULL szUrl to request required length.
 * To leave an existing component unchanged, use NULL for strings and -1 for 
 * ports.
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlUpdate(char*       szUrl,
                                           size_t &    nUrl,
                                           const char* szNewUsername,
                                           const char* szNewHostname,
                                           const int   iNewPort) 
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;    
    UtlString   results ;

    if (szUrl)
    { 
        Url url(szUrl) ;
        
        if (szNewUsername)
            url.setUserId(szNewUsername) ;
        if (szNewHostname)
            url.setHostAddress(szNewHostname) ;
        if (iNewPort != -1)
            url.setHostPort(iNewPort) ;
        
        url.toString(results) ;

        if (szUrl && results.length() < nUrl) 
        {
            strcpy(szUrl, results) ;
            rc = SIPX_RESULT_SUCCESS ;
        }        
    }
    nUrl = results.length() + 1 ;

    return rc ;
}

static bool findUrlParameter(Url*        pUrl,
                             const char* szName,
                             size_t      index,
                             char*       szValue,
                             size_t      nValue)
{
    bool      bRC = false ;
    int       paramCount = 0 ;
    UtlString name ;
    UtlString value ;

    while (bRC == false && pUrl->getUrlParameter(paramCount, name, value))
    {
        if (name.compareTo(szName, UtlString::ignoreCase) == 0)
        {
            if (index == 0)
            {
                bRC = true ;
                strncpy(szValue, value.data(), nValue) ;
            }
            else
            {
                index-- ;
            }
        }
        paramCount++ ;
    }

    return bRC ;
}



SIPXTAPI_API SIPX_RESULT sipxUtilUrlGetUrlParam(const char* szUrl,
                                                const char* szParamName,
                                                size_t      nParamIndex,
                                                char*       szParamValue,
                                                size_t      nParamValue)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;    
    UtlString   results ;

    if (szUrl && szParamName && szParamValue)
    {
        UtlString value ;

        //  Assume addr-spec, but allow name-addr
        Url addrSpec(szUrl, true) ;
        Url nameAddr(szUrl, false) ;
        
        if (    findUrlParameter(&addrSpec, szParamName, nParamIndex, 
                szParamValue, nParamValue) ||
                findUrlParameter(&nameAddr, szParamName, nParamIndex, 
                szParamValue, nParamValue))
        {
            rc = SIPX_RESULT_SUCCESS ;
        }
        else
        {
            rc = SIPX_RESULT_FAILURE ;
        }
    }

    return rc ;
}

