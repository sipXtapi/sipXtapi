//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//  
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsIntTypes.h"
#include <os/OsSysLog.h>
#include "mp/MpResampler.h"
#include <mp/MpAudioUtils.h>
#if defined(HAVE_SPEEX) || defined(HAVE_SPEEX_RESAMPLER)
#  include "mp/MpResamplerSpeex.h"
#endif


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpResamplerBase *MpResamplerBase::createResampler(uint32_t numChannels, 
                                                  uint32_t inputRate, 
                                                  uint32_t outputRate, 
                                                  int32_t quality)
{
   return new 
#if defined(HAVE_SPEEX) || defined(HAVE_SPEEX_RESAMPLER)
      MpResamplerSpeex
#else
      MpResamplerBase
#endif
                        (numChannels, inputRate, outputRate, quality);
}

MpResamplerBase::MpResamplerBase(uint32_t numChannels, 
                                 uint32_t inputRate, uint32_t outputRate, 
                                 int32_t quality)
   : mNumChannels(numChannels)
   , mInputRate(inputRate)
   , mOutputRate(outputRate)
   , mQuality(quality >= 0 ? quality : 0)
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
      ret = OS_INVALID_ARGUMENT;
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

OsStatus MpResamplerBase::resampleBufPtr(const MpAudioBufPtr& inBuf,
                                         MpAudioBufPtr& outBuf,
                                         uint32_t inRate,
                                         uint32_t outRate,
                                         UtlString optionalIdStr)
{
   OsStatus stat = OS_SUCCESS;

   if(!inBuf.isValid())
   {
      // If the buffer is not valid, then we can't resample it,
      // but we should still indicate success (nothing went wrong).
      return OS_SUCCESS;
   }

   if(optionalIdStr.isNull())
   {
      optionalIdStr = 
         UtlString(", Unique identifier string: \"") + optionalIdStr + "\"";
   }

   if(inRate != outRate)
   {
      // Get new audio buffer for resampled sound
      MpAudioBufPtr resampledOut = MpMisc.RawAudioPool->getBuffer();
      if (!resampledOut.isValid())
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "MprToOutputDevice::resampleAndReplace - Failed to get audio "
            "buffer from buffer pool for use in resampling.  "
            "Input sample rate %dHz, output sample rate %dHz"
            "%s.\n",
            inRate, outRate, optionalIdStr.data());
         return OS_NO_MEMORY;
      }

      uint32_t nExpectedOutSamples = (uint32_t)
         (((uint64_t)inBuf->getSamplesNumber() * (uint64_t)outRate) / (uint64_t)inRate);
      resampledOut->setSamplesNumber(nExpectedOutSamples);
      if (resampledOut->getSamplesNumber() != nExpectedOutSamples)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "MprToOutputDevice::resampleAndReplace - Failed to set samples "
            "number on new audio buffer retrieved from pool for use in "
            "resampling.  Requested samples number %d, actual samples number %d, "
            "Input sample rate %dHz, output sample rate %dHz"
            "%s.\n", 
            nExpectedOutSamples, resampledOut->getSamplesNumber(), 
            inRate, outRate, optionalIdStr.data());
         return OS_NO_MEMORY;
      }
      // Copy speech status from input buffer.
      resampledOut->setSpeechType(inBuf->getSpeechType());

      uint32_t nDevSamplesProcessed = 0;
      uint32_t nSamplesWritten = 0;
      stat = resample(0, inBuf->getSamplesPtr(), inBuf->getSamplesNumber(), 
         nDevSamplesProcessed, resampledOut->getSamplesWritePtr(), 
         resampledOut->getSamplesNumber(), nSamplesWritten);

      if(stat != OS_SUCCESS)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "MprToOutputDevice::resampleAndReplace - Failed while resampling.  "
            "Input sample rate %d Hz, output sample rate %d Hz %s.\n"
            "resample(0, %p, %d, %d, %p, %d, %d) returned: %d",
            inRate, outRate, optionalIdStr.data(),
            inBuf->getSamplesPtr(), inBuf->getSamplesNumber(), 
            nDevSamplesProcessed, resampledOut->getSamplesWritePtr(), 
            resampledOut->getSamplesNumber(), nSamplesWritten, stat);
         return OS_FAILED;
      }

      // Ok, done resampling and all is good.
      // Just set the out buf pointer passed in to the newly resampled
      // data.  To optimize for speed a bit, we use MpBufPtr's swap() method.
      outBuf.swap(resampledOut);
   }

   return OS_SUCCESS;
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
