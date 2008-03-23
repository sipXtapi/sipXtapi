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


#ifndef _MpJitterBuffer_h_
#define _MpJitterBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpRtpBuf.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpResampler.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpDecoderPayloadMap;
class MpPlcBase;

/**
*  @brief Class for decoding of incoming RTP, resampling it to target
*         sample rate and slicing to frames of target size.
*/
class MpJitterBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpJitterBuffer(const UtlString &plcName = "",
                  MpDecoderPayloadMap *pPayloadMap = NULL);
     /**<
     *  @param[in] plcName - name of PLC algorithm to use.
     *  @param[in] pPayloadMap - set of decoders, mapped to their RTP payload
     *             types.
     */

   void init(unsigned int samplesPerSec, unsigned int samplesPerFrame);

     /// Destructor
   virtual
   ~MpJitterBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Push packet into decoder buffer.
   OsStatus pushPacket(MpRtpBufPtr &rtpPacket);
     /**<
     *  Packet will be decoded and decoded data will be copied to internal buffer.
     *  If no decoder is available for this packet's payload type packet will be
     *  ignored.
     *
     *  @note This implementation does not check packets sequence number in any
     *  manner. So it behave very bad if packets come reordered or if some
     *  packets are missed.
     *
     *  @retval OS_SUCCESS if RTP packet was successfully decoded.
     *  @retval OS_FAILED in case of any problems.
     */

     /// Get next frame from decoder buffer.
   MpAudioBufPtr getSamples();

     /// Update list of available decoders.
   void setCodecList(MpDecoderPayloadMap *pPayloadMap);

     /// Change PLC algorithm to the one with given name.
   void setPlc(const UtlString &plcName = "");
     /**<
     *  @param[in] plcName - name of PLC algorithm to use. If given name is
     *             not known, default PLC algorithm will be used. Thus, to
     *             select default algorithm, empty string could be used.
     */

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

   enum {
      FRAMES_TO_STORE = 16,         ///< Number of frames to store in buffer.
                                    ///< Should be a power of 2.
      DECODED_DATA_MAX_LENGTH = 6 * 160 ///< Size of mDecodedData temporary buffer.
   };

   unsigned int mSampleRate;        ///< Output sample rate for decoded data.
                                    ///< Samples from codecs with different
                                    ///< sample rate will be resampled to this
                                    ///< sample rate.
   unsigned int mSamplesPerFrame;   ///< Number of samples to put to output buffers.
   MpAudioSample mDecodedData[DECODED_DATA_MAX_LENGTH]; ///< Buffer, used to
                                    ///< temporarily store decoded data.
   MpResamplerBase *mpResampler;    ///< Resampler instance to convert codec
                                    ///< sample rate to flowgraph sample rate.

   UtlBoolean mIsFirstPacket;       ///< Is next packet first received or not?
   RtpTimestamp mOldestTimestamp;   ///< Oldest timestamp in buffer.
   unsigned mOldestFrameNum;        ///< Internal sequence number of oldest
                                    ///< frame in mFrames[]. It is also used
                                    ///< as a base for frames index calculation.
   MpAudioBufPtr mFrames[FRAMES_TO_STORE]; ///< Buffer for decoded, resampled and sliced audio.

   MpDecoderPayloadMap *mpPayloadMap; ///< Map of RTP payload types to decoders.
                                    ///< Note, we do not own instance of this map,
                                    ///< we only store pointer to it.
   MpPlcBase *mpPlc;                ///< Instance of PLC algorithm.
   MpAudioBufPtr mTempPlcFrame;     ///< This audio frame pointer is used to
                                    ///< reduce number of frames allocations/
                                    ///< deallocations by keeping unused
                                    ///< frame between calls to getSamples().

   /// Copy constructor
   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);

   /// Assignment operator
   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBuffer_h_
