//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpConnection.h"
#include "mp/MprDejitter.h"
#include "mp/MprFromNet.h"
#include "mp/NetInTask.h"
/* for dejitter handling purpose */
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"

//===========================================================
// Clock tick: for packet detaining period calculation
extern volatile int* pOsTC;
//===========================================================

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter(const UtlString& rName, MpConnection* pConn,
                         int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL),
   mNumPackets(0),
   mNumDiscarded(0)
{
}

// Destructor
MprDejitter::~MprDejitter()
{
}
/* ============================ MANIPULATORS ============================== */

//Add a buffer containing an incoming RTP packet to the dejitter pool
OsStatus MprDejitter::pushPacket(const MpRtpBufPtr &pRtp)
{
   int index;
   OsLock locker(mRtpLock);

   index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;
   if (mpPackets[index].isValid()) {
      mNumDiscarded++;
#ifdef MP_STREAM_DEBUG /* [ */
      if (mNumDiscarded < 40) {
         osPrintf("Dej: discard#%d Seq: %d -> %d at 0x%X\n",
            mNumDiscarded,
            mpPackets[index]->getRtpSequenceNumber(),
            pRtp->getRtpSequenceNumber(),
            *pOsTC);
      }
#endif /* MP_STREAM_DEBUG ] */
      mNumPackets--;
   }
   mpPackets[index] = pRtp;
   mNumPackets++;

   return OS_SUCCESS;
}

// Get a pointer to the next RTP packet, or NULL if none is available.

MpRtpBufPtr MprDejitter::pullPacket()
{
   MpRtpBufPtr found;
   int curSeq ;
   int first = -1; 
   int firstSeq ;
   int i;
   OsLock locker(mRtpLock);


   // Find smallest seq number
   for (i=0;i<MAX_RTP_PACKETS; i++)
   {
       if (mpPackets[i].isValid()) 
       {
           curSeq = mpPackets[i]->getRtpSequenceNumber();
           if (first == -1)
           {
               first = i ;
               firstSeq = curSeq ;
           }
           else if (curSeq < firstSeq)
           {
               first = i ;
               firstSeq = curSeq ;
           }
       }
   }

   if (first != -1)
   {
      found.swap(mpPackets[first]);
      mNumPackets--;
   }

   return found;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprDejitter::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
   UtlBoolean ret = FALSE;

   if (!isEnabled)
      return TRUE;

   if ((1 != inBufsSize) || (1 != outBufsSize))
      return FALSE;

   outBufs[0].swap(inBufs[0]);
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
