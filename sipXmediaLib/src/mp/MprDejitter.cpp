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
{
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
         mStreamData.mLastPushed = index;  

         if (  MpDspUtils::compareSerials(iBufSeqNo, mStreamData.mMaxPulledSeqNo) < 0
            && MpDspUtils::compareSerials(iNewSeqNo, mStreamData.mMaxPulledSeqNo) > 0) 
         {
            mStreamData.mNumLatePackets--;
            mStreamData.mNumPackets++;
         }
      } else {
         // Don't insert the new packet - it is a old delayed packet
         mStreamData.mNumDiscarded++;
         return OS_FAILED;
      }
   } else {
      mStreamData.mLastPushed = index;
      mStreamData.mpPackets[index] = pRtp;
      if (  !mStreamData.mIsFirstPulledPacket
         && MpDspUtils::compareSerials( pRtp->getRtpSequenceNumber()
                                      , mStreamData.mMaxPulledSeqNo) <= 0)
      {
         mStreamData.mNumLatePackets++;
      }
      else
      {
         mStreamData.mNumPackets++;
      }
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
   return pullPacket(0, NULL, false);
}

// Get next RTP packet with given timestamp, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(RtpTimestamp maxTimestamp,
                                    UtlBoolean *nextFrameAvailable,
                                    bool lockToTimestamp)
{
   MpRtpBufPtr found; ///< RTP packet we will return

   // Return none if there are no packets
   if (mStreamData.mNumPackets==0 && mStreamData.mNumLatePackets==0)
      return found;

   // Search for first available packet
   int minSeqIdx = 0;
   for (; minSeqIdx < MAX_RTP_PACKETS; minSeqIdx++)
      if (mStreamData.mpPackets[minSeqIdx].isValid())
         break;

   // Search for packet with minimum sequence number
   for (int i = minSeqIdx+1; i < MAX_RTP_PACKETS; i++)
   {
      if (!mStreamData.mpPackets[i].isValid())
         continue;

      if (MpDspUtils::compareSerials(mStreamData.mpPackets[minSeqIdx]->getRtpSequenceNumber(),
                                     mStreamData.mpPackets[i]->getRtpSequenceNumber()) > 0)
      {
         minSeqIdx = i;
      }
   }

   if (  minSeqIdx < MAX_RTP_PACKETS
      && (!lockToTimestamp
         || MpDspUtils::compareSerials(mStreamData.mpPackets[minSeqIdx]->getRtpTimestamp(),
                                       maxTimestamp) <= 0
         )
      )
   {
      // Retrieve packet 
      found.swap(mStreamData.mpPackets[minSeqIdx]);
      RtpSeq foundRtpSeq = found->getRtpSequenceNumber();
      if (mStreamData.mIsFirstPulledPacket)
      {
         mStreamData.mIsFirstPulledPacket = FALSE;
         mStreamData.mMaxPulledSeqNo = foundRtpSeq;
         mStreamData.mNumPackets--;
      }
      else
      {
         if (MpDspUtils::compareSerials(foundRtpSeq, mStreamData.mMaxPulledSeqNo)<=0)
         {
            mStreamData.mNumLatePackets--;
         }
         else
         {
            mStreamData.mMaxPulledSeqNo = foundRtpSeq;
            mStreamData.mNumPackets--;
         }
      }

      // Check for next packet
      if (nextFrameAvailable)
      {
         int nextPacket = (minSeqIdx+1)%MAX_RTP_PACKETS;
         *nextFrameAvailable = mStreamData.mpPackets[nextPacket].isValid();
      }
   }

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
