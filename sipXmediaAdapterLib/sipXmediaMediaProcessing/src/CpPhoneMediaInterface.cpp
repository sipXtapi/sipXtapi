// 
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
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

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashMap.h>
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

#ifdef SIPX_VIDEO // [
#include <mp/video/MpVideoCallFlowGraph.h>
#endif // SIPX_VIDEO ]

#include <sdp/SdpCodec.h>

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
    CpPhoneMediaConnection(int connectionId = -1)
    : UtlInt(connectionId)
    , mRtpSendHostAddress()
    , mDestinationSet(FALSE)
    , mpRtpAudioSocket(NULL)
    , mpRtcpAudioSocket(NULL)
    , mRtpAudioSendHostPort(0)
    , mRtcpAudioSendHostPort(0)
    , mRtpAudioReceivePort(0)
    , mRtcpAudioReceivePort(0)
    , mRtpAudioSending(FALSE)
    , mRtpAudioReceiving(FALSE)
    , mpAudioCodec(NULL)
#ifdef SIPX_VIDEO // [
    , mpRtpVideoSocket(NULL)
    , mpRtcpVideoSocket(NULL)
    , mRtpVideoSendHostPort(0)
    , mRtcpVideoSendHostPort(0)
    , mRtpVideoReceivePort(0)
    , mRtcpVideoReceivePort(0)
    , mRtpVideoSending(FALSE)
    , mRtpVideoReceiving(FALSE)
    , mpVideoCodec(NULL)
#endif // SIPX_VIDEO ]
    , mpCodecFactory(NULL)
    , mContactType(CONTACT_AUTO)
    , mbAlternateDestinations(FALSE)
    {
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

#ifdef SIPX_VIDEO // [
        if(mpRtpVideoSocket)
        {

#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpPhoneMediaConnection deleting RTP socket: %p descriptor: %d",
                mpRtpVideoSocket, mpRtpVideoSocket->getSocketDescriptor());
#endif
            delete mpRtpVideoSocket;
            mpRtpVideoSocket = NULL;
        }

        if(mpRtcpVideoSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpPhoneMediaConnection deleting RTCP socket: %p descriptor: %d",
                mpRtcpVideoSocket, mpRtcpVideoSocket->getSocketDescriptor());
#endif
            delete mpRtcpVideoSocket;
            mpRtcpVideoSocket = NULL;
        }
#endif // SIPX_VIDEO ]

        if(mpCodecFactory)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpPhoneMediaConnection deleting SdpCodecFactory %p",
                mpCodecFactory);
            delete mpCodecFactory;
            mpCodecFactory = NULL;
        }

        if (mpAudioCodec)
        {
            delete mpAudioCodec;
            mpAudioCodec = NULL; 
        }              

        mConnectionProperties.destroyAll();
    }

    UtlString mRtpSendHostAddress;
    UtlBoolean mDestinationSet;

    OsNatDatagramSocket* mpRtpAudioSocket;
    OsNatDatagramSocket* mpRtcpAudioSocket;
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    UtlBoolean mRtpAudioSending;
    UtlBoolean mRtpAudioReceiving;
    SdpCodec* mpAudioCodec;

#ifdef SIPX_VIDEO // [
    OsNatDatagramSocket* mpRtpVideoSocket;
    OsNatDatagramSocket* mpRtcpVideoSocket;
    int mRtpVideoSendHostPort;
    int mRtcpVideoSendHostPort;
    int mRtpVideoReceivePort;
    int mRtcpVideoReceivePort;
    UtlBoolean mRtpVideoSending;
    UtlBoolean mRtpVideoReceiving;
    SdpCodec* mpVideoCodec;
#endif // SIPX_VIDEO ]


    SdpCodecFactory* mpCodecFactory;
    SIPX_CONTACT_TYPE mContactType ;
    UtlString mLocalAddress ;
    UtlBoolean mbAlternateDestinations ;
    UtlHashMap mConnectionProperties;
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
                                             UtlBoolean bEnableICE,
                                             MpCaptureDeviceBase* pCaptureDevice,
                                             const MpVideoStreamParams* pVideoCaptureParams)
: CpMediaInterface(pFactoryImpl)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::CpPhoneMediaInterface creating a new CpMediaInterface %p",
                 this);

   mpFlowGraph = new MpCallFlowGraph(locale);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::CpPhoneMediaInterface creating a new MpCallFlowGraph %p",
                 mpFlowGraph);

#ifdef SIPX_VIDEO // [
   mpVideoFlowGraph = new MpVideoCallFlowGraph(pCaptureDevice, pVideoCaptureParams);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::CpPhoneMediaInterface creating a new MpVideoCallFlowGraph %p",
                 mpVideoFlowGraph);
#endif // SIPX_VIDEO ]
   
   mStunServer = szStunServer ;
   mStunPort = iStunPort ;
   mStunRefreshPeriodSecs = iStunKeepAlivePeriodSecs ;
   mTurnServer = szTurnServer ;
   mTurnPort = iTurnPort ;
   mTurnRefreshPeriodSecs = iTurnKeepAlivePeriodSecs ;
   mTurnUsername = szTurnUsername ;
   mTurnPassword = szTurnPassword ;
   mbEnableICE = bEnableICE ;

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
#ifdef CODEC_VALIDATION
       /* 
        * 2006-10-12/Bob: These checks are now disabled -- I don't believe
        *     the CpPhoneMediaInterface should need to know about ALL the 
        *     possible codecs types.  Our goal is have dynamic pluggable 
        *     codec support.  The getCapabilities function should validate
        *     the codec list -- not this class.
        */
       UtlString codecList("");
       // Test plausibility of passed in codecs, don't add any that media system
       // does not support - the media system knows best.
       for (int i=0; i<numCodecs && sdpCodecArray[i]; i++)
       {
          SdpCodec::SdpCodecTypes cType = sdpCodecArray[i]->getCodecType();
          
          switch (cType)
          {
          case SdpCodec::SDP_CODEC_TONES:
             codecList.append("telephone-event ");
             break;
          case SdpCodec::SDP_CODEC_GIPS_PCMU:
             codecList.append("pcmu ");
             break;
          case SdpCodec::SDP_CODEC_GIPS_PCMA:
             codecList.append("pcma ");
             break;
#ifdef HAVE_GIPS
          case SdpCodec::SDP_CODEC_GIPS_IPCMU:
             codecList.append("eg711u ");
             break;
          case SdpCodec::SDP_CODEC_GIPS_IPCMA:
             codecList.append("eg711a ");
             break;
#endif /* HAVE_GIPS */
          default:
              OsSysLog::add(FAC_CP, PRI_WARNING, 
                            "CpPhoneMediaInterface::CpPhoneMediaInterface dropping codec type %d as not supported",
                            cType);
              break;
          }  
       }
       mSupportedCodecs.buildSdpCodecFactory(codecList);
       
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "CpPhoneMediaInterface::CpPhoneMediaInterface creating codec factory with %s",
                     codecList.data());

       // Assign any unset payload types
       mSupportedCodecs.bindPayloadTypes();
#else
       mSupportedCodecs.addCodecs(numCodecs, sdpCodecArray);

       // Assign any unset payload types
       mSupportedCodecs.bindPayloadTypes();
#endif
   }
   else
   {
       // Temp hard code codecs
       //SdpCodec mapCodecs1(SdpCodec::SDP_CODEC_PCMU, SdpCodec::SDP_CODEC_PCMU);
       //mSupportedCodecs.addCodec(mapCodecs1);
       //SdpCodec mapCodecs2(SdpCodec::SDP_CODEC_PCMA, SdpCodec::SDP_CODEC_PCMA);
       //mSupportedCodecs.addCodec(mapCodecs2);
       //mapCodecs[2] = new SdpCodec(SdpCodec::SDP_CODEC_L16_MONO);
       UtlString codecs = 
#ifdef HAVE_SPEEX // [
                          "SPEEX SPEEX_5 SPEEX_15 SPEEX_24 "
#endif // HAVE_SPEEX ]
#ifdef HAVE_GSM // [
                          "GSM "
#endif // HAVE_GSM ]
#ifdef SIPX_VIDEO // [
                          "H264-CIF H264-QCIF H264-SQCIF H264-QVGA "
#endif // SIPX_VIDEO ]
                          "PCMU PCMA TELEPHONE-EVENT";
       OsSysLog::add(FAC_CP, PRI_WARNING, "CpPhoneMediaInterface::CpPhoneMediaInterface hard-coded codec factory %s ...",
                     codecs.data());
       mSupportedCodecs.buildSdpCodecFactory(codecs);
   }

   mExpeditedIpTos = expeditedIpTos;
}


// Destructor
CpPhoneMediaInterface::~CpPhoneMediaInterface()
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::~CpPhoneMediaInterface deleting the CpMediaInterface %p",
                 this);

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

        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::~CpPhoneMediaInterface deleting the MpCallFlowGraph %p",
                      mpFlowGraph);
        delete mpFlowGraph;
        mpFlowGraph = NULL;
    }

#ifdef SIPX_VIDEO // [
    if (mpVideoFlowGraph)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::~CpPhoneMediaInterface deleting the MpVideoCallFlowGraph %p",
                      mpVideoFlowGraph);
        delete mpVideoFlowGraph;
        mpVideoFlowGraph = NULL;
    }
#endif // SIPX_VIDEO ]

    // Delete the properties and their values
    mInterfaceProperties.destroyAll();
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
                                                 ISocketEvent* pIdleEvent,
                                                 IMediaEventListener* pMediaEventListener,
                                                 const RtpTransportOptions rtpTransportOptions)
{
   CpPhoneMediaConnection* mediaConnection=NULL;

   connectionId = mpFlowGraph->createConnection();

   mediaConnection = new CpPhoneMediaConnection();
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::createConnection creating a new connection %p",
                  mediaConnection);
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

   // Create the sockets for audio stream
   createRtpSocketPair(mediaConnection->mLocalAddress, mediaConnection->mContactType,
                       mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtcpAudioSocket);

   // Start the audio packet pump
   mpFlowGraph->startReceiveRtp(NULL, 0,
                                *mediaConnection->mpRtpAudioSocket,
                                *mediaConnection->mpRtcpAudioSocket,
                                connectionId);

   // Store audio stream settings
   mediaConnection->mRtpAudioReceivePort = mediaConnection->mpRtpAudioSocket->getLocalHostPort() ;
   mediaConnection->mRtcpAudioReceivePort = mediaConnection->mpRtcpAudioSocket->getLocalHostPort() ;

   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpPhoneMediaInterface::createConnection creating a new RTP socket: %p descriptor: %d",
            mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpPhoneMediaInterface::createConnection creating a new RTCP socket: %p descriptor: %d",
            mediaConnection->mpRtcpAudioSocket, mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());

#ifdef SIPX_VIDEO // [
   // TODO:: sometimes this code should be generalized and used extracted to
   //        external function and use for audio and video and etc.

   // Create the sockets for outgoing video stream
   createRtpSocketPair(mediaConnection->mLocalAddress, mediaConnection->mContactType,
                       mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtcpVideoSocket);

   if (videoWindowHandle != NULL)
   {
      // Set window for remote video
      setVideoWindowDisplay(videoWindowHandle);

      // Start the audio packet pump
      mpVideoFlowGraph->startReceiveRtp(NULL, 0,
                                       *mediaConnection->mpRtpAudioSocket,
                                       *mediaConnection->mpRtcpAudioSocket);

      // Store video stream settings
      mediaConnection->mRtpVideoReceivePort = mediaConnection->mpRtpVideoSocket->getLocalHostPort() ;
      mediaConnection->mRtcpVideoReceivePort = mediaConnection->mpRtcpVideoSocket->getLocalHostPort() ;

      OsSysLog::add(FAC_CP, PRI_DEBUG, 
               "CpPhoneMediaInterface::createConnection creating a new RTP socket: %p descriptor: %d",
               mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtpVideoSocket->getSocketDescriptor());
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
               "CpPhoneMediaInterface::createConnection creating a new RTCP socket: %p descriptor: %d",
               mediaConnection->mpRtcpVideoSocket, mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());
   }
#endif // SIPX_VIDEO ]

   // Set codec factory
   mediaConnection->mpCodecFactory = new SdpCodecFactory(mSupportedCodecs);
   mediaConnection->mpCodecFactory->bindPayloadTypes();
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpPhoneMediaInterface::createConnection creating a new SdpCodecFactory %p",
            mediaConnection->mpCodecFactory);

    return OS_SUCCESS;
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
    CpPhoneMediaConnection* pMediaConn = getMediaConnection(connectionId);
    rtpAudioPort = 0 ;
    rtcpAudioPort = 0 ;
    rtpVideoPort = 0 ;
    rtcpVideoPort = 0 ; 
    videoBandwidth = 0 ;

    if (pMediaConn)
    {
        if (    (pMediaConn->mContactType == CONTACT_AUTO) || 
                (pMediaConn->mContactType == CONTACT_NAT_MAPPED))
        {
            // Audio RTP
            if (pMediaConn->mpRtpAudioSocket)
            {
                // The "rtpHostAddress" is used for the first RTP stream -- 
                // others are ignored.  They *SHOULD* be the same as the first.  
                // Possible exceptions: STUN worked for the first, but not the
                // others.  Not sure how to handle/recover from that case.
                if (pMediaConn->mContactType == CONTACT_RELAY)
                {
                    if (!pMediaConn->mpRtpAudioSocket->getRelayIp(&rtpHostAddress, &rtpAudioPort))
                    {
                        rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                        rtpHostAddress = mRtpReceiveHostAddress ;
                    }

                }
                else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtpAudioSocket->getMappedIp(&rtpHostAddress, &rtpAudioPort))
                    {
                        rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                        rtpHostAddress = mRtpReceiveHostAddress ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_LOCAL)
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
                if (pMediaConn->mContactType == CONTACT_RELAY)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtcpAudioSocket->getRelayIp(&tempHostAddress, &rtcpAudioPort))
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtcpAudioSocket->getMappedIp(&tempHostAddress, &rtcpAudioPort))
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_LOCAL)
                {
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

#ifdef SIPX_VIDEO // [
            // Video RTP
            if (pMediaConn->mpRtpVideoSocket)
            {
                if (pMediaConn->mContactType == CONTACT_RELAY)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtpVideoSocket->getRelayIp(&tempHostAddress, &rtpVideoPort))
                    {
                        rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of audio RTP stream
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtpVideoSocket->getMappedIp(&tempHostAddress, &rtpVideoPort))
                    {
                        rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of audio RTP stream
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_LOCAL)
                {
                    rtpVideoPort = pMediaConn->mpRtpVideoSocket->getLocalHostPort();
                    if (rtpVideoPort <= 0)
                    {
                        rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
                    }
                }
                else
                {
                    assert(0);
                }               
            }

            // Video RTCP
            if (pMediaConn->mpRtcpVideoSocket)
            {
                if (pMediaConn->mContactType == CONTACT_RELAY)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtcpVideoSocket->getRelayIp(&tempHostAddress, &rtcpVideoPort))
                    {
                        rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of audio RTP stream
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
                {
                    UtlString tempHostAddress;
                    if (!pMediaConn->mpRtcpVideoSocket->getMappedIp(&tempHostAddress, &rtcpVideoPort))
                    {
                        rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of audio RTP stream
                        assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (pMediaConn->mContactType == CONTACT_LOCAL)
                {
                    rtcpVideoPort = pMediaConn->mpRtcpVideoSocket->getLocalHostPort();
                    if (rtcpVideoPort <= 0)
                    {
                        rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
                    }
                }                
                else
                {
                    assert(0);
                }
            }
#endif // SIPX_VIDEO ]
        }
        else
        {
            rtpHostAddress = mRtpReceiveHostAddress ;
            rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
            rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
#ifdef SIPX_VIDEO // [
            rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
            rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
#endif // SIPX_VIDEO ]
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
                                                  RTP_TRANSPORT transportTypes[],
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
            case CONTACT_LOCAL:
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
            case CONTACT_RELAY:
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

        // TODO: Need to get real transport types
        for (int i=0; i<nActualAddresses; i++)
        {
            transportTypes[i] = RTP_TRANSPORT_UDP ;
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
        }

        if(pMediaConnection->mpRtcpAudioSocket && (remoteAudioRtcpPort > 0))
        {
            pMediaConnection->mpRtcpAudioSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtcpPort) ;
            pMediaConnection->mpRtcpAudioSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtcpPort) ;
        }
        else
        {
            pMediaConnection->mRtcpAudioSendHostPort = 0 ;
        }

        /*
         * Video Setup
         */
#ifdef SIPX_VIDEO // [
        if (pMediaConnection->mpRtpVideoSocket)
        {
            pMediaConnection->mRtpVideoSendHostPort = remoteVideoRtpPort ;                   
            pMediaConnection->mpRtpVideoSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtpPort) ;
            pMediaConnection->mpRtpVideoSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtpPort) ;

            if(pMediaConnection->mpRtcpVideoSocket && (remoteVideoRtcpPort > 0))
            {
                pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;               
                pMediaConnection->mpRtcpVideoSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtcpPort) ;
                pMediaConnection->mpRtcpVideoSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtcpPort) ;
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
#endif // SIPX_VIDEO ]

        returnCode = OS_SUCCESS;
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
#ifdef SIPX_VIDEO
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
#ifdef SIPX_VIDEO
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
   SdpCodec* audioCodec = NULL;
   SdpCodec* dtmfCodec = NULL;
   SdpCodec* videoCodec = NULL;
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return returnCode;

   // Find primary audio, DTMF and video codecs
   for (i=0; i<numCodecs; i++) {
      if (SdpCodec::SDP_CODEC_TONES == sendCodecs[i]->getValue()) {
         if (NULL == dtmfCodec) {
            dtmfCodec = sendCodecs[i];
         }
         continue;
      }

      UtlString codecMediaType;
      sendCodecs[i]->getMediaType(codecMediaType);

      if (codecMediaType == "audio" && audioCodec == NULL) {
         audioCodec = sendCodecs[i];
      }
      else if (codecMediaType == "video" && videoCodec == NULL)
      {
         videoCodec = sendCodecs[i];
      }
   }

   // If we haven't set a destination and we have set alternate destinations
   if (!mediaConnection->mDestinationSet && mediaConnection->mbAlternateDestinations)
   {
      applyAlternateDestinations(connectionId) ;
   }

   // Make sure we use the same payload types as the remote
   // side.  Its the friendly thing to do.
   if (mediaConnection->mpCodecFactory)
   {
      mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                         sendCodecs);
   }

   if (mpFlowGraph)
   {
#ifdef TEST_PRINT
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Sending RTP/RTCP codec: %d sockets: %p/%p descriptors: %d/%d\n",
           audioCodec ? audioCodec->getCodecType() : -2,
           (mediaConnection->mpRtpAudioSocket), (mediaConnection->mpRtcpAudioSocket),
           mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
#endif

       // Store the primary codec for cost calculations later
       if (mediaConnection->mpAudioCodec != NULL)
       {
           delete mediaConnection->mpAudioCodec ;
           mediaConnection->mpAudioCodec = NULL ;
       }
       if (audioCodec != NULL)
       {
           mediaConnection->mpAudioCodec = new SdpCodec();
           *mediaConnection->mpAudioCodec = *audioCodec ;
       }

       // Make sure we use the same payload types as the remote
       // side.  Its the friendly thing to do.
       if (mediaConnection->mpCodecFactory)
       {
           mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                            sendCodecs);
       }

       if (mediaConnection->mRtpAudioSending)
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
                                   audioCodec,
                                   dtmfCodec);

         mediaConnection->mRtpAudioSending = TRUE;
         returnCode = OS_SUCCESS;
      }
   }

#ifdef SIPX_VIDEO // [
   if (mpVideoFlowGraph)
   {
#ifdef TEST_PRINT
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Sending RTP/RTCP codec: %d sockets: %p/%p descriptors: %d/%d\n",
           videoCodec ? videoCodec->getCodecType() : -2,
           (mediaConnection->mpRtpVideoSocket), (mediaConnection->mpRtcpVideoSocket),
           mediaConnection->mpRtpVideoSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());
#endif
       // Store the primary codec for cost calculations later
       if (mediaConnection->mpVideoCodec != NULL)
       {
           delete mediaConnection->mpVideoCodec ;
           mediaConnection->mpVideoCodec = NULL ;
       }
       if (videoCodec != NULL)
       {
           mediaConnection->mpVideoCodec = new SdpCodec();
           *mediaConnection->mpVideoCodec = *videoCodec ;
       }

       if (mediaConnection->mRtpVideoSending)
       {
           mpVideoFlowGraph->stopSendRtp();
       }

      if ( !mediaConnection->mRtpSendHostAddress.isNull()
         && mediaConnection->mRtpSendHostAddress.compareTo("0.0.0.0")
         && mediaConnection->mpRtpVideoSocket != NULL
         && mediaConnection->mpRtcpVideoSocket != NULL
         && mediaConnection->mpVideoCodec != NULL
         )
      {
         if (mpVideoFlowGraph->startSendRtp(*(mediaConnection->mpVideoCodec),
                                            *(mediaConnection->mpRtpVideoSocket),
                                            *(mediaConnection->mpRtcpVideoSocket))
             == OS_SUCCESS)
         {
            mediaConnection->mRtpVideoSending = TRUE;
            returnCode = OS_SUCCESS;
         }
         else
         {
            mediaConnection->mRtpVideoSending = FALSE;
         }
      }
   }
#endif // SIPX_VIDEO ]

   return returnCode;
}


OsStatus CpPhoneMediaInterface::startRtpReceive(int connectionId,
                                                int numCodecs,
                                                SdpCodec* receiveCodecs[])
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;


   // Make sure we use the same payload types as the remote
   // side.  It's the friendly thing to do.
   if (mediaConnection->mpCodecFactory)
   {
         mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                           receiveCodecs);
   }

   if (mpFlowGraph)
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

#ifdef SIPX_VIDEO // [
   if (mpVideoFlowGraph)
   {
#ifdef TEST_PRINT
      int i;

      OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Receiving RTP/RTCP, %d codec%s; sockets: %p/%p descriptors: %d/%d\n",
           numCodecs, ((1==numCodecs)?"":"s"),
           (mediaConnection->mpRtpVideoSocket),
           (mediaConnection->mpRtcpVideoSocket),
           mediaConnection->mpRtpVideoSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());
      for (i=0; i<numCodecs; i++) {
          osPrintf("   %d:  i:%d .. x:%d\n", i+1,
                   receiveCodecs[i]->getCodecType(),
                   receiveCodecs[i]->getCodecPayloadFormat());
      }
#endif

      if (mediaConnection->mRtpVideoReceiving)
      {
         // This is not supposed to be necessary and may be
         // causing an audible glitch when codecs are changed
         mpVideoFlowGraph->stopReceiveRtp();
         mediaConnection->mRtpVideoReceiving = FALSE;
      }

      if (mpVideoFlowGraph->startReceiveRtp(receiveCodecs, numCodecs,
                                            *(mediaConnection->mpRtpVideoSocket),
                                            *(mediaConnection->mpRtcpVideoSocket))
          == OS_SUCCESS)
      {
         mediaConnection->mRtpVideoReceiving = TRUE;
      }

      returnCode = OS_SUCCESS;
   }
#endif // SIPX_VIDEO ]

   return returnCode;
}

OsStatus CpPhoneMediaInterface::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if (mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioSending)
   {
      mpFlowGraph->stopSendRtp(connectionId);
      mediaConnection->mRtpAudioSending = FALSE;
      returnCode = OS_SUCCESS;
   }

#ifdef SIPX_VIDEO // [
   if (mpVideoFlowGraph && mediaConnection &&
       mediaConnection->mRtpVideoSending)
   {
      mpVideoFlowGraph->stopSendRtp();
      mediaConnection->mRtpVideoSending = FALSE;
      returnCode = OS_SUCCESS;
   }
#endif // SIPX_VIDEO ]

   return(returnCode);
}

OsStatus CpPhoneMediaInterface::stopRtpReceive(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpPhoneMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if (mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioReceiving)
   {
      mpFlowGraph->stopReceiveRtp(connectionId);
      mediaConnection->mRtpAudioReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }

#ifdef SIPX_VIDEO // [
   if (mpVideoFlowGraph && mediaConnection &&
       mediaConnection->mRtpVideoReceiving)
   {
      mpVideoFlowGraph->stopReceiveRtp();
      mediaConnection->mRtpVideoReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }
#endif // SIPX_VIDEO ]

   return returnCode;
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

   if(mediaConnection == NULL)
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                  "CpPhoneMediaInterface::doDeleteConnection mediaConnection is NULL!");
      return OS_NOT_FOUND;
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPhoneMediaInterface::deleteConnection deleting the connection %p",
      mediaConnection);

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
   else 
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "CpPhoneMediaInterface::doDeleteConnection NULL socket: mpRtpAudioSocket=0x%08x, mpRtpAudioSocket=0x%08x",
                    mediaConnection->mpRtpAudioSocket,
                    mediaConnection->mpRtcpAudioSocket);
#endif

   returnCode = stopRtpSend(mediaConnection->getValue());
   returnCode = stopRtpReceive(mediaConnection->getValue());

   // TODO:: Stop video RTP stream here.

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

#ifdef SIPX_VIDEO // [
   // Delete video RTP sockets

#ifdef TEST_PRINT
   if (mediaConnection && mediaConnection->mpRtpVideoSocket && mediaConnection->mpRtcpVideoSocket)
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "stopping RTP/RTCP send & receive sockets %p/%p descriptors: %d/%d",
                    mediaConnection->mpRtpVideoSocket,
                    mediaConnection->mpRtcpVideoSocket,
                    mediaConnection->mpRtpVideoSocket->getSocketDescriptor(),
                    mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());
   else 
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "CpPhoneMediaInterface::doDeleteConnection NULL socket: mpRtpVideoSocket=0x%08x, mpRtpVideoSocket=0x%08x",
                    mediaConnection->mpRtpVideoSocket,
                    mediaConnection->mpRtcpVideoSocket);
#endif
   mpFactoryImpl->releaseRtpPort(mediaConnection->mRtpVideoReceivePort) ;

   if(mediaConnection->mpRtpVideoSocket)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, "deleting RTP socket: %p descriptor: %d",
                    mediaConnection->mpRtpVideoSocket,
                    mediaConnection->mpRtpVideoSocket->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtpVideoSocket;
      mediaConnection->mpRtpVideoSocket = NULL;
   }
   if(mediaConnection->mpRtcpVideoSocket)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, "deleting RTCP socket: %p descriptor: %d",
         mediaConnection->mpRtcpVideoSocket,
         mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtcpVideoSocket;
      mediaConnection->mpRtcpVideoSocket = NULL;
   }
#endif // SIPX_VIDEO ]

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
                                           OsProtectedEvent* pEvent,
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
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return playAudio(url, repeat, local, remote, mixWithMic, downScaling) ;
}


OsStatus CpPhoneMediaInterface::stopChannelAudio(int connectionId) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return stopAudio() ;
}


OsStatus CpPhoneMediaInterface::recordChannelAudio(int connectionId,
                                                   const char* szFile) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    // 
    // Also -- not sure of the if the spkr is the correct place to record -- 
    // this probably doesn't include mic data...
    ///

    double duration = 0 ;
    int dtmf = 0 ;
    return mpFlowGraph->record(1, -1, NULL, NULL, szFile) ;
}

OsStatus CpPhoneMediaInterface::stopRecordChannelAudio(int connectionId) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.


    return stopRecording() ;
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

OsStatus CpPhoneMediaInterface::recordMic(int ms,
                                          int silenceLength,
                                          const char* fileName)
{
    OsStatus ret = OS_UNSPECIFIED;
    if (mpFlowGraph && fileName)
    {
        ret = mpFlowGraph->recordMic(ms, silenceLength, fileName);
    }
    return ret ;
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

void CpPhoneMediaInterface::setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) 
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
    // TODO:: setConnectionFramerate()
    return OS_NOT_SUPPORTED ;
}


OsStatus CpPhoneMediaInterface::setSecurityAttributes(const void* security) 
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpPhoneMediaInterface::generateVoiceQualityReport(int         connectiond,
                                                           const char* callId,
                                                           UtlString&  report) 
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
   // TODO:: setVideoQuality()
   return OS_SUCCESS;
}

OsStatus CpPhoneMediaInterface::setVideoParameters(int bitRate, int frameRate)
{
   // TODO:: setVideoParameters()
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
         if (mediaConnection->mpAudioCodec != NULL)
         {
            int iCodecCost = mediaConnection->mpAudioCodec->getCPUCost();
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
    UtlString codecType;
    CpPhoneMediaConnection* pConnection = getMediaConnection(connectionId);
    if (pConnection == NULL)
       return OS_NOT_FOUND;

    if (pConnection->mpAudioCodec != NULL)
    {
        pConnection->mpAudioCodec->getEncodingName(audioCodec);
        *audioPayloadType = pConnection->mpAudioCodec->getCodecPayloadFormat();
    }

#ifdef SIPX_VIDEO // [
    if (pConnection->mpVideoCodec != NULL)
    {
        pConnection->mpVideoCodec->getEncodingName(videoCodec);
        *videoPayloadType = pConnection->mpVideoCodec->getCodecPayloadFormat();
    }
#else // SIPX_VIDEO ][
    videoCodec="";
    *videoPayloadType=0;
#endif // SIPX_VIDEO ]

   return OS_SUCCESS;
}

OsStatus CpPhoneMediaInterface::getVideoQuality(int& quality)
{
   // TODO:: getVideoQuality()
   quality = 0;
   return OS_SUCCESS;
}

OsStatus CpPhoneMediaInterface::getVideoBitRate(int& bitRate)
{
   // TODO:: getVideoBitRate()
   bitRate = 0;
   return OS_SUCCESS;
}


OsStatus CpPhoneMediaInterface::getVideoFrameRate(int& frameRate)
{
   // TODO:: getVideoFrameRate()
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
       osPrintf("CpPhoneMediaInterface::isSendingRtpAudio invalid connectionId: %d\n",
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
       osPrintf("CpPhoneMediaInterface::isReceivingRtpAudio invalid connectionId: %d\n",
          connectionId);
   }
   return(receiving);
}

UtlBoolean CpPhoneMediaInterface::isSendingRtpVideo(int connectionId)
{
   UtlBoolean sending = FALSE;

#ifdef SIPX_VIDEO // [
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
       sending = mediaConnection->mRtpVideoSending;
   }
   else
   {
       osPrintf("CpPhoneMediaInterface::isSendingRtpVideo invalid connectionId: %d\n",
          connectionId);
   }
#endif // SIPX_VIDEO ]

   return(sending);
}

UtlBoolean CpPhoneMediaInterface::isReceivingRtpVideo(int connectionId)
{
   UtlBoolean receiving = FALSE;

#ifdef SIPX_VIDEO // [
   CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
      receiving = mediaConnection->mRtpVideoReceiving;
   }
   else
   {
       osPrintf("CpPhoneMediaInterface::isReceivingRtpVideo invalid connectionId: %d\n",
          connectionId);
   }
#endif // SIPX_VIDEO ]

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


UtlBoolean CpPhoneMediaInterface::isVideoInitialized(int connectionId)
{
    // TODO:: isVideoInitialized()
#ifdef SIPX_VIDEO // [
   return true ;
#else // SIPX_VIDEO ][
   return false ;
#endif // SIPX_VIDEO ]
}

UtlBoolean CpPhoneMediaInterface::isAudioInitialized(int connectionId) 
{
    return true ;
}

UtlBoolean CpPhoneMediaInterface::isAudioAvailable() 
{
    return true ;
}

OsStatus CpPhoneMediaInterface::setVideoWindowDisplay(const void* hWnd)
{
#ifdef SIPX_VIDEO // [
   mpVideoFlowGraph->setVideoWindow(hWnd);
   return OS_SUCCESS;
#else // SIPX_VIDEO ][
   return OS_NOT_YET_IMPLEMENTED;
#endif // SIPX_VIDEO ]
}

const void* CpPhoneMediaInterface::getVideoWindowDisplay()
{
#ifdef SIPX_VIDEO // [
   return mpVideoFlowGraph->getVideoWindow();
#else // SIPX_VIDEO ][
   return NULL;
#endif // SIPX_VIDEO ]
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */


UtlBoolean CpPhoneMediaInterface::getLocalAddresses(int connectionId,
                                              UtlString& hostIp,
                                              int& rtpAudioPort,
                                              int& rtcpAudioPort,
                                              int& rtpVideoPort,
                                              int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
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
                bRC = TRUE ;
            }

            // Audio rtcp port (optional) 
            if (pMediaConn->mpRtcpAudioSocket && bRC)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
            }        
        }

#ifdef SIPX_VIDEO
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

UtlBoolean CpPhoneMediaInterface::getNatedAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
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

                    bRC = TRUE ;
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

#ifdef SIPX_VIDEO
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
                if (pMediaConn->mpRtcpVideoSocket)
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

UtlBoolean CpPhoneMediaInterface::getRelayAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
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

                    bRC = TRUE ;
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

#ifdef SIPX_VIDEO
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
                if (pMediaConn->mpRtcpVideoSocket)
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
        UtlBoolean bDuplicate = FALSE ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = TRUE ;
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

    CpPhoneMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if (pMediaConnection)
    {
        assert(!pMediaConnection->mDestinationSet) ;
        pMediaConnection->mDestinationSet = true ;

        pMediaConnection->mRtpSendHostAddress.remove(0) ;
        pMediaConnection->mRtpAudioSendHostPort = 0 ;
        pMediaConnection->mRtcpAudioSendHostPort = 0 ;
#ifdef SIPX_VIDEO
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
            }
        }

        // Connect RTCP Audio Socket
        if (pMediaConnection->mpRtcpAudioSocket)
        {
            if (pMediaConnection->mpRtcpAudioSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtcpAudioSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpAudioSendHostPort = destPort;                
            }            
        }

        // TODO:: Enable/Disable RTCP

#ifdef SIPX_VIDEO
        // Connect RTP Video Socket
        if (pMediaConnection->mpRtpVideoSocket)
        {
            if (pMediaConnection->mpRtpVideoSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtpVideoSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtpVideoSendHostPort = destPort;
            }            
        }

        // Connect RTCP Video Socket
        if (pMediaConnection->mpRtcpVideoSocket)
        {
            if (pMediaConnection->mpRtcpVideoSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pMediaConnection->mpRtcpVideoSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpVideoSendHostPort = destPort;
            }            
        }

        // TODO:: Enable/Disable RTCP
#endif
    }
}

OsStatus CpPhoneMediaInterface::setMediaProperty(const UtlString& propertyName,
                                                 const UtlString& propertyValue)
{
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpPhoneMediaInterface::setMediaProperty %p propertyName=\"%s\" propertyValue=\"%s\"",
        this, propertyName.data(), propertyValue.data());

    UtlString* oldProperty = (UtlString*)mInterfaceProperties.findValue(&propertyValue);
    if(oldProperty)
    {
        // Update the old value
        (*oldProperty) = propertyValue;
    }
    else
    {
        // No prior value for this property create copies for the map
        mInterfaceProperties.insertKeyAndValue(new UtlString(propertyName),
                                               new UtlString(propertyValue));
    }
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpPhoneMediaInterface::getMediaProperty(const UtlString& propertyName,
                                                 UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_FOUND;
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpPhoneMediaInterface::getMediaProperty %p propertyName=\"%s\"",
        this, propertyName.data());

    UtlString* foundValue = (UtlString*)mInterfaceProperties.findValue(&propertyName);
    if(foundValue)
    {
        propertyValue = *foundValue;
        returnCode = OS_SUCCESS;
    }
    else
    {
        propertyValue = "";
        returnCode = OS_NOT_FOUND;
    }

    returnCode = OS_NOT_YET_IMPLEMENTED;
    return(returnCode);
}

OsStatus CpPhoneMediaInterface::setMediaProperty(int connectionId,
                                                 const UtlString& propertyName,
                                                 const UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_YET_IMPLEMENTED;
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpPhoneMediaInterface::setMediaProperty %p connectionId=%d propertyName=\"%s\" propertyValue=\"%s\"",
        this, connectionId, propertyName.data(), propertyValue.data());

    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if(mediaConnection)
    {
        UtlString* oldProperty = (UtlString*)(mediaConnection->mConnectionProperties.findValue(&propertyValue));
        if(oldProperty)
        {
            // Update the old value
            (*oldProperty) = propertyValue;
        }
        else
        {
            // No prior value for this property create copies for the map
            mediaConnection->mConnectionProperties.insertKeyAndValue(new UtlString(propertyName),
                                                                     new UtlString(propertyValue));
        }
    }
    else
    {
        returnCode = OS_NOT_FOUND;
    }


    return(returnCode);
}

OsStatus CpPhoneMediaInterface::getMediaProperty(int connectionId,
                                                 const UtlString& propertyName,
                                                 UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_FOUND;
    propertyValue = "";

    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpPhoneMediaInterface::getMediaProperty %p connectionId=%d propertyName=\"%s\"",
        this, connectionId, propertyName.data());

    CpPhoneMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if(mediaConnection)
    {
        UtlString* oldProperty = (UtlString*)(mediaConnection->mConnectionProperties.findValue(&propertyName));
        if(oldProperty)
        {
            propertyValue = *oldProperty;
            returnCode = OS_SUCCESS;
        }
    }

    return OS_NOT_YET_IMPLEMENTED;//(returnCode);
}
/* //////////////////////////// PROTECTED ///////////////////////////////// */

void CpPhoneMediaInterface::createRtpSocketPair(UtlString localAddress,
                                                SIPX_CONTACT_TYPE contactType,
                                                OsNatDatagramSocket* &rtpSocket,
                                                OsNatDatagramSocket* &rtcpSocket)
{
   int localPort;
   int firstRtpPort;

   mpFactoryImpl->getNextRtpPort(localPort);
   firstRtpPort = localPort;

   // Eventually this should use a specified address as this
   // host may be multi-homed
   rtpSocket = new OsNatDatagramSocket(0, NULL, localPort, localAddress, NULL);
   rtpSocket->enableTransparentReads(false);

   rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                        localAddress, NULL);
   rtcpSocket->enableTransparentReads(false);

   // Validate local port is not auto-selecting.
   if (localPort != 0)
   {
      // If either of the sockets are bad (e.g. already in use) or
      // if either have stuff on them to read (e.g. someone is
      // sending junk to the ports, look for another port pair
      while(!rtpSocket->isOk() ||
            !rtcpSocket->isOk() ||
             rtcpSocket->isReadyToRead() ||
             rtpSocket->isReadyToRead(60))
      {
            localPort +=2;
            // This should use mLastRtpPort instead of some
            // hardcoded MAX, but I do not think mLastRtpPort
            // is set correctly in all of the products.
            if(localPort > firstRtpPort + MAX_RTP_PORTS) 
            {
               OsSysLog::add(FAC_CP, PRI_ERR, 
                  "No available ports for RTP and RTCP in range %d - %d",
                  firstRtpPort, firstRtpPort + MAX_RTP_PORTS);
               break;  // time to give up
            }

            delete rtpSocket;
            delete rtcpSocket;
            rtpSocket = new OsNatDatagramSocket(0, NULL, localPort,
               mLocalAddress.data(), NULL);
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
      oRtp = 20000;
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
   if ((mStunServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mbEnableICE))
   {
      rtpSocket->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
   }

   // Enable Turn if we have a stun server and either non-local contact type or 
   // ICE is enabled.
   if ((mTurnServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mbEnableICE))
   {
      rtpSocket->enableTurn(mTurnServer, mTurnPort, 
               mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
   }

   // Enable Stun if we have a stun server and either non-local contact type or 
   // ICE is enabled.
   if ((mStunServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mbEnableICE))
   {
      rtcpSocket->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
   }

   // Enable Turn if we have a stun server and either non-local contact type or 
   // ICE is enabled.
   if ((mTurnServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mbEnableICE))
   {
      rtcpSocket->enableTurn(mTurnServer, mTurnPort, 
               mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

