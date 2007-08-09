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
#include <utl/UtlHashBag.h>
#include <os/OsDatagramSocket.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsMulticastSocket.h>
#include <os/OsProtectEventMgr.h>
#include <os/OsSysLog.h>
#include <os/OsStatus.h>
#include <mp/MpTopologyGraph.h>
#include <mp/MpResourceTopology.h>
#include <mp/MprBufferRecorder.h>
#include <mp/MprToneGen.h>
#include <mp/MprFromFile.h>
#include <mp/MpRtpInputAudioConnection.h>
#include <mp/MpRtpOutputAudioConnection.h>
#include <mp/dtmflib.h>
#include <mp/MpMediaTask.h>
#include <include/CpTopologyGraphInterface.h>
#include <include/CpTopologyGraphFactoryImpl.h>

#include <sdp/SdpCodec.h>
#include <net/SdpCodecFactory.h>

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

class CpTopologyMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    CpTopologyMediaConnection(int connectionId = -1)
    : UtlInt(connectionId)
    , mRtpSendHostAddress()
    , mDestinationSet(FALSE)
    , mIsMulticast(FALSE)
    , mpRtpAudioSocket(NULL)
    , mpRtcpAudioSocket(NULL)
    , mRtpAudioSendHostPort(0)
    , mRtcpAudioSendHostPort(0)
    , mRtpAudioReceivePort(0)
    , mRtcpAudioReceivePort(0)
    , mRtpAudioSending(FALSE)
    , mRtpAudioReceiving(FALSE)
    , mpAudioCodec(NULL)
    , mpCodecFactory(NULL)
    , mContactType(CONTACT_AUTO)
    , mbAlternateDestinations(FALSE)
    {
    };

    virtual ~CpTopologyMediaConnection()
    {
        if(mpRtpAudioSocket)
        {

#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting RTP socket: %p descriptor: %d",
                mpRtpAudioSocket, mpRtpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtpAudioSocket;
            mpRtpAudioSocket = NULL;
        }

        if(mpRtcpAudioSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting RTCP socket: %p descriptor: %d",
                mpRtcpAudioSocket, mpRtcpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtcpAudioSocket;
            mpRtcpAudioSocket = NULL;
        }

        if(mpCodecFactory)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpTopologyMediaConnection deleting SdpCodecFactory %p",
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
    UtlBoolean mIsMulticast;
    OsDatagramSocket* mpRtpAudioSocket;
    OsDatagramSocket* mpRtcpAudioSocket;
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    UtlBoolean mRtpAudioSending;
    UtlBoolean mRtpAudioReceiving;
    SdpCodec* mpAudioCodec;
    SdpCodecFactory* mpCodecFactory;
    SIPX_CONTACT_TYPE mContactType ;
    UtlString mLocalAddress ;
    UtlBoolean mbAlternateDestinations ;
    UtlHashMap mConnectionProperties;
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphInterface::CpTopologyGraphInterface(CpTopologyGraphFactoryImpl* pFactoryImpl,
                                             const char* publicAddress,
                                             const char* localAddress,
                                             int numCodecs,
                                             SdpCodec* sdpCodecArray[],
                                             const char* locale,
                                             int expeditedIpTos,
                                             const char* stunServer,
                                             int stunPort,
                                             int stunKeepAlivePeriodSecs,
                                             const char* turnServer,
                                             int turnPort,
                                             const char* turnUsername,
                                             const char* turnPassword,
                                             int turnKeepAlivePeriodSecs,
                                             UtlBoolean enableIce)
    : CpMediaInterface(pFactoryImpl)
{
    mLastConnectionId = 0;

   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new CpMediaInterface %p",
                 this);

   mpTopologyGraph = new MpTopologyGraph(80,
                                         8000,
                                         *(pFactoryImpl->getInitialResourceTopology()),
                                         *(pFactoryImpl->getResourceFactory()));
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new MpTopologyGraph %p",
                 mpTopologyGraph);
   
   mStunServer = stunServer;
   mStunPort = stunPort;
   mStunRefreshPeriodSecs = stunKeepAlivePeriodSecs;
   mTurnServer = turnServer;
   mTurnPort = turnPort;
   mTurnRefreshPeriodSecs = turnKeepAlivePeriodSecs;
   mTurnUsername = turnUsername;
   mTurnPassword = turnPassword;
   mEnableIce = enableIce;

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
        *     the CpTopologyGraphInterface should need to know about ALL the 
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
                            "CpTopologyGraphInterface::CpTopologyGraphInterface dropping codec type %d as not supported",
                            cType);
              break;
          }  
       }
       mSupportedCodecs.buildSdpCodecFactory(codecList);
       
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "CpTopologyGraphInterface::CpTopologyGraphInterface creating codec factory with %s",
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
                          "PCMU PCMA TELEPHONE-EVENT";
       OsSysLog::add(FAC_CP, PRI_WARNING, "CpTopologyGraphInterface::CpTopologyGraphInterface hard-coded codec factory %s ...",
                     codecs.data());
       mSupportedCodecs.buildSdpCodecFactory(codecs);
   }

   mExpeditedIpTos = expeditedIpTos;
}


// Destructor
CpTopologyGraphInterface::~CpTopologyGraphInterface()
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::~CpTopologyGraphInterface deleting the CpMediaInterface %p",
                 this);

    CpTopologyMediaConnection* mediaConnection = NULL;
    while ((mediaConnection = (CpTopologyMediaConnection*) mMediaConnections.get()))
    {
        doDeleteConnection(mediaConnection);
        delete mediaConnection;
        mediaConnection = NULL;
    }

    if(mpTopologyGraph)
    {
      // Free up the resources used by tone generation ASAP
      stopTone();

        // Stop the net in/out stuff before the sockets are deleted
        //mpMediaFlowGraph->stopReceiveRtp();
        //mpMediaFlowGraph->stopSendRtp();

        //MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);

        // take focus away from the flow graph if it is focus
        //if(mpTopologyGraph == (MpCallFlowGraph*) mediaTask->getFocus())
        //{
        //    mediaTask->setFocus(NULL);
        //}

        OsSysLog::add(FAC_CP, PRI_DEBUG, 
        //printf(
                      "CpTopologyGraphInterface::~CpTopologyGraphInterface deleting the MpTopologyGraph %p",
                      mpTopologyGraph);
        delete mpTopologyGraph;
        mpTopologyGraph = NULL;
    }

    // Delete the properties and their values
    mInterfaceProperties.destroyAll();
}

/**
 * public interface for destroying this media interface
 */ 
void CpTopologyGraphInterface::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */

OsStatus CpTopologyGraphInterface::createConnection(int& connectionId,
                                                    const char* szLocalAddress,
                                                    int localPort,
                                                    void* videoWindowHandle, 
                                                    void* const pSecurityAttributes,
                                                    ISocketEvent* pIdleEvent,
                                                    IMediaEventListener* pMediaEventListener,
                                                    const RtpTransportOptions rtpTransportOptions)
{
   OsStatus retValue = OS_SUCCESS;
   CpTopologyMediaConnection* mediaConnection=NULL;
   CpTopologyGraphFactoryImpl* pTopologyFactoryImpl = (CpTopologyGraphFactoryImpl*)mpFactoryImpl;

   connectionId = getNextConnectionId();
   if (connectionId == -1)
   {
      return OS_LIMIT_REACHED;
   }
   mpTopologyGraph->addResources(*pTopologyFactoryImpl->getConnectionResourceTopology(),
                                 pTopologyFactoryImpl->getResourceFactory(),
                                 connectionId);

   mediaConnection = new CpTopologyMediaConnection(connectionId);
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CpTopologyGraphInterface::createConnection "
                 "creating a new connection %p",
                 mediaConnection);
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

   mediaConnection->mIsMulticast = OsSocket::isMcastAddr(mediaConnection->mLocalAddress);
   if (mediaConnection->mIsMulticast)
   {
      mediaConnection->mContactType = CONTACT_LOCAL;
   }

   // Create the sockets for audio stream
   retValue = createRtpSocketPair(mediaConnection->mLocalAddress, localPort,
                                  mediaConnection->mContactType,
                                  mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtcpAudioSocket);
   if (retValue != OS_SUCCESS)
   {
       return retValue;
   }

   // Start the audio packet pump
   UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
   MpRtpInputAudioConnection::startReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                              inConnectionName,
                                              NULL,
                                              0,
                                              *(mediaConnection->mpRtpAudioSocket),
                                              *(mediaConnection->mpRtcpAudioSocket));


   // Store audio stream settings
   mediaConnection->mRtpAudioReceivePort = mediaConnection->mpRtpAudioSocket->getLocalHostPort() ;
   mediaConnection->mRtcpAudioReceivePort = mediaConnection->mpRtcpAudioSocket->getLocalHostPort() ;

   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new RTP socket: %p descriptor: %d",
            mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new RTCP socket: %p descriptor: %d",
            mediaConnection->mpRtcpAudioSocket, mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());

   // Set codec factory
   mediaConnection->mpCodecFactory = new SdpCodecFactory(mSupportedCodecs);
   mediaConnection->mpCodecFactory->bindPayloadTypes();
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new SdpCodecFactory %p",
            mediaConnection->mpCodecFactory);

    return retValue;
}

/// @copydoc CpMediaInterface::setMediaNotificationsEnabled()
OsStatus 
CpTopologyGraphInterface::setMediaNotificationsEnabled(bool enabled, 
                                                       const UtlString& resourceName)
{
   return mpTopologyGraph->setNotificationsEnabled(enabled, resourceName);
}

OsStatus CpTopologyGraphInterface::getConnectionPortOnBridge(int connectionId, 
                                                             int& portOnBridge)
{
   assert(connectionId >=0);

   UtlString connectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(connectionName, connectionId);

   MpResource* inConnection = NULL;

   OsStatus retStatus = mpTopologyGraph->lookupResource(connectionName, inConnection);
   if(OS_SUCCESS == retStatus)
   {
      MpResource* doNotTouchResource = NULL;

      inConnection->getOutputInfo(0, // first and only port on in connection
                                  doNotTouchResource, // not safe to access
                                  portOnBridge);
   }
   else
   {
      portOnBridge = -1;
   }
   return(retStatus);
}

OsStatus CpTopologyGraphInterface::getCapabilities(int connectionId,
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
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);
    rtpAudioPort = 0 ;
    rtcpAudioPort = 0 ;
    rtpVideoPort = 0 ;
    rtcpVideoPort = 0 ; 
    videoBandwidth = 0 ;

    if (pMediaConn)
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
                assert(!pMediaConn->mIsMulticast);
                if (!((OsNatDatagramSocket*)pMediaConn->mpRtpAudioSocket)->
                                            getRelayIp(&rtpHostAddress, &rtpAudioPort))
                {
                    rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                    rtpHostAddress = mRtpReceiveHostAddress ;
                }

            }
            else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
            {
                assert(!pMediaConn->mIsMulticast);
                if (!pMediaConn->mpRtpAudioSocket->getMappedIp(&rtpHostAddress, &rtpAudioPort))
                {
                    rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                    rtpHostAddress = pMediaConn->mLocalAddress ;
                }
            }
            else if (pMediaConn->mContactType == CONTACT_LOCAL)
            {
                 rtpHostAddress = pMediaConn->mpRtpAudioSocket->getLocalIp();
                 rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
                 if (rtpAudioPort <= 0)
                 {
                     rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                     rtpHostAddress = pMediaConn->mLocalAddress ;
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
                assert(!pMediaConn->mIsMulticast);
                if (!((OsNatDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->
                                            getRelayIp(&tempHostAddress, &rtcpAudioPort))
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
                assert(!pMediaConn->mIsMulticast);
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


OsStatus CpTopologyGraphInterface::getCapabilitiesEx(int connectionId, 
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
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);
    nActualAddresses = 0 ;

    // Clear input rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, rtcpVideoPorts
    // and transportTypes arrays. Do not suppose them to be cleaned by caller.
    memset(rtpAudioPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtcpAudioPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtpVideoPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtcpVideoPorts, 0, nMaxAddresses*sizeof(int));
    for (int i = 0; i < nMaxAddresses; i++)
    {
       transportTypes[i] = RTP_TRANSPORT_UNKNOWN;
    }

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

CpTopologyMediaConnection* CpTopologyGraphInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId);
   return((CpTopologyMediaConnection*) mMediaConnections.find(&matchConnectionId));
}

OsStatus CpTopologyGraphInterface::setConnectionDestination(int connectionId,
                                                         const char* remoteRtpHostAddress,
                                                         int remoteAudioRtpPort,
                                                         int remoteAudioRtcpPort,
                                                         int remoteVideoRtpPort,
                                                         int remoteVideoRtcpPort)
{
    OsStatus returnCode = OS_NOT_FOUND;
    CpTopologyMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if(pMediaConnection && remoteRtpHostAddress && *remoteRtpHostAddress)
    {
        returnCode = OS_SUCCESS;
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
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpAudioSocket;
                pSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtpPort) ;
                pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtpPort) ;
            }
            else
            {
                pMediaConnection->mpRtpAudioSocket->doConnect(remoteAudioRtpPort,
                                                              remoteRtpHostAddress,
                                                              TRUE);
            }
        }

        if(pMediaConnection->mpRtcpAudioSocket && (remoteAudioRtcpPort > 0))
        {
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpAudioSocket;
                pSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtcpPort) ;
                pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtcpPort) ;
            }
            else
            {
                pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                               remoteRtpHostAddress,
                                                               TRUE);
            }
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
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pRtpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpVideoSocket;
                pRtpSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtpPort) ;
                pRtpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtpPort) ;
            }
            else
            {
                pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                               remoteRtpHostAddress,
                                                               TRUE);
            }

            if(pMediaConnection->mpRtcpVideoSocket && (remoteVideoRtcpPort > 0))
            {
                pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;               
                if (!pMediaConnection->mIsMulticast)
                {
                   OsNatDatagramSocket *pRctpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpVideoSocket;
                   pRctpSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtcpPort) ;
                   pRctpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtcpPort) ;
                }
                else
                {
                   pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                                  remoteRtpHostAddress,
                                                                  TRUE);
                }
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

OsStatus CpTopologyGraphInterface::addAudioRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpAudioSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtpAudioSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus CpTopologyGraphInterface::addAudioRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpAudioSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtcpAudioSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus CpTopologyGraphInterface::addVideoRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpVideoSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtpVideoSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

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

OsStatus CpTopologyGraphInterface::addVideoRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpVideoSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtcpVideoSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

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


OsStatus CpTopologyGraphInterface::startRtpSend(int connectionId,
                                             int numCodecs,
                                             SdpCodec* sendCodecs[])
{
   // need to set default payload types in get capabilities

   int i;
   SdpCodec* audioCodec = NULL;
   SdpCodec* dtmfCodec = NULL;
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

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

   if (mpTopologyGraph)
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
           //mpTopologyGraph->stopSendRtp(connectionId);
       }

#ifdef TEST_PRINT
      UtlString dtmfCodecString;
      if(dtmfCodec) dtmfCodec->toString(dtmfCodecString);
      OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::startRtpSend %susing DTMF codec: %s\n",
         dtmfCodec ? "" : "NOT ",
         dtmfCodecString.data());
#endif

      if (!mediaConnection->mRtpSendHostAddress.isNull() && mediaConnection->mRtpSendHostAddress.compareTo("0.0.0.0"))
      {
         // This is the new interface for parallel codecs
         //mpTopologyGraph->startSendRtp(*(mediaConnection->mpRtpAudioSocket),
         //                          *(mediaConnection->mpRtcpAudioSocket),
         //                          connectionId,
         //                          audioCodec,
         //                          dtmfCodec);
         UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
         MpResourceTopology::replaceNumInName(outConnectionName, connectionId);

         MpRtpOutputAudioConnection::startSendRtp(*(mpTopologyGraph->getMsgQ()),
                                                  outConnectionName,
                                                  *(mediaConnection->mpRtpAudioSocket),
                                                  *(mediaConnection->mpRtcpAudioSocket),
                                                  audioCodec,
                                                  dtmfCodec);

         mediaConnection->mRtpAudioSending = TRUE;
         returnCode = OS_SUCCESS;
      }
   }
   return returnCode;
}


OsStatus CpTopologyGraphInterface::startRtpReceive(int connectionId,
                                                int numCodecs,
                                                SdpCodec* receiveCodecs[])
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;


   // Make sure we use the same payload types as the remote
   // side.  It's the friendly thing to do.
   if (mediaConnection->mpCodecFactory)
   {
         mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                           receiveCodecs);
   }

   if (mpTopologyGraph)
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

      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);

      if(mediaConnection->mRtpAudioReceiving)
      {
         // This is not supposed to be necessary and may be
         // causing an audible glitch when codecs are changed
         //mpTopologyGraph->stopReceiveRtp(connectionId);
      }


      MpRtpInputAudioConnection::startReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                 inConnectionName,
                                                 receiveCodecs,
                                                 numCodecs,
                                                 *(mediaConnection->mpRtpAudioSocket),
                                                 *(mediaConnection->mpRtcpAudioSocket));

      mediaConnection->mRtpAudioReceiving = TRUE;

      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus CpTopologyGraphInterface::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if (mpTopologyGraph && mediaConnection &&
       mediaConnection->mRtpAudioSending)
   {
      UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
      MpRtpOutputAudioConnection::stopSendRtp(*(mpTopologyGraph->getMsgQ()),
                                               outConnectionName);

      mediaConnection->mRtpAudioSending = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus CpTopologyGraphInterface::stopRtpReceive(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

#ifdef TEST_PRINT
   printf("CpTopologyGraphInterface::stopRtpReceive(%d) mpTopologyGraph %p mediaCOnnection: %p receiving: %s\n", 
       connectionId,
       mpTopologyGraph,
       mediaConnection, 
       mediaConnection && mediaConnection->mRtpAudioReceiving ? "TRUE" : "FALSE");
#endif

   if (mpTopologyGraph && mediaConnection &&
       mediaConnection->mRtpAudioReceiving)
   {
      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);

#ifdef TEST_PRINT
      printf("sending stopReceiveRtp message RTP socket: %p RTCP socket: %p\n",
             mediaConnection->mpRtpAudioSocket,
             mediaConnection->mpRtcpAudioSocket);
#endif

      MpRtpInputAudioConnection::stopReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                inConnectionName);
      
      mediaConnection->mRtpAudioReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus CpTopologyGraphInterface::deleteConnection(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   UtlInt matchConnectionId(connectionId);
   mMediaConnections.remove(&matchConnectionId) ;

   returnCode = doDeleteConnection(mediaConnection);

   delete mediaConnection ;

   return(returnCode);
}

OsStatus CpTopologyGraphInterface::doDeleteConnection(CpTopologyMediaConnection* mediaConnection)
{
   OsStatus returnCode = OS_NOT_FOUND;

   if(mediaConnection == NULL)
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                  "CpTopologyGraphInterface::doDeleteConnection mediaConnection is NULL!");
      return OS_NOT_FOUND;
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::deleteConnection deleting the connection %p",
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
                    "CpTopologyGraphInterface::doDeleteConnection NULL socket: mpRtpAudioSocket=0x%08x, mpRtpAudioSocket=0x%08x",
                    mediaConnection->mpRtpAudioSocket,
                    mediaConnection->mpRtcpAudioSocket);
#endif


   // The connections may be removed from the list, so we cannot use
   // the method that takes a connection id
   //returnCode = stopRtpSend(mediaConnection->getValue());
      UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(outConnectionName, mediaConnection->getValue());
      MpRtpOutputAudioConnection::stopSendRtp(*(mpTopologyGraph->getMsgQ()),
                                               outConnectionName);
   //returnCode = stopRtpReceive(mediaConnection->getValue());
      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, mediaConnection->getValue());
//      printf("doDeleteConnection sending stopReceiveRtp message RTP socket: %p RTCP socket: %p\n",
//             mediaConnection->mpRtpAudioSocket,
//             mediaConnection->mpRtcpAudioSocket);
      MpRtpInputAudioConnection::stopReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                inConnectionName);

   // Make sure the sockets are no longer used before further operations
   mpTopologyGraph->synchronize();

   if(mediaConnection->getValue() >= 0)
   {
       UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
       MpResourceTopology::replaceNumInName(inConnectionName, mediaConnection->getValue());
       UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
       MpResourceTopology::replaceNumInName(outConnectionName, mediaConnection->getValue());

       MpResource* inConnectionResource = NULL;
       returnCode = mpTopologyGraph->lookupResource(inConnectionName,
                                                    inConnectionResource);
       // If we failed to find the connection resource, sync. in case we
       // are deleting before the resources got linked into the flowgraph
       if(returnCode != OS_SUCCESS)
       {
           mpTopologyGraph->synchronize();
           returnCode = mpTopologyGraph->lookupResource(inConnectionName,
                                                        inConnectionResource);
       }
       assert(returnCode == OS_SUCCESS);
       assert(inConnectionResource);

       if(inConnectionResource)
       {
           mpTopologyGraph->removeResource(*inConnectionResource);
       }
       MpResource* outConnectionResource = NULL;
       returnCode = mpTopologyGraph->lookupResource(outConnectionName,
                                                    outConnectionResource);
       assert(returnCode == OS_SUCCESS);
       assert(outConnectionResource);

       if(outConnectionResource)
       {
           mpTopologyGraph->removeResource(*outConnectionResource);
       }
       mediaConnection->setValue(-1);
       mpTopologyGraph->synchronize();

       delete inConnectionResource;
       delete outConnectionResource;
   }

   mpFactoryImpl->releaseRtpPort(mediaConnection->mRtpAudioReceivePort) ;

   if(mediaConnection->mpRtpAudioSocket)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
      //printf(
                    "CpTopologyGraphInterface::doDeleteConnection deleting RTP socket: %p descriptor: %d",
                    mediaConnection->mpRtpAudioSocket,
                    mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtpAudioSocket;
      mediaConnection->mpRtpAudioSocket = NULL;
   }
   if(mediaConnection->mpRtcpAudioSocket)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
      //printf(
                    "deleting RTCP socket: %p descriptor: %d",
         mediaConnection->mpRtcpAudioSocket,
         mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtcpAudioSocket;
      mediaConnection->mpRtcpAudioSocket = NULL;
   }

   return(returnCode);
}


OsStatus CpTopologyGraphInterface::playAudio(const char* url,
                                          UtlBoolean repeat,
                                          UtlBoolean local,
                                          UtlBoolean remote,
                                          UtlBoolean mixWithMic,
                                          int downScaling,
                                          OsNotification *event)
{
    OsStatus stat = OS_NOT_FOUND;
    UtlString filename;
    if(url) filename.append(url);
    size_t filePrefixIdx = filename.index("file://");
    if(filePrefixIdx == 0) filename.remove(0, 6);

    if(mpTopologyGraph && !filename.isNull())
    {
       // Currently, this ignores "local", "mixWithMic" and "downScaling".
       stat = MprFromFile::playFile(DEFAULT_FROM_FILE_RESOURCE_NAME, 
          *mpTopologyGraph->getMsgQ(), filename, repeat, event);
    }

    if(stat != OS_SUCCESS)
    {
        osPrintf("Cannot play audio file: %s\n", filename.data());
    }

    return(stat);
}

OsStatus CpTopologyGraphInterface::playBuffer(char* buf,
                                           unsigned long bufSize,
                                           int type, 
                                           UtlBoolean repeat,
                                           UtlBoolean local,
                                           UtlBoolean remote, 
                                           OsProtectedEvent* pEvent,
                                           UtlBoolean mixWithMic,
                                           int downScaling)
{
    OsStatus stat = OS_NOT_FOUND;
    if(mpTopologyGraph && buf)
    {
       // Currently, this ignores "local/remote", "mixWithMic" and "downScaling".
       // I also don't know what to do with this blasted protected event.. sending NULL now.
       // If it was an OsNotification -- no problem..
       stat = MprFromFile::playBuffer(DEFAULT_FROM_FILE_RESOURCE_NAME, 
          *mpTopologyGraph->getMsgQ(), buf, bufSize, type, repeat, NULL);

       /*
         old stuff:
         int toneOptions=0;

         if (local)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_SPKR;
         }                  
         
         if(remote)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_NET;
         }

        // Start playing the audio file
        stat = mpTopologyGraph->playBuffer(buf, bufSize, type, repeat, toneOptions, NULL);
        */
    }

    if(stat != OS_SUCCESS)
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

    return(stat);
}


OsStatus CpTopologyGraphInterface::stopAudio()
{
   OsStatus stat = OS_FAILED;
   if(mpTopologyGraph != NULL)
   {
      stat = MprFromFile::stopFile(DEFAULT_FROM_FILE_RESOURCE_NAME, 
                                  *mpTopologyGraph->getMsgQ());
   }
   else
   {
      stat = OS_NOT_FOUND;
   }
   return stat;
}


OsStatus CpTopologyGraphInterface::playChannelAudio(int connectionId,
                                                 const char* url,
                                                 UtlBoolean repeat,
                                                 UtlBoolean local,
                                                 UtlBoolean remote,
                                                 UtlBoolean mixWithMic,
                                                 int downScaling,
                                                 OsNotification *event) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return playAudio(url, repeat, local, remote, mixWithMic, downScaling, event) ;
}


OsStatus CpTopologyGraphInterface::stopChannelAudio(int connectionId) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return stopAudio() ;
}


OsStatus CpTopologyGraphInterface::recordChannelAudio(int connectionId,
                                                   const char* szFile) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::stopRecordChannelAudio(int connectionId) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.


    return stopRecording() ;
}



OsStatus CpTopologyGraphInterface::createPlayer(MpStreamPlayer** ppPlayer, 
                                             const char* szStream, 
                                             int flags, 
                                             OsMsgQ *pMsgQ, 
                                             const char* szTarget)
{
   return OS_NOT_SUPPORTED;
}


OsStatus CpTopologyGraphInterface::destroyPlayer(MpStreamPlayer* pPlayer)
{
   return OS_NOT_SUPPORTED;   
}


OsStatus CpTopologyGraphInterface::createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer, 
                                                     OsMsgQ *pMsgQ, 
                                                     const char* szTarget)
{
   return OS_NOT_SUPPORTED;
}


OsStatus CpTopologyGraphInterface::destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
{
   return OS_NOT_SUPPORTED;   
}


OsStatus CpTopologyGraphInterface::createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                                  OsMsgQ *pMsgQ, 
                                                  const char* szTarget)
{
   return OS_NOT_SUPPORTED;
}


OsStatus CpTopologyGraphInterface::destroyQueuePlayer(MpStreamQueuePlayer* pPlayer)
{
   return OS_NOT_SUPPORTED;   
}


OsStatus CpTopologyGraphInterface::startTone(int toneId,
                                             UtlBoolean local,
                                             UtlBoolean remote)
{
   // TODO: deal with "local" and "remote"...
   OsStatus stat = OS_FAILED;
   if(mpTopologyGraph != NULL)
   {
      stat = MprToneGen::startTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                   *mpTopologyGraph->getMsgQ(), toneId);
   }
   else
   {
      stat = OS_NOT_FOUND;
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::stopTone()
{
   OsStatus stat = OS_FAILED;
   if(mpTopologyGraph != NULL)
   {
      stat = MprToneGen::stopTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                  *mpTopologyGraph->getMsgQ());
   }
   else
   {
      stat = OS_NOT_FOUND;
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::startChannelTone(int connectionId, int toneId, UtlBoolean local, UtlBoolean remote) 
{
    return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::stopChannelTone(int connectionId)
{
    return OS_NOT_SUPPORTED;
}


OsStatus CpTopologyGraphInterface::giveFocus()
{
    if(mpTopologyGraph)
    {
        // Set the flow graph to have the focus
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
        mediaTask->setFocus(mpTopologyGraph);
   }

   return OS_SUCCESS ;
}

OsStatus CpTopologyGraphInterface::defocus()
{
   if(mpTopologyGraph)
   {
      MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);

      // take focus away from the flow graph if it is focus
      if (mpTopologyGraph == (MpTopologyGraph*) mediaTask->getFocus())
      {
         mediaTask->setFocus(NULL);
      }
   }

   return OS_SUCCESS ;
}


// Limits the available codecs to only those within the designated limit.
void CpTopologyGraphInterface::setCodecCPULimit(int iLimit)
{
   mSupportedCodecs.setCodecCPULimit(iLimit) ;

   CpTopologyMediaConnection* mediaConnection = NULL;
   UtlDListIterator connectionIterator(mMediaConnections);
   while ((mediaConnection = (CpTopologyMediaConnection*) connectionIterator()))
   {
      mediaConnection->mpCodecFactory->setCodecCPULimit(iLimit) ;
   }
}

OsStatus CpTopologyGraphInterface::stopRecording()
{
   return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::recordMic(UtlString* pAudioBuffer)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprBufferRecorder::startRecording(
                DEFAULT_BUFFER_RECORDER_RESOURCE_NAME,
                *mpTopologyGraph->getMsgQ(), pAudioBuffer);
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::recordMic(int ms,
                                             int silenceLength,
                                             const char* fileName)
{
    return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::ezRecord(int ms, 
                                         int silenceLength, 
                                         const char* fileName, 
                                         double& duration,
                                         int& dtmfterm,
                                         OsProtectedEvent* ev)
{
   return OS_NOT_SUPPORTED;
}

void CpTopologyGraphInterface::addToneListener(OsNotification *pListener, int connectionId)
{
}

void CpTopologyGraphInterface::removeToneListener(int connectionId)
{
}

void CpTopologyGraphInterface::setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) 
{
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        if (pMediaConn->mIsMulticast && eType == CONTACT_AUTO)
        {
            pMediaConn->mContactType = CONTACT_LOCAL;
        }
        else
        {
            // Only CONTACT_LOCAL is allowed for multicast addresses.
            assert(!pMediaConn->mIsMulticast || eType == CONTACT_LOCAL);
            pMediaConn->mContactType = eType;
        }
    }
}

OsStatus CpTopologyGraphInterface::setAudioCodecBandwidth(int connectionId, int bandWidth) 
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpTopologyGraphInterface::rebuildCodecFactory(int connectionId, 
                                                    int audioBandwidth, 
                                                    int videoBandwidth, 
                                                    UtlString& videoCodec)
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpTopologyGraphInterface::setConnectionBitrate(int connectionId, int bitrate) 
{
    return OS_NOT_SUPPORTED ;
}


OsStatus CpTopologyGraphInterface::setConnectionFramerate(int connectionId, int framerate) 
{
    return OS_NOT_SUPPORTED ;
}


OsStatus CpTopologyGraphInterface::setSecurityAttributes(const void* security) 
{
    return OS_NOT_SUPPORTED ;
}

OsStatus CpTopologyGraphInterface::generateVoiceQualityReport(int         connectiond,
                                                           const char* callId,
                                                           UtlString&  report) 
{
	return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

int CpTopologyGraphInterface::getNextConnectionId()
{
    return(++mLastConnectionId);
}

void CpTopologyGraphInterface::setPremiumSound(UtlBoolean enabled)
{
}

// Calculate the current cost for our sending/receiving codecs
int CpTopologyGraphInterface::getCodecCPUCost()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   

   if (mMediaConnections.entries() > 0)
   {      
      CpTopologyMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpTopologyMediaConnection*) connectionIterator()))
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
int CpTopologyGraphInterface::getCodecCPULimit()
{   
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   
   int         iCodecs = 0 ;
   SdpCodec**  codecs ;


   //
   // If have connections; report what we have offered
   //
   if (mMediaConnections.entries() > 0)
   {      
      CpTopologyMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpTopologyMediaConnection*) connectionIterator()))
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
   OsMsgQ* CpTopologyGraphInterface::getMsgQ()
{
   return mpTopologyGraph->getMsgQ() ;
}

OsStatus CpTopologyGraphInterface::getPrimaryCodec(int connectionId, 
                                                UtlString& audioCodec,
                                                UtlString& videoCodec,
                                                int* audioPayloadType,
                                                int* videoPayloadType,
                                                bool& isEncrypted)
{
    UtlString codecType;
    CpTopologyMediaConnection* pConnection = getMediaConnection(connectionId);
    if (pConnection == NULL)
       return OS_NOT_FOUND;

    if (pConnection->mpAudioCodec != NULL)
    {
        pConnection->mpAudioCodec->getEncodingName(audioCodec);
        *audioPayloadType = pConnection->mpAudioCodec->getCodecPayloadFormat();
    }

    videoCodec="";
    *videoPayloadType=0;

   return OS_SUCCESS;
}

OsStatus CpTopologyGraphInterface::getVideoQuality(int& quality)
{
   quality = 0;
   return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::getVideoBitRate(int& bitRate)
{
   bitRate = 0;
   return OS_NOT_SUPPORTED;
}


OsStatus CpTopologyGraphInterface::getVideoFrameRate(int& frameRate)
{
   frameRate = 0;
   return OS_NOT_SUPPORTED;
}

/* ============================ INQUIRY =================================== */
UtlBoolean CpTopologyGraphInterface::isSendingRtpAudio(int connectionId)
{
   UtlBoolean sending = FALSE;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
       sending = mediaConnection->mRtpAudioSending;
   }
   else
   {
       osPrintf("CpTopologyGraphInterface::isSendingRtpAudio invalid connectionId: %d\n",
          connectionId);
   }

   return(sending);
}

UtlBoolean CpTopologyGraphInterface::isReceivingRtpAudio(int connectionId)
{
   UtlBoolean receiving = FALSE;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
      receiving = mediaConnection->mRtpAudioReceiving;
   }
   else
   {
       osPrintf("CpTopologyGraphInterface::isReceivingRtpAudio invalid connectionId: %d\n",
          connectionId);
   }
   return(receiving);
}

UtlBoolean CpTopologyGraphInterface::isSendingRtpVideo(int connectionId)
{
   UtlBoolean sending = FALSE;

   return(sending);
}

UtlBoolean CpTopologyGraphInterface::isReceivingRtpVideo(int connectionId)
{
   UtlBoolean receiving = FALSE;

   return(receiving);
}


UtlBoolean CpTopologyGraphInterface::isDestinationSet(int connectionId)
{
    UtlBoolean isSet = FALSE;
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

    if(mediaConnection)
    {
        isSet = mediaConnection->mDestinationSet;
    }
    else
    {
       osPrintf("CpTopologyGraphInterface::isDestinationSet invalid connectionId: %d\n",
          connectionId);
    }
    return(isSet);
}

UtlBoolean CpTopologyGraphInterface::canAddParty() 
{
    return (mMediaConnections.entries() < 4) ;
}


UtlBoolean CpTopologyGraphInterface::isVideoInitialized(int connectionId)
{
   return false ;
}

UtlBoolean CpTopologyGraphInterface::isAudioInitialized(int connectionId) 
{
    return true ;
}

UtlBoolean CpTopologyGraphInterface::isAudioAvailable() 
{
    return true ;
}

OsStatus CpTopologyGraphInterface::setVideoWindowDisplay(const void* hWnd)
{
   return OS_NOT_YET_IMPLEMENTED;
}

const void* CpTopologyGraphInterface::getVideoWindowDisplay()
{
   return NULL;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */


UtlBoolean CpTopologyGraphInterface::getLocalAddresses(int connectionId,
                                              UtlString& hostIp,
                                              int& rtpAudioPort,
                                              int& rtcpAudioPort,
                                              int& rtpVideoPort,
                                              int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);    

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

UtlBoolean CpTopologyGraphInterface::getNatedAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
    UtlString host ;
    int port ;
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);

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

                    bRC = FALSE ;
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

UtlBoolean CpTopologyGraphInterface::getRelayAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
    UtlString host ;
    int port ;
    CpTopologyMediaConnection* pMediaConn = getMediaConnection(connectionId);

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        assert(!pMediaConn->mIsMulticast);
        if (pMediaConn->mIsMulticast)
        {
           return FALSE;
        }

        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (((OsNatDatagramSocket*)pMediaConn->mpRtpAudioSocket)->getRelayIp(&host, &port))
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
                    if (((OsNatDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->getRelayIp(&host, &port))
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
            if ((OsNatDatagramSocket*)pMediaConn->mpRtpVideoSocket)->getRelayIp(&host, &port))
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
                    if ((OsNatDatagramSocket*)pMediaConn->mpRtcpVideoSocket)->getRelayIp(&host, &port))
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

OsStatus CpTopologyGraphInterface::addLocalContacts(int connectionId, 
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


OsStatus CpTopologyGraphInterface::addNatedContacts(int connectionId, 
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


OsStatus CpTopologyGraphInterface::addRelayContacts(int connectionId, 
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


void CpTopologyGraphInterface::applyAlternateDestinations(int connectionId) 
{
    UtlString destAddress ;
    int       destPort ;

    CpTopologyMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if (pMediaConnection)
    {
        assert(!pMediaConnection->mIsMulticast);
        if (pMediaConnection->mIsMulticast)
        {
           return;
        }

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
           OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpAudioSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort);
                pMediaConnection->mRtpSendHostAddress = destAddress;
                pMediaConnection->mRtpAudioSendHostPort = destPort;
            }
        }

        // Connect RTCP Audio Socket
        if (pMediaConnection->mpRtcpAudioSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpAudioSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpAudioSendHostPort = destPort;                
            }            
        }

        // TODO:: Enable/Disable RTCP

#ifdef VIDEO
        // Connect RTP Video Socket
        if (pMediaConnection->mpRtpVideoSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpVideoSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtpVideoSendHostPort = destPort;
            }            
        }

        // Connect RTCP Video Socket
        if (pMediaConnection->mpRtcpVideoSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpVideoSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort) ;                
                pMediaConnection->mRtcpVideoSendHostPort = destPort;
            }            
        }

        // TODO:: Enable/Disable RTCP
#endif
    }
}

OsStatus CpTopologyGraphInterface::setMediaProperty(const UtlString& propertyName,
                                                 const UtlString& propertyValue)
{
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpTopologyGraphInterface::setMediaProperty %p propertyName=\"%s\" propertyValue=\"%s\"",
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

OsStatus CpTopologyGraphInterface::getMediaProperty(const UtlString& propertyName,
                                                 UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_FOUND;
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpTopologyGraphInterface::getMediaProperty %p propertyName=\"%s\"",
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

OsStatus CpTopologyGraphInterface::setMediaProperty(int connectionId,
                                                 const UtlString& propertyName,
                                                 const UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_YET_IMPLEMENTED;
    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpTopologyGraphInterface::setMediaProperty %p connectionId=%d propertyName=\"%s\" propertyValue=\"%s\"",
        this, connectionId, propertyName.data(), propertyValue.data());

    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
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

OsStatus CpTopologyGraphInterface::getMediaProperty(int connectionId,
                                                 const UtlString& propertyName,
                                                 UtlString& propertyValue)
{
    OsStatus returnCode = OS_NOT_FOUND;
    propertyValue = "";

    OsSysLog::add(FAC_CP, PRI_ERR, 
        "CpTopologyGraphInterface::getMediaProperty %p connectionId=%d propertyName=\"%s\"",
        this, connectionId, propertyName.data());

    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
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

OsStatus CpTopologyGraphInterface::createRtpSocketPair(UtlString localAddress,
                                                       int localPort,
                                                       SIPX_CONTACT_TYPE contactType,
                                                       OsDatagramSocket* &rtpSocket,
                                                       OsDatagramSocket* &rtcpSocket)
{
   int firstRtpPort;
   bool localPortGiven = (localPort != 0); // Does user specified the local port?
   UtlBoolean isMulticast = OsSocket::isMcastAddr(localAddress);

   if (!localPortGiven)
   {
      mpFactoryImpl->getNextRtpPort(localPort);
      firstRtpPort = localPort;
   }

   if (isMulticast)
   {
        rtpSocket = new OsMulticastSocket(localPort, localAddress,
                                          localPort, localAddress);
        rtcpSocket = new OsMulticastSocket(
                              localPort == 0 ? 0 : localPort + 1, localAddress,
                              localPort == 0 ? 0 : localPort + 1, localAddress);
   }
   else
   {
       rtpSocket = new OsNatDatagramSocket(0, NULL, localPort, localAddress, NULL);
       ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

       rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                            localAddress, NULL);
       ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
   }

   // Validate local port is not auto-selecting.
   if (localPort != 0 && !localPortGiven)
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
            if (isMulticast)
            {
               OsMulticastSocket* rtpSocket = new OsMulticastSocket(
                        localPort, localAddress,
                        localPort, localAddress);
               OsMulticastSocket* rtcpSocket = new OsMulticastSocket(
                        localPort == 0 ? 0 : localPort + 1, localAddress,
                        localPort == 0 ? 0 : localPort + 1, localAddress);
            }
            else
            {
               rtpSocket = new OsNatDatagramSocket(0, NULL, localPort, localAddress, NULL);
               ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

               rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                                   localAddress, NULL);
               ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
            }
      }
   }

   // Did our sockets get created OK?
   if (!rtpSocket->isOk() || !rtcpSocket->isOk())
   {
       delete rtpSocket;
       delete rtcpSocket;
       return OS_NETWORK_UNAVAILABLE;
   }

   if (isMulticast)
   {
       // Set multicast options
       const unsigned char MC_HOP_COUNT = 8;
       ((OsMulticastSocket*)rtpSocket)->setHopCount(MC_HOP_COUNT);
       ((OsMulticastSocket*)rtcpSocket)->setHopCount(MC_HOP_COUNT);
       ((OsMulticastSocket*)rtpSocket)->setLoopback(false);
       ((OsMulticastSocket*)rtcpSocket)->setLoopback(false);
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
      
#ifndef WIN32 // [
            // Under Windows this options are supported under Win2000 only and
            // are not recommended to use.
      setsockopt (sRtp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
      setsockopt (sRtcp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
#else  // WIN32 ][
      // TODO:: Implement QoS  request under Windows.
#endif // WIN32 ]
   }

   if (!isMulticast)
   {
       // Enable Stun if we have a stun server and either non-local contact type or 
       // ICE is enabled.
       if ((mStunServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mEnableIce))
       {
          ((OsNatDatagramSocket*)rtpSocket)->enableStun(mStunServer, mStunPort,
                                                        mStunRefreshPeriodSecs, 0, false) ;
       }

       // Enable Turn if we have a stun server and either non-local contact type or 
       // ICE is enabled.
       if ((mTurnServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mEnableIce))
       {
          ((OsNatDatagramSocket*)rtpSocket)->enableTurn(mTurnServer, mTurnPort, 
                   mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
       }

       // Enable Stun if we have a stun server and either non-local contact type or 
       // ICE is enabled.
       if ((mStunServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mEnableIce))
       {
          ((OsNatDatagramSocket*)rtcpSocket)->enableStun(mStunServer, mStunPort,
                                                         mStunRefreshPeriodSecs, 0, false) ;
       }

       // Enable Turn if we have a stun server and either non-local contact type or 
       // ICE is enabled.
       if ((mTurnServer.length() != 0) && ((contactType != CONTACT_LOCAL) || mEnableIce))
       {
          ((OsNatDatagramSocket*)rtcpSocket)->enableTurn(mTurnServer, mTurnPort, 
                   mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;
       }
   }

   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

