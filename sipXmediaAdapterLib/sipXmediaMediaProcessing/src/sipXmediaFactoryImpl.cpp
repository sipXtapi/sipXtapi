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
typedef int MpConnectionID;
#include "mp/MpCallFlowGraph.h"
#include "mp/dmaTask.h"
#include "net/SdpCodecFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"

#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION

#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           16

// Audio codecs number calculation:

#define GENERIC_AUDIO_CODECS_NUM 3

#define GIPS_CODECS_BEGIN  GENERIC_AUDIO_CODECS_NUM
#ifdef HAVE_GIPS /* [ */
#define GIPS_AUDIO_CODECS_NUM 3
#else /* HAVE_GIPS ] [ */
#define GIPS_AUDIO_CODECS_NUM 0
#endif /* HAVE_GIPS ] */

#define SPEEX_AUDIO_CODECS_BEGIN  (GIPS_CODECS_BEGIN+GIPS_AUDIO_CODECS_NUM)
#ifdef HAVE_SPEEX /* [ */
#define SPEEX_AUDIO_CODECS_NUM 4
#else /* HAVE_SPEEX ] [ */
#define SPEEX_AUDIO_CODECS_NUM 0
#endif /* HAVE_SPEEX ] */

#define GSM_AUDIO_CODECS_BEGIN  (SPEEX_AUDIO_CODECS_BEGIN+SPEEX_AUDIO_CODECS_NUM)
#ifdef HAVE_GSM /* [ */
#define GSM_AUDIO_CODECS_NUM 1
#else /* HAVE_GSM ] [ */
#define GSM_AUDIO_CODECS_NUM 0
#endif /* HAVE_GSM ] */

#define ILBC_AUDIO_CODECS_BEGIN  (GSM_AUDIO_CODECS_BEGIN+GSM_AUDIO_CODECS_NUM)
#ifdef HAVE_ILBC /* [ */
#define ILBC_AUDIO_CODECS_NUM 1
#else /* HAVE_ILBC ] [ */
#define ILBC_AUDIO_CODECS_NUM 0
#endif /* HAVE_ILBC ] */
 

#define TOTAL_AUDIO_CODECS_NUM (ILBC_AUDIO_CODECS_BEGIN + ILBC_AUDIO_CODECS_NUM)

// Video codecs  number calculation:


#define TOTAL_VIDEO_CODECS_NUM 0

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
        pConfigDb->get(CONFIG_PHONESET_SEND_INBAND_DTMF, strInBandDTMF) ;
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
        mpStartTasks();  
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
        // Temporarily comment out this function because it causes the program hung.
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

OsStatus sipXmediaFactoryImpl::buildCodecFactory(SdpCodecFactory *pFactory, 
                                                 const UtlString& sAudioPreferences,
                                                 const UtlString& sVideoPreferences,
                                                 int videoFormat,
                                                 int* iRejected)
{
    OsStatus rc = OS_FAILED;

    UtlString codecName;
    UtlString codecList;

    SdpCodec::SdpCodecTypes codecs[TOTAL_AUDIO_CODECS_NUM+TOTAL_VIDEO_CODECS_NUM];

    *iRejected = 0;

    int numAudioCodecs = TOTAL_AUDIO_CODECS_NUM;
    SdpCodec::SdpCodecTypes *audioCodecs = codecs;
    int numVideoCodecs = TOTAL_VIDEO_CODECS_NUM;
    SdpCodec::SdpCodecTypes *videoCodecs = codecs+TOTAL_AUDIO_CODECS_NUM;

    codecs[0] = SdpCodec::SDP_CODEC_GIPS_PCMU;
    codecs[1] = SdpCodec::SDP_CODEC_GIPS_PCMA;
    codecs[2] = SdpCodec::SDP_CODEC_TONES;

#ifdef HAVE_GIPS /* [ */
    codecs[GIPS_CODECS_BEGIN+0] = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    codecs[GIPS_CODECS_BEGIN+1] = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    codecs[GIPS_CODECS_BEGIN+2] = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
#endif /* HAVE_GIPS ] */

#ifdef HAVE_SPEEX /* [ */
    codecs[SPEEX_AUDIO_CODECS_BEGIN+0] = SdpCodec::SDP_CODEC_SPEEX;
    codecs[SPEEX_AUDIO_CODECS_BEGIN+1] = SdpCodec::SDP_CODEC_SPEEX_5;
    codecs[SPEEX_AUDIO_CODECS_BEGIN+2] = SdpCodec::SDP_CODEC_SPEEX_15;
    codecs[SPEEX_AUDIO_CODECS_BEGIN+3] = SdpCodec::SDP_CODEC_SPEEX_24;
#endif /* HAVE_SPEEX ] */

#ifdef HAVE_GSM /* [ */
    codecs[GSM_AUDIO_CODECS_BEGIN+0] = SdpCodec::SDP_CODEC_GSM;
#endif /* HAVE_GSM ] */

#ifdef HAVE_ILBC /* [ */
    codecs[ILBC_AUDIO_CODECS_BEGIN+0] = SdpCodec::SDP_CODEC_ILBC;
#endif /* HAVE_ILBC ] */

    if (pFactory)
    {
        pFactory->clearCodecs();

        // add preferred audio codecs first
        if (sAudioPreferences.length() > 0)
        {
            UtlString references = sAudioPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(references);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: sReferences = %s with NumReject %d",
                           references.data(), *iRejected);
                           
            // Now pick preferences out of all available codecs
            SdpCodec** codecsArray = NULL;
            pFactory->getCodecs(numAudioCodecs, codecsArray);
            
            UtlString preferences;
            int i;
            for (i = 0; i < numAudioCodecs; i++)
            {
                if (getCodecNameByType(codecsArray[i]->getCodecType(), codecName) == OS_SUCCESS)
                {
                    preferences = preferences + " " + codecName;
                }
            }
            
            pFactory->clearCodecs();
            *iRejected = pFactory->buildSdpCodecFactory(preferences);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: supported codecs = %s with NumReject %d",
                          preferences.data(), *iRejected);
                          
            // Free up the codecs and the array
            for (i = 0; i < numAudioCodecs; i++)
            {
                delete codecsArray[i];
                codecsArray[i] = NULL;
            }
            delete[] codecsArray;
            codecsArray = NULL;
                          
            rc = OS_SUCCESS;
        }
        else
        {
            // Build up the supported codecs
            *iRejected = pFactory->buildSdpCodecFactory(numAudioCodecs, audioCodecs);
            rc = OS_SUCCESS;
        }


        // add preferred video codecs first
        if (sVideoPreferences.length() > 0)
        {
            UtlString references = sVideoPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(references);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: sReferences = %s with NumReject %d",
                           references.data(), *iRejected);
                           
            // Now pick preferences out of all available codecs
            SdpCodec** codecsArray = NULL;
            pFactory->getCodecs(numVideoCodecs, codecsArray);
            
            UtlString preferences;
            int i;
            for (i = 0; i < numVideoCodecs; i++)
            {
                if (getCodecNameByType(codecsArray[i]->getCodecType(), codecName) == OS_SUCCESS)
                {
                    preferences = preferences + " " + codecName;
                }
            }
            
            pFactory->clearCodecs();
            *iRejected = pFactory->buildSdpCodecFactory(preferences);
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "sipXmediaFactoryImpl::buildCodecFactory: supported codecs = %s with NumReject %d",
                          preferences.data(), *iRejected);
                          
            // Free up the codecs and the array
            for (i = 0; i < numVideoCodecs; i++)
            {
                delete codecsArray[i];
                codecsArray[i] = NULL;
            }
            delete[] codecsArray;
            codecsArray = NULL;
                          
            rc = OS_SUCCESS;
        }
        else
        {
            // Build up the supported codecs
            *iRejected = pFactory->buildSdpCodecFactory(numVideoCodecs, videoCodecs);
            rc = OS_SUCCESS;
        }

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


OsStatus sipXmediaFactoryImpl::getNumOfCodecs(int& iCodecs) const
{
    iCodecs = TOTAL_AUDIO_CODECS_NUM;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::getCodec(int iCodec, UtlString& codec, int &bandWidth) const
{
    OsStatus rc = OS_SUCCESS;

    switch (iCodec)
    {
    case 0: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMU;
        break;
    case 1: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMA;
        break;
    case 2: codec = (const char*) SdpCodec::SDP_CODEC_TONES;
        break;

#ifdef HAVE_GIPS /* [ */
    case GIPS_CODECS_BEGIN+0: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_IPCMU;
        break;
    case GIPS_CODECS_BEGIN+1: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_IPCMA;
        break;
    case GIPS_CODECS_BEGIN+2: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_IPCMWB;
        break;
#endif /* HAVE_GIPS ] */

#ifdef HAVE_SPEEX /* [ */
    case SPEEX_AUDIO_CODECS_BEGIN+0: codec = (const char*) SdpCodec::SDP_CODEC_SPEEX;
        break;
    case SPEEX_AUDIO_CODECS_BEGIN+1: codec = (const char*) SdpCodec::SDP_CODEC_SPEEX_5;
        break;
    case SPEEX_AUDIO_CODECS_BEGIN+2: codec = (const char*) SdpCodec::SDP_CODEC_SPEEX_15;
        break;
    case SPEEX_AUDIO_CODECS_BEGIN+3: codec = (const char*) SdpCodec::SDP_CODEC_SPEEX_24;
        break;
#endif /* HAVE_SPEEX ] */

#ifdef HAVE_GSM /* [ */
    case GSM_AUDIO_CODECS_BEGIN+0: codec = (const char*) SdpCodec::SDP_CODEC_GSM;
        break;
#endif /* HAVE_GSM ] */

#ifdef HAVE_ILBC /* [ */
    case ILBC_AUDIO_CODECS_BEGIN+0: codec = (const char*) SdpCodec::SDP_CODEC_ILBC;
        break;
#endif /* HAVE_ILBC ] */

    default: rc = OS_FAILED;
    }

    return rc;
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

OsStatus sipXmediaFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes type, UtlString& codecName) const
{
    OsStatus rc = OS_FAILED;

    codecName = "";

    switch (type)
    {
    case SdpCodec::SDP_CODEC_TONES:
        codecName = SIPX_CODEC_ID_TELEPHONE;
        break;
    case SdpCodec::SDP_CODEC_G729A:
        codecName = SIPX_CODEC_ID_G729;
        break;
    case SdpCodec::SDP_CODEC_GIPS_PCMA:
        codecName = SIPX_CODEC_ID_PCMA;
        break;
    case SdpCodec::SDP_CODEC_GIPS_PCMU:
        codecName = SIPX_CODEC_ID_PCMU;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMA:
        codecName = SIPX_CODEC_ID_EG711A;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMU:
        codecName = SIPX_CODEC_ID_EG711U;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
        codecName = SIPX_CODEC_ID_IPCMWB;
        break;
    case SdpCodec::SDP_CODEC_ILBC:
        codecName = SIPX_CODEC_ID_ILBC;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ISAC:
        codecName = SIPX_CODEC_ID_ISAC;
        break;
    case SdpCodec::SDP_CODEC_SPEEX:
        codecName = SIPX_CODEC_ID_SPEEX;
        break;
    case SdpCodec::SDP_CODEC_SPEEX_5:
        codecName = SIPX_CODEC_ID_SPEEX_5;
        break;
    case SdpCodec::SDP_CODEC_SPEEX_15:
        codecName = SIPX_CODEC_ID_SPEEX_15;
        break;
    case SdpCodec::SDP_CODEC_SPEEX_24:
        codecName = SIPX_CODEC_ID_SPEEX_24;
        break;
    case SdpCodec::SDP_CODEC_GSM:
        codecName = SIPX_CODEC_ID_GSM;
        break;
    default:
        OsSysLog::add(FAC_MP, PRI_WARNING,
                      "sipXmediaFactoryImpl::getCodecNameByType unsupported type %d.",
                      type);
    
    }

    if (codecName != "")
    {
        rc = OS_SUCCESS;
    }

    return rc;
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


