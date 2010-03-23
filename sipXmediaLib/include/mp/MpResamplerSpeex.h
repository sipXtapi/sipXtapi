//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
