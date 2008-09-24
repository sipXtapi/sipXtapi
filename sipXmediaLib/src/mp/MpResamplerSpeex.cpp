//  
// Copyright (C) 2007-2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifdef HAVE_SPEEX

// WIN32: Add libspeex to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libspeex.lib")
#endif // WIN32 ]

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpResamplerSpeex.h>
#include <speex/speex_resampler.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpResamplerSpeex::MpResamplerSpeex(uint32_t numChannels, 
                                   uint32_t inputRate, uint32_t outputRate, 
                                   int32_t quality)
: MpResamplerBase(numChannels, inputRate, outputRate,
                  quality >= 0 ? quality : SPEEX_RESAMPLER_QUALITY_VOIP)
{
   // Call speex init.
   int speexErr = 0;
   mpState = speex_resampler_init(mNumChannels, mInputRate, mOutputRate, 
                                  mQuality, &speexErr);

   assert(speexErr == RESAMPLER_ERR_SUCCESS);
}

MpResamplerSpeex::~MpResamplerSpeex()
{
   // Destroy speex state object
   speex_resampler_destroy(mpState);
   mpState = NULL;
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpResamplerSpeex::resetStream()
{
   return speexErrToOsStatus(speex_resampler_reset_mem(mpState));
}

// Single-channel resampling.
OsStatus MpResamplerSpeex::resample(uint32_t channelIndex,
                                    const MpAudioSample* pInBuf, uint32_t inBufLength, 
                                    uint32_t& inSamplesProcessed, 
                                    MpAudioSample* pOutBuf, uint32_t outBufLength, 
                                    uint32_t& outSamplesWritten)
{
   OsStatus ret = OS_FAILED;
   if(channelIndex >= mNumChannels)
   {
      // Specified a channel number that was outside the defined number of channels!
      return OS_INVALID_ARGUMENT;
   }

   inSamplesProcessed = inBufLength;
   outSamplesWritten = outBufLength;
   int speexErr = speex_resampler_process_int(mpState, channelIndex, 
                                              pInBuf, &inSamplesProcessed, 
                                              pOutBuf, &outSamplesWritten);
   return speexErrToOsStatus(speexErr);
}

// Interleaved stereo resampling.
OsStatus MpResamplerSpeex::resampleInterleavedStereo(const MpAudioSample* pInBuf, 
                                                     uint32_t inBufLength,
                                                     uint32_t& inSamplesProcessed,
                                                     MpAudioSample* pOutBuf, 
                                                     uint32_t outBufLength,
                                                     uint32_t& outSamplesWritten)
{
   if(mNumChannels != 2)
   {
      // Cannot do interleaved stereo resampling when internal state does not 
      // indicate we have 2 channels.
      return OS_INVALID_STATE;
   }

   inSamplesProcessed = inBufLength;
   outSamplesWritten = outBufLength;
   int speexErr = speex_resampler_process_interleaved_int(mpState, 
                                                          pInBuf, 
                                                          &inSamplesProcessed, 
                                                          pOutBuf, 
                                                          &outSamplesWritten);
   return speexErrToOsStatus(speexErr);
}

OsStatus MpResamplerSpeex::setInputRate(const uint32_t inputRate)
{
   OsStatus stat = MpResamplerBase::setInputRate(inputRate);
   if(stat == OS_SUCCESS)
   {
      stat = speexErrToOsStatus(speex_resampler_set_rate(mpState, mInputRate, mOutputRate));
   }
   return stat;
}

OsStatus MpResamplerSpeex::setOutputRate(const uint32_t outputRate)
{
   OsStatus stat = MpResamplerBase::setOutputRate(outputRate);
   if(stat == OS_SUCCESS)
   {
      stat = speexErrToOsStatus(speex_resampler_set_rate(mpState, mInputRate, mOutputRate));
   }
   return stat;
}

OsStatus MpResamplerSpeex::setQuality(const int32_t quality)
{
   OsStatus stat = MpResamplerBase::setQuality(quality);
   if(stat == OS_SUCCESS)
   {
      stat = speexErrToOsStatus(speex_resampler_set_quality(mpState, mQuality));
   }
   return stat;
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */

OsStatus MpResamplerSpeex::speexErrToOsStatus(int speexErr)
{
   OsStatus ret = OS_SUCCESS;
   switch(speexErr)
   {
   case RESAMPLER_ERR_SUCCESS:
      ret = OS_SUCCESS;
      break;
   case RESAMPLER_ERR_ALLOC_FAILED:
      ret = OS_NO_MEMORY;
      break;
   case RESAMPLER_ERR_BAD_STATE:
      ret = OS_INVALID_STATE;
      break;
   case RESAMPLER_ERR_INVALID_ARG:
      ret = OS_INVALID_ARGUMENT;
      break;
   case RESAMPLER_ERR_PTR_OVERLAP:
      ret = OS_INVALID;
      break;
   default:
      ret = OS_FAILED;
      break;
   }
   return ret;
}

/* ============================== FUNCTIONS =============================== */

#endif // HAVE_SPEEX
