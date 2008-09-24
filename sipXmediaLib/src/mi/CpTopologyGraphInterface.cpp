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
#include <os/OsMediaContact.h>
#include <mp/MpTopologyGraph.h>
#include <mp/MpResourceTopology.h>
#include <mp/MprBufferRecorder.h>
#include <mp/MprToneGen.h>
#include <mp/MprFromFile.h>
#include <mp/MpRtpInputAudioConnection.h>
#include <mp/MpRtpOutputAudioConnection.h>
#include <mp/dtmflib.h>
#include <mp/MpMediaTask.h>
#include <mp/MpCodecFactory.h>
#ifdef WIN32
#include <mp/DmaTask.h>
#endif
#include "mi/CpTopologyGraphInterface.h"
#include "mi/CpTopologyGraphFactoryImpl.h"
#include "mi/CpTopologyPacketPusher.h"
#include "mediaBaseImpl/CpMediaSocketAdapter.h"
#include "ARS/ArsReceiveAdapter.h"
#include "upnp/UPnpAgent.h"



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


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphInterface::CpTopologyGraphInterface(CpTopologyGraphFactoryImpl* pFactoryImpl,
                                                   uint32_t samplesPerFrame,
                                                   uint32_t samplesPerSec, 
                                                   const ProxyDescriptor& stunServer,
                                                   const ProxyDescriptor& turnServer,
                                                   const char* publicAddress,
                                                   const char* localAddress,
                                                   int numCodecs,
                                                   SdpCodec* sdpCodecArray[],
                                                   const char* locale,
                                                   int expeditedIpTos,
                                                   UtlBoolean enableIce
                                                  )
    : CpMediaInterface()
      
{
    mpFactoryImpl = pFactoryImpl;
    mpMediaGuard = mpFactoryImpl->getLock() ;
    assert(mpMediaGuard != NULL) ;
    mLastConnectionId = 0;

   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new CpMediaInterface %p",
                 this);

   mpTopologyGraph = new MpTopologyGraph(samplesPerFrame,
                                         samplesPerSec,
                                         *(pFactoryImpl->getInitialResourceTopology()),
                                         *(pFactoryImpl->getResourceFactory()));
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::CpTopologyGraphInterface creating a new MpTopologyGraph %p",
                 mpTopologyGraph);

   mStunServer = stunServer;
   mTurnProxy = turnServer;
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

       // Assign any unset payload types
       mSupportedCodecs.bindPayloadTypes();
   }
   else
   {
       MpCodecFactory *pCodecFactory = MpCodecFactory::getMpCodecFactory();
       pCodecFactory->addCodecsToList(mSupportedCodecs);

       if (OsSysLog::willLog(FAC_CP, PRI_INFO))
       {
          UtlString codecsList;
          mSupportedCodecs.toString(codecsList);
          OsSysLog::add(FAC_CP, PRI_INFO, "CpTopologyGraphInterface::CpTopologyGraphInterface hard-coded codec factory %s ...",
                        codecsList.data());
       }
   }

   mExpeditedIpTos = expeditedIpTos;
}


// Destructor
CpTopologyGraphInterface::~CpTopologyGraphInterface()
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphInterface::~CpTopologyGraphInterface deleting the CpMediaInterface %p",
                 this);

    CpMediaConnection* mediaConnection = NULL;
    while ((mediaConnection = (CpMediaConnection*) mMediaConnections.get()))
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
                                     bool bInitiating,
                                     const char* szLocalAddress,
                                     int localPort,
                                     void* videoWindowHandle,
                                     unsigned long flags,
                                     void* const pSecurityAttributes,
                                     ISocketEvent* pIdleEvent,
                                     IMediaEventListener* pMediaEventListener,
                                     const RtpTransportOptions rtpTransportOptions,
                                     int callHandle)
{
   OsStatus retValue = OS_SUCCESS;
   CpMediaConnection* mediaConnection=NULL;
   CpTopologyGraphFactoryImpl* pTopologyFactoryImpl = (CpTopologyGraphFactoryImpl*)mpFactoryImpl;

   connectionId = getNextConnectionId();
   if (connectionId == -1)
   {
      return OS_LIMIT_REACHED;
   }

   mpTopologyGraph->addResources(*pTopologyFactoryImpl->getConnectionResourceTopology(),
                                 pTopologyFactoryImpl->getResourceFactory(),
                                 connectionId);

   mediaConnection = new CpMediaConnection(connectionId);
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CpTopologyGraphInterface::createConnection "
                 "creating a new connection %p",
                 mediaConnection);

   mediaConnection->mbEnableLocal = ((flags & CPMI_FLAGS_ENABLE_LOCAL) == CPMI_FLAGS_ENABLE_LOCAL) ;
   mediaConnection->mbEnableSTUN = ((flags & CPMI_FLAGS_ENABLE_STUN) == CPMI_FLAGS_ENABLE_STUN) ;
   mediaConnection->mbEnableTURN = ((flags & CPMI_FLAGS_ENABLE_TURN) == CPMI_FLAGS_ENABLE_TURN) ;
   mediaConnection->mbEnableARS = ((flags & CPMI_FLAGS_ENABLE_ARS) == CPMI_FLAGS_ENABLE_ARS) ;
   mediaConnection->mbEnableICE = ((flags & CPMI_FLAGS_ENABLE_ICE) == CPMI_FLAGS_ENABLE_ICE) ;

   mediaConnection->mRtpTransport = (RTP_TRANSPORT_UDP | RTP_TRANSPORT_TCP) & rtpTransportOptions;
   RtpTcpRoles role = rtpTransportOptions & (RTP_TCP_ROLE_ACTIVE | RTP_TCP_ROLE_PASSIVE | RTP_TCP_ROLE_ACTPASS);
   mediaConnection->setTcpRole(role);


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
   mediaConnection->mbInitiating = bInitiating ;
   // Create the sockets for audio stream
   OsNatDatagramSocket* pRtpSocket;
   OsNatDatagramSocket* pRtcpSocket;
   retValue = createRtpSocketPair(mediaConnection->mLocalAddress,
                                  localPort,
                                  mediaConnection->mContactType,
                                  pRtpSocket,
                                  pRtcpSocket,
                                  mediaConnection);
   mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX] = pRtpSocket;
   mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX] = pRtcpSocket;

   mediaConnection->mpAudioSocketAdapterArray[0] = new 
            CpMediaSocketAdapter(MEDIA_TYPE_AUDIO, 
            pRtpSocket, 
            pRtcpSocket,
            mpMediaPacketCallback) ;


    if (mArsProxy.isValid() && mediaConnection->mbEnableARS)
    {
        mediaConnection->mpArsAudioSocket = getArsSocket(
            mArsProxy, mArsHttpProxy, ARS_MIMETYPE_AUDIO, mediaConnection->mLocalAddress) ;
        mediaConnection->mpArsAudioRAdapter = new ArsReceiveAdapter(MEDIA_TYPE_AUDIO,
                new CpTopologyPacketPusher(mpTopologyGraph),
                mpMediaGuard) ;

        mediaConnection->mpArsAudioSocket->setPacketHandler(mediaConnection->mpArsAudioRAdapter) ;

        mAudioMediaConnectivityInfo.setArsServer(mArsProxy.getAddress()) ;
        mAudioMediaConnectivityInfo.setArsHttpsProxy(mArsHttpProxy.getAddress()) ;

    }

   
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
                                              *(mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX])->getSocket(),
                                              *(mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX])->getSocket(),
                                              NULL);

   // Store audio stream settings
   mediaConnection->mRtpAudioReceivePort = mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort() ;
   mediaConnection->mRtcpAudioReceivePort = mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getLocalHostPort() ;

   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new RTP socket: %p descriptor: %d",
            mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket(), 
            mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getSocketDescriptor());
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new RTCP socket: %p descriptor: %d",
            mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket(), 
            mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getSocketDescriptor());

   // Set codec factory
   mediaConnection->mpCodecFactory = new SdpCodecList(mSupportedCodecs);
   mediaConnection->mpCodecFactory->bindPayloadTypes();
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphInterface::createConnection creating a new mpCodecFactory %p",
            mediaConnection->mpCodecFactory);

    return retValue;
}

OsStatus CpTopologyGraphInterface::setPlcMethod(int connectionId,
                                                const UtlString &methodName)
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;

   if (mpTopologyGraph)
   {
      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);

      MpRtpInputAudioConnection::setPlc(inConnectionName,
                                        *(mpTopologyGraph->getMsgQ()),
                                        methodName);

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
                                    UtlSList* pAudioContacts,
                                    UtlSList* pVideoContacts,                                    
                                    SdpCodecList& supportedCodecs,
                                    SdpSrtpParameters& srtpParams,
                                    int bandWidth,
                                    int& videoBandwidth,
                                    int& videoFramerate)

{   
    OsStatus rc = OS_FAILED ;
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    videoBandwidth = 0 ;
    if (pMediaConn)
    {
        if (pMediaConn->mbEnableTURN && mTurnProxy.isValid())
            doEnableTurn(pMediaConn, true) ;

        if (pAudioContacts && pVideoContacts)
        {
            bool bAddAudio = (mAudioMediaConnectivityInfo.getNumLocalCandidates() == 0) ;
            bool bAddVideo = (mVideoMediaConnectivityInfo.getNumLocalCandidates() == 0) ;

            switch (pMediaConn->mContactType)
            {
                case CONTACT_LOCAL:
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
                case CONTACT_RELAY:
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
                default:
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
            }

            if ((pAudioContacts->entries() > 0) || (pVideoContacts->entries() > 0))
            {
                rc = OS_SUCCESS ;
            }
        }
        else
        {
            rc = OS_SUCCESS ;
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



OsStatus CpTopologyGraphInterface::addVideoRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }
    }
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
   CpMediaConnection* mediaConnection = getMediaConnection(connectionId);

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
       bool bAudioFailure = false ;
       bool bVideoFailure = false ;
       if (!applyAlternateDestinations(connectionId, bAudioFailure, bVideoFailure) )
       {
            IMediaEventListener* pListener = getMediaListener(connectionId) ;
            if (pListener)
            {
                if (bAudioFailure)
                    pListener->onIceFailed(IDevice_Audio) ;

                if (bVideoFailure)
                    pListener->onIceFailed(IDevice_Video) ;                    
            }
       }
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
           (mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()), 
           (mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()),
           mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getSocketDescriptor(),
           mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()->getSocketDescriptor());
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

       if (mediaConnection->mRtpSendingAudio)
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
                                              mediaConnection->mpAudioSocketAdapterArray[UDP_TRANSPORT_INDEX],
                                              audioCodec,
                                              dtmfCodec);
       mediaConnection->mRtpSendingAudio = TRUE;
       returnCode = OS_SUCCESS;
   }
   return returnCode;
}


OsStatus CpTopologyGraphInterface::startRtpReceive(int connectionId,
                                                int numCodecs,
                                                SdpCodec* receiveCodecs[])
{
   OsStatus returnCode = OS_NOT_FOUND;

   CpMediaConnection* mediaConnection = getMediaConnection(connectionId);

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
           (mediaConnection->getRtpAudioSocket()),
           (mediaConnection->getRtcpAudioSocket()),
           mediaConnection->getRtpAudioSocket()->getSocket()->getSocketDescriptor(),
           mediaConnection->getRtcpAudioSocket()->getSocket()->getSocketDescriptor());
      for (i=0; i<numCodecs; i++) {
          osPrintf("   %d:  i:%d .. x:%d\n", i+1,
                   receiveCodecs[i]->getCodecType(),
                   receiveCodecs[i]->getCodecPayloadFormat());
      }
#endif

      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);

      if(mediaConnection->mRtpReceivingAudio)
      {
         // This is not supposed to be necessary and may be
         // causing an audible glitch when codecs are changed
         //mpTopologyGraph->stopReceiveRtp(connectionId);
      }


      MpRtpInputAudioConnection::startReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                 inConnectionName,
                                                 receiveCodecs,
                                                 numCodecs,
                                                 *(mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()),
                                                 *(mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket()));

      mediaConnection->mRtpReceivingAudio = TRUE;

      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus CpTopologyGraphInterface::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   if (mpTopologyGraph && mediaConnection &&
       mediaConnection->mRtpSendingAudio)
   {
      UtlString outConnectionName(DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(outConnectionName, connectionId);
      MpRtpOutputAudioConnection::stopSendRtp(*(mpTopologyGraph->getMsgQ()),
                                               outConnectionName);

      mediaConnection->mRtpSendingAudio = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus CpTopologyGraphInterface::stopRtpReceive(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

#ifdef TEST_PRINT
   printf("CpTopologyGraphInterface::stopRtpReceive(%d) mpTopologyGraph %p mediaCOnnection: %p receiving: %s\n", 
       connectionId,
       mpTopologyGraph,
       mediaConnection, 
       mediaConnection && mediaConnection->mRtpReceivingAudio ? "TRUE" : "FALSE");
#endif

   if (mpTopologyGraph && mediaConnection &&
       mediaConnection->mRtpReceivingAudio)
   {
      UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
      MpResourceTopology::replaceNumInName(inConnectionName, connectionId);

#ifdef TEST_PRINT
      printf("sending stopReceiveRtp message RTP socket: %p RTCP socket: %p\n",
             mediaConnection->getRtpAudioSocket(),
             mediaConnection->getRtcpAudioSocket());
#endif

      MpRtpInputAudioConnection::stopReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                inConnectionName);
      
      mediaConnection->mRtpReceivingAudio = FALSE;
      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus CpTopologyGraphInterface::deleteConnection(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   CpMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   UtlInt matchConnectionId(connectionId);
   mMediaConnections.remove(&matchConnectionId) ;

   returnCode = doDeleteConnection(mediaConnection);

   delete mediaConnection ;

   return(returnCode);
}

OsStatus CpTopologyGraphInterface::doDeleteConnection(CpMediaConnection* mediaConnection)
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
   if (mediaConnection && mediaConnection->getRtpAudioSocket() && mediaConnection->getRtcpAudioSocket())
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "stopping RTP/RTCP send & receive sockets %p/%p descriptors: %d/%d",
                    mediaConnection->getRtpAudioSocket(),
                    mediaConnection->getRtcpAudioSocket(),
                    mediaConnection->getRtpAudioSocket()->getSocket()->getSocketDescriptor(),
                    mediaConnection->getRtcpAudioSocket()->getSocket()->getSocketDescriptor());
   else 
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "CpTopologyGraphInterface::doDeleteConnection NULL socket: mpRtpAudioSocket=0x%08x, mpRtpAudioSocket=0x%08x",
                    mediaConnection->getRtpAudioSocket(),
                    mediaConnection->getRtcpAudioSocket());
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

   if(mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket())
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                    "CpTopologyGraphInterface::doDeleteConnection deleting RTP socket: %p descriptor: %d",
                    mediaConnection->getRtpAudioSocket(),
                    mediaConnection->getRtpAudioSocket()->getSocket()->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket();
      mediaConnection->mpRtpAudioSocketArray[UDP_TRANSPORT_INDEX] = NULL;
   }
   if(mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket())
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "deleting RTCP socket: %p descriptor: %d",
            mediaConnection->getRtcpAudioSocket(),
            mediaConnection->getRtcpAudioSocket()->getSocket()->getSocketDescriptor());
#endif

      delete mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX]->getSocket();
      mediaConnection->mpRtcpAudioSocketArray[UDP_TRANSPORT_INDEX] = NULL;
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
          *mpTopologyGraph->getMsgQ(), mpTopologyGraph->getSamplesPerSec(),
          filename, repeat, event);
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
                                           int downScaling)
{
    OsStatus stat = OS_NOT_FOUND;
    if(mpTopologyGraph && buf)
    {
       uint32_t fgRate = mpTopologyGraph->getSamplesPerSec();
       // Currently, this ignores "local/remote", "mixWithMic" and "downScaling".
       // I also don't know what to do with this blasted protected event.. sending NULL now.
       // If it was an OsNotification -- no problem..
       stat = MprFromFile::playBuffer(DEFAULT_FROM_FILE_RESOURCE_NAME, 
          *mpTopologyGraph->getMsgQ(), buf, bufSize, bufRate, fgRate, type, repeat, NULL);

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

   CpMediaConnection* mediaConnection = NULL;
   UtlSListIterator connectionIterator(mMediaConnections);
   while ((mediaConnection = (CpMediaConnection*) connectionIterator()))
   {
      mediaConnection->mpCodecFactory->setCodecCPULimit(iLimit) ;
   }
}

OsStatus CpTopologyGraphInterface::stopRecording()
{
   return OS_NOT_SUPPORTED;
}

OsStatus CpTopologyGraphInterface::recordMic(int ms, UtlString* pAudioBuffer)
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpTopologyGraph != NULL)
   {
      stat = MprBufferRecorder::startRecording(
                DEFAULT_BUFFER_RECORDER_RESOURCE_NAME,
                *mpTopologyGraph->getMsgQ(), ms, pAudioBuffer);
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
                                                              char*  szReport,
                                                              size_t reprotSize)
{
	return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

int CpTopologyGraphInterface::getNextConnectionId()
{
    return(++mLastConnectionId);
}

// Calculate the current cost for our sending/receiving codecs
int CpTopologyGraphInterface::getCodecCPUCost()
{   
   SdpCodec::SdpCodecCPUCost iCost = SdpCodec::SDP_CODEC_CPU_LOW ;   

   if (mMediaConnections.entries() > 0)
   {      
      CpMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlSListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpMediaConnection*) connectionIterator()))
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
      CpMediaConnection* mediaConnection = NULL;

      // Iterate the connections and determine the most expensive supported 
      // codec.
      UtlSListIterator connectionIterator(mMediaConnections);
      while ((mediaConnection = (CpMediaConnection*) connectionIterator()))
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
    UtlString codecType;
    CpMediaConnection* pConnection = getMediaConnection(connectionId);
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
    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);

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

const void* CpTopologyGraphInterface::getVideoWindowDisplay()
{
   return NULL;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
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

    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
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

    CpMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if(mediaConnection)
    {
        UtlString* oldProperty = (UtlString*)(mediaConnection->mConnectionProperties.findValue(&propertyName));
        if(oldProperty)
        {
            propertyValue = *oldProperty;
            returnCode = OS_SUCCESS;
        }
    }

    return OS_NOT_YET_IMPLEMENTED ; // returnCode 
}

OsStatus CpTopologyGraphInterface::getMediaDeviceInfo(int connectionId,
                                                      MediaDeviceInfo::MediaDeviceInfoType type,
                                                      MediaDeviceInfo& info) 
{
    OsStatus rc = OS_FAILED ;
    UtlString device ;

    info.reset() ;
    switch (type)
    {
        case MediaDeviceInfo::MDIT_AUDIO_INPUT: 
            {
                // getMediaProperty(connectionId, "audioInput1.device", device) ;
                MediaDeviceInfo audioInput(MediaDeviceInfo::MDIT_AUDIO_INPUT) ;
#ifdef WIN32
                audioInput.setSelected(DmaTask::getMicDevice().data()) ;
#endif
                info = audioInput ;
                rc = OS_SUCCESS ;
            }
            break ;
        case MediaDeviceInfo::MDIT_AUDIO_OUTPUT: 
            {
                // getMediaProperty(connectionId, "audioOutput1.speakerDevice", device) ;
                MediaDeviceInfo audioOutput(MediaDeviceInfo::MDIT_AUDIO_OUTPUT) ;
#ifdef WIN32
                audioOutput.setSelected(DmaTask::getCallDevice().data()) ;
#endif
                info = audioOutput ;
                rc = OS_SUCCESS ;
            }
            break ;
        default:
            break ;
    }
    return rc ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus CpTopologyGraphInterface::createRtpSocketPair(UtlString localAddress,
                                                       int localPort,
                                                       SIPX_CONTACT_TYPE contactType,
                                                       OsNatDatagramSocket* &rtpSocket,
                                                       OsNatDatagramSocket* &rtcpSocket,
                                                       CpMediaConnection* pMediaConnection)
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
       ((OsNatDatagramSocket*)rtpSocket)->setTransparentStunRead(false);
       rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                            localAddress, NULL);

       ((OsNatDatagramSocket*)rtcpSocket)->setTransparentStunRead(false);

       UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
       MpResourceTopology::replaceNumInName(inConnectionName, pMediaConnection->getValue());

       MpRtpInputAudioConnection::prepareStartReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                              inConnectionName,
                                              *rtpSocket,
                                              *rtcpSocket);

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


            UtlString inConnectionName(DEFAULT_RTP_INPUT_RESOURCE_NAME);
            MpResourceTopology::replaceNumInName(inConnectionName,pMediaConnection->getValue());
        //      printf("doDeleteConnection sending stopReceiveRtp message RTP socket: %p RTCP socket: %p\n",
        //             mediaConnection->mpRtpAudioSocket,
        //             mediaConnection->mpRtcpAudioSocket);
            MpRtpInputAudioConnection::stopReceiveRtp(*(mpTopologyGraph->getMsgQ()),
                                                        inConnectionName);

            rtpSocket->close();
            delete rtpSocket;
            rtcpSocket->close();
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
               ((OsNatDatagramSocket*)rtpSocket)->setTransparentStunRead(false);

               rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                                   localAddress, NULL);
               ((OsNatDatagramSocket*)rtcpSocket)->setTransparentStunRead(false);
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

   UPnpAgent::getInstance()->bindToAvailablePort(localAddress.data(), localPort, UPnpAgent::getInstance()->getTimeoutSeconds());
   UPnpAgent::getInstance()->bindToAvailablePort(localAddress.data(), localPort+1, UPnpAgent::getInstance()->getTimeoutSeconds());

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
      if (mStunServer.isValid() && pMediaConnection->mbEnableSTUN)
      {
         // Enable Stun if we have a server and either non-local contact type or 
         // ICE is enabled.
         if ((contactType != CONTACT_LOCAL) || mEnableIce)
         {
            ((OsNatDatagramSocket*)rtpSocket)->enableStun(mStunServer.getAddress(), mStunServer.getPort(),
                  mStunServer.getKeepalive(), 0, false) ;
            mAudioMediaConnectivityInfo.setStunServer(mStunServer.getAddress()) ;
         }

         // Enable Stun if we have a server and either non-local contact type or 
         // ICE is enabled.
         if ((contactType != CONTACT_LOCAL) || mEnableIce)
         {
            ((OsNatDatagramSocket*)rtcpSocket)->enableStun(mStunServer.getAddress(), mStunServer.getPort(),
                  mStunServer.getKeepalive(), 0, false) ;
         }
      }


      if (mTurnProxy.isValid() && pMediaConnection->mbEnableTURN)
      {
         // Enable Turn if we have a server and either non-local contact type or 
         // ICE is enabled.
         if ((contactType != CONTACT_LOCAL) || mEnableIce)
         {
            ((OsNatDatagramSocket*)rtpSocket)->enableTurn(mTurnProxy.getAddress(), mTurnProxy.getPort(),
                  mTurnProxy.getKeepalive(), mTurnProxy.getUsername(), mTurnProxy.getPassword(), false) ;
            mAudioMediaConnectivityInfo.setTurnServer(mTurnProxy.getAddress()) ;
               
         }

         // Enable Turn if we have a server and either non-local contact type or 
         // ICE is enabled.
         if ((contactType != CONTACT_LOCAL) || mEnableIce)
         {
            ((OsNatDatagramSocket*)rtcpSocket)->enableTurn(mTurnProxy.getAddress(), mTurnProxy.getPort(),
                  mTurnProxy.getKeepalive(), mTurnProxy.getUsername(), mTurnProxy.getPassword(), false) ;
         }
      }
   }

   return OS_SUCCESS;
}

ArsConnectionSocket* CpTopologyGraphInterface::getArsSocket(const ProxyDescriptor& arsProxy,
                                                            const ProxyDescriptor& arsHttpsProxy,
                                                            ARS_MIMETYPE mimeType,
                                                            const char* szLocalAddress) 
{
    ArsConnectionSocket* pSocket = new ArsConnectionSocket(
            arsProxy.getAddress(), arsProxy.getPort(),
            arsProxy.getUsername(),
            mimeType,
            szLocalAddress) ;

    if (arsHttpsProxy.isValid())
    {
        pSocket->setHttpsProxy(
                arsHttpsProxy.getAddress(),
                arsHttpsProxy.getPort(),
                arsHttpsProxy.getUsername(),
                arsHttpsProxy.getPassword()) ;
    }

#if VSF_TRACE
    printf("VoiceEngineSocketFactory::getArsSocket 0x%08X\n", pSocket) ;
#endif

    return pSocket ;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


