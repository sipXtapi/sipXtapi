//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "rtcp/RtcpConfig.h"

#undef WANT_RTCP_LOG_MESSAGES
#undef DROP_SOME_PACKETS

// SYSTEM INCLUDES
#include <assert.h>

#ifdef _VXWORKS /* [ */
#include <iosLib.h>
#include <netinet/in.h>
#endif /* _VXWORKS ] */

#ifdef WIN32 /* [ */
#include <winsock2.h>
#   ifndef WINCE
#       include <io.h>
#   endif
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
#include "os/OsIntTypes.h"
#include <sys/types.h>
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprToNet.h"
#include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
#include "mp/MpIntResourceMsg.h"
#include "mp/dmaTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#ifdef ENABLE_PACKET_HACKING /* [ */
int MprToNet::sDebug1 = 0;
int MprToNet::sDebug2 = 0;
int MprToNet::sDebug3 = 0;
int MprToNet::sDebug4 = 0;
int MprToNet::sDebug5 = 0;
#endif /* ENABLE_PACKET_HACKING ] */

static int doPadRtp = 1;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToNet::MprToNet()
:  mpFromNetPal(NULL)
,  mRtcpPackets(0)
,  mRtcpFrameCount(0)
,  mRtcpFrameLimit(500)
#ifdef DEBUG /* [ */
,  mRtpSampleCounter(0)
#endif /* DEBUG ] */
,  mTimestampDelta(0)
,  mSeqNum(0)
,  mSSRC(0)
,  mpRtpSocket(NULL)
,  mpRtcpSocket(NULL)
,  mNumRtpWriteErrors(0)
,  mNumRtcpWriteErrors(0)
#ifdef INCLUDE_RTCP /* [ */
,  mpiRTPAccumulator(NULL)
#endif /* INCLUDE_RTCP ] */
,  mSRAdjustUSecs(0)

{
}

// Destructor
MprToNet::~MprToNet()
{
#ifdef INCLUDE_RTCP /* [ */

//  Release the reference held to the RTP Accumulator interface used to
//  RTP outbound stream statistics
    if(mpiRTPAccumulator)
    {
        mpiRTPAccumulator->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        mpiRTPAccumulator = NULL;
    }
#endif /* INCLUDE_RTCP ] */
    if (0 != mNumRtpWriteErrors) {
        osPrintf("MprToNet: %d network write errors on RTP socket!\n",
            mNumRtpWriteErrors);
    }
    if (0 != mNumRtcpWriteErrors) {
        osPrintf("MprToNet: %d network write errors on RTCP socket!\n",
            mNumRtcpWriteErrors);
    }
}

/* ============================ MANIPULATORS ============================== */

// Set the outbound RTP and RTCP sockets.
OsStatus MprToNet::setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   mpRtpSocket  = &rRtpSocket;
   mpRtcpSocket = &rRtcpSocket;

   return OS_SUCCESS;
}

// Clear the outbound RTP and RTCP sockets.
OsStatus MprToNet::resetSockets(void)
{
   mpRtpSocket  = NULL;
   mpRtcpSocket = NULL;

   return OS_SUCCESS;
}

void MprToNet::setSSRC(ssrc_t iSSRC)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToNet::setSSRC(0x%08X)", iSSRC);
   mSSRC = iSSRC;
   mSeqNum = rand_timer32() | 0xfc00;
   mTimestampDelta = rand_timer32();
#ifdef INCLUDE_RTCP /* [ */
   if (mpiRTPAccumulator) {
       mpiRTPAccumulator->CSR_SetRTPTimestamp(mTimestampDelta);
       mpiRTPAccumulator->SetSRAdjustUSecs(mSRAdjustUSecs);
   }
#endif /* INCLUDE_RTCP ] */
}

#ifdef INCLUDE_RTCP /* [ */
void MprToNet::setRTPAccumulator(ISetSenderStatistics *piRTPAccumulator)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToNet::setRTPAccumulator(%p), TSD=%d=0x%08X", piRTPAccumulator, mTimestampDelta, mTimestampDelta);
   mpiRTPAccumulator = piRTPAccumulator;
   if(mpiRTPAccumulator)
   {
       mpiRTPAccumulator->CSR_SetRTPTimestamp(mTimestampDelta);
       mpiRTPAccumulator->SetSRAdjustUSecs(mSRAdjustUSecs);
   }
}

#endif /* INCLUDE_RTCP ] */

#ifdef DROP_SOME_PACKETS // [
static int dropLimit = 1<<30;
static int dropCount = 0;

extern "C" {
extern int dropEvery(int limit);
}

int dropEvery(int limit)
{
   int save = dropLimit;
   if (limit < 0) limit = 1<<30;
   if (limit > 0) {
      dropLimit = limit;
      dropCount = 0;
   }
   return save;
}
#endif // DROP_SOME_PACKETS ]

int MprToNet::writeRtp(int payloadType, UtlBoolean markerState,
                       const unsigned char* payloadData, int payloadOctets,
                       unsigned int timestamp, void* csrcList)
{
   MpRtpBufPtr pRtpPacket;
   char paddingLength;

   // Nothing to do when no socket specified.
   if (mpRtpSocket == NULL)
      return 0;

   // Allocate new RTP packet.
   pRtpPacket = MpMisc.RtpPool->getBuffer();
   assert(pRtpPacket.isValid());

   // Label buf so we know which flowgraph is using it
   pRtpPacket.setFlowGraph(mpFlowGraph);

   // Get rid of packet sequence number.
   mSeqNum++;

   // Fill packet RTP header.
   pRtpPacket->setRtpVersion(2);
   pRtpPacket->setRtpPayloadType(payloadType);
   pRtpPacket->setPayloadSize(payloadOctets);
   if (markerState)
      pRtpPacket->enableRtpMarker();
   else
      pRtpPacket->disableRtpMarker();
   pRtpPacket->disableRtpExtension();
   pRtpPacket->setRtpSequenceNumber(mSeqNum);
   pRtpPacket->setRtpTimestamp(mTimestampDelta + timestamp);
   pRtpPacket->setRtpSSRC(mSSRC);
   pRtpPacket->setRtpCSRCCount(0);

#ifdef ENABLE_PACKET_HACKING /* [ */
   adjustRtpPacket(pRtpPacket->getRtpHeader());
#endif /* ENABLE_PACKET_HACKING ] */

   // Calculate number of padding bytes
   paddingLength = doPadRtp ? ((4 - (payloadOctets & 3))&3) : 0;

   if (paddingLength>0)
      pRtpPacket->enableRtpPadding();
   else
      pRtpPacket->disableRtpPadding();

   if (csrcList != NULL)
   {
      int csrcSize = pRtpPacket->getRtpCSRCCount() * sizeof(RtpSRC);
      // TODO:: implement CSRC list.
      assert(csrcSize == 0);
   }

   //////////////////////////////////////////////////////
   // Next part of code should separated from previous //
   //////////////////////////////////////////////////////

   char *pBuf;
   unsigned int numBytesSent;
   MpUdpBufPtr pUdpPacket;

   // Allocate UDP packet
   pUdpPacket = MpMisc.UdpPool->getBuffer();
   
   // Label buf so we know which flowgraph is using it
   pUdpPacket.setFlowGraph(mpFlowGraph);

   pBuf = pUdpPacket->getDataWritePtr();

   // Set size of data to send
   // *** Note that this makes no allowance for a CSRC list...
   // WAS: assert(pUdpPacket->getMaximumPacketSize() >= sizeof(RtpHeader)+payloadOctets+paddingLength);
   {
      size_t lMax, lHeader, lPayload, lPad;
      lMax = pUdpPacket->getMaximumPacketSize();
      lHeader = sizeof(RtpHeader);
      lPayload = payloadOctets;
      lPad = paddingLength;
      if (pUdpPacket->getMaximumPacketSize() < sizeof(RtpHeader)+payloadOctets+paddingLength) {
        // *** Note that this truncates the payload data, I hope the other end can take a joke...
        //   Maybe we should just "return 0:" and cut our losses.
         OsSysLog::add(FAC_MP, PRI_ERR, "MprToNet::writeRtp payload length too large: max=%ld < lHdr=%ld + lPayload=%ld + lPad=%ld; adjusting length to %ld-(%ld+%ld) = %ld", lMax, lHeader, lPayload, lPad, lMax, lHeader, lPad, lMax-(lHeader+lPad));
         payloadOctets = lMax - (lHeader + lPad);
      }
   }

   pUdpPacket->setPacketSize(sizeof(RtpHeader)+payloadOctets+paddingLength);

   // Copy RTP header to UDP packet
   memcpy(pBuf, &pRtpPacket->getRtpHeader(), sizeof(RtpHeader));
   pBuf += sizeof(RtpHeader);

   // TODO:: Copy CSRC list here.

   // Copy payload to UDP packet
   memcpy(pBuf, payloadData, payloadOctets);
   pBuf += payloadOctets;

   // Fill padding
   switch (paddingLength) {
   case 3:
      pBuf[1] = 0;           // fall through
   case 2:
      pBuf[0] = 0;           // fall through
   case 1:
      pBuf[paddingLength-1] = paddingLength;  // fall through
   case 0:
      break;
   }

#ifdef INCLUDE_RTCP /* [ */
   // Update the Accumulated statistics kept for an inbound RTP packet.
   // These statistics comprise a Sender Report that is sent out periodically
   // to the originating site
   if(mpiRTPAccumulator)
   {
       mpiRTPAccumulator->IncrementCounts(payloadOctets, mTimestampDelta, timestamp, mSSRC);
   }
#endif /* INCLUDE_RTCP ] */

   const char* writeBytes = pUdpPacket->getDataPtr();
   int packetSize = pUdpPacket->getPacketSize();

   // Socket can get set asynchronously to NULL when RTP send is stopped.  So check immediately before accessing socket.
   //  -- Well... if that's the case, save the value of mpRtpSocket in a temporary.  Then
   //      check the temp for NULL, and then dereference the temp only if it is not NULL.
   // The original code only made the race condition less likely to cause trouble, and so
   //    just made it more obscure and harder to find -- without actually fixing it...
   OsSocket*    pRtpSocket = mpRtpSocket;

#ifdef DROP_SOME_PACKETS /* [ */
   if (dropCount++ == dropLimit) {
      dropCount = 0;
      numBytesSent = packetSize;
      packetSize = 0;
   } 
#endif /* DROP_SOME_PACKETS ] */
   if(!pRtpSocket) {
      packetSize = 0;  // prevent dereference if NULL socket.
   }
   if(packetSize)
   {
      numBytesSent = pRtpSocket->write(writeBytes, packetSize);
   }
   else
   {
       numBytesSent = 0;
   }

#ifdef TEST_PRINT
   UtlString remoteIp("null socket");
   int remotePort;
   if(mpRtpSocket)
   {
      mpRtpSocket->getRemoteHostIp(&remoteIp, &remotePort);
   }
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToNet::writeRtp payload: %d send to address: %s port: %d numBytesSent: %d errno: %d",
      payloadType, remoteIp.data(), remotePort, numBytesSent, errno);
#endif

   if (numBytesSent != pUdpPacket->getPacketSize()) {
      switch (errno) {
         /* insert other benign errno values here */
            case 0:
            case 55: // Network disconnected, continue and hope it comes back
               break;
            default:
               // close(fd);  MAYBE: mpRtpSocket->close() ?
               // mpRtpSocket = NULL;
               break;
      }

   }
   return numBytesSent;
   return (pUdpPacket->getPacketSize() == numBytesSent) ? pUdpPacket->getPacketSize() : numBytesSent;
}

#ifdef ENABLE_PACKET_HACKING /* [ */
extern "C" {
extern int sTRA(int a, int b, int c);
extern int setToRtpAdjustment(int a, int b, int c);
};

int setToRtpAdjustment(int ssrc, int seq, int ts)
{
   MprToNet::sDebug1 = ssrc;
   MprToNet::sDebug2 = seq;
   MprToNet::sDebug3 = ts;
   return 0;
}

int sTRA(int a, int b, int c) {return setToRtpAdjustment(a,b,c);}
#endif /* ENABLE_PACKET_HACKING ] */


void MprToNet::setSRAdjustUSecs(int iUSecs)
{
   mSRAdjustUSecs = iUSecs;
#ifdef INCLUDE_RTCP /* [ */
   if(mpiRTPAccumulator)
   {
       mpiRTPAccumulator->SetSRAdjustUSecs(iUSecs);
   }
#endif /* INCLUDE_RTCP ] */
}

OsStatus MprToNet::setSRAdjustUSecs(const UtlString& namedResource, OsMsgQ& fgQ, int adjustUSecs)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_SR_ADJUST_USECS, namedResource, adjustUSecs);
   return fgQ.send(msg);
}



/* ============================ ACCESSORS ================================= */

OsStatus MprToNet::setFlowGraph(MpFlowGraphBase* flowgraph)
{
    mpFlowGraph = flowgraph;

    return(OS_SUCCESS);
}

OsStatus MprToNet::setRtpPal(MprFromNet* pal)
{
   mpFromNetPal = pal;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef ENABLE_PACKET_HACKING /* [ */
void MprToNet::adjustRtpPacket(struct RtpHeader* rp)
{
   struct RtpHeader rh;

   memcpy((char *) &rh, (char *) rp, sizeof(struct RtpHeader));
   // rh.vpxcc = rh.vpxcc;
   // rh.mpt = rh.mpt;
   rh.ssrc = ntohl(rh.ssrc) - sDebug1;
   rh.seq = ntohs(rh.seq) - sDebug2;
   rh.timestamp = ntohl(rh.timestamp) - sDebug3;
   rh.ssrc = htonl(rh.ssrc);
   rh.seq = htons(rh.seq);
   rh.timestamp = htonl(rh.timestamp);
   memcpy((char *) rp, (char *) &rh, sizeof(struct RtpHeader));
}
#endif /* ENABLE_PACKET_HACKING ] */

/* ============================ FUNCTIONS ================================= */
