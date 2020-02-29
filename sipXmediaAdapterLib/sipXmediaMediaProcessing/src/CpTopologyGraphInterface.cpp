// 
// Copyright (C) 2006-2020 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
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
#include <utl/CircularBuffer.h>
#include <os/OsDatagramSocket.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsMulticastSocket.h>
#include <os/OsProtectEventMgr.h>
#include <os/OsSysLog.h>
#include <os/OsStatus.h>
#include <mp/MpTopologyGraph.h>
#include <mp/MpResourceTopology.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprToneGen.h>
#include <mp/MprFromFile.h>
#include <mp/MprDecode.h>
#include <mp/MprDelay.h>
#include <mp/MpRtpInputConnection.h>
#include <mp/MpRtpOutputConnection.h>
#include <mp/MprEncode.h>
#include <mp/MprBridge.h>
#include <mp/MprRecorder.h>
#include <mp/MprFromInputDevice.h>
#include <mp/dtmflib.h>
#include <mp/MprFromNet.h>
#include <mp/MpMediaTask.h>
#include <mp/MpCodecFactory.h>
#include <mp/MprToNet.h>
#include <CpTopologyGraphInterface.h>
#include <CpTopologyGraphFactoryImpl.h>
#include <TypeConverter.h>

#if MAXIMUM_RECORDER_CHANNELS > 1
#  include <mp/MpResourceFactory.h>
#  include <mp/MprBridgeConstructor.h>
#endif

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
    , mpResourceTopology(NULL)
    , mRtpAudioSendHostAddress()
    , mAudioDestinationSet(FALSE)
    , mIsMulticast(FALSE)
    , mIsCustomSockets(FALSE)
    , mNumRtpStreams(0)
    , mpRtpAudioSocket(NULL)
    , mpRtcpAudioSocket(NULL)
#ifdef VIDEO
    , mRtpVideoSendHostAddress()
    , mVideoDestinationSet(FALSE)
    , mpRtpVideoSocket(NULL)
    , mpRtcpVideoSocket(NULL)
    , mRtpVideoSendHostPort(0)
    , mRtcpVideoSendHostPort(0)
    , mRtpVideoReceivePort(0)
    , mRtcpVideoReceivePort(0)
    , mVideoPassThroughEnabled(FALSE)
    , mRtpVideoSending(FALSE)
    , mRtpVideoReceiving(FALSE)
    , mpVideoCodec(NULL)
#endif
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
        if(!mIsCustomSockets && mpRtpAudioSocket)
        {

//#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting audio RTP socket: %p descriptor: %d",
                mpRtpAudioSocket, mpRtpAudioSocket->getSocketDescriptor());
//#endif
            delete mpRtpAudioSocket;
            mpRtpAudioSocket = NULL;
        }

        if(!mIsCustomSockets && mpRtcpAudioSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting audio RTCP socket: %p descriptor: %d",
                mpRtcpAudioSocket, mpRtcpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtcpAudioSocket;
            mpRtcpAudioSocket = NULL;
        }

#ifdef VIDEO
        if(!mIsCustomSockets && mpRtpVideoSocket)
        {
//#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting video RTP socket: %p descriptor: %d",
                mpRtpVideoSocket, mpRtpVideoSocket->getSocketDescriptor());
//#endif
            delete mpRtpVideoSocket;
            mpRtpVideoSocket = NULL;
        }

        if(!mIsCustomSockets && mpRtcpVideoSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
            //printf(
                "~CpTopologyMediaConnection deleting video RTCP socket: %p descriptor: %d",
                mpRtcpVideoSocket, mpRtcpVideoSocket->getSocketDescriptor());
#endif
            delete mpRtcpVideoSocket;
            mpRtcpVideoSocket = NULL;
        }
#endif

        if(mpCodecFactory)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~CpTopologyMediaConnection deleting mpCodecFactory %p",
                mpCodecFactory);
            delete mpCodecFactory;
            mpCodecFactory = NULL;
        }

        if (mpAudioCodec)
        {
            delete mpAudioCodec;
            mpAudioCodec = NULL; 
        }              

#ifdef VIDEO
        if(mpVideoCodec)
        {
            delete mpVideoCodec;
            mpVideoCodec = NULL;
        }
#endif

        mConnectionProperties.destroyAll();
    }

    MpResourceTopology *mpResourceTopology;
    UtlString mRtpAudioSendHostAddress;
    UtlBoolean mAudioDestinationSet;
    UtlBoolean mIsMulticast;
    UtlBoolean mIsCustomSockets;
    int mNumRtpStreams;
    OsSocket* mpRtpAudioSocket;
    OsSocket* mpRtcpAudioSocket;

#ifdef VIDEO
    UtlString mRtpVideoSendHostAddress;
    UtlBoolean mVideoDestinationSet;
    OsSocket* mpRtpVideoSocket;
    OsSocket* mpRtcpVideoSocket;
    int mRtpVideoSendHostPort;
    int mRtcpVideoSendHostPort;
    int mRtpVideoReceivePort;
    int mRtcpVideoReceivePort;
    UtlBoolean mVideoPassThroughEnabled;
    UtlBoolean mRtpVideoSending;
    UtlBoolean mRtpVideoReceiving;
    SdpCodec* mpVideoCodec;
#endif

    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    UtlBoolean mRtpAudioSending;
    UtlBoolean mRtpAudioReceiving;
    SdpCodec* mpAudioCodec;
    SdpCodecList* mpCodecFactory;
    SIPX_CONTACT_TYPE mContactType;
    UtlString mLocalAddress;
    UtlBoolean mbAlternateDestinations;
    UtlHashMap mConnectionProperties;
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphInterface::CpTopologyGraphInterface(CpTopologyGraphFactoryImpl* pFactoryImpl,
                                                   uint32_t samplesPerFrame,
                                                   uint32_t samplesPerSec, 
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
                                                   UtlBoolean enableIce,
                                                   OsMsgDispatcher* pDispatcher,
                                                   MpInputDeviceManager* pInputDeviceManager,
                                                   MpInputDeviceHandle inputDeviceHandle,
                                                   UtlBoolean inputDeviceAlreadyEnabled,
                                                   MpOutputDeviceManager* pOutputDeviceManager,
                                                   MpOutputDeviceHandle outputDeviceHandle
                                                  )
: CpMediaInterface(pFactoryImpl)
, mpInputDeviceManager(pInputDeviceManager)
, mInputDeviceHandle(inputDeviceHandle)
, mpOutputDeviceManager(pOutputDeviceManager)
, mOutputDeviceHandle(outputDeviceHandle)
{
    mLastConnectionId = 0;
    int rtpPoolSize = MpMisc.RtpPool->getNumBlocks();
    int rtpBufs = MpMisc.RtpPool->getFreeBufferCount();
    int udpPoolSize = MpMisc.UdpPool->getNumBlocks();
    int udpBufs = MpMisc.UdpPool->getFreeBufferCount();
    int audioPoolSize = MpMisc.RawAudioPool->getNumBlocks();
    int audioBufs  = MpMisc.RawAudioPool->getFreeBufferCount();


   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new CpMediaInterface %p",
                 this);

   OsMsgDispatcher *pTmpDispatcherPtr;
   mTranslatorDispatcher.setDispatcher(pDispatcher);
   if(pDispatcher != NULL)
   {
      pTmpDispatcherPtr = &mTranslatorDispatcher;
   }
   else
   {
      pTmpDispatcherPtr = NULL;
   }

   MpResourceTopology* topology = pFactoryImpl->getInitialResourceTopology();
#ifdef DUMP_TOPOLGY
   UtlString topoDump;
   topology->dumpResources(topoDump);
   printf("MpResourceTopology resources:\n%s\n", topoDump.data());
   topology->dumpConnections(topoDump);
   printf("MpResourceTopology connections:\n%s\n", topoDump.data());
#endif

   MpResourceFactory* factory = pFactoryImpl->getResourceFactory();

   mpTopologyGraph = new MpTopologyGraph(samplesPerFrame,
                                         samplesPerSec,
                                         *topology,
                                         *factory,
                                         pTmpDispatcherPtr);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new MpTopologyGraph %p",
                 mpTopologyGraph);
   OsSysLog::add(FAC_CP, PRI_DEBUG,
        "available pooled buffers udp: %d/%d rtp: %d/%d audio: %d/%d",
        udpBufs, udpPoolSize, rtpBufs, rtpPoolSize, audioBufs, audioPoolSize);
  
   // Temporary hack while we determine VAD performance issue
#ifdef ANDROID
   OsMsgQ* flowgraphQueue = mpTopologyGraph->getMsgQ();
   MpResource::disable(DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX, *flowgraphQueue);
#endif
 
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
       mSupportedCodecs.addCodecs(numCodecs, sdpCodecArray);

       UtlString codecsListString;
       mSupportedCodecs.toString(codecsListString);

       OsSysLog::add(FAC_CP, PRI_INFO, "CpTopologyGraphInterface::CpTopologyGraphInterface %d codec(s) added:\n%s",
                     numCodecs, codecsListString.data());
       // Don't want to bind any unset payload types until last minute so
       // that we avoid creating payload ID conflicts if we get payload IDs from
       // the other side.
       //mSupportedCodecs.bindPayloadTypes();
   }
   else
   {
       MpCodecFactory *pCodecFactory = MpCodecFactory::getMpCodecFactory();
       pCodecFactory->addCodecsToList(mSupportedCodecs);

       if (OsSysLog::willLog(FAC_CP, PRI_INFO))
       {
          UtlString codecsListString;
          mSupportedCodecs.toString(codecsListString);
          OsSysLog::add(FAC_CP, PRI_INFO, "CpTopologyGraphInterface::CpTopologyGraphInterface hard-coded codec factory %s ...",
                        codecsListString.data());
       }
   }

   mExpeditedIpTos = expeditedIpTos;

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "CpTopologyGraphInterface::CpTopologyGraphInterface using input device: %d already enabled: %s",
                 mInputDeviceHandle, inputDeviceAlreadyEnabled ? "true" : "false");

   if (!inputDeviceAlreadyEnabled && mpInputDeviceManager != NULL
      && mInputDeviceHandle > -1)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "CpTopologyGraphInterface::CpTopologyGraphInterface enabling input device: %d",
                    mInputDeviceHandle);
      mpInputDeviceManager->enableDevice(mInputDeviceHandle);
   }
}


// Destructor
CpTopologyGraphInterface::~CpTopologyGraphInterface()
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::~CpTopologyGraphInterface deleting the CpMediaInterface %p",
                 this);

    CpTopologyMediaConnection* mediaConnection = NULL;
    while ((mediaConnection = (CpTopologyMediaConnection*) mMediaConnections.get()))
    {
        deleteMediaConnection(mediaConnection);
        mediaConnection = NULL;
    }

    if(mpTopologyGraph)
    {
        // Free up the resources used by tone generation ASAP
        stopTone();

        // Stop the net in/out stuff before the sockets are deleted
        //mpMediaFlowGraph->stopReceiveRtp();
        //mpMediaFlowGraph->stopSendRtp();

        //MpMediaTask* mediaTask = MpMediaTask::getMediaTask();

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

    // Don't want to do this as multiple flowgraphs could be using the same input device
#if 0
    if (mpInputDeviceManager != NULL && mInputDeviceHandle > -1)
    {
       mpInputDeviceManager->disableDevice(mInputDeviceHandle);
    }
#endif

    // Delete the properties and their values
    mInterfaceProperties.destroyAll();

    int rtpPoolSize = MpMisc.RtpPool->getNumBlocks();
    int rtpBufs = MpMisc.RtpPool->getFreeBufferCount();
    int udpPoolSize = MpMisc.UdpPool->getNumBlocks();
    int udpBufs = MpMisc.UdpPool->getFreeBufferCount();
    int audioPoolSize = MpMisc.RawAudioPool->getNumBlocks();
    int audioBufs  = MpMisc.RawAudioPool->getFreeBufferCount();

    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "available pooled buffers udp: %d/%d rtp: %d/%d audio: %d/%d",
        udpBufs, udpPoolSize, rtpBufs, rtpPoolSize, audioBufs, audioPoolSize);
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
                                                    const RtpTransportOptions rtpTransportOptions)
{
   OsStatus retValue = OS_SUCCESS;
   UtlString localAddress;
   UtlBoolean isMulticast;

   // Determine local address
   if (szLocalAddress && strlen(szLocalAddress))
   {
      localAddress = szLocalAddress ;
   }
   else
   {
      localAddress = mLocalAddress ;
   }

   // Is multicast stream requested?
   isMulticast = OsSocket::isMcastAddr(localAddress);

   // Create connection.
   CpTopologyMediaConnection* mediaConnection = createMediaConnection(connectionId,
                                                                      isMulticast);
   if (connectionId == -1)
   {
      return OS_LIMIT_REACHED;
   }

   // Save connection properties.
   mediaConnection->mLocalAddress = localAddress;
   mediaConnection->mIsMulticast = isMulticast;
   if (mediaConnection->mIsMulticast)
   {
      mediaConnection->mContactType = CONTACT_LOCAL;
   }

   // Create the sockets for the audio stream
   retValue = createRtpSocketPair(mediaConnection->mLocalAddress, localPort,
                                  mediaConnection->mIsMulticast,
                                  mediaConnection->mContactType,
                                  mediaConnection->mpRtpAudioSocket, 
                                  mediaConnection->mpRtcpAudioSocket);
   if (retValue != OS_SUCCESS)
   {
      deleteConnection(connectionId);
      connectionId = -1;
      return retValue;
   }

   // Store audio stream settings
   mediaConnection->mRtpAudioReceivePort = mediaConnection->mpRtpAudioSocket->getLocalHostPort() ;
   mediaConnection->mRtcpAudioReceivePort = mediaConnection->mpRtcpAudioSocket->getLocalHostPort() ;

   OsSysLog::add(FAC_CP, PRI_DEBUG, 
      "CpTopologyGraphInterface::createConnection creating a new audio RTP socket: %p descriptor: %d",
      mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
      "CpTopologyGraphInterface::createConnection creating a new audio RTCP socket: %p descriptor: %d",
      mediaConnection->mpRtcpAudioSocket, mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());


#ifdef VIDEO
   // TODO: need to figure out conditional on when to create video sockets
   // Create the sockets for the video stream
   retValue = createRtpSocketPair(mediaConnection->mLocalAddress, localPort != 0 ? localPort + 2 : localPort,
                                  mediaConnection->mIsMulticast,
                                  mediaConnection->mContactType,
                                  mediaConnection->mpRtpVideoSocket, 
                                  mediaConnection->mpRtcpVideoSocket);
   if (retValue != OS_SUCCESS)
   {
       OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphInterface::createConnection could not create RTP socket pair for video. status: %d",
           retValue);
   }
   else
   {
      // Store video stream settings
      mediaConnection->mRtpVideoReceivePort = mediaConnection->mpRtpVideoSocket->getLocalHostPort() ;
      mediaConnection->mRtcpVideoReceivePort = mediaConnection->mpRtcpVideoSocket->getLocalHostPort() ;

      OsSysLog::add(FAC_CP, PRI_DEBUG, 
         "CpTopologyGraphInterface::createConnection creating a new video RTP socket: %p descriptor: %d",
         mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtpVideoSocket->getSocketDescriptor());
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
         "CpTopologyGraphInterface::createConnection creating a new video RTCP socket: %p descriptor: %d",
         mediaConnection->mpRtcpVideoSocket, mediaConnection->mpRtcpVideoSocket->getSocketDescriptor());

      UtlString videoInConnectionName(DEFAULT_VIDEO_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(videoInConnectionName, connectionId);

      // Not sure why the audio sockets are set upon creation.  Setting the sockets
      // causes them to be read.  It would seem either they should be read all the time and dropped
      // on the floor or they should only be read when we are receiving.  We currently stop reading
      // when we stopRtpReceive.
      //
      // construct and send message to set sockets on video in rtp connection
      //MprFromNet::setSockets(videoInConnectionName, *(mpTopologyGraph->getMsgQ()),
      //    mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtcpVideoSocket);
   }


#endif

   // Fence between calls to synchronous flowgraph methods and asynchronous.
   // Also this is required to ensure that the connection has completed being
   // created before exiting this function. This will allow API's such as
   // getConnectionPortOnBridge() to be used immediately upon return from this
   // function.
   mpTopologyGraph->synchronize("CpTopologyGraphInterface::createConnection");

   // Provide sockets to input connection
   UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
   MpRtpInputConnection *pInputConnection;
   OsStatus tmpStat = mpTopologyGraph->lookupResource(inConnectionName,
                                                      (MpResource*&)pInputConnection);
   assert(tmpStat == OS_SUCCESS);
   pInputConnection->setSockets(*(mediaConnection->mpRtpAudioSocket),
                                *(mediaConnection->mpRtcpAudioSocket));

   return retValue;
}

OsStatus CpTopologyGraphInterface::createConnection(int& connectionId,
                                                    OsSocket* rtpSocket,
                                                    OsSocket* rtcpSocket,
                                                    UtlBoolean isMulticast)
{
   OsStatus retValue = OS_SUCCESS;
   CpTopologyMediaConnection* mediaConnection = createMediaConnection(connectionId,
                                                                      isMulticast);

   if (connectionId == -1)
   {
      return OS_LIMIT_REACHED;
   }

   // Assign the passed in sockets to the mediaConnection
   mediaConnection->mpRtpAudioSocket = rtpSocket;
   mediaConnection->mpRtcpAudioSocket = rtcpSocket;
   mediaConnection->mAudioDestinationSet = TRUE;
   mediaConnection->mRtpAudioSendHostAddress = "127.0.0.1";  // dummy address so that startRtpSend will work
   mediaConnection->mContactType = CONTACT_LOCAL;
   mediaConnection->mIsCustomSockets = TRUE;

   // Fence between calls to synchronous flowgraph methods and asynchronous.
   // Also this is required to ensure that the connection has completed being
   // created before exiting this function. This will allow API's such as
   // getConnectionPortOnBridge() to be used immediately upon return from this
   // function.
   mpTopologyGraph->synchronize("CpTopologyGraphInterface::createConnection sockets");

   // Provide sockets to input connection
   UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
   MpRtpInputConnection *pInputConnection;
   OsStatus tmpStat = mpTopologyGraph->lookupResource(inConnectionName,
                                                      (MpResource*&)pInputConnection);
   assert(tmpStat == OS_SUCCESS);
   pInputConnection->setSockets(*(mediaConnection->mpRtpAudioSocket),
                                *(mediaConnection->mpRtcpAudioSocket));

   return retValue;
}

OsStatus CpTopologyGraphInterface::setPlcMethod(int connectionId,
                                                const UtlString &methodName)
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;

   if (mpTopologyGraph)
   {
      UtlString decoderName(DEFAULT_DECODE_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(decoderName, connectionId);
      decoderName.append(STREAM_NAME_SUFFIX);

      // Loop over all decoders and set PLC method.
      for (int i=0; i<mediaConnection->mNumRtpStreams; i++)
      {
         UtlString tmpName(decoderName);
         MpResourceTopology::replaceNumInName(tmpName, i);
         MprDecode::setPlc(tmpName,
                           *(mpTopologyGraph->getMsgQ()),
                           methodName);
      }

      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsMsgDispatcher*  
CpTopologyGraphInterface::setNotificationDispatcher(OsMsgDispatcher* pNotificationDispatcher)
{
   // If there is no topology graph, return NULL.
   if(mpTopologyGraph == NULL)
      return NULL;

   // Give the pointer to the dispatcher that is given to us to the translator
   // dispatcher, so that messages can be translated from mediaLib notifications
   // to abstract mediaAdapter notifications when being sent to dispatcher.
   OsMsgDispatcher* oldDispatcher = 
      mTranslatorDispatcher.setDispatcher(pNotificationDispatcher);

   // Now, if a non-NULL notification dispatcher was set, then
   // set the translator dispatcher on the flowgraph.  
   if(pNotificationDispatcher != NULL)
   {
      // This will be a redundant call if MI setNotificationDispatcher is 
      // called more than once with a valid dispatcher, but isn't very expensive.
      mpTopologyGraph->setNotificationDispatcher(&mTranslatorDispatcher);
   }
   else
   {
      // Otherwise, remove any dispatchers from the flowgraph, so as not to waste
      // posting notifications to the translator dispatcher when it won't
      // be doing anything with them.
      mpTopologyGraph->setNotificationDispatcher(NULL);
   }
   return oldDispatcher;
}

OsStatus 
CpTopologyGraphInterface::setNotificationsEnabled(bool enabled, 
                                                  const UtlString& resourceName)
{
   return mpTopologyGraph ? 
      mpTopologyGraph->setNotificationsEnabled(enabled, resourceName) :
      OS_FAILED;
}  

int CpTopologyGraphInterface::getNumBridgePorts()
{
   UtlString bridgeName(DEFAULT_BRIDGE_RESOURCE_NAME);
   MpResource *pBridge;
   if (mpTopologyGraph->lookupResource(bridgeName, pBridge) != OS_SUCCESS)
   {
      assert(!"Can't find Bride in the flowgraph!");
      return -1;
   }
   return pBridge->numInputs();
}

OsStatus CpTopologyGraphInterface::getConnectionPortOnBridge(int connectionId,
                                                             int streamNum,
                                                             int& portOnBridge)
{
   assert(connectionId >=0);

   UtlString connectionName(VIRTUAL_NAME_RTP_STREAM_OUTPUT);
   MpResourceTopology::replaceNumInName(connectionName, connectionId);
   connectionName.appendFormat(STREAM_NAME_SUFFIX, streamNum);

   return getResourceInputPortOnBridge(connectionName, 0, portOnBridge);
}

OsStatus CpTopologyGraphInterface::getResourceInputPortOnBridge(const UtlString &resourceName,
                                                                int outputPortIdx,
                                                                int& portOnBridge)
{
   MpResource* pResource = NULL;
   int realPortIndex;

   OsStatus retStatus = mpTopologyGraph->lookupOutput(resourceName,
                                                      outputPortIdx,
                                                      pResource,
                                                      realPortIndex);
   if(OS_SUCCESS == retStatus)
   {
      MpResource* doNotTouchResource = NULL;

      pResource->getOutputInfo(realPortIndex,
                               doNotTouchResource, // not safe to access
                               portOnBridge);
   }
   else
   {
      portOnBridge = -1;
   }
   return retStatus;
}

OsStatus CpTopologyGraphInterface::getResourceOutputPortOnBridge(const UtlString &resourceName,
                                                                 int inputPortIdx,
                                                                 int& portOnBridge)
{
   MpResource* pResource = NULL;
   int realPortIndex;

   OsStatus retStatus = mpTopologyGraph->lookupOutput(resourceName,
                                                      inputPortIdx,
                                                      pResource,
                                                      realPortIndex);
   if(OS_SUCCESS == retStatus)
   {
      MpResource* doNotTouchResource = NULL;

      pResource->getInputInfo(realPortIndex,
                              doNotTouchResource, // not safe to access
                              portOnBridge);
   }
   else
   {
      portOnBridge = -1;
   }
   return retStatus;
}

OsStatus CpTopologyGraphInterface::getCapabilities(int connectionId,
                                                UtlString& rtpHostAddress,
                                                int& rtpAudioPort,
                                                int& rtcpAudioPort,
                                                int& rtpVideoPort,
                                                int& rtcpVideoPort,
                                                SdpCodecList& supportedCodecs,
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
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::getCapabilities mContactType: %d",
            pMediaConn->mContactType);

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
            else if(pMediaConn->mContactType == CONTACT_CONFIG)
            {
                UtlString dummyHostIp;
                rtpHostAddress = mConfiguredIpAddress;
                getLocalAddresses(connectionId, dummyHostIp, rtpAudioPort,
                        rtcpAudioPort, rtpVideoPort, rtcpVideoPort);
            }
            else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
            {
                assert(!pMediaConn->mIsMulticast);
                if (!((OsDatagramSocket*)pMediaConn->mpRtpAudioSocket)->getMappedIp(&rtpHostAddress, &rtpAudioPort))
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
                    // but its better to ignore this and have no RTCP than take the call down.
                    //assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphInterface::getCapabilities mapped RTP IP: %s differs from mapped RTCP IP: %s",
                        rtpHostAddress.data(), tempHostAddress.data());
                }
            }
            else if (pMediaConn->mContactType == CONTACT_CONFIG)
            {
                // Do nothing as it was taken care of above for RTP.
            }
            else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
            {
                UtlString tempHostAddress;
                assert(!pMediaConn->mIsMulticast);
                if (!((OsDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->getMappedIp(&tempHostAddress, &rtcpAudioPort))
                {
                    rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                }
                else
                {
                    // External address should match that of Audio RTP
                    // but its better to ignore this and have no RTCP than take the call down.
                    //assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                    OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphInterface::getCapabilities mapped RTP IP: %s differs from mapped RTCP IP: %s",
                        rtpHostAddress.data(), tempHostAddress.data());
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
                                                  SdpCodecList& supportedCodecs,
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
            case CONTACT_CONFIG:
                if(nActualAddresses < nMaxAddresses)
                {
                    UtlString dummyHostIp;
                    rtpHostAddresses[nActualAddresses] = mConfiguredIpAddress;
                    getLocalAddresses(connectionId, dummyHostIp, rtpAudioPorts[nActualAddresses],
                        rtcpAudioPorts[nActualAddresses], rtpVideoPorts[nActualAddresses], rtcpVideoPorts[nActualAddresses]);
                    nActualAddresses++;
                }
                break;

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

OsStatus CpTopologyGraphInterface::setMediaPassThrough(int connectionId,
                                                       MEDIA_STREAM_TYPE mediaType,
                                                       int mediaTypeStreamIndex,
                                                       UtlString& receiveAddress,
                                                       int rtpPort,
                                                       int rtcpPort)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpTopologyGraphInterface::setMediaPassThrough(connectionId=%d, mediaType=%d, mediaTypeStreamIndex=%d, receiveAddress=\"%s\", rtpPort=%d, rtcpPort=%d",
                  connectionId, (int)mediaType, mediaTypeStreamIndex, receiveAddress.data(), rtpPort, rtcpPort);

    // May support multiple audio or video stream in future.
    assert(mediaType == VIDEO_STREAM);
    assert(mediaTypeStreamIndex == 0);
    OsStatus status = OS_NOT_YET_IMPLEMENTED;
    if(mediaType == VIDEO_STREAM &&
       mediaTypeStreamIndex == 0)
    {
        CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

        if(mediaConnection)
        {
            switch(mediaType)
            {
            case VIDEO_STREAM:
#ifdef VIDEO
                mediaConnection->mVideoPassThroughEnabled = TRUE;
                mediaConnection->mRtpVideoReceivePort = rtpPort;
                mediaConnection->mRtcpVideoReceivePort = rtcpPort;
                status = OS_SUCCESS;
#else
                status = OS_NOT_SUPPORTED;
#endif

            break;

            default:
            break;
            }
        }
        else
        {
            status = OS_INVALID_ARGUMENT;
        }
    }

    return(status);
}

OsStatus CpTopologyGraphInterface::setConnectionDestination(int connectionId,
                                                         const char* remoteRtpHostAddress,
                                                         int remoteAudioRtpPort,
                                                         int remoteAudioRtcpPort,
                                                         int remoteVideoRtpPort,
                                                         int remoteVideoRtcpPort)
{

    // OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination[1](%d, '%s', A: %d, %d, V: %d, %d)", connectionId, remoteRtpHostAddress, remoteAudioRtpPort, remoteAudioRtcpPort, remoteVideoRtpPort, remoteVideoRtcpPort);
    OsStatus status = setConnectionDestination(connectionId, AUDIO_STREAM, 0, remoteRtpHostAddress, remoteAudioRtpPort, remoteAudioRtcpPort);

    if(status == OS_SUCCESS)
    {
        status = setConnectionDestination(connectionId, VIDEO_STREAM, 0, remoteRtpHostAddress, remoteVideoRtpPort, remoteVideoRtcpPort);
    }
    // OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination([1]) returning %d", status);

    return(status);
}

OsStatus CpTopologyGraphInterface::setConnectionDestination(int connectionId,
                                                            CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                                            int streamindex,
                                                            const char* remoteRtpHostAddress,
                                                            int remoteRtpPort,
                                                            int remoteRtcpPort)
{
    // OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination[2](%d, %d, %d, '%s', %d, %d)", connectionId, mediaType, streamindex, remoteRtpHostAddress, remoteRtpPort, remoteRtcpPort);
    assert(streamindex == 0);
    OsStatus returnCode = OS_NOT_FOUND;
    CpTopologyMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if (remoteRtcpPort < 1)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination called with RTCP port==%d.  Setting to %d", remoteRtcpPort, remoteRtpPort+1);
        remoteRtcpPort = remoteRtpPort + 1;
    }

    if(pMediaConnection && remoteRtpHostAddress && *remoteRtpHostAddress)
    {
        returnCode = OS_SUCCESS;
        /*
         * Common Setup
         */

        if(mediaType == CpMediaInterface::AUDIO_STREAM)
        {
            /*
             * Audio Setup
             */
            pMediaConnection->mAudioDestinationSet = TRUE;
            pMediaConnection->mRtpAudioSendHostAddress = remoteRtpHostAddress ;
            pMediaConnection->mRtpAudioSendHostPort = remoteRtpPort;
            pMediaConnection->mRtcpAudioSendHostPort = remoteRtcpPort;

            if(!pMediaConnection->mIsCustomSockets)
            {
                if(pMediaConnection->mpRtpAudioSocket)
                {
                    if (!pMediaConnection->mIsMulticast)
                    {
                        OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpAudioSocket;
                        pSocket->readyDestination(remoteRtpHostAddress, remoteRtpPort) ;
                        pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteRtpPort) ;
                        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination setting remote RTP address: %s port: %d",
                            remoteRtpHostAddress, remoteRtpPort);
                    }
                    else
                    {
                        ((OsDatagramSocket*)pMediaConnection->mpRtpAudioSocket)->doConnect(remoteRtpPort,
                                                 remoteRtpHostAddress,
                                                 TRUE);
                    }
                }

                if(pMediaConnection->mpRtcpAudioSocket && (remoteRtcpPort > 0))           
                {
                    if (!pMediaConnection->mIsMulticast)
                    {
                        OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpAudioSocket;
                        pSocket->readyDestination(remoteRtpHostAddress, remoteRtcpPort);
                        pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteRtcpPort);
                        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination setting remote RTCP address: %s port: %d",
                            remoteRtpHostAddress, remoteRtcpPort);
                    }
                    else
                    {
                        ((OsDatagramSocket*)pMediaConnection->mpRtcpAudioSocket)->doConnect(remoteRtpPort,
                                                  remoteRtpHostAddress,
                                                  TRUE);
                    }
                }
                else
                {
                    pMediaConnection->mRtcpAudioSendHostPort = 0 ;
                }
            }
        }

        else if(mediaType == CpMediaInterface::VIDEO_STREAM)
        {
            /*
             * Video Setup
             */
#ifdef VIDEO
            pMediaConnection->mVideoDestinationSet = TRUE;
            pMediaConnection->mRtpVideoSendHostAddress = remoteRtpHostAddress;

            if (pMediaConnection->mpRtpVideoSocket)
            {
                pMediaConnection->mRtpVideoSendHostPort = remoteRtpPort;
                if (!pMediaConnection->mIsMulticast)
                {
                    OsNatDatagramSocket *pRtpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpVideoSocket;
                    pRtpSocket->readyDestination(remoteRtpHostAddress, remoteRtpPort);
                    pRtpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteRtpPort);
                    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination setting remote Video RTP address: %s port: %d",
                        remoteRtpHostAddress, remoteRtpPort);
                }
                else
                {
                    ((OsDatagramSocket*)pMediaConnection->mpRtpVideoSocket)->doConnect(remoteRtpPort,
                                                      remoteRtpHostAddress,
                                                      TRUE);
                }

                if(pMediaConnection->mpRtcpVideoSocket && (remoteRtcpPort > 0))
                {
                    pMediaConnection->mRtcpVideoSendHostPort = remoteRtcpPort;               
                    if (!pMediaConnection->mIsMulticast)
                    {
                       OsNatDatagramSocket *pRctpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpVideoSocket;
                       pRctpSocket->readyDestination(remoteRtpHostAddress, remoteRtcpPort);
                       pRctpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteRtcpPort);
                       OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination setting remote Video RTCP address: %s port: %d",
                           remoteRtpHostAddress, remoteRtcpPort);
                    }
                    else
                    {
                       ((OsDatagramSocket*)pMediaConnection->mpRtcpVideoSocket)->doConnect(remoteRtcpPort,
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
    }

   // OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::setConnectionDestination([2]) returning %d", returnCode);
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
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::addAudioRtcpConnectionDestination adding alt send address: %s port: %d",
                candidateIp, candidatePort);

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

OsStatus CpTopologyGraphInterface::copyPayloadIds(int connectionId, int numCodecs, SdpCodec* remoteCodecs[])
{
    OsStatus status = OS_NOT_FOUND;
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

    if(mediaConnection && mediaConnection->mpCodecFactory)
    {
//#ifdef TEST_PRINT
        UtlString remoteCodecsString;
        UtlString codecString;
        for(int codecIndex = 0; codecIndex < numCodecs; codecIndex++)
        {
            remoteCodecs[codecIndex]->toString(codecString);
            if(codecIndex > 0)
            {
                remoteCodecsString.append("\n");
            }
            remoteCodecsString.append(codecString);
        }
        OsSysLog::add(FAC_CP, PRI_DEBUG,
            "remote codecs (%d): %s", numCodecs, remoteCodecsString.data());

        UtlString beforeCodecs;
        mediaConnection->mpCodecFactory->toString(beforeCodecs);
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::copyPayloadIds mpCodecFactory:\n%s",
                      beforeCodecs.data());
//#endif

        mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                          (const SdpCodec**) remoteCodecs);

//#ifdef TEST_PRINT
        UtlString afterCodecs;
        mediaConnection->mpCodecFactory->toString(afterCodecs);
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::copyPayloadIds codec payload IDs change to:\n%s",
                      afterCodecs.data());
//#endif

        status = OS_SUCCESS;
    }

    return(status);
}

OsStatus CpTopologyGraphInterface::startRtpSend(int connectionId,
                                             int numCodecs,
                                             SdpCodec* sendCodecs[])
{
   // need to set default payload types in get capabilities
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "CpTopologyGraphInterface::startRtpSend(%d, %d, %p)",
                  connectionId,
                  numCodecs,
                  sendCodecs);
#endif

   int i;
   SdpCodec* audioCodec = NULL;
   SdpCodec* videoCodec = NULL;
   SdpCodec* dtmfCodec = NULL;
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return returnCode;

   // Find primary audio, DTMF and video codecs
   for (i=0; i<numCodecs; i++)
   {
      if (SdpCodec::SDP_CODEC_TONES == sendCodecs[i]->getValue())
      {
         if (NULL == dtmfCodec)
         {
            dtmfCodec = sendCodecs[i];
         }
         continue;
      }

      UtlString codecMediaType;
      sendCodecs[i]->getMediaType(codecMediaType);

      if(audioCodec == NULL && codecMediaType == "audio")
      {
         audioCodec = sendCodecs[i];
         continue;
      }

      if(videoCodec == NULL && codecMediaType.compareTo(MIME_TYPE_VIDEO, UtlString::ignoreCase) == 0)
      {
         videoCodec = sendCodecs[i];
      }

   }

   // If we haven't set a destination and we have set alternate destinations
   if (!mediaConnection->mAudioDestinationSet && 
#ifdef VIDEO
       !mediaConnection->mVideoDestinationSet && 
#endif
       mediaConnection->mbAlternateDestinations)
   {
      applyAlternateDestinations(connectionId) ;
   }

   UtlString beforeCodecs;
   mediaConnection->mpCodecFactory->toString(beforeCodecs);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "startRtpSend mpCodecFactory:\n%s",
                 beforeCodecs.data());
   // Disabling this as it causes us to expect wrong payload IDs for incoming RTP when
   // we make the SDP offer and their response has different payload IDs
   if(0) // (mediaConnection->mpCodecFactory)
   {
      // Make sure we use the same payload types as the remote
      // side.  Its the friendly thing to do.
      mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                        (const SdpCodec**) sendCodecs);
      UtlString afterCodecs;
      mediaConnection->mpCodecFactory->toString(afterCodecs);
      OsSysLog::add(FAC_CP, PRI_DEBUG, "startRtpSend startRtpSend codec payload IDs change to:\n%s",
                    afterCodecs.data());
   }

   if (mpTopologyGraph)
   {
//#ifdef TEST_PRINT
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Sending audio RTP/RTCP codec: %d sockets: %p/%p descriptors: %d/%d address: %s send ports: %d, %d\n",
           audioCodec ? audioCodec->getCodecType() : -2,
           (mediaConnection->mpRtpAudioSocket), (mediaConnection->mpRtcpAudioSocket),
           mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocket->getSocketDescriptor(),
           mediaConnection->mRtpAudioSendHostAddress.data(),
           mediaConnection->mRtpAudioSendHostPort, mediaConnection->mRtcpAudioSendHostPort);
//#endif

       if (mediaConnection->mpAudioCodec != NULL)
       {
           delete mediaConnection->mpAudioCodec ;
           mediaConnection->mpAudioCodec = NULL ;
       }
#ifdef VIDEO
       if (mediaConnection->mpVideoCodec != NULL)
       {
           delete mediaConnection->mpVideoCodec ;
           mediaConnection->mpVideoCodec = NULL ;
       }
#endif

       // Should the following be done inside the if we have an audio send address scope?????
       // Store the primary codec for cost calculations later
       if (audioCodec != NULL)
       {
           mediaConnection->mpAudioCodec = new SdpCodec();
           *mediaConnection->mpAudioCodec = *audioCodec ;
           UtlString audioMimeSubType;
           audioCodec->getEncodingName(audioMimeSubType);
           OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::startRtpSend primary audio codec: %s payload ID: %d",
               audioMimeSubType.data(), audioCodec->getCodecPayloadFormat());
       }

       // Make sure we use the same payload types as the remote
       // side.  Its the friendly thing to do.
       if (mediaConnection->mpCodecFactory)
       {
           mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                             (const SdpCodec**) sendCodecs);
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

      // Start sending RTP if destination address is present.
      if ( !mediaConnection->mRtpAudioSendHostAddress.isNull()
#ifdef VIDEO  // Should this be mRtpAudioSendHostAddress???
         && mediaConnection->mRtpVideoSendHostAddress.compareTo("0.0.0.0")
#endif
         )
      {
         UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
         UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
         MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
         MpResourceTopology::replaceNumInName(encodeName, connectionId);
         MpRtpOutputConnection *pConnection;
         OsStatus stat = mpTopologyGraph->lookupResource(outConnectionName,
                                                         (MpResource*&)pConnection);
         OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::startRtpSend lookupResource of MpConnection %s returned: %d",
             outConnectionName.data(), stat);
         assert(stat == OS_SUCCESS);

         // Set sockets to send to.
         pConnection->setSockets(*mediaConnection->mpRtpAudioSocket,
                                 *mediaConnection->mpRtcpAudioSocket);

         // Tell encoder which codecs to use (data codec and signaling codec)
         // and enable it.
         MprEncode::selectCodecs(encodeName, *mpTopologyGraph->getMsgQ(),
                                 audioCodec, dtmfCodec);
         MpResource::enable(encodeName, *mpTopologyGraph->getMsgQ());
         MpResource::enable(outConnectionName, *mpTopologyGraph->getMsgQ());

         mediaConnection->mRtpAudioSending = TRUE;
         returnCode = OS_SUCCESS;
      }

#ifdef VIDEO
      // Start sending RTP if destination address is present.
      if ( mediaConnection->mRtpVideoSendHostPort > 0 &&
         !mediaConnection->mRtpVideoSendHostAddress.isNull() &&
         mediaConnection->mRtpVideoSendHostAddress.compareTo("0.0.0.0"))
      {
          // Do the same for video
          // Note: temporarily use MprDecode message for video encode as we want full list of
          // codecs to make inteligent choice at media layer
          UtlString videoConnectionName(DEFAULT_VIDEO_RTP_OUTPUT_RESOURCE_NAME);
          MpResourceTopology::replaceNumInName(videoConnectionName, connectionId);

          MprFromNet::setSockets(videoConnectionName, *(mpTopologyGraph->getMsgQ()),
              mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtcpVideoSocket);

          MprDecode::selectCodecs(videoConnectionName, *mpTopologyGraph->getMsgQ(),
                                  sendCodecs, numCodecs);

          MpResource::enable(videoConnectionName, *mpTopologyGraph->getMsgQ());
          mediaConnection->mRtpVideoSending = TRUE;

          returnCode = OS_SUCCESS;

          if (videoCodec != NULL)
          {
              mediaConnection->mpVideoCodec = new SdpCodec();
              *mediaConnection->mpVideoCodec = *videoCodec ;
              UtlString videoMimeSubType;
              videoCodec->getEncodingName(videoMimeSubType);
              OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::startRtpSend primary video codec: %s payload ID: %d",
                  videoMimeSubType.data(), videoCodec->getCodecPayloadFormat());
          }
      }
#endif
   }

   return (returnCode);
}


OsStatus CpTopologyGraphInterface::startRtpReceive(int connectionId,
                                                   int numCodecs,
                                                   SdpCodec* receiveCodecs[])
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;

   UtlString beforeCodecs;
   mediaConnection->mpCodecFactory->toString(beforeCodecs);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "startRtpReceive mpCodecFactory:\n%s",
                 beforeCodecs.data());
   // Disabling this as it causes us to expect wrong payload IDs for incoming RTP when
   // we make the SDP offer and their response has different payload IDs
   if(0)// (mediaConnection->mpCodecFactory)
   {
        // Make sure we use the same payload types as the remote
        // side.  It's the friendly thing to do.
        mediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                          (const SdpCodec**) receiveCodecs);
        UtlString afterCodecs;
        mediaConnection->mpCodecFactory->toString(afterCodecs);
        OsSysLog::add(FAC_CP, PRI_DEBUG, "startRtpReceive mpCodecFactory payload IDs changed to:\n%s",
                      afterCodecs.data());
   }

   if (mpTopologyGraph)
   {
//#ifdef TEST_PRINT
      int i;

      OsSysLog::add(FAC_CP, PRI_DEBUG, "Start Receiving RTP/RTCP, %d codec%s; sockets: %p/%p descriptors: %d/%d\n",
           numCodecs, ((1==numCodecs)?"":"s"),
           (mediaConnection->mpRtpAudioSocket),
           (mediaConnection->mpRtcpAudioSocket),
           mediaConnection->mpRtpAudioSocket->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
      UtlString audioMimeSubType;
      for (i=0; i<numCodecs; i++)
      {
          receiveCodecs[i]->getEncodingName(audioMimeSubType);
          OsSysLog::add(FAC_CP, PRI_DEBUG, "startRtpReceive codec[%d]: %s internal ID: %d payload ID: %d\n", 
                        i+1,
                        audioMimeSubType.data(),
                        receiveCodecs[i]->getCodecType(),
                        receiveCodecs[i]->getCodecPayloadFormat());
      }
//#endif

      if(mediaConnection->mRtpAudioReceiving)
      {
         // This is not supposed to be necessary and may be
         // causing an audible glitch when codecs are changed
         //mpTopologyGraph->stopReceiveRtp(connectionId);
      }

      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
      UtlString decoderName(DEFAULT_DECODE_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(decoderName, connectionId);
      decoderName.append(STREAM_NAME_SUFFIX);

      if (numCodecs)
      {
         // Loop over all decoders and select codecs.
         for (int i=0; i<mediaConnection->mNumRtpStreams; i++)
         {
            UtlString tmpName(decoderName);
            MpResourceTopology::replaceNumInName(tmpName, i);
            MprDecode::selectCodecs(tmpName,
                                    *mpTopologyGraph->getMsgQ(),
                                    receiveCodecs,
                                    numCodecs);
         }
      }

#ifdef VIDEO
      // Set codecs on video in RTP stream
      UtlString inVideoConnectionName(DEFAULT_VIDEO_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inVideoConnectionName, connectionId);
      MprDecode::selectCodecs(inVideoConnectionName, 
                              *(mpTopologyGraph->getMsgQ()),
                              receiveCodecs,
                              numCodecs);

      // Enabled video in RTP stream
      MprDecode::enable(inVideoConnectionName, *(mpTopologyGraph->getMsgQ()));

      // Set video sockets to read RTP
      MprFromNet::setSockets(inVideoConnectionName, *(mpTopologyGraph->getMsgQ()),
          mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtcpVideoSocket);
      mediaConnection->mRtpVideoReceiving = TRUE;
#endif

      // Fence before calling asynchronous method setSockets().
      mpTopologyGraph->synchronize("CpTopologyGraphInterface::startRtpReceive");

      MpRtpInputConnection *pInputConnection;
      OsStatus tmpStat = mpTopologyGraph->lookupResource(inConnectionName,
                                                         (MpResource*&)pInputConnection);
      assert(tmpStat == OS_SUCCESS);
      pInputConnection->setSockets(*(mediaConnection->mpRtpAudioSocket),
                                   *(mediaConnection->mpRtcpAudioSocket));

      // And finally enable decode resource, if this connection is unicast.
      // For multicast connections decoders will be enabled on the fly
      // to reduce CPU consumption.
      if (numCodecs && !mediaConnection->mIsMulticast)
      {
         // Loop over all decoders and enable them.
         for (int i=0; i<mediaConnection->mNumRtpStreams; i++)
         {
            UtlString tmpName(decoderName);
            MpResourceTopology::replaceNumInName(tmpName, i);
            MprDecode::enable(tmpName, *mpTopologyGraph->getMsgQ());
         }
      }

      mediaConnection->mRtpAudioReceiving = TRUE;

      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

const SdpCodecList* CpTopologyGraphInterface::getConnectionCodecList(int connectionId)
{
   const SdpCodecList* codecList = NULL;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
      codecList = mediaConnection->mpCodecFactory;
   }

   return(codecList);
}

OsStatus CpTopologyGraphInterface::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if (mpTopologyGraph && mediaConnection &&
       mediaConnection->mRtpAudioSending)
   {
      stopRtpSend(mediaConnection);
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
#ifdef TEST_PRINT
      printf("sending stopReceiveRtp message RTP socket: %p RTCP socket: %p\n",
             mediaConnection->mpRtpAudioSocket,
             mediaConnection->mpRtcpAudioSocket);
#endif

      stopRtpReceive(mediaConnection);

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

   returnCode = deleteMediaConnection(mediaConnection);

   return(returnCode);
}

OsStatus CpTopologyGraphInterface::playAudio(const char* url,
                                          UtlBoolean repeat,
                                          UtlBoolean local,
                                          UtlBoolean remote,
                                          UtlBoolean mixWithMic,
                                          int downScaling,
                                          UtlBoolean autoStopAfterFinish)
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
          *mpTopologyGraph->getMsgQ(), mpTopologyGraph->getSamplesPerSec(),
          filename, repeat, autoStopAfterFinish);
    }

    if(stat != OS_SUCCESS)
    {
        osPrintf("Cannot play audio file: %s\n", filename.data());
    }

    return(stat);
}

OsStatus CpTopologyGraphInterface::playBuffer(char* buf,
                                              unsigned long bufSize,
                                              uint32_t bufRate,
                                              int type, 
                                              UtlBoolean repeat,
                                              UtlBoolean local,
                                              UtlBoolean remote, 
                                              OsProtectedEvent* pEvent,
                                              UtlBoolean mixWithMic,
                                              int downScaling,
                                              UtlBoolean autoStopOnFinish)
{
    OsStatus stat = OS_NOT_FOUND;
    if(mpTopologyGraph && buf)
    {
       uint32_t fgRate = mpTopologyGraph->getSamplesPerSec();
       // Currently, this ignores "local/remote", "mixWithMic" and "downScaling".
       stat = MprFromFile::playBuffer(DEFAULT_FROM_FILE_RESOURCE_NAME, 
                                      *mpTopologyGraph->getMsgQ(),
                                      buf, bufSize, bufRate, fgRate,
                                      type, repeat, pEvent, autoStopOnFinish);
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

OsStatus CpTopologyGraphInterface::pauseAudio()
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprFromFile::pauseFile(DEFAULT_FROM_FILE_RESOURCE_NAME,
                                    *mpTopologyGraph->getMsgQ());
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::resumeAudio()
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprFromFile::resumeFile(DEFAULT_FROM_FILE_RESOURCE_NAME,
                                     *mpTopologyGraph->getMsgQ());
   }
   return stat;
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
                                                    UtlBoolean autoStopOnFinish) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return playAudio(url, repeat, local, remote, mixWithMic, downScaling, autoStopOnFinish) ;
}


OsStatus CpTopologyGraphInterface::stopChannelAudio(int connectionId) 
{
    // TODO:: This API is designed to record the audio from a single channel.  
    // If the connectionId is -1, record all.

    return stopAudio() ;
}


OsStatus CpTopologyGraphInterface::recordChannelAudio(int connectionId,
                                                      const char* szFile,
                                                      CpAudioFileFormat cpFileFormat,
                                                      UtlBoolean appendToFile,
                                                      int numChannels)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
#if MAXIMUM_RECORDER_CHANNELS > 1
      {
          int numLocalBridgePorts = MAXIMUM_RECORDER_CHANNELS < 3 ? 3 : MAXIMUM_RECORDER_CHANNELS + 1;
          // getNumBridgePorts provides the number of used ports, not the maximum
          //int totalBridgePorts = getNumBridgePorts();
          int totalBridgePorts = DEFAULT_BRIDGE_MAX_IN_OUTPUTS;
          float* recorderWeights = new float[totalBridgePorts];
          for(int channelIndex = 0; channelIndex < numChannels; channelIndex++)
          {
              for(int portIndex = 0; portIndex < totalBridgePorts; portIndex++)
              {
                  recorderWeights[portIndex] = 0.0;

                  // If first channel, all the local ports are eanbles
                  if(channelIndex == 0 && 
                     portIndex < numLocalBridgePorts)
                  {
                      recorderWeights[portIndex] = 1.0;
                  }

                  // If middle channel only a single remote stream is recorded
                  if(channelIndex > 0 && channelIndex < numChannels -1 &&
                     portIndex == numLocalBridgePorts + channelIndex - 1)
                  {
                      recorderWeights[portIndex] = 1.0;
                  }

                  // If last channel, all remaining remote streams are mixed in
                  if(channelIndex == numChannels - 1 &&
                     portIndex >= numLocalBridgePorts + channelIndex - 1)
                  {
                      recorderWeights[portIndex] = 1.0;
                  }
              }

              // Set the weights for this channel on the recorder
              setMixWeightsForOutput(channelIndex, totalBridgePorts, recorderWeights);
          }
          delete[] recorderWeights;
      }
#endif

      MprRecorder::RecordFileFormat recordFormat = MprRecorder::WAV_PCM_16;
      switch(cpFileFormat)
      {
          case CP_WAVE_PCM_16:
              recordFormat = MprRecorder::WAV_PCM_16;
              stat = OS_SUCCESS;
              break;

          case CP_WAVE_GSM:
              recordFormat = MprRecorder::WAV_GSM;
              stat = OS_SUCCESS;
              break;

          case CP_OGG_OPUS:
              recordFormat = MprRecorder::OGG_OPUS;
              stat = OS_SUCCESS;
              break;

          default:
              OsSysLog::add(FAC_CP, PRI_ERR,
                      "Invalid record file format: %d",
                      cpFileFormat);
              OsSysLog::flush();
              assert(0);
              stat = OS_INVALID_ARGUMENT;
              break;
      }

      if(stat == OS_SUCCESS)
      {
          stat = MprRecorder::startFile(DEFAULT_RECORDER_RESOURCE_NAME,
                                        *mpTopologyGraph->getMsgQ(),
                                        szFile,
                                        recordFormat,
                                        0, // max record length = unlimited
                                        -1, // don't check/stop after silence
                                        appendToFile,
                                        numChannels);
      }
   }

   return(stat);
}

OsStatus CpTopologyGraphInterface::pauseRecordChannelAudio(int connectionId) 
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::pause(DEFAULT_RECORDER_RESOURCE_NAME,
                                *mpTopologyGraph->getMsgQ());
   }
   return(stat);
}

OsStatus CpTopologyGraphInterface::resumeRecordChannelAudio(int connectionId) 
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::resume(DEFAULT_RECORDER_RESOURCE_NAME,
                                 *mpTopologyGraph->getMsgQ());
   }
   return(stat);
}

OsStatus CpTopologyGraphInterface::stopRecordChannelAudio(int connectionId) 
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::stop(DEFAULT_RECORDER_RESOURCE_NAME,
                               *mpTopologyGraph->getMsgQ());
   }
   return stat;
}


OsStatus CpTopologyGraphInterface::recordBufferChannelAudio(int connectionId,
                                                            char* pBuffer,
                                                            int bufferSize,
                                                            int maxRecordTime,
                                                            int maxSilence) 
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::startBuffer(DEFAULT_RECORDER_RESOURCE_NAME,
                                      *mpTopologyGraph->getMsgQ(),
                                      (MpAudioSample*)pBuffer,
                                      bufferSize/sizeof(MpAudioSample),
                                      maxRecordTime,
                                      maxSilence);
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::stopRecordBufferChannelAudio(int connectionId) 
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::stop(DEFAULT_RECORDER_RESOURCE_NAME,
                               *mpTopologyGraph->getMsgQ());
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::recordCircularBufferChannelAudio(int connectionId,
                                                                    CircularBufferPtr & buffer,
                                                                    CpMediaInterface::CpAudioFileFormat recordingFormat,
                                                                    unsigned long recordingBufferNotificationWatermark)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
       MprRecorder::RecordFileFormat format;
       stat = TypeConverter::translateRecordingFormat(recordingFormat, format);

       if (stat == OS_SUCCESS)
       {
           stat = MprRecorder::startCircularBuffer(DEFAULT_RECORDER_RESOURCE_NAME,
               *mpTopologyGraph->getMsgQ(),
               buffer,
               format,
               recordingBufferNotificationWatermark);
       }
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::stopRecordCircularBufferChannelAudio(int connectionId)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::stop(DEFAULT_RECORDER_RESOURCE_NAME,
                               *mpTopologyGraph->getMsgQ());
   }
   return stat;
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
      // Generate in-band tone
      stat = MprToneGen::startTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                   *mpTopologyGraph->getMsgQ(), toneId);

      // Generate RFC4733 out-of-band tone
      CpTopologyMediaConnection* mediaConnection = NULL;
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpTopologyMediaConnection*) connectionIterator()))
      {
         UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
         MpResourceTopology::replaceNumInName(encodeName, mediaConnection->getValue());
         stat = MprEncode::startTone(encodeName, *mpTopologyGraph->getMsgQ(), toneId);
      }
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
      // Stop in-band tone
      stat = MprToneGen::stopTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                  *mpTopologyGraph->getMsgQ());

      // Stop RFC4733 out-of-band tone
      CpTopologyMediaConnection* mediaConnection = NULL;
      UtlDListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpTopologyMediaConnection*) connectionIterator()))
      {
         UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
         MpResourceTopology::replaceNumInName(encodeName, mediaConnection->getValue());
         stat = MprEncode::stopTone(encodeName, *mpTopologyGraph->getMsgQ());
      }
   }
   else
   {
      stat = OS_NOT_FOUND;
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::setRtcpTimeOffset(int connectionId,
                                                     CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                                     int streamIndex,
                                                     int timeOffset) // Milliseconds
{

    // Get the connection just to validate the connectionId
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
    OsStatus status = OS_INVALID_ARGUMENT;
    if(streamIndex != 0)
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
            "CpTopologyGraphInterface::setRtcpTimeOffset invalid stream index: %d",
            streamIndex);
        OsSysLog::flush();
        assert(streamIndex == 0);

    }
    else if(mediaConnection)
    {
        // Construct name of  resource
        UtlString outConnectionName;
        if(mediaType == CpMediaInterface::AUDIO_STREAM)
        {
            outConnectionName = DEFAULT_RTP_OUTPUT_RESOURCE_NAME;
        }
        else if(mediaType == CpMediaInterface::VIDEO_STREAM)
        {
            outConnectionName = DEFAULT_VIDEO_RTP_OUTPUT_RESOURCE_NAME;
        }
        else
        {
            OsSysLog::add(FAC_CP, PRI_ERR,
                "CpTopologyGraphInterface::setRtcpTimeOffset invalid stream type: %d",
                mediaType);
            OsSysLog::flush();
            assert(0);
        }
        MpResourceTopology::replaceNumInName(outConnectionName, connectionId);

        // Post message to set offset for stream
        if(mpTopologyGraph)
        {
            // Convert milliseconds to microseconds
            MprToNet::setSRAdjustUSecs(outConnectionName, *(mpTopologyGraph->getMsgQ()), timeOffset * 1000);
            status = OS_SUCCESS;
        }
        else
        {
            status = OS_NOT_FOUND;
        }
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                "CpTopologyGraphInterface::setRtcpTimeOffset connection %d not found",
                connectionId);
    }

    return(status);
}

OsStatus CpTopologyGraphInterface::startChannelTone(int connectionId,
                                                    int toneId,
                                                    UtlBoolean local,
                                                    UtlBoolean remote) 
{
   OsStatus stat = OS_FAILED;

   if (mpTopologyGraph)
   {
      // Generate in-band tone
      stat = MprToneGen::startTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                   *mpTopologyGraph->getMsgQ(), toneId);

      // Generate RFC4733 out-of-band tone
      if(connectionId > getInvalidConnectionId())
      {
          UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
          MpResourceTopology::replaceNumInName(encodeName, connectionId);
          stat = MprEncode::startTone(encodeName, *mpTopologyGraph->getMsgQ(), toneId);
      }
      // else OK to not emit inband DTMF if no connection
   }
   else
   {
      stat = OS_NOT_FOUND;
   }

   return stat;
}

OsStatus CpTopologyGraphInterface::stopChannelTone(int connectionId)
{
   OsStatus stat = OS_FAILED;

   if (mpTopologyGraph)
   {
      // Stop in-band tone
      stat = MprToneGen::stopTone(DEFAULT_TONE_GEN_RESOURCE_NAME, 
                                   *mpTopologyGraph->getMsgQ());

      // Stop RFC4733 out-of-band tone
      if(connectionId > getInvalidConnectionId())
      {
          UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
          MpResourceTopology::replaceNumInName(encodeName, connectionId);
          stat = MprEncode::stopTone(encodeName, *mpTopologyGraph->getMsgQ());
      }
   }
   else
   {
      stat = OS_NOT_FOUND;
   }

   return stat;
}


OsStatus CpTopologyGraphInterface::giveFocus()
{
    if(mpTopologyGraph)
    {
        // Set the flow graph to have the focus
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask();
        mediaTask->setFocus(mpTopologyGraph);
   }

   return OS_SUCCESS ;
}

OsStatus CpTopologyGraphInterface::defocus()
{
   if(mpTopologyGraph)
   {
      MpMediaTask* mediaTask = MpMediaTask::getMediaTask();

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

OsStatus CpTopologyGraphInterface::recordMic(int ms,
                                             int16_t* pAudioBuf,
                                             int bufferSize)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprRecorder::startBuffer(DEFAULT_RECORDER_RESOURCE_NAME,
                                      *mpTopologyGraph->getMsgQ(),
                                      pAudioBuf, bufferSize, ms);
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::recordMic(int ms,
                                             int silenceLength,
                                             const char* fileName)
{
    return OS_NOT_SUPPORTED;
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

OsStatus CpTopologyGraphInterface::limitCodecs(int connectionId, const SdpCodecList& includeOnlyCodecList)
{
#ifdef TEST_PRINT
    UtlString limitCodecListString;
    includeOnlyCodecList.toString(limitCodecListString);
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::limitCodecs limit codec list: %s",
        limitCodecListString.data());

    UtlString unlimitedCodecListString;
    mSupportedCodecs.toString(unlimitedCodecListString);
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::limitCodecs unlimited interface codec list: %s",
        unlimitedCodecListString.data());
#endif

    OsStatus returnCode = OS_NOT_FOUND;
    if(connectionId == getInvalidConnectionId())
    {
        // Limit the codec set for the media interface
        mSupportedCodecs.limitCodecs(includeOnlyCodecList);

#ifdef TEST_PRINT
        UtlString limitedCodecListString;
        mSupportedCodecs.toString(limitedCodecListString);
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::limitCodecs interface codec list limited: %s",
            limitedCodecListString.data());
#endif
        returnCode = OS_SUCCESS;
    }
    else
    {
        CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

        if(mediaConnection && mediaConnection->mpCodecFactory)
        {
#ifdef TEST_PRINT
            UtlString connectionUnlimitedCodecListString;
            mediaConnection->mpCodecFactory->toString(connectionUnlimitedCodecListString);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::limitCodecs unlimited connection codec list: %s",
                connectionUnlimitedCodecListString.data());
#endif

            mediaConnection->mpCodecFactory->limitCodecs(includeOnlyCodecList);

#ifdef TEST_PRINT
            UtlString connnectionCodecListString;
            mediaConnection->mpCodecFactory->toString(connnectionCodecListString);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::limitCodecs connectioncodec list limited: %s",
                connnectionCodecListString.data());
#endif
            returnCode = OS_SUCCESS;
        }
    }

    return(returnCode);
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

OsStatus CpTopologyGraphInterface::enableDtx(int connectionId, UtlBoolean enable)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mpTopologyGraph && mediaConnection)
   {
      UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(encodeName, connectionId);
      MprEncode::enableDtx(encodeName,
                           *mpTopologyGraph->getMsgQ(),
                           enable);

      returnCode = OS_SUCCESS;
   }

   return returnCode;
}

OsStatus CpTopologyGraphInterface::setRtpInactivityTimeout(int connectionId,
                                                           int timeoutMs)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mpTopologyGraph && mediaConnection)
   {
      UtlString rtpInputName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(rtpInputName, connectionId);
      MpRtpInputConnection::setRtpInactivityTimeout(rtpInputName,
                                                    *mpTopologyGraph->getMsgQ(),
                                                    timeoutMs);
      returnCode = OS_SUCCESS;
   }

   return returnCode;
}

/* ============================ ACCESSORS ================================= */

// Calculate the current cost for our sending/receiving codecs
int CpTopologyGraphInterface::getCodecCPUCost()
{   
   SdpCodec::SdpCodecCPUCost iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   

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
            SdpCodec::SdpCodecCPUCost iCodecCost =
               mediaConnection->mpAudioCodec->getCPUCost();
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

uint32_t CpTopologyGraphInterface::getSamplesPerSec()
{
   return mpTopologyGraph ? mpTopologyGraph->getSamplesPerSec() : 0;
}

uint32_t CpTopologyGraphInterface::getSamplesPerFrame()
{
   return mpTopologyGraph ? mpTopologyGraph->getSamplesPerFrame() : 0;
}

// Returns the flowgraph's message queue
   OsMsgQ* CpTopologyGraphInterface::getMsgQ()
{
   return mpTopologyGraph->getMsgQ() ;
}

OsMsgDispatcher* CpTopologyGraphInterface::getNotificationDispatcher()
{
   return mpTopologyGraph ? mpTopologyGraph->getNotificationDispatcher() : NULL;
}

OsStatus CpTopologyGraphInterface::getPrimaryCodec(int connectionId, 
                                                UtlString& audioCodec,
                                                UtlString& videoCodec,
                                                int* audioPayloadType,
                                                int* videoPayloadType,
                                                bool& isEncrypted)
{
    OsStatus status =  OS_NOT_FOUND;
    UtlString codecType;
    CpTopologyMediaConnection* pConnection = getMediaConnection(connectionId);
    if(pConnection)
    {
        status = OS_SUCCESS;

        if (pConnection->mpAudioCodec != NULL)
        {
            pConnection->mpAudioCodec->getEncodingName(audioCodec);
            *audioPayloadType = pConnection->mpAudioCodec->getCodecPayloadFormat();
        }

#ifdef VIDEO
        if(pConnection->mpVideoCodec)
        {
            pConnection->mpVideoCodec->getEncodingName(videoCodec);
            *videoPayloadType = pConnection->mpVideoCodec->getCodecPayloadFormat();
        }
        else
#endif
        {
            videoCodec="";
            *videoPayloadType=0;
        }
    }

    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpTopologyGraphInterface::getPrimaryCodec returning %d",
            status);

    return(status);
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

#ifdef VIDEO
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
       sending = mediaConnection->mRtpVideoSending;
   }
   else
   {
       osPrintf("CpTopologyGraphInterface::isSendingRtpVideo invalid connectionId: %d\n",
          connectionId);
   }
#endif

   return(sending);
}

UtlBoolean CpTopologyGraphInterface::isReceivingRtpVideo(int connectionId)
{
   UtlBoolean receiving = FALSE;

#ifdef VIDEO
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);
   if(mediaConnection)
   {
      receiving = mediaConnection->mRtpVideoReceiving;
   }
   else
   {
       osPrintf("CpTopologyGraphInterface::isReceivingRtpVideo invalid connectionId: %d\n",
          connectionId);
   }

#endif

   return(receiving);
}


UtlBoolean CpTopologyGraphInterface::isDestinationSet(int connectionId)
{
    UtlBoolean isSet = FALSE;
    CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

    if(mediaConnection)
    {
        isSet = mediaConnection->mAudioDestinationSet 
#ifdef VIDEO
                || mediaConnection->mVideoDestinationSet
#endif
                ;
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
   unsigned int maxConnections = ((CpTopologyGraphFactoryImpl*)mpFactoryImpl)->getMaxInputConnections();
   return mMediaConnections.entries() < maxConnections;
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
        // Video receive socket exists, get rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            rtpVideoPort = pMediaConn->mpRtpVideoSocket->getLocalHostPort();

            // Video rtcp port (optional)
            if (pMediaConn->mpRtcpVideoSocket)
            {
                rtcpVideoPort = pMediaConn->mpRtcpVideoSocket->getLocalHostPort();
            }
        }

        // Video pass through enabled
        else if(pMediaConn->mRtpVideoReceivePort > 0 && bRC)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "CpTopologyGraphInterface::getLocalAddresses got video pass through mRtpVideoReceivePort=%d, mRtcpVideoReceivePort=%d",
                pMediaConn->mRtpVideoReceivePort, pMediaConn->mRtcpVideoReceivePort);
            rtpVideoPort = pMediaConn->mRtpVideoReceivePort;
            rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort;
        }
#endif
    }

    return(bRC);
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
        assert(!pMediaConn->mIsCustomSockets);
        if(pMediaConn->mIsCustomSockets)
        {
           return FALSE;
        }

        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (((OsDatagramSocket*)pMediaConn->mpRtpAudioSocket)->getMappedIp(&host, &port))
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
                    if (((OsDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->getMappedIp(&host, &port))
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
            if (((OsDatagramSocket*)pMediaConn->mpRtpVideoSocket)->getMappedIp(&host, &port))
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
                    if (((OsDatagramSocket*)pMediaConn->mpRtcpVideoSocket)->getMappedIp(&host, &port))
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
        assert(!pMediaConn->mIsMulticast && !pMediaConn->mIsCustomSockets);
        if (pMediaConn->mIsMulticast || pMediaConn->mIsCustomSockets)
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
            if (((OsNatDatagramSocket*)pMediaConn->mpRtpVideoSocket)->getRelayIp(&host, &port))
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
                    if (((OsNatDatagramSocket*)pMediaConn->mpRtcpVideoSocket)->getRelayIp(&host, &port))
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

        assert(!pMediaConnection->mAudioDestinationSet);
        pMediaConnection->mAudioDestinationSet = true;

        pMediaConnection->mRtpAudioSendHostAddress.remove(0);
        pMediaConnection->mRtpAudioSendHostPort = 0 ;
        pMediaConnection->mRtcpAudioSendHostPort = 0 ;
#ifdef VIDEO
        assert(!pMediaConnection->mVideoDestinationSet);
        pMediaConnection->mVideoDestinationSet = true;

        pMediaConnection->mRtpVideoSendHostAddress.remove(0);
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
                pMediaConnection->mRtpAudioSendHostAddress = destAddress;
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
                pMediaConnection->mRtpVideoSendHostAddress = destAddress;
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

OsStatus CpTopologyGraphInterface::getAudioRtpSourceIDs(int connectionId,
                                                        unsigned int& uiSendingSSRC,
                                                        unsigned int& uiReceivingSSRC)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpTopologyMediaConnection* mediaConnection = getMediaConnection(connectionId);

   uiReceivingSSRC = 0;

   if (mpTopologyGraph && mediaConnection)
   {
      UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
      MpRtpOutputConnection *pOutConn;
      returnCode = mpTopologyGraph->lookupResource(outConnectionName,
                                             (MpResource*&)pOutConn);

      if (returnCode == OS_SUCCESS)
      {
         uiSendingSSRC = pOutConn->getSSRC();

         mediaConnection->mRtpAudioSending = FALSE;
      }
   }

   return returnCode;
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

OsStatus CpTopologyGraphInterface::getConnectionInputLatency(int &latency,
                                                             MpConnectionID connectionId,
                                                             int streamId)
{
   OsStatus stat = OS_NOT_YET_IMPLEMENTED;
   if (connectionId < 0)
   {
      // Local input from Mic
      MpRtpOutputConnection *pStartResource;
      stat = mpTopologyGraph->lookupResource(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME,
                                             (MpResource*&)pStartResource);
      if (stat != OS_SUCCESS)
      {
         return OS_NOT_FOUND;
      }
      stat = mpTopologyGraph->getLatencyForPath(pStartResource, MpResource::ASSOCIATED_LATENCY,
                                                DEFAULT_BRIDGE_RESOURCE_NAME, 0,
                                                FALSE, latency);
   }
   else
   {
      // Remote (RTP) input
      MpRtpOutputConnection *pStartResource;
      UtlString realStreamName;
      int streamPort;
      CpTopologyMediaConnection* pMediaConnection;

      // Get media connection. We need it for resource topology
      pMediaConnection = getMediaConnection(connectionId);
      if (pMediaConnection == NULL)
      {
         return OS_NOT_FOUND;
      }

      // Get real stream output resource name from resource topology
      { // Scope for virtualStreamName
         UtlString virtualStreamName(VIRTUAL_NAME_RTP_STREAM_OUTPUT);
         virtualStreamName.appendFormat(STREAM_NAME_SUFFIX, streamId);

         stat = pMediaConnection->mpResourceTopology->getOutputVirtualResource(virtualStreamName, 0,
                                                                               realStreamName, streamPort);
         if (stat != OS_SUCCESS)
         {
            return OS_NOT_FOUND;
         }
         MpResourceTopology::replaceNumInName(realStreamName, connectionId);
      }

      // Get real resource for stream output
      stat = mpTopologyGraph->lookupResource(realStreamName,
                                             (MpResource*&)pStartResource);
      if (stat != OS_SUCCESS)
      {
         return OS_NOT_FOUND;
      }

      // Now get latency
      UtlString rtpInputName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(rtpInputName, connectionId);
      stat = mpTopologyGraph->getLatencyForPathReverse(pStartResource, streamPort,
                                                       rtpInputName, MpResource::ASSOCIATED_LATENCY,
                                                       TRUE,
                                                       latency);
   }

   return stat;
}

OsStatus CpTopologyGraphInterface::getConnectionOutputLatency(int &latency,
                                                              MpConnectionID connectionId)
{
   OsStatus stat = OS_NOT_YET_IMPLEMENTED;
   if (connectionId < 0)
   {
      // Local output to Speaker
      MpRtpOutputConnection *pStartResource;
      stat = mpTopologyGraph->lookupResource(DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME,
                                             (MpResource*&)pStartResource);
      if (stat != OS_SUCCESS)
      {
         return OS_NOT_FOUND;
      }
      stat = mpTopologyGraph->getLatencyForPathReverse(pStartResource, MpResource::ASSOCIATED_LATENCY,
                                                       DEFAULT_BRIDGE_RESOURCE_NAME, 0,
                                                       FALSE, latency);
   }
   else
   {
      // Remote (RTP) input
      MpRtpOutputConnection *pStartResource;
      CpTopologyMediaConnection* pMediaConnection;
      UtlString outRtpName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);

      // Get media connection. We need it for resource topology
      pMediaConnection = getMediaConnection(connectionId);
      if (pMediaConnection == NULL)
      {
         return OS_NOT_FOUND;
      }

      // Get resource for stream output
      MpResourceTopology::replaceNumInName(outRtpName, connectionId);
      stat = mpTopologyGraph->lookupResource(outRtpName,
                                             (MpResource*&)pStartResource);
      if (stat != OS_SUCCESS)
      {
         return OS_NOT_FOUND;
      }

      // Now get latency
      stat = mpTopologyGraph->getLatencyForPathReverse(pStartResource, MpResource::ASSOCIATED_LATENCY,
                                                       DEFAULT_BRIDGE_RESOURCE_NAME, 0,
                                                       FALSE,
                                                       latency);
   }

   return stat;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus CpTopologyGraphInterface::createRtpSocketPair(UtlString localAddress,
                                                       int localPort,
                                                       UtlBoolean isMulticast,
                                                       SIPX_CONTACT_TYPE contactType,
                                                       OsSocket* &rtpSocket,
                                                       OsSocket* &rtcpSocket)
{
   int firstRtpPort = localPort;
   bool localPortGiven = (localPort != 0); // Does user specified the local port?

   if (!localPortGiven)
   {
      mpFactoryImpl->getNextRtpPort(localAddress, localPort);
      firstRtpPort = localPort;
   }

//#ifdef TEST_PRINT
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::createRtpSocketPair localPortGiven: %s, localPort: %d firstRtpPort: %d",
       localPortGiven ? "true" : "false", localPort, firstRtpPort);
//#endif

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
               rtpSocket = new OsMulticastSocket(
                        localPort, localAddress,
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
      }
   }

   // Did our sockets get created OK?
   if (!rtpSocket->isOk() || !rtcpSocket->isOk())
   {
       delete rtpSocket;
       delete rtcpSocket;
       rtpSocket = NULL;
       rtcpSocket = NULL;
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
      // TODO:: Implement QoS  request under Windows.  The following works under NT 4.0 and Windows 9x only (http://support.microsoft.com/kb/248611)
      setsockopt (sRtp, IPPROTO_IP, 3 /* IP_TOS */, (char *)&tos, sizeof(int));
      setsockopt (sRtcp, IPPROTO_IP, 3 /* IP_TOS */, (char *)&tos, sizeof(int));
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

int CpTopologyGraphInterface::getNextConnectionId()
{
    return(++mLastConnectionId);
}

CpTopologyMediaConnection* CpTopologyGraphInterface::createMediaConnection(int& connectionId,
                                                                           UtlBoolean isMcast)
{
   CpTopologyMediaConnection* mediaConnection=NULL;
   CpTopologyGraphFactoryImpl* pTopologyFactoryImpl = (CpTopologyGraphFactoryImpl*)mpFactoryImpl;
   MpResourceTopology *pResourceTopology;

   connectionId = getNextConnectionId();
   if (connectionId == -1)
   {
      return NULL;
   }
   pResourceTopology = isMcast ? pTopologyFactoryImpl->getMcastConnectionResourceTopology()
                               : pTopologyFactoryImpl->getConnectionResourceTopology();
   mpTopologyGraph->addResources(*pResourceTopology,
                                 pTopologyFactoryImpl->getResourceFactory(),
                                 connectionId);

   mediaConnection = new CpTopologyMediaConnection(connectionId);
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CpTopologyGraphInterface::createConnection "
                 "creating a new connection %p",
                 mediaConnection);
   mMediaConnections.append(mediaConnection);

   // Save resource topology and RTP streams number
   mediaConnection->mpResourceTopology = pResourceTopology;
   mediaConnection->mNumRtpStreams = isMcast ? pTopologyFactoryImpl->getNumMcastRtpStreams()
                                             : 1;

   // Set codec factory
   mediaConnection->mpCodecFactory = new SdpCodecList(mSupportedCodecs);

   // Don't want to bind any unset payload types until last minute so
   // that we avoid creating payload ID conflicts if we get payload IDs from
   // the other side.
   //mediaConnection->mpCodecFactory->bindPayloadTypes();
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
                 "CpTopologyGraphInterface::createMediaConnection creating a new mpCodecFactory %p",
                 mediaConnection->mpCodecFactory);

   // discardLoopbackRtp() and setConnectionToConnectionWeight() use
   // asynchronous call lookupResource(), so fence is needed.
   mpTopologyGraph->synchronize("CpTopologyGraphInterface::createMediaConnection");

   // Discard looped back RTP packets if multicast is used.
   if (isMcast)
   {
      discardLoopbackRtp(mediaConnection);
   }

   // Disable echoing of local connection to itself. This is the problem
   // if connection have more then one stream (like in multicast case).
   // Then default bridge matrix tries to mix 2nd, 3rd, etc streams
   // to output connection, which is definitely not what we want.
   setConnectionToConnectionWeight(mediaConnection, connectionId, 0);

   return mediaConnection;
}

OsStatus CpTopologyGraphInterface::deleteMediaConnection(CpTopologyMediaConnection* mediaConnection)
{
   if(mediaConnection == NULL)
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "CpTopologyGraphInterface::deleteMediaConnection mediaConnection is NULL!");
      return OS_NOT_FOUND;
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CpTopologyGraphInterface::deleteConnection deleting the connection %p",
                 mediaConnection);

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
                    "CpTopologyGraphInterface::deleteMediaConnection NULL socket: mpRtpAudioSocket=0x%08x, mpRtpAudioSocket=0x%08x",
                    mediaConnection->mpRtpAudioSocket,
                    mediaConnection->mpRtcpAudioSocket);
#endif

   OsStatus returnCode = OS_SUCCESS;
   mediaConnection->mAudioDestinationSet = FALSE;

   // Stop sending RTP
   stopRtpSend(mediaConnection);

   // Stop receiving RTP
   stopRtpReceive(mediaConnection);

   // No need for the fence, because releaseSockets() for audio
   // is an asynchronous call.
#ifdef VIDEO
   // Video sockets are released via message.  So we need to
   // be sure they are removed from NetInTask before removing the
   // resources which process the socket remove message and before
   // the sockets get deleted out from under NetInTask.
   mpTopologyGraph->synchronize("CpTopologyGraphInterface::deleteMediaConnection");
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
       "CpTopologyGraphInterface::deleteMediaConnection done synchronize");
#endif

   if(mediaConnection->getValue() >= 0)
   {
       mpTopologyGraph->destroyResources(*mediaConnection->mpResourceTopology,
                                         mediaConnection->getValue());
       mediaConnection->setValue(-1);
       // I don't think this fence is required.
//       mpTopologyGraph->synchronize();
   }

   if(!mediaConnection->mIsCustomSockets && mediaConnection->mpRtpAudioSocket)
   {
//#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
      //printf(
                    "CpTopologyGraphInterface::deleteMediaConnection deleting RTP socket: %p descriptor: %d",
                    mediaConnection->mpRtpAudioSocket,
                    mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
//#endif

      delete mediaConnection->mpRtpAudioSocket;
      mediaConnection->mpRtpAudioSocket = NULL;

      // Let port manager know port is available
      mpFactoryImpl->releaseRtpPort(mediaConnection->mRtpAudioReceivePort);
   }
   if(!mediaConnection->mIsCustomSockets && mediaConnection->mpRtcpAudioSocket)
   {
//#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
      //printf(
                    "deleting RTCP socket: %p descriptor: %d",
         mediaConnection->mpRtcpAudioSocket,
         mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());
//#endif

      delete mediaConnection->mpRtcpAudioSocket;
      mediaConnection->mpRtcpAudioSocket = NULL;
   }

#ifdef VIDEO
   mediaConnection->mVideoDestinationSet = FALSE;

   if(!mediaConnection->mIsCustomSockets && mediaConnection->mpRtpVideoSocket)
   {
      delete mediaConnection->mpRtpVideoSocket;
      mediaConnection->mpRtpVideoSocket = NULL;

      // Let port manager know this port is not longer used.
      // If pass through is enabled, it may be masking the real port of the video socket
      mpFactoryImpl->releaseRtpPort(!mediaConnection->mVideoPassThroughEnabled ? 
              mediaConnection->mRtpVideoReceivePort : 
              mediaConnection->mpRtpVideoSocket->getLocalHostPort());
   }
   if(!mediaConnection->mIsCustomSockets && mediaConnection->mpRtcpVideoSocket)
   {
      delete mediaConnection->mpRtcpVideoSocket;
      mediaConnection->mpRtcpVideoSocket = NULL;
   }
#endif

   delete mediaConnection;

   return(returnCode);
}

CpTopologyMediaConnection* CpTopologyGraphInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId);
   return((CpTopologyMediaConnection*) mMediaConnections.find(&matchConnectionId));
}

OsStatus CpTopologyGraphInterface::discardLoopbackRtp(CpTopologyMediaConnection* mediaConnection)
{
   MpConnectionID connectionId = mediaConnection->getValue();
   UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
   UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
   MpRtpOutputConnection *pOutputConnection;
   OsStatus stat = mpTopologyGraph->lookupResource(outConnectionName,
                                                   (MpResource*&)pOutputConnection);
   assert(stat == OS_SUCCESS);
   if (stat == OS_SUCCESS)
   {
      // Get our SSRC and start discarding stream with it.
      RtpSRC ssrc = pOutputConnection->getSSRC();
      stat = MpRtpInputConnection::enableSsrcDiscard(inConnectionName,
                                                     *mpTopologyGraph->getMsgQ(),
                                                     TRUE, ssrc);
      assert(stat == OS_SUCCESS);
   }

   return stat;
}

void CpTopologyGraphInterface::stopRtpReceive(CpTopologyMediaConnection* mediaConnection)
{
   MpConnectionID connectionId = mediaConnection->getValue();

#ifdef VIDEO
   UtlString inVideoConnectionName(DEFAULT_VIDEO_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inVideoConnectionName, connectionId);
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
       "CpTopologyGraphInterface::stopRtpReceive sending stop recieve for %s RTP/RTCP sockets %p/%p",
       inVideoConnectionName.data(), mediaConnection->mpRtpVideoSocket, mediaConnection->mpRtcpVideoSocket);
   MprFromNet::resetSockets(inVideoConnectionName, *(mpTopologyGraph->getMsgQ()));
   mediaConnection->mRtpVideoReceiving = FALSE;
#endif

   // Stop receiving data from the network (asynchronous call!)
   UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(inConnectionName, connectionId);
   MpRtpInputConnection *pInputConnection;
   OsStatus tmpStat = mpTopologyGraph->lookupResource(inConnectionName,
                                                      (MpResource*&)pInputConnection);
   assert(tmpStat == OS_SUCCESS);
   pInputConnection->releaseSockets();

   // Deselect codecs and disable decoders
   UtlString decoderName(DEFAULT_DECODE_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(decoderName, connectionId);
   decoderName.append(STREAM_NAME_SUFFIX);
   for (int i=0; i<mediaConnection->mNumRtpStreams; i++)
   {
      UtlString tmpName(decoderName);
      MpResourceTopology::replaceNumInName(tmpName, i);
      MprDecode::deselectCodecs(tmpName, *mpTopologyGraph->getMsgQ());
      MprDecode::disable(tmpName, *mpTopologyGraph->getMsgQ());
   }
}

void CpTopologyGraphInterface::stopRtpSend(CpTopologyMediaConnection* mediaConnection)
{
   MpConnectionID connectionId = mediaConnection->getValue();
   UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
   UtlString encodeName(DEFAULT_ENCODE_RESOURCE_NAME);
   MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
   MpResourceTopology::replaceNumInName(encodeName, connectionId);
   MpRtpOutputConnection *pConnection;
   OsStatus stat = mpTopologyGraph->lookupResource(outConnectionName,
                                                   (MpResource*&)pConnection);
   assert(stat == OS_SUCCESS);

   // Release sockets, deselect codecs and disable encoder.
   pConnection->releaseSockets();
   MprEncode::deselectCodecs(encodeName, *getMsgQ());
   MpResource::disable(encodeName, *getMsgQ());
   MpResource::disable(outConnectionName, *getMsgQ());

   mediaConnection->mRtpAudioSending = FALSE;

#ifdef VIDEO
   // Do the same for video
   if(mediaConnection->mpRtpVideoSocket || mediaConnection->mpRtcpVideoSocket)
   {
      UtlString videoConnectionName(DEFAULT_VIDEO_RTP_OUTPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(videoConnectionName, connectionId);

      MprFromNet::resetSockets(videoConnectionName, *(mpTopologyGraph->getMsgQ()));

      MpResource::disable(videoConnectionName, *mpTopologyGraph->getMsgQ());
   }
   mediaConnection->mRtpVideoSending = FALSE;
#endif
}

OsStatus CpTopologyGraphInterface::setMixWeightForOutput(int bridgeOutputPort, float weight)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "CpTopologyGraphInterface::setMixWeightForOutput(bridgeOutputPort: %d, weight: %f)",
        bridgeOutputPort, weight);
    OsSysLog::flush();
    assert(bridgeOutputPort >= 0);
    // Determine number of bridge ports and allocate array for weights
    int numBridgePorts = getNumBridgePorts();
    if (numBridgePorts < 0)
    {
       assert(!"Can't determine number of bridge ports!");
       numBridgePorts = 20;
    }

    float* weightVector = new float[numBridgePorts];
    UtlString gainVectorString;
    for(int weightIndex = 0; weightIndex < numBridgePorts; weightIndex++)
    {
        // Mutually exclusve mix so a stream does not get its own feedback
        if(weightIndex == bridgeOutputPort)
        {
            weightVector[weightIndex] = 0.0f;
        }
        else
        {
            weightVector[weightIndex] = weight;
        }
        gainVectorString.appendFormat("%f, ", weightVector[weightIndex]);
    }

    OsStatus status = setMixWeightsForOutput(bridgeOutputPort, numBridgePorts, weightVector);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "CpTopologyGraphInterface::setMixWeightForOutput gain vector: {%s} length: %d setMixWeightsForOutput return: %d", 
        gainVectorString.data(), numBridgePorts, status);
    delete[] weightVector;
    weightVector = NULL;

    return(status);
}

OsStatus CpTopologyGraphInterface::setMixWeightsForOutput(int bridgeOutputPort, int numWeights, float weights[])
{
    // Convert in case we are running with fixed point
    MpBridgeGain* gainWeights = new MpBridgeGain[numWeights];
    for(int weightIndex = 0; weightIndex < numWeights; weightIndex++)
    {
        gainWeights[weightIndex] = MPF_BRIDGE_FLOAT(weights[weightIndex]);
    }

    OsStatus status = 
        MprBridge::setMixWeightsForOutput(DEFAULT_BRIDGE_RESOURCE_NAME,
                                          *mpTopologyGraph->getMsgQ(),
                                          bridgeOutputPort,
                                          numWeights,
                                          gainWeights);
    delete[] gainWeights;
    gainWeights = NULL;

    return(status);
}

OsStatus CpTopologyGraphInterface::setConnectionWeightOnBridge(CpTopologyMediaConnection *mediaConnection,
                                                               int destPort,
                                                               float weight)
{
   int i;
   MpConnectionID connectionId = mediaConnection->getValue();

   // Prepare input RTP streams names.
   UtlString inStreamName(VIRTUAL_NAME_RTP_STREAM_OUTPUT);
   MpResourceTopology::replaceNumInName(inStreamName, connectionId);
   inStreamName.append(STREAM_NAME_SUFFIX);

   // Determine number of bridge ports and allocate array for weights
   int numBridgePorts = getNumBridgePorts();
   if (numBridgePorts<0)
   {
      assert(!"Can't determine number of bridge ports!");
      return OS_NOT_FOUND;
   }
   MpBridgeGain *weights = new MpBridgeGain[numBridgePorts];
   MpBridgeGain bridgeWeight = MPF_BRIDGE_FLOAT(weight);

   for (i=0; i<numBridgePorts; i++)
   {
      weights[i] = MP_BRIDGE_GAIN_UNDEFINED;
   }

   for (i=0; i<mediaConnection->mNumRtpStreams; i++)
   {
      UtlString tmpName(inStreamName);
      MpResourceTopology::replaceNumInName(tmpName, i);
      int port;
      OsStatus stat = getResourceInputPortOnBridge(tmpName, 0, port);
      if (stat != OS_SUCCESS)
      {
         assert(!"Can't determine bridge port number for stream!");
         delete[] weights;
         return OS_FAILED;
      }
      assert(port>=0 && port<numBridgePorts);
      weights[port] = bridgeWeight;
   }

   MprBridge::setMixWeightsForOutput(DEFAULT_BRIDGE_RESOURCE_NAME,
                                     *mpTopologyGraph->getMsgQ(),
                                     destPort,
                                     numBridgePorts,
                                     weights);

   delete[] weights;

   return OS_SUCCESS;
}

OsStatus CpTopologyGraphInterface::setConnectionToConnectionWeight(CpTopologyMediaConnection *srcConnection,
                                                                   int destConnectionId,
                                                                   float weight)
{
   OsStatus stat = OS_INVALID_ARGUMENT;
   // Verify valid connectionId
   CpTopologyMediaConnection* pDestConnection = getMediaConnection(destConnectionId);

   if(pDestConnection != NULL)
   {
       int destPort;
       stat = getConnectionPortOnBridge(destConnectionId, 0, destPort);
       if (stat != OS_SUCCESS)
       {
          return stat;
       }
       assert(destPort >= 0);

       stat = setConnectionWeightOnBridge(srcConnection, destPort, weight);
   }
   return stat;
}

OsStatus CpTopologyGraphInterface::setMicGain(float gain)
{
#if 0
   setMicWeightOnBridge(weight);
#else
   OsSysLog::add(FAC_CP, PRI_INFO,
                 "CpTopologyGraphInterface::setMicGain(%f)\n", gain);

   // Set Gain in MprFromInputDevice
   MprFromInputDevice::setGain(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME,
                               *mpTopologyGraph->getMsgQ(),
                               MPF_BRIDGE_FLOAT(gain));

   return OS_SUCCESS;
#endif
}

OsStatus CpTopologyGraphInterface::setMicWeightOnBridge(float weight)
{
   // Prepare input RTP streams names.
   UtlString inStreamName(VIRTUAL_NAME_LOCAL_STREAM_OUTPUT);

   // Determine number of bridge ports
   int numBridgePorts = getNumBridgePorts();
   if (numBridgePorts<0)
   {
      assert(!"Can't determine number of bridge ports!");
      return OS_NOT_FOUND;
   }

   // Get the port number of the Mic stream
   int port;
   OsStatus stat = getResourceInputPortOnBridge(inStreamName, 0, port);
   if (stat != OS_SUCCESS)
   {
      assert(!"Can't determine bridge port number for stream!");
      return OS_FAILED;
   }
   assert(port>=0 && port<numBridgePorts);

   // Allocate array for weights
   MpBridgeGain *weights = new MpBridgeGain[numBridgePorts];
   MpBridgeGain bridgeWeight = MPF_BRIDGE_FLOAT(weight);

   // Fill in the weights vector
   for (int i=0; i<numBridgePorts; i++)
   {
      weights[i] = bridgeWeight;
   }
   weights[port] = 0;

   // Set weights
   MprBridge::setMixWeightsForInput(DEFAULT_BRIDGE_RESOURCE_NAME,
                                    *mpTopologyGraph->getMsgQ(),
                                    port,
                                    numBridgePorts,
                                    weights);

   delete[] weights;

   return OS_SUCCESS;
}

/* ============================ FUNCTIONS ================================= */
