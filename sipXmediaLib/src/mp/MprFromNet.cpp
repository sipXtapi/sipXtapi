//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2006-2016 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include <rtcp/RtcpConfig.h>

#undef WANT_RTCP_LOG_MESSAGES

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#include <os/OsIntTypes.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef _VXWORKS
#include <inetlib.h>
#endif

// APPLICATION INCLUDES
#include <mp/MprFromNet.h>
#include <mp/MpMisc.h>
#include <mp/MpUdpBuf.h>
#include <mp/MprRtpDispatcher.h>
#ifdef INCLUDE_RTCP /* [ */
#include <rtcp/RTPHeader.h>
#include <rtcp/RTCPHeader.h>
#endif /* INCLUDE_RTCP ] */
#include <os/OsEvent.h>
#include <os/OsMutex.h>
#include <os/OsDefs.h>
#include <mp/MpResourceMsg.h>
#include <mp/MpSetSocketsMsg.h>
#include <mp/NetInTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpDtlsPacketMsg.h>
#include <mp/MpDtls.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

//#define ENABLE_MULTIPLE_NETINTASKS

MprFromNet::MprFromNet()
: mDiscardCtlMutex(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
, mRegistrationSyncMutex(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
#ifdef ENABLE_MULTIPLE_NETINTASKS
, mNetInTask(NetInTask::createNetInTask())
#else
, mNetInTask(NetInTask::getNetInTask())
#endif
, mRegistered(FALSE)
, mpRtpDispatcher(NULL)
, mDiscardSelectedStream(FALSE)
, mDiscardedSSRC(0)
, mpFlowGraph(NULL)
, mpDtls(NULL)
#ifdef INCLUDE_RTCP /* [ */
, mpiRTCPDispatch(NULL)
, mpiRTPDispatch(NULL)
#else /* INCLUDE_RTCP ] [ */
, mRtcpCount(0)
#endif /* INCLUDE_RTCP ] */
, mNumPushed(0)
, mNumPktsRtcp(0)
, mNumPktsRtp(0)
, mNumEncDropped(0)
, mNumLoopDropped(0)
{
#ifndef INCLUDE_RTCP /* [ */
   mInRtpHandle  = StartRtpSession(NULL, RTP_DIR_IN, (char) -1);
#endif /* INCLUDE_RTCP ] */
}

MprFromNet::~MprFromNet()
{
   resetSockets();

#ifdef INCLUDE_RTCP /* [ */
// Release the references held to the RTP and RTCP Dispatchers used for
// routing packets to the RTCP component
   if(mpiRTPDispatch)
   {
      mpiRTPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
      mpiRTPDispatch = NULL;
   }

   if(mpiRTCPDispatch)
   {
      mpiRTCPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
      mpiRTCPDispatch = NULL;
   }
#else /* INCLUDE_RTCP ] [ */
   if (NULL != mInRtpHandle)
      FinishRtpSession(mInRtpHandle);
   mInRtpHandle  = NULL;
#endif /* INCLUDE_RTCP ] */

#ifdef ENABLE_MULTIPLE_NETINTASKS
   mNetInTask->destroy();
#endif
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprFromNet::setSockets(const UtlString& resourceName,
                                OsMsgQ& flowgraphQueue,
                                OsSocket* rtpSocket,
                                OsSocket* rtcpSocket)
{
    MpSetSocketsMsg message(resourceName, rtpSocket, rtcpSocket);

    return(flowgraphQueue.send(message, OsTime::OS_INFINITY /*sOperationQueueTimeout*/));
}

OsStatus MprFromNet::setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   OsStatus res;
   OsEvent notify;

   // We should release mutex before blocking on notify event to avoid deadlock.
   {
      OsLock lock(mRegistrationSyncMutex);

      resetSocketsInternal();
      res = mNetInTask->addNetInputSources(&rRtpSocket, &rRtcpSocket, this, &notify);
      assert(res == OS_SUCCESS);

      mRegistered = TRUE;
   }
   notify.wait();

   return OS_SUCCESS;
}

OsStatus MprFromNet::resetSockets(const UtlString& resourceName, OsMsgQ& flowgraphMessageQueue)
{
   MpResourceMsg message(MpResourceMsg::MPRM_RESET_SOCKETS, resourceName);

   return(flowgraphMessageQueue.send(message, OsTime::OS_INFINITY /*sOperationQueueTimeout*/));
}

OsStatus MprFromNet::resetSockets()
{
   UtlBoolean needWait;
   OsEvent notify;

   // We should release mutex before blocking on notify event to avoid deadlock.
   {
      OsLock lock(mRegistrationSyncMutex);
      needWait = resetSocketsInternal(&notify);
   }

   if (needWait)
   {
      notify.wait();
   }

   return OS_SUCCESS;
}

UtlBoolean MprFromNet::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   switch (rMsg.getMsg())
   {
       case MpResourceMsg::MPRM_SET_SOCKETS:
       {
           MpSetSocketsMsg* pMsg = (MpSetSocketsMsg*)&rMsg;
           OsSocket* rtpSocket = pMsg->getRtpSocket();
           OsSocket* rtcpSocket = pMsg->getRtcpSocket();
           assert(rtpSocket);
           assert(rtcpSocket);
           setSockets(*rtpSocket, *rtcpSocket);
           handled = TRUE;
       }
       break;

       case MpResourceMsg::MPRM_DTLS_PACKET:
       {
          if (mpDtls != NULL)
          {
             MpDtlsPacketMsg* pMsg = (MpDtlsPacketMsg*)&rMsg;
             const UtlString& packet = pMsg->getPacket();
             mpDtls->processIncomingPacket(packet.data(), packet.length());
          }
          handled = TRUE;
       }
       break;

       case MpResourceMsg::MPRM_DTLS_RETRANSMIT:
       {
          if (mpDtls != NULL)
          {
             mpDtls->handleRetransmit();
          }
          handled = TRUE;
       }
       break;

       case MpResourceMsg::MPRM_DTLS_HANDSHAKE_TIMEOUT:
       {
          if (mpDtls != NULL)
          {
             mpDtls->handleHandshakeTimeout();
          }
          handled = TRUE;
       }
       break;

       default:
           OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::handleMessage unhandled message type: %d",
               rMsg.getMsg());
           OsSysLog::flush();
           assert(0);
   }

   return(handled);
}

UtlBoolean MprFromNet::setSrtpParams(SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey)
{
   return mSrtp.setSrtpParams(cryptoSuite, cryptoKey, TRUE /* forUnprotect? */);
}

void MprFromNet::setDtls(MpDtls* pDtls, const UtlString& resourceName)
{
   mpDtls = pDtls;
   mDtlsResourceName = resourceName;
}

#ifndef INCLUDE_RTCP /* [ */
OsStatus MprFromNet::getRtcpStats(MprRtcpStats& stats)
{
   stats.ssrc = mInRtpHandle->ssrc;
   stats.seqNumCycles = mInRtpHandle->cycles;
   stats.highSeqNum = mInRtpHandle->seq;
   return OS_SUCCESS;
}

OsStatus MprFromNet::rtcpStats(struct RtpHeader* rtpH)
{
   if (mInRtpHandle->ssrc != rtpH->ssrc)
   {
       const char* name =
#ifdef INCLUDE_RTCP /* [ */
           getName();
#else
           "";
#endif /* INCLUDE_RTCP ] */

      static int firstfew = 0;

      if (firstfew++ < 20)
      {
         if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
            osPrintf("%s: New SSRC (0x%08X) detected\n", name, rtpH->ssrc);
      }
      mInRtpHandle->ssrc = rtpH->ssrc;
      mInRtpHandle->seq = rtpH->seq - 1;
      mInRtpHandle->cycles = 0;
      mRtcpCount = 0;
   }
   if (mInRtpHandle->seq < rtpH->seq)
   {
      mInRtpHandle->seq = rtpH->seq;
   }
   else
   {
      if ((mInRtpHandle->seq > 0xc000) && (rtpH->seq < 0x4000))
      {
         mInRtpHandle->cycles++;
      }
   }
   return OS_SUCCESS;
}
#endif /* INCLUDE_RTCP ] */

OsStatus MprFromNet::pushPacket(const MpUdpBufPtr &udpBuf, bool isRtcp)
{
   OsLock lock(mDiscardCtlMutex);
   MpRtpBufPtr rtpBuf;
   OsStatus ret = OS_SUCCESS;

#ifdef INCLUDE_RTCP /* [ */
   CRTPHeader   oRTPHeader;
#endif /* INCLUDE_RTCP ] */

   mNumPushed++;

   const char* packetData = udpBuf->getDataPtr();
   int packetSize = udpBuf->getPacketSize();

   // RFC 7983 demultiplex when DTLS-SRTP is configured for this connection.
   // This runs on the NetInTask thread; for DTLS records we ferry the
   // bytes to the media thread via MpDtlsPacketMsg, where MpDtls actually
   // processes them. Note: RTCP traffic is not currently DTLS-demuxed.
   if (mpDtls != NULL && !isRtcp && packetSize > 0)
   {
      unsigned char firstByte = (unsigned char)packetData[0];

      if (firstByte >= 20 && firstByte <= 63)
      {
         // DTLS record. Post to media thread for MpDtls processing.
         // Don't fall through to the SRTP unprotect path.
         ret = OS_FAILED;
         if (mpFlowGraph != NULL && !mDtlsResourceName.isNull())
         {
            MpDtlsPacketMsg msg(mDtlsResourceName, packetData, packetSize);
            ret = mpFlowGraph->getMsgQ()->send(msg, OsTime::OS_INFINITY);
         }
         if (ret != OS_SUCCESS)
         {
            OsSysLog::add(FAC_MP, PRI_WARNING, "MprFromNet::pushPacket: post DTLS packet failed (%d), dropping", ret);
         }
         return ret;
      }
      else if (firstByte <= 3)
      {
         // STUN - should have been picked off by OsNatDatagramSocket - drop
         return OS_SUCCESS;
      }
      else if (firstByte >= 128 && firstByte <= 191)
      {
         // RTP/RTCP. Pre-handshake-or-pre-keyed we drop -- packets can't be
         // authenticated yet. Once both DTLS is active AND SRTP keys have
         // been installed via MpSetSrtpParamsMsg, fall through to the
         // existing srtpUnprotectIfNeeded path.
         if (!mpDtls->isActive() || !mSrtp.isSessionCreated())
         {
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MprFromNet::pushPacket payload of size: %d dropped while waiting for DTLS-SRTP handshake to finish",
               packetSize);
            return OS_INVALID;
         }
         // fall through
      }
      else
      {
         // Unrecognized first byte - drop.
         OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::pushPacket: Unrecognized first byte (0x%02x)", firstByte);
         return OS_INVALID;
      }
   }

   // Note: Will NoOp and return OS_SUCCESS if ENABLE_SRTP is not defined
   if (!mSrtp.srtpUnprotectIfNeeded((const uint8_t*)packetData, &packetSize, isRtcp))
   {
      return OS_INVALID;
   }

   // WARNING: do not use udpBuf->getPacketSize() after this point.
   // Size may have changed after SRTP unprotection use packetSize variable instead.

   if (isRtcp == false)
   {
      mNumPktsRtp++;

      // Use more general parse function that takes data pointer and size, since size may have changed after SRTP unprotection
      //rtpBuf = parseRtpPacket(udpBuf);
      rtpBuf = parseRtpPacket(packetData, packetSize, udpBuf->getIP(), udpBuf->getUdpPort());
      if (!rtpBuf.isValid()) 
      {
         return OS_INVALID;
      }

      // Label buf so we know which flowgraph it is used in
      rtpBuf.setFlowGraph(mpFlowGraph);

      // Discard requested RTP stream
      if (mDiscardSelectedStream && rtpBuf->getRtpSSRC() == mDiscardedSSRC)
      {
         return ret;
      }

#ifndef INCLUDE_RTCP /* [ */
      rtcpStats(&rtpBuf->getRtpHeader());
#endif /* INCLUDE_RTCP ] */

      // Send the RTP packet to the RTP dispatcher
      ret = mpRtpDispatcher->pushPacket(rtpBuf);

#ifdef INCLUDE_RTCP /* [ */
      // This is the logic that forwards RTP packets to the RTCP subsystem
      // for Receiver Report calculations.


/**************************************************************************
 *
 *  This is broken.  Not really sure what it should be doing, but this is
 *   doing some calculation based on (1) 8000 Hz sample rate, and (2) the
 *   StrongARM crystal counter.
 *  Worse, the getTimecode() method will always return 0 (I believe).
 *
 *************************************************************************/

      // Set RTP Header Received Timestamp
      {
         unsigned long t = (unsigned long)udpBuf->getTimecode();
         double x;
         x = ((((double) t) * 8000.) / 3686400.);
         t = (unsigned long) x;
         oRTPHeader.SetRecvTimestamp(t);
      }

      // Parse the packet stream into an RTP header
      oRTPHeader.ParseRTPHeader((unsigned char *)packetData);

      // Dispatch packet to RTCP Render object
      if(mpiRTPDispatch)
      {
         mpiRTPDispatch->ForwardRTPHeader((IRTPHeader *)&oRTPHeader);
      }
#endif /* INCLUDE_RTCP ] */

   }
#ifdef INCLUDE_RTCP /* [ */
   else
   {  // RTCP packet
      // Dispatch the RTCP data packet to the RTCP Source object registered
      if(mpiRTCPDispatch)
      {
         int nBytes = CRTCPHeader::VetPacket((unsigned char*)packetData, packetSize);
         if (nBytes > 0) 
         {
            mpiRTCPDispatch->ProcessPacket((unsigned char*)packetData, nBytes);
         }
      }
   }
#endif /* INCLUDE_RTCP ] */

   return ret;
}

OsStatus MprFromNet::enableSsrcDiscard(UtlBoolean enable, RtpSRC ssrc)
{
   OsLock lock(mDiscardCtlMutex);
   mDiscardSelectedStream = enable;
   mDiscardedSSRC = ssrc;
 
   return OS_SUCCESS;
}

OsStatus MprFromNet::setRtpDispatcher(MprRtpDispatcher *pRtpDispatcher)
{
   mpRtpDispatcher = pRtpDispatcher;
   assert(mpRtpDispatcher != NULL);

   return OS_SUCCESS;
}

#ifdef INCLUDE_RTCP /* [ */
void  MprFromNet::setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch)
{
    IRTPDispatch *pOldRTPDispatch = mpiRTPDispatch;
    INetDispatch *pOldRTCPDispatch = mpiRTCPDispatch;

    // Add our references to the objects
    if (piRTPDispatch) piRTPDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    if (piRTPDispatch) piRTCPDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

#define RTCP_DEBUG
#ifdef RTCP_DEBUG /* [ */
    {
        int rc1 = -1, rc2 = -1;
        if (piRTPDispatch) {
            piRTPDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            rc1 = piRTPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }
        if (piRTPDispatch) {
            piRTCPDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            rc2 = piRTCPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }
        OsSysLog::add(FAC_MP, PRI_DEBUG, "MprFromNet::setDispatchers: this=%p, mpiRTPDispatch=%p, mpiRTCPDispatch=%p, piRTPDispatch=%p, piRTCPDispatch=%p, rfs: %d, %d", this, mpiRTPDispatch, mpiRTCPDispatch, piRTPDispatch, piRTCPDispatch, rc1, rc2);
    }
#endif /* RTCP_DEBUG ] */

// Set the dispatch pointers for both RTP and RTCP
    mpiRTPDispatch   = piRTPDispatch;
    mpiRTCPDispatch  = piRTCPDispatch;

    // release old references if the member values were not NULL
    if(pOldRTPDispatch)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG, "MprFromNet::setDispatchers: freeing pOldRTPDispatch = %p", pOldRTPDispatch);
        pOldRTPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    }

    // Release RTCP Render object
    if(pOldRTCPDispatch)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG, "MprFromNet::setDispatchers: freeing pOldRTCPDispatch = %p", pOldRTCPDispatch);
        pOldRTCPDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    }
}

#endif /* INCLUDE_RTCP ] */


/* ============================ ACCESSORS ================================= */

OsStatus MprFromNet::setFlowGraph(MpFlowGraphBase* flowgraph)
{
    mpFlowGraph = flowgraph;

    return(OS_SUCCESS);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

MpRtpBufPtr MprFromNet::parseRtpPacket(const MpUdpBufPtr &buf)
{
   return parseRtpPacket(buf->getDataPtr(), buf->getPacketSize(), buf->getIP(), buf->getUdpPort());
}

MpRtpBufPtr MprFromNet::parseRtpPacket(const char* packetData, int packetLength, const in_addr& ip, int udpPort)
{
   MpRtpBufPtr rtpBuf;
   int offset;
   int csrcSize;
   int csrcCount;

   if (packetLength < (int)sizeof(RtpHeader)) {
      // INVALID: shorter than an RTP packet header.
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: packet too short (%d)", packetLength);
      return rtpBuf; // contained pointer is still NULL
   }

   // Get new RTP buffer
   rtpBuf = MpMisc.RtpPool->getBuffer();

   // Copy source IP and port
   rtpBuf->setIP(ip);
   rtpBuf->setUdpPort(udpPort);

   // Copy RTP header data to RTP buffer.
   memcpy(&rtpBuf->getRtpHeader(), packetData, sizeof(RtpHeader));
   offset = sizeof(RtpHeader);

   if (2 != rtpBuf->getRtpVersion()) {
      // INVALID: we have only heard of version 2
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: RTP version is not 2 (%d)", rtpBuf->getRtpVersion());
      rtpBuf.release();
      return rtpBuf;
   }

   // Adjust packet size according to padding
   if (rtpBuf->isRtpPadding()) {
      uint8_t padBytes = *(packetData + packetLength - 1);

      if ((padBytes > 3) || (padBytes == 0)) {
         // INVALID: padding count is greater than 3.
         OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: improper RTP padding (%d)", padBytes);
         rtpBuf.release();
         return rtpBuf;
      }

      packetLength -= padBytes;
      rtpBuf->disableRtpPadding();
   }

   // Copy CSRC list to RTP buffer
   csrcCount = rtpBuf->getRtpCSRCCount();
   csrcSize = csrcCount * sizeof(RtpSRC);
   if ((offset + csrcSize) > packetLength) {
      // INVALID: CSRC count indicates more CSRCs than remaining packet data
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: packet too short (%d) for CSRC count (%d)", packetLength, csrcCount);
      rtpBuf.release();
      return rtpBuf;
   }

   RtpSRC* pCSRCsrc = (RtpSRC*)packetData + offset;
   RtpSRC* pCSRCdst = rtpBuf->getRtpCSRCs();
   int i;
   for (i=0; i<csrcCount; i++) {
      RtpSRC temp = *pCSRCsrc++;
      *pCSRCdst = ntohl(temp);  // use temp to avoid side effects if ntohl is a macro.
   }
   offset += csrcSize;

   // Check for RTP Header extension
   if (rtpBuf->isRtpExtension()) {
      int xLen;     // number of 32-bit words after extension header
      short* pXhdr; // pointer to extension header, after CSRC list

      // Length (in 32bit words) is beared in the second 16bits of first
      // 32bit word of extension header.
      pXhdr = (short*)(packetData + offset);
      xLen = ntohs(pXhdr[1]);

      // Increment offset by extension header plus extension size
      offset += (sizeof(uint32_t) * (1 + xLen));
      if (offset > packetLength) {
         // INVALID: we have moved beyond the end of data before reaching payload
         OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: packet too short (%d) CSRC count=%d, extLen=%d", packetLength, csrcCount, (int)(sizeof(uint32_t) * (1 + xLen)));
         rtpBuf.release();
         return rtpBuf;
      }

   }

   if (!rtpBuf->setPayloadSize(packetLength - offset)) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromNet::parseRtpPacket: RTP buffer size is too small: %d (need %d)\n", rtpBuf->getPayloadSize(), packetLength - offset);
      rtpBuf.release();
      return rtpBuf;
   }

   // Copy payload to RTP buffer.
   memcpy(rtpBuf->getDataWritePtr(), packetData + offset, rtpBuf->getPayloadSize());

   return rtpBuf;
}

UtlBoolean MprFromNet::resetSocketsInternal(OsEvent *pEvent)
{
   if (!mRegistered)
   {
      return FALSE;
   }

   OsStatus res;

   mRegistered = FALSE;
   res = mNetInTask->removeNetInputSources(this, pEvent);
   if (res == OS_SUCCESS)
   {
      return TRUE;
   }
   else
   {
      assert(FALSE);
      return FALSE;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
