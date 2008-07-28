// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include "mi/sipXmediaFactoryImpl.h"
#include <sdp/SdpCodec.h>
#include <mp/MpMediaTask.h>
#include <mp/MpMisc.h>
#include <mp/MpCodec.h>
#include <mp/MpCallFlowGraph.h>
#include <mp/dmaTask.h>
#include <mp/MpCodecFactory.h>
#include <sdp/SdpCodecList.h>
#include <os/OsServerSocket.h>
#include <os/OsDatagramSocket.h>
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
int siInstanceCount=0;

#define DEFAULT_NUM_CODEC_PATHS 10;
// STATIC VARIABLE INITIALIZATIONS



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
sipXmediaFactoryImpl::sipXmediaFactoryImpl()
{

}

void sipXmediaFactoryImpl::initialize(OsConfigDb* pConfigDb, 
                                           uint32_t frameSizeMs, 
                                           uint32_t maxSamplesPerSec,
                                           uint32_t defaultSamplesPerSec)
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
   mpMediaTask = MpMediaTask::getMediaTask(maxFlowGraph); 

#ifdef INCLUDE_RTCP /* [ */
   mpiRTCPControl = CRTCManager::getRTCPControl();
#endif /* INCLUDE_RTCP ] */

   if (miInstanceCount == 0)
   {
#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
      mpStartTasks();  
#else
      if (OS_SUCCESS != startNetInTask()) 
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
   // TODO: Shutdown
   --miInstanceCount;
   if (miInstanceCount == 0)
   {
   }
}

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

#ifndef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
# ifdef WIN32
   dmaSignalMicDeviceChange();
# endif
#else
   rc = OS_NOT_YET_IMPLEMENTED;
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
