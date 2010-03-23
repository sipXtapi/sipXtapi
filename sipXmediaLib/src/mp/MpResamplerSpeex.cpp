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

#if defined(HAVE_SPEEX) || defined(HAVE_SPEEX_RESAMPLER)
// WIN32: Add libspeexdsp to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libspeexdsp.lib")
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
/// Size of chunks of data we pass to Speex resampler.
/// I'd recommend to set it to be equal to Speex's FIXED_STACK_ALLOC (1024 by default).
#define SPEEX_RESAMPLER_FRAME    1024

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
                                    const MpAudioSample* pInBuf,
                                    uint32_t inBufLength, 
                                    uint32_t& inSamplesProcessed, 
                                    MpAudioSample* pOutBuf,
                                    uint32_t outBufLength, 
                                    uint32_t& outSamplesWritten)
{
   if(channelIndex >= mNumChannels)
   {
      // Specified a channel number that was outside the defined number of channels!
      return OS_INVALID_ARGUMENT;
   }

   for (inSamplesProcessed=0, outSamplesWritten=0;
        inSamplesProcessed<inBufLength && outSamplesWritten<outBufLength;
        )
   {
      spx_uint32_t inSamplesNum = sipx_min(SPEEX_RESAMPLER_FRAME, inBufLength-inSamplesProcessed);
      spx_uint32_t outSamplesNum = sipx_min(SPEEX_RESAMPLER_FRAME, outBufLength-outSamplesWritten);
      int speexErr = speex_resampler_process_int(mpState, channelIndex,
                                                 (const spx_int16_t*)&pInBuf[inSamplesProcessed],
                                                 &inSamplesNum,
                                                 (spx_int16_t*)&pOutBuf[outSamplesWritten],
                                                 &outSamplesNum);
      if (speexErr != RESAMPLER_ERR_SUCCESS)
      {
         return speexErrToOsStatus(speexErr);
      }
      inSamplesProcessed += inSamplesNum;
      outSamplesWritten += outSamplesNum;
   }

   return OS_SUCCESS;
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


   for (inSamplesProcessed=0, outSamplesWritten=0;
        inSamplesProcessed<inBufLength && outSamplesWritten<outBufLength;
        )
   {
      spx_uint32_t inSamplesNum = sipx_min(SPEEX_RESAMPLER_FRAME, inBufLength-inSamplesProcessed);
      spx_uint32_t outSamplesNum = sipx_min(SPEEX_RESAMPLER_FRAME, outBufLength-outSamplesWritten);
      int speexErr = speex_resampler_process_interleaved_int(mpState,
                                                             (const spx_int16_t*)&pInBuf[inSamplesProcessed],
                                                             &inSamplesNum,
                                                             (spx_int16_t*)&pOutBuf[outSamplesWritten],
                                                             &outSamplesNum);
      if (speexErr != RESAMPLER_ERR_SUCCESS)
      {
         return speexErrToOsStatus(speexErr);
      }
      inSamplesProcessed += inSamplesNum;
      outSamplesWritten += outSamplesNum;
   }

   return OS_SUCCESS;
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
