//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MprDejitter_h_
#define _MprDejitter_h_

#include "mp/MpMisc.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResource.h"
#include "mp/MprFromNet.h"

// DEFINES
#define DEBUGGING_LATENCY
#undef DEBUGGING_LATENCY
#define MAX_CODECS 10
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpConnection;

//:The "Dejitter" media processing resource
class MprDejitter : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

#ifdef DEBUGGING_LATENCY /* [ */
   enum { MAX_RTP_PACKETS = 64};  // MUST BE A POWER OF 2, AND SHOULD BE >3
        // 20 Apr 2001 (HZM): Increased from 16 to 64 for debugging purposes.
		// 15 Dec 2004: This isn't the actual amount of buffer used, just the size of the container
#else /* DEBUGGING_LATENCY ] [ */
   enum { MAX_RTP_PACKETS = 64};  // MUST BE A POWER OF 2, AND SHOULD BE >3
#endif /* DEBUGGING_LATENCY ] */

   enum { GET_ALL = 1 }; // get all packets, ignoring timestamps.  For NetEQ

/* ============================ CREATORS ================================== */

   MprDejitter(const UtlString& rName, MpConnection* pConn,
      int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprDejitter();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus pushPacket(MpBufPtr pRtp);
     //:Add a buffer containing an incoming RTP packet to the dejitter pool

   MpBufPtr pullPacket(int PayloadType);
     //:Submit all RTP packets to the Jitter Buffer.

   int getAveBufferLength(int PayloadType);
     //:Returns the average number of packets in the jitter buffer since the last call

   OsStatus getPacketsInfo(int& nPackets,
                           unsigned int& lowTimestamp);
     //:Return status info on current backlog.

   void dumpState();

/* ============================ ACCESSORS ================================= */
public:
   static unsigned short getSeqNum(MpBufPtr pRtp);
   static unsigned int getTimestamp(MpBufPtr pRtp);
   static unsigned int getPayloadType(MpBufPtr pRtp);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpBufPtr      mpPackets[MAX_CODECS][MAX_RTP_PACKETS];
   int           mBufferLookup[256];   
   OsBSem        mRtpLock;
   int           mNumPackets[MAX_CODECS];
   int           mNumDiscarded[MAX_CODECS];
   //int			 mPullState[MAX_CODECS];
   int          mFrameCount[MAX_CODECS];
    int         mPacketCount[MAX_CODECS];
//#define DEFAULT_REORDER_BUFFER_LENGTH 3
   // This length is the initial setting for how many buffers we require before
   // we start to pass the buffers on to the remaining processing elements. 
   // For G711, 3 buffers is 20msec * 3 = 80msec.
   // Must be less than MAX_RTP_PACKETS (defined above)
   int			 mBufferLength[MAX_CODECS];

#ifdef DEJITTER_DEBUG /* [ */
   // These are only used if DEJITTER_DEBUG is defined, but I am
   // leaving them in all the time so that changing that definition
   // does not require recompiling more things...
   int           mPullCount;
   int           mLatencyMax;
   int           mLatencyMin;
   int           mPrevNumPackets;
   int           mPrevPullTime;
#endif /* DEJITTER_DEBUG ] */
  int mLastPulled[MAX_CODECS];
  int mLastPushed[MAX_CODECS]; // As packets are added, we change this value to indicate where the
     // buffer is wrapping
  //int mLastSeqNum;
  UtlBoolean bDataFlowing[MAX_CODECS];
   /* end of Dejitter handling variables */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprDejitter(const MprDejitter& rMprDejitter);
     //:Copy constructor (not implemented for this class)

   MprDejitter& operator=(const MprDejitter& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
