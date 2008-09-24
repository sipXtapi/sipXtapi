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
: mNumPackets(0)
, mNumLatePackets(0)
, mNumDiscarded(0)
, mLastPushed(0)
, mIsFirstPulledPacket(TRUE)
// mMaxPulledSeqNo will be initialized on first arrived packet
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
   if (mpPackets[index].isValid())
   {
      // Check for packets already in the buffer. Overwrite them if 
      // the just-arriving packet is newer than the existing packet
      // Don't overwrite if the just-arriving packet is older
      RtpSeq iBufSeqNo = mpPackets[index]->getRtpSequenceNumber();
      RtpSeq iNewSeqNo = pRtp->getRtpSequenceNumber();

      if (MpDspUtils::compareSerials(iNewSeqNo, iBufSeqNo) > 0) 
      {
         // Insert the new packet over the old packet
         mNumDiscarded++;
         if (mNumDiscarded < 40) 
         {
            debugPrintf("Dej: discard#%d Seq: %d -> %d Pt:%d\n",
                        mNumDiscarded, iBufSeqNo, iNewSeqNo,
                        pRtp->getRtpPayloadType());
         }
         mpPackets[index] = pRtp;
         mLastPushed = index;  

         if (  !mIsFirstPulledPacket
            && MpDspUtils::compareSerials(iBufSeqNo, mMaxPulledSeqNo) < 0
            && MpDspUtils::compareSerials(iNewSeqNo, mMaxPulledSeqNo) > 0) 
         {
            mNumLatePackets--;
            mNumPackets++;
         }
      } else {
         // Don't insert the new packet - it is an old delayed packet
         mNumDiscarded++;
         return OS_FAILED;
      }
   } else {
      mLastPushed = index;
      mpPackets[index] = pRtp;
      if (  !mIsFirstPulledPacket
         && MpDspUtils::compareSerials( pRtp->getRtpSequenceNumber()
                                      , mMaxPulledSeqNo) <= 0)
      {
         mNumLatePackets++;
      }
      else
      {
         mNumPackets++;
      }
   }

#ifdef DEBUG_PRINT
   debugPrintf("%5u (%2u) -> (", pRtp->getRtpSequenceNumber(), index);
   for (int i=0; i< MAX_RTP_PACKETS; i++)
   {
      if (mpPackets[i].isValid())
      {
         debugPrintf("%5u ", mpPackets[i]->getRtpSequenceNumber());
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
   if (mNumPackets==0 && mNumLatePackets==0)
      return found;

   // Search for first available packet
   int minSeqIdx = 0;
   for (; minSeqIdx < MAX_RTP_PACKETS; minSeqIdx++)
      if (mpPackets[minSeqIdx].isValid())
         break;

   // Search for packet with minimum sequence number
   for (int i = minSeqIdx+1; i < MAX_RTP_PACKETS; i++)
   {
      if (!mpPackets[i].isValid())
         continue;

      if (MpDspUtils::compareSerials(mpPackets[minSeqIdx]->getRtpSequenceNumber(),
                                     mpPackets[i]->getRtpSequenceNumber()) > 0)
      {
         minSeqIdx = i;
      }
   }

   if (  minSeqIdx < MAX_RTP_PACKETS
      && (!lockToTimestamp
         || MpDspUtils::compareSerials(mpPackets[minSeqIdx]->getRtpTimestamp(),
                                       maxTimestamp) <= 0
         )
      )
   {
      // Retrieve packet 
      found.swap(mpPackets[minSeqIdx]);
      RtpSeq foundRtpSeq = found->getRtpSequenceNumber();
      if (mIsFirstPulledPacket)
      {
         mIsFirstPulledPacket = FALSE;
         mMaxPulledSeqNo = foundRtpSeq;
         mNumPackets--;
      }
      else
      {
         if (MpDspUtils::compareSerials(foundRtpSeq, mMaxPulledSeqNo)<=0)
         {
            mNumLatePackets--;
         }
         else
         {
            mMaxPulledSeqNo = foundRtpSeq;
            mNumPackets--;
         }
      }

      // Check for next packet
      if (nextFrameAvailable)
      {
         int nextPacket = (minSeqIdx+1)%MAX_RTP_PACKETS;
         *nextFrameAvailable = mpPackets[nextPacket].isValid();
      }
   }

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MprDejitter::reset()
{
   for (int i=0; i<MAX_RTP_PACKETS; i++)
   {
      if (mpPackets[i].isValid())
      {
         mpPackets[i].release();
      }
   }
   mNumPackets = 0;
   mNumLatePackets = 0;
   mNumDiscarded = 0;
   mLastPushed = 0;
   mIsFirstPulledPacket = TRUE;
   // mMaxPulledSeqNo will be initialized on first arrived packet
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
