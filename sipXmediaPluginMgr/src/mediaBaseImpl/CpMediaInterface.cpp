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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
// 
// Copyright (C) 2005-2006 SIPez LLC.
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


// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES
#include <assert.h>
#include "mediaBaseImpl/CpMediaInterface.h"
#include "os/OsPerfLog.h"
#include "os/OsLock.h"
#include "os/OsMediaContact.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"

// APPLICATION INCLUDES

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int CpMediaInterface::sInvalidConnectionId = -1;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


// Default Constructor
CpMediaInterface::CpMediaInterface()
    : mpSocketIdleSink(NULL)
    , mbConferenceEnabled(false)
    , mbIsEncrypted(false)
    , mPrimaryVideoCodec(NULL)
    , mpSecurityAttributes(NULL)
    , mpMediaPacketCallback(NULL)
    , mpFactoryImpl(NULL)
    , mAudioMediaConnectivityInfo(MediaConnectivityInfo::MCIT_AUDIO_RTP)
    , mVideoMediaConnectivityInfo(MediaConnectivityInfo::MCIT_VIDEO_RTP)
{

}

// Constructor
CpMediaInterface::CpMediaInterface(IMediaDeviceMgr* pFactoryImpl,
                                   const char* publicAddress,
                                   const char* localAddress,
                                   int numCodecs,
                                   SdpCodec* sdpCodecArray[],
                                   const char* locale,
                                   int expeditedIpTos,
                                   const ProxyDescriptor& stunServer,
                                   const ProxyDescriptor& turnProxy,
                                   const ProxyDescriptor& arsProxy,
                                   const ProxyDescriptor& arsHttpProxy,
                                   UtlBoolean bDTMFOutOfBand,
  	                               UtlBoolean bDTMFInBand,
  	                               UtlBoolean bEnableRTCP,
                                   const char* szRtcpName,
                                   SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback)
    : mpSocketIdleSink(NULL)
    , mbConferenceEnabled(false)
    , mbIsEncrypted(false)
    , mInitialCodecs(numCodecs, sdpCodecArray)
    , mSupportedCodecs(numCodecs, sdpCodecArray)
    , mPrimaryVideoCodec(NULL)
    , mpSecurityAttributes(NULL)
    , mpMediaPacketCallback(pMediaPacketCallback)
    , mpFactoryImpl(pFactoryImpl)
    , mAudioMediaConnectivityInfo(MediaConnectivityInfo::MCIT_AUDIO_RTP)
    , mVideoMediaConnectivityInfo(MediaConnectivityInfo::MCIT_VIDEO_RTP)
   {
   }


// Destructor
CpMediaInterface::~CpMediaInterface()
{
}

OsStatus CpMediaInterface::setUserAgent(int connectionId,
                                        const char* szUserAgent)
{
    OsStatus rc = OS_FAILED ;
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn && szUserAgent && strlen(szUserAgent))
    {        
        if (pMediaConn->mUserAgent.compareTo(szUserAgent, UtlString::ignoreCase) != 0)
        {
            pMediaConn->mUserAgent = szUserAgent ;
            rc = OS_SUCCESS ;
        }
    }
    else if (pMediaConn == NULL)
    {
        assert(FALSE) ;
    }

    return rc ;

}


const char* CpMediaInterface::getUserAgent(int connectionId) 
{
    const char* szRC = NULL ;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn)
    {        
        szRC = pMediaConn->mUserAgent.data() ;
    }

    return szRC ;
}



OsStatus CpMediaInterface::setMediaData(const int data)
{
    mMediaData = data;
    return OS_SUCCESS;
}

OsStatus CpMediaInterface::setSrtpParams(SdpSrtpParameters& srtpParameters)
{
    if (srtpParameters.masterKey[0] != '\0') // only set the key if it comes from the caller
    {
        memcpy((void*)&mSrtpParams, (void*)&srtpParameters, sizeof(SdpSrtpParameters));
    }
    return OS_SUCCESS;
    
}

OsStatus CpMediaInterface::enableRtpReadNotification(int connectionId,
                                                     UtlBoolean bEnable) 
{
   return OS_NOT_SUPPORTED;
};

OsStatus CpMediaInterface::recordMic(int ms, UtlString* pAudioBuf) 
{ 
   return OS_NOT_SUPPORTED; 
};

/* ============================ ACCESSORS ================================= */

int CpMediaInterface::getInvalidConnectionId()
{
    return(sInvalidConnectionId);
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpMediaInterface::isConnectionIdValid(int connectionId)
{
    return(connectionId > sInvalidConnectionId);
}

int CpMediaInterface::getNumCodecs(int connectionId)
{
    OS_PERF_FUNC("CpMediaInterface::getNumCodecs") ;
    OsLock lock(*mpMediaGuard) ;
    int iCodecs = 0;
    CpMediaConnection* 
        pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn && pMediaConn->mpCodecFactory)
    {
        iCodecs = pMediaConn->mpCodecFactory->getCodecCount();
    }

    return iCodecs;
}

//////////////////////////////////////////////////////////////////////////////


CpMediaConnection* CpMediaInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId) ;
   return ((CpMediaConnection*) mMediaConnections.find(&matchConnectionId)) ;
}

bool CpMediaInterface::isIceEnabled(int connectionId) 
{
    bool bEnabled = false ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bEnabled = pMediaConnection->mbEnableICE ;
    }

    return bEnabled ;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addAudioRtpConnectionDestination(int         connectionId,
                                                            int         iPriority,
                                                            const char* candidateIp, 
                                                            int         candidatePort) 
{
    OS_PERF_FUNC("CpMediaInterface::addAudioRtpConnectionDestination") ;
    OsLock lock(*mpMediaGuard) ;

    traceAPI("CpMediaInterface::addAudioRtpConnectionDestination") ;

    trace(PRI_INFO, "adding audio rtp destination: id=%d priority=%d dest=%s:%d",
            connectionId,
            iPriority,
            candidateIp,
            candidatePort) ;

    OsStatus returnCode = OS_NOT_FOUND;

    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            if (    (candidateIp != NULL) && 
                    (strlen(candidateIp) > 0) && 
                    (strcmp(candidateIp, "0.0.0.0") != 0) &&
                    portIsValid(candidatePort) && 
                    (mediaConnection->mpRtpAudioSocketArray[i] != NULL))
            {
        
                mediaConnection->mbAlternateDestinations = true ;
                mediaConnection->mpRtpAudioSocketArray[i]->primeTurnDestination(candidateIp, 
                        candidatePort) ;
                mediaConnection->mpRtpAudioSocketArray[i]->addAlternateDestination(
                        candidateIp, candidatePort, iPriority) ;
                returnCode = OS_SUCCESS;
            }
        }
    }

    return returnCode ;
}

void CpMediaInterface::doEnableTurn(CpMediaConnection* pMediaConnection, 
                                             bool bEnable) 
{
    if (pMediaConnection)
    {
        for (int i = 0; i < 2; i++)
        {
            if (pMediaConnection->mpRtpAudioSocketArray[i])
            {
                if (bEnable)
                {
                    pMediaConnection->mpRtpAudioSocketArray[i]->enableTurn(
                            mTurnProxy.getAddress(), mTurnProxy.getPort(), 
                            mTurnProxy.getKeepalive(), 
                            mTurnProxy.getUsername(), mTurnProxy.getPassword(), 
                            false) ;
                }
                else
                    pMediaConnection->mpRtpAudioSocketArray[i]->disableTurn() ;
            }

            if (pMediaConnection->mpRtcpAudioSocketArray[i])
            {
                if (bEnable)
                    pMediaConnection->mpRtcpAudioSocketArray[i]->enableTurn(
                            mTurnProxy.getAddress(), mTurnProxy.getPort(),
                            mTurnProxy.getKeepalive(), 
                            mTurnProxy.getUsername(), mTurnProxy.getPassword(), 
                            false) ;
                else
                    pMediaConnection->mpRtcpAudioSocketArray[i]->disableTurn() ;
            }
            
            if (pMediaConnection->mpRtpVideoSocketArray[i])
            {
                if (bEnable)
                {
                    pMediaConnection->mpRtpVideoSocketArray[i]->enableTurn(
                            mTurnProxy.getAddress(), mTurnProxy.getPort(), 
                            mTurnProxy.getKeepalive(),
                            mTurnProxy.getUsername(), mTurnProxy.getPassword(), 
                            false) ;
                }
                else
                    pMediaConnection->mpRtpVideoSocketArray[i]->disableTurn() ;
            }

            if (pMediaConnection->mpRtcpVideoSocketArray[i])
            {
                if (bEnable)
                    pMediaConnection->mpRtcpVideoSocketArray[i]->enableTurn(
                            mTurnProxy.getAddress(), mTurnProxy.getPort(),
                            mTurnProxy.getKeepalive(),
                            mTurnProxy.getUsername(), mTurnProxy.getPassword(), 
                            false) ;
                else
                    pMediaConnection->mpRtcpVideoSocketArray[i]->disableTurn() ;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addAudioRtcpConnectionDestination(int         connectionId,
                                                                      int         iPriority,
                                                                      const char* candidateIp, 
                                                                      int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    if (mbEnableRTCP)
    {
        OS_PERF_FUNC("CpMediaInterface::addAudioRtcpConnectionDestination") ;
        OsLock lock(*mpMediaGuard) ;

        traceAPI("CpMediaInterface::addAudioRtcpConnectionDestination") ;

        trace(PRI_INFO, "adding audio rtcp destination: id=%d priority=%d dest=%s:%d",
                connectionId,
                iPriority,
                candidateIp,
                candidatePort) ;        

        CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
        if (mediaConnection) 
        {        
            // This is not applicable to multicast sockets
            if (mediaConnection->mIsMulticast)
            {
                return OS_FAILED;
            }
            for (int i = 0; i < 2; i++) // UDP and TCP
            {
                if (    (candidateIp != NULL) && 
                        (strlen(candidateIp) > 0) && 
                        (strcmp(candidateIp, "0.0.0.0") != 0) &&
                        portIsValid(candidatePort) && 
                        (mediaConnection->mpRtcpAudioSocketArray[i] != NULL))
                {
                    mediaConnection->mbAlternateDestinations = true ;
                    mediaConnection->mpRtcpAudioSocketArray[i]->primeTurnDestination(candidateIp, 
                            candidatePort) ;

                    mediaConnection->mpRtcpAudioSocketArray[i]->addAlternateDestination(
                            candidateIp, candidatePort, iPriority) ;
                    returnCode = OS_SUCCESS;
                }
            }
        }
    }

    return returnCode ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addVideoRtpConnectionDestination(int         connectionId,
                                                                     int         iPriority,
                                                                     const char* candidateIp, 
                                                                     int         candidatePort) 
{
    OS_PERF_FUNC("CpMediaInterface::addVideoRtpConnectionDestination") ;
    OsLock lock(*mpMediaGuard) ;

    traceAPI("CpMediaInterface::addVideoRtpConnectionDestination") ;

    trace(PRI_INFO, "adding video rtp destination: id=%d priority=%d dest=%s:%d",
            connectionId,
            iPriority,
            candidateIp,
            candidatePort) ;


    OsStatus returnCode = OS_NOT_FOUND;

    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {
        // This is not applicable to multicast sockets
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }
        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            if (    (candidateIp != NULL) && 
                    (strlen(candidateIp) > 0) && 
                    (strcmp(candidateIp, "0.0.0.0") != 0) &&
                    portIsValid(candidatePort) && 
                    (mediaConnection->mpRtpVideoSocketArray[i] != NULL))
            {
                mediaConnection->mbAlternateDestinations = true ;
                mediaConnection->mpRtpVideoSocketArray[i]->primeTurnDestination(candidateIp, 
                        candidatePort) ;
                mediaConnection->mpRtpVideoSocketArray[i]->addAlternateDestination(
                        candidateIp, candidatePort, iPriority) ;

                returnCode = OS_SUCCESS;
            }
        }
    }


    return returnCode ;
}

//////////////////////////////////////////////////////////////////////////////


OsStatus CpMediaInterface::addVideoRtcpConnectionDestination(int         connectionId,
                                                                      int         iPriority,
                                                                      const char* candidateIp, 
                                                                      int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    if (mbEnableRTCP)
    {
        OS_PERF_FUNC("CpMediaInterface::addVideoRtcpConnectionDestination") ;
        OsLock lock(*mpMediaGuard) ;
        

        traceAPI("CpMediaInterface::addVideoRtcpConnectionDestination") ;

        trace(PRI_INFO, "adding video rtcp destination: id=%d priority=%d dest=%s:%d",
                connectionId,
                iPriority,
                candidateIp,
                candidatePort) ;

        CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
        if (mediaConnection) 
        {        
            for (int i = 0; i < 2; i++) // UDP and TCP
            {
                if (    (candidateIp != NULL) && 
                        (strlen(candidateIp) > 0) && 
                        (strcmp(candidateIp, "0.0.0.0") != 0) &&
                        portIsValid(candidatePort) && 
                        (mediaConnection->mpRtcpVideoSocketArray[i] != NULL))
                {                
                    mediaConnection->mbAlternateDestinations = true ;
                    mediaConnection->mpRtcpVideoSocketArray[i]->primeTurnDestination(candidateIp, 
                            candidatePort) ;
                    mediaConnection->mpRtcpVideoSocketArray[i]->addAlternateDestination(
                            candidateIp, candidatePort, iPriority) ;
                    returnCode = OS_SUCCESS;
                }            
            }
        }
    }

    return returnCode ;
}


OsStatus CpMediaInterface::addAudioArsConnectionDestination(int         connectionId,
                                                                     int         iPriority,
                                                                     const char* candidateIp, 
                                                                     int         candidatePort) 
{
    CpMediaNetTask* pNetTask = CpMediaNetTask::getCpMediaNetTask();
    OsStatus rc = OS_FAILED ;

    trace(PRI_INFO, "adding audio ars destination: id=%d priority=%d dest=%s:%d",
            connectionId,
            iPriority,
            candidateIp,
            candidatePort) ;
    OsLock lock(*mpMediaGuard) ;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn && pMediaConn->mpArsAudioSocket) 
    {
        if (pMediaConn->mbInitiating)
        {
            // ignore ice candidate -- we are only interested in the priority
            rc = OS_SUCCESS ;
        }
        else
        {
            ARS_COOKIE cookie ;
            memset(cookie.data, 0xBA, sizeof(cookie.data)) ;
            if (pMediaConn->mpArsAudioSocket->connectArs(candidateIp, candidatePort, cookie))
            {
                pNetTask->addInputSource(pMediaConn->mpArsAudioSocket);
                rc = OS_SUCCESS ;
            }
        }
        pMediaConn->miArsAudioPriority = iPriority ;
    }

    return rc ;
}


OsStatus CpMediaInterface::addVideoArsConnectionDestination(int         connectionId,
                                                                     int         iPriority,
                                                                     const char* candidateIp, 
                                                                     int         candidatePort) 
{
    CpMediaNetTask* pNetTask = CpMediaNetTask::getCpMediaNetTask();
    OsStatus rc = OS_FAILED ;

    trace(PRI_INFO, "adding video ars destination: id=%d priority=%d dest=%s:%d",
            connectionId,
            iPriority,
            candidateIp,
            candidatePort) ;

    OsLock lock(*mpMediaGuard) ;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn && pMediaConn->mpArsVideoSocket) 
    {
        if (pMediaConn->mbInitiating)
        {
            // ignore ice candidate -- we are only interested in the priority
            rc = OS_SUCCESS ;
        }
        else
        {
            ARS_COOKIE cookie ;
            memset(cookie.data, 0xBA, sizeof(cookie.data)) ;
            if (pMediaConn->mpArsVideoSocket->connectArs(candidateIp, candidatePort, cookie))
            {
                pNetTask->addInputSource(pMediaConn->mpArsVideoSocket);
                rc = OS_SUCCESS ;
            }
        }
        pMediaConn->miArsVideoPriority = iPriority ;
    }

    return rc ;

}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::enableRtpReadNotification(int  connectionId,
                                                              bool bEnable)
{
    OS_PERF_FUNC("CpMediaInterface::enableRtpReadNotification") ;
    OsLock lock(*mpMediaGuard) ;

    traceAPI("CpMediaInterface::enableRtpReadNotification") ;
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;  
    if (pMediaConnection)
    {
        if (bEnable)
            pMediaConnection->startReadNotifyAdapters(mpSocketIdleSink) ;
        else
            pMediaConnection->stopReadNotifyAdapters() ;
    }

    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

void  CpMediaInterface::setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) 
{
    OsLock lock(*mpMediaGuard) ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        if (pMediaConnection->mIsMulticast && eType == CONTACT_AUTO)
        {
            pMediaConnection->mContactType = CONTACT_LOCAL;
            pMediaConnection->mContactId = contactId ;
        }
        else
        {
            // Only CONTACT_LOCAL is allowed for multicast addresses.
            assert(!pMediaConnection->mIsMulticast || eType == CONTACT_LOCAL);

            pMediaConnection->mContactType = eType;
            pMediaConnection->mContactId = contactId ;

            mpFactoryImpl->applyMediaContactTypeOverride(pMediaConnection->mContactType) ;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

// Calculate the current cost for our sending/receiving codecs
int CpMediaInterface::getCodecCPUCost()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   
   
   return iCost ;
}

//////////////////////////////////////////////////////////////////////////////

// Calculate the worst case cost for our sending/receiving codecs
int CpMediaInterface::getCodecCPULimit()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   

   return iCost ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::getPrimaryCodec(int connectionId, 
                                                    UtlString& audioCodec,
                                                    UtlString& videoCodec,
                                                    int* audioPayloadType,
                                                    int* videoPayloadType,
                                                    bool& isEncrypted)
{
    OS_PERF_FUNC("CpMediaInterface::getPrimaryCodec") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && pMediaConnection->mPrimaryCodecType != 
            SdpCodec::SDP_CODEC_UNKNOWN)
    {
        if (mpFactoryImpl->getCodecNameByType(pMediaConnection->mPrimaryCodecType, audioCodec))
        {
            if (NULL != audioPayloadType)
            {
                *audioPayloadType = pMediaConnection->mRtpPayloadType;
            }
            isEncrypted = mbIsEncrypted;
            rc = OS_SUCCESS;
        }
        if (videoPayloadType)
        {
            *videoPayloadType = -1;
        }
        videoCodec = "";
        if (pMediaConnection->mbIsPrimaryVideoCodecSet)
        {
            if (mpFactoryImpl->getCodecNameByType(pMediaConnection->mPrimaryVideoCodec.getCodecType(), videoCodec))
            {
                if (NULL != videoPayloadType)
                {
                    *videoPayloadType = pMediaConnection->mRtpVideoPayloadType;
                }

                rc = OS_SUCCESS;
            }
        }
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::setAudioCodecBandwidth(int connectionId, int bandWidth)
{
    OS_PERF_FUNC("CpMediaInterface::setAudioCodecBandwidth") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    int numCodecs;
    int codecIndex;
    SdpCodec** codecsArray = NULL;
    UtlString codecName;
    UtlString videoCodecs("");
    UtlString audioCodecs("");
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    // Additionally check if bandwidth is even legal
    if (pMediaConn && (bandWidth >= AUDIO_MICODEC_BW_LOW && bandWidth <= AUDIO_MICODEC_BW_HIGH))
    {
        // Remember supported video codecs
        pMediaConn->mpCodecFactory->getCodecs(numCodecs, codecsArray, "video");
        for (codecIndex = 0; codecIndex<numCodecs; codecIndex++)
        {
            if (mpFactoryImpl->getCodecNameByType(codecsArray[codecIndex]->getCodecType(), codecName))
            {
                videoCodecs = videoCodecs + " " + codecName;
            }
        }
        // Free up the codecs and the array
        for (codecIndex = 0; codecIndex < numCodecs; codecIndex++)
        {
            delete codecsArray[codecIndex];
            codecsArray[codecIndex] = NULL;
        }
        delete[] codecsArray;
        codecsArray = NULL;

        // Rebuild factory with all audio codecs so we can go in afterwards
        // and pick them by bandwidth
        pMediaConn->mpCodecFactory->clearCodecs() ;
        UtlString emptyCodecPref ;
        int iRejected ;
        getFactoryImpl()->buildCodecFactory(pMediaConn->mpCodecFactory, emptyCodecPref, videoCodecs, -1, &iRejected) ;

        // Filter out non-match bandwidth
        pMediaConn->mpCodecFactory->getCodecs(numCodecs, codecsArray);
        for (codecIndex = 0; codecIndex<numCodecs; codecIndex++)
        {
            if (codecsArray[codecIndex]->getBWCost() > bandWidth)
            {
                UtlString mimeType ;
                codecsArray[codecIndex]->getMediaType(mimeType) ;
                if (mimeType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase))
                {
                    pMediaConn->mpCodecFactory->removeCodecByType(codecsArray[codecIndex]->getCodecType()) ;
                }
            }
        }

        // Free up the codecs and the array
        for (codecIndex = 0; codecIndex < numCodecs; codecIndex++)
        {
            delete codecsArray[codecIndex];
            codecsArray[codecIndex] = NULL;
        }
        delete[] codecsArray;
        codecsArray = NULL;

        pMediaConn->mRebuiltAudioBandwidth = bandWidth;

        rc = OS_SUCCESS;
    }
    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::rebuildCodecFactory(int connectionId, 
                                                        int audioBandwidth, 
                                                        int videoBandwidth, 
                                                        UtlString& videoCodec)
{
    OS_PERF_FUNC("CpMediaInterface::rebuildCodecFactory") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    SdpCodecList tempFac;
    int codecIndex=0;
    int numAudioCodecs;
    int numVideoCodecs;
    int videoSize=0;
    UtlString subMimeType;
    UtlString sizeString;
    UtlString encodingName;
    SdpCodec** audioCodecsArray = NULL;
    SdpCodec** videoCodecsArray = NULL;
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    // Find out video size from videoCodec name, split codec name into submimetype
    // and size
    int sep = videoCodec.index('-');
    if (sep != UTL_NOT_FOUND)
    {
        subMimeType = videoCodec(0, sep);
        sizeString = videoCodec(sep+1, videoCodec.length() - (sep+1));
        sizeString.toLower();

        if (sizeString.compareTo("cif") == 0)
        {
#ifdef INCLUDE_CIF_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_CIF;
#endif
        }
        else if (sizeString.compareTo("qcif") == 0)
        {
#ifdef INCLUDE_QCIF_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_QCIF;
#endif
        }
        else if (sizeString.compareTo("sqcif") == 0)
        {
#ifdef INCLUDE_SQCIF_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_SQCIF;
#endif
        }
        else if (sizeString.compareTo("qvga") == 0)
        {
#ifdef INCLUDE_QVGA_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_QVGA;
#endif
        }
        else if (sizeString.compareTo("vga") == 0)
        {
#ifdef INCLUDE_VGA_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_VGA;
#endif
        }
        else if (sizeString.compareTo("4cif") == 0)
        {
#ifdef INCLUDE_4CIF_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_4CIF;
#endif
        }
        else if (sizeString.compareTo("16cif") == 0)
        {
#ifdef INCLUDE_16CIF_RESOLUTION
            videoSize = SDP_VIDEO_FORMAT_16CIF;
#endif
        }
    }
    // Translate video bandwidth into bitrate and framerate, ignore VIDEO_CODEC_BW_DEFAULT
    if (pMediaConn && (videoBandwidth >= VIDEO_CODEC_BW_LOW && videoBandwidth <= VIDEO_CODEC_BW_HIGH))
    {
        switch (videoBandwidth)
        {
        case VIDEO_CODEC_BW_LOW:
            pMediaConn->mConnectionBitrate = 5;
            pMediaConn->mConnectionFramerate = 10;
            break;
        case VIDEO_CODEC_BW_NORMAL:
            // Leave framerate alone for higher bitrates
            pMediaConn->mConnectionBitrate = 70;
            break;
        case VIDEO_CODEC_BW_HIGH:
            pMediaConn->mConnectionBitrate = 400;
            break;
        }
    }

    // Reset supported codecs in case they have been limited before
    mSupportedCodecs.clearCodecs();
    mSupportedCodecs = mInitialCodecs;

    // Did we have a legal video codec name and size?
    if (pMediaConn && videoSize != 0)
    {
        // Get all initial audio codecs
        mInitialCodecs.getCodecs(numAudioCodecs, audioCodecsArray, "audio");
        // Build temporary factory with new bandwidth requirement for audio codecs
        for (codecIndex = 0; codecIndex<numAudioCodecs; codecIndex++)
        {
            if (audioCodecsArray[codecIndex]->getBWCost() <= audioBandwidth)
            {
                tempFac.addCodec(*audioCodecsArray[codecIndex]);
            }
        }
        // Now get all initial video codecs
        mInitialCodecs.getCodecs(numVideoCodecs, videoCodecsArray, "video");
        // Find matching video codec name and size and add only one to codec factory
        for (codecIndex = 0; codecIndex<numVideoCodecs; codecIndex++)
        {
            videoCodecsArray[codecIndex]->getEncodingName(encodingName);
            // Add codec only if submimetype and video format match
            if (subMimeType.compareTo(encodingName, UtlString::ignoreCase) == 0 &&
                videoCodecsArray[codecIndex]->getVideoFormat() == videoSize)
            {
                tempFac.addCodec(*videoCodecsArray[codecIndex]);
            }
        }
        // Clear our old codec factory and copy over the just rebuilt one
        pMediaConn->mpCodecFactory->clearCodecs();
        delete pMediaConn->mpCodecFactory;
        pMediaConn->mpCodecFactory = new SdpCodecList(tempFac);

        // Free up the codecs and the array
        for (codecIndex = 0; codecIndex < numAudioCodecs; codecIndex++)
        {
            delete audioCodecsArray[codecIndex];
            audioCodecsArray[codecIndex] = NULL;
        }
        delete[] audioCodecsArray;
        for (codecIndex = 0; codecIndex < numVideoCodecs; codecIndex++)
        {
            delete videoCodecsArray[codecIndex];
            videoCodecsArray[codecIndex] = NULL;
        }
        delete[] videoCodecsArray;
        videoCodecsArray = NULL;        
        audioCodecsArray = NULL;

    }
    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::setConnectionBitrate(int connectionId, int bitrate)
{
    OS_PERF_FUNC("CpMediaInterface::setConnectionBitrate") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        // If the b field in the SDP indicates a low bitrate client, set the
        // connection bitrate to 5
        if (bitrate < 64 && bitrate != 0)
        {
            pMediaConn->mConnectionBitrate = 5;
        }
    }
    return OS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::setConnectionFramerate(int connectionId, int framerate)
{
    OS_PERF_FUNC("CpMediaInterface::setConnectionFramerate") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        if (framerate != 0)
        {
            //Not quite sure if we should set this globally or just for H263 
            // pMediaConn->mConnectionFramerate = framerate;
        }
    }
    return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpMediaInterface::isSendingRtpAudio(int connectionId)
{
    OsLock lock(*mpMediaGuard) ;
    UtlBoolean bSending = FALSE ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bSending = pMediaConnection->mRtpSendingAudio ;
    }
    else
    {
        osPrintf("CpMediaInterface::isSendingRtpAudio invalid connectionId: %d\n",
           connectionId);
    }
    return bSending ;
}

//////////////////////////////////////////////////////////////////////////////


UtlBoolean CpMediaInterface::isSendingRtpVideo(int connectionId)
{
    OsLock lock(*mpMediaGuard) ;
    UtlBoolean bSending = FALSE ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bSending = pMediaConnection->mRtpSendingVideo ;
    }
    return bSending ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean CpMediaInterface::isReceivingRtpAudio(int connectionId)
{
    OsLock lock(*mpMediaGuard) ;
    UtlBoolean bReceiving = FALSE ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bReceiving = pMediaConnection->mRtpReceivingAudio ;
    }
    else
    {
        osPrintf("CpMediaInterface::isReceivingRtpAudio invalid connectionId: %d\n",
           connectionId);
    }

    return bReceiving ;
}


//////////////////////////////////////////////////////////////////////////////

UtlBoolean CpMediaInterface::isReceivingRtpVideo(int connectionId)
{
    OsLock lock(*mpMediaGuard) ;
    UtlBoolean bReceiving = FALSE ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bReceiving = pMediaConnection->mRtpReceivingVideo ;
    }


    return bReceiving ;
}


//////////////////////////////////////////////////////////////////////////////

UtlBoolean CpMediaInterface::isAudioInitialized(int connectionId)
{
	bool bInitialized = false;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bInitialized = pMediaConnection->mbAudioInitialized ;
    }
	return bInitialized;
}

//////////////////////////////////////////////////////////////////////////////


UtlBoolean CpMediaInterface::isVideoInitialized(int connectionId)
{
	bool bInitialized = false;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bInitialized = pMediaConnection->mbVideoInitialized ;
    }
	return bInitialized;
}


//////////////////////////////////////////////////////////////////////////////

UtlBoolean CpMediaInterface::isVideoConferencing()
{
    return mbConferenceEnabled;
}

//////////////////////////////////////////////////////////////////////////////

IMediaEventListener* CpMediaInterface::getMediaListener(int connectionId)
{
    IMediaEventListener* pRC = NULL ;

    OsLock lock(*mpMediaGuard) ;
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection != NULL)
    {
        pRC = pMediaConnection->mpMediaEventListener ;
    }

    return pRC ;  
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaInterface::lookupResolution(int sdpVideoFormatId, unsigned short& width, unsigned short& height) const 
{
    switch (sdpVideoFormatId)
    {
        case SDP_VIDEO_FORMAT_CIF:
            width = 352;
            height = 288;
            break;
        case SDP_VIDEO_FORMAT_QVGA:
            width = 320;
            height = 240;
            break;
        case SDP_VIDEO_FORMAT_QCIF:
            width = 176;
            height = 144;
            break;
        case SDP_VIDEO_FORMAT_SQCIF:
            width = 128;
            height = 96;
            break;
        case SDP_VIDEO_FORMAT_VGA:
            width = 640;
            height = 480;
            break;
        case SDP_VIDEO_FORMAT_4CIF:
            width = 704;
            height = 576;
            break;
        case SDP_VIDEO_FORMAT_16CIF:
            width = 1408;
            height = 1152;
            break;
        default:
            assert(false);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::applyAlternateDestinations(int connectionId, bool& bAudioFailure, bool &bVideoFailure) 
{
    OS_PERF_FUNC("CpMediaInterface::applyAlternateDestinations") ;

    UtlString destAddress ;
    int       destPort ;
    bool      bDone = false ;
    bool      bError = false ;
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    bAudioFailure = false ;
    bVideoFailure = false ;

    if (pMediaConnection)
    {
        if (pMediaConnection->mIsMulticast)
        {
            return false;
        }

        assert(!pMediaConnection->mDestinationSet) ;
        pMediaConnection->mDestinationSet = true ;

        UtlString rtpAudioSendHostAddress ;
        UtlString rtpVideoSendHostAddress ;
        int rtpAudioSendHostPort = 0 ;
        int rtcpAudioSendHostPort = 0 ;
        int rtpVideoSendHostPort = 0 ;
        int rtcpVideoSendHostPort = 0 ;
        bool bViaOurAudioRelay = false ;
        bool bViaOurVideoRelay = false ;
        bool bIgnore = false ;
        bool bWaitLonger = false ;
        bool bAudioSelectedArs = false ;
        bool bVideoSelectedArs = false ;
        int  audioPriority = 0 ;
        int  videoPriority = 0 ;
        int  iIgnore = 0 ;

        // First get all the data and verify that everything is Copasetic
        for (int i = 0; (i < 2); i++)
        {
            if (pMediaConnection->mpRtpAudioSocketArray[i])
            {
                if (pMediaConnection->mpRtpAudioSocketArray[i]->waitForBestDestination(false, destAddress, destPort, bViaOurAudioRelay, audioPriority))
                    rtpAudioSendHostAddress = destAddress;                    
                else 
                    bWaitLonger = true ;                    
            }

            if (pMediaConnection->mpRtcpAudioSocketArray[i] && mbEnableRTCP)
            {
                if (pMediaConnection->mpRtcpAudioSocketArray[i]->waitForBestDestination(false, destAddress, destPort, bIgnore, iIgnore))
                {
                    if ((rtpAudioSendHostAddress.compareTo(destAddress) != 0) || (bViaOurAudioRelay != bIgnore))
                        bWaitLonger = true ;
                }
                else 
                    bWaitLonger = true ;                    
            }

            
            if (pMediaConnection->mpRtpVideoSocketArray[i])
            {
                if (pMediaConnection->mpRtpVideoSocketArray[i]->waitForBestDestination(false, destAddress, destPort, bViaOurVideoRelay, videoPriority))
                    rtpVideoSendHostAddress = destAddress;
                else
                    bWaitLonger = true ;
            }

            if (pMediaConnection->mpRtcpVideoSocketArray[i] && mbEnableRTCP)
            {
                if (pMediaConnection->mpRtcpVideoSocketArray[i]->waitForBestDestination(false, destAddress, destPort, bIgnore, iIgnore))
                {
                    if ((rtpVideoSendHostAddress.compareTo(destAddress) != 0) || (bViaOurVideoRelay != bIgnore))
                        bWaitLonger = true ;
                }
                else
                    bWaitLonger = true ;
            }
        }

        // Adjust wait-for based on ARS audio connection
        if (pMediaConnection->mpArsAudioSocket)
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG, "ICE audio, best priority=%d, ars priority=%d, connected=%d",
                   audioPriority, pMediaConnection->miArsAudioPriority,
                   pMediaConnection->mpArsAudioSocket->isArsConnected()) ;
            if (pMediaConnection->mpArsAudioSocket->isArsConnected())
            {
                bWaitLonger = false;
            }
            else if (pMediaConnection->miArsAudioPriority > audioPriority)
            {
                // ARS is higher priority -- wait for it or short circuit the wait
                if (!pMediaConnection->mpArsAudioSocket->isArsConnected())
                    bWaitLonger = true ;
                else
                    bWaitLonger = false ;
            }
        }

        // Adjust wait-for based on ARS video connection
        if (pMediaConnection->mpArsVideoSocket)
        {
            if (pMediaConnection->miArsVideoPriority > videoPriority)            
            {
                // ARS is higher priority -- wait for it or short circuit the wait
                if (!pMediaConnection->mpArsVideoSocket->isArsConnected())
                    bWaitLonger = true ;
                else
                    bWaitLonger = false ;
            }
            else if (pMediaConnection->mpArsVideoSocket->isArsConnected())
                bWaitLonger = false ;
        }

        // Wait longer if needed
        if (bWaitLonger)
        {
            for (int i = 0; (i < 2); i++)
            {
                if (pMediaConnection->mpRtpAudioSocketArray[i])
                    pMediaConnection->mpRtpAudioSocketArray[i]->waitForBestDestination(true, destAddress, destPort, bViaOurAudioRelay, iIgnore)  ;

                if (pMediaConnection->mpRtcpAudioSocketArray[i] && mbEnableRTCP)
                    pMediaConnection->mpRtcpAudioSocketArray[i]->waitForBestDestination(true, destAddress, destPort, bIgnore, iIgnore) ;
                if (pMediaConnection->mpRtpVideoSocketArray[i])
                    pMediaConnection->mpRtpVideoSocketArray[i]->waitForBestDestination(true, destAddress, destPort, bViaOurVideoRelay, iIgnore) ;

                if (pMediaConnection->mpRtcpVideoSocketArray[i] && mbEnableRTCP)
                    pMediaConnection->mpRtcpVideoSocketArray[i]->waitForBestDestination(true, destAddress, destPort, bIgnore, iIgnore) ;
            }
        }

        // Get the real data
        for (int i = 0; (i < 2) && !bDone; i++)
        {
            if (pMediaConnection->mpRtpAudioSocketArray[i])
            {
                int timeMS = 0 ;
                mAudioMediaConnectivityInfo.setIce(true) ;
                if (pMediaConnection->mpRtpAudioSocketArray[i]->getBestDestinationAddress(destAddress, destPort, bViaOurAudioRelay, audioPriority, timeMS))
                {                    
                    rtpAudioSendHostAddress = destAddress;
                    rtpAudioSendHostPort = destPort;
                    bDone = true ;
                }
                else
                {
                    audioPriority = 0 ;
                    bAudioFailure = true ;
                    bError = true ;
                    OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Unable to get audio rtp address") ;
                }
                mAudioMediaConnectivityInfo.setIceTime(timeMS) ;
            }

            if (pMediaConnection->mpRtcpAudioSocketArray[i] && mbEnableRTCP)
            {
                int timeMS = 0 ;
                if (pMediaConnection->mpRtcpAudioSocketArray[i]->getBestDestinationAddress(destAddress, destPort, bIgnore, iIgnore, timeMS))
                {
                    rtcpAudioSendHostPort = destPort;
                    if ((rtpAudioSendHostAddress.compareTo(destAddress) != 0) || (bViaOurAudioRelay != bIgnore))
                    {
                        bError = true ;
                        OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Audio Rtp/Rtcp host mismatch: %s != %s or relay mismatch %d != %d",
                                rtpAudioSendHostAddress.data(),
                                destAddress.data(),
                                bViaOurAudioRelay,
                                bIgnore) ;
                    }
                }
                else
                {
                    bError = true ;
                    OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Unable to get audio rtcp address") ;
                }
            }
            
            if (pMediaConnection->mpRtpVideoSocketArray[i])
            {
                mVideoMediaConnectivityInfo.setIce(true) ;
                int timeMS = 0 ;
                if (pMediaConnection->mpRtpVideoSocketArray[i]->getBestDestinationAddress(destAddress, destPort, bViaOurVideoRelay, videoPriority, timeMS))
                {
                    rtpVideoSendHostAddress = destAddress;
                    rtpVideoSendHostPort = destPort;
                    if (pMediaConnection->mpRtpAudioSocketArray[i] == NULL)
                    {
                        bDone = true ;
                    }
                }          
                else
                {   
                    videoPriority = 0 ;
                    bError = true ;
                    bVideoFailure = true ;
                    OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Unable to get video rtp address") ;
                }
                mVideoMediaConnectivityInfo.setIceTime(timeMS) ;
            }

            if (pMediaConnection->mpRtcpVideoSocketArray[i] && mbEnableRTCP)
            {
                int timeMS = 0 ;
                if (pMediaConnection->mpRtcpVideoSocketArray[i]->getBestDestinationAddress(destAddress, destPort, bIgnore, iIgnore, timeMS))
                {
                    rtcpVideoSendHostPort = destPort;
                    if ((rtpVideoSendHostAddress.compareTo(destAddress) != 0) || (bViaOurVideoRelay != bIgnore))
                    {
                        bError = true ;

                        OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Video Rtp/Rtcp host mismatch: %s != %s or relay mismatch %d != %d",
                                rtpVideoSendHostAddress.data(),
                                destAddress.data(),
                                bViaOurVideoRelay,
                                bIgnore) ;
                    }
                }
                else
                {
                    bError = true ;
                    OsSysLog::add(FAC_MP, PRI_ERR, "ICE FAILURE: Unable to get video rtcp address") ;
                }
            }
        }

        // Select ARS audio if warranted
        if (pMediaConnection->mpArsAudioSocket && 
                ((pMediaConnection->miArsAudioPriority > audioPriority) || bError))
        {
            if (pMediaConnection->mpArsAudioSocket->isArsConnected())
            {
                if (pMediaConnection->mpArsAudioSocket->getArsFusion(rtpAudioSendHostAddress, rtpAudioSendHostPort))
                {
                    rtcpAudioSendHostPort = rtpAudioSendHostPort ;
                    bDone = true ;
                    bAudioSelectedArs = true ;
                    bAudioFailure = false ;
                    OsSysLog::add(FAC_MP, PRI_INFO, "Selected ARS for audio\n") ;
                }
            }
        }
        // Select ARS video if warranted
        if (pMediaConnection->mpArsVideoSocket && 
                ((pMediaConnection->miArsVideoPriority > videoPriority) || bAudioSelectedArs || bError))
        {
            if (pMediaConnection->mpArsVideoSocket->isArsConnected())
            {
                if (pMediaConnection->mpArsVideoSocket->getArsFusion(rtpVideoSendHostAddress, rtpVideoSendHostPort))
                {
                    rtcpVideoSendHostPort = rtpVideoSendHostPort ;
                    bDone = true ;
                    bVideoFailure = false ;
                    bVideoSelectedArs = true ;
                    OsSysLog::add(FAC_MP, PRI_INFO, "Selected ARS for video\n") ;
                }
            }
        }

        if (bAudioSelectedArs && (pMediaConnection->mpArsVideoSocket == NULL || bVideoSelectedArs))
            bError = false ;

        setConnectionDestination(connectionId, 
                rtpAudioSendHostAddress, rtpAudioSendHostPort, rtcpAudioSendHostPort, 
                bAudioSelectedArs ? 2 : (bViaOurAudioRelay ? 1 : 0),
                rtpVideoSendHostAddress, rtpVideoSendHostPort, rtcpVideoSendHostPort, 
                bVideoSelectedArs ? 2 : (bViaOurVideoRelay ? 1 : 0)) ;
    }

    return bDone && !bError ;
}

OsStatus CpMediaInterface::setConnectionDestination(int connectionId,
                                                    const char* remoteAudioAddress,
                                                    int remoteAudioRtpPort,
                                                    int remoteAudioRtcpPort,
                                                    int remoteAudioType,
                                                    const char* remoteVideoAddress,
                                                    int remoteVideoRtpPort,
                                                    int remoteVideoRtcpPort,
                                                    int remoteVideoType)
{    
    OS_PERF_FUNC("VoiceEngineMediaInterface::setConnectionDestination") ;
    OsLock lock(*mpMediaGuard) ;

    traceAPI("VoiceEngineMediaInterface::setConnectionDestination") ;

    trace(PRI_INFO, "setConnectionDestination: id=%d\n\taudio=%s:%d/%d ourRelay=%d ars=%d\n\tvideo=%s:%d/%d ourRelay=%d ars=%d",
            connectionId,
            remoteAudioAddress ? remoteAudioAddress : "",
            remoteAudioRtpPort,
            remoteAudioRtcpPort,
            remoteAudioType == 1,
            remoteAudioType == 2,
            remoteVideoAddress  ? remoteVideoAddress : "",
            remoteVideoRtpPort,
            remoteVideoRtcpPort,
            remoteVideoType == 1,
            remoteVideoType == 2) ;
   
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        // Store connection destination Info for Audio
        mAudioMediaConnectivityInfo.setRemoteMediaAddr(remoteAudioAddress, remoteAudioRtpPort) ;
        switch (remoteAudioType)
        {
        case 0:
            mAudioMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_NONE) ;
            break ;
        case 1:
            mAudioMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_TURN_UDP) ;
            break ;
        case 2:
            assert(pMediaConnection->mpArsAudioSocket != NULL) ;
            if (pMediaConnection->mpArsAudioSocket && pMediaConnection->mpArsAudioSocket->isUsingHttpsProxy())
                mAudioMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_ARS_HTTPS) ;
            else
                mAudioMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_ARS) ;
            break ;
        default:
            assert(false) ;
            break ;
        }

        // Store connection destination Info for Video
        mVideoMediaConnectivityInfo.setRemoteMediaAddr(remoteVideoAddress, remoteVideoRtpPort) ;
        switch (remoteVideoType)
        {
        case 0:
            mVideoMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_NONE) ;
            break ;
        case 1:
            mVideoMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_TURN_UDP) ;
            break ;
        case 2:
            assert(pMediaConnection->mpArsVideoSocket != NULL) ;
            if (pMediaConnection->mpArsVideoSocket && pMediaConnection->mpArsVideoSocket->isUsingHttpsProxy())
                mVideoMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_ARS_HTTPS) ;
            else
                mVideoMediaConnectivityInfo.setOurRelayType(MediaConnectivityInfo::MCIRT_ARS) ;
            break ;
        default:
            assert(false) ;
            break ;
        }

        /*
         * Common setup
         */
        pMediaConnection->mDestinationSet = TRUE;
        pMediaConnection->mRtpSendHostAddress = remoteAudioAddress;
        pMediaConnection->mRtpAudioSendHostPort = remoteAudioRtpPort;
        pMediaConnection->mRtcpAudioSendHostPort = remoteAudioRtcpPort;
        pMediaConnection->mRtpVideoSendHostPort = remoteVideoRtpPort;
        pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort;

        /*
         * Audio setup
         */             
        for (int i = 0; i < 2; i++) // UDP and TCP
        {    
            if (pMediaConnection->mpRtpAudioSocketArray[i])
            {
                if (remoteAudioType == 2)   // ars
                {
                    pMediaConnection->mbUsingAudioARS = true ;
                    pMediaConnection->mpAudioSocketAdapterArray[i]->setArsSocket(pMediaConnection->mpArsAudioSocket) ;
                    pMediaConnection->mpRtpAudioSocketArray[i]->disableTurn(false) ;
                    doUnmonitor(pMediaConnection->mpRtpAudioSocketArray[i]->getSocket()) ;
                }
                else if (remoteAudioType == 1)   // our turn relay
                {
                    pMediaConnection->mpRtpAudioSocketArray[i]->setTurnDestination(remoteAudioAddress, remoteAudioRtpPort) ;
                    pMediaConnection->mpAudioSocketAdapterArray[i]->setRtpDestination(mTurnProxy.getPort(), mTurnProxy.getAddress()) ;
                    if (getMediaSocketPtr(pMediaConnection->mpRtpAudioSocketArray[i]))
                        getMediaSocketPtr(pMediaConnection->mpRtpAudioSocketArray[i])->setPreferredReceiveAddress(mTurnProxy.getAddress(), mTurnProxy.getPort()) ;
                    pMediaConnection->mbUsingAudioARS = false ;
                    if (pMediaConnection->mpArsAudioSocket)
                    {
                        doUnmonitor(pMediaConnection->mpArsAudioSocket) ;
                        pMediaConnection->mpArsAudioSocket->close() ;
                    }
                }
                else
                {
                    pMediaConnection->mpRtpAudioSocketArray[i]->disableTurn(false) ;
                    pMediaConnection->mpAudioSocketAdapterArray[i]->setRtpDestination(remoteAudioRtpPort, remoteAudioAddress) ;
                    if (getMediaSocketPtr(pMediaConnection->mpRtpAudioSocketArray[i]))
                        getMediaSocketPtr(pMediaConnection->mpRtpAudioSocketArray[i])->setPreferredReceiveAddress(remoteAudioAddress, remoteAudioRtpPort) ;
                    pMediaConnection->mbUsingAudioARS = false ;
                    if (pMediaConnection->mpArsAudioSocket) 
                    {
                        doUnmonitor(pMediaConnection->mpArsAudioSocket) ;
                        pMediaConnection->mpArsAudioSocket->close() ;
                    }
                }

                if (mbEnableRTCP && (remoteAudioRtcpPort > 0) && pMediaConnection->mpRtcpAudioSocketArray[i])
                {
                    if (remoteAudioType == 2)   // ars
                    {
                        pMediaConnection->mpRtcpAudioSocketArray[i]->disableTurn(false) ;
                    }
                    else if (remoteAudioType == 1)   // our turn relay
                    {
                        pMediaConnection->mpRtcpAudioSocketArray[i]->setTurnDestination(remoteAudioAddress, remoteAudioRtcpPort) ;
                        pMediaConnection->mpAudioSocketAdapterArray[i]->setRtcpDestination(mTurnProxy.getPort(), mTurnProxy.getAddress()) ;
                        if (getMediaSocketPtr(pMediaConnection->mpRtcpAudioSocketArray[i]))
                            getMediaSocketPtr(pMediaConnection->mpRtcpAudioSocketArray[i])->setPreferredReceiveAddress(mTurnProxy.getAddress(), mTurnProxy.getPort()) ;
                        // ars socket is closed during rtp check
                    }
                    else
                    {
                        pMediaConnection->mpRtcpAudioSocketArray[i]->disableTurn(false) ;
                        pMediaConnection->mpAudioSocketAdapterArray[i]->setRtcpDestination(remoteAudioRtcpPort, remoteAudioAddress) ;
                        if (getMediaSocketPtr(pMediaConnection->mpRtcpAudioSocketArray[i]))
                            getMediaSocketPtr(pMediaConnection->mpRtcpAudioSocketArray[i])->setPreferredReceiveAddress(remoteAudioAddress, remoteAudioRtcpPort) ;
                        // ars socket is closed during rtp check
                    }
                }
                else
                {
                    pMediaConnection->mRtcpAudioSendHostPort = 0 ;
                }
            }          
        }
        /*
         * Video Setup
         */
        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            if (pMediaConnection->mpRtpVideoSocketArray[i])
            {
                if (remoteVideoType == 2)   // ars
                {
                    pMediaConnection->mbUsingVideoARS = true ;
                    pMediaConnection->mpVideoSocketAdapterArray[i]->setArsSocket(pMediaConnection->mpArsVideoSocket) ;
                    pMediaConnection->mpRtpVideoSocketArray[i]->disableTurn(false) ;
                    doUnmonitor(pMediaConnection->mpRtpVideoSocketArray[i]->getSocket()) ;
                }
                else if (remoteVideoType == 1)    // our turn relay
                {
                    pMediaConnection->mpRtpVideoSocketArray[i]->setTurnDestination(remoteVideoAddress, remoteVideoRtpPort) ;
                    pMediaConnection->mpVideoSocketAdapterArray[i]->setRtpDestination(mTurnProxy.getPort(), mTurnProxy.getAddress()) ;
                    if (getMediaSocketPtr(pMediaConnection->mpRtpVideoSocketArray[i]))
                        getMediaSocketPtr(pMediaConnection->mpRtpVideoSocketArray[i])->setPreferredReceiveAddress(mTurnProxy.getAddress(), mTurnProxy.getPort()) ;                    
                    pMediaConnection->mbUsingVideoARS = false ;
                    if (pMediaConnection->mpArsVideoSocket)
                    {
                        doUnmonitor(pMediaConnection->mpArsVideoSocket) ;
                        pMediaConnection->mpArsVideoSocket->close() ;
                    }
                }
                else
                {
                    pMediaConnection->mpRtpVideoSocketArray[i]->disableTurn(false) ;
                    pMediaConnection->mpVideoSocketAdapterArray[i]->setRtpDestination(remoteVideoRtpPort, remoteVideoAddress) ;
                    if (getMediaSocketPtr(pMediaConnection->mpRtpVideoSocketArray[i]))
                        getMediaSocketPtr(pMediaConnection->mpRtpVideoSocketArray[i])->setPreferredReceiveAddress(remoteVideoAddress, remoteVideoRtpPort) ;
                    pMediaConnection->mbUsingVideoARS = false ;
                    if (pMediaConnection->mpArsVideoSocket)
                    {
                        doUnmonitor(pMediaConnection->mpArsVideoSocket) ;
                        pMediaConnection->mpArsVideoSocket->close() ;
                    }
                }

                if (mbEnableRTCP && (remoteVideoRtcpPort > 0) && pMediaConnection->mpRtcpVideoSocketArray[i])
                {
                    if (remoteVideoType == 2)   // ars
                    {
                        pMediaConnection->mpRtcpVideoSocketArray[i]->disableTurn(false) ;
                    }
                    else if (remoteVideoType == 1)   // our turn relay
                    {
                        pMediaConnection->mpRtcpVideoSocketArray[i]->setTurnDestination(remoteVideoAddress, remoteVideoRtcpPort) ;
                        pMediaConnection->mRtcpVideoSendHostPort = mTurnProxy.getPort() ;
                        pMediaConnection->mpVideoSocketAdapterArray[i]->setRtcpDestination(mTurnProxy.getPort(), mTurnProxy.getAddress()) ;
                        if (getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i]))
                            getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i])->setPreferredReceiveAddress(mTurnProxy.getAddress(), mTurnProxy.getPort()) ;
                        // ars socket is closed during rtp check
                    }
                    else
                    {
                        pMediaConnection->mpRtcpVideoSocketArray[i]->disableTurn(false) ;
                        pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;
                        pMediaConnection->mpVideoSocketAdapterArray[i]->setRtcpDestination(remoteVideoRtcpPort, remoteVideoAddress) ;
                        if (getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i]))
                            getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i])->setPreferredReceiveAddress(remoteVideoAddress, remoteVideoRtcpPort) ;
                        // ars socket is closed during rtp check
                    }
                }
                else
                {
                    pMediaConnection->mRtcpVideoSendHostPort = 0 ;
                }
            }           
        }
    }

    return OS_SUCCESS ;
}

void CpMediaInterface::doEnableMonitoring(bool bEnable, OsSocket* pSocket)
{
    CpMediaSocketMonitorTask* pMonTask = CpMediaSocketMonitorTask::getInstance() ;

    if (pSocket && pMonTask->isMonitored(pSocket))
    {
        if (bEnable)
        {
            pMonTask->enableSocket(pSocket) ;
        }
        else
        {            
            pMonTask->disableSocket(pSocket) ;
        }
    }
}

void CpMediaInterface::doEnableMonitoring(bool bEnable, 
                                                   bool bAudio,
                                                   bool bVideo,
                                                   CpMediaConnection* pMediaConnection) 
{
    CpMediaSocketMonitorTask* pMonTask = CpMediaSocketMonitorTask::getInstance() ;

    if (pMonTask && pMediaConnection)
    {
        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            if (bAudio && pMediaConnection->mpRtpAudioSocketArray[i])
            {
                doEnableMonitoring(bEnable, pMediaConnection->mpRtpAudioSocketArray[i]->getSocket()) ;
            }

            if (bVideo && pMediaConnection->mpRtpVideoSocketArray[i])
            {
                doEnableMonitoring(bEnable, pMediaConnection->mpRtpVideoSocketArray[i]->getSocket()) ;
            }
        }

        if (bAudio && pMediaConnection->mpArsAudioSocket)
        {
            doEnableMonitoring(bEnable, pMediaConnection->mpArsAudioSocket) ;
        }

        if (bVideo && pMediaConnection->mpArsVideoSocket)
        {
            doEnableMonitoring(bEnable, pMediaConnection->mpArsVideoSocket) ;
        }
    }
}

void CpMediaInterface::doUnmonitor(OsSocket* pSocket) 
{
    CpMediaSocketMonitorTask* pMonTask = CpMediaSocketMonitorTask::getInstance() ;

    if (pMonTask && pSocket)
        pMonTask->unmonitor(pSocket) ;
}

bool CpMediaInterface::hasAudioInputDevice()
{
    bool bRet = false;

    bRet = (mpFactoryImpl->getNumAudioInputDevices() > 0);

    return bRet;
}

bool CpMediaInterface::hasAudioOutputDevice()
{
    bool bRet = false;

    bRet = (mpFactoryImpl->getNumAudioOutputDevices() > 0);

    return bRet;
}

bool CpMediaInterface::getMediaConnectivityInfo(MediaConnectivityInfo::MediaConnectivityInfoType type,
                                                MediaConnectivityInfo& rMediaConnectivityInfo) 
{
    bool bRC = false ;

    switch (type)
    {
    case MediaConnectivityInfo::MCIT_AUDIO_RTP:
        rMediaConnectivityInfo = mAudioMediaConnectivityInfo ;
        bRC = true ;
        break ;
    case MediaConnectivityInfo::MCIT_AUDIO_RTCP:
        // Not implemented
        break ;
    case MediaConnectivityInfo::MCIT_VIDEO_RTP:
        rMediaConnectivityInfo = mVideoMediaConnectivityInfo ;
        bRC = true ;
        break ;
    case MediaConnectivityInfo::MCIT_VIDEO_RTCP:
        // Not implemented
        break ;
    default:
        assert(false) ; // Bogus value
        break ;

    }
    return bRC ;
}


//////////////////////////////////////////////////////////////////////////////

UtlBoolean CpMediaInterface::containsVideoCodec(int numCodecs, SdpCodec* codecs[]) 
{
    UtlBoolean bFound = false ;

    for (int i=0; i<numCodecs; i++)
    {
        UtlString mimeType ;
        codecs[i]->getMediaType(mimeType) ;
        if (mimeType.compareTo(MIME_TYPE_VIDEO, UtlString::ignoreCase) == 0)
        {
            bFound = true ;
            break ;
        }
    }

    return bFound ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::getLocalUdpAddresses(int connectionId,
                                                     UtlString& hostIp,
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;
    transportType = RTP_TRANSPORT_UDP ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX])
        {
            hostIp = pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalIp();
            rtpAudioPort = pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();  
            if (rtpAudioPort > 0)
            {            
                bRC = true ;
            }

            // Audio rtcp port (optional) 
            if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX] && bRC)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();    
            }        
        }

        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX] && bRC)
        {
            rtpVideoPort = pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();

            // Video rtcp port (optional)
            if (pMediaConn->mpRtcpVideoSocketArray[UDP_TRANSPORT_INDEX])
            {
                rtcpVideoPort = pMediaConn->mpRtcpVideoSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();
            }
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::getLocalTcpAddresses(int connectionId,
                                                     UtlString& hostIp,
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;
    transportType = RTP_TRANSPORT_TCP ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocketArray[TCP_TRANSPORT_INDEX])
        {
            hostIp = pMediaConn->mpRtpAudioSocketArray[TCP_TRANSPORT_INDEX]->getSocket()->getLocalIp();
            rtpAudioPort = pMediaConn->mpRtpAudioSocketArray[TCP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();            
            if (rtpAudioPort > 0)
            {            
                bRC = true ;
            }

            // Audio rtcp port (optional) 
            if (pMediaConn->mpRtcpAudioSocketArray[TCP_TRANSPORT_INDEX] && bRC)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocketArray[TCP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();                
            }        
        }

        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocketArray[TCP_TRANSPORT_INDEX] && bRC)
        {
            rtpVideoPort = pMediaConn->mpRtpVideoSocketArray[TCP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();

            // Video rtcp port (optional)
            if (pMediaConn->mpRtcpVideoSocketArray[TCP_TRANSPORT_INDEX])
            {
                rtcpVideoPort = pMediaConn->mpRtcpVideoSocketArray[TCP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort();
            }
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::getNatedUdpAddresses(int connectionId,
                                                     UtlString& hostIp,
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    UtlString host ;
    int port ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;
    transportType = RTP_TRANSPORT_UDP ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX])
        {
            if (pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getMappedIp(&host, &port))
            {
                if (port > 0)
                {
                    hostIp = host ;
                    rtpAudioPort = port ;
                    bRC = true ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX] && bRC)
                {
                    if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getMappedIp(&host, &port))
                    {
                        rtcpAudioPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Stun host IP mismatches for rtcp/audio (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }
        }

        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX] && bRC)
        {
            if (pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX]->getMappedIp(&host, &port))
            {
                rtpVideoPort = port ;
                if (host.compareTo(hostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Stun host IP mismatches for rtp/video (%s != %s)", 
                            hostIp.data(), host.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX])
                {
                    if (pMediaConn->mpRtcpVideoSocketArray[UDP_TRANSPORT_INDEX]->getMappedIp(&host, &port))
                    {
                        rtcpVideoPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Stun host IP mismatches for rtcp/video (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }            
        }
    } // end if (pMediaCon)

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::getRelayUdpAddresses(int connectionId,
                                                     UtlString& audioHostIp,
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     UtlString& videoHostIp,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    UtlString host ;
    int port ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    audioHostIp.remove(0) ;
    videoHostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;
    transportType = RTP_TRANSPORT_UDP ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX])
        {
            if (pMediaConn->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getTurnIp(&host, &port))
            {
                if (port > 0)
                {
                    audioHostIp = host ;
                    rtpAudioPort = port ;

                    bRC = true ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX] && bRC)
                {
                    if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getTurnIp(&host, &port))
                    {
                        rtcpAudioPort = port ;
                        if (host.compareTo(audioHostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/audio (%s != %s)", 
                                    audioHostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }
        }

        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX] && bRC)
        {
            if (pMediaConn->mpRtpVideoSocketArray[UDP_TRANSPORT_INDEX]->getTurnIp(&host, &port))
            {
                if (port > 0)
                {
                    videoHostIp = host ;
                    rtpVideoPort = port ;

                }
                if (videoHostIp.compareTo(audioHostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Turn host IP mismatches for rtp/video (%s != %s)", 
                            videoHostIp.data(), audioHostIp.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX])
                {
                    if (pMediaConn->mpRtcpVideoSocketArray[UDP_TRANSPORT_INDEX]->getTurnIp(&host, &port))
                    {
                        rtcpVideoPort = port ;
                        if (host.compareTo(videoHostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/video (%s != %s)", 
                                    videoHostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }            
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool CpMediaInterface::getRelayTcpAddresses(int connectionId,
                                                     UtlString& audioHostIp,
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     UtlString& videoHostIp,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    UtlString host ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    audioHostIp.remove(0) ;
    videoHostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;
    transportType = RTP_TCP_ROLE_ACTIVE ;

    if (pMediaConn)
    {
        // TODO:: MCOHEN -- USE new TCP TURN connection
        
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////


#define MAX_ARS_WAIT_COUNT   400 
#define MAX_ARS_WAIT_DELAY   20
bool CpMediaInterface::getArsAddresses(int connectionId,
                                                UtlString& audioHostIp,
                                                int& rtpAudioPort,
                                                UtlString& videoHostIp,
                                                int& rtpVideoPort,
                                                RTP_TRANSPORT& transportType)
{
    bool bRC = false ;
    UtlString host ;
    int port ;
    CpMediaConnection* pMediaConn = 
            getMediaConnection(connectionId) ;

    audioHostIp.remove(0) ;
    videoHostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    transportType = RTP_TRANSPORT_ARS ;
    int waitCount = 0 ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpArsAudioSocket)
        {
            if (pMediaConn->mbInitiating)
            {
                while ( !pMediaConn->mpArsAudioSocket->isArsFusionAvailable() &&
                        !pMediaConn->mpArsAudioSocket->isArsFailed() &&                       
                        waitCount < MAX_ARS_WAIT_COUNT)
                {
                    OsTask::delay(MAX_ARS_WAIT_DELAY) ;                    
                    waitCount++ ;
                }

                if (pMediaConn->mpArsAudioSocket->getArsFusion(host, port))
                {
                    if (port > 0)
                    {
                        audioHostIp = host ;
                        rtpAudioPort = port ;

                        bRC = true ;
                    }
                }
                else
                    trace(PRI_ERR, "Unable to get audio ARS connection for SDP") ;
            }
            else
            {
                audioHostIp = "0.0.0.0" ;
                rtpAudioPort = mArsProxy.getPort() ;
                bRC = true ;
            }
        }

        // Video rtp port (optional)
        if (pMediaConn->mpArsVideoSocket && bRC)
        {
            if (pMediaConn->mbInitiating)
            {

                while ( !pMediaConn->mpArsVideoSocket->isArsFusionAvailable() &&
                        !pMediaConn->mpArsVideoSocket->isArsFailed() &&                       
                        waitCount < MAX_ARS_WAIT_COUNT+2)
                {
                    OsTask::delay(MAX_ARS_WAIT_DELAY) ;                    
                    waitCount++ ;
                }

                if (pMediaConn->mpArsVideoSocket->getArsFusion(host, port))
                {
                    if (port > 0)
                    {
                        videoHostIp = host ;
                        rtpVideoPort = port ;
                    }
                }
                else
                    trace(PRI_ERR, "Unable to get video ARS connection for SDP") ;

            }
            else
            {
                videoHostIp = "0.0.0.0" ;
                rtpVideoPort = mArsProxy.getPort() ;
            }
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addLocalContacts(int connectionId, 
                                            UtlSList& audioContacts,
                                            bool bAddAudioToConnInfo,
                                            UtlSList& videoContacts,
                                            bool bAddVideoToConnInfo)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    RTP_TRANSPORT transportType = RTP_TRANSPORT_UNKNOWN ;
    OsStatus rc = OS_FAILED ;

    // Local UDP Addresses
    // only add UDP addresses if the connection's mRtpTransport includes UDP
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
    if (    pConnection && 
            pConnection->mbEnableLocal &&
            (pConnection->mRtpTransport & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
    {
        if (getLocalUdpAddresses(connectionId, hostIp, rtpAudioPort, 
                rtcpAudioPort, rtpVideoPort, rtcpVideoPort, transportType))
        {
#if FORCE_AUDIO_TURN
            if (rtpAudioPort > 0)   rtpAudioPort += 10 ;
            if (rtcpAudioPort > 0)  rtcpAudioPort += 10 ;
#endif
#if FORCE_VIDEO_TURN
            if (rtpVideoPort > 0)   rtpVideoPort += 10 ;            
            if (rtcpVideoPort > 0)  rtcpVideoPort += 10 ;
#endif
            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(hostIp, rtpAudioPort, rtcpAudioPort, OsSocket::UDP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;
                
                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("udp", hostIp, rtpAudioPort) ;
            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(hostIp, rtpVideoPort, rtcpVideoPort, OsSocket::UDP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("udp", hostIp, rtpVideoPort) ;
            }           
            rc = OS_SUCCESS ;
        }
    }
    
        
    if (    pConnection && 
            pConnection->mbEnableLocal &&
            (pConnection->mRtpTransport & RTP_TRANSPORT_TCP) == RTP_TRANSPORT_TCP)
    {
        // Local TCP Address
        if (getLocalTcpAddresses(connectionId, hostIp, rtpAudioPort, 
                rtcpAudioPort, rtpVideoPort, rtcpVideoPort, transportType))
        {
#if FORCE_AUDIO_TURN
            if (rtpAudioPort > 0)   rtpAudioPort += 10 ;
            if (rtcpAudioPort > 0)  rtcpAudioPort += 10 ;
#endif
#if FORCE_VIDEO_TURN
            if (rtpVideoPort > 0)   rtpVideoPort += 10 ;            
            if (rtcpVideoPort > 0)  rtcpVideoPort += 10 ;
#endif
            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(hostIp, rtpAudioPort, rtcpAudioPort, OsSocket::TCP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;

                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("tcp", hostIp, rtpAudioPort) ;

            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(hostIp, rtpVideoPort, rtcpVideoPort, OsSocket::TCP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("tcp", hostIp, rtpVideoPort) ;
            }           
            rc = OS_SUCCESS ;
        }    
    }
    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addNatedContacts(int connectionId, 
                                            UtlSList& audioContacts,
                                            bool bAddAudioToConnInfo,
                                            UtlSList& videoContacts,
                                            bool bAddVideoToConnInfo)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    RTP_TRANSPORT transportType = RTP_TRANSPORT_UNKNOWN ;
    OsStatus rc = OS_FAILED ;

    // only add UDP addresses if the connection's mRtpTransport includes UDP
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
    if (    pConnection && 
            pConnection->mbEnableSTUN &&
            (pConnection->mRtpTransport & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
    {
        // NAT UDP Addresses
        if (getNatedUdpAddresses(connectionId, hostIp, rtpAudioPort, 
                rtcpAudioPort, rtpVideoPort, rtcpVideoPort, transportType))
        {
#if FORCE_AUDIO_TURN
            if (rtpAudioPort > 0)   rtpAudioPort += 10 ;
            if (rtcpAudioPort > 0)  rtcpAudioPort += 10 ;
#endif
#if FORCE_VIDEO_TURN
            if (rtpVideoPort > 0)   rtpVideoPort += 10 ;            
            if (rtcpVideoPort > 0)  rtcpVideoPort += 10 ;
#endif
            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(hostIp, rtpAudioPort, rtcpAudioPort, OsSocket::UDP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;

                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("udp_stun", hostIp, rtpAudioPort) ;
            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(hostIp, rtpVideoPort, rtcpVideoPort, OsSocket::UDP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("udp_stun", hostIp, rtpVideoPort) ;
            }           
            rc = OS_SUCCESS ;
        }
    }

    // TCP doesn't make sense for NAT contacts...
    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addRelayContacts(int connectionId, 
                                            UtlSList& audioContacts,
                                            bool bAddAudioToConnInfo,
                                            UtlSList& videoContacts,
                                            bool bAddVideoToConnInfo)
{
    UtlString audioHostIp, videoHostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    RTP_TRANSPORT transportType = RTP_TRANSPORT_UNKNOWN ;
    
    OsStatus rc = OS_FAILED ;

    // Relay UDP Addresses
    // only add UDP addresses if the connection's mRtpTransport includes UDP
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
    if (    pConnection && 
            pConnection->mbEnableTURN &&
            (pConnection->mRtpTransport & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
    {
        if (getRelayUdpAddresses(connectionId, audioHostIp, rtpAudioPort, 
                rtcpAudioPort, videoHostIp, rtpVideoPort, rtcpVideoPort, transportType))
        {
#if FORCE_AUDIO_TURN
            if (rtpAudioPort > 0)   rtpAudioPort += 10 ;
            if (rtcpAudioPort > 0)  rtcpAudioPort += 10 ;
#endif
#if FORCE_VIDEO_TURN
            if (rtpVideoPort > 0)   rtpVideoPort += 10 ;            
            if (rtcpVideoPort > 0)  rtcpVideoPort += 10 ;
#endif
            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(audioHostIp, rtpAudioPort, rtcpAudioPort, OsSocket::UDP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;

                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("udp_turn", audioHostIp, rtpAudioPort) ;
            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(videoHostIp, rtpVideoPort, rtcpVideoPort, OsSocket::UDP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("udp_turn", videoHostIp, rtpVideoPort) ;
            }           
            rc = OS_SUCCESS ;
        }
    }    
    
    
    // Relay TCP Addresses
    if (    pConnection && 
            pConnection->mbEnableTURN &&
            (pConnection->mRtpTransport & RTP_TRANSPORT_TCP) == RTP_TRANSPORT_TCP)
    {
    
        if (getRelayTcpAddresses(connectionId, audioHostIp, rtpAudioPort, 
                rtcpAudioPort, videoHostIp, rtpVideoPort, rtcpVideoPort, transportType))
        {
#if FORCE_AUDIO_TURN
            if (rtpAudioPort > 0)   rtpAudioPort  += 10 ;
            if (rtcpAudioPort > 0)  rtcpAudioPort += 10 ;
#endif
#if FORCE_VIDEO_TURN
            if (rtpVideoPort > 0)   rtpVideoPort  += 10 ;            
            if (rtcpVideoPort > 0)  rtcpVideoPort += 10 ;
#endif

            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(audioHostIp, rtpAudioPort, rtcpAudioPort, OsSocket::TCP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;

                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("tcp_turn", audioHostIp, rtpAudioPort) ;
            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(videoHostIp, rtpVideoPort, rtcpVideoPort, OsSocket::TCP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("tcp_turn", videoHostIp, rtpVideoPort) ;

            }           
            rc = OS_SUCCESS ;
        }
    }    

    return rc ;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus CpMediaInterface::addArsContacts(int connectionId, 
                                          UtlSList& audioContacts,
                                          bool bAddAudioToConnInfo,
                                          UtlSList& videoContacts,
                                          bool bAddVideoToConnInfo)
{
    UtlString audioHostIp, videoHostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    RTP_TRANSPORT transportType = RTP_TRANSPORT_ARS ;
    
    OsStatus rc = OS_FAILED ;

    // Relay UDP Addresses
    // only add UDP addresses if the connection's mRtpTransport includes UDP
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
    if (pConnection && pConnection->mbEnableARS)
    {
        if (getArsAddresses(connectionId, audioHostIp, rtpAudioPort, 
                videoHostIp, rtpVideoPort, transportType))
        {
            if (rtpAudioPort > 0)
            {
                OsMediaContact* pAudioContact = new OsMediaContact(audioHostIp, rtpAudioPort, PORT_NONE, OsSocket::TCP, transportType) ;
                if (!audioContacts.contains(pAudioContact))
                    audioContacts.append(pAudioContact) ;
                else
                    delete pAudioContact ;

                if (bAddAudioToConnInfo)
                    mAudioMediaConnectivityInfo.addLocalCandidate("ars", audioHostIp, rtpAudioPort) ;

            }

            if (rtpVideoPort > 0)
            {
                OsMediaContact* pVideoContact = new OsMediaContact(videoHostIp, rtpVideoPort, PORT_NONE, OsSocket::TCP, transportType) ;
                if (!videoContacts.contains(pVideoContact))
                    videoContacts.append(pVideoContact) ;
                else
                    delete pVideoContact ;

                if (bAddVideoToConnInfo)
                    mVideoMediaConnectivityInfo.addLocalCandidate("ars", videoHostIp, rtpVideoPort) ;                
            }
            rc = OS_SUCCESS ;
        }
    }    
    
    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

int CpMediaInterface::getNumConnections()
{
    return mMediaConnections.entries();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void CpMediaInterface::limitVideoCodecs(int connectionId, UtlString& videoCodec)
{
    OS_PERF_FUNC("CpMediaInterface::limitVideoCodecs") ;
    OsLock lock(*mpMediaGuard) ;

    SdpCodecList tempFac;
    int codecIndex=0;
    int numAudioCodecs;
    int numVideoCodecs;
    int videoSize=0;
    UtlString subMimeType;
    UtlString sizeString;
    UtlString encodingName;
    SdpCodec** audioCodecsArray = NULL;
    SdpCodec** videoCodecsArray = NULL;
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    // Find out video size from videoCodec name, split codec name into submimetype
    // and size
    int sep = videoCodec.index('-');
    if (sep != UTL_NOT_FOUND)
    {
        subMimeType = videoCodec(0, sep);
        sizeString = videoCodec(sep+1, videoCodec.length() - (sep+1));
        sizeString.toLower();
        if (sizeString.compareTo("cif") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_CIF;
        }
#ifdef INCLUDE_QCIF_RESOLUTION
        else if (sizeString.compareTo("qcif") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_QCIF;
        }
#endif
#ifdef INCLUDE_SQCIF_RESOLUTION
        else if (sizeString.compareTo("sqcif") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_SQCIF;
        }
#endif
#ifdef INCLUDE_QVGA_RESOLUTION
        else if (sizeString.compareTo("qvga") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_QVGA;
        }
#endif
#ifdef INCLUDE_VGA_RESOLUTION
        else if (sizeString.compareTo("vga") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_VGA;
        }
#endif
#ifdef INCLUDE_4CIF_RESOLUTION
        else if (sizeString.compareTo("4cif") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_4CIF;
        }
#endif
#ifdef INCLUDE_16CIF_RESOLUTION
        else if (sizeString.compareTo("16cif") == 0)
        {
            videoSize = SDP_VIDEO_FORMAT_16CIF;
        }
#endif
    }
    // Did we have a legal video codec name and size?
    if (pMediaConn && videoSize != 0)
    {
        // Get all supported audio codecs
        mSupportedCodecs.getCodecs(numAudioCodecs, audioCodecsArray, "audio");
        // Build temporary factory for audio codecs
        for (codecIndex = 0; codecIndex<numAudioCodecs; codecIndex++)
        {
            tempFac.addCodec(*audioCodecsArray[codecIndex]);
        }
        // Now get all supported video codecs
        mSupportedCodecs.getCodecs(numVideoCodecs, videoCodecsArray, "video");
        // Find matching video codec name and size and add only one to codec factory
        for (codecIndex = 0; codecIndex<numVideoCodecs; codecIndex++)
        {
            videoCodecsArray[codecIndex]->getEncodingName(encodingName);
            // Add codec only if submimetype and video format match
            if (subMimeType.compareTo(encodingName, UtlString::ignoreCase) == 0 &&
                videoCodecsArray[codecIndex]->getVideoFormat() == videoSize)
            {
                tempFac.addCodec(*videoCodecsArray[codecIndex]);
            }
        }
        // Clear our old codec factory and copy over the just rebuilt one
        mSupportedCodecs.clearCodecs();
        mSupportedCodecs =  tempFac;

        // Free up the codecs and the array
        for (codecIndex = 0; codecIndex < numAudioCodecs; codecIndex++)
        {
            delete audioCodecsArray[codecIndex];
            audioCodecsArray[codecIndex] = NULL;
        }
        delete[] audioCodecsArray;
        for (codecIndex = 0; codecIndex < numVideoCodecs; codecIndex++)
        {
            delete videoCodecsArray[codecIndex];
            videoCodecsArray[codecIndex] = NULL;
        }
        delete[] videoCodecsArray;
        videoCodecsArray = NULL;        
        audioCodecsArray = NULL;

    }
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaInterface::setConnectionTcpRole(const int connectionId,
                                                      const RtpTcpRoles role)
{
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
    
    if (pConnection)
    {
        pConnection->setTcpRole(role);
    }
}

void CpMediaInterface::trace(OsSysLogPriority priority, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    /* Guess we need no more than 128 bytes. */
    int n, size = 128;
    char *p;

    p = (char*) malloc(size) ;
    
    while (p != NULL)
    {
        /* Try to print in the allocated space. */
#ifdef _WIN32
        n = _vsnprintf (p, size, format, args);
#else
        n = vsnprintf (p, size, format, args);
#endif

        /* If that worked, return the string. */
        if (n > -1 && n < size)
        {
            break;
        }
        /* Else try again with more space. */
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        if ((p = (char*) realloc (p, size)) == NULL)
        {
            break;
        }
    }

    if (p != NULL)
    {
        OsSysLog::add(FAC_MP, priority, "%s", p) ;
        if (mbConsoleTrace)
        {
            printf("%s\n", p) ;            
        }
        free(p) ;
    }

    va_end(args) ;
}

void CpMediaInterface::traceAPI(const char* szAPI) 
{
#if defined(_WIN32) && defined(_DEBUG)  
    UtlString consoleOutput ;
    OsDateTime::getLocalTimeString(consoleOutput) ;
    consoleOutput.append(" ") ;
    consoleOutput.append(szAPI) ;

    trace(PRI_DEBUG, consoleOutput.data()) ;
#endif
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
