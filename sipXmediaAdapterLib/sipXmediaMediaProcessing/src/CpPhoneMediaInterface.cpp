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
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <os/OsDatagramSocket.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsProtectEventMgr.h>
#include "include/CpPhoneMediaInterface.h"
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include <mp/MpMediaTask.h>
#include <mp/MpCallFlowGraph.h>
#include <mp/MpStreamPlayer.h>
#include <mp/MpStreamPlaylistPlayer.h>
#include <mp/MpStreamQueuePlayer.h>
#include <mp/dtmflib.h>

#include <net/SdpCodec.h>


#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MINIMUM_DTMF_LENGTH 60
#define MAX_RTP_PORTS 1000

//#define TEST_PRINT

// STATIC VARIABLE INITIALIZATIONS

class CpPhoneMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    CpPhoneMediaConnection(int connectionId = -1) :
      UtlInt(connectionId)
    {
        mpRtpAudioSocket = NULL;
        mpRtcpAudioSocket = NULL;
        mRtpAudioSendHostPort = 0;
        mRtcpAudioSendHostPort = 0;
        mRtpAudioReceivePort = 0;
        mRtcpAudioReceivePort = 0;
        mDestinationSet = FALSE;       
        mRtpAudioSending = FALSE;
        mRtpAudioReceiving = FALSE;
        mpCodecFactory = NULL;
        mpPrimaryCodec = NULL;
        mContactType = AUTO ;
        mbAlternateDestinations = FALSE ;
    };

    virtual ~CpPhoneMediaConnection()
    {
        if(mpRtpAudioSocket)
        {

#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpPhoneMediaConnection deleting RTP socket: %p descriptor: %d",
                mpRtpAudioSocket, mpRtpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtpAudioSocket;
            mpRtpAudioSocket = NULL;
        }
        if(mpRtcpAudioSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpPhoneMediaConnection deleting RTCP socket: %p descriptor: %d",
                mpRtcpAudioSocket, mpRtcpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtcpAudioSocket;
            mpRtcpAudioSocket = NULL;
        }
        if(mpCodecFactory)
        {
            delete mpCodecFactory;
            mpCodecFactory = NULL;
        }

        if (mpPrimaryCodec)
        {
            delete mpPrimaryCodec;
            mpPrimaryCodec = NULL; 
        }              
    }

    OsNatDatagramSocket* mpRtpAudioSocket;
    OsNatDatagramSocket* mpRtcpAudioSocket;
    UtlString mRtpSendHostAddress;
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    UtlBoolean mDestinationSet;
    UtlBoolean mRtpAudioSending;
    UtlBoolean mRtpAudioReceiving;
    SdpCodecFactory* mpCodecFactory;
    SdpCodec* mpPrimaryCodec;
    CONTACT_TYPE mContactType ;
    UtlString mLocalAddress ;
    UtlBoolean mbAlternateDestinations ;
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpPhoneMediaInterface::CpPhoneMediaInterface(CpMediaInterfaceFactoryImpl* pFactoryImpl,
                                             const char* publicAddress,
                                             const char* localAddress,
                                             int numCodecs,
                                             SdpCodec* sdpCodecArray[],
                                             const char* locale,
                                             int expeditedIpTos,
                                             const char* szStunServer,
                                             int iStunPort,
                                             int iStunKeepAlivePeriodSecs,
                                             const char* szTurnServer,
                                             int iTurnPort,
                                             const char* szTurnUsername,
                                             const char* szTurnPassword,
                                             int iTurnKeepAlivePeriodSecs,
                                             bool mbEnableICE)
    : CpMediaInterface(pFactoryImpl)
{
   mpFlowGraph = new MpCallFlowGraph(locale);
   
   mStunServer = szStunServer ;
   mStunPort = iStunPort ;
   mStunRefreshPeriodSecs = iStunKeepAlivePeriodSecs ;
   mTurnServer = szTurnServer ;
   mTurnPort = iTurnPort ;
   mTurnRefreshPeriodSecs = iTurnKeepAlivePeriodSecs ;
   mTurnUsername = szTurnUsername ;
   mTurnPassword = szTurnPassword ;
   mbEnableICE = mbEnableICE ;

   if(localAddress && *localAddress)
   {
       mRtpReceiveHostAddress = localAddress;
       mLocalAddress = localAddress;
   }
   else
   {
       OsSocket::getHostIp(&mLocalAddress);
   }

   if(sdpCodecArray && numCodecs > 0)
   {
       mSupportedCodecs.addCodecs(numCodecs, sdpCodecArray);

       // Assign any unset payload types
       mSupportedCodecs.bindPayloadTypes();
   }
   else
   {
       // Temp hard code codecs
       //SdpCodec mapCodecs1(SdpCodec::SDP_CODEC_PCMU, SdpCodec::SDP_CODEC_PCMU);
       //mSupportedCodecs.addCodec(mapCodecs1);
       //SdpCodec mapCodecs2(SdpCodec::SDP_CODEC_PCMA, SdpCodec::SDP_CODEC_PCMA);
       //mSupportedCodecs.addCodec(mapCodecs2);
       //mapCodecs[2] = new SdpCodec(SdpCodec::SDP_CODEC_L16_MONO);
       UtlString codecs = "PCMU PCMA TELEPHONE-EVENT";
       mSupportedCodecs.buildSdpCodecFactory(codecs);
   }

   mExpeditedIpTos = expeditedIpTos;
}


// Destructor
CpPhoneMediaInterface::~CpPhoneMediaInterface()
{
    CpPhoneMediaConnection* mediaConnection = NULL;
    while ((mediaConnection = (CpPhoneMediaConnection*) mMediaConnections.get()))
    {
        doDeleteConnection(mediaConnection);
        delete mediaConnection;
        mediaConnection = NULL;
    }

    if(mpFlowGraph)
    {
      // Free up the resources used by tone generation ASAP
      stopTone();

        // Stop the net in/out stuff before the sockets are deleted
        //mpMediaFlowGraph->stopReceiveRtp();
        //mpMediaFlowGraph->stopSendRtp();

        MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);

        // take focus away from the flow graph if it is focus
        if(mpFlowGraph == (MpCallFlowGraph*) mediaTask->getFocus())
        {
            mediaTask->setFocus(NULL);
        }
        delete mpFlowGraph;
        mpFlowGraph = NULL;
    }
}

/**
 * public interface for destroying this media interface
 */ 
void CpPhoneMediaInterface::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */

OsStatus CpPhoneMediaInterface::createConnection(int& connectionId,
                                                 const char* szLocalAddress,
                                                 void* videoWindowHandle, 
                                                 void* const pSecurityAttributes,
                                                 ISocketIdle* pIdleSink,
                                                 IMediaEventListener* pMediaEventListener )
{
    int localPort  ;
    OsStatus returnCode;
    {
        connectionId = mpFlowGraph->createConnection();
        mpFactoryImpl->getNextRtpPort(localPort);

        int iNextRtpPort = localPort ;

        CpPhoneMediaConnection* mediaConnection = new CpPhoneMediaConnection();
        mediaConnection->setValue(connectionId) ;
        mMediaConnections.append(mediaConnection);

        // Set Local address
        if (szLocalAddress && strlen(szLocalAddress))
        {
            mediaConnection->mLocalAddress = szLocalAddress ;
        }
        else
        {
            mediaConnection->mLocalAddress = mLocalAddress ;
        }

        // Create the sockets
        // Eventually this should use a specified address as this
        // host may be multi-homed
        OsNatDatagramSocket* rtpSocket = new OsNatDatagramSocket(0, NULL,
            localPort, mediaConnection->mLocalAddress, NULL);
        rtpSocket->enableTransparentReads(false);

        OsNatDatagramSocket* rtcpSocket = new OsNatDatagramSocket(0, NULL,
            localPort == 0 ? 0 : localPort + 1, mediaConnection->mLocalAddress, NULL);
        rtcpSocket->enableTransparentReads(false);

        // Validate local port is not auto-selecting.
        if (localPort != 0)
        {
            // If either of the sockets are bad (e.g. already in use) or
            // if either have stuff on them to read (e.g. someone is
            // sending junk to the ports, look for another port pair
            while(!rtpSocket->isOk() || !rtcpSocket->isOk() ||
                   rtcpSocket->isReadyToRead() ||
                   rtpSocket->isReadyToRead(60))
            {
                localPort +=2;
                // This should use mLastRtpPort instead of some
                // hardcoded MAX, but I do not think mLastRtpPort
                // is set correctly in all of the products.
                if(localPort > iNextRtpPort + MAX_RTP_PORTS) 
                {
                    OsSysLog::add(FAC_CP, PRI_ERR, 
                        "No available ports for RTP and RTCP in range %d - %d",
                        iNextRtpPort, iNextRtpPort + MAX_RTP_PORTS);
                    break;  // time to give up
                }

                delete rtpSocket;
                delete rtcpSocket;
                rtpSocket = new OsNatDatagramSocket(0, NULL, localPort,
                   mLocalAddress.data(), NULL) ;
                rtcpSocket = new OsNatDatagramSocket(0, NULL, localPort + 1,
                   mLocalAddress.data(), NULL);
            }
        }

        // Set a maximum on the buffers for the sockets so
        // that the network stack does not get swamped by early media
        // from the other side;
        {
            int sRtp, sRtcp, oRtp, oRtcp, optlen;

            sRtp = rtpSocket->getSocketDescriptor();
            sRtcp = rtcpSocket->getSocketDescriptor();

            optlen = sizeof(int);
            oRtp = 2000;
            setsockopt(sRtp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtp), optlen);
            oRtcp = 500;
            setsockopt(sRtcp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtcp), optlen);

            // Set the type of service (DiffServ code point) to low delay
            int tos = mExpeditedIpTos;
            
            setsockopt (sRtp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
            setsockopt (sRtcp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
        }

        // Enable Stun if we have a stun server and either non-local contact type or 
        // ICE is enabled.
        if ((mStunServer.length() != 0) && ((mediaConnection->mContactType != LOCAL) || mbEnableICE))
        {
            rtpSocket->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
        }

        // Enable Turn if we have a stun server and either non-local contact type or 
        // ICE is enabled.
        if ((mTurnServer.length() != 0) && ((mediaConnection->mContactType != LOCAL) || mbEnableICE))
        {
            rtpSocket->enableTurn(mTurnServer, mTurnPort, 
                    mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
        }

        // Enable Stun if we have a stun server and either non-local contact type or 
        // ICE is enabled.
        if ((mStunServer.length() != 0) && ((mediaConnection->mContactType != LOCAL) || mbEnableICE))
        {
            rtcpSocket->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
        }

        // Enable Turn if we have a stun server and either non-local contact type or 
        // ICE is enabled.
        if ((mTurnServer.length() != 0) && ((mediaConnection->mContactType != LOCAL) || mbEnableICE))
        {
            rtcpSocket->enableTurn(mTurnServer, mTurnPort, 
                    mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
        }

        // Store settings
        mediaConnection->mpRtpAudioSocket = rtpSocket;
        mediaConnection->mpRtcpAudioSocket = rtcpSocket;
        mediaConnection->mRtpAudioReceivePort = rtpSocket->getLocalHostPort() ;
        mediaConnection->mRtcpAudioReceivePort = rtcpSocket->getLocalHostPort() ;
        mediaConnection->mpCodecFactory = new SdpCodecFactory(mSupportedCodecs);
        mediaConnection->mpCodecFactory->bindPayloadTypes();

        returnCode = OS_SUCCESS;
    }

    return(returnCode);
}


OsStatus CpPhoneMediaInterface::getCapabilities(int connectionId,
                                                UtlString& rtpHostAddress,
                                                int& rtpAudioPort,
                                                int& rtcpAudioPort,
                                                int& rtpVideoPort,
                                                int& rtcpVideoPort,
                                                SdpCodecFactory& supportedCodecs,
                                                SdpSrtpParameters& srtpParams,
                                                int bandWidth,
                                                int& videoBandwidth,
                                                int& videoFramerate)
{
    OsStatus rc = OS_FAILED ;
    UtlString ignored ;    
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);
    rtpAudioPort = 0 ;
    rtcpAudioPort = 0 ;
    rtpVideoPort = 0 ;
    rtcpVideoPort = 0 ; 
    videoBandwidth = 0 ;

    if (pMediaConn)
    {
        if (    (pMediaConn->mContactType == AUTO) || 
                (pMediaConn->mContactType == NAT_MAPPED))
        {
            // Audio RTP
            if (pMediaConn->mpRtpAudioSocket)
            {
                // The "rtpHostAddress" is used for the rtp stream -- others 
                // are ignored.  They *SHOULD* be the same as the first.  
                // Possible exceptions: STUN worked for the first, but not the
                // others.  Not sure how to handle/recover from that case.
                if (pMediaConn->mContactType == RELAY)
                {
                    if (!pMediaConn->mpRtpAudioSocket->getRelayIp(&rtpHostAddress, &rtpAudioPort))
                    {
                        rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                        rtpHostAddress = mRtpReceiveHostAddress ;
                    }

                }
                else if (pMediaConn->mContactType == AUTO || pMediaConn->mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtpAudioSocket->getMappedIp(&rtpHostAddress, &rtpAudioPort))
                    {
                        rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                        rtpHostAddress = mRtpReceiveHostAddress ;
                    }
                }
                else if (pMediaConn->mContactType == LOCAL)
                {
                     rtpHostAddress = pMediaConn->mpRtpAudioSocket->getLocalIp();
                     rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
                     if (rtpAudioPort <= 0)
                     {
                         rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                         rtpHostAddress = mRtpReceiveHostAddress ;
                     }
                }
                else
                {
                  assert(0);
                }               
            }

            // Audio RTCP
            if (pMediaConn->mpRtcpAudioSocket)
            {
                if (pMediaConn->mContactType == RELAY)
                {
                    if (!pMediaConn->mpRtcpAudioSocket->getRelayIp(&ignored, &rtcpAudioPort))
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(ignored.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == AUTO || pMediaConn->mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtcpAudioSocket->getMappedIp(&ignored, &rtcpAudioPort))
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(ignored.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == LOCAL)
                {
                    ignored = pMediaConn->mpRtcpAudioSocket->getLocalIp();
                    rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
                    if (rtcpAudioPort <= 0)
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                }                
                else
                {
                    assert(0);
                }
            }
        }
        else
        {
            rtpHostAddress = mRtpReceiveHostAddress ;
            rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
            rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
        }

        // Codecs
        if (bandWidth != AUDIO_MICODEC_BW_DEFAULT)
        {
            setAudioCodecBandwidth(connectionId, bandWidth);
        }
        supportedCodecs = *(pMediaConn->mpCodecFactory);
        supportedCodecs.bindPayloadTypes();   

        // Setup SRTP parameters here
        memset((void*)&srtpParams, 0, sizeof(SdpSrtpParameters));

        rc = OS_SUCCESS ;
    }

    return rc ;
}


OsStatus CpPhoneMediaInterface::getCapabilitiesEx(int connectionId, 
                                                  int nMaxAddresses,
                                                  UtlString rtpHostAddresses[], 
                                                  int rtpAudioPorts[],
                                                  int rtcpAudioPorts[],
                                                  int rtpVideoPorts[],
                                                  int rtcpVideoPorts[],
                                                  int& nActualAddresses,
                                                  SdpCodecFactory& supportedCodecs,
                                                  SdpSrtpParameters& srtpParameters,
                                                  int bandWidth,
                                                  int& videoBandwidth,
                                                  int& videoFramerate)
{   
    OsStatus rc = OS_FAILED ;
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);
    nActualAddresses = 0 ;

    if (pMediaConn)
    {        
        switch (pMediaConn->mContactType)
        {
            case LOCAL:
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;
            case RELAY:
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;
            default:
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;


        }

        // Codecs
        if (bandWidth != AUDIO_MICODEC_BW_DEFAULT)
        {
            setAudioCodecBandwidth(connectionId, bandWidth);
        }
        supportedCodecs = *(pMediaConn->mpCodecFactory);
        supportedCodecs.bindPayloadTypes();

        memset((void*)&srtpParameters, 0, sizeof(SdpSrtpParameters));
        if (nActualAddresses > 0)
        {
            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}

CpPhoneMediaConnection* CpPhoneMediaInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId);
   return((CpPhoneMediaConnection*) mMediaConnections.find(&matchConnectionId));
}

OsStatus CpPhoneMediaInterface::setConnectionDestination(int connectionId,
                                                         const char* remoteRtpHostAddress,
                                                         int remoteAudioRtpPort,
                                                         int remoteAudioRtcpPort,
                                                         int remoteVideoRtpPort,
                                                         int remoteVideoRtcpPort)
{
    OsStatus returnCode = OS_NOT_FOUND;
    CpPhoneMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if(pMediaConnection && remoteRtpHostAddress && *remoteRtpHostAddress)
    {
        /*
         * Common Setup
         */
        pMediaConnection->mDestinationSet = TRUE;
        pMediaConnection->mRtpSendHostAddress = remoteRtpHostAddress ;

        /*
         * Audio Setup
         */
        pMediaConnection->mRtpAudioSendHostPort = remoteAudioRtpPort;
        pMediaConnection->mRtcpAudioSendHostPort = remoteAudioRtcpPort;

        if(pMediaConnection->mpRtpAudioSocket)
        {
            pMediaConnection->mpRtpAudioSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtpPort) ;
            pMediaConnection->mpRtpAudioSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtpPort) ;

            // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
            // breaks ICE)
            pMediaConnection->mpRtpAudioSocket->doConnect(remoteAudioRtpPort, remoteRtpHostAddress, TRUE);
            returnCode = OS_SUCCESS;
        }

        if(pMediaConnection->mpRtcpAudioSocket && (remoteAudioRtcpPort > 0))
        {
            pMediaConnection->mpRtcpAudioSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtcpPort) ;
            pMediaConnection->mpRtcpAudioSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtcpPort) ;

            // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
            // breaks ICE)
            pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtcpPort, remoteRtpHostAddress, TRUE);

            // TODO:: Enable RTCP (forget if it is enabled by default)
        }
        else
        {
            pMediaConnection->mRtcpAudioSendHostPort = 0 ;
        }

        /*
         * Video Setup
         */
#ifdef VIDEO
        if (pMediaConnection->mpRtpVideoSocket)
        {
            pMediaConnection->mRtpVideoSendHostPort = remoteVideoRtpPort ;                   
            pMediaConnection->mpRtpVideoSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtpPort) ;
            pMediaConnection->mpRtpVideoSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtpPort) ;

            // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
            // breaks ICE)
            mediaConnection->mpRtpVideoSocket->doConnect(remoteAudioRtpPort, remoteRtpHostAddress, TRUE);

            if(mediaConnection->mpRtcpVideoSocket && (remoteVideoRtcpPort > 0))
            {
                pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;               
                pMediaConnection->mpRtcpVideoSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtcpPort) ;
                pMediaConnection->mpRtcpVideoSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtcpPort) ;

                // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
                // breaks ICE)
                mediaConnection->mpRtcpVideoSocket->doConnect(remoteAudioRtcpPort, remoteRtpHostAddress, TRUE);
            }
            else
            {
                pMediaConnection->mRtcpVideoSendHostPort = 0 ;
            }
        }
        else
        {
            pMediaConnection->mRtpVideoSendHostPort = 0 ;
            pMediaConnection->mRtcpVideoSendHostPort = 0 ;
        }        
#endif
    }

   return(returnCode);
}

OsStatus CpPhoneMediaInterface::addAudioRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpAudioSocket != NULL))
        {
            mediaConnection->mbAlternateDestinations = true ;
            mediaConnection->mpRtpAudioSocket->addAlternateDestination(
                    candidateIp, candidatePort, iPriority) ;
            mediaConnection->mpRtpAudioSocket->readyDestination(candidateIp, 
                    candidatePort) ;

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus CpPhoneMediaInterface::addAudioRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpAudioSocket != NULL))
        {
            mediaConnection->mbAlternateDestinations = true ;
            mediaConnection->mpRtcpAudioSocket->addAlternateDestination(
                    candidateIp, candidatePort, iPriority) ;
            mediaConnection->mpRtcpAudioSocket->readyDestination(candidateIp, 
                    candidatePort) ;

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus CpPhoneMediaInterface::addVideoRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpVideoSocket != NULL))
        {
            mediaConnection->mbAlternateDestinations = true ;
            mediaConnection->mpRtpVideoSocket->addAlternateDestination(
                    candidateIp, candidatePort, iPriority) ;
            mediaConnection->mpRtpVideoSocket->readyDestination(candidateIp, 
                    candidatePort) ;

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }
#endif
    return returnCode ;    
}

OsStatus CpPhoneMediaInterface::addVideoRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpVideoSocket != NULL))
        {
            mediaConnection->mbAlternateDestinations = true ;
            mediaConnection->mpRtcpVideoSocket->addAlternateDestination(
                    candidateIp, candidatePort, iPriority) ;
            mediaConnection->mpRtcpVideoSocket->readyDestination(candidateIp, 
                    candidatePort) ;

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }
#endif
    return returnCode ;    
}


OsStatus CpPhoneMediaInterface::startRtpSend(int connectionId,
                                             int numCodecs,
                                             SdpCodec* sendCodecs[])
{
   // need to set default payload types in get capabilities

   int i;
   SdpCodec* primaryCodec = NULL;
   SdpCodec* dtmfCodec = NULL;
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   for (i=0; i<numCodecs; i++) {
      if (SdpCodec::SDP_CODEC_TONES == sendCodecs[i]->getValue()) {
         if (NULL == dtmfCodec) {
            dtmfCodec = sendCodecs[i];
         }
      } else if (NULL == primaryCodec) {
         primaryCodec = sendCodecs[i];
      }
   }
   if(mpFlowGraph && mediaConnection)
   {
#ifdef TEST_PRINT
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Sending RTP/RTCP codec: %d sockets: %p/%p descriptors: %d/%d\n",
           primaryCodec ? primaryCodec->getCodecType() : -2,
           (mediaConnection->mpRtpAudioSocket), (mediaConnection->mpRtcpAudioSocket),
           mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
#endif

        // If we haven't set a destination and we have set alternate destinations
        if (!mediaConnection->mDestinationSet && mediaConnection->mbAlternateDestinations)
        {
            applyAlternateDestinations(connectionId) ;
        }

       // Store the primary codec for cost calculations later
       if (mediaConnection->mpPrimaryCodec != NULL)
       {
           delete mediaConnection->mpPrimaryCodec ;
           mediaConnection->mpPrimaryCodec = NULL ;
       }
       if (primaryCodec != NULL)
       {
           mediaConnection->mpPrimaryCodec = new SdpCodec();
           *mediaConnection->mpPrimaryCodec = *primaryCodec ;
       }

       // Make sure we use the same payload types as the remote
       // side.  Its the friendly thing to do.
       if(mediaConnection->mpCodecFactory)
       {
           mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                            sendCodecs);
       }

       if(mediaConnection->mRtpAudioSending)
       {
           mpFlowGraph->stopSendRtp(connectionId);
       }

#ifdef TEST_PRINT
      UtlString dtmfCodecString;
      if(dtmfCodec) dtmfCodec->toString(dtmfCodecString);
      OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::startRtpSend %susing DTMF codec: %s\n",
         dtmfCodec ? "" : "NOT ",
         dtmfCodecString.data());
#endif

      if (!mediaConnection->mRtpSendHostAddress.isNull() && mediaConnection->mRtpSendHostAddress.compareTo("0.0.0.0"))
      {
         // This is the new interface for parallel codecs
         mpFlowGraph->startSendRtp(*(mediaConnection->mpRtpAudioSocket),
                                   *(mediaConnection->mpRtcpAudioSocket),
                                   connectionId,
                                   primaryCodec,
                                   dtmfCodec,
                                   NULL); // no redundant codecs

         mediaConnection->mRtpAudioSending = TRUE;
      }
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}


OsStatus CpPhoneMediaInterface::startRtpReceive(int connectionId,
                                                int numCodecs,
                                                SdpCodec* receiveCodecs[])
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mpFlowGraph && mediaConnection)
   {
#ifdef TEST_PRINT
      int i;

      OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Receiving RTP/RTCP, %d codec%s; sockets: %p/%p descriptors: %d/%d\n",
           numCodecs, ((1==numCodecs)?"":"s"),
           (mediaConnection->mpRtpAudioSocket),
           (mediaConnection->mpRtcpAudioSocket),
           mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
      for (i=0; i<numCodecs; i++) {
          osPrintf("   %d:  i:%d .. x:%d\n", i+1,
                   receiveCodecs[i]->getCodecType(),
                   receiveCodecs[i]->getCodecPayloadFormat());
      }
#endif

      // Make sure we use the same payload types as the remote
      // side.  It's the friendly thing to do.
      if(mediaConnection->mpCodecFactory)
      {
          mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                           receiveCodecs);
      }

      if(mediaConnection->mRtpAudioReceiving)
      {
         // This is not supposed to be necessary and may be
         // causing an audible glitch when codecs are changed
         mpFlowGraph->stopReceiveRtp(connectionId);
      }

      mpFlowGraph->startReceiveRtp(receiveCodecs, numCodecs,
           *(mediaConnection->mpRtpAudioSocket), *(mediaConnection->mpRtcpAudioSocket),
           connectionId);
      mediaConnection->mRtpAudioReceiving = TRUE;



      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus CpPhoneMediaInterface::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if(mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioSending)
   {
      mpFlowGraph->stopSendRtp(connectionId);
      mediaConnection->mRtpAudioSending = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus CpPhoneMediaInterface::stopRtpReceive(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if(mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioReceiving)
   {
      mpFlowGraph->stopReceiveRtp(connectionId);
      mediaConnection->mRtpAudioReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus CpPhoneMediaInterface::deleteConnection(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   UtlInt matchConnectionId(connectionId);
   mMediaConnections.remove(&matchConnectionId) ;

   returnCode = doDeleteConnection(mediaConnection);

   delete mediaConnection ;

   return(returnCode);
}

OsStatus CpPhoneMediaInterface::doDeleteConnection(CpPhoneMediaConnection* mediaConnection)
{
   OsStatus returnCode = OS_NOT_FOUND;
   if(mediaConnection)
   {
      returnCode = OS_SUCCESS;
      mediaConnection->mDestinationSet = FALSE;
#ifdef TEST_PRINT
      if (mediaConnection && mediaConnection->mpRtpAudioSocket && mediaConnection->mpRtcpAudioSocket)
    OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "stopping RTP/RTCP send & receive sockets %p/%p descriptors: %d/%d",
                mediaConnection->mpRtpAudioSocket,
                mediaConnection->mpRtcpAudioSocket,
                mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
                mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
     else if (!mediaConnection)
    OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "CpPhoneMediaInterface::doDeleteConnection mediaConnection is NULL!");
     else 
    OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "CpPhoneMediaInterface::doDeleteConnection NULL socket: mpRtpAudioSocket=0x%08x, mpRtpAudioSocket=0x%08x",
                mediaConnection->mpRtpAudioSocket,
                                mediaConnection->mpRtcpAudioSocket);
#endif

      returnCode = stopRtpSend(mediaConnection->getValue());
      returnCode = stopRtpReceive(mediaConnection->getValue());

      if(mediaConnection->getValue() >= 0)
      {
          mpFlowGraph->deleteConnection(mediaConnection->getValue());
          mediaConnection->setValue(-1);
          mpFlowGraph->synchronize();
      }

      mpFactoryImpl->releaseRtpPort(mediaConnection->mRtpAudioReceivePort) ;

      if(mediaConnection->mpRtpAudioSocket)
      {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "deleting RTP socket: %p descriptor: %d",
                mediaConnection->mpRtpAudioSocket,
                mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
#endif

         delete mediaConnection->mpRtpAudioSocket;
         mediaConnection->mpRtpAudioSocket = NULL;
      }
      if(mediaConnection->mpRtcpAudioSocket)
      {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "deleting RTCP socket: %p descriptor: %d",
                mediaConnection->mpRtcpAudioSocket,
                mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
#endif

         delete mediaConnection->mpRtcpAudioSocket;
         mediaConnection->mpRtcpAudioSocket = NULL;
      }


   }
   return(returnCode);
}


OsStatus CpPhoneMediaInterface::playAudio(const char* url,
                                          UtlBoolean repeat,
                                          UtlBoolean local,
                                          UtlBoolean remote,
                                          UtlBoolean mixWithMic,
                                          int downScaling)
{
    OsStatus returnCode = OS_NOT_FOUND;
    UtlString urlString;
    if(url) urlString.append(url);
    int fileIndex = urlString.index("file://");
    if(fileIndex == 0) urlString.remove(0, 6);

    if(mpFlowGraph && !urlString.isNull())
    {

        // Start playing the audio file
        returnCode = mpFlowGraph->playFile(urlString.data(),
            repeat,
            remote ? MpCallFlowGraph::TONE_TO_NET :
                MpCallFlowGraph::TONE_TO_SPKR);
    }

    if(returnCode != OS_SUCCESS)
    {
        osPrintf("Cannot play audio file: %s\n", urlString.data());
    }

    return(returnCode);
}

OsStatus CpPhoneMediaInterface::playBuffer(char* buf,
                                           unsigned long bufSize,
                                           int type, 
                                           UtlBoolean repeat,
                                           UtlBoolean local,
                                           UtlBoolean remote, 
                                           OsNotification* pEvent,
                                           UtlBoolean mixWithMic,
                                           int downScaling)
{
    OsStatus returnCode = OS_NOT_FOUND;
    if(mpFlowGraph && buf)
    {

        // Start playing the audio file
        returnCode = mpFlowGraph->playBuffer(buf, bufSize, type, 
               repeat,
               remote ? MpCallFlowGraph::TONE_TO_NET : MpCallFlowGraph::TONE_TO_SPKR,
               NULL);
    }

    if(returnCode != OS_SUCCESS)
    {
        osPrintf("Cannot play audio buffer: %10p\n", buf);
    }

    if (pEvent)
    {
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        if(OS_ALREADY_SIGNALED == pEvent->signal(true))
        {
            eventMgr->release((OsProtectedEvent*) pEvent);
        }
    }

    return(returnCode);
}


OsStatus CpPhoneMediaInterface::stopAudio()
{
    OsStatus returnCode = OS_NOT_FOUND;
    if(mpFlowGraph)
    {
        mpFlowGraph->stopFile(TRUE);
        returnCode = OS_SUCCESS;
    }
    return(returnCode);
}


OsStatus CpPhoneMediaInterface::playChannelAudio(int connectionId,
                                                 const char* url,
                                                 UtlBoolean repeat,
                                                 UtlBoolean local,
                                                 UtlBoolean remote,
                                                 UtlBoolean mixWithMic,
                                                 int downScaling) 
{
    return playAudio(url, repeat, local, remote, mixWithMic, downScaling) ;
}


OsStatus CpPhoneMediaInterface::stopChannelAudio(int connectionId) 
{
    return stopAudio() ;
}


OsStatus CpPhoneMediaInterface::recordChannelAudio(int connectionId,
                                                   const char* szFile) 
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpPhoneMediaInterface::stopRecordChannelAudio(int connectionId) 
{
    return OS_NOT_SUPPORTED ;
}



OsStatus CpPhoneMediaInterface::createPlayer(MpStreamPlayer** ppPlayer, 
                                             const char* szStream, 
                                             int flags, 
                                             OsMsgQ *pMsgQ, 
                                             const char* szTarget)
{
   OsStatus returnCode = OS_NOT_FOUND;

   if ((pMsgQ == NULL) && (mpFlowGraph != NULL))
      pMsgQ = mpFlowGraph->getMsgQ() ;


   if(pMsgQ != NULL)
   {
      Url url(szStream) ;

      *ppPlayer = new MpStreamPlayer(pMsgQ, url, flags, szTarget) ;

      returnCode = OS_SUCCESS;
   }

   return(returnCode);
}


OsStatus CpPhoneMediaInterface::destroyPlayer(MpStreamPlayer* pPlayer)
{
   if (pPlayer != NULL)
   {
      pPlayer->destroy() ;
   }

   return OS_SUCCESS;   
}


OsStatus CpPhoneMediaInterface::createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer, 
                                                     OsMsgQ *pMsgQ, 
                                                     const char* szTarget)
{
   OsStatus returnCode = OS_NOT_FOUND;

   if ((pMsgQ == NULL) && (mpFlowGraph != NULL))
      pMsgQ = mpFlowGraph->getMsgQ() ;

   if(pMsgQ != NULL)   
   {
      *ppPlayer = new MpStreamPlaylistPlayer(pMsgQ, szTarget) ;
      returnCode = OS_SUCCESS;
   }

   return(returnCode);
}


OsStatus CpPhoneMediaInterface::destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
{
   if (pPlayer != NULL)
   {
      pPlayer->destroy() ;
   }

   return OS_SUCCESS;   
}


OsStatus CpPhoneMediaInterface::createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                                  OsMsgQ *pMsgQ, 
                                                  const char* szTarget)
{
   OsStatus returnCode = OS_NOT_FOUND;
   
   if ((pMsgQ == NULL) && (mpFlowGraph != NULL))
      pMsgQ = mpFlowGraph->getMsgQ() ;

   if(pMsgQ != NULL)   
   {
      *ppPlayer = new MpStreamQueuePlayer(pMsgQ, szTarget) ;
      returnCode = OS_SUCCESS;
   }

   return(returnCode);
}


OsStatus CpPhoneMediaInterface::destroyQueuePlayer(MpStreamQueuePlayer* pPlayer)
{
   if (pPlayer != NULL)
   {
      pPlayer->destroy() ;
   }

   return OS_SUCCESS;   
}


OsStatus CpPhoneMediaInterface::startTone(int toneId,
                                          UtlBoolean local,
                                          UtlBoolean remote)
{
   OsStatus returnCode = OS_SUCCESS;
   int toneDestination = 0 ;

   if(mpFlowGraph)
   {
      if (local)
      {
         toneDestination |= MpCallFlowGraph::TONE_TO_SPKR;
      }                  
      
      if(remote)
      {
         toneDestination |= MpCallFlowGraph::TONE_TO_NET;
      }
     
      mpFlowGraph->startTone(toneId, toneDestination);

      // Make sure the DTMF tone is on the minimum length
      OsTask::delay(MINIMUM_DTMF_LENGTH);
   } 

   return(returnCode);
}

OsStatus CpPhoneMediaInterface::stopTone()
{
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      mpFlowGraph->stopTone();
   }

   return(returnCode);
}

OsStatus CpPhoneMediaInterface::startChannelTone(int connectionId, int toneId, UtlBoolean local, UtlBoolean remote) 
{
    return startTone(toneId, local, remote) ;
}

OsStatus CpPhoneMediaInterface::stopChannelTone(int connectionId)
{
    return stopTone() ;
}


OsStatus CpPhoneMediaInterface::giveFocus()
{
    if(mpFlowGraph)
    {
        // There should probably be a lock here
        // Set the flow graph to have the focus
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
        mediaTask->setFocus(mpFlowGraph);
        // osPrintf("Setting focus for flow graph\n");
   }

   return OS_SUCCESS ;
}

OsStatus CpPhoneMediaInterface::defocus()
{
    if(mpFlowGraph)
    {
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);

        // There should probably be a lock here
        // take focus away from the flow graph if it is focus
        if(mpFlowGraph == (MpCallFlowGraph*) mediaTask->getFocus())
        {
            mediaTask->setFocus(NULL);
            // osPrintf("Setting NULL focus for flow graph\n");
        }
    }
    return OS_SUCCESS ;
}


// Limits the available codecs to only those within the designated limit.
void CpPhoneMediaInterface::setCodecCPULimit(int iLimit)
{
   mSupportedCodecs.setCodecCPULimit(iLimit) ;

   CpPhoneMediaConnection* mediaConnection = NULL;
   UtlDListIterator connectionIterator(mMediaConnections);
   while ((mediaConnection = (CpPhoneMediaConnection*) connectionIterator()))
   {
      mediaConnection->mpCodecFactory->setCodecCPULimit(iLimit) ;
   }
}

OsStatus CpPhoneMediaInterface::stopRecording()
{
   OsStatus ret = OS_UNSPECIFIED;
   if (mpFlowGraph)
   {
#ifdef TEST_PRINT
     osPrintf("CpPhoneMediaInterface::stopRecording() : calling flowgraph::stoprecorders\n");
     OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::stopRecording() : calling flowgraph::stoprecorders");
#endif
     mpFlowGraph->closeRecorders();
     ret = OS_SUCCESS;
   }
   
   return ret;
}


OsStatus CpPhoneMediaInterface::ezRecord(int ms, 
                                         int silenceLength, 
                                         const char* fileName, 
                                         double& duration,
                                         int& dtmfterm,
                                         OsProtectedEvent* ev)
{
   OsStatus ret = OS_UNSPECIFIED;
   if (mpFlowGraph && fileName)
   {
     if (!ev) // default behavior
        ret = mpFlowGraph->ezRecord(ms, 
                                 silenceLength, 
                                 fileName, 
                                 duration, 
                                 dtmfterm, 
                                 MprRecorder::WAV_PCM_16);
     else
        ret = mpFlowGraph->mediaRecord(ms, 
                                 silenceLength, 
                                 fileName, 
                                 duration, 
                                 dtmfterm, 
                                 MprRecorder::WAV_PCM_16,
                                 ev);
   }
   
   return ret;
}

void CpPhoneMediaInterface::addToneListener(OsNotification *pListener, int connectionId)
{
    if ((mpFlowGraph) && (connectionId >= 0))
    {
        mpFlowGraph->addToneListener(pListener, (MpConnectionID) connectionId);
    }
}

void CpPhoneMediaInterface::removeToneListener(int connectionId)
{
    if ((mpFlowGraph) && (connectionId >= 0))
    {
        mpFlowGraph->removeToneListener((MpConnectionID) connectionId) ;
    }
}

void CpPhoneMediaInterface::setContactType(int connectionId, CONTACT_TYPE eType, CONTACT_ID contactId) 
{
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        pMediaConn->mContactType = eType ;
    }
}

OsStatus CpPhoneMediaInterface::setAudioCodecBandwidth(int connectionId, int bandWidth) 
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpPhoneMediaInterface::rebuildCodecFactory(int connectionId, 
                                                    int audioBandwidth, 
                                                    int videoBandwidth, 
                                                    UtlString& videoCodec)
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpPhoneMediaInterface::setConnectionBitrate(int connectionId, int bitrate) 
{
    return OS_NOT_SUPPORTED ;
}


OsStatus CpPhoneMediaInterface::setConnectionFramerate(int connectionId, int framerate) 
{
    return OS_NOT_SUPPORTED ;
}


OsStatus CpPhoneMediaInterface::setSecurityAttributes(const void* security) 
{
    return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

void CpPhoneMediaInterface::setPremiumSound(UtlBoolean enabled)
{
    if(mpFlowGraph)
    {
        if(enabled)
        {
            mpFlowGraph->enablePremiumSound();
        }
        else
        {
            mpFlowGraph->disablePremiumSound();
        }
    }
}


OsStatus CpPhoneMediaInterface::setVideoQuality(int quality)
{
   return OS_SUCCESS;
}

OsStatus CpPhoneMediaInterface::setVideoParameters(int bitRate, int frameRate)
{
   return OS_SUCCESS;
}

// Calculate the current cost for our sending/receiving codecs
int CpPhoneMediaInterface::getCodecCPUCost()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   

   if (mMediaConnections.entries() > 0)
   {      
      CpPhoneMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpPhoneMediaConnection*) connectionIterator()))
      {
         // If the codec is null, assume LOW.
         if (mediaConnection->mpPrimaryCodec != NULL)
         {
            int iCodecCost = mediaConnection->mpPrimaryCodec->getCPUCost();
            if (iCodecCost > iCost)
               iCost = iCodecCost;
         }

         // Optimization: If we have already hit the highest, kick out.
         if (iCost == SdpCodec::SDP_CODEC_CPU_HIGH)
            break ;
      }
   }
   
   return iCost ;
}


// Calculate the worst case cost for our sending/receiving codecs
int CpPhoneMediaInterface::getCodecCPULimit()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   
   int         iCodecs = 0 ;
   SdpCodec**  codecs ;


   //
   // If have connections; report what we have offered
   //
   if (mMediaConnections.entries() > 0)
   {      
      CpPhoneMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpPhoneMediaConnection*) connectionIterator()))
      {
         mediaConnection->mpCodecFactory->getCodecs(iCodecs, codecs) ;      
         for(int i = 0; i < iCodecs; i++)
         {
            // If the cost is greater than what we have, then make that the cost.
            int iCodecCost = codecs[i]->getCPUCost();
            if (iCodecCost > iCost)
               iCost = iCodecCost;

             delete codecs[i];
         }
         delete[] codecs;

         // Optimization: If we have already hit the highest, kick out.
         if (iCost == SdpCodec::SDP_CODEC_CPU_HIGH)
            break ;
      }
   }
   //
   // If no connections; report what we plan on using
   //
   else
   {
      mSupportedCodecs.getCodecs(iCodecs, codecs) ;  
      for(int i = 0; i < iCodecs; i++)
      {
         // If the cost is greater than what we have, then make that the cost.
         int iCodecCost = codecs[i]->getCPUCost();
         if (iCodecCost > iCost)
            iCost = iCodecCost;

          delete codecs[i];
      }
      delete[] codecs;
   }

   return iCost ;
}

// Returns the flowgraph's message queue
   OsMsgQ* CpPhoneMediaInterface::getMsgQ()
{
   return mpFlowGraph->getMsgQ() ;
}

OsStatus CpPhoneMediaInterface::getPrimaryCodec(int connectionId, 
                                                UtlString& audioCodec,
                                                UtlString& videoCodec,
                                                int* audioPayloadType,
                                                int* videoPayloadType,
                                                bool& isEncrypted)
{
    isEncrypted = FALSE ;
    *audioPayloadType = 0;
    *videoPayloadType = 0;
    return OS_SUCCESS;   
}

OsStatus CpPhoneMediaInterface::getVideoQuality(int& quality)
{
   quality = 0;
   return OS_SUCCESS;
}

OsStatus CpPhoneMediaInterface::getVideoBitRate(int& bitRate)
{
   bitRate = 0;
   return OS_SUCCESS;
}


OsStatus CpPhoneMediaInterface::getVideoFrameRate(int& frameRate)
{
   frameRate = 0;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */
UtlBoolean CpPhoneMediaInterface::isSendingRtpAudio(int connectionId)
{
   UtlBoolean sending = FALSE;
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
       sending = mediaConnection->mRtpAudioSending;
   }
   else
   {
       osPrintf("CpPhoneMediaInterface::isSendingRtp invalid connectionId: %d\n",
          connectionId);
   }

   return(sending);
}

UtlBoolean CpPhoneMediaInterface::isReceivingRtpAudio(int connectionId)
{
   UtlBoolean receiving = FALSE;
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
      receiving = mediaConnection->mRtpAudioReceiving;
   }
   else
   {
       osPrintf("CpPhoneMediaInterface::isReceivingRtp invalid connectionId: %d\n",
          connectionId);
   }
   return(receiving);
}

UtlBoolean CpPhoneMediaInterface::isSendingRtpVideo(int connectionId)
{
   UtlBoolean sending = FALSE;

   return(sending);
}

UtlBoolean CpPhoneMediaInterface::isReceivingRtpVideo(int connectionId)
{
   UtlBoolean receiving = FALSE;

   return(receiving);
}


UtlBoolean CpPhoneMediaInterface::isDestinationSet(int connectionId)
{
    UtlBoolean isSet = FALSE;
    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

    if(mediaConnection)
    {
        isSet = mediaConnection->mDestinationSet;
    }
    else
    {
       osPrintf("CpPhoneMediaInterface::isDestinationSet invalid connectionId: %d\n",
          connectionId);
    }
    return(isSet);
}

UtlBoolean CpPhoneMediaInterface::canAddParty() 
{
    return (mMediaConnections.entries() < 4) ;
}


bool CpPhoneMediaInterface::isVideoInitialized(int connectionId)
{
    return false ;
}

bool CpPhoneMediaInterface::isAudioInitialized(int connectionId) 
{
    return true ;
}

bool CpPhoneMediaInterface::isAudioAvailable() 
{
    return true ;
}

OsStatus CpPhoneMediaInterface::setVideoWindowDisplay(const void* hWnd)
{
    return OS_NOT_YET_IMPLEMENTED;
    
}
const void* CpPhoneMediaInterface::getVideoWindowDisplay()
{
    return NULL;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */


bool CpPhoneMediaInterface::getLocalAddresses(int connectionId,
                                              UtlString& hostIp,
                                              int& rtpAudioPort,
                                              int& rtcpAudioPort,
                                              int& rtpVideoPort,
                                              int& rtcpVideoPort)
{
    bool bRC = false ;
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);    

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            hostIp = pMediaConn->mpRtpAudioSocket->getLocalIp();
            rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
            if (rtpAudioPort > 0)
            {
                bRC = true ;
            }

            // Audio rtcp port (optional) 
            if (pMediaConn->mpRtcpAudioSocket && bRC)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
            }        
        }

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            rtpVideoPort = pMediaConn->mpRtpVideoSocket->getLocalHostPort();

            // Video rtcp port (optional)
            if (pMediaConn->mpRtcpVideoSocket)
            {
                rtcpVideoPort = pMediaConn->mpRtcpVideoSocket->getLocalHostPort();
            }
        }
#endif
    }

    return bRC ;
}

bool CpPhoneMediaInterface::getNatedAddresses(int connectionId,
                                              UtlString& hostIp,
                                              int& rtpAudioPort,
                                              int& rtcpAudioPort,
                                              int& rtpVideoPort,
                                              int& rtcpVideoPort)
{
    bool bRC = false ;
    UtlString host ;
    int port ;
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (pMediaConn->mpRtpAudioSocket->getMappedIp(&host, &port))
            {
                if (port > 0)
                {
                    hostIp = host ;
                    rtpAudioPort = port ;

                    bRC = true ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocket && bRC)
                {
                    if (pMediaConn->mpRtcpAudioSocket->getMappedIp(&host, &port))
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

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            if (pMediaConn->mpRtpVideoSocket->getMappedIp(&host, &port))
            {
                rtpVideoPort = port ;
                if (host.compareTo(hostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Stun host IP mismatches for rtp/video (%s != %s)", 
                            hostIp.data(), host.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpAudioSocket)
                {
                    if (pMediaConn->mpRtcpVideoSocket->getMappedIp(&host, &port))
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
#endif
    }

    return bRC ;
}

bool CpPhoneMediaInterface::getRelayAddresses(int connectionId,
                                              UtlString& hostIp,
                                              int& rtpAudioPort,
                                              int& rtcpAudioPort,
                                              int& rtpVideoPort,
                                              int& rtcpVideoPort)
{
    bool bRC = false ;
    UtlString host ;
    int port ;
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (pMediaConn->mpRtpAudioSocket->getRelayIp(&host, &port))
            {
                if (port > 0)
                {
                    hostIp = host ;
                    rtpAudioPort = port ;

                    bRC = true ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocket && bRC)
                {
                    if (pMediaConn->mpRtcpAudioSocket->getRelayIp(&host, &port))
                    {
                        rtcpAudioPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/audio (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }
        }

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            if (pMediaConn->mpRtpVideoSocket->getRelayIp(&host, &port))
            {
                rtpVideoPort = port ;
                if (host.compareTo(hostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Turn host IP mismatches for rtp/video (%s != %s)", 
                            hostIp.data(), host.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpAudioSocket)
                {
                    if (pMediaConn->mpRtcpVideoSocket->getRelayIp(&host, &port))
                    {
                        rtcpVideoPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/video (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }            
        }
#endif
    }

    return bRC ;
}

OsStatus CpPhoneMediaInterface::addLocalContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    OsStatus rc = OS_FAILED ;

    // Local Addresses
    if (    (nActualAddresses < nMaxAddresses) && 
            getLocalAddresses(connectionId, hostIp, rtpAudioPort, 
            rtcpAudioPort, rtpVideoPort, rtcpVideoPort) )
    {
        bool bDuplicate = false ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = true ;
                break ;
            }
        }

        if (!bDuplicate)
        {
            rtpHostAddresses[nActualAddresses] = hostIp ;
            rtpAudioPorts[nActualAddresses] = rtpAudioPort ;
            rtcpAudioPorts[nActualAddresses] = rtcpAudioPort ;
            rtpVideoPorts[nActualAddresses] = rtpVideoPort ;
            rtcpVideoPorts[nActualAddresses] = rtcpVideoPort ;
            nActualAddresses++ ;

            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}


OsStatus CpPhoneMediaInterface::addNatedContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    OsStatus rc = OS_FAILED ;

    // NAT Addresses
    if (    (nActualAddresses < nMaxAddresses) && 
            getNatedAddresses(connectionId, hostIp, rtpAudioPort, 
            rtcpAudioPort, rtpVideoPort, rtcpVideoPort) )
    {
        bool bDuplicate = false ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = true ;
                break ;
            }
        }

        if (!bDuplicate)
        {
            rtpHostAddresses[nActualAddresses] = hostIp ;
            rtpAudioPorts[nActualAddresses] = rtpAudioPort ;
            rtcpAudioPorts[nActualAddresses] = rtcpAudioPort ;
            rtpVideoPorts[nActualAddresses] = rtpVideoPort ;
            rtcpVideoPorts[nActualAddresses] = rtcpVideoPort ;
            nActualAddresses++ ;

            rc = OS_SUCCESS ;
        }
    }
    return rc ;
}


OsStatus CpPhoneMediaInterface::addRelayContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    OsStatus rc = OS_FAILED ;

    // Relay Addresses
    if (    (nActualAddresses < nMaxAddresses) && 
            getRelayAddresses(connectionId, hostIp, rtpAudioPort, 
            rtcpAudioPort, rtpVideoPort, rtcpVideoPort) )
    {
        bool bDuplicate = false ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = true ;
                break ;
            }
        }

        if (!bDuplicate)
        {
            rtpHostAddresses[nActualAddresses] = hostIp ;
            rtpAudioPorts[nActualAddresses] = rtpAudioPort ;
            rtcpAudioPorts[nActualAddresses] = rtcpAudioPort ;
            rtpVideoPorts[nActualAddresses] = rtpVideoPort ;
            rtcpVideoPorts[nActualAddresses] = rtcpVideoPort ;
            nActualAddresses++ ;

            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}


void CpPhoneMediaInterface::applyAlternateDestinations(int connectionId) 
{
    UtlString destAddress ;
    int       destPort ;
    int       rc ;
    CpPhoneMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if (pMediaConnection)
    {
        assert(!pMediaConnection->mDestinationSet) ;
        pMediaConnection->mDestinationSet = true ;

        pMediaConnection->mRtpSendHostAddress.remove(0) ;
        pMediaConnection->mRtpAudioSendHostPort = 0 ;
        pMediaConnection->mRtcpAudioSendHostPort = 0 ;
#ifdef VIDEO
        pMediaConnection->mRtpVideoSendHostPort = 0 ;
        pMediaConnection->mRtcpVideoSendHostPort = 0 ;
#endif

        // TODO: We should REALLY store a different host for each connection -- they could
        //       differ when using TURN (could get forwarded to another turn server)
        //       For now, we store the rtp host


        // Connect RTP Audio Socket
        if (pMediaConnection->mpRtpAudioSocket)
        {
            if (pMediaConnection->mpRtpAudioSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtpAudioSocket->applyDestinationAddress(destAddress, destPort) ;                                
                pMediaConnection->mRtpSendHostAddress = destAddress;
                pMediaConnection->mRtpAudioSendHostPort = destPort;

                // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
                // breaks ICE)
                pMediaConnection->mpRtpAudioSocket->doConnect(destPort, destAddress, TRUE);
            }
        }

        // Connect RTCP Audio Socket
        if (pMediaConnection->mpRtcpAudioSocket)
        {
            if (pMediaConnection->mpRtcpAudioSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtcpAudioSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpAudioSendHostPort = destPort;                

                // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
                // breaks ICE)
                pMediaConnection->mpRtcpAudioSocket->doConnect(destPort, destAddress, TRUE);
            }            
        }

        // TODO:: Enable/Disable RTCP

#ifdef VIDEO
        // Connect RTP Video Socket
        if (pMediaConnection->mpRtpVideoSocket)
        {
            if (pMediaConnection->mpRtpVideoSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtpVideoSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtpVideoSendHostPort = destPort;

                // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
                // breaks ICE)
                pMediaConnection->mpRtpVideoSocket->doConnect(destPort, destAddress, TRUE);
            }            
        }

        // Connect RTCP Video Socket
        if (pMediaConnection->mpRtcpVideoSocket)
        {
            if (pMediaConnection->mpRtcpVideoSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtcpVideoSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpVideoSendHostPort = destPort;

                // TODO:: Do not call doConnect -- this filters out packets from other sources (e.g. 
                // breaks ICE)
                pMediaConnection->mRtcpVideoSendHostPort->doConnect(destPort, destAddress, TRUE);
            }            
        }

        // TODO:: Enable/Disable RTCP
#endif
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

