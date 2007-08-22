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

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter()
: mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL)
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
            osPrintf("Dej: discard#%d Seq: %d -> %d\n",
               pStreamData->mNumDiscarded, iBufSeqNo, iNewSeqNo);
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
      // If we reach valid packet, move it out of the buffer and break search loop
      if (  pStreamData->mpPackets[iNextPull].isValid()
         && (!lockToTimestamp
            || MpDspUtils::compareSerials(pStreamData->mpPackets[iNextPull]->getRtpTimestamp(), maxTimestamp)<=0
            )
         )
      {
         found.swap(pStreamData->mpPackets[iNextPull]);
         pStreamData->mNumPackets--;
         break;
      }

      // Wrap iNextPull counter if we reach end of buffer
      iNextPull = (iNextPull + 1) % MAX_RTP_PACKETS;
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

   return mpStreamData[codecIndex].mNumPackets;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
