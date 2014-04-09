//
// Copyright (C) 2005-2013 SIPez LLC.  All rights reserved.
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
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsConfigDb.h>
#include "sipXmediaFactoryImpl.h"
#include "CpPhoneMediaInterface.h"
#include <sdp/SdpCodec.h>
#include <mp/MpMediaTask.h>
#include <mp/MpMisc.h>
#include <mp/MpCodec.h>
#include <mp/MpCallFlowGraph.h>
#include <mp/dmaTask.h>
#include <mp/MpCodecFactory.h>
#include <sdp/SdpCodecList.h>
#include "mi/CpMediaInterfaceFactoryFactory.h"

#ifdef INCLUDE_RTCP /* [ */
#include <rtcp/RTCManager.h>
#endif /* INCLUDE_RTCP ] */

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
#include <mp/NetInTask.h>
#endif


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION

#define MAX_MANAGED_FLOW_GRAPHS 16

// STATIC VARIABLE INITIALIZATIONS
int sipXmediaFactoryImpl::miInstanceCount=0;

CpMediaInterfaceFactory* spFactory = NULL;
int siInstanceCount=0;

#ifndef DISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb,
                                                            uint32_t frameSizeMs, 
                                                            uint32_t maxSamplesPerSec,
                                                            uint32_t defaultSamplesPerSec,
                                                            UtlBoolean enableLocalAudio,
                                                            const UtlString &inputDeviceName,
                                                            const UtlString &outputDeviceName)
{
   // TODO: Add locking

   if (spFactory == NULL)
   {
      spFactory = new CpMediaInterfaceFactory();
      spFactory->setFactoryImplementation(new sipXmediaFactoryImpl(pConfigDb,
                                                                   frameSizeMs, 
                                                                   maxSamplesPerSec,
                                                                   defaultSamplesPerSec,
                                                                   enableLocalAudio,
                                                                   inputDeviceName,
                                                                   outputDeviceName));
   }
   siInstanceCount++;

   // Assert some sane value
   assert(siInstanceCount < 11);
   return spFactory;
}
#endif

extern "C" void sipxDestroyMediaFactoryFactory()
{
   // TODO: Add locking

   if (siInstanceCount > 0)
   {
      siInstanceCount--;
      if (siInstanceCount == 0)
      {
         if (spFactory)
         {
            OsSysLog::add(FAC_MP, PRI_DEBUG, "sipxDestroyMediaFactoryFactory spFactory: %p", spFactory);
            delete spFactory;
            spFactory = NULL;
         }
         else
         {
            OsSysLog::add(FAC_MP, PRI_WARNING, "sipxDestroyMediaFactoryFactory null factory\n");
         }
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG, "sipxDestroyMediaFactoryFactory siInstanceCount: %d\n", siInstanceCount);
      }
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "sipxDestroyMediaFactoryFactory siInstanceCount <= 0 siInstanceCount: %d\n", siInstanceCount);
   }
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
sipXmediaFactoryImpl::sipXmediaFactoryImpl(OsConfigDb* pConfigDb, 
                                           uint32_t frameSizeMs, 
                                           uint32_t maxSamplesPerSec,
                                           uint32_t defaultSamplesPerSec,
                                           UtlBoolean enableLocalAudio,
                                           const UtlString &inputDeviceName,
                                           const UtlString &outputDeviceName)
{
   // See Doxygen comments for this constructor for information on the impact 
   // of the values of maxSamplesPerFrame and maxSamplesPerSec.
   mFrameSizeMs = (frameSizeMs == 0) ? 10 : frameSizeMs;
   mMaxSamplesPerSec = (maxSamplesPerSec == 0) ? 8000 : maxSamplesPerSec;

   // Default the default sample rate to 8kHz, so NB users will be happy.
   mDefaultSamplesPerSec = (defaultSamplesPerSec == 0) ? 8000 : defaultSamplesPerSec;
   assert(mDefaultSamplesPerSec <= mMaxSamplesPerSec);
   if(mDefaultSamplesPerSec > mMaxSamplesPerSec)
   {
      OsSysLog::add(FAC_MP, PRI_CRIT, 
         "sipXmediaFactoryImpl constructor - %d > %d: "
         "default sample rate is higher than max sample rate!", 
         mDefaultSamplesPerSec, mMaxSamplesPerSec);
   }

   int maxFlowGraph = -1; 
   UtlString strInBandDTMF;
   if (pConfigDb)
   {
      pConfigDb->get("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxFlowGraph);
      pConfigDb->get("PHONESET_SEND_INBAND_DTMF", strInBandDTMF);
      strInBandDTMF.toUpper();

      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "sipXmediaFactoryImpl::sipXmediaFactoryImpl"
                    " maxFlowGraph = %d",
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
      mpStartUp(mMaxSamplesPerSec, (mFrameSizeMs*mMaxSamplesPerSec)/1000, 
                16*maxFlowGraph, pConfigDb, mnCodecPaths, mpCodecPaths);
   }

   // Should we send inband DTMF by default?    
   if (strInBandDTMF.compareTo("DISABLE") == 0)
   {
      MpCallFlowGraph::setInbandDTMF(false);
   }
   else
   {
      MpCallFlowGraph::setInbandDTMF(true);
   }

   // init the media processing task
   mpMediaTask = MpMediaTask::createMediaTask(maxFlowGraph, enableLocalAudio); 

#ifdef INCLUDE_RTCP /* [ */
   mpiRTCPControl = CRTCManager::getRTCPControl();
#endif /* INCLUDE_RTCP ] */

   if (miInstanceCount == 0)
   {
#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
      mpStartTasks();  
#else
      NetInTask *pTask = NetInTask::getNetInTask();
      if (NULL == pTask) 
      {
         OsSysLog::add(FAC_MP, PRI_ERR, "Could not start NetInTask!!");
      }
#endif
   }

   miGain = 7;
   ++miInstanceCount;

   // We are missing synchronization -- give the tasks time to start
   OsTask::delay(100);
}


// Destructor
sipXmediaFactoryImpl::~sipXmediaFactoryImpl()
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, "~sipXmediaFactoryImpl miInstanceCount: %d\n", miInstanceCount);
   // TODO: Shutdown
   --miInstanceCount;
   if (miInstanceCount == 0)
   {
#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
      mpStopTasks();
#else
      NetInTask *pTask = NetInTask::getNetInTask();
      pTask->destroy();
#endif
      mpShutdown();
   }
}

/* ============================ MANIPULATORS ============================== */
#ifndef DISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY
CpMediaInterface* sipXmediaFactoryImpl::createMediaInterface(const char* publicAddress,
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
                                                             UtlBoolean bEnableICE,
                                                             uint32_t samplesPerSec,
                                                             OsMsgDispatcher* pDispatcher) 
{
   // if the sample rate passed in is zero, use the default.
   samplesPerSec = (samplesPerSec == 0) ? mDefaultSamplesPerSec : samplesPerSec;
   CpMediaInterface *pIf =
      new CpPhoneMediaInterface(this, (mFrameSizeMs*samplesPerSec)/1000, 
      samplesPerSec, publicAddress, localAddress, numCodecs, sdpCodecArray, 
      locale, expeditedIpTos, szStunServer, iStunPort, iStunKeepAliveSecs, 
      szTurnServer, iTurnPort, szTurnUsername, szTurnPassword, 
      iTurnKeepAlivePeriodSecs, bEnableICE, pDispatcher);

   pIf->setValue(miInterfaceId);
   miInterfaceId++;
   return pIf;
}
#endif

OsStatus sipXmediaFactoryImpl::setSpeakerVolume(int iVolume) 
{
   OsStatus rc = OS_SUCCESS;
   MpCodec_setVolume(iVolume);

   return rc;
}

OsStatus sipXmediaFactoryImpl::setSpeakerDevice(const UtlString& device) 
{
   OsStatus rc = OS_SUCCESS;
   DmaTask::setCallDevice(device.data());
   return rc;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneGain(int iGain) 
{
   OsStatus rc;

   miGain = iGain;
   rc = MpCodec_setGain(miGain);
   return rc;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneDevice(const UtlString& device) 
{
   OsStatus rc = OS_SUCCESS;
   DmaTask::setInputDevice(device.data());
#ifdef WIN32
   dmaSignalMicDeviceChange();
#endif
   return rc;
}

OsStatus sipXmediaFactoryImpl::muteMicrophone(UtlBoolean bMute) 
{
   if (bMute)
   {
      MpCodec_setGain(0);
   }
   else
   {
      MpCodec_setGain(miGain);
   }
   return OS_SUCCESS;
}

OsStatus sipXmediaFactoryImpl::setAudioAECMode(const MEDIA_AEC_MODE mode)
{
   if (MpCallFlowGraph::setAECMode((FLOWGRAPH_AEC_MODE)mode))
   {
      return OS_SUCCESS;
   }
   else
   {
      return OS_NOT_SUPPORTED;
   }
}

OsStatus sipXmediaFactoryImpl::enableAGC(UtlBoolean bEnable) 
{
   if (MpCallFlowGraph::setAGC(bEnable)) 
   {
      return OS_SUCCESS;
   }
   else 
   {
      return OS_NOT_SUPPORTED; 
   }
}

OsStatus sipXmediaFactoryImpl::setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode) 
{
   if (mode == MEDIA_NOISE_REDUCTION_DISABLED)
   {
      if (MpCallFlowGraph::setAudioNoiseReduction(FALSE))
      {
         return OS_SUCCESS;
      }
   }
   else
   {
      if (MpCallFlowGraph::setAudioNoiseReduction(TRUE))
      {
         return OS_SUCCESS;
      }
   }
   return OS_NOT_SUPPORTED;
}

OsStatus sipXmediaFactoryImpl::buildCodecFactory(SdpCodecList* pFactory, 
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
         *iRejected = pFactory->addCodecs(sAudioPreferences);
         OsSysLog::add(FAC_MP, PRI_DEBUG, 
                       "sipXmediaFactoryImpl::buildCodecFactory: "
                       "supported codecs = %s with NumReject %d",
                       sAudioPreferences.data(), *iRejected);
         rc = OS_SUCCESS;
      }
      else
      {
         // Build up the supported codecs
         MpCodecFactory *pCodecFactory = MpCodecFactory::getMpCodecFactory();
         pCodecFactory->addCodecsToList(*pFactory);

         *iRejected = 0;
         rc = OS_SUCCESS;
      }

#ifdef VIDEO // [
      // If preferred codecs supplied - add them, else add all supported
      // codecs.
      if (sVideoPreferences.length() > 0)
      {
         *iRejected = pFactory->addCodecs(sVideoPreferences);
         OsSysLog::add(FAC_MP, PRI_DEBUG, 
                       "sipXmediaFactoryImpl::buildCodecFactory: "
                       "supported codecs = %s with NumReject %d",
                       sVideoPreferences.data(), *iRejected);
         rc = OS_SUCCESS;
      }
      else
      {
         // Build up the supported codecs
         SdpCodec::SdpCodecTypes videoCodecs[] = {};
         const int numVideoCodecs = sizeof(videoCodecs)/sizeof(SdpCodec::SdpCodecTypes);

         *iRejected = pFactory->addCodecs(numVideoCodecs, videoCodecs);
         rc = OS_SUCCESS;
      }
#endif // VIDEO ]
   }
   return rc;
}


OsStatus sipXmediaFactoryImpl::updateVideoPreviewWindow(void* displayContext) 
{
   return OS_NOT_SUPPORTED;
}


/* ============================ ACCESSORS ================================= */

OsStatus sipXmediaFactoryImpl::getSpeakerVolume(int& iVolume) const
{
   OsStatus rc = OS_SUCCESS;

   iVolume = MpCodec_getVolume();
   if (iVolume==-1)
   {
      rc = OS_FAILED;
      iVolume = 0;
   }
   return rc;
}

OsStatus sipXmediaFactoryImpl::getSpeakerDevice(UtlString& device) const
{
   OsStatus rc = OS_SUCCESS;
   device = DmaTask::getCallDevice();
   return rc;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneGain(int& iGain) const
{
   OsStatus rc = OS_SUCCESS;
   iGain = MpCodec_getGain();
   return rc;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
   OsStatus rc = OS_SUCCESS;
   device = DmaTask::getMicDevice();
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

OsStatus sipXmediaFactoryImpl::getLocalAudioConnectionId(int& connectionId) const 
{
   connectionId = -1;
   return OS_NOT_SUPPORTED;

}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
