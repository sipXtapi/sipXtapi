//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#ifndef SIPXMEDIA_EXCLUDE
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "cp/CpMediaInterfaceFactory.h"
#include "cp/CpMediaInterfaceFactoryFactory.h"
#include "cp/sipXmediaFactoryImpl.h"
#include "cp/CpPhoneMediaInterface.h"
#include "net/SdpCodec.h"
#ifndef SIPXMEDIA_EXCLUDE
    #include "mp/MpMediaTask.h"
    #include "mp/MpMisc.h"
    #include "mp/MpCodec.h"
    #include "mp/MpCallFlowGraph.h"
    #include "mp/dmaTask.h"
    #include "net/SdpCodecFactory.h"
#endif
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION
CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    // special case:  because the call manager creates its own 
    // sipXmediaFactory, and sets its implementation as well, we 
    // can just return NULL here.  3rd party media libraries'
    // implementation of this method must create a Factory and set
    // the factory implementation, but we don't have to.
    return NULL;
}



#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           16
// STATIC VARIABLE INITIALIZATIONS
int sipXmediaFactoryImpl::miInstanceCount=0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
sipXmediaFactoryImpl::sipXmediaFactoryImpl(OsConfigDb* pConfigDb)
{    
    int maxFlowGraph = -1 ; 
    UtlString strInBandDTMF ;
    
    if (pConfigDb)
    {
        pConfigDb->get("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxFlowGraph) ;
        pConfigDb->get(CONFIG_PHONESET_SEND_INBAND_DTMF, strInBandDTMF) ;
        strInBandDTMF.toUpper() ;

    }

    // Max Flow graphs
    if (maxFlowGraph <=0 ) 
    {
        maxFlowGraph = MAX_MANAGED_FLOW_GRAPHS;
    }
    if (miInstanceCount == 0)
    {
        mpStartUp(8000, 80, 16*maxFlowGraph, pConfigDb);
    }

    // Should we send inband DTMF by default?    
    if (strInBandDTMF.compareTo("DISABLE") == 0)
    {
        MpCallFlowGraph::setInbandDTMF(false) ;
    }
    else
    {
        MpCallFlowGraph::setInbandDTMF(true) ;
    }

    // init the media processing task
    mpMediaTask = MpMediaTask::getMediaTask(maxFlowGraph); 

#ifdef INCLUDE_RTCP /* [ */
    mpiRTCPControl = CRTCManager::getRTCPControl();
#endif /* INCLUDE_RTCP ] */

    if (miInstanceCount == 0)
    {
        mpStartTasks();  
    }

    miGain = 7 ;
    ++miInstanceCount;
}


// Destructor
sipXmediaFactoryImpl::~sipXmediaFactoryImpl()
{
    // TODO: Shutdown
    --miInstanceCount;
    if (miInstanceCount == 0)
    {
        mpStopTasks();
        mpShutdown();
    }
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* sipXmediaFactoryImpl::createMediaInterface( const char* publicAddress,
                                                              const char* localAddress,
                                                              int numCodecs,
                                                              SdpCodec* sdpCodecArray[],
                                                              const char* locale,
                                                              int expeditedIpTos,
                                                              const char* szStunServer,
                                                              int iStunKeepAliveSecs 
                                                            ) 
{
    return new CpPhoneMediaInterface(this, publicAddress, localAddress, 
            numCodecs, sdpCodecArray, locale, expeditedIpTos, szStunServer,
            iStunKeepAliveSecs) ;
}


OsStatus sipXmediaFactoryImpl::setSpeakerVolume(int iVolume) 
{
    OsStatus rc = OS_SUCCESS ;
//#ifdef WIN32
    MpCodec_setVolume(iVolume) ;
//#endif

    return rc ;
}

OsStatus sipXmediaFactoryImpl::setSpeakerDevice(const UtlString& device) 
{
    OsStatus rc = OS_SUCCESS ;
#ifdef WIN32
    DmaTask::setCallDevice(device.data()) ;
#endif
    return rc ;    
}

OsStatus sipXmediaFactoryImpl::setMicrophoneGain(int iGain) 
{
    OsStatus rc ;

    miGain = iGain ;
#   ifdef WIN32
    rc = MpCodec_setGain(miGain) ;
#   else
    rc = OS_FAILED;
#   endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneDevice(const UtlString& device) 
{
    OsStatus rc = OS_SUCCESS ;
#ifdef WIN32
    DmaTask::setInputDevice(device.data()) ;
#endif
    return rc ;    
}

OsStatus sipXmediaFactoryImpl::muteMicrophone(UtlBoolean bMute) 
{
//#ifdef WIN32
    if (bMute)
    {
        MpCodec_setGain(0) ;
    }
    else
    {
        MpCodec_setGain(miGain) ;
    }
//#endif
    return OS_SUCCESS ;
}

OsStatus sipXmediaFactoryImpl::enableAudioAEC(UtlBoolean bEnable)
{
    return OS_SUCCESS;
}

OsStatus sipXmediaFactoryImpl::enableOutOfBandDTMF(UtlBoolean bEnable)
{
    return OS_SUCCESS;
}

OsStatus sipXmediaFactoryImpl::buildCodecFactory(SdpCodecFactory *pFactory, 
                                                 const UtlString& sPreferences,
                                                 int* iRejected)
{
    OsStatus rc = OS_FAILED;

    SdpCodec::SdpCodecTypes codecs[3];

#ifdef HAVE_GIPS /* [ */
    codecs[0] = SdpCodec::SDP_CODEC_GIPS_PCMU ;
    codecs[1] = SdpCodec::SDP_CODEC_GIPS_PCMA ;
#else /* HAVE_GIPS ] [ */
    codecs[0] = SdpCodec::SDP_CODEC_PCMU ;
    codecs[1] = SdpCodec::SDP_CODEC_PCMA ;
#endif /* HAVE_GIPS ] */
    codecs[2] = SdpCodec::SDP_CODEC_TONES ;
    if (pFactory)
    {
        pFactory->clearCodecs();
        *iRejected = pFactory->buildSdpCodecFactory(3, codecs);
        rc = OS_SUCCESS;
    }
    return rc;
}

/* ============================ ACCESSORS ================================= */

OsStatus sipXmediaFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

//#ifdef WIN32
    iVolume = MpCodec_getVolume() ;
//#endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    device = DmaTask::getCallDevice() ;
#endif
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    iGain = MpCodec_getGain() ;
#endif
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    device = DmaTask::getMicDevice() ;
#endif   
    return rc ;
}


OsStatus sipXmediaFactoryImpl::isAudioAECEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = false;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = false;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::getNumOfCodecs(int& iCodecs) const
{
    iCodecs = 3;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::getCodec(int iCodec, UtlString& codec, int &bandWidth) const
{
    OsStatus rc = OS_SUCCESS;

    switch (iCodec)
    {
#ifdef HAVE_GIPS /* [ */
    case 0: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMU;
        break;
    case 1: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMA;
        break;
#else /* HAVE_GIPS ] [ */
    case 0: codec = (const char*) SdpCodec::SDP_CODEC_PCMU;
        break;
    case 1: codec = (const char*) SdpCodec::SDP_CODEC_PCMA;
        break;
#endif /* HAVE_GIPS ] */
    case 2: codec = (const char*) SdpCodec::SDP_CODEC_TONES;
        break;
    default: rc = OS_FAILED;
    }

    return rc;
}

OsStatus sipXmediaFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes type, UtlString& codec) const
{
    OsStatus rc = OS_FAILED;

    codec = "";

    switch (type)
    {
    case SdpCodec::SDP_CODEC_TONES:
        codec = GIPS_CODEC_ID_TELEPHONE;
        break;
#ifdef HAVE_GIPS /* [ */
    case SdpCodec::SDP_CODEC_GIPS_PCMA:
        codec = GIPS_CODEC_ID_PCMA;
        break;
    case SdpCodec::SDP_CODEC_GIPS_PCMU:
        codec = GIPS_CODEC_ID_PCMU;
        break;
#else /* HAVE_GIPS ] [ */
    case SdpCodec::SDP_CODEC_PCMA:
        codec = GIPS_CODEC_ID_PCMA;
        break;
    case SdpCodec::SDP_CODEC_PCMU:
        codec = GIPS_CODEC_ID_PCMU;
        break;
#endif /* HAVE_GIPS ] */
    }

    if (codec != "")
    {
        rc = OS_SUCCESS;
    }

    return rc;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
#endif // #ifndef SIPXMEDIA_EXCLUDE

