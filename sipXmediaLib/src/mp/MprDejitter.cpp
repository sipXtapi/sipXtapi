//  
// Copyright (C) 2006-2010 SIPez LLC. 
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
#ifdef ANDROID
   // Set to 1 to disable, 0 to enable verbose (LOGV) messages
#  define LOG_NDEBUG 1
#  define LOG_TAG "MprDejitter"

#  include <utils/Log.h>
#endif

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
//#define RTL_ENABLED

// MACROS
#ifdef DEBUG_PRINT // [
#  define debugPrintf    printf
#else  // DEBUG_PRINT ][
static void debugPrintf(...) {}
#endif // DEBUG_PRINT ]
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else // RTL_ENABLED ][
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif // RTL_ENABLED ]

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter(MpConnectionID connId, int streamId)
: mNumPackets(0)
, mNumLatePackets(0)
, mNumDiscarded(0)
, mLastPushed(0)
, mIsFirstPulledPacket(TRUE)
// mMaxPulledSeqNo will be initialized on first arrived packet
, mConnectionId(connId)
, mStreamId(streamId)
{
   mResourceName.appendFormat("MpDejitter-%d-%d", mConnectionId, mStreamId);
}

// Destructor
MprDejitter::~MprDejitter()
{
}

/* ============================ MANIPULATORS ============================== */

void MprDejitter::reset()
{
   RTL_EVENT(mResourceName+"_reset", 1);
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

OsStatus MprDejitter::pushPacket(MpRtpBufPtr &pRtp)
{
   int index;

   // Find place for incoming packet
   index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;
   RTL_EVENT(mResourceName+"_push_seq", pRtp->getRtpSequenceNumber());
   RTL_EVENT(mResourceName+"_push_index", index);
#ifdef DEBUG_PRINT
   debugPrintf("%5u (%2u) -> (", pRtp->getRtpSequenceNumber(), index);
#endif // DEBUG_PRINT

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
         RTL_EVENT(mResourceName+"_push_result", 2);
         mNumDiscarded++;
         if (mNumDiscarded < 40) 
         {
            debugPrintf("Dej: discard#%d Seq: %d -> %d Pt:%d\n",
                        mNumDiscarded, iBufSeqNo, iNewSeqNo,
                        pRtp->getRtpPayloadType());
         }

         if (  !mIsFirstPulledPacket
            && MpDspUtils::compareSerials(iBufSeqNo, mMaxPulledSeqNo) < 0
            && MpDspUtils::compareSerials(iNewSeqNo, mMaxPulledSeqNo) > 0) 
         {
            mNumLatePackets--;
            mNumPackets++;
         }

         mpPackets[index].swap(pRtp);
         mLastPushed = index;  
      } else {
         // Don't insert the new packet - it is an old delayed packet
         RTL_EVENT(mResourceName+"_push_result", 0);
         mNumDiscarded++;
         return OS_FAILED;
      }
   } else {
      if (  !mIsFirstPulledPacket
         && MpDspUtils::compareSerials( pRtp->getRtpSequenceNumber()
                                      , mMaxPulledSeqNo) <= 0)
      {
         RTL_EVENT(mResourceName+"_push_result", 3);
         mNumLatePackets++;
      }
      else
      {
         RTL_EVENT(mResourceName+"_push_result", 1);
         mNumPackets++;
      }
      mLastPushed = index;
      mpPackets[index] = pRtp;
   }

   RTL_EVENT(mResourceName+"_numPackets", mNumPackets);
   RTL_EVENT(mResourceName+"_numLatePackets", mNumLatePackets);
   RTL_EVENT(mResourceName+"_numDiscarded", mNumDiscarded);
   RTL_EVENT(mResourceName+"_push_lastPushed", mLastPushed);

   // Fake data to keep data streams in sync
   RTL_EVENT(mResourceName+"_pop_pulled", -1);
   RTL_EVENT(mResourceName+"_pop_minSeqIndex", -1);
   RTL_EVENT(mResourceName+"_pop_minSeq", -1);
   RTL_EVENT(mResourceName+"_pop_nextFrameAvailable", -1);

#ifdef DEBUG_PRINT
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

MpRtpBufPtr MprDejitter::pullPacket()
{
   return pullPacket(0, NULL, false);
}

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

   RTL_EVENT(mResourceName+"_pop_minSeqIndex", minSeqIdx);
   RTL_EVENT(mResourceName+"_pop_minSeq", mpPackets[minSeqIdx]->getRtpSequenceNumber());

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

      RTL_EVENT(mResourceName+"_pop_pulled", 1);
   }
   else
   {
      RTL_EVENT(mResourceName+"_pop_pulled", 0);
   }

#ifdef ANDROID
   LOGV("pullPacket %s packet, maxTime: %u seq: %u time: %ud numPackets: %d late packets: %d",
          found.isValid() ? "got" : "no", 
          maxTimestamp,
          found.isValid() ? found->getRtpSequenceNumber() : mMaxPulledSeqNo,
          found.isValid() ? found->getRtpTimestamp() : 0,
          mNumPackets, mNumLatePackets);
#endif

   RTL_EVENT(mResourceName+"_numPackets", mNumPackets);
   RTL_EVENT(mResourceName+"_numLatePackets", mNumLatePackets);
   RTL_EVENT(mResourceName+"_numDiscarded", mNumDiscarded);
   RTL_EVENT(mResourceName+"_pop_nextFrameAvailable", *nextFrameAvailable);

   // Fake data to keep data streams in sync
   RTL_EVENT(mResourceName+"_push_result", -1);
   RTL_EVENT(mResourceName+"_push_lastPushed", -1);
   RTL_EVENT(mResourceName+"_push_seq", -1);
   RTL_EVENT(mResourceName+"_push_index", -1);

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

void MprDejitter::setConnectionId(MpConnectionID connId)
{
   mConnectionId = connId;
   mResourceName.remove(0);
   mResourceName.append(mFlowgraphName);
   mResourceName.appendFormat("_MpDejitter-%d-%d", mConnectionId, mStreamId);
}

void MprDejitter::setStreamId(int streamId)
{
   mStreamId = streamId;
   mResourceName.remove(0);
   mResourceName.append(mFlowgraphName);
   mResourceName.appendFormat("_MpDejitter-%d-%d", mConnectionId, mStreamId);
}

void MprDejitter::setFlowgrapName(const UtlString &fgName)
{
   mFlowgraphName = fgName;
   mResourceName.remove(0);
   mResourceName.append(mFlowgraphName);
   mResourceName.appendFormat("_MpDejitter-%d-%d", mConnectionId, mStreamId);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

OsStatus MprDejitter::getFirstPacketInfo(RtpSeq& firstSeq, RtpTimestamp& firstTime) const
{
   OsStatus foundStatus = OS_FAILED;

   int firstPacketIndex = 0;
   for (; firstPacketIndex < MAX_RTP_PACKETS; firstPacketIndex++)
   if (mpPackets[firstPacketIndex].isValid())
   {
      foundStatus = OS_SUCCESS;
      break;
   }

   for(int packetIndex = firstPacketIndex + 1; packetIndex < MAX_RTP_PACKETS; packetIndex++)
   {
      if(mpPackets[packetIndex].isValid() &&
         (mpPackets[packetIndex]->getRtpSequenceNumber() <
          mpPackets[firstPacketIndex]->getRtpSequenceNumber()))
      {
         firstPacketIndex = packetIndex;
      }
   }

   if(foundStatus == OS_SUCCESS)
   {
      firstSeq = mpPackets[firstPacketIndex]->getRtpSequenceNumber();
      firstTime = mpPackets[firstPacketIndex]->getRtpTimestamp();
   }

   return(foundStatus);
}
/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
