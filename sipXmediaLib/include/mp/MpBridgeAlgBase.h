//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpBridgeAlgBase_h_
#define _MpBridgeAlgBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpTypes.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpDspUtils.h"

// DEFINES
// MACROS

// Use Q5.10 (signed) values when fixed point is enabled.
#define MP_BRIDGE_INT_LENGTH          INT16_C(5)
#define MP_BRIDGE_FRAC_LENGTH         INT16_C(10)
#define MPF_BRIDGE_FLOAT(num)         ((MpBridgeGain)MPF_FLOAT((num), MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))
#define MPF_BRIDGE_MAX                ((MpBridgeGain)MPF_MAX(MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))
#define MPF_BRIDGE_STEP               ((MpBridgeGain)MPF_STEP(MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))

#define MP_BRIDGE_GAIN_UNDEFINED      INT16_MAX
#define MP_BRIDGE_GAIN_MAX            MPF_BRIDGE_MAX
#define MP_BRIDGE_GAIN_MIN            MPF_BRIDGE_FLOAT(0.0f)
#define MP_BRIDGE_GAIN_STEP           MPF_BRIDGE_STEP
#define MP_BRIDGE_GAIN_MUTED          MP_BRIDGE_GAIN_MIN
#define MP_BRIDGE_GAIN_PASSTHROUGH    MPF_BRIDGE_FLOAT(1.0f)

#define MAX_AMPLITUDE_ROUND  (1<<(MP_AUDIO_SAMPLE_SIZE-1))

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
#ifdef MP_FIXED_POINT // [
   typedef int32_t MpBridgeAccum;
#else  // MP_FIXED_POINT ][
   typedef float MpBridgeAccum;
#endif // MP_FIXED_POINT ]

#ifdef MP_FIXED_POINT // [
   typedef int16_t MpBridgeGain;
#else  // MP_FIXED_POINT ][
   typedef float MpBridgeGain;
#endif // MP_FIXED_POINT ]

// FORWARD DECLARATIONS

/**
*  @brief Base class for the Bridge mixing algorithms.
*/
class MpBridgeAlgBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor.
   MpBridgeAlgBase(int maxInputs, int maxOutputs, UtlBoolean mixSilence)
   : mMaxInputs(maxInputs)
   , mMaxOutputs(maxOutputs)
   , mMixSilence(mixSilence)
   , mpPrevAmplitudes(new MpAudioSample[mMaxInputs])
   {
      // Save magic value to the array to tell that it hasn't been initialized yet.
      for (int i=0; i<mMaxInputs; i++)
      {
         mpPrevAmplitudes[i]= -1;
      }
   }

     /// Destructor.
   virtual ~MpBridgeAlgBase()
   {
      delete[] mpPrevAmplitudes;
   };

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// @brief Mix together inputs onto outputs according to mpGainMatrix matrix.
   virtual UtlBoolean doMix(MpBufPtr inBufs[], int inBufsSize,
                            MpBufPtr outBufs[], int outBufsSize,
                            int samplesPerFrame) =0;

     /// Set selected gain to the given value.
   virtual void setGainMatrixValue(int column, int row, MpBridgeGain val) =0;

     /// Set selected gain row to the given value.
   virtual void setGainMatrixRow(int row, int numValues, const MpBridgeGain val[]) =0;

     /// Set selected gain column to the given value.
   virtual void setGainMatrixColumn(int column, int numValues, const MpBridgeGain val[]) =0;

     /// Save buffers amplitudes to internal array.
   inline void saveAmplitudes(MpBufPtr inBufs[], int inBufsSize);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Get maximum number of inputs.
   inline
   int maxInputs() const;

     /// Get maximum number of outputs.
   inline
   int maxOutputs() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

   int mMaxInputs;         ///< Number of possible bridge inputs.
   int mMaxOutputs;        ///< Number of possible bridge outputs.
   UtlBoolean mMixSilence; ///< Should Bridge ignore or mix frames marked as silence?
   MpAudioSample *mpPrevAmplitudes; ///< Saved amplitude of the inputs from
                           ///< the previous frame processing interval.

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

int MpBridgeAlgBase::maxInputs() const
{
   return mMaxInputs;
}

int MpBridgeAlgBase::maxOutputs() const
{
   return mMaxOutputs;
}

void MpBridgeAlgBase::saveAmplitudes(MpBufPtr inBufs[], int inBufsSize)
{
   assert(inBufsSize <= mMaxInputs);
   for (int i=0; i<inBufsSize; i++)
   {
      if (inBufs[i].isValid())
      {
         MpAudioBufPtr pAudioBuf = inBufs[i];
         MpAudioSample amplitude = pAudioBuf->getAmplitude();
         mpPrevAmplitudes[i] = amplitude == 0 ? 1 : amplitude;
      }
   }
}

#endif  // _MpBridgeAlgBase_h_
