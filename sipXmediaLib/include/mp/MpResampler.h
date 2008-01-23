//  
// Copyright (C) 2007-2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com> and Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpResamplerBase_h_
#define _MpResamplerBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpTypes.h>
#include <os/OsStatus.h>
#include <mp/MpMisc.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Generic audio resampler.
*/
class MpResamplerBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   MpResamplerBase(uint32_t numChannels, 
                   uint32_t inputRate, 
                   uint32_t outputRate, 
                   int32_t quality);
     /**<
     *  @copydoc MpResamplerBase::MpResamplerBase(uint32_t,uint32_t,uint32_t)
     *  @param[in] quality - The quality parameter is used by some resamplers to
     *             control the tradeoff of quality for latency and complexity.
     */

     /// Constructor
   MpResamplerBase(uint32_t numChannels, 
                   uint32_t inputRate, 
                   uint32_t outputRate);
     /**<
     *  @param[in] numChannels - The number of channels that the resampler will 
     *             process.
     *  @param[in] inputRate - The sample rate of the input audio.
     *  @param[in] outputRate - The sample rate of the output audio.
     */

     /// Destructor
   virtual ~MpResamplerBase();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Reset resampler state to prepare for processing new (unrelated) stream.
   virtual OsStatus resetStream();

     /// Resample audio data coming from the specified channel.
   virtual OsStatus resample(uint32_t channelIndex,
                             const MpAudioSample* pInBuf,
                             uint32_t inBufLength,
                             uint32_t& inSamplesProcessed,
                             MpAudioSample* pOutBuf,
                             uint32_t outBufLength,
                             uint32_t& outSamplesWritten);
     /**<
     *  @param[in] channelIndex - The index of the channel to process - base 0.
     *  @copydoc MpResamplerBase::resampleInterleavedStereo()
     */

     /// Resample interleaved stereo audio data.
   virtual OsStatus resampleInterleavedStereo(const MpAudioSample* pInBuf,
                                              uint32_t inBufLength,
                                              uint32_t& inSamplesProcessed,
                                              MpAudioSample* pOutBuf,
                                              uint32_t outBufLength,
                                              uint32_t& outSamplesWritten);
     /**<
     *  @param[in] pInBuf - Pointer to the audio to resample.
     *  @param[in] inBufLength - The length in samples of the audio to resample.
     *  @param[out] inSamplesProcessed - The number of samples read from 
     *              /p pInBuf during resampling.
     *  @param[out] pOutBuf - A pointer where the resampled audio will be stored.
     *  @param[in] outBufLength - The length in samples of /p pOutBuf.
     *  @param[out] outSamplesWritten - The number of resampled samples written 
     *              to /p pOutBuf.
     *
     *  @retval OS_INVALID_ARGUMENT if the channelIndex is out of bounds.
     *  @retval OS_SUCCESS if the audio was resampled successfully.
     */

     /// @brief resample the buffer given, and return a new resampled one.
   OsStatus resampleBufPtr(const MpAudioBufPtr& inBuf, MpAudioBufPtr& outBuf,
                           uint32_t inRate, uint32_t outRate,
                           UtlString optionalIdStr = "");
     /**<
     *  Resample the buffer given.  If errors happen, they are logged, outBuf
     *  is left unchanged, and return status is set to a value that is not 
     *  OS_SUCCESS.
     *  
     *  @param[in] inBuf - the ptr to buffer to resample.
     *  @param[out] outBuf - the ptr to the destination that will hold 
     *              the resampled buffer.
     *  @param[in] inRate - The  sample rate that inBuf samples are recorded in.
     *  @param[in] outRate - The sample rate that is requested to be converted to.
     *  @param[in] optionalIdStr - an optional identifier string used when reporting errors.
     *  @retval OS_SUCCESS if the resampling happened without error, outBuf now
     *          will point to resampled buffer.
     *  @retval All other values - failure.
     */

     /// Set the input sample rate, in Hz
   virtual OsStatus setInputRate(const uint32_t inputRate);
     /**<
     *  @param[in] inputRate - The sample rate of the input audio.
     */

     /// Set the output sample rate, in Hz
   virtual OsStatus setOutputRate(const uint32_t outputRate);
     /**<
     *  @param[in] outputRate - The sample rate of the output audio.
     */

     /// Set the quality of resampling conversion
   virtual OsStatus setQuality(const int32_t quality);
     /**<
     *  @param[in] quality - The quality parameter is used by some resamplers to
     *             control the tradeoff of quality for latency and complexity.
     */

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Return input sampling rate.
   uint32_t getInputRate() const;

     /// Return output sampling rate.
   uint32_t getOutputRate() const;

     /// Return quality of resampling conversion.
   int32_t getQuality() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   uint32_t mNumChannels;
   uint32_t mInputRate;
   uint32_t mOutputRate;
   int32_t mQuality;


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#if defined(HAVE_SPEEX)
#  include "mp/MpResamplerSpeex.h"
typedef class MpResamplerSpeex MpResampler;
#else
typedef class MpResamplerBase MpResampler;
#endif

#endif  // _MpResamplerBase_h_
