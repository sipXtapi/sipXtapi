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
   MpJitterBuffer(MpDecoderPayloadMap *pPayloadMap = NULL);
     /**<
     *  @param[in] pPayloadMap - set of decoders, mapped to their RTP payload
     *             types.
     */

   void init(unsigned int samplesPerSec, unsigned int samplesPerFrame);

     /// Destructor
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
   void getFrame(MpAudioBufPtr &pFrame, int &numOriginalSamples);
     /**<
     *  @note \p pFrame must be NULL before passing to this method!
     *
     *  @param[out] pFrame - pointer to returned frame.
     *  @param[out] numOriginalSamples - number of samples in returned frame
     *              would be without resampling.
     */

     /// Update list of available decoders.
   void setCodecList(MpDecoderPayloadMap *pPayloadMap);

     /// Flush all unread buffers and prepare for next decode.
   void flush();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Get number of samples, remaining in buffer.
   inline
   int getSamplesNum() const;

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

   unsigned mCurFrameNum;        ///< Internal sequence number of oldest
                                    ///< frame in mFrames[]. It is also used
                                    ///< as a base for frames index calculation.
   unsigned mRemainingSamplesNum;   ///< Total number of samples still residing
                                    ///< in mFrames[].
   MpAudioBufPtr mFrames[FRAMES_TO_STORE]; ///< Buffer for decoded, resampled and sliced audio.
   int mOriginalSamples[FRAMES_TO_STORE]; ///< Numbers of samples in frames
                                    ///< before resampling was done.

   MpDecoderPayloadMap *mpPayloadMap; ///< Map of RTP payload types to decoders.
                                    ///< Note, we do not own instance of this map,
                                    ///< we only store pointer to it.

   /// Copy constructor
   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);

   /// Assignment operator
   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);
};

/* ============================ INLINE METHODS ============================ */

int MpJitterBuffer::getSamplesNum() const
{
   return mRemainingSamplesNum;
}

#endif  // _MpJitterBuffer_h_
