//  
// Copyright (C) 2006-2014 SIPez LLC.  All rights reserved.
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
class MpVadBase;
class MpAgcBase;
class MpFlowGraphBase;

/**
*  @brief Class for decoding of incoming RTP, resampling it to target
*         sample rate and slicing to frames of target size.
*
*  This class is not thread-safe. For thread-safety it relies on external
*  synchronization mechanisms in MprDecode.
*/
class MpJitterBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpJitterBuffer(MpDecoderPayloadMap *pPayloadMap = NULL,
                  const UtlString& resourceName = "unknown");
     /**<
     *  @param[in] pPayloadMap - set of decoders, mapped to their RTP payload
     *             types.
     *  @param[in] resourceName - name of the resource that this is part
     *             of.  Used for debugging and logging purposes.
     */

     /// Initialize with given sample rate and frame size.
   void init(unsigned int samplesPerSec, unsigned int samplesPerFrame);
     /**<
     *  Should be called only once upon construction of an object.
     */

     /// Destructor
   ~MpJitterBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Reset class to the initial state, preparing for handling new stream.
   void reset();

     /// Push packet into decoder buffer.
   OsStatus pushPacket(const MpRtpBufPtr &rtpPacket,
                       int minBufferSamples,
                       int wantedBufferSamples,
                       int &decodedSamples,
                       int &adjustment,
                       UtlBoolean &played);
     /**<
     *  Packet will be decoded and decoded data will be copied to internal buffer.
     *  If no decoder is available for this packet's payload type packet will be
     *  ignored.
     *
     *  @param[in]  rtpPacket - RTP packet to be decoded.
     *  @param[in]  minBufferSamples - minimum number of samples to remain in
     *              buffer after decoding and applying adjustment. This is
     *              useful, when we do not want to decode more packets
     *              at this moment, but need a frame of audio for processing.
     *  @param[in]  wantedBufferSamples - number of samples we want to have
     *              in buffer after decoding packet. Note, this value may be
     *              negative - this means we want to shorten our buffer as
     *              much as possible.
     *  @param[out] decodedSamples - number of samples, decoded from packet.
     *  @param[out] adjustment - how many samples were added or removed from
     *              stream to fulfil \p wantedBufferSamples request.
     *  @param[out] played - was passed RTP packet decoded and added to
     *              decoder buffer, or it was dropped or used just to update
     *              PLC history. I.e. if \p played=FALSE then number of samples
     *              in buffer was not increased.
     *
     *  @note This implementation behave unpredictable if packets come reordered.
     *  @note Valid RTP packet MUST be passed with first call to this function.
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

     /// Change PLC algorithm to one provided.
   void setPlc(const UtlString &plcName);


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   /// Set the pointer to the parent flowgraph for debug purposes.
   void setFlowGraph(MpFlowGraphBase* pFlowgraph);

      /// Get number of samples, remaining in buffer.
   inline
   int getSamplesNum() const;

   /// Set parameter on the VAD component
   OsStatus setVadParam(const UtlString& name, int value);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Reduce or extend audio fragment.
   int adjustStream(MpAudioSample *pBuffer, int bufferSize, unsigned numSamples,
                    int wantedAdjustment);

   OsStatus sliceToFrames(int decodedSamples, int codecSampleRate,
                          const MpSpeechParams &speechParams);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum {
      FRAMES_TO_STORE = 32,         ///< Number of frames to store in buffer.
                                    ///< Should be a power of 2.
      DECODED_DATA_MAX_LENGTH = 10 * 160 ///< Size of mDecodedData temporary buffer.
   };

///@name Resampler variables.
//@{
   int mStreamSampleRate;           ///< Sample rate of incoming RTP stream.
   int mOutputSampleRate;           ///< Output sample rate for decoded data.
                                    ///< Samples from codecs with different
                                    ///< sample rates will be resampled to this
                                    ///< sample rate.
   int mSamplesPerFrame;            ///< Number of samples to put to output buffers.
   MpAudioSample mDecodedData[DECODED_DATA_MAX_LENGTH]; ///< Buffer, used to
                                    ///< temporarily store decoded data.
   MpResamplerBase *mpResampler;    ///< Resampler instance to convert codec
                                    ///< sample rate to flowgraph sample rate.
//@}

///@name Audio buffers variables.
//@{
   unsigned mCurFrameNum;           ///< Internal sequence number of oldest
                                    ///< frame in mFrames[]. It is also used
                                    ///< as a base for frames index calculation.
   unsigned mRemainingSamplesNum;   ///< Total number of samples still residing
                                    ///< in mFrames[].
   MpAudioBufPtr mFrames[FRAMES_TO_STORE]; ///< Buffer for decoded, resampled and sliced audio.
   int mOriginalSamples[FRAMES_TO_STORE]; ///< Numbers of samples in frames
                                    ///< before resampling was done.
//@}

///@name Decoding related variables.
//@{
   UtlBoolean mIsFirstPacket;       ///< Have we received our first packet or not.
   RtpSeq mStreamSeq;               ///< Sequence number of last played RTP packet.
   RtpTimestamp mStreamTimestamp;   ///< Timestamp of last played RTP packet.
   uint8_t mStreamRtpPayload;       ///< Payload type of last received RTP packet.
   MpDecoderPayloadMap *mpPayloadMap; ///< Map of RTP payload types to decoders.
                                    ///< Note, we do not own instance of this map,
                                    ///< we only store pointer to it.
   unsigned mSamplesPerPacket;      ///< Number of samples in RTP packet.
//@}

   UtlString  mPlcName;             ///< Packet Loss Concealer algorithm name.
   MpPlcBase *mpPlc;                ///< Packet Loss Concealer instance.
   MpVadBase *mpVad;                ///< Voice Activity Detector instance.
   MpAgcBase *mpAgc;                ///< Automatic Gain Calculator instance.
   MpFlowGraphBase* mpFlowGraph;    ///< Parent flowgraph for debugging

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
