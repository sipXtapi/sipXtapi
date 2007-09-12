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


// SYSTEM INCLUDES
#ifdef __pingtel_on_posix__ // [
#include <sys/types.h>
#include <netinet/in.h>
#elif _VXWORKS // ][
#include <inetlib.h>
#endif // _VXWORKS ]

// APPLICATION INCLUDES
#include "mp/MpdPtAVT.h"

// DEFINES
#define DEBUG_TONES
#undef  DEBUG_TONES

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD FUNCTION DECLARATION
void dumpRawAvtPacket(const MpRtpBufPtr &pRtp, void *pThis);

// STATIC VARIABLE INITIALIZATIONS
const MpCodecInfo MpdPtAVT::smCodecInfo(
         SdpCodec::SDP_CODEC_TONES, "telephone-events",
         8000, 0, 1, 0, 6400, 128, 128, 128, 0, 0, TRUE, FALSE);


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpdPtAVT::MpdPtAVT(int payloadType)
: MpDecoderBase(payloadType)
, mHaveValidData(FALSE)
, mIsEventActive(FALSE)
, mActiveEvent(0)
, mLastKeyUpTimetsamp(0)
, mStartingTimestamp(0)
, mLastEventDuration(0)
{
}

MpdPtAVT::~MpdPtAVT()
{
   freeDecode();
}

OsStatus MpdPtAVT::initDecode()
{
   mHaveValidData = FALSE;
   mIsEventActive = FALSE;
   mActiveEvent = 0;
   mLastKeyUpTimetsamp = 0;
   mStartingTimestamp = 0;
   mLastEventDuration = 0;
   return OS_SUCCESS;
}

OsStatus MpdPtAVT::freeDecode(void)
{
   return OS_SUCCESS;
}

/* ============================ MANIPULATORS ============================== */

int MpdPtAVT::decode(const MpRtpBufPtr &pPacket,
                     unsigned decodedBufferLength,
                     MpAudioSample *samplesBuffer)
{
   // Just save passed packet header and data - they will be used
   // in getSignalingData() called right after decode().
   mLastPacketData = *(const AvtPacket*) pPacket->getDataPtr();
   mLastRtpHeader = pPacket->getRtpHeader();
   assert(mHaveValidData == FALSE);
   mHaveValidData = TRUE;

#ifdef DEBUG_TONES // [
   dumpRawAvtPacket(pPacket, this);
#endif // DEBUG_TONES ]

   return 0;
}

/* ============================ ACCESSORS ================================= */

OsStatus MpdPtAVT::getSignalingData(uint8_t &event,
                                    UtlBoolean &isStarted,
                                    UtlBoolean &isStopped,
                                    uint16_t &duration)
{
   RtpTimestamp ts = ntohl(mLastRtpHeader.timestamp);

   // == Stage 0 ==
   // Just return, if we have nothing to decode
   if (mHaveValidData == FALSE)
   {
      return OS_NO_MORE_DATA;
   }

   // == Stage 1 ==
   // If this is a delayed packet from last event, we could drop it safely.
   // RFC4733 recommend to send end-of-event packet three times, so it's
   // normal to hit this check twice if no packets are lost.
   // Note: actually this check should use MpDspUtils::compareSerials(),
   //       but then we have to keep track of current stream timestamp.
   //       Else timestamp may wrap around between events, as much may
   //       pass between them. Later we could call decode() or getSignalingData()
   //       even if no packet is received to make this real.
   if (mLastKeyUpTimetsamp == ts)
   {
      mHaveValidData = FALSE;
      return OS_NO_MORE_DATA;
   }

   // == Stage 2 ==
   // Signal key up for previous tone if we lost its end packets.
   if (  mIsEventActive            // (1) Previous tone still active
      && mStartingTimestamp != ts  // (2) This is not an update packet for active event
      )
   {
      // Fill in returned data.
      event = mActiveEvent;
      isStarted = FALSE;
      isStopped = TRUE;
      duration = mLastEventDuration;

      // Reset decoder to no-active-event state. Saved RTP data and header are
      // kept as is to be used in next call to getSignalingData().
      resetEventState();

      // Ok, we have something to signal.
      return OS_SUCCESS;
   }

   // == Stage 3 ==
   // Parse received RTP packet.

   // Update last received event duration.
   mLastEventDuration = ntohs(mLastPacketData.duration);

   // Check for start of event conditions
   if (  // start bit marked
         ((mLastRtpHeader.mpt&RTP_M_MASK) && (ts != mStartingTimestamp))
      || // key up interpreted as key down if no previous start event received 
         (mIsEventActive == FALSE)
      )
   {
      mIsEventActive = TRUE;
      mActiveEvent = mLastPacketData.key;
      mStartingTimestamp = ts;

      isStarted = TRUE;
   }
   else
   {
      isStarted = FALSE;
   }

   // Check for end of event condition
   if ((0x80 & mLastPacketData.dB) > 0)
   {
      // Reset decoder to no-active-event state.
      resetEventState();

      isStopped = TRUE;
   }
   else
   {
      isStopped = FALSE;
   }

   // Signal that packet was parsed.
   mHaveValidData = FALSE;

   // Fill in remaining data to be returned.
   event = mLastPacketData.key;
   duration = mLastPacketData.duration;

   // Ok, we have something to signal.
   return OS_SUCCESS;
}

const MpCodecInfo* MpdPtAVT::getInfo() const
{
   return &smCodecInfo;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MpdPtAVT::resetEventState()
{
   mIsEventActive = FALSE;
   mLastKeyUpTimetsamp = mStartingTimestamp;
   mStartingTimestamp = 0;
   mLastEventDuration = 0;
}

/* ============================ FUNCTIONS ================================= */

void dumpRawAvtPacket(const MpRtpBufPtr &pRtp, void *pThis)
{
   uint8_t vpxcc;
   uint8_t mpt;
   RtpSeq seq;
   RtpTimestamp timestamp;
   RtpSRC ssrc;

   uint8_t  key;
   uint8_t  dB;
   UtlBoolean endBit;
   uint16_t duration;

   AvtPacket *pAvt;

   vpxcc = pRtp->getRtpHeader().vpxcc;
   mpt = pRtp->getRtpHeader().mpt;
   seq = pRtp->getRtpSequenceNumber();
   timestamp = pRtp->getRtpTimestamp();
   ssrc = pRtp->getRtpSSRC();

   pAvt = (AvtPacket*)pRtp->getDataPtr();
   key = pAvt->key;
   dB  = pAvt->dB & 0x3F;
   endBit = pAvt->dB >> 7;
   duration = ntohs(pAvt->duration);

   printf("MpdPtAvt(%p): Raw packet: "
          "seq:%6u ts:%10u key:%2u end:%1d dB:%3d duration:%5d\n",
          pThis,
          seq, timestamp, key, endBit, dB, duration);
}
