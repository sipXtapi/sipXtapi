//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "include/sipXmediaFactoryImpl.h"
#include "include/CpPhoneMediaInterface.h"
#include "sdp/SdpCodec.h"
#include "mp/MpMediaTask.h"
#include "mp/MpMisc.h"
#include "mp/MpCodec.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/dmaTask.h"
#include "sdp/SdpCodecList.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"

#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
#include "mp/NetInTask.h"
#endif


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION

#define MAX_MANAGED_FLOW_GRAPHS           16

// STATIC VARIABLE INITIALIZATIONS
int sipXmediaFactoryImpl::miInstanceCount=0;

CpMediaInterfaceFactory* spFactory = NULL;
int siInstanceCount=0;

extern "C" CpMediaInterfaceFactory* cpDefaultMediaFactoryFactory(OsConfigDb* pConfigDb)
{
    // TODO: Add locking

    if (spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new sipXmediaFactoryImpl(pConfigDb));
    }    
    siInstanceCount++ ;
    
    // Assert some sane value
    assert(siInstanceCount < 11) ;
    return spFactory;
}

#ifndef DISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    return(cpDefaultMediaFactoryFactory(pConfigDb));
}
#endif

extern "C" void sipxDestroyMediaFactoryFactory()
{
    // TODO: Add locking

    if (siInstanceCount > 0)
    {
        siInstanceCount-- ;
        if (siInstanceCount == 0)
        {
            if (spFactory)
            {
                delete spFactory ;
                spFactory = NULL ;
            }
        }
    }
}


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
        pConfigDb->get("PHONESET_SEND_INBAND_DTMF", strInBandDTMF) ;
        strInBandDTMF.toUpper() ;

        OsSysLog::add(FAC_MP, PRI_DEBUG, 
                      "sipXmediaFactoryImpl::sipXmediaFactoryImpl maxFlowGraph = %d",
                      maxFlowGraph);
    }

    // Max Flow graphs
    if (maxFlowGraph <=0 ) 
    {
        maxFlowGraph = MAX_MANAGED_FLOW_GRAPHS;
    }

    // Start audio subsystem if still not started.
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
#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
        mpStartTasks();  
#else
        if (OS_SUCCESS != startNetInTask()) {
           OsSysLog::add(FAC_MP, PRI_ERR,
                         "Could not start NetInTask!!");
        }
#endif
    }

    miGain = 7 ;
    ++miInstanceCount;

    // We are missing synchronization -- give the tasks time to start
    OsTask::delay(100) ;
}


// Destructor
sipXmediaFactoryImpl::~sipXmediaFactoryImpl()
{
    // TODO: Shutdown
    --miInstanceCount;
    if (miInstanceCount == 0)
    {
#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
        mpStopTasks();
#else
        shutdownNetInTask();
#endif
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
                                                              int iStunPort,
                                                              int iStunKeepAliveSecs,
                                                              const char* szTurnServer,
                                                              int iTurnPort,
                                                              const char* szTurnUsername,
                                                              const char* szTurnPassword,
                                                              int iTurnKeepAlivePeriodSecs,
                                                              UtlBoolean bEnableICE) 
{
    return new CpPhoneMediaInterface(this, publicAddress, localAddress, 
            numCodecs, sdpCodecArray, locale, expeditedIpTos, szStunServer,
            iStunPort, iStunKeepAliveSecs, szTurnServer, iTurnPort, 
            szTurnUsername, szTurnPassword, iTurnKeepAlivePeriodSecs, 
            bEnableICE) ;
}


OsStatus sipXmediaFactoryImpl::setSpeakerVolume(int iVolume) 
{
    OsStatus rc = OS_SUCCESS ;
    MpCodec_setVolume(iVolume) ;

    return rc ;
}

OsStatus sipXmediaFactoryImpl::setSpeakerDevice(const UtlString& device) 
{
    OsStatus rc = OS_SUCCESS ;
    DmaTask::setCallDevice(device.data()) ;
    return rc ;    
}

OsStatus sipXmediaFactoryImpl::setMicrophoneGain(int iGain) 
{
    OsStatus rc ;

    miGain = iGain ;
    rc = MpCodec_setGain(miGain) ;
    return rc ;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneDevice(const UtlString& device) 
{
    OsStatus rc = OS_SUCCESS ;
    DmaTask::setInputDevice(device.data()) ;
#ifdef WIN32
    dmaSignalMicDeviceChange();
#endif
    return rc ;    
}

OsStatus sipXmediaFactoryImpl::muteMicrophone(UtlBoolean bMute) 
{
    if (bMute)
    {
        MpCodec_setGain(0) ;
    }
    else
    {
        MpCodec_setGain(miGain) ;
    }
    return OS_SUCCESS ;
}

OsStatus sipXmediaFactoryImpl::setAudioAECMode(const MEDIA_AEC_MODE mode)
{
  if (MpCallFlowGraph::setAECMode((FLOWGRAPH_AEC_MODE)mode)) {
    return OS_SUCCESS;
  }else {
    return OS_NOT_SUPPORTED; 
  }
}

OsStatus sipXmediaFactoryImpl::enableAGC(UtlBoolean bEnable) {
  if (MpCallFlowGraph::setAGC(bEnable)) {
    return OS_SUCCESS;
  }else {
    return OS_NOT_SUPPORTED; 
  }
}

OsStatus sipXmediaFactoryImpl::setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode) {
  if (mode == MEDIA_NOISE_REDUCTION_DISABLED) {
    if (MpCallFlowGraph::setAudioNoiseReduction(FALSE)) {
      return OS_SUCCESS;
    }
  } else {
    if (MpCallFlowGraph::setAudioNoiseReduction(TRUE)) {
      return OS_SUCCESS;
    }
  }
  return OS_NOT_SUPPORTED;
}

OsStatus sipXmediaFactoryImpl::buildCodecFactory(SdpCodecList*    pFactory, 
                                                 const UtlString& sAudioPreferences,
                                                 const UtlString& sVideoPreferences,
                                                 int videoFormat,
                                                 int* iRejected)
{
    OsStatus rc = OS_FAILED;

    *iRejected = 0;

    if (pFactory)
    {
        pFactory->clearCodecs();

        // If preferred codecs supplied - add them, else add all supported
        // codecs.
        if (sAudioPreferences.length() > 0)
        {
            *iRejected = pFactory->buildSdpCodecFactory(sAudioPreferences);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: supported codecs = %s with NumReject %d",
                          sAudioPreferences.data(), *iRejected);
                          
            rc = OS_SUCCESS;
        }
        else
        {
            // Build up the supported codecs
            SdpCodec::SdpCodecTypes audioCodecs[] = {
#ifdef HAVE_SPEEX // [
                          SdpCodec::SDP_CODEC_SPEEX,
                          SdpCodec::SDP_CODEC_SPEEX_5,
                          SdpCodec::SDP_CODEC_SPEEX_15,
                          SdpCodec::SDP_CODEC_SPEEX_24,
#endif // HAVE_SPEEX ]
#ifdef HAVE_GSM // [
                          SdpCodec::SDP_CODEC_GSM,
#endif // HAVE_GSM ]
#ifdef HAVE_ILBC // [
                          SdpCodec::SDP_CODEC_ILBC,
#endif // HAVE_ILBC ]
                          SdpCodec::SDP_CODEC_PCMU,
                          SdpCodec::SDP_CODEC_PCMA,
                          SdpCodec::SDP_CODEC_TONES};
            const int numAudioCodecs = sizeof(audioCodecs)/sizeof(SdpCodec::SdpCodecTypes);

            // Register all codecs
            *iRejected = pFactory->buildSdpCodecFactory(numAudioCodecs, audioCodecs);
            rc = OS_SUCCESS;
        }


#ifdef VIDEO // [
        // If preferred codecs supplied - add them, else add all supported
        // codecs.
        if (sVideoPreferences.length() > 0)
        {
            *iRejected = pFactory->buildSdpCodecFactory(sVideoPreferences);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: supported codecs = %s with NumReject %d",
                          sVideoPreferences.data(), *iRejected);

            rc = OS_SUCCESS;
        }
        else
        {
            // Build up the supported codecs
            SdpCodec::SdpCodecTypes videoCodecs[] = {};
            const int numVideoCodecs = sizeof(videoCodecs)/sizeof(SdpCodec::SdpCodecTypes);

            *iRejected = pFactory->buildSdpCodecFactory(numVideoCodecs, videoCodecs);
            rc = OS_SUCCESS;
        }
#endif // VIDEO ]

    }            

    return rc;
}


OsStatus sipXmediaFactoryImpl::updateVideoPreviewWindow(void* displayContext) 
{
    return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

OsStatus sipXmediaFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

    iVolume = MpCodec_getVolume() ;
    if (iVolume==-1) {
        rc = OS_FAILED;
        iVolume = 0;
    }
    return rc ;
}

OsStatus sipXmediaFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

    device = DmaTask::getCallDevice() ;
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsStatus rc = OS_SUCCESS ;

    iGain = MpCodec_getGain() ;
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

    device = DmaTask::getMicDevice() ;
    return rc ;
}


OsStatus sipXmediaFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::setVideoQuality(int quality)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoQuality(int& quality) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoBitRate(int& bitRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getLocalAudioConnectionId(int& connectionId) const 
{
    connectionId = -1 ;

    return OS_NOT_SUPPORTED ;

}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


