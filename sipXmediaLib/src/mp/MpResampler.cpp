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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpResampler.h"
#include <mp/MpAudioUtils.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpResamplerBase::MpResamplerBase(uint32_t numChannels, 
                                 uint32_t inputRate, uint32_t outputRate, 
                                 int32_t quality)
   : mNumChannels(numChannels)
   , mInputRate(inputRate)
   , mOutputRate(outputRate)
   , mQuality(quality)
{
   // No other initialization needed.
}

MpResamplerBase::MpResamplerBase(uint32_t numChannels, 
                                 uint32_t inputRate, uint32_t outputRate)
   : mNumChannels(numChannels)
   , mInputRate(inputRate)
   , mOutputRate(outputRate)
   , mQuality(0)
{
   // No other initialization needed.
}

MpResamplerBase::~MpResamplerBase()
{
   // No de-initialization needed.
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpResamplerBase::resetStream()
{
   // nothing to reset in this case.
   // Meant for child classes.
   return OS_SUCCESS;
}

OsStatus MpResamplerBase::resample(uint32_t channelIndex,
                                   const MpAudioSample* pInBuf, uint32_t inBufLength, 
                                   uint32_t& inSamplesProcessed, 
                                   MpAudioSample* pOutBuf, uint32_t outBufLength, 
                                   uint32_t& outSamplesWritten )
{
   // state is not maintained between calls for this particular default
   // resampler, so the channelIndex doesn't really matter here, but for
   // consistency, we do check to see if the channel specified is out of 
   // range (i.e. larger than the number of channels that were specified)

   OsStatus ret = OS_FAILED;
   if(channelIndex >= mNumChannels)
   {
      // Specified a channel number that was outside the defined number of channels!
      return OS_INVALID_ARGUMENT;
   }
   else if(mInputRate > mOutputRate)
   {
      // Upsampling is not supported without SPEEX.
      ret = OS_NOT_YET_IMPLEMENTED;
   }
   else if(mInputRate == mOutputRate)
   {
      // Input samples just need to be copied to output samples - 
      // no resampling needed, but this really isn't necessary -- user shouldn't
      // have even called this.
      ret = OS_NOT_YET_IMPLEMENTED;
   }
   else
   {
      // Downsampling.
      uint32_t keptSamples = 0, currentSample = 0;
      uint32_t rkeptSamples = 0, rcurrentSample = 0;

      uint32_t rateGcd = gcd(mInputRate, mOutputRate);
      uint32_t inRateDivGCD = mInputRate / rateGcd;
      uint32_t outRateDivGCD = mOutputRate / rateGcd;

      for(; currentSample < inBufLength && keptSamples < outBufLength; 
          currentSample++, rcurrentSample++)
      {
         if (rkeptSamples * inRateDivGCD <= rcurrentSample * outRateDivGCD)
         {
            pOutBuf[rkeptSamples++, keptSamples++] = pInBuf[currentSample];
            if(rkeptSamples == outRateDivGCD && rcurrentSample == inRateDivGCD)
               rkeptSamples = rcurrentSample = 0;
         }
      }
      inSamplesProcessed = currentSample;
      outSamplesWritten = keptSamples;

      ret = (outSamplesWritten == outBufLength) ? OS_SUCCESS : OS_NO_MEMORY;
   }
   return ret;
}

OsStatus MpResamplerBase::resampleInterleavedStereo(const MpAudioSample* pInBuf, 
                                                    uint32_t inBufLength, 
                                                    uint32_t& inSamplesProcessed, 
                                                    MpAudioSample* pOutBuf, 
                                                    uint32_t outBufLength, 
                                                    uint32_t& outSamplesWritten)
{
   // Default resampler does not do interleaved stereo resampling.
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus MpResamplerBase::setInputRate( const uint32_t inputRate )
{
   mInputRate = inputRate;
   return OS_SUCCESS;
}

OsStatus MpResamplerBase::setOutputRate( const uint32_t outputRate )
{
   mOutputRate = outputRate;
   return OS_SUCCESS;
}

OsStatus MpResamplerBase::setQuality( const int32_t quality )
{
   mQuality = quality;
   return OS_SUCCESS;
}

/* ============================== ACCESSORS =============================== */

uint32_t MpResamplerBase::getInputRate() const
{
   return mInputRate;
}

uint32_t MpResamplerBase::getOutputRate() const
{
   return mOutputRate;

}

int32_t MpResamplerBase::getQuality() const
{
   return mQuality;
}

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */
