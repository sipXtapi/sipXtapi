//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
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
#endif /* INCLUDE_RTCP ] */
#include <os/OsEvent.h>
#include <os/OsMutex.h>
#include <os/OsDefs.h>
#include <mp/MpResourceMsg.h>
#include <mp/MpSetSocketsMsg.h>
#include <mp/NetInTask.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MprFromNet::MprFromNet()
: mMutex(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
, mNetInTask(NetInTask::getNetInTask())
, mRegistered(FALSE)
, mpRtpDispatcher(NULL)
, mDiscardSelectedStream(FALSE)
, mDiscardedSSRC(0)
, mpFlowGraph(NULL)
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
      mpiRTPDispatch->Release();
      mpiRTPDispatch = NULL;
   }

   if(mpiRTCPDispatch)
   {
      mpiRTCPDispatch->Release();
      mpiRTCPDispatch = NULL;
   }
#else /* INCLUDE_RTCP ] [ */
   if (NULL != mInRtpHandle)
      FinishRtpSession(mInRtpHandle);
   mInRtpHandle  = NULL;
#endif /* INCLUDE_RTCP ] */


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
      OsLock lock(mMutex);

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
      OsLock lock(mMutex);
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

    switch(rMsg.getMsg())
   {
   case MpResourceMsg::MPRM_SET_SOCKETS:
   {
      OsSocket* rtpSocket = ((MpSetSocketsMsg&)rMsg).getRtpSocket();
      OsSocket* rtcpSocket = ((MpSetSocketsMsg&)rMsg).getRtcpSocket();
      assert(rtpSocket);
      assert(rtcpSocket);
      setSockets(*rtpSocket, *rtcpSocket);
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
      const char* name = getName();
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
   OsLock lock(mMutex);
   MpRtpBufPtr rtpBuf;
   OsStatus ret = OS_SUCCESS;

#ifdef INCLUDE_RTCP /* [ */
   CRTPHeader   oRTPHeader;
#endif /* INCLUDE_RTCP ] */

   mNumPushed++;

   if (isRtcp == false)
   {
      mNumPktsRtp++;

      rtpBuf = parseRtpPacket(udpBuf);

      // Label buf so we know which flowgraph its used in
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

      // Set RTP Header Received Timestamp
      {
         unsigned long t = (unsigned long)udpBuf->getTimecode();
         double x;
         x = ((((double) t) * 8000.) / 3686400.);
         t = (unsigned long) x;
         oRTPHeader.SetRecvTimestamp(t);
      }

      // Parse the packet stream into an RTP header
      oRTPHeader.ParseRTPHeader((unsigned char *)udpBuf->getDataPtr());

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
         mpiRTCPDispatch->ProcessPacket((unsigned char *)udpBuf->getDataPtr(),
                                        (unsigned long)udpBuf->getPacketSize());
      }
   }
#endif /* INCLUDE_RTCP ] */

   return ret;
}

OsStatus MprFromNet::enableSsrcDiscard(UtlBoolean enable, RtpSRC ssrc)
{
   OsLock lock(mMutex);
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
   MpRtpBufPtr rtpBuf;
   int packetLength;
   int offset;
   int csrcSize;

   // Get new RTP buffer
   rtpBuf = MpMisc.RtpPool->getBuffer();

   // Copy source IP and port
   rtpBuf->setIP(buf->getIP());
   rtpBuf->setUdpPort(buf->getUdpPort());

   // Copy RTP header data to RTP buffer.
   memcpy(&rtpBuf->getRtpHeader(), buf->getDataPtr(), sizeof(RtpHeader));
   offset = sizeof(RtpHeader);

   // Adjust packet size according to padding
   packetLength = buf->getPacketSize();
   if (rtpBuf->isRtpPadding()) {
      uint8_t padBytes = *(buf->getDataPtr() + packetLength - 1);

      // Ipse: I'm not sure why we do this... Say me if you know.
      if ((padBytes & (~3)) != 0) {
         padBytes = 0;
      }

      packetLength -= padBytes;
      rtpBuf->disableRtpPadding();
   }

   // Copy CSRC list to RTP buffer
   csrcSize = rtpBuf->getRtpCSRCCount() * sizeof(RtpSRC);
   memcpy(rtpBuf->getRtpCSRCs(), buf->getDataPtr()+offset, csrcSize);
   offset += csrcSize;

   // Check for RTP Header extension
   if (rtpBuf->isRtpExtension()) {
      int xLen;     // number of 32-bit words after extension header
      short* pXhdr; // pointer to extension header, after CSRC list

      // Length (in 32bit words) is beared in the second 16bits of first
      // 32bit word of extension header.
      pXhdr = (short*) (buf->getDataPtr() + offset);
      xLen = ntohs(pXhdr[1]);

      // Increment offset by extension header plus extension size
      offset += (sizeof(int) * (1 + xLen));
   }

   if (!rtpBuf->setPayloadSize(packetLength - offset)) {
      osPrintf( "RTP buffer size is too small: %d (need %d)\n"
              , rtpBuf->getPayloadSize()
              , packetLength - offset);
   }

   // Copy payload to RTP buffer.
   memcpy( rtpBuf->getDataWritePtr(), buf->getDataPtr()+offset
         , rtpBuf->getPayloadSize());

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
