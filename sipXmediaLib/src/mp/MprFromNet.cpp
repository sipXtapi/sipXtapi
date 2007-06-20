//  
// Copyright (C) 2006-2007 SIPez LLC. 
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


#include "rtcp/RtcpConfig.h"

#undef WANT_RTCP_LOG_MESSAGES

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef _VXWORKS
#include <inetlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTPHeader.h"
#endif /* INCLUDE_RTCP ] */
#include "os/OsEvent.h"
#include "os/OsMutex.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const int MprFromNet::SSRC_SWITCH_MISMATCH_COUNT = 8;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromNet::MprFromNet()
:  mMutex(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE),
   mRegistered(FALSE),
   mpDejitter(NULL),
#ifdef INCLUDE_RTCP /* [ */
   mpiRTCPDispatch(NULL),
   mpiRTPDispatch(NULL),
#else /* INCLUDE_RTCP ] [ */
   mRtcpCount(0),
#endif /* INCLUDE_RTCP ] */
   mPrevIP(0),
   mPrevPort(-1),
   mNumPushed(0),
   mNumWarnings(0),
   mPrefSsrc(0),
   mPrefSsrcValid(FALSE),
   mRtpDestIp(0),
   mRtpDestPort(0),
   mNumNonPrefPackets(0),
   mRtpRtcpMatchSsrc(0),
   mRtpRtcpMatchSsrcValid(FALSE),
   mRtpDestMatchIpOnlySsrc(0),
   mRtpDestMatchIpOnlySsrcValid(FALSE),
   mRtpOtherSsrc(0),
   mRtpOtherSsrcValid(FALSE)
{
#ifndef INCLUDE_RTCP /* [ */
   mInRtpHandle  = StartRtpSession(NULL, RTP_DIR_IN, (char) -1);
#endif /* INCLUDE_RTCP ] */
}

// Destructor
MprFromNet::~MprFromNet()
{
    resetSockets();

#ifdef INCLUDE_RTCP /* [ */
//  Release the references held to the RTP and RTCP Dispatchers used for
//  routing packets to the RTCP component
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
   if (NULL != mInRtpHandle)  FinishRtpSession(mInRtpHandle);
   mInRtpHandle  = NULL;
#endif /* INCLUDE_RTCP ] */


}

/* ============================ MANIPULATORS ============================== */

// Set the inbound RTP and RTCP sockets.
OsStatus MprFromNet::setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   OsStatus res;
   OsEvent notify;

   mMutex.acquire();

   resetSockets();
   res = addNetInputSources(&rRtpSocket, &rRtcpSocket, this, &notify);
   assert(res == OS_SUCCESS);
   notify.wait();
   mRegistered = TRUE;

   mMutex.release();
   return OS_SUCCESS;
}

// Deregister the inbound RTP and RTCP sockets.
OsStatus MprFromNet::resetSockets(void)
{
   mMutex.acquire();
   if (mRegistered) {
      OsStatus res;
      OsEvent notify;

      mRegistered = FALSE;
      res = removeNetInputSources(this, &notify);
      if (res == OS_SUCCESS) {
         notify.wait();
      } else {
         assert(FALSE);
      }
   }
   mMutex.release();
   return OS_SUCCESS;
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
   if (mInRtpHandle->ssrc != rtpH->ssrc) {
      const char* name = getName();
      static int firstfew = 0;

      if (firstfew++ < 20)
      {
         osPrintf("%s: New SSRC (0x%08X) detected\n", name, rtpH->ssrc);
      }
      mInRtpHandle->ssrc = rtpH->ssrc;
      mInRtpHandle->seq = rtpH->seq - 1;
      mInRtpHandle->cycles = 0;
      mRtcpCount = 0;
   }
   if (mInRtpHandle->seq < rtpH->seq) {
      mInRtpHandle->seq = rtpH->seq;
   } else {
      if ((mInRtpHandle->seq > 0xc000) && (rtpH->seq < 0x4000)) {
         mInRtpHandle->cycles++;
      }
   }
   return OS_SUCCESS;
}
#endif /* INCLUDE_RTCP ] */

MpRtpBufPtr MprFromNet::parseRtpPacket(const MpUdpBufPtr &buf)
{
   MpRtpBufPtr rtpBuf;
   int packetLength;
   int offset;
   int csrcSize;

   // Get new RTP buffer
   rtpBuf = MpMisc.RtpPool->getBuffer();

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

      // Increment offset by extention header plus extension size
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

/**************************************************************************
 *            HACK   HACK   HACK   $$$
 *
 * Here is a temporary workaround for a memory leak in the RTCP code.
 **************************************************************************/

static volatile int DoForwardRtcp = 0;
static volatile int RtcpDiscards = 0;

#ifdef _VXWORKS /* [ */

static volatile int TotalRtcpDiscards = 0;

extern "C" {
extern int SFR(); // Show Forward RTCP status
extern int NFR(); // Do Not Forward RTCP
extern int FR();  // Do Forward RTCP
}

static int ForwardRtcp(int YesNo)
{
   int save = DoForwardRtcp;
   DoForwardRtcp = YesNo;
   TotalRtcpDiscards += RtcpDiscards;
   printf("Discarded %d RTCP packets (%d total)\n",
      RtcpDiscards, TotalRtcpDiscards);
   RtcpDiscards = 0;
   return save;
}

int SFR() {return ForwardRtcp(DoForwardRtcp);}
int NFR() {return ForwardRtcp(0);}
int FR() {return ForwardRtcp(1);}

#endif /* _VXWORKS ] */

/**************************************************************************/

// Take in a buffer from the NetIn task
OsStatus MprFromNet::pushPacket(const MpUdpBufPtr &udpBuf, bool isRtcp)
{
    MpRtpBufPtr rtpBuf;
    OsStatus ret = OS_SUCCESS;
    int      thisSsrc;

#ifdef INCLUDE_RTCP /* [ */
    CRTPHeader   oRTPHeader;
#endif /* INCLUDE_RTCP ] */

    mNumPushed++;
    if (0 == (mNumPushed & ((1<<11)-1))) mNumWarnings = 0; // every 2048

    if (isRtcp == false)
    {
        rtpBuf = parseRtpPacket(udpBuf);

        thisSsrc = rtpBuf->getRtpSSRC();

        // Update preferred SSRC if it is not valid
        if (!mPrefSsrcValid) {
            setPrefSsrc(thisSsrc);
        }

        // Check packet's SSRC for validity
        if (thisSsrc == getPrefSsrc()) {
            mNumNonPrefPackets = 0;
        } else {
            if (mNumWarnings++ < 20) {
                UtlString Old(""), New("");
                struct in_addr t;
                t.s_addr = mRtpDestIp;
                OsSocket::inet_ntoa_pt(t, Old);
                OsSocket::inet_ntoa_pt(udpBuf->getIP(), New);
                osPrintf("   pushPacket: Pref:0x%X, rtpDest=%s:%d,\n"
                    "       this:0x%X (src=%s:%d)\n",
                    getPrefSsrc(), Old.data(), mRtpDestPort,
                    thisSsrc, New.data(), udpBuf->getUdpPort());
            }
            if (  (udpBuf->getIP().s_addr == mRtpDestIp)
               && ((int)udpBuf->getUdpPort() == mRtpDestPort)) {
                setPrefSsrc(thisSsrc);
            } else if (  mRtpRtcpMatchSsrcValid
                      && (thisSsrc == mRtpRtcpMatchSsrc)) {
                setPrefSsrc(thisSsrc);
            } else {
                mNumNonPrefPackets++;
                if (udpBuf->getIP().s_addr == mRtpDestIp) {
                    mRtpDestMatchIpOnlySsrc = thisSsrc;
                    mRtpDestMatchIpOnlySsrcValid = TRUE;
                } else {
                    mRtpOtherSsrc = thisSsrc;
                    mRtpOtherSsrcValid = TRUE;
                }
                if (SSRC_SWITCH_MISMATCH_COUNT <= mNumNonPrefPackets) {
                    setPrefSsrc(mRtpDestMatchIpOnlySsrcValid ?
                        mRtpDestMatchIpOnlySsrc : mRtpOtherSsrc);
                }
            }
            return ret;
        }

        if (  (mPrevIP != udpBuf->getIP().s_addr)
           || (mPrevPort != (int) udpBuf->getUdpPort()))
        {
            if (mNumWarnings++ < 20) {
                UtlString Old(""), New("");
                struct in_addr t;
                t.s_addr = mPrevIP;
                OsSocket::inet_ntoa_pt(t, Old);
                OsSocket::inet_ntoa_pt(udpBuf->getIP(), New);
/*
                osPrintf("MprFromNet(%d): SrcIP changed"
                    " from '%s:%d' to '%s:%d'\n", mNumPushed, Old.data(),
                    mPrevPort, New.data(), fromPort);
*/
            }
            mPrevIP = udpBuf->getIP().s_addr;
            mPrevPort = udpBuf->getUdpPort();
        }

#ifndef INCLUDE_RTCP /* [ */
        rtcpStats(&rtpBuf->getRtpHeader());
#endif /* INCLUDE_RTCP ] */

        ret = getMyDejitter()->pushPacket(rtpBuf);

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
#ifdef DUMP_RTCP_PACKETS /* [ */
        const char*       name;

        name = getName();
        osPrintf("%s: RTCP packet received, length = %d\n",
                                    name, buf->getSamplesNumber());
#endif /* DUMP_RTCP_PACKETS ] */

//      Dispatch the RTCP data packet to the RTCP Source object registered
/**************************************************************************
 *            HACK   HACK   HACK   $$$
 *
 * Here is a temporary workaround for a memory leak in the RTCP code.
 **************************************************************************/
        if(mpiRTCPDispatch)
        {
            if (DoForwardRtcp) {
                mpiRTCPDispatch->ProcessPacket(
                          (unsigned char *)udpBuf->getDataPtr(), 
                          (unsigned long)udpBuf->getPacketSize());
            } else {
                RtcpDiscards++;
            }
        }
    }
#endif /* INCLUDE_RTCP ] */

    return ret;
}

void MprFromNet::setMyDejitter(MprDejitter* pDJ)
{
   mpDejitter = pDJ;
}

void MprFromNet::setDestIp(OsSocket& newDest)
{
   struct in_addr t;
   newDest.getRemoteHostIp(&t, &mRtpDestPort);
   mRtpDestIp = t.s_addr;
   {
      int a, b, c, d;
      a = (mRtpDestIp >>  0) & 0xff;
      b = (mRtpDestIp >>  8) & 0xff;
      c = (mRtpDestIp >> 16) & 0xff;
      d = (mRtpDestIp >> 24) & 0xff;
/*
      osPrintf("MprFromNet::setDestIp: DestIP=0x%08lX, Dest = %d.%d.%d.%d:%d\n",
         mRtpDestIp, a, b, c, d, mRtpDestPort);
*/
   }
}

/* ============================ ACCESSORS ================================= */

MprDejitter* MprFromNet::getMyDejitter(void)
{
   assert(NULL != mpDejitter);
   return mpDejitter;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MprFromNet::getPrefSsrc()
{
   return mPrefSsrc;
}

int MprFromNet::setPrefSsrc(int newSsrc)
{
   // osPrintf("MprFromNet::setPrefSsrc(0x%X) -- was 0x%X\n", newSsrc, mPrefSsrc);
   mPrefSsrc = newSsrc;
   mPrefSsrcValid = TRUE;
   mNumNonPrefPackets = 0;
   mRtpRtcpMatchSsrcValid = FALSE;
   mRtpDestMatchIpOnlySsrcValid = FALSE;
   mRtpOtherSsrcValid = FALSE;
   return 0;
}

/* ============================ FUNCTIONS ================================= */

