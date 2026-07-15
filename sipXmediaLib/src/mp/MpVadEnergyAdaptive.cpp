//
// Copyright (C) 2026 SIP Spectrum, Inc. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <os/OsSysLog.h>
#include <mp/MpVadBase.h>
#include <mp/MpVadEnergyAdaptive.h>

// DEFINES
#define ENERGY_SHIFT          8     ///< Per-sample square is scaled down by this.

// Tunable defaults (overridable via setParam).
#define DEFAULT_MIN_ENERGY    10000 ///< Absolute threshold floor (== MpVadSimple).
#define DEFAULT_SPEECH_MARGIN 4     ///< Speech energy must exceed noise floor by this.
#define DEFAULT_HANGOVER_MS   250   ///< Hold ACTIVE this long after energy drops.

// Noise-floor estimator tuning (fixed).
#define NOISE_LEAK_SHIFT      10    ///< Slow upward leak: floor += floor>>this (+1)/frame.
#define NOISE_FLOOR_MIN       200   ///< Floor never drops below this (avoids zero threshold).

// STATIC VARIABLE INITIALIZATIONS
const char *MpVadEnergyAdaptive::name = "EnergyAdaptive";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpVadEnergyAdaptive::MpVadEnergyAdaptive()
: mSamplesPerSecond(0)
, mMinEnergy(DEFAULT_MIN_ENERGY)
, mSpeechMargin(DEFAULT_SPEECH_MARGIN)
, mHangoverMs(DEFAULT_HANGOVER_MS)
, mHangoverSamples(0)
{
   resetState();
}

MpVadEnergyAdaptive::~MpVadEnergyAdaptive()
{
}

OsStatus MpVadEnergyAdaptive::init(int samplesPerSec)
{
   mSamplesPerSecond = samplesPerSec;
   // Convert the hangover time to a sample count for this stream's rate.
   mHangoverSamples = (mHangoverMs * mSamplesPerSecond) / 1000;
   resetState();
   return OS_SUCCESS;
}

void MpVadEnergyAdaptive::resetState()
{
   mLastFrameSize = 0;
   mLastEnergy = 0;
   mNoiseFloor = 0;
   mHangoverSamplesLeft = 0;
   mHaveNoiseEstimate = FALSE;
}

void MpVadEnergyAdaptive::reset()
{
   // Drop per-stream adaptation but keep configured parameters.
   resetState();
}

/* ============================ MANIPULATORS ============================== */

int MpVadEnergyAdaptive::getEnergy() const
{
   if ((mSamplesPerSecond == 0) || (mLastFrameSize == 0))
      return 0;
   // Mean per-sample weighted energy, rescaled - frame-size independent.
   return (mLastEnergy / mLastFrameSize) << ENERGY_SHIFT;
}

MpSpeechType MpVadEnergyAdaptive::processFrame(uint32_t packetTimeStamp,
                                               const MpAudioSample* pBuf,
                                               unsigned inSamplesNum,
                                               const MpSpeechParams &speechParams,
                                               UtlBoolean calcEnergyOnly)
{
   if (mSamplesPerSecond == 0)
      return MP_SPEECH_UNKNOWN;

   // Compute frame energy (same metric as MpVadSimple).
   int energy = 0;
   for (unsigned i = 0; i < inSamplesNum; i++)
      energy += ((int)pBuf[i] * (int)pBuf[i]) >> ENERGY_SHIFT;

   mLastFrameSize = inSamplesNum;
   mLastEnergy = energy;

   // Use the frame-size-normalized energy for all VAD decisions so the noise
   // floor and thresholds stay in consistent units across codec frame sizes.
   const int frameEnergy = getEnergy();

   if (calcEnergyOnly)
   {
      // Caller only wants the energy; don't run/alter the detector.
      return speechParams.mSpeechType;
   }

   // --- Adaptive noise floor (minimum statistics with a slow upward leak) ---
   // Instant-track downward captures the true noise minimum even between
   // words; slow leak upward recovers if the background level rises and keeps
   // sustained speech from permanently corrupting the estimate.
   if (!mHaveNoiseEstimate)
   {
      mNoiseFloor = frameEnergy;
      mHaveNoiseEstimate = TRUE;
   }
   else if (frameEnergy < mNoiseFloor)
   {
      mNoiseFloor = frameEnergy;
   }
   else
   {
      mNoiseFloor += (mNoiseFloor >> NOISE_LEAK_SHIFT) + 1;
   }
   if (mNoiseFloor < NOISE_FLOOR_MIN)
      mNoiseFloor = NOISE_FLOOR_MIN;

   // Speech threshold: a margin above the noise floor, but never below the
   // absolute minimum. 64-bit math avoids overflow when the floor is large.
   int64_t threshold = (int64_t)mNoiseFloor * mSpeechMargin;
   if (threshold < mMinEnergy)
      threshold = mMinEnergy;

   // --- Decision with hangover ---
   MpSpeechType result;
   if ((int64_t)frameEnergy > threshold)
   {
      // Active speech - (re)arm the hangover hold.
      mHangoverSamplesLeft = mHangoverSamples;
      result = MP_SPEECH_ACTIVE;
   }
   else if (mHangoverSamplesLeft > 0)
   {
      // Below threshold but still within the hangover window.
      mHangoverSamplesLeft -= mLastFrameSize;
      result = MP_SPEECH_ACTIVE;
   }
   else
   {
      result = MP_SPEECH_SILENT;
   }

   // --- TEMP VAD diagnostic (remove when done) ---
   {
      static unsigned sVadDbgCounter = 0;
      if ((sVadDbgCounter++ % 50) == 0)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "VADDBG %s energy=%d noiseFloor=%d threshold=%d margin=%d hangoverLeft=%d result=%s",
            mName.data(), frameEnergy, mNoiseFloor, (int)threshold, mSpeechMargin,
            mHangoverSamplesLeft, (result == MP_SPEECH_ACTIVE) ? "ACTIVE" : "SILENT");
      }
   }
   // --- end diagnostic ---

   return result;
}

OsStatus MpVadEnergyAdaptive::setParam(const char* paramName, void* value)
{
   if (strcmp(paramName, "MinimumEnergy") == 0)
   {
      mMinEnergy = *(int*)value;
      return OS_SUCCESS;
   }
   else if (strcmp(paramName, "SpeechMargin") == 0)
   {
      int margin = *(int*)value;
      if (margin >= 1)
      {
         mSpeechMargin = margin;
         return OS_SUCCESS;
      }
   }
   else if (strcmp(paramName, "HangoverMs") == 0)
   {
      mHangoverMs = *(int*)value;
      if (mHangoverMs < 0)
         mHangoverMs = 0;
      // Recompute the sample count if we already know the rate.
      if (mSamplesPerSecond > 0)
         mHangoverSamples = (mHangoverMs * mSamplesPerSecond) / 1000;
      return OS_SUCCESS;
   }
   return OS_FAILED;
}

/* ============================ FUNCTIONS ================================= */
