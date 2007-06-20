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

//The averaged latency of packets in dejitter buffer is calculated in method 
//PullPacket( ) for the purpose of dejitter buffer
//backlog control (or called jitter control) by the decoder in down stream. 
//The decoder will look at the latency at certain frequency to make 
//the decision. -Brian Puh
//

// SYSTEM INCLUDES
#include <assert.h>
#if defined(_VXWORKS) || defined(__pingtel_on_posix__)
#include <sys/types.h>
#include <netinet/in.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsLock.h"
#include "mp/MpBuf.h"
#include "mp/MprDejitter.h"
#include "mp/MpMisc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter()
: mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
   memset(mBufferLookup, -1, 256 * sizeof(int));
   memset(mNumPackets, 0, MAX_CODECS * sizeof(int));
   memset(mNumDiscarded, 0, MAX_CODECS * sizeof(int));
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
         maxCodecIndex = max(maxCodecIndex,mBufferLookup[i]);
      maxCodecIndex++;

      // Codecs limit reached
      if (maxCodecIndex > MAX_CODECS)
         return OS_LIMIT_REACHED;

      // Store new codec index
      mBufferLookup[payloadType]=maxCodecIndex;
      codecIndex = maxCodecIndex;
   }

   // Find place for incoming packet
   index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;

   // Place packet to the buffer
   if (mpPackets[codecIndex][index].isValid())
   {
      // Check for packets already in the buffer. Overwrite them if 
      // the just-arriving packet is newer than the existing packet
      // Don't overwrite if the just-arriving packet is older
      RtpSeq iBufSeqNo = mpPackets[codecIndex][index]->getRtpSequenceNumber();
      RtpSeq iNewSeqNo = pRtp->getRtpSequenceNumber();

      if (compare(iNewSeqNo, iBufSeqNo) > 0) 
      {
         // Insert the new packet over the old packet
         mNumDiscarded[codecIndex]++;
         if (mNumDiscarded[codecIndex] < 40) 
         {
            osPrintf("Dej: discard#%d Seq: %d -> %d\n",
               mNumDiscarded[codecIndex], iBufSeqNo, iNewSeqNo);
         }
         mpPackets[codecIndex][index] = pRtp;
         mLastPushed[codecIndex] = index;  
         // mNumPackets remain unchanged, since we discarded a packet, and added one
      } else {
         // Don't insert the new packet - it is a old delayed packet
      }
   } else {
      mLastPushed[codecIndex] = index;
      mpPackets[codecIndex][index] = pRtp;
      mNumPackets[codecIndex]++;
   }

   return OS_SUCCESS;
}

// Get a pointer to the next RTP packet, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(int payloadType)
{
   return pullPacket(payloadType, 0, false);
}

// Get next RTP packet with given timestamp, or NULL if none is available.
MpRtpBufPtr MprDejitter::pullPacket(int payloadType, RtpTimestamp maxTimestamp, bool lockToTimestamp)
{
   OsLock locker(mRtpLock);

   MpRtpBufPtr found; ///< RTP packet we will return

   // Get codec index for incoming packet. Return none if we have not seen this
   // payload type before
   int codecIndex = mBufferLookup[payloadType];
   if (codecIndex < 0)
      return MpRtpBufPtr();

   // Return none if there are no packets
   if (mNumPackets[codecIndex]==0)
      return MpRtpBufPtr();

   // We find a packet by starting to look in the JB just AFTER where the latest
   // push was done, and loop around until we found valid packet or reach last
   // pushed packet.
   for (int iNextPull = mLastPushed[codecIndex] + 1;
        iNextPull != mLastPushed[codecIndex];
        iNextPull++)
   {
      // Wrap iNextPull counter if we reach end of buffer
      if (iNextPull >= MAX_RTP_PACKETS)
         iNextPull = 0;

      // If we reach valid packet, move it out of the buffer and break search loop
      if (  mpPackets[codecIndex][iNextPull].isValid()
         && (!lockToTimestamp
            || compare(mpPackets[codecIndex][iNextPull]->getRtpTimestamp(), maxTimestamp)<=0
            )
         )
      {
         found.swap(mpPackets[codecIndex][iNextPull]);
         mNumPackets[codecIndex]--;
         break;
      }
   }

   // Make sure we does not have copy of this buffer left in other threads.
   found.requestWrite();

   return found;
}

/* ============================ ACCESSORS ================================= */

int MprDejitter::getBufferLength(int payload)
{
   int codecIndex = mBufferLookup[payload];
   if (codecIndex < 0)
      return 0;

   return mNumPackets[codecIndex];
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
