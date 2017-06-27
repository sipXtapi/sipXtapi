//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include <os/OsDefs.h> // for min macro
#include <mp/MpBuf.h>
#include <mp/MprBridge.h>
#include <mp/MpMisc.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprBridgeSetGainsMsg.h>
#include <mp/MpBridgeAlgSimple.h>
#include <mp/MpBridgeAlgLinear.h>

#ifdef PRINT_CLIPPING_STATS
#  include <os/OsSysLog.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
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
                     UtlBoolean mixSilence,
                     AlgType algorithm)
:  MpAudioResource(rName, 
                   1, maxInOutputs, 
                   1, maxInOutputs)
#ifdef TEST_PRINT_CONTRIBUTORS  // [
, mpMixContributors(NULL)
, mpLastOutputContributors(NULL)
#endif // TEST_PRINT_CONTRIBUTORS ]
, mAlgType(algorithm)
, mpBridgeAlg(NULL)
, mMixSilence(mixSilence)
#ifdef PRINT_CLIPPING_STATS
, mClippedFramesCounted(0)
, mpOutputClippingCount(NULL)
#endif

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

#ifdef PRINT_CLIPPING_STATS
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MprBridge::MprBridge new int[%d]", maxInOutputs);
   mpOutputClippingCount = new int[maxInOutputs];
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MprBridge::MprBridge after new int[%d]", maxInOutputs);
   OsSysLog::flush();
   for(int outIndex = 0; outIndex < maxInOutputs; outIndex++)
   {
      mpOutputClippingCount[outIndex] = 0;
   }
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MprBridge::MprBridge initialized maxInOutputs");
#endif
}

// Destructor
MprBridge::~MprBridge()
{
   delete mpBridgeAlg;

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

#ifdef PRINT_CLIPPING_STATS
   delete[] mpOutputClippingCount;
   mpOutputClippingCount = NULL;
#endif
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprBridge::setMixWeightsForOutput(int bridgeOutputPort,
                                           int numWeights,
                                           const MpBridgeGain gains[])
{
   assert(bridgeOutputPort >= 0);
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
    if(OsSysLog::willLog(FAC_MP, PRI_DEBUG))
    {
        UtlString gainsString("[");
        for(int gainIndex = 0; gainIndex < numWeights; gainIndex++)
        {
            if(gainIndex > 0)
            {
                gainsString.append(", ");
            }
            gainsString.appendFormat("%.2f", 
#ifdef MP_FIXED_POINT
                    ((float)gains[gainIndex]) / ((float)(1 << MP_BRIDGE_FRAC_LENGTH)));
#else
                    gains[gainIndex]);
#endif
        }
        gainsString.append("]");

        OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MprBridge::setMixWeightsForOutput(\"%s\", %p, %d, %d,\n%s)",
                namedResource.data(),
                &fgQ,
                bridgeOutputPort,
                numWeights,
                gainsString.data());
    }

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

   MpFlowGraphMsg msg(SET_WEIGHTS_FOR_INPUT, this,
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

OsStatus MprBridge::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
      // Check whether we've been added to flowgraph or removed.
      if (pFlowGraph != NULL)
      {
         switch (mAlgType)
         {
         case ALG_SIMPLE:
            mpBridgeAlg = new MpBridgeAlgSimple(maxInputs(), maxOutputs(),
                                                mMixSilence,
                                                mpFlowGraph->getSamplesPerFrame());
         	break;
         case ALG_LINEAR:
            mpBridgeAlg = new MpBridgeAlgLinear(maxInputs(), maxOutputs(),
                                                mMixSilence,
                                                mpFlowGraph->getSamplesPerFrame());
            break;
         default:
            assert(!"Unknown bridge algorithm type!");
            return OS_FAILED;
         }
      } 
      else
      {
         delete mpBridgeAlg;
         mpBridgeAlg = NULL;
      }
   }

   return res;
}

UtlBoolean MprBridge::doMix(MpBufPtr inBufs[], int inBufsSize,
                            MpBufPtr outBufs[], int outBufsSize,
                            int samplesPerFrame)
{
   assert(mpBridgeAlg != NULL);
   return mpBridgeAlg->doMix(inBufs, inBufsSize, outBufs, outBufsSize,
                             samplesPerFrame);
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
#ifdef TEST_PRINT
    UtlString weightString;
    for(int wIndex = 0; wIndex < numWeights; wIndex++)
    {
#  ifdef MP_FIXED_POINT
        weightString.appendFormat("%s%d", 
            wIndex > 0 ? ", " : "", gain[wIndex]);
#  else
        weightString.appendFormat("%s%f", 
            wIndex > 0 ? ", " : "", (double)gain[wIndex]);
#  endif
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MprBridge::handleSetMixWeightsForOutput(outputPort: %d, numWeights: %d, weights[%s]",
        bridgeOutputPort, numWeights, weightString.data());
#endif

   // New gains vector must feet into matrix
   assert(bridgeOutputPort >= 0);
   assert(numWeights <= maxInputs());
   if (numWeights > maxInputs())
   {
      return FALSE;
   }

   // Set gain for output.
   mpBridgeAlg->setGainMatrixRow(bridgeOutputPort, numWeights, gain);

   return TRUE;
}

UtlBoolean MprBridge::handleSetMixWeightsForInput(int bridgeInputPort,
                                                  int numWeights,
                                                  const MpBridgeGain gain[])
{
#ifdef TEST_PRINT
    UtlString weightString;
    for(int wIndex = 0; wIndex < numWeights; wIndex++)
    {
#  ifdef MP_FIXED_POINT
        weightString.appendFormat("%s%d", 
            wIndex > 0 ? ", " : "", gain[wIndex]);
#  else
        weightString.appendFormat("%s%f", 
            wIndex > 0 ? ", " : "", (double)gain[wIndex]);
#  endif
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MprBridge::handleSetMixWeightsForInput(inputPort: %d, numWeights: %d, weights[%s]",
        bridgeInputPort, numWeights, weightString.data());
#endif

   // New gains vector must fit into matrix
   assert(numWeights <= maxOutputs());
   if (numWeights > maxOutputs())
   {
      return FALSE;
   }

   // Set gain for input.
   mpBridgeAlg->setGainMatrixColumn(bridgeInputPort, numWeights, gain);

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

#ifdef PRINT_CLIPPING_STATS
   UtlString outputsClippingMessage;
   mClippedFramesCounted++;
   //OsSysLog::add(FAC_MP, PRI_DEBUG, "Bridge clipping frame count: %d outBufsSize: %d", 
   //   mClippedFramesCounted, outBufsSize);

   for(int outIndex = 0; outIndex < outBufsSize; outIndex++)
   {
      if(outBufs[outIndex].isValid())
      {
         MpAudioBufPtr audioBufPtr = outBufs[outIndex];

          mpOutputClippingCount[outIndex] +=
            MpDspUtils::countClippedValues(audioBufPtr->getSamplesPtr(), 
                                           audioBufPtr->getSamplesNumber());
      }

      if(mClippedFramesCounted >= PRINT_CLIPPING_FREQUENCY)
      {
         outputsClippingMessage.appendFormat("\noutput[%d]: %d", 
            outIndex, mpOutputClippingCount[outIndex]);

         mpOutputClippingCount[outIndex] = 0;
      }
   }

   if(mClippedFramesCounted >= PRINT_CLIPPING_FREQUENCY)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "Bridge clipping for last %d samples on:%s", 
         PRINT_CLIPPING_FREQUENCY * mpFlowGraph->getSamplesPerFrame(), 
         outputsClippingMessage.data());

      mClippedFramesCounted = 0;
   }

   //OsSysLog::add(FAC_MP, PRI_DEBUG, "Bridge clipping frame count: %d", mClippedFramesCounted);
#endif

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


