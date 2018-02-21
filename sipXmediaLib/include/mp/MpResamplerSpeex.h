//  
// Copyright (C) 2008-2018 SIPez LLC. All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpResamplerSpeex_h_
#define _MpResamplerSpeex_h_

#if defined(HAVE_SPEEX) || defined(HAVE_SPEEX_RESAMPLER)

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpResampler.h"
#include <mp/MpTypes.h>
#include <os/OsStatus.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// NOTE: If the below forward declaration causes problems for some compilers
//       due to double-typedef, then it can be corrected by including 
//       speex_resampler.h instead.
typedef struct SpeexResamplerState_ SpeexResamplerState;

/**
*  @brief Wrapper for Speex audio resampler.
*/
class MpResamplerSpeex : public MpResamplerBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

   MpResamplerSpeex(uint32_t numChannels, 
                    uint32_t inputRate, 
                    uint32_t outputRate, 
                    int32_t quality = -1);
     /**<
     *  @copydoc MpResamplerBase::MpResamplerBase(uint32_t,uint32_t,uint32_t,int32_t)
     * Note: Speex resampler introduces latency.  At the default quality (3) latency is introduced
     * (in either direction) roughly as follows:
     *   32000 to 8000: 0.0035 seconds
     *   32000 to 16000: 0.002 seconds
     *   48000 to 8000: 0.003 seconds
     *   48000 to 16000: 0.0015 seconds
     *   48000 to 32000: 0.00075 seconds
     * Using higher quality values also increases the latency.
     */

   ~MpResamplerSpeex();
     /**<
     *  @copydoc MpResamplerBase::~MpResamplerBase()
     */

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   OsStatus resetStream();
     /**<
     *  @copydoc MpResamplerBase::resetStream()
     */

   OsStatus resample(uint32_t channelIndex,
                     const MpAudioSample* pInBuf,
                     uint32_t inBufLength,
                     uint32_t& inSamplesProcessed,
                     MpAudioSample* pOutBuf,
                     uint32_t outBufLength,
                     uint32_t& outSamplesWritten);
     /**<
     *  @copydoc MpResamplerBase::resample()
     *  @retval OS_INVALID if the speex resampler encounters pointer overlap.
     */

   OsStatus resampleInterleavedStereo(const MpAudioSample* pInBuf,
                                      uint32_t inBufLength,
                                      uint32_t& inSamplesProcessed,
                                      MpAudioSample* pOutBuf,
                                      uint32_t outBufLength,
                                      uint32_t& outSamplesWritten);
     /**<
     *  @copydoc MpResamplerBase::resampleInterleavedStereo()
     *  @retval OS_INVALID if the speex resampler encounters pointer overlap.
     */

   OsStatus setInputRate(const uint32_t inputRate);
     /**<
     *  @copydoc MpResamplerBase::setInputRate()
     *  @param[in] inputRate - The sample rate of the input audio.
     */

     // Set the output sample rate, in Hz
   OsStatus setOutputRate(const uint32_t outputRate);
     /**<
     *  @copydoc MpResamplerBase::setOutputRate()
     */

     // Set the quality of resampling conversion
   OsStatus setQuality(const int32_t quality);
     /**<
     *  @copydoc MpResamplerBase::setQuality()
     */

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   OsStatus speexErrToOsStatus(int speexErr);

private:
   SpeexResamplerState* mpState;

};

/* ============================ INLINE METHODS ============================ */

#endif  // HAVE_SPEEX
#endif  // _MpResamplerSpeex_h_
