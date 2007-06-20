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

#ifndef _MprDejitter_h_
#define _MprDejitter_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsBSem.h"
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// The "Dejitter" media processing resource
class MprDejitter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum {
      MAX_RTP_PACKETS = 64,  ///< MUST BE A POWER OF 2, AND SHOULD BE >3
        // 20 Apr 2001 (HZM): Increased from 16 to 64 for debugging purposes.
        // 15 Dec 2004: This isn't the actual amount of buffer used, just the size of the container

      MAX_CODECS = 10, ///< Maximum number of codecs in incoming RTP streams.

      GET_ALL = 1    ///< get all packets, ignoring timestamps.  For NetEQ
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDejitter();

     /// Destructor
   virtual
   ~MprDejitter();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Add a buffer containing an incoming RTP packet to the dejitter pool
   OsStatus pushPacket(const MpRtpBufPtr &pRtp);
     /**<
     *  This method places the packet to the pool depending the modulo division
     *  value.
     *
     *  @return OS_SUCCESS on success
     *  @return OS_LIMIT_REACHED if too many codecs used in incoming RTP packets
     */

     /// Get next RTP packet, or NULL if none is available.
   MpRtpBufPtr pullPacket(int payloadType);
     /**<
     *  @note Significant change is that the downstream puller may NOT pull all
     *        the available packets at once. Instead it is paced according to
     *        the needs of the RTP payload type.
     *
     *  This buffer is the primary dejitter/reorder buffer for the internal
     *  codecs. Some codecs may do their own dejitter stuff too. But we can't
     *  eliminate this buffer because then out-of-order packets would just be
     *  dumped on the ground.
     *
     *  This buffer does NOT substitute silence packets. That is done in
     *  MpJitterBuffer called from MprDecode.
     *
     *  If packets arrive out of order, and the newer packet has already been
     *  pulled due to the size of the jitter buffer set by the codec, this
     *  buffer will NOT discard the out-of-order packet, but send it along
     *  anyway it is up to the codec to discard the packets it cannot use. This
     *  allows this JB to be a no-op buffer for when the commercial library is
     *  used.
     */

     /// Get next RTP packet with given timestamp, or NULL if none is available.
   MpRtpBufPtr pullPacket(int payloadType, RtpTimestamp timestamp, bool lockTimestamp=true);
     /**<
     *  This version of pullPacket() works exactly the same as above version
     *  of pullPacket() with one exception: if (lockTimestamp == true) it checks 
     *  every found packet's timestamp. And return NULL if there are no packets
     *  with timestamp less or equal then passed timestamp.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     // Return number of packets in buffer for given payload type.
   int getBufferLength(int payload);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

                  /// Mutual exclusion lock for internal data
   OsBSem        mRtpLock;

                  /// Buffer for incoming RTP packets
   MpRtpBufPtr   mpPackets[MAX_CODECS][MAX_RTP_PACKETS];

                  /// Mapping of payload type to internal codec index
   int           mBufferLookup[256];

                  /// Number of packets in buffer
   int           mNumPackets[MAX_CODECS];

                  /// Number of packets overwritten with newly came packets.
   int           mNumDiscarded[MAX_CODECS];
                  /**<
                  *  If this value is not zero, then jitter buffer length
                  *  is not enough, or there are clock skew.
                  *   
                  *  Used for only debug purposes for now.
                  */

                  /// Index of the last inserted packet.
   int           mLastPushed[MAX_CODECS];
                  /**<
                  *  As packets are added, we change this value to indicate
                  *  where the buffer is wrapping.
                  */

   /* end of Dejitter handling variables */

     /// Copy constructor (not implemented for this class)
   MprDejitter(const MprDejitter& rMprDejitter);

     /// Assignment operator (not implemented for this class)
   MprDejitter& operator=(const MprDejitter& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
