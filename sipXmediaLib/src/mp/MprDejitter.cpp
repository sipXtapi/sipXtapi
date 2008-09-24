//  
// Copyright (C) 2006-2008 SIPez LLC. 
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
#undef  DEBUG_PRINT

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
: mNextPullTimerCount(0)
, mFramesSinceLastUpdate(0)
{
   mStreamData.resetStream();
}

// Destructor
MprDejitter::~MprDejitter()
{
}

/* ============================ MANIPULATORS ============================== */

// Add a buffer containing an incoming RTP packet to the dejitter pool.
// This method places the packet to the pool depending the modulo division value.
OsStatus MprDejitter::pushPacket(const MpRtpBufPtr &pRtp, UtlBoolean isSignaling)
{
   int index;

   // Find place for incoming packet
   index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;

   // Place packet to the buffer
   if (mStreamData.mpPackets[index].isValid())
   {
      // Check for packets already in the buffer. Overwrite them if 
      // the just-arriving packet is newer than the existing packet
      // Don't overwrite if the just-arriving packet is older
      RtpSeq iBufSeqNo = mStreamData.mpPackets[index]->getRtpSequenceNumber();
      RtpSeq iNewSeqNo = pRtp->getRtpSequenceNumber();

      if (MpDspUtils::compareSerials(iNewSeqNo, iBufSeqNo) > 0) 
      {
         // Insert the new packet over the old packet
         mStreamData.mNumDiscarded++;
         if (mStreamData.mNumDiscarded < 40) 
         {
            debugPrintf("Dej: discard#%d Seq: %d -> %d Pt:%d\n",
                        mStreamData.mNumDiscarded, iBufSeqNo, iNewSeqNo,
                        pRtp->getRtpPayloadType());
         }
         mStreamData.mpPackets[index] = pRtp;
         mStreamData.mpSignalingFlag[index] = isSignaling;
         mStreamData.mLastPushed = index;  
         // mNumPackets remain unchanged, since we discarded a packet, and added one
      } else {
         // Don't insert the new packet - it is a old delayed packet
         return OS_FAILED;
      }
   } else {
      mStreamData.mLastPushed = index;
      mStreamData.mpPackets[index] = pRtp;
      mStreamData.mpSignalingFlag[index] = isSignaling;
      mStreamData.mNumPackets++;
   }

#ifdef DEBUG_PRINT
   debugPrintf("%5u (%2u) -> (", pRtp->getRtpSequenceNumber(), index);
   for (int i=0; i< MAX_RTP_PACKETS; i++)
   {
      if (mStreamData.mpPackets[i].isValid())
      {
         debugPrintf("%5u ", mStreamData.mpPackets[i]->getRtpSequenceNumber());
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
MpRtpBufPtr MprDejitter::pullPacket()
{
   return pullPacket(0, false);
}

// Get next RTP packet with given timestamp, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(RtpTimestamp maxTimestamp,
                                    bool lockToTimestamp)
{
   MpRtpBufPtr found; ///< RTP packet we will return

   if (mFramesSinceLastUpdate==0)
   {
//      printf("JB length for codec %d: %d\n", payloadType, pStreamData->mNumPackets);
   }

   // Return none if there are no packets
   if (mStreamData.mNumPackets==0)
      return MpRtpBufPtr();

   // We find a packet by starting to look in the JB just AFTER where the latest
   // push was done, and loop MAX_RTP_PACKETS times or until we find a valid frame
   int iNextPull = (mStreamData.mLastPushed + 1) % MAX_RTP_PACKETS;

   for (int i = 0; i < MAX_RTP_PACKETS; i++)
   {
      // Check if this packet valid.
      if (  mStreamData.mpPackets[iNextPull].isValid()
         && (!lockToTimestamp
            || MpDspUtils::compareSerials(mStreamData.mpPackets[iNextPull]->getRtpTimestamp(),
                                          maxTimestamp) <= 0
            )
         )
      {
         found.swap(mStreamData.mpPackets[iNextPull]);
         mStreamData.mNumPackets--;
      }

      // Wrap iNextPull counter if we reach end of buffer
      iNextPull = (iNextPull + 1) % MAX_RTP_PACKETS;
   }

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

/* ============================ ACCESSORS ================================= */

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

/* ============================ FUNCTIONS ================================= */
