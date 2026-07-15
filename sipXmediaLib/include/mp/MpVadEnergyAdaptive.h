//
// Copyright (C) 2026 SIPez LLC. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpVadEnergyAdaptive_h_
#define _MpVadEnergyAdaptive_h_

#include <mp/MpVadBase.h>

/**
*  Energy VAD with an adaptive noise floor and hangover.
*
*  Improves on MpVadSimple (a bare fixed-energy threshold) in two ways:
*
*   - Adaptive noise floor: instead of comparing frame energy to a fixed
*     threshold, it tracks the background-noise energy (minimum-statistics
*     estimate with a slow upward leak) and declares speech only when the
*     frame energy exceeds the noise floor by a configurable margin. This
*     lets a noisy line's steady background read as SILENT (so the bridge AGC
*     gate engages on it) while still detecting speech on a quiet line.
*
*   - Hangover: once speech is detected, the ACTIVE decision is held for a
*     configurable time after the energy drops back below threshold. This
*     bridges short intra-word pauses, stops soft word-endings from being
*     clipped, and gives the AGC stable speech/silence segments instead of
*     per-frame chatter.
*
*  Returns only MP_SPEECH_ACTIVE or MP_SPEECH_SILENT.
*
*  Parameters (settable via setParam, value is an int*):
*   - "MinimumEnergy" : absolute energy floor for the threshold (compatible
*                       with MpVadSimple); the threshold is never lower than
*                       this regardless of the adaptive estimate.
*   - "SpeechMargin"  : how many times above the noise floor a frame must be
*                       to count as speech (energy ratio).
*   - "HangoverMs"    : hangover hold time in milliseconds.
*
*  @nosubgrouping
*/
class MpVadEnergyAdaptive : public MpVadBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const char *name; ///< Name of this VAD algorithm for MpVadBase::createVad().

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpVadEnergyAdaptive();

     /// @copydoc MpVadBase::init()
   OsStatus init(int samplesPerSec);

     /// Destructor
   ~MpVadEnergyAdaptive();

     /// @copydoc MpVadBase::reset()
   void reset();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpVadBase::processFrame()
   MpSpeechType processFrame(uint32_t packetTimeStamp,
                             const MpAudioSample* pBuf,
                             unsigned inSamplesNum,
                             const MpSpeechParams &speechParams,
                             UtlBoolean calcEnergyOnly = FALSE);

     /// @copydoc MpVadBase::setParam()
   OsStatus setParam(const char* paramName, void* value);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
     /// @copydoc MpVadBase::getEnergy()
   int getEnergy() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Reset just the per-stream adaptive state (keeps configured params).
   void resetState();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Configuration (persist across reset()).
   int mSamplesPerSecond; ///< Stream sample rate.
   int mMinEnergy;        ///< Absolute lower bound for the speech threshold.
   int mSpeechMargin;     ///< Frame must exceed noise floor by this factor.
   int mHangoverMs;       ///< Hangover hold time, milliseconds.
   int mHangoverSamples;  ///< Hangover hold time converted to samples.

   // Per-stream adaptive state (cleared on reset()).
   int  mLastFrameSize;       ///< Samples in the last processed frame.
   int  mLastEnergy;          ///< Accumulated weighted energy of last frame.
   int  mNoiseFloor;          ///< Adaptive background-noise energy estimate.
   int  mHangoverSamplesLeft; ///< Remaining hangover, samples.
   UtlBoolean mHaveNoiseEstimate; ///< Has mNoiseFloor been seeded yet?
};

#endif // _MpVadEnergyAdaptive_h_
