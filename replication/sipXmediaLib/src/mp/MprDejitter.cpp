//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//The averaged latency of packets in dejitter buffer is calculated in method 
//PullPacket( ) for the purpose of dejitter buffer
//backlog control (or called jitter control) by the decoder in down stream. 
//The decoder will look at the latency at certain frequency to make 
//the decision. -Brian Puh
//

#define DEJITTER_DEBUG
#undef  DEJITTER_DEBUG

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
#include "os/OsDateTime.h"
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

unsigned short MprDejitter::getSeqNum(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return ntohs(((struct rtpHeader*) MpBuf_getStorage(pRtp))->seq);
}

unsigned int MprDejitter::getTimestamp(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return ntohl(((struct rtpHeader*) MpBuf_getStorage(pRtp))->timestamp);
}

unsigned int MprDejitter::getPayloadType(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return (0x7f & (((struct rtpHeader*) MpBuf_getStorage(pRtp))->mpt));
}

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter(const UtlString& rName, MpConnection* pConn,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL)
   /* for Dejitter handling */
#ifdef DEJITTER_DEBUG /* [ */
   , mPullCount(0),
   mLatencyMax(0x80000000),
   mLatencyMin(0x7FFFFFFF)
#endif /* DEJITTER_DEBUG ] */
{
   memset(mpPackets, 0, MAX_RTP_PACKETS * MAX_CODECS * sizeof(MpBufPtr));
   memset(mBufferLookup,-1,256 * sizeof(int));
   memset(mNumPackets,0,MAX_CODECS * sizeof(int));
   memset(mNumDiscarded,0,MAX_CODECS * sizeof(int));
   memset(mLastPulled,-1,MAX_CODECS * sizeof(int));
   memset(mFrameCount,0,MAX_CODECS * sizeof(int));
   memset(mPacketCount,0,MAX_CODECS * sizeof(int));
   for(int ii=0;ii<MAX_CODECS;ii++) 
   {
        bDataFlowing[ii]=false;
   }
   //memset(mPullState,0,MAX_CODECS * sizeof(int));
   //memset(mPullState,DEFAULT_REORDER_BUFFER_LENGTH,MAX_CODECS * sizeof(int));
}

// Destructor
MprDejitter::~MprDejitter()
{
   int i,j;

   mRtpLock.acquire();
   for (i=0; i<MAX_RTP_PACKETS; i++) {
       for(j=0; j<MAX_CODECS; j++) 
       {
            MpBuf_delRef(mpPackets[j][i]);
            mpPackets[j][i] = NULL;
       }
   }
   mRtpLock.release();
}
/* ============================ MANIPULATORS ============================== */

//Add a buffer containing an incoming RTP packet to the dejitter pool.
// This method places the packet in the place in the the pool dependingin the modulo division value
//
//int iFrameCount=0;
//int iPacketCount=0;

int MprDejitter::getAveBufferLength(int pt)
{
	if((mFrameCount[pt]==0)||(mPacketCount[pt]==0)) return 0;
	int iAve = mPacketCount[pt] / mFrameCount[pt];
	mPacketCount[pt]=0;
	mFrameCount[pt] = 0;
	return iAve;
}

OsStatus MprDejitter::pushPacket(MpBufPtr pRtp)
{
   int index;

   MpBuf_touch(pRtp);
   mRtpLock.acquire();
   int pt = getPayloadType(pRtp);
   //int iNo = getSeqNum(pRtp);
OsTime timeNow;
         OsDateTime::getCurTime(timeNow);
   //if(pt > 90) osPrintf("DTMF seqno is %d\n",iNo);
   //osPrintf("%d ",pt);
   int jb = mBufferLookup[pt];
   if(jb < 0) 
   {
       int maxjb=-1;
        for(int ii=0;ii<256;ii++) 
        {
            maxjb = max(maxjb,mBufferLookup[ii]);
        }
        maxjb++;
        mBufferLookup[pt]=maxjb;
        jb = maxjb;
   }
   index = getSeqNum(pRtp) % MAX_RTP_PACKETS;
   if (NULL != mpPackets[jb][index]) {
	   // Check for packets already in the buffer. Overwrite them if 
	   // the just-arriving packet is newer than the existing packet
	   // Don't overwrite if the just-arriving packet is older
      int iBufSeqNo = getSeqNum(mpPackets[jb][index]);
	  int iNewSeqNo = getSeqNum(pRtp);

	  if(iNewSeqNo > iBufSeqNo) 
	  {
		  // Insert the new packet over the old packet
	      mNumDiscarded[jb]++;
          if (mNumDiscarded[jb] < 40) 
		  {
         osPrintf("Dej: discard#%d Seq: %d -> %d at 0x%X\n",
                      mNumDiscarded[jb], getSeqNum(mpPackets[jb][index]),
            getSeqNum(pRtp), *pOsTC);
      }
          MpBuf_delRef(mpPackets[jb][index]);
          mpPackets[jb][index] = NULL;
          mpPackets[jb][index] = pRtp;
		  mLastPushed[jb] = index;  
		  // mNumPackets++;  Neutral, since we discarded a packet, and added one
	  } else {
		  // Don't insert the new packet - it is a old delayed packet
	  }
   } else {
		mLastPushed[jb] = index;  
		mpPackets[jb][index] = pRtp;
		mLastPushed[jb] = index;	
		mNumPackets[jb]++;
   }
   mRtpLock.release();

   return OS_SUCCESS;
}

// Get a pointer to the next RTP packet, or NULL if none is available.
int iDebugCnt=0;
MpBufPtr MprDejitter::pullPacket(int PayloadType)  
{
   /* 
   Initial conditions of an empty buffer. Fill buffer to "DEFAULT_REORDER_BUFFER_LENGTH" number of
   buffers. Since at this point, we don't know the frequency of the packets coming in, the
   actual time of initial delay will vary by codec. Once packets are flowing, 
   the next element (MprDecode) will come back in and adjust the delay based on 
   codec type and perhaps other dynamic conditions/

   NOTE: Significant change is that the downstream puller may NOT pull all the available
         packets at once. Instead it is paced according to the needs of the RTP paylod type.

   This buffer is the primary dejitter/reorder buffer for the internal codecs. From what I
   understand, the GIPS codecs may do their own dejitter stuff too. 
   But we can't eliminate this buffer because
   then out-of-order packets would just be dumped on the ground.

   This buffer does NOT substitute silence packets. 
   That is done in MpJitterBuffer called from MprDecode.

   If packets arrive out of order, and the newer packet has already been pulled due to the size of the
   jitter buffer set by the codec, this buffer will NOT discard the out-of-order packet, but send it along anyway
   it is up to the codec to discard the packets it cannot use. This allows this JB to be a no-op buffer
   for when the commercial library is used.
   */
   MpBufPtr found = NULL;
   int first = -1; 
   //osPrintf("%d ",PayloadType);
    int jb = mBufferLookup[PayloadType];
   if(jb < 0) 
   {
       int maxjb=-1;
        for(int ii=0;ii<256;ii++) 
       {
            maxjb = max(maxjb,mBufferLookup[ii]);
        }
        maxjb++;
        mBufferLookup[PayloadType]=maxjb;
        jb = maxjb;
   }

   // when there are no packets flowing out, buffer up to mBufferLength before we
   // turn on the flow. Then the flow continues all the time regardless of how many
   // packets are currently in the circular buffer.
   if(mNumPackets[jb]==0) {
	   // If we run out of packets in the buffer, reset to a "no data flowing" condition
	   // and then let the buffer fill up to mBufferLength packets again
	   mLastPulled[jb] = -1;
	   return NULL;
           }

   mRtpLock.acquire();
	// we find a packet by starting to look in the JB just AFTER where the latest push was
    // done, and loop around until we reach the last pulled packet index.
	int iNextPull = mLastPushed[jb] + 1;
	if(iNextPull >= MAX_RTP_PACKETS) iNextPull = 0;
	found=NULL;
	int iFound = 0;
	while(iNextPull != mLastPushed[jb]) {
        found = mpPackets[jb][iNextPull];
		if(found !=NULL) {
			iFound=1;
			break;
		}
		iNextPull++;
		if(iNextPull >= MAX_RTP_PACKETS) iNextPull = 0;
	}
    // Look for the last packet...
    if((iFound==0) && (iNextPull ==  mLastPushed[jb])) {
        found = mpPackets[jb][iNextPull];
		    if(found !=NULL) {
			iFound=1;
           }
       }
	if(iFound==0) {
		mRtpLock.release();
		return NULL;
   }
	mpPackets[jb][iNextPull]=0;
	mNumPackets[jb]--;
      MpBuf_touch(found);
   mRtpLock.release();

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
	//iPacketCount += mNumPackets;
	//iFrameCount++;
   if (!isEnabled) return TRUE;

   if ((1 != inBufsSize) || (1 != outBufsSize))
      ret = FALSE;
   else
   {
      *outBufs = *inBufs;
      *inBufs = NULL;
      ret = TRUE;
   }
   return ret;
}

/* ============================ FUNCTIONS ================================= */
#ifdef DEJITTER_DEBUG /* [ */
int dejitterdebug(int Flag)
{
    int save = iShowDejitterInfoFlag;
    if( Flag != 0) {
        Flag = 1;
    }
    iShowDejitterInfoFlag = Flag;
    return (save);
}
int ShowDejitterInfo(int iFlag) {
   return dejitterdebug(iFlag);
}
#endif /* DEJITTER_DEBUG ] */
