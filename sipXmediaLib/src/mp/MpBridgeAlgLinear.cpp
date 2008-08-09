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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBridgeAlgLinear.h"
#include "mp/MpMisc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpBridgeAlgLinear::MpBridgeAlgLinear(int inputs, int outputs,
                                     UtlBoolean mixSilence,
                                     int samplesPerFrame)
: MpBridgeAlgBase(inputs, outputs, mixSilence)
, mActiveInputsListSize(0)
, mpActiveInputsList(NULL)
, mMixActionsStackLength(0)
, mMixActionsStackTop(0)
, mpMixActionsStack(NULL)
, mMixDataStackStep(0)
, mMixDataStackLength(0)
, mpMixDataStack(0)
, mMixDataInfoStackStep(0)
, mMixDataInfoStackLength(0)
, mpMixDataInfoStackTop(NULL)
, mpMixDataInfoStack(NULL)
, mMixDataInfoProcessedStackLength(0)
, mMixDataInfoProcessedStackTop(0)
, mpMixDataInfoProcessedStack(NULL)
{
   // Initialize all data structures, used for optimization:

   // Initialize extended inputs, taking into account that we have "standard"
   // (inverse unity) matrix.
   mExtendedInputs.init_simple(maxInputs(), maxOutputs());
   // Allocate array for list of active inputs.
   mActiveInputsListSize = maxInputs()*maxOutputs();
   mpActiveInputsList = new int[mActiveInputsListSize];
   // Allocate array for mix action stack.
   mMixActionsStackLength = maxInputs()*maxOutputs();
   mpMixActionsStack = new MixAction[mMixActionsStackLength];
   // Allocate array for mix temporary data.
   mMixDataStackStep = samplesPerFrame;
   mMixDataStackLength = maxInputs()*maxOutputs()*mMixDataStackStep;
   mpMixDataStack = new MpBridgeAccum[mMixDataStackLength];
   mpMixDataSpeechType = new MpSpeechType[maxInputs()*maxOutputs()];
   // Allocate array for mix temporary data info.
   mMixDataInfoStackStep = maxInputs()*maxOutputs();
   mMixDataInfoStackLength = maxInputs()*maxOutputs()*mMixDataInfoStackStep;
   mpMixDataInfoStack = new int[mMixDataInfoStackLength];
   mpMixDataInfoStackTop = mpMixDataInfoStack;
   mMixDataInfoProcessedStackLength = maxInputs()*maxOutputs();
   mpMixDataInfoProcessedStack = new int[mMixDataInfoProcessedStackLength];
}

MpBridgeAlgLinear::~MpBridgeAlgLinear()
{
   delete[] mpActiveInputsList;
   delete[] mpMixActionsStack;
   delete[] mpMixDataStack;
   delete[] mpMixDataSpeechType;
   delete[] mpMixDataInfoStack;
   delete[] mpMixDataInfoProcessedStack;
}

/* ============================= MANIPULATORS ============================= */

UtlBoolean MpBridgeAlgLinear::doMix(MpBufPtr inBufs[], int inBufsSize,
                                    MpBufPtr outBufs[], int outBufsSize,
                                    int samplesPerFrame)
{
   int numActiveInputs = 0;
   const int numExtendedInputs = mExtendedInputs.getExtendedInputsNum();

   assert(inBufsSize == maxInputs());
   assert(outBufsSize == maxOutputs());

   //
   //  Build list of active inputs (stored in mpActiveInputsList).
   //
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
   printf("Contributors (inversed order): ");
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
   int extInput;
   for ( extInput=numExtendedInputs-1; extInput>=0; extInput-- )
   {
      if (mExtendedInputs.isNotMuted(extInput))
      {
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
         printf("(%d)NM",extInput);
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
         int origInput = mExtendedInputs.getOrigin(extInput);
         if (inBufs[origInput].isValid())
         {
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
            printf("V");
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
            MpAudioBufPtr pAudio = inBufs[origInput];
            if ((mMixSilence || isActiveAudio(pAudio->getSpeechType())) &&
                pAudio->getSpeechType() != MP_SPEECH_MUTED)
            {
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
               printf("A");
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
               mpActiveInputsList[numActiveInputs] = extInput;
               numActiveInputs++;
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
               printf("+");
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
            }
         }
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
         printf(" ");
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]
      }
   }
#ifdef TEST_PRINT_MIXING_CONTRIBUTORS // [
   printf("\n");
   printf("Total active inputs: %d\n", numActiveInputs);
#endif // TEST_PRINT_MIXING_CONTRIBUTORS ]

   //
   //  Build mix actions stack.
   //

   // Prepare 0: Initialize temporary data structures.
   initMixActions();
   initMixDataInfoStack(numActiveInputs);
   // Prepare 1: Copy data from inputs to temporary mix data storage beginning.
   //   NOTE: Inputs data must be always first. This is used later for
   //         optimization!
   for (int input=0; input<numActiveInputs; input++)
   {
      int tmpInput=0;
      for (; tmpInput<input; tmpInput++)
      {
         mpMixDataInfoStackTop[tmpInput] = 0;
      }
      mpMixDataInfoStackTop[tmpInput] = 1;
      tmpInput++;
      for (; tmpInput<numActiveInputs; tmpInput++)
      {
         mpMixDataInfoStackTop[tmpInput] = 0;
      }

      // Commands to copy data from inputs will be queued after main loop.
      // ...

      // Advance stack pointers.
      mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = INT_MAX;
      PRINT_TOP_MIX_DATA();
      mMixDataInfoProcessedStackTop++;
      mpMixDataInfoStackTop += mMixDataInfoStackStep;
      assert(mpMixDataInfoStackTop<mpMixDataInfoStack+mMixDataInfoStackLength);
   }
   // Prepare 2: Append outputs data to temporary mix data storage.
   for (int output=0; output<maxOutputs(); output++)
   {
      int dataBitCounter = 0;

      // Generate data set for this output.
      for (int input=0; input<mMixDataInfoStackStep; input++)
      {
         const int origInput = mExtendedInputs.getOrigin(mpActiveInputsList[input]);
         mpMixDataInfoStackTop[input] =
            (mExtendedInputs.getExtendedInput(origInput, output) == mpActiveInputsList[input]) ? 1 : 0;
         dataBitCounter += mpMixDataInfoStackTop[input];
      }

      // Skip output if it is muted.
      if (dataBitCounter > 0)
      {
         // Check for uniqueness.
         int twinData;
         if (searchData(mpMixDataInfoStackTop, twinData))
         {
            // Stack already contain this data.

            // Request copy to output.
            pushMixActionCopyToOutput(twinData, output);
         }
         else
         {
            // Data is unique.

            // Request copy to output.
            pushMixActionCopyToOutput(mMixDataInfoProcessedStackTop, output);

            // Push output mask to mix data array.
            mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
            mMixDataInfoProcessedStackTop++;
            mpMixDataInfoStackTop += mMixDataInfoStackStep;
            assert(mpMixDataInfoStackTop<mpMixDataInfoStack+mMixDataInfoStackLength);
         }
      }
   }

   // Main loop: walk through data info stack, looking for the way to get
   //            requested data. New data is added on top of data info stack
   //            if needed.
   int *pDataInfoStackPtr = mpMixDataInfoStack;
   int  dataInfoStackPos  = 0;
   while (pDataInfoStackPtr<mpMixDataInfoStackTop)
   {
      // Do processing only if not processed already
      if (mpMixDataInfoProcessedStack[dataInfoStackPos]<0)
      {
         int *pClosestDataInfoStackPtr;
         int  closestDataInfoStackPos;
         if (searchClosestUnprocessedData(pDataInfoStackPtr,
                                          pDataInfoStackPtr, dataInfoStackPos,
                                          pClosestDataInfoStackPtr, closestDataInfoStackPos))
         {
            // Pair for this data found.

            int *pCommonDataPtr = mpMixDataInfoStackTop;
            int *pFirstDataDiffPtr  = mpMixDataInfoStackTop+mMixDataInfoStackStep;
            int *pSecondDataDiffPtr = mpMixDataInfoStackTop+2*mMixDataInfoStackStep;
            mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
            mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop+1] = -1;
            mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop+2] = -1;
            int  commonDataBitCounter = 0;
            int  firstDataDiffBitCounter  = 0;
            int  secondDataDiffBitCounter = 0;

            assert(pSecondDataDiffPtr+mMixDataInfoStackStep<mpMixDataInfoStack+mMixDataInfoStackLength);

            // Calculate common part, first only part and second only part.
            for (int i=0; i<mMixDataInfoStackStep; i++)
            {
               pCommonDataPtr[i]     =  pDataInfoStackPtr[i] &  pClosestDataInfoStackPtr[i];
               pFirstDataDiffPtr[i]  =  pDataInfoStackPtr[i] & !pClosestDataInfoStackPtr[i];
               pSecondDataDiffPtr[i] = !pDataInfoStackPtr[i] &  pClosestDataInfoStackPtr[i];
               commonDataBitCounter     += pCommonDataPtr[i];
               firstDataDiffBitCounter  += pFirstDataDiffPtr[i];
               secondDataDiffBitCounter += pSecondDataDiffPtr[i];
            }
            // Data sets must have common part!
            assert(commonDataBitCounter > 0);
            // Data sets must be different!
            assert(firstDataDiffBitCounter > 0 || secondDataDiffBitCounter > 0);

            int commonDataPos;
            int firstDataDiffPos;
            int secondDataDiffPos;

            // Look is this data is already in our stack.
            // Note, that data we're searching for is always in array,
            // because we've stored it temporarily there, so search function
            // always succeed.
            // Note also, that if firstDataBitCounter or secondDataBitCounter
            // is zero, then common part is equal to first or second data.
            if (firstDataDiffBitCounter > 0)
            {
               searchExistingUnprocessedData(pFirstDataDiffPtr, firstDataDiffPos,
                                             numActiveInputs);
               if (secondDataDiffBitCounter > 0)
               {
                  searchExistingUnprocessedData(pCommonDataPtr, commonDataPos,
                                                numActiveInputs);
               }
            }
            if (secondDataDiffBitCounter > 0)
            {
               searchExistingUnprocessedData(pSecondDataDiffPtr, secondDataDiffPos,
                                             numActiveInputs);
               if (firstDataDiffBitCounter == 0)
               {
                  // Common data is equal to first data.
                  commonDataPos = dataInfoStackPos;
               }
            }
            else
            {
               // Common data is equal to second data.
               commonDataPos = closestDataInfoStackPos;
            }

            // Remove unused data from our stack.
            if ((firstDataDiffBitCounter == 0) ||
                (secondDataDiffBitCounter == 0) ||
                (commonDataPos < mMixDataInfoProcessedStackTop))
            {
               // Common data is not needed, so move other data to its place.
               if ((firstDataDiffBitCounter > 0) &&
                   (firstDataDiffPos > mMixDataInfoProcessedStackTop))
               {
                  // First data is needed. Move it and advance stack pointer.
                  if (commonDataPos >= firstDataDiffPos)
                  {
                     commonDataPos--;
                  }
                  firstDataDiffPos--;
                  memcpy(pCommonDataPtr, pFirstDataDiffPtr,
                         mMixDataInfoStackStep*sizeof(int));
                  mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                  PRINT_TOP_MIX_DATA();
                  mpMixDataInfoStackTop += mMixDataInfoStackStep;
                  mMixDataInfoProcessedStackTop++;

                  if ((secondDataDiffBitCounter > 0) &&
                      (secondDataDiffPos >= mMixDataInfoProcessedStackTop))
                  {
                     // Second data is needed. Move it and advance stack pointer.
                     if (commonDataPos == secondDataDiffPos)
                     {
                        commonDataPos--;
                     }
                     secondDataDiffPos--;
                     memcpy(pFirstDataDiffPtr, pSecondDataDiffPtr,
                            mMixDataInfoStackStep*sizeof(int));
                     mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                     PRINT_TOP_MIX_DATA();
                     mpMixDataInfoStackTop += mMixDataInfoStackStep;
                     mMixDataInfoProcessedStackTop++;
                  }
               }
               else
               {
                  if ((secondDataDiffBitCounter > 0) &&
                      (secondDataDiffPos >= mMixDataInfoProcessedStackTop))
                  {
                     // Second data is needed. Move it and advance stack pointer.
                     if (commonDataPos == secondDataDiffPos)
                     {
                        commonDataPos -= 2;
                     }
                     secondDataDiffPos -= 2;
                     memcpy(pCommonDataPtr, pSecondDataDiffPtr,
                            mMixDataInfoStackStep*sizeof(int));
                     mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                     PRINT_TOP_MIX_DATA();
                     mpMixDataInfoStackTop += mMixDataInfoStackStep;
                     mMixDataInfoProcessedStackTop++;
                  }
               }
            }
            else
            {
               // Common data is new. Advance stack pointer.
               mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
               PRINT_TOP_MIX_DATA();
               mpMixDataInfoStackTop += mMixDataInfoStackStep;
               mMixDataInfoProcessedStackTop++;

               if ((firstDataDiffBitCounter > 0) &&
                   (firstDataDiffPos >= mMixDataInfoProcessedStackTop))
               {
                  // First data is needed. Advance stack pointer.
                  mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                  PRINT_TOP_MIX_DATA();
                  mpMixDataInfoStackTop += mMixDataInfoStackStep;
                  mMixDataInfoProcessedStackTop++;

                  if ((secondDataDiffBitCounter > 0) &&
                      (secondDataDiffPos >= mMixDataInfoProcessedStackTop))
                  {
                     // Second data is needed. Advance stack pointer.
                     mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                     PRINT_TOP_MIX_DATA();
                     mpMixDataInfoStackTop += mMixDataInfoStackStep;
                     mMixDataInfoProcessedStackTop++;
                  }
               }
               else
               {
                  if ((secondDataDiffBitCounter > 0) &&
                      (secondDataDiffPos >= mMixDataInfoProcessedStackTop))
                  {
                     // Second data is needed. Move it and advance stack pointer.
                     mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
                     secondDataDiffPos--;
                     memcpy(pFirstDataDiffPtr, pSecondDataDiffPtr,
                            mMixDataInfoStackStep*sizeof(int));
                     PRINT_TOP_MIX_DATA();
                     mpMixDataInfoStackTop += mMixDataInfoStackStep;
                     mMixDataInfoProcessedStackTop++;
                  }
               }
            }
            assert(mpMixDataInfoStackTop<mpMixDataInfoStack+mMixDataInfoStackLength);

            if (firstDataDiffBitCounter > 0)
            {
               // Queue command to generate first data and mark it as processed
               // if it is not a subset of second data. In later case leave it
               // unprocessed for later processing.
               pushMixActionMix(commonDataPos, firstDataDiffPos, dataInfoStackPos);
//               moveDataUp(numActiveInputs, firstDataDiffPos);
            }

            if (secondDataDiffBitCounter > 0)
            {
               // Queue command to generate second data and mark it as processed
               // if it is not a subset of first data. In later case leave it
               // unprocessed for later processing.
               pushMixActionMix(commonDataPos, secondDataDiffPos, closestDataInfoStackPos);
//               moveDataUp(numActiveInputs, secondDataDiffPos);
            }

//            moveDataUp(numActiveInputs, commonDataPos);
         }
         else
         {
            // No pair for this data found. Cut one input from it and continue.
            // Note, that data must have more then one "1", because it is
            // not just an input.

            // Copy data to the top of stack, and remove first "1" from it.
            memcpy(mpMixDataInfoStackTop, pDataInfoStackPtr,
                   sizeof(int)*mMixDataInfoStackStep);
            mpMixDataInfoProcessedStack[mMixDataInfoProcessedStackTop] = -1;
            int extInput;
            for (extInput=0; extInput<mMixDataInfoStackStep; extInput++)
            {
               if (mpMixDataInfoStackTop[extInput] == 1)
               {
                  mpMixDataInfoStackTop[extInput] = 0;
                  break;
               }
            }
            assert(extInput<numActiveInputs);

            int newDataPos;
            searchExistingUnprocessedData(mpMixDataInfoStackTop, newDataPos,
                                          numActiveInputs);
            if (newDataPos >= mMixDataInfoProcessedStackTop)
            {
               // Advance stack pointer.
               PRINT_TOP_MIX_DATA();
               mpMixDataInfoStackTop += mMixDataInfoStackStep;
               mMixDataInfoProcessedStackTop++;
            }

            // Queue command to generate data.
            // Here we use knowledge, that inputs are located at the end of
            // data stack.
            pushMixActionMix(extInput, newDataPos, dataInfoStackPos);
//            moveDataUp(numActiveInputs, newDataPos);
         }

         if (mpMixDataInfoProcessedStack[dataInfoStackPos]>=0)
         {
            // Step to next data if we've processed this one.
            pDataInfoStackPtr += mMixDataInfoStackStep;
            dataInfoStackPos++;
         }
      }
      else
      {
         // Step to next data info.
         pDataInfoStackPtr += mMixDataInfoStackStep;
         dataInfoStackPos++;
      }
   }

   // Finalization: request copying of input data to temporary storage
   for (extInput=0; extInput<numActiveInputs; extInput++)
   {
      pushMixActionCopyFromInput(mpActiveInputsList[extInput], extInput);
   }

   //
   //  Apply mix actions from stack.
   //
   initMixDataStack();

   // Initialize amplitudes if they haven't been initialized yet.
   for (int i=0; i<inBufsSize; i++)
   {
      if (mpPrevAmplitudes[i] < 0 && inBufs[i].isValid())
      {
         MpAudioBufPtr pAudioBuf = inBufs[i];
         mpPrevAmplitudes[i] = pAudioBuf->getAmplitude();
      }
   }

#ifdef TEST_PRINT_MIXING // [
   printf("-----------------------------------\n");
#endif // TEST_PRINT_MIXING ]

   for (int action=mMixActionsStackTop-1; action>=0; action--)
   {
      switch (mpMixActionsStack[action].mType)
      {
      case MixAction::DO_MIX:
         {
            MpDspUtils::add(&mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mSrc1],
                            &mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mSrc2],
                            &mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                            mMixDataStackStep);
            mpMixDataSpeechType[mpMixActionsStack[action].mDst] =
               mixSpeechTypes(mpMixDataSpeechType[mpMixActionsStack[action].mSrc1],
                              mpMixDataSpeechType[mpMixActionsStack[action].mSrc2]);
         }
#ifdef TEST_PRINT_MIXING // [
         printf("DO_MIX:     %2d + %2d -> %2d [0x%08X + 0x%08X -> 0x%08X]\n",
                mpMixActionsStack[action].mSrc1,
                mpMixActionsStack[action].mSrc2,
                mpMixActionsStack[action].mDst,
                mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mSrc1],
                mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mSrc2],
                mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst]);
#endif // TEST_PRINT_MIXING ]
         break;
      case MixAction::COPY_TO_OUTPUT:
         {
            const int src = mpMixActionsStack[action].mSrc1;

            if (src < numActiveInputs &&
                mExtendedInputs.getGain(src) == MP_BRIDGE_GAIN_PASSTHROUGH)
            {
               // This is direct input to output copy.
               const int origInput = mExtendedInputs.getOrigin(mpActiveInputsList[src]);
               outBufs[mpMixActionsStack[action].mDst] = inBufs[origInput];

#ifdef TEST_PRINT_MIXING // [
               printf("COPY_TO_OUTPUT*: %2d -> %2d\n",
                      mpMixActionsStack[action].mSrc1,
                      mpMixActionsStack[action].mDst);
#endif // TEST_PRINT_MIXING ]
            } 
            else
            {
               // This is mixed data. Convert it and copy to buffer.

               // Get buffer for output data.
               MpAudioBufPtr pOutBuf = MpMisc.RawAudioPool->getBuffer();
               assert(pOutBuf.isValid());
               pOutBuf->setSamplesNumber(samplesPerFrame);
               pOutBuf->setSpeechType(mpMixDataSpeechType[src]);

               MpDspUtils::convert_Att(&mpMixDataStack[mMixDataStackStep * src],
                                       pOutBuf->getSamplesWritePtr(),
                                       mMixDataStackStep, MP_BRIDGE_FRAC_LENGTH);

   #ifdef TEST_PRINT_MIXING // [
               printf("COPY_TO_OUTPUT:  %2d -> %2d [0x%08X -> 0x%08X]\n",
                      mpMixActionsStack[action].mSrc1,
                      mpMixActionsStack[action].mDst,
                      mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mSrc1],
                      *pOutBuf->getSamplesPtr());
   #endif // TEST_PRINT_MIXING ]

               outBufs[mpMixActionsStack[action].mDst].swap(pOutBuf);
            }
         }
         break;
      case MixAction::COPY_FROM_INPUT:
         {
            const int extInput = mpMixActionsStack[action].mSrc1;
            const int origInput = mExtendedInputs.getOrigin(extInput);
            const MpAudioBufPtr pInBuf(inBufs[origInput]);
            MpAudioSample prevAmplitude = mpPrevAmplitudes[origInput];
            MpAudioSample curAmplitude = pInBuf->getAmplitude();

            mpMixDataSpeechType[mpMixActionsStack[action].mDst] = pInBuf->getSpeechType();
            if (  mExtendedInputs.getGain(extInput) == MP_BRIDGE_GAIN_PASSTHROUGH
               && prevAmplitude == MpSpeechParams::MAX_AMPLITUDE
               && curAmplitude == MpSpeechParams::MAX_AMPLITUDE)
            {
               MpDspUtils::convert_Gain(pInBuf->getSamplesPtr(),
                                        &mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                                        mMixDataStackStep, MP_BRIDGE_FRAC_LENGTH);
#ifdef TEST_PRINT_MIXING // [
               printf("COPY_FROM_INPUT: %2d <- %2d [0x%08X <- 0x%08X <<%d]\n",
                      mpMixActionsStack[action].mDst,
                      mpMixActionsStack[action].mSrc1,
                      mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                      *pInBuf->getSamplesPtr(),
                      MP_BRIDGE_FRAC_LENGTH);
#endif // TEST_PRINT_MIXING ]
            }
            else if (curAmplitude == prevAmplitude)
            {
               // Calculate gain taking into account input amplitude.
               MpBridgeGain scaledGain = (MpBridgeGain)
                  ((mExtendedInputs.getGain(extInput)*MAX_AMPLITUDE_ROUND)/curAmplitude);

               MpDspUtils::mul(pInBuf->getSamplesPtr(),
                               scaledGain,
                               &mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                               mMixDataStackStep);
#ifdef TEST_PRINT_MIXING // [
               printf("COPY_FROM_INPUT: %2d <- %2d [0x%08X <- 0x%08X *%d]\n",
                      mpMixActionsStack[action].mDst,
                      mpMixActionsStack[action].mSrc1,
                      mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                      *pInBuf->getSamplesPtr(),
                      mExtendedInputs.getGain(extInput));
#endif // TEST_PRINT_MIXING ]
            }
            else
            {
               // Calculate gain start and end taking into account previous
               // and current input amplitudes.
               MpBridgeGain origGain = mExtendedInputs.getGain(extInput);
               MpBridgeGain scaledGainStart = (MpBridgeGain)
                  ((origGain*MAX_AMPLITUDE_ROUND)/prevAmplitude);
               MpBridgeGain scaledGainEnd = (MpBridgeGain)
                  ((origGain*MAX_AMPLITUDE_ROUND)/curAmplitude);

               MpDspUtils::mulLinear(pInBuf->getSamplesPtr(),
                                     scaledGainStart,
                                     scaledGainEnd,
                                     &mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                                     mMixDataStackStep);
#ifdef TEST_PRINT_MIXING // [
               printf("COPY_FROM_INPUT: %2d <- %2d [0x%08X <- 0x%08X *%d/(%d->%d)]\n",
                      mpMixActionsStack[action].mDst,
                      mpMixActionsStack[action].mSrc1,
                      mpMixDataStack[mMixDataStackStep * mpMixActionsStack[action].mDst],
                      *pInBuf->getSamplesPtr(),
                      origGain,
                      prevAmplitude,
                      curAmplitude);
#endif // TEST_PRINT_MIXING ]
            }
         }
         break;
      case MixAction::NO_OPERATION:
         break;
      }
   }

   // Save input amplitudes for later use.
   saveAmplitudes(inBufs, inBufsSize);

   return TRUE;
}

void MpBridgeAlgLinear::setGainMatrixValue(int column, int row, MpBridgeGain val)
{
   mExtendedInputs.setGain(column, row, val);
}

void MpBridgeAlgLinear::setGainMatrixRow(int row, int numValues, const MpBridgeGain val[])
{
   // Copy gain data to mix matrix row.
   for (int i=0; i<numValues; i++)
   {
      if (val[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         mExtendedInputs.setGain(i, row, val[i]);
      }
   }
}

void MpBridgeAlgLinear::setGainMatrixColumn(int column, int numValues, const MpBridgeGain val[])
{
   // Copy gain data to mix matrix column.
   for (int i=0; i<numValues; i++)
   {
      if (val[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         mExtendedInputs.setGain(column, i, val[i]);
      }
   }
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

