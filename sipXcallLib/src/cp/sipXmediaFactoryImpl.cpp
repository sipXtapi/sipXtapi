// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "cp/sipXmediaFactoryImpl.h"
#include "cp/CpPhoneMediaInterface.h"
#include "os/OsConfigDb.h"
#include "mp/MpMediaTask.h"
#include "mp/MpMisc.h"
#include "mp/MpCodec.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/dmaTask.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           10
// STATIC VARIABLE INITIALIZATIONS

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
    mpStartUp(8000, 80, 16*maxFlowGraph, pConfigDb);

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

    mpStartTasks();  

    miGain = 7 ;
    mbAEC = FALSE ;
}


// Destructor
sipXmediaFactoryImpl::~sipXmediaFactoryImpl()
{
    // TODO: Shutdown
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* sipXmediaFactoryImpl::createMediaInterface( int startRtpPort, 
                                                              int lastRtpPort,
                                                              const char* publicAddress,
                                                              const char* localAddress,
                                                              int numCodecs,
                                                              SdpCodec* sdpCodecArray[],
                                                              const char* locale,
                                                              int expeditedIpTos,
                                                              const char* szStunServer,
                                                              int iStunKeepAliveSecs 
                                                            ) 
{
    return new CpPhoneMediaInterface(startRtpPort, lastRtpPort, 
            publicAddress, localAddress, numCodecs, sdpCodecArray, locale,
            expeditedIpTos, szStunServer, iStunKeepAliveSecs) ;
}


OsStatus sipXmediaFactoryImpl::setSpeakerVolume(int iVolume) 
{
    OsStatus rc = OS_SUCCESS ;
#ifdef WIN32
    MpCodec_setVolume(iVolume) ;
#endif

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
#ifdef WIN32
    rc = MpCodec_setGain(miGain) ;
#endif
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
#ifdef WIN32
    if (bMute)
    {
        MpCodec_setGain(0) ;
    }
    else
    {
        MpCodec_setGain(miGain) ;
    }
#endif
    return OS_SUCCESS ;
}


OsStatus sipXmediaFactoryImpl::enableAEC(UtlBoolean bEnable) 
{
    // Mark and set if state is different
    if (mbAEC != bEnable)
    {
        mbAEC = bEnable ;
#ifndef __pingtel_on_posix__
        MpCallFlowGraph::setEnableAEC(mbAEC) ;
#endif
    }

    return OS_SUCCESS ;
}

/* ============================ ACCESSORS ================================= */

OsStatus sipXmediaFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    iVolume = MpCodec_getVolume() ;
#endif
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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


