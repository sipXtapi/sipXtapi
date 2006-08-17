//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprDejitter_h_
#define _MprDejitter_h_

#include "mp/MpMisc.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MprFromNet.h"

// DEFINES
#define DEBUGGING_LATENCY
#undef DEBUGGING_LATENCY

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpConnection;

/// The "Dejitter" media processing resource
class MprDejitter : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

#ifdef DEBUGGING_LATENCY /* [ */
   enum { MAX_RTP_PACKETS = 64};  ///< MUST BE A POWER OF 2, AND SHOULD BE >3
        // 20 Apr 2001 (HZM): Increased from 16 to 64 for debugging purposes.
#else /* DEBUGGING_LATENCY ] [ */
   enum { MAX_RTP_PACKETS = 16};  ///< MUST BE A POWER OF 2, AND SHOULD BE >3
#endif /* DEBUGGING_LATENCY ] */

   enum { GET_ALL = 1 }; ///< get all packets, ignoring timestamps.  For NetEQ

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDejitter(const UtlString& rName, MpConnection* pConn,
      int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprDejitter();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Add a buffer containing an incoming RTP packet to the dejitter pool
   OsStatus pushPacket(const MpRtpBufPtr &pRtp);

     /// Submit all RTP packets to the Jitter Buffer.
   MpRtpBufPtr pullPacket();

     /// Return status info on current backlog.
   OsStatus getPacketsInfo(int& nPackets,
                           unsigned int& lowTimestamp);

   void dumpState();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpRtpBufPtr   mpPackets[MAX_RTP_PACKETS];
   OsBSem        mRtpLock;
   int           mNumPackets;
   int           mNumDiscarded;

   /* end of Dejitter handling variables */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprDejitter(const MprDejitter& rMprDejitter);

     /// Assignment operator (not implemented for this class)
   MprDejitter& operator=(const MprDejitter& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
