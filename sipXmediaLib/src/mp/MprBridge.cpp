//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprBridge.h>
#include <mp/MpMisc.h>
#include <mp/MprBridgeSetGainsMsg.h>
#ifdef RTL_ENABLED
#  include <mp/MpFlowGraphBase.h>
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// LOCAL CLASSES DECLARATION
#ifdef TEST_PRINT_CONTRIBUTORS
class MpContributorVector
{
public:
   MpContributorVector(int maxContributors = 0)
   {
      mMaxContributors = maxContributors;
      if (mMaxContributors > 0)
      {
         mpContributorVector = new int[mMaxContributors];
         zero();
      }
      else
      {
         mMaxContributors = 0;
         mpContributorVector = NULL;
      }
   };

   ~MpContributorVector()
   {
      if (mpContributorVector)
      {
         delete mpContributorVector;
         mpContributorVector = NULL;
      }
   };

   void zero()
   {
      if (mpContributorVector && mMaxContributors > 0)
      {
         memset(mpContributorVector, 0, sizeof(int) * mMaxContributors);
      }
   };

   void set(int contributorIndex, int mixWeight)
   {
      if (mpContributorVector && 
          contributorIndex >= 0 && 
          contributorIndex < mMaxContributors)
      {
         mpContributorVector[contributorIndex] = mixWeight;
      }
   };

   int get(int contributorIndex) const
   {
      int retValue;
      if (mpContributorVector && 
          contributorIndex >= 0 && 
          contributorIndex < mMaxContributors)
      {
         retValue = mpContributorVector[contributorIndex];
      }
      else
      {
         retValue = 0;
      }
      return retValue;
   };

   UtlBoolean differs(const MpContributorVector& otherVector) const
   {
      assert(otherVector.mMaxContributors == mMaxContributors);
      UtlBoolean isDiff = FALSE;
      for (int i = 0; i < mMaxContributors; i++)
      {
         if (mpContributorVector[i] != otherVector.mpContributorVector[i])
         {
            isDiff = TRUE;
            break;
         }
      }
      return isDiff;
   }

   UtlBoolean operator==(const MpContributorVector& otherVector) const
   {
      return !differs(otherVector);
   }

   UtlBoolean operator!=(const MpContributorVector& otherVector) const
   {
      return differs(otherVector);
   }

   MpContributorVector& operator=(const MpContributorVector& source)
   {
      assert(source.mMaxContributors == mMaxContributors);
      memcpy(mpContributorVector, 
             source.mpContributorVector,
             sizeof(int) * mMaxContributors);
      return *this;
   }

private:
   int mMaxContributors;
   int* mpContributorVector;

   MpContributorVector(const MpContributorVector&);

};
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBridge::MprBridge(const UtlString& rName,
                     int maxInOutputs,
                     int samplesPerFrame, 
                     int samplesPerSec)
:  MpAudioResource(rName, 
                   1, maxInOutputs, 
                   1, maxInOutputs, 
                   samplesPerFrame, 
                   samplesPerSec)
#ifdef TEST_PRINT_CONTRIBUTORS  // [
, mpMixContributors(NULL)
, mpLastOutputContributors(NULL)
#endif // TEST_PRINT_CONTRIBUTORS ]
, mpGainMatrix(NULL)
, mpMixAccumulator(NULL)
, mpGainedInputs(NULL)
, mpIsInputActive(NULL)
{
   handleDisable();

#ifdef TEST_PRINT_CONTRIBUTORS
   mpMixContributors = new MpContributorVector(maxInOutputs);
   mpLastOutputContributors = new MpContributorVector*[maxInOutputs];
   for (int i = 0; i < maxInOutputs; i++)
   {
      mpLastOutputContributors[i] = new MpContributorVector(maxInOutputs);
   }
#endif

   // Create temporary storage for mixing data.
   mpMixAccumulator = new MpBridgeAccum[getSamplesPerFrame()];
   assert(mpMixAccumulator != NULL);

   // Allocate mix matrix.
   mpGainMatrix = new MpBridgeGain[maxInputs()*maxOutputs()];
   assert(mpGainMatrix != NULL);
   
   // Initially mute all inputs.
/*   for (int j=maxInputs()*maxOutputs()-1; j>=0; j--)
   {
      mpGainMatrix[j] = MP_BRIDGE_GAIN_MUTED;
   }
*/
   // Initially set matrix to inversed unity matrix, with zeros along
   // main diagonal.
   MpBridgeGain *pGain = mpGainMatrix;
   *pGain = MP_BRIDGE_GAIN_MUTED;
   pGain++;
   for (int row=0; row<maxOutputs()-1; row++)
   {
      for (int i=0; i<maxInputs(); i++)
      {
         *pGain = MP_BRIDGE_GAIN_PASSTHROUGH;
         pGain++;
      }
      *pGain = MP_BRIDGE_GAIN_MUTED;
      pGain++;
   }
}

// Destructor
MprBridge::~MprBridge()
{
   delete[] mpGainMatrix;
   delete[] mpMixAccumulator;
   delete[] mpGainedInputs;
   delete[] mpIsInputActive;

#ifdef TEST_PRINT_CONTRIBUTORS
   delete mpMixContributors;
   mpMixContributors = NULL;
   for (int i = 0; i < mMaxOutputs; i++)
   {
      delete mpLastOutputContributors[i];
      mpLastOutputContributors[i] = NULL;
   }
   delete[] mpLastOutputContributors;
   mpLastOutputContributors = NULL;
#endif

}

/* ============================ MANIPULATORS ============================== */

OsStatus MprBridge::setMixWeightsForOutput(int bridgeOutputPort,
                                           int numWeights,
                                           const MpBridgeGain gains[])
{
   MpBridgeGain *pGainsCopy = new MpBridgeGain[numWeights];
   memcpy(pGainsCopy, gains, numWeights*sizeof(MpBridgeGain));

   MpFlowGraphMsg msg(SET_WEIGHTS_FOR_OUTPUT, this,
                      (void*)pGainsCopy, NULL,
                      bridgeOutputPort, numWeights);
   return postMessage(msg);
}

OsStatus MprBridge::setMixWeightsForOutput(const UtlString& namedResource, 
                                           OsMsgQ& fgQ, 
                                           int bridgeOutputPort,
                                           int numWeights,
                                           const MpBridgeGain gains[])
{
   MprBridgeSetGainsMsg msg(namedResource, bridgeOutputPort, numWeights,
                            gains,
                            MprBridgeSetGainsMsg::GAINS_ROW);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprBridge::setMixWeightsForInput(int bridgeInputPort,
                                          int numWeights,
                                          const MpBridgeGain gains[])
{
   MpBridgeGain *pGainsCopy = new MpBridgeGain[numWeights];
   memcpy(pGainsCopy, gains, numWeights*sizeof(MpBridgeGain));

   MpFlowGraphMsg msg(SET_WEIGHTS_FOR_OUTPUT, this,
                      (void*)pGainsCopy, NULL,
                      bridgeInputPort, numWeights);
   return postMessage(msg);
}

OsStatus MprBridge::setMixWeightsForInput(const UtlString& namedResource, 
                                          OsMsgQ& fgQ, 
                                          int bridgeInputPort,
                                          int numWeights,
                                          const MpBridgeGain gains[])
{
   MprBridgeSetGainsMsg msg(namedResource, bridgeInputPort, numWeights,
                            gains,
                            MprBridgeSetGainsMsg::GAINS_COLUMN);
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprBridge::doMix(MpBufPtr inBufs[], int inBufsSize,
                            MpBufPtr outBufs[], int outBufsSize,
                            int samplesPerFrame)
{
   assert(inBufsSize <= maxInputs());
   assert(outBufsSize <= maxOutputs());

   // Loop over all outputs
   for (int outputNum=0; outputNum<outBufsSize; outputNum++)
   {
      MpBridgeGain *pInputGains = &mpGainMatrix[outputNum*maxInputs()];

      // Initialize accumulator
      for (int i=0; i<samplesPerFrame; i++)
      {
         mpMixAccumulator[i] = MPF_BRIDGE_FLOAT(0.0f);
      }

      // Get buffer for output data.
      MpAudioBufPtr pOutBuf = MpMisc.RawAudioPool->getBuffer();
      assert(pOutBuf.isValid());
      pOutBuf->setSamplesNumber(samplesPerFrame);
      pOutBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);

      // Mix input data to accumulator
      for (int inputNum=0; inputNum<inBufsSize; inputNum++)
      {
         if (inBufs[inputNum].isValid() &&
             pInputGains[inputNum] != MP_BRIDGE_GAIN_MUTED)
         {
            MpAudioBufPtr pFrame = inBufs[inputNum];
            assert(pFrame->getSamplesNumber() == samplesPerFrame);
            if (pInputGains[inputNum] == MP_BRIDGE_GAIN_PASSTHROUGH)
            {
               MpDspUtils::add_IS(pFrame->getSamplesPtr(), mpMixAccumulator,
                                  samplesPerFrame, MP_BRIDGE_FRAC_LENGTH);
            }
            else
            {
               MpDspUtils::addMul_I(pFrame->getSamplesPtr(),
                                    pInputGains[inputNum],
                                    mpMixAccumulator,
                                    samplesPerFrame);
            }
         }
      }

      // Move data from accumulator to output.
      MpDspUtils::convert_S(mpMixAccumulator, pOutBuf->getSamplesWritePtr(),
                            samplesPerFrame, -MP_BRIDGE_FRAC_LENGTH);
      outBufs[outputNum].swap(pOutBuf);
   }

   return TRUE;

#if 0
   // == High level algorithm description ==
   //
   // This description assume that we have N inputs and N outputs for simplicity.
   //
   // Phase 0: Apply gain to inputs.
   //
   // Apply gain to all inputs and by the way convert them to 32-bit integers
   // to save precision.
   //
   // Phase 1: Collect sum of all inputs.
   //
   // Sum all inputs to accumulator array (32-bit). During this we'll get
   // data for one output ready - we should copy data from accumulator
   // to last output before last input will be added to accumulator.
   // This will take (N-1) addition operations per sample.
   //
   // Phase 2: Calculate output signal.
   //
   // For all outputs (except one, filled on phase 1) subtract corresponding
   // input signal from accumulator. Save calculated output data to provided
   // buffers after saturation.
   // This will take (N-1) subtraction operations per sample.
   //
   // Note: For outputs that do not have corresponding active input accumulator
   //       data will be used after saturation.
   //
   //  Note: Input is considered active if:
   //        - buffer pointer is not NULL;
   //        - buffer contain not silent data;
   //        - it does not muted by gain/attenuation.
   //
   int i;                  ///< Loop counter.
   int32_t *pGainedData;   ///< Iterator for mpGainedInputs 2D array.
   int lastNotMutedOutput; ///< Index of last output with not muted gain/attenuation.

   // Phase 0: Apply gain to inputs.

   // Get pointer to the end of gained data. This pointer will be decremented
   // in the loop to always point to processed input data.
   pGainedData = &mpGainedInputs[inBufsSize*samplesPerFrame];
   for (i=inBufsSize; i>=0; i--)
   {
      MpAudioBufPtr tempBuf;
      tempBuf.swap(inBufs[i]);

      // Adjust pointer to gained input data.
      pGainedData -= samplesPerFrame;

      if (  tempBuf.isValid()
         && mpInputGains[i] != MP_GAIN_MUTED
         && mpInputAttenuations[i] != MP_ATTENUATION_MUTED
         && tempBuf->isActiveAudio())
      {
         // Mark input as active.
         mpIsInputActive[i] = true;

         // Apply gain.
         assert(tempBuf->getSamplesNumber() == samplesPerFrame);
         MpDspUtils::mulConst_16s32s(tempBuf->getSamples(),
                                     mpInputGains[i],
                                     pGainedData,
                                     samplesPerFrame);
         MpDspUtils::divConst_32s_I(mpInputAttenuations[i],
                                    pGainedData,
                                    samplesPerFrame);
      }
      else
      {
         // Mark this input as silent. Do not touch gained data for
         // this input, as it will not be used.
         mpIsInputActive[i] = false;
      }
   }

   // Note, that inBufs[] do not contain data from this point. Input data
   // (already gained) could be found in mpGainedInputs buffer.

   // Phase 1: Collect sum of all inputs.

   // Phase 1.1: Search for first (from the end) not muted output. By the way
   // sum data from first (from the end) inputs.
   for (i=inBufsSize; i>=0; i--)
   {
   }

   // Phase 2: Calculate output signal.

   return TRUE;

#endif // 0 ]

#if 0
    int inputsValid;      // Number of ports with available data
    int inputsActive;     // Number of ports with active speech
    int lastValid;        // Port number of last port with available data
    int lastActive;       // Port number of last port with active speech

    // Count contributing inputs
    inputsValid = 0;
    inputsActive = 0;
    lastValid = -1;
    lastActive = -1;
    for (int inIdx=0; inIdx < inBufsSize; inIdx++)
    {
          if (inBufs[inIdx].isValid())
          {
             inputsValid++;
             lastValid = inIdx;
             if (inBufs[inIdx]->isActiveAudio())
             {
                inputsActive++;
                lastActive = inIdx;
             }
          }
    }

    // If there is only one input we could skip all processing and copy it
    // to output. Special case for single input is needed because other case
    // make decision make its choice depending on voice activity, which lead
    // to unwanted silence insertion. Someday this function will get smarter...
    if (inputsValid == 1)
    {
       out = inBufs[lastValid];
    }
    else if (inputsActive == 1)
    {
       // If only one active input then just return it
       out = inBufs[lastActive];
    }
    else if (inputsActive > 1)
    {
        // Compute a logarithmic scale factor to renormalize (approximately)
        int scale = 0;
        while (inputsActive > 1)
        {
            scale++;
            inputsActive = inputsActive >> 1;
        }

        // Get new buffer for mixed output
        out = MpMisc.RawAudioPool->getBuffer();
        if (!out.isValid())
            return FALSE;
        out->setSamplesNumber(samplesPerFrame);

        // Fill output buffer with silence
        MpAudioSample* outstart = out->getSamplesWritePtr();
        memset((char *) outstart, 0, samplesPerFrame * sizeof(MpAudioSample));

        // Mix them all
        for (int inIdx=0; inIdx < inBufsSize; inIdx++)
        {
            if (isPortActive(inIdx))
            {
                MpAudioSample* output = outstart;
                // Mix only non-silent audio
                if (inBufs[inIdx].isValid() && inBufs[inIdx]->isActiveAudio())
                { 
                    const MpAudioSample* input = inBufs[inIdx]->getSamplesPtr();
                    int n = min(inBufs[inIdx]->getSamplesNumber(), samplesPerFrame);
                    for (int i=0; i<n; i++)
                        *output++ += (*input++) >> scale;
                }
            }
        }
    }

    return TRUE;
#endif
}

UtlBoolean MprBridge::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) {
   case SET_WEIGHTS_FOR_INPUT:
      {
         MpBridgeGain *pGains = (MpBridgeGain*)rMsg.getPtr1();
         msgHandled = handleSetMixWeightsForInput(rMsg.getInt1(), rMsg.getInt2(),
                                                  pGains);
         delete[] pGains;
      }
      break;

   case SET_WEIGHTS_FOR_OUTPUT:
      {
         MpBridgeGain *pGains = (MpBridgeGain*)rMsg.getPtr1();
         msgHandled = handleSetMixWeightsForOutput(rMsg.getInt1(), rMsg.getInt2(),
                                                   pGains);
         delete[] pGains;
      }
      break;

   default:
      msgHandled = MpAudioResource::handleMessage(rMsg);
      break;
   }
   return msgHandled;
}

UtlBoolean MprBridge::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   MprBridgeSetGainsMsg* pBridgeMsg = NULL;
   switch (rMsg.getMsg()) 
   {
   case MpResourceMsg::MPRM_BRIDGE_SET_GAINS:
      pBridgeMsg = (MprBridgeSetGainsMsg*)&rMsg;

      if (pBridgeMsg->getType() == MprBridgeSetGainsMsg::GAINS_ROW)
      {
         // Set row in mix matrix
         handleSetMixWeightsForOutput(pBridgeMsg->getPort(),
                                      pBridgeMsg->getGainsNum(),
                                      pBridgeMsg->getGains());
      }
      else if (pBridgeMsg->getType() == MprBridgeSetGainsMsg::GAINS_COLUMN)
      {
         // Set column in mix matrix
         handleSetMixWeightsForInput(pBridgeMsg->getPort(),
                                     pBridgeMsg->getGainsNum(),
                                     pBridgeMsg->getGains());
      }
      else
      {
         // Unknown type
         assert(false);
      }

      msgHandled = TRUE;
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

UtlBoolean MprBridge::handleSetMixWeightsForOutput(int bridgeOutputPort,
                                                   int numWeights,
                                                   const MpBridgeGain gain[])
{
   // New gains vector must feet into matrix
   assert(numWeights <= maxInputs());
   if (numWeights > maxInputs())
   {
      return FALSE;
   }

   // Copy gain data to mix matrix row.
   MpBridgeGain *pCurGain = &mpGainMatrix[bridgeOutputPort*maxInputs()];
   for (int i=0; i<numWeights; i++)
   {
      if (gain[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         *pCurGain = gain[i];
      }
      pCurGain++;
   }

   return TRUE;
}

UtlBoolean MprBridge::handleSetMixWeightsForInput(int bridgeInputPort,
                                                  int numWeights,
                                                  const MpBridgeGain gain[])
{
   // New gains vector must feet into matrix
   assert(numWeights <= maxOutputs());
   if (numWeights > maxOutputs())
   {
      return FALSE;
   }

   // Copy gain data to mix matrix column.
   MpBridgeGain *pCurGain = &mpGainMatrix[bridgeInputPort];
   for (int i=0; i<numWeights; i++)
   {
      if (gain[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         *pCurGain = gain[i];
      }
      pCurGain += maxInputs();
   }
   
   return TRUE;
}

UtlBoolean MprBridge::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpAudioBufPtr in;
   UtlBoolean ret = FALSE;

#ifdef RTL_AUDIO_ENABLED
   int frameIndex = mpFlowGraph ? mpFlowGraph->numFramesProcessed() : 0;
   RTL_AUDIO_BUFFER("bridge_input_0", samplesPerSecond, ((MpAudioBufPtr)inBufs[0]), frameIndex);
   RTL_AUDIO_BUFFER("bridge_input_1", samplesPerSecond, ((MpAudioBufPtr)inBufs[1]), frameIndex);
   RTL_AUDIO_BUFFER("bridge_input_2", samplesPerSecond, ((MpAudioBufPtr)inBufs[2]), frameIndex);
#endif

   // We're disabled or have nothing to process.
   if ( outBufsSize == 0 || inBufsSize == 0 || !isEnabled )
   {
      return TRUE;
   }

   // We want correct in/out pairs
   if (inBufsSize != outBufsSize)
   {
      return FALSE;
   }

   ret = doMix(inBufs, inBufsSize, outBufs, outBufsSize, samplesPerFrame);

#ifdef TEST_PRINT_CONTRIBUTORS
   for (int outIdx=0; outIdx < outBufsSize; outIdx++) {
      mpMixContributors->zero();

      // Keep track of the sources mixed for this output
      if (*mpLastOutputContributors[outIdx] != *mpMixContributors)
      {
         int contribIndex;
         printf("Bridge output: %d vector change: %d", 
                outIdx, mpMixContributors->get(0));
         for (contribIndex = 1; contribIndex < inBufsSize; contribIndex++)
         {
            printf(", %d", mpMixContributors->get(contribIndex));
         }
         printf("\n");

         *mpLastOutputContributors[outIdx] = *mpMixContributors;
      }
   }
#endif

#ifdef RTL_AUDIO_ENABLED
   RTL_AUDIO_BUFFER("bridge_output_0", samplesPerSecond, ((MpAudioBufPtr)outBufs[0]), frameIndex);
#endif

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

