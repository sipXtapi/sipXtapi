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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsLock.h"
#include "mp/MpBuf.h"
#include "mp/MprDejitter.h"
#include "mp/MpMisc.h"
#include "mp/MpDspUtils.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#define DEBUG_PRINT
//#undef  DEBUG_PRINT

// MACROS
#ifdef DEBUG_PRINT // [
#  define debugPrintf    printf
#else  // DEBUG_PRINT ][
static void debugPrintf(...) {}
#endif // DEBUG_PRINT ]

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter()
: mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL)
, mNextPullTimerCount(0)
, mFramesSinceLastUpdate(0)
{
   memset(mBufferLookup, -1, 256 * sizeof(int));
}

// Destructor
MprDejitter::~MprDejitter()
{
}

/* ============================ MANIPULATORS ============================== */

// Add a buffer containing an incoming RTP packet to the dejitter pool.
// This method places the packet to the pool depending the modulo division value.
OsStatus MprDejitter::pushPacket(const MpRtpBufPtr &pRtp)
{
   int index;
   int payloadType;
   int codecIndex;

   OsLock lock(mRtpLock);

   // Get codec index for incoming packet and allocate new, if it does not have
   // one already.
   payloadType = pRtp->getRtpPayloadType();
   codecIndex = mBufferLookup[payloadType];
   if (codecIndex < 0)
   {
      // Search for maximum allocated codec index
      int maxCodecIndex = -1;
      for (int i=0; i<256; i++)
         maxCodecIndex = sipx_max(maxCodecIndex,mBufferLookup[i]);
      maxCodecIndex++;

      // Codecs limit reached
      if (maxCodecIndex > MAX_CODECS)
         return OS_LIMIT_REACHED;

      // Store new codec index
      mBufferLookup[payloadType]=maxCodecIndex;
      codecIndex = maxCodecIndex;
      mpStreamData[codecIndex].resetStream();
   }

   // Find place for incoming packet
   index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;

   // Store pointer to stream data structure for our convenience.
   StreamData *pStreamData = &mpStreamData[codecIndex];

   // Place packet to the buffer
   if (pStreamData->mpPackets[index].isValid())
   {
      // Check for packets already in the buffer. Overwrite them if 
      // the just-arriving packet is newer than the existing packet
      // Don't overwrite if the just-arriving packet is older
      RtpSeq iBufSeqNo = pStreamData->mpPackets[index]->getRtpSequenceNumber();
      RtpSeq iNewSeqNo = pRtp->getRtpSequenceNumber();

      if (MpDspUtils::compareSerials(iNewSeqNo, iBufSeqNo) > 0) 
      {
         // Insert the new packet over the old packet
         pStreamData->mNumDiscarded++;
         if (pStreamData->mNumDiscarded < 40) 
         {
            debugPrintf("Dej: discard#%d Seq: %d -> %d Pt:%d\n",
                        pStreamData->mNumDiscarded, iBufSeqNo, iNewSeqNo, payloadType);
         }
         pStreamData->mpPackets[index] = pRtp;
         pStreamData->mLastPushed = index;  
         // mNumPackets remain unchanged, since we discarded a packet, and added one
      } else {
         // Don't insert the new packet - it is a old delayed packet
         return OS_FAILED;
      }
   } else {
      pStreamData->mLastPushed = index;
      pStreamData->mpPackets[index] = pRtp;
      pStreamData->mNumPackets++;
   }

#ifdef DEBUG_PRINT
   debugPrintf("%5u (%2u) -> (", pRtp->getRtpSequenceNumber(), index);
   for (int i=0; i< MAX_RTP_PACKETS; i++)
   {
      if (pStreamData->mpPackets[i].isValid())
      {
         debugPrintf("%5u ", pStreamData->mpPackets[i]->getRtpSequenceNumber());
      } 
      else
      {
//         debugPrintf("----- ");
      }
   }
   debugPrintf(")\n");
#endif // DEBUG_PRINT

   return OS_SUCCESS;
}

// Get a pointer to the next RTP packet, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(int payloadType, bool isSignaling)
{
   return pullPacket(payloadType, 0, false, isSignaling);
}

// Get next RTP packet with given timestamp, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(int payloadType, RtpTimestamp maxTimestamp,
                                    bool lockToTimestamp, bool isSignaling)
{
   OsLock locker(mRtpLock);

   MpRtpBufPtr found; ///< RTP packet we will return

   // Get codec index for incoming packet. Return none if we have not seen this
   // payload type before
   int codecIndex = mBufferLookup[payloadType];
   if (codecIndex < 0)
      return MpRtpBufPtr();

   // Store pointer to stream data structure for our convenience.
   StreamData *pStreamData = &mpStreamData[codecIndex];

   // Return none if there are no packets
   if (pStreamData->mNumPackets==0)
      return MpRtpBufPtr();

   // We find a packet by starting to look in the JB just AFTER where the latest
   // push was done, and loop MAX_RTP_PACKETS times or until we find a valid frame
   int iNextPull = (pStreamData->mLastPushed + 1) % MAX_RTP_PACKETS;

   for (int i = 0; i < MAX_RTP_PACKETS; i++)
   {
      // Check if this packet valid.
      if (  pStreamData->mpPackets[iNextPull].isValid()
         && (!lockToTimestamp
            || MpDspUtils::compareSerials(pStreamData->mpPackets[iNextPull]->getRtpTimestamp(), maxTimestamp)<=0
            )
         )
      {
         // This call lets the codec decide if it wants this packet or not. If
         // the codec rejects out-of-order packets, it will return a negative value.
         // It may also (someday) dynamically adjust the size of the jitter buffer.
         int checkRes = pStreamData->checkPacket(pStreamData->mpPackets[iNextPull],
                                                 mNextPullTimerCount, isSignaling);
         debugPrintf("checkPacket() returned %d for payload %d\n",
                     checkRes, payloadType);
         if (checkRes > 0) 
         {
            found.swap(pStreamData->mpPackets[iNextPull]);
            pStreamData->mNumPackets--;
            break;
         } else if (checkRes == 0) {
            // Stop iterating and return NULL.
            break;
         } else if (checkRes == -1) {
            // checkRes < 0, this means that this is out of order packet.
            // Just discard it.
            pStreamData->mpPackets[iNextPull].release();
         }

      }

      // Wrap iNextPull counter if we reach end of buffer
      iNextPull = (iNextPull + 1) % MAX_RTP_PACKETS;
   }

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

void MprDejitter::frameIncrement() 
{
   // increment the pull timer count one frame's worth
   mNextPullTimerCount+=80;
//   osPrintf("Dej %p, frame time: %d\n", this, mNextPullTimerCount);

   // increment number of frames since last statistic update and update
   // statistic if enough time passed.
   mFramesSinceLastUpdate++;
   if (mFramesSinceLastUpdate >= 100)
   {
      mFramesSinceLastUpdate = 0;

      // Loop through all streams and update statistic
      for (int payloadType=0; payloadType<256; payloadType++)
      {
         int codecIndex = mBufferLookup[payloadType];
         if (codecIndex >= 0)
         {
            int iAveLen = getBufferLength(payloadType);
            mpStreamData[codecIndex].updateStatistic(iAveLen);
         }
      }
   }

}

/* ============================ ACCESSORS ================================= */

int MprDejitter::getBufferLength(int payload)
{
   int codecIndex = mBufferLookup[payload];
   if (codecIndex < 0)
      return 0;

   return mpStreamData[codecIndex].mNumPackets;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MprDejitter::StreamData::resetStream()
{
   mUnderflowCount = 0;
   mTimestampOffset = 0;
   mLastSeqNo = 0;
   mIsFirstFrame = true;
   mClockDrift = false;
   mLastReportSize = -1;
   mLastSSRC = 0;
   mWaitTimeInFrames = 2;
}

int MprDejitter::StreamData::checkPacket(const MpRtpBufPtr &pPacket,
                                         RtpTimestamp nextPullTimestamp,
                                         UtlBoolean isSignaling)
{
   RtpTimestamp rtpTimestamp = pPacket->getRtpTimestamp()-mTimestampOffset;
   RtpTimestamp delta = 0; // Contain the difference between the current pull
                           // pointer and the rtpTimestamp. Use the delta because
                           // rtpTimestamp and nextPullTimestamp are unsigned,
                           // so a straight subtraction will fail.

   if (!mIsFirstFrame && pPacket->getRtpSSRC() != mLastSSRC)
   {
      // SSRC changed, reset statistics, consider this as the first frame
      debugPrintf("Reset stream data due to SSRC change: %X->%X",
                  mLastSSRC, pPacket->getRtpSSRC());
      resetStream();
   }

   // Special case: Always accept packets from signaling codecs.
   if (isSignaling)
   {
      return 1;
   }

   // If this is our first packet

   if (mIsFirstFrame)
   {
      mIsFirstFrame = false;
      mTimestampOffset = pPacket->getRtpTimestamp()-nextPullTimestamp+(160*mWaitTimeInFrames);
      mLastSeqNo = pPacket->getRtpSequenceNumber();
      mLastSSRC = pPacket->getRtpSSRC();

      debugPrintf("payload %d: rtpTimestamp=%10u mTimestampOffset=%10u nextPullTimestamp=%10u\n",
                  pPacket->getRtpPayloadType(), pPacket->getRtpTimestamp(), mTimestampOffset, nextPullTimestamp);

      // Always accept the first packet
      return 1;
   }

   // Compute absolute value of delta between rtpTimestamp and nextPullTimestamp
   if (MpDspUtils::compareSerials(rtpTimestamp, nextPullTimestamp) > 0)
   {
      if (rtpTimestamp > nextPullTimestamp)
      {
         delta = rtpTimestamp - nextPullTimestamp;
      } else {
         delta = (UINT32_MAX - nextPullTimestamp) + rtpTimestamp;
      }
   }
   else
   {
      if (rtpTimestamp > nextPullTimestamp)
      {
         delta = (UINT32_MAX - rtpTimestamp) + nextPullTimestamp;
      } else {
         delta = nextPullTimestamp - rtpTimestamp;
      }
   }
   debugPrintf("payload %d: delta=%10u rtpTimestamp=%10u mTimestampOffset=%10u nextPullTimestamp=%10u\n",
               pPacket->getRtpPayloadType(), delta, pPacket->getRtpTimestamp(), mTimestampOffset, nextPullTimestamp);

   if (delta > (160*(mWaitTimeInFrames*2)))
   {
      // Detected timer count silence, skip or stream startup, resetting
      // nextPullTimerCount if we have underflowed the JB, make the reset to
      // a higher value
      if (mClockDrift)
      {
         // Clock drift detected, too few packets in buffer!
         mTimestampOffset = pPacket->getRtpTimestamp()-nextPullTimestamp+(160*(mWaitTimeInFrames*2));
      } else {
         mTimestampOffset = pPacket->getRtpTimestamp()-nextPullTimestamp+(160*mWaitTimeInFrames);
      }

      // Throw out this packet and stop frame processing for this frame.
      debugPrintf("payload %d: counter silence%s mTimestampOffset=%u\n",
                  pPacket->getRtpPayloadType(), mClockDrift?" (clock skew)":"", mTimestampOffset);
      return 0;
   }

   if (MpDspUtils::compareSerials(rtpTimestamp, nextPullTimestamp) <= 0) {
      // A packet is available within the allotted time span
      mUnderflowCount=0;
      // Process the frame if enough time has passed
      RtpSeq iSeqNo = pPacket->getRtpSequenceNumber();
      if (MpDspUtils::compareSerials(iSeqNo, mLastSeqNo) < 0)
      {
         // Out of Order Discard
         return -1;  // Discard the packet, it is out of order
      }
      mLastSeqNo = iSeqNo;
      return 1;
   } else {
      // Count errors if we are not pulling packets for some reason
      mUnderflowCount++;

      debugPrintf("payload %d: unwanted packet\n", pPacket->getRtpPayloadType());
      return 0;  // We don't want this packet
   }
}

void MprDejitter::StreamData::updateStatistic(int averageLength)
{
   // Zero or one is the starting condition
   if(averageLength <= 1)
      return;

   int maxLength = mWaitTimeInFrames+2;
   int minLength = mWaitTimeInFrames-1; 

   if (minLength < 1)
      minLength = 1;

   if (mLastReportSize == -1)
      mLastReportSize = averageLength;

   if (averageLength < minLength)
   {
      // There are too few packets in the buffer.
      if (mLastReportSize-averageLength <= 1)
      {
         // Only react when the buffer length changes mildly, not dramatically,
         // since the latter probably isn't clock drift
         mClockDrift = true;
      }
   }
   mLastReportSize = averageLength;
}

/* ============================ FUNCTIONS ================================= */
