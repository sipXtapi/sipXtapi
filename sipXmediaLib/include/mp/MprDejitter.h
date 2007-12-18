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
#define LONG_DEJITTER

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
#ifdef LONG_DEJITTER // [
      MAX_RTP_PACKETS = 64,  ///< MUST BE A POWER OF 2, AND SHOULD BE >3
        // 20 Apr 2001 (HZM): Increased from 16 to 64 for debugging purposes.
        // 15 Dec 2004: This isn't the actual amount of buffer used, just the size of the container
#else // LONG_DEJITTER ][
      MAX_RTP_PACKETS = 3,  ///< Could be any value.
#endif // LONG_DEJITTER ]

      MAX_CODECS = 10, ///< Maximum number of codecs in incoming RTP streams.
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
   MpRtpBufPtr pullPacket(int payloadType, bool isSignaling=false);
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
     *
     *  If pulled packet belong to signaling codec (e.g. RFC2833 DTMF), then
     *  set isSignaling to true. Else packet will be hold for undefined
     *  amount of time, possible forever.
     */

     /// Get next RTP packet with given timestamp, or NULL if none is available.
   MpRtpBufPtr pullPacket(int payloadType, RtpTimestamp timestamp,
                          bool lockTimestamp=true, bool isSignaling=false);
     /**<
     *  This version of pullPacket() works exactly the same as above version
     *  of pullPacket() with one exception: if (lockTimestamp == true) it checks 
     *  every found packet's timestamp. And return NULL if there are no packets
     *  with timestamp less or equal then passed timestamp.
     *
     *  If pulled packet belong to signaling codec (e.g. RFC2833 DTMF), then
     *  set isSignaling to true. Else packet will be hold for undefined
     *  amount of time, possible forever.
     */

     /// Call this function on every frame processing tick.
   void frameIncrement(int samplesNum);
     /**<
     *  This tells dejitter that we have to pull next packet from it.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return number of packets in buffer for given payload type.
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

   /// Storage for all stream related data.
   /**
   *  We're able to handle several RTP streams with one instance of dejitter.
   *  This structure encapsulate all data, specific to stream as opposite to
   *  global dejitter data.
   */
   struct StreamData
   {
       /// Constructor, initialize data to meaningful initial state.
      StreamData()
      : mNumPackets(0)
      , mNumDiscarded(0)
      , mLastPushed(0)
      {
      }

                     /// Buffer for incoming RTP packets
      MpRtpBufPtr   mpPackets[MAX_RTP_PACKETS];

                     /// Number of packets in buffer
      int           mNumPackets;

                     /// Number of packets overwritten with newly came packets.
      int           mNumDiscarded;
                     /**<
                     *  If this value is not zero, then jitter buffer length
                     *  is not enough, or there are clock skew.
                     *   
                     *  Right now used for debug purposes only.
                     */

                     /// Index of the last inserted packet.
      int           mLastPushed;
                     /**<
                     *  As packets are added, we change this value to indicate
                     *  where the buffer is wrapping.
                     */

      unsigned int mWaitTimeInFrames; ///< Size of jitter buffer. Frames will be
                                      ///< delayed for mWaitTimeInFrames*20ms.
      int mUnderflowCount;
      RtpTimestamp mTimestampOffset;  ///< Difference between our local clocks
                                      ///< and remote clocks.
      RtpSeq mLastSeqNo;      ///< Keep track of the last sequence number so that
                              ///< we don't take out-of-order packets.
      bool mIsFirstFrame;     ///< True, if no frames decoded.
      bool mClockDrift;       ///< True, if clock drift detected.
      int mLastReportSize;
      RtpSRC mLastSSRC;       ///< RTP SSRC of currently processed stream

        /// Reset all data to meaningful initial state.
      void resetStream();
        /**<
        *  Call this function when stream source have changed to start
        *  collecting new statistic.
        */

        /// Check do we want this RTP packet or not.
      int checkPacket(const MpRtpBufPtr &pPacket,
                      RtpTimestamp nextPullTimestamp,
                      UtlBoolean isSignaling);
        /**<
        *  @param[in] pPacket - RTP packet to check.
        *  @param[in] nextPullTimestamp - timestamp of packet will be pulled next.
        *  @param[in] isSignaling - Is this codec signaling (e.g. RFC2833 DTMF)?
        *             Packets from signaling codecs are always accepted.
        *
        *  @note This method can be called more than one time per frame interval.
        *
        *  @retval >0 - pass this buffer to decoder
        *  @retval 0  - wait for next tick
        *  @retval -1 - discard packet (e.g. out of order packet)
        */

        /// Update collected statistic.
      void updateStatistic(int averageLength);
        /**<
        *  Every second this function is called with the average number of
        *  packets in the dejitter. We want to keep it near target buffer size.
        */
   };

                  /// Mutual exclusion lock for internal data
   OsBSem        mRtpLock;

                  /// Timestamp of frame we expect next
   RtpTimestamp  mNextPullTimerCount;
                  /**<
                  *  This is kept global, because we should keep all streams
                  *  in sync.
                  */

                  /// Number of frames, passed since last statistic update
   int mFramesSinceLastUpdate;

                  /// Storage for all stream related data
   StreamData    mpStreamData[MAX_CODECS];

                  /// Mapping of payload type to internal codec index
   int           mBufferLookup[256];

     /// Copy constructor (not implemented for this class)
   MprDejitter(const MprDejitter& rMprDejitter);

     /// Assignment operator (not implemented for this class)
   MprDejitter& operator=(const MprDejitter& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
