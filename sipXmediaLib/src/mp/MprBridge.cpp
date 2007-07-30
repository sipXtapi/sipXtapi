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
#include <limits.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprBridge.h>
#include <mp/MpMisc.h>
#include <mp/MprBridgeSetGainsMsg.h>

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
, mpMixAccumulator(NULL)
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

   // Allocate mix matrix.
   mpGainMatrix = new MpBridgeGain[maxInputs()*maxOutputs()];
   assert(mpGainMatrix != NULL);

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
   mMixDataStackStep = getSamplesPerFrame();
   mMixDataStackLength = maxInputs()*maxOutputs()*mMixDataStackStep;
   mpMixDataStack = new MpBridgeAccum[mMixDataStackLength];
   // Allocate array for mix temporary data info.
   mMixDataInfoStackStep = maxInputs()*maxOutputs();
   mMixDataInfoStackLength = maxInputs()*maxOutputs()*mMixDataInfoStackStep;
   mpMixDataInfoStack = new int[mMixDataInfoStackLength];
   mpMixDataInfoStackTop = mpMixDataInfoStack;
   mMixDataInfoProcessedStackLength = maxInputs()*maxOutputs();
   mpMixDataInfoProcessedStack = new int[mMixDataInfoProcessedStackLength];
   // Allocate temporary storage for mixing data.
   mpMixAccumulator = new MpBridgeAccum[getSamplesPerFrame()];
   assert(mpMixAccumulator != NULL);
}

// Destructor
MprBridge::~MprBridge()
{
   delete[] mpGainMatrix;
   delete[] mpActiveInputsList;
   delete[] mpMixActionsStack;
   delete[] mpMixDataStack;
   delete[] mpMixDataInfoStack;
   delete[] mpMixDataInfoProcessedStack;
   delete[] mpMixAccumulator;

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
#ifdef LINEAR_COMPLEXITY_BRIDGE // [
   int numActiveInputs = 0;
   const int numExtendedInputs = mExtendedInputs.getExtendedInputsNum();

   assert(inBufsSize == maxInputs());
   assert(outBufsSize == maxOutputs());

   //
   //  Build list of active inputs (stored in mpActiveInputsList).
   //
#ifdef TEST_PRINT_CONTRIBUTORS // [
   printf("Contributors (inversed order): ");
#endif // TEST_PRINT_CONTRIBUTORS ]
   int extInput;
   for ( extInput=numExtendedInputs-1; extInput>=0; extInput-- )
   {
      if (mExtendedInputs.isNotMuted(extInput))
      {
#ifdef TEST_PRINT_CONTRIBUTORS // [
         printf("(%d)NM",extInput);
#endif // TEST_PRINT_CONTRIBUTORS ]
         int origInput = mExtendedInputs.getOrigin(extInput);
         if (inBufs[origInput].isValid())
         {
#ifdef TEST_PRINT_CONTRIBUTORS // [
            printf("V");
#endif // TEST_PRINT_CONTRIBUTORS ]
            MpAudioBufPtr pAudio = inBufs[origInput];
            if (pAudio->isActiveAudio())
            {
#ifdef TEST_PRINT_CONTRIBUTORS // [
               printf("A");
#endif // TEST_PRINT_CONTRIBUTORS ]
               mpActiveInputsList[numActiveInputs] = extInput;
               numActiveInputs++;
#ifdef TEST_PRINT_CONTRIBUTORS // [
               printf("+");
#endif // TEST_PRINT_CONTRIBUTORS ]
            }
         }
#ifdef TEST_PRINT_CONTRIBUTORS // [
         printf(" ");
#endif // TEST_PRINT_CONTRIBUTORS ]
      }
   }
#ifdef TEST_PRINT_CONTRIBUTORS // [
   printf("\n");
   printf("Total active inputs: %d\n", numActiveInputs);
#endif // TEST_PRINT_CONTRIBUTORS ]

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
               pOutBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);

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
            if (mExtendedInputs.getGain(extInput) == MP_BRIDGE_GAIN_PASSTHROUGH)
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
            else
            {
               MpDspUtils::mul(pInBuf->getSamplesPtr(),
                               mExtendedInputs.getGain(extInput),
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
         }
         break;
      case MixAction::NO_OPERATION:
         break;
      }
   }

   return TRUE;
#else // LINEAR_COMPLEXITY_BRIDGE ][

   // Loop over all outputs and mix
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
               MpDspUtils::add_IGain(pFrame->getSamplesPtr(), mpMixAccumulator,
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
      MpDspUtils::convert_Att(mpMixAccumulator, pOutBuf->getSamplesWritePtr(),
                              samplesPerFrame, MP_BRIDGE_FRAC_LENGTH);
      outBufs[outputNum].swap(pOutBuf);
   }

   return TRUE;

#endif // LINEAR_COMPLEXITY_BRIDGE ]

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
         mExtendedInputs.setGain(i, bridgeOutputPort, gain[i]);
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
         mExtendedInputs.setGain(bridgeInputPort, i, gain[i]);
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

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


